#include "Mesh.h"
#include "OpenGL.h"
#include "Textures.h"

#include "Log.h"

Mesh::Mesh() : Resource(ResourceType::MESH), VAO(0), VBO(0), EBO(0) {
    isLoaded = false;
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures)
    : Resource(ResourceType::MESH), vertices(vertices), indices(indices), textures(textures),
    VAO(0), VBO(0), EBO(0)
{
    setupMesh();
    isLoaded = true;


    drawVertNormals = false;
    drawFaceNormals = false;


    CalculateNormals();

    uuid = UUIDGen::GenerateUUID();

}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);
}

void Mesh::Draw(Shader& shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int roughnessNr = 1;
    unsigned int metallicNr = 1;
    unsigned int aoNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = textures[i].get()->mapType;
        

        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_roughness")
            number = std::to_string(roughnessNr++);
        else if (name == "texture_metallic")
            number = std::to_string(metallicNr++);
        else if (name == "texture_ao")
            number = std::to_string(aoNr++);

        shader.setInt((name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].get()->GetUUID());
    }

    if (drawFaceNormals) {


        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), true);
        glUniform4f(glGetUniformLocation(shader.ID, "lineColor"), 0.0f, 1.0f, 0.0f, 1.0f); //green for vertex


        glBegin(GL_LINES);


        for (int i = 0; i < indices.size(); i += 3) {
            glm::vec3 start = vertices[indices[i]].Position;
            glm::vec3 end = start + normals[indices[i]] * 0.2f;
            glVertex3fv(glm::value_ptr(start));
            glVertex3fv(glm::value_ptr(end));
        }

        glEnd();
        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), false);
    }

    if (drawVertNormals) {


        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), true);
        glUniform4f(glGetUniformLocation(shader.ID, "lineColor"), 0.0f, 0.9f, 1.0f, 1.0f); //blue for face

        glBegin(GL_LINES);


        for (int i = 0; i < vertices.size(); i += 3) {
            glm::vec3 v0 = vertices[indices[i]].Position;
            glm::vec3 v1 = vertices[indices[i + 1]].Position;
            glm::vec3 v2 = vertices[indices[i + 2]].Position;


            glm::vec3 normalDir = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            glm::vec3 center = (v0 + v1 + v2) / 3.0f;
            glm::vec3 end = center + normalDir * 0.2f;


            glVertex3fv(glm::value_ptr(center));
            glVertex3fv(glm::value_ptr(end));
        }
        glEnd();
        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), false);
    }


    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

void Mesh::SetMeshData(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds, const std::vector<std::shared_ptr<Texture>>& texs) {
    // Unload old mesh data if it exists
    if (isLoaded) {
        UnloadFromMemory();
    }

    // Set new data
    vertices = verts;
    indices = inds;
    textures = texs;

    // Setup OpenGL buffers with new data
    setupMesh();
    isLoaded = true;

    LOG("Mesh data updated: %d vertices, %d indices, %d textures",
        vertices.size(), indices.size(), textures.size());
}

bool Mesh::LoadToMemory() {
    if (isLoaded) return true;

    if (vertices.empty() || indices.empty()) {
        LOG("ERROR: Cannot load mesh - no vertex/index data");
        return false;
    }

    setupMesh();
    isLoaded = true;
    return true;
}

void Mesh::UnloadFromMemory() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO != 0) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
}

void Mesh::CalculateNormals() {
    normals.resize(vertices.size(), glm::vec3(0.0f));


    for (int i = 0; i < indices.size(); i += 3) {
        glm::vec3 v0 = vertices[indices[i]].Position;
        glm::vec3 v1 = vertices[indices[i + 1]].Position;
        glm::vec3 v2 = vertices[indices[i + 2]].Position;

        //With just vertices[i] instead of vertices[indices[i]], you’d be assuming that every 3 consecutive vertices form a triangle.
        //However, that's not always the case, as most meshes reuse vertices between faces.

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        //compute cross product with v0->v1, v0->v2 
        //both from the same point (v0), because the resulting perpendicular vector has to sit on a common vertex

        //add normal vector on top of each vertex
        normals[indices[i]] += normal;
        normals[indices[i + 1]] += normal;
        normals[indices[i + 2]] += normal;

    }

    for (glm::vec3 normal : normals)
    {
        //since we added the normals to the indices, they aren't normalized anymore, so we do it again
        normal = glm::normalize(normal);
    }

}