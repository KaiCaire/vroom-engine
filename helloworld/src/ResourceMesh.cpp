#include "ResourceMesh.h"
#include "OpenGL.h"
#include "ResourceTexture.h"

#include "Log.h"

ResourceMesh::ResourceMesh() : Resource(ResourceType::MESH), VAO(0), VBO(0), EBO(0) {
    isLoadedToRAM = false;
    isLoadedToGPU = false;
}

ResourceMesh::ResourceMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<ResourceTexture>> textures)
    : Resource(ResourceType::MESH), vertices(vertices), indices(indices), textures(textures), VAO(0), VBO(0), EBO(0)
{
    LoadToGPU();

    drawVertNormals = false;
    drawFaceNormals = false;


    CalculateNormals();

    uuid = UUIDGen::GenerateUUID();

}

void ResourceMesh::LoadToGPU() {
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

    isLoadedToGPU = true;
}

void ResourceMesh::Draw(Shader& shader) {
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
        glBindTexture(GL_TEXTURE_2D, textures[i].get()->gpu_id);
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

void ResourceMesh::SetMeshData(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds, const std::vector<std::shared_ptr<ResourceTexture>>& texs) {
    // Unload old mesh data if it exists
    if (isLoadedToRAM) {
        FreeMemory();
    }

    // Set new data
    vertices = verts;
    indices = inds;
    textures = texs;

    // Setup OpenGL buffers with new data
    LoadToGPU();
   

    LOG("Mesh data updated: %d vertices, %d indices, %d textures", vertices.size(), indices.size(), textures.size());
}



void ResourceMesh::CalculateNormals() {
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

void ResourceMesh::SaveBin() {
    
    std::string directory = fs->GetDirFromPath(libraryPath.c_str());
    if (!directory.empty() && !fs->Exists(directory.c_str())) {
        fs->CreateDir(directory.c_str());
    }
   
    uint vertexDataSize = (uint)vertices.size() * sizeof(float);
    uint indexDataSize = (uint)indices.size() * sizeof(unsigned int);

    uint headerSize = sizeof(VroomUUID) + sizeof(uint) * 2;
    uint totalSize = headerSize + vertexDataSize + indexDataSize;
    
    char* buffer = new char[totalSize];
    char* cursor = buffer;

    VroomUUID uuid = GetUUID();
    memcpy(cursor, &uuid, sizeof(VroomUUID));
    cursor += sizeof(VroomUUID);

    uint currentNumVertices = (uint)vertices.size();
    memcpy(cursor, &currentNumVertices, sizeof(uint));
    cursor += sizeof(uint);

    uint currentNumIndices = (uint)indices.size();
    memcpy(cursor, &currentNumIndices, sizeof(uint));
    cursor += sizeof(uint);

    
    if (!vertices.empty()) {
        memcpy(cursor, vertices.data(), vertexDataSize);
        cursor += vertexDataSize;
    }

    if (!indices.empty()) {
        memcpy(cursor, indices.data(), indexDataSize);
        cursor += indexDataSize;
    }

    fs->WriteBinData(libraryPath.c_str(), buffer, totalSize);

    delete[] buffer;

    LOG("Mesh saved in: %s (Vertices: %u, Indices: %u)", libraryPath.c_str(), currentNumVertices, currentNumIndices);
}


void ResourceMesh::LoadBin() {
    uint fileSize;
    char* buffer = fs->ReadBinData(libraryPath.c_str(), &fileSize);
    if (!buffer) {
        LOG("Error loading mesh: %s", libraryPath.c_str());
        return;
    }

    char* cursor = buffer;

    // --- READ HEADER ---
    VroomUUID loadedUUID;
    memcpy(&loadedUUID, cursor, sizeof(VroomUUID));
    cursor += sizeof(VroomUUID);

    // Read counts

    memcpy(&numVertices, cursor, sizeof(uint));
    cursor += sizeof(uint);
    memcpy(&numIndices, cursor, sizeof(uint));
    cursor += sizeof(uint);

    // 2. Calculate expected data sizes
    uint vertexDataSize = numVertices * sizeof(float);
    uint indexDataSize = numIndices * sizeof(unsigned int);

    // 3. Allocate vectors and copy data

    // Copy vertex data
    vertices.resize(numVertices);
    memcpy(vertices.data(), cursor, vertexDataSize);
    cursor += vertexDataSize;

    // Copy index data
    indices.resize(numIndices);
    memcpy(indices.data(), cursor, indexDataSize);
    

    
    delete[] buffer;

    isLoadedToRAM = true;
    LOG("Mesh loaded: %s (Vertices: %u, Indices: %u)", libraryPath.c_str(), numVertices, numIndices);
}

void ResourceMesh::FreeMemory() {
    vertices.clear();
    vertices.shrink_to_fit();
    indices.clear();
    indices.shrink_to_fit();

    // Reset counts for safety
    numVertices = 0;
    numIndices = 0;

    isLoadedToRAM = false;
   
}

void ResourceMesh::SaveMeta() const {
     std::string metaPath = assetsPath + ".meta";
     nlohmann::json meta = fs->LoadJSON(metaPath.c_str());

     meta["uuid"] = GetUUID();
     meta["modTime"] = fs->GetFileModTime(assetsPath);
     meta["name"] = name;
     meta["numVertices"] = numVertices; 
     meta["numIndices"] = numIndices;   

     fs->SaveJSON(metaPath.c_str(), meta);
}


void ResourceMesh::LoadMeta() {
     std::string metaPath = assetsPath + ".meta";
     if (!fs->Exists(metaPath.c_str())) { return; }
     nlohmann::json meta = fs->LoadJSON(metaPath.c_str());

     if (meta.contains("name")) name = meta["name"];
     if (meta.contains("numVertices")) numVertices = meta["numVertices"]; 
     if (meta.contains("numIndices")) numIndices = meta["numIndices"];    
}

void ResourceMesh::UnloadFromGPU() {
    if (!isLoadedToGPU) {
        // LOG("Mesh already unloaded from GPU or never loaded: %s", name.c_str());
        return;
    }

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);


    VAO = VBO = EBO = 0;
    isLoadedToGPU = false;
    // LOG("Mesh unloaded from GPU: %s", name.c_str());
}
