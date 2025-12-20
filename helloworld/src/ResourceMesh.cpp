#include "ResourceMesh.h"
#include "OpenGL.h"
#include "ResourceTexture.h"
#include "Importer.h"
#include "TextureImporter.h"

#include "Log.h"

ResourceMesh::ResourceMesh() : Resource(ResourceType::MESH), VAO(0), VBO(0), EBO(0) {
    isLoadedToRAM = false;
    isLoadedToGPU = false;

}

ResourceMesh::ResourceMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<ResourceTexture>> textures)
    : Resource(ResourceType::MESH), vertices(vertices), indices(indices), textures(textures), VAO(0), VBO(0), EBO(0)
{

    isLoadedToRAM = true;
    LoadToGPU();

    drawVertNormals = false;
    drawFaceNormals = false;


    CalculateNormals();

    //uuid = 
    // GenerateUUID();

}

ResourceMesh::ResourceMesh(std::shared_ptr<ResourceMesh> mesh) : Resource(ResourceType::MESH), vertices(mesh->vertices), indices(mesh->indices), textures(mesh->textures), VAO(0), VBO(0), EBO(0)
{
    isLoadedToRAM = mesh->isLoadedToRAM;

    drawVertNormals = mesh->drawVertNormals;
    drawFaceNormals = mesh->drawFaceNormals;

    // Normals may need recalculation
    CalculateNormals();

    // Upload to GPU
    LoadToGPU();
}

ResourceMesh::~ResourceMesh() {
    UnloadFromGPU();
    FreeMemory();
}


void ResourceMesh::LoadToGPU() {

    if (vertices.empty() || indices.empty()) {
        LOG("WARNING: Attempted to upload empty mesh to GPU. Skipping.");
        return;
    }

    if (isLoadedToGPU){

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

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

//void ResourceMesh::Draw(Shader& shader) {
//
//    unsigned int diffuseNr = 1;
//    unsigned int specularNr = 1;
//    unsigned int normalNr = 1;
//    unsigned int roughnessNr = 1;
//    unsigned int metallicNr = 1;
//    unsigned int aoNr = 1;
//
//    for (unsigned int i = 0; i < textures.size(); i++) {
//        glActiveTexture(GL_TEXTURE0 + i);
//
//        std::string number;
//        std::string name = textures[i].get()->mapType;
//        
//
//        if (name == "texture_diffuse")
//            number = std::to_string(diffuseNr++);
//        else if (name == "texture_specular")
//            number = std::to_string(specularNr++);
//        else if (name == "texture_normal")
//            number = std::to_string(normalNr++);
//        else if (name == "texture_roughness")
//            number = std::to_string(roughnessNr++);
//        else if (name == "texture_metallic")
//            number = std::to_string(metallicNr++);
//        else if (name == "texture_ao")
//            number = std::to_string(aoNr++);
//
//        std::string uniformName = name + number;
//        shader.setInt(uniformName.c_str(), i);
//
//       /*
//        LOG("Binding texture: uniform=%s, unit=%d, gpu_id=%u, UUID=%llu",
//            uniformName.c_str(),
//            i,
//            textures[i].get()->gpu_id,
//            textures[i].get()->GetUUID());*/
//
//        glBindTexture(GL_TEXTURE_2D, textures[i].get()->gpu_id);
//    }
//
//    if (drawFaceNormals) {
//
//
//        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), true);
//        glUniform4f(glGetUniformLocation(shader.ID, "lineColor"), 0.0f, 1.0f, 0.0f, 1.0f); //green for vertex
//
//
//        glBegin(GL_LINES);
//
//
//        for (int i = 0; i < indices.size(); i += 3) {
//            glm::vec3 start = vertices[indices[i]].Position;
//            glm::vec3 end = start + normals[indices[i]] * 0.2f;
//            glVertex3fv(glm::value_ptr(start));
//            glVertex3fv(glm::value_ptr(end));
//        }
//
//        glEnd();
//        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), false);
//    }
//
//    if (drawVertNormals) {
//
//
//        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), true);
//        glUniform4f(glGetUniformLocation(shader.ID, "lineColor"), 0.0f, 0.9f, 1.0f, 1.0f); //blue for face
//
//        glBegin(GL_LINES);
//
//
//        for (int i = 0; i < vertices.size(); i += 3) {
//            glm::vec3 v0 = vertices[indices[i]].Position;
//            glm::vec3 v1 = vertices[indices[i + 1]].Position;
//            glm::vec3 v2 = vertices[indices[i + 2]].Position;
//
//
//            glm::vec3 normalDir = glm::normalize(glm::cross(v1 - v0, v2 - v0));
//            glm::vec3 center = (v0 + v1 + v2) / 3.0f;
//            glm::vec3 end = center + normalDir * 0.2f;
//
//
//            glVertex3fv(glm::value_ptr(center));
//            glVertex3fv(glm::value_ptr(end));
//        }
//        glEnd();
//        glUniform1i(glGetUniformLocation(shader.ID, "useLineColor"), false);
//    }
//
//
//    glBindVertexArray(VAO);
//    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
//
//    glActiveTexture(GL_TEXTURE0);
//
//
//}

void ResourceMesh::Draw(Shader& shader, MaterialComponent* material) {

    // CRITICAL: If the mesh isn't on the GPU, don't even try to bind or draw.
    if (!isLoadedToGPU || VAO == 0) {
        return;
    }

    unsigned int unit = 0;
    // Helper to bind textures and set the sampler uniform inside the 'material' struct
    auto bindTex = [&](const std::shared_ptr<ResourceTexture>& tex, const std::string& memberName) {
        if (tex && tex->isLoadedToGPU) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, tex->gpu_id);

            // Matches your shader struct: "material.texture_diffuse1"
            std::string uniformName = "material." + memberName;
            shader.setInt(uniformName.c_str(), unit);
            unit++;
        }
        };

    if (material) {
        // Bind maps from the material component
        bindTex(material->GetDiffuseMap(), "texture_diffuse1");
        bindTex(material->GetNormalMap(), "texture_normal1");
        bindTex(material->GetMetallicMap(), "texture_metallic1");
        bindTex(material->GetRoughnessMap(), "texture_roughness1");
        bindTex(material->GetAOMap(), "texture_ao1");


    }
    else {
        //USE CHECKERS AS FALLBACK
        std::string defaultPath = Application::GetInstance().importer.get()->defaultTexDir;
        //should use a weakptr for better performance but don't have the time to waste on extra refactoring, if it works it works lol
        std::shared_ptr<Resource> defaultResource = Application::GetInstance().resourceManager->RequestResource(defaultPath);
        std::shared_ptr<ResourceTexture> defaultTex = std::dynamic_pointer_cast<ResourceTexture>(defaultResource);

        //binds the default texture to the diffuse slot
        bindTex(defaultTex, "texture_diffuse1");
    }

    // 2. DRAW FACE NORMALS (DEBUG)
    if (drawFaceNormals) {
        shader.setBool("useLineColor", true);
        shader.setVec4("lineColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green

        glBegin(GL_LINES);
        for (int i = 0; i < (int)indices.size(); i += 3) {
            glm::vec3 start = vertices[indices[i]].Position;
            glm::vec3 end = start + normals[indices[i]] * 0.2f;
            glVertex3fv(glm::value_ptr(start));
            glVertex3fv(glm::value_ptr(end));
        }
        glEnd();
        shader.setBool("useLineColor", false);
    }

    // 3. DRAW VERTEX NORMALS (DEBUG)
    if (drawVertNormals) {
        shader.setBool("useLineColor", true);
        shader.setVec4("lineColor", glm::vec4(0.0f, 0.9f, 1.0f, 1.0f)); // Blue

        glBegin(GL_LINES);
        for (int i = 0; i < (int)vertices.size(); i += 3) {
            if (i + 2 >= (int)indices.size()) break; // Safety check

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
        shader.setBool("useLineColor", false);
    }

    // 4. MAIN GEOMETRY RENDER
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset state
    glActiveTexture(GL_TEXTURE0);
}

void ResourceMesh::SetMeshData(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds/*, const std::vector<std::shared_ptr<ResourceTexture>>& texs*/) {
    // Unload old mesh data if it exists
    if (isLoadedToRAM) {
        FreeMemory();
    }

    // Set new data
    vertices = verts;
    indices = inds;
    //textures = texs;

    if (!textures.empty()) {
        textures.clear();
    }

    // 3. Update counts and GPU state
    numVertices = (uint)vertices.size();
    numIndices = (uint)indices.size();

    // Setup OpenGL buffers with new data
    LoadToGPU();
    isLoadedToRAM = true;

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

//currently saving textures
//void ResourceMesh::SaveBin() {
//
//    if (!isLoadedToRAM) {
//        LOG("ERROR: Cannot save mesh binary - data not loaded to RAM");
//        return;
//    }
//
//    std::string binPath = GetLibraryFilePath();
//    std::string meshAssetPath = GetAssetFilePath();
//    
//    if (binPath.empty()) {
//        LOG("ERROR: Library path not set for mesh");
//        return;
//    }
//    
//    std::string directory = fs->GetDirFromPath(libraryPath.c_str());
//    if (!directory.empty() && !fs->Exists(directory.c_str())) {
//        fs->CreateDir(directory.c_str());
//    }
//   
//    // Calculate total size needed
//    uint vertexCount = vertices.size();
//    uint indexCount = indices.size();
//    uint textureCount = textures.size();
//
//
//
//    // Calculate buffer size
//    size_t bufferSize = sizeof(uint) * 3; // header (3 counts)
//    bufferSize += vertexCount * sizeof(Vertex);
//    bufferSize += indexCount * sizeof(unsigned int);
//
//    // Add texture string sizes
//    for (const auto& tex : textures) {
//        bufferSize += sizeof(uint); // path length
//        std::string texAssetPath = tex->GetAssetFilePath();
//        bufferSize += texAssetPath.length();
//        bufferSize += sizeof(uint); // type length
//        bufferSize += tex.get()->mapType.length();
//    }
//
//    // Create buffer
//    char* buffer = new char[bufferSize];
//    char* ptr = buffer;
//
//    // Write header
//    std::memcpy(ptr, &vertexCount, sizeof(uint));
//    ptr += sizeof(uint);
//    std::memcpy(ptr, &indexCount, sizeof(uint));
//    ptr += sizeof(uint);
//    std::memcpy(ptr, &textureCount, sizeof(uint));
//    ptr += sizeof(uint);
//
//    // Write vertices
//    std::memcpy(ptr, vertices.data(), vertexCount * sizeof(Vertex));
//    ptr += vertexCount * sizeof(Vertex);
//
//    // Write indices
//    std::memcpy(ptr, indices.data(), indexCount * sizeof(unsigned int));
//    ptr += indexCount * sizeof(unsigned int);
//
//    // Write texture info
//    for (const auto& tex : textures) {
//        std::string texPath = tex.get()->GetAssetFilePath();
//        uint pathLength = texPath.length();
//        std::memcpy(ptr, &pathLength, sizeof(uint));
//        ptr += sizeof(uint);
//        std::memcpy(ptr, texPath.c_str(), pathLength);
//        ptr += pathLength;
//
//        uint typeLength = tex.get()->mapType.length();
//        std::memcpy(ptr, &typeLength, sizeof(uint));
//        ptr += sizeof(uint);
//        std::memcpy(ptr, tex.get()->mapType.c_str(), typeLength);
//        ptr += typeLength;
//    }
//
//    // Write using FileSystem
//    fs->WriteBinData(binPath.c_str(), buffer, bufferSize);
//
//
//    delete[] buffer;
//
//    LOG("Mesh binary saved: %s", binPath.c_str());
//}

void ResourceMesh::SaveBin() {

    if (!isLoadedToRAM) {
        LOG("ERROR: Cannot save mesh binary - data not loaded to RAM");
        return;
    }

    std::string binPath = GetLibraryFilePath();

    if (binPath.empty()) {
        LOG("ERROR: Library path not set for mesh");
        return;
    }

    std::string directory = fs->GetDirFromPath(libraryPath.c_str());
    if (!directory.empty() && !fs->Exists(directory.c_str())) {
        fs->CreateDir(directory.c_str());
    }

    // Calculate total size needed (Removed textureCount)
    uint vertexCount = vertices.size();
    uint indexCount = indices.size();

    // Calculate buffer size: Header (2 counts) + Vertices + Indices
    size_t bufferSize = sizeof(uint) * 2;
    bufferSize += vertexCount * sizeof(Vertex);
    bufferSize += indexCount * sizeof(unsigned int);

    // Create buffer
    char* buffer = new char[bufferSize];
    char* ptr = buffer;

    // Write header (Only vertex and index counts)
    std::memcpy(ptr, &vertexCount, sizeof(uint));
    ptr += sizeof(uint);
    std::memcpy(ptr, &indexCount, sizeof(uint));
    ptr += sizeof(uint);

    // Write vertices
    std::memcpy(ptr, vertices.data(), vertexCount * sizeof(Vertex));
    ptr += vertexCount * sizeof(Vertex);

    // Write indices
    std::memcpy(ptr, indices.data(), indexCount * sizeof(unsigned int));
    ptr += indexCount * sizeof(unsigned int);

    // Write using FileSystem
    fs->WriteBinData(binPath.c_str(), buffer, bufferSize);

    delete[] buffer;

}
//currently loading textures
//void ResourceMesh::LoadBin() {
//    std::string binPath = GetLibraryFilePath();
//    if (binPath.empty()) {
//        LOG("ERROR: Library path not set for mesh");
//        return;
//    }
//    fs = Application::GetInstance().fileSystem.get();
//    if (!fs->Exists(binPath.c_str())) {
//        LOG("ERROR: Mesh binary file does not exist: %s", binPath.c_str());
//        return;
//    }
//
//    // Read using FileSystem
//    uint size = 0;
//    char* buffer = fs->ReadBinData(binPath.c_str(), &size);
//    if (!buffer) {
//        LOG("ERROR: Failed to read mesh binary: %s", binPath.c_str());
//        return;
//    }
//
//    char* ptr = buffer;
//
//    // Read header
//    uint vertexCount, indexCount, textureCount;
//    std::memcpy(&vertexCount, ptr, sizeof(uint));
//    ptr += sizeof(uint);
//    std::memcpy(&indexCount, ptr, sizeof(uint));
//    ptr += sizeof(uint);
//    std::memcpy(&textureCount, ptr, sizeof(uint));
//    ptr += sizeof(uint);
//
//    // Read vertices
//    vertices.resize(vertexCount);
//    std::memcpy(vertices.data(), ptr, vertexCount * sizeof(Vertex));
//    ptr += vertexCount * sizeof(Vertex);
//
//    // Read indices
//    indices.resize(indexCount);
//    std::memcpy(indices.data(), ptr, indexCount * sizeof(unsigned int));
//    ptr += indexCount * sizeof(unsigned int);
//
//    if (textures.size() != 0) {
//        textures.clear();
//    }
//    textures.reserve(textureCount);
//
//    for (uint i = 0; i < textureCount; i++) {
//        uint pathLength;
//        std::memcpy(&pathLength, ptr, sizeof(uint));
//        ptr += sizeof(uint);
//
//        std::string path(ptr, pathLength);
//        ptr += pathLength;
//
//        uint typeLength;
//        std::memcpy(&typeLength, ptr, sizeof(uint));
//        ptr += sizeof(uint);
//
//        std::string mapType(ptr, typeLength);
//        ptr += typeLength;
//
//       
//        /*auto tex = Application::GetInstance().importer.get()->textureImporter->Import(path);*/
//        auto resTex = Application::GetInstance().resourceManager.get()->RequestResource(path);
//        auto tex = std::dynamic_pointer_cast<ResourceTexture>(resTex);
//        
//        
//        if (!tex) {
//            LOG("WARNING: Failed to load texture '%s' from cached mesh", path.c_str());
//            // Create placeholder
//            tex = std::make_shared<ResourceTexture>();
//            tex->SetAssetFilePath(path);
//            tex->SetName(fs->GetFileNameFromPath(path.c_str()));
//            
//        }
//        
//        tex->mapType = mapType;
//        textures.push_back(tex);
//        LOG("Loaded texture: %s (type: %s, gpu_id: %u)", path.c_str(), mapType.c_str(), tex->gpu_id);
//    }
//
//    delete[] buffer;
//    isLoadedToRAM = true;
//    LOG("Mesh binary loaded: %s (%d vertices, %d indices, %d textures)", binPath.c_str(), vertexCount, indexCount, textureCount);
//}

void ResourceMesh::LoadBin() {
    std::string binPath = GetLibraryFilePath();
    if (binPath.empty()) {
        LOG("ERROR: Library path not set for mesh");
        return;
    }

    fs = Application::GetInstance().fileSystem.get();
    if (!fs->Exists(binPath.c_str())) {
        LOG("ERROR: Mesh binary file does not exist: %s", binPath.c_str());
        return;
    }

    // Read using FileSystem
    uint size = 0;
    char* buffer = fs->ReadBinData(binPath.c_str(), &size);
    if (!buffer) {
        LOG("ERROR: Failed to read mesh binary: %s", binPath.c_str());
        return;
    }

    char* ptr = buffer;

    // 1. Read header (Now only 2 uints: vertex and index counts)
    uint vertexCount, indexCount;
    std::memcpy(&vertexCount, ptr, sizeof(uint));
    ptr += sizeof(uint);
    std::memcpy(&indexCount, ptr, sizeof(uint));
    ptr += sizeof(uint);

    // 2. Read vertices
    vertices.resize(vertexCount);
    std::memcpy(vertices.data(), ptr, vertexCount * sizeof(Vertex));
    ptr += vertexCount * sizeof(Vertex);

    // 3. Read indices
    indices.resize(indexCount);
    std::memcpy(indices.data(), ptr, indexCount * sizeof(unsigned int));
    ptr += indexCount * sizeof(unsigned int);

    // 4. Cleanup
    delete[] buffer;

    //// Clear any old texture references since this mesh is now geometry-only
    //if (!textures.empty()) {
    //    textures.clear();
    //}

    isLoadedToRAM = true;
    LOG("Mesh binary loaded: %s (%d vertices, %d indices)", binPath.c_str(), vertexCount, indexCount);
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

}

void ResourceMesh::SaveMeta() const {
  //  std::string assetPath = GetAssetFilePath();
  //  if (assetPath.empty()) {
  //      LOG("ERROR: Asset path not set for mesh");
  //      return;
  //  }

  //  std::string metaPath = assetPath + ".meta";
  // 

  //  nlohmann::json meta;
  //  meta["uuid"] = GetUUID();
  ///*  meta["modTime"] = fs->GetFileModTime(assetPath);*/
  //  meta["type"] = "mesh";
  //  meta["vertexCount"] = vertices.size();
  //  meta["indexCount"] = indices.size();
  //  meta["textureCount"] = textures.size();

  //  fs->SaveJSON(metaPath.c_str(), meta);
  //  LOG("Mesh meta saved: %s", metaPath.c_str());
}


void ResourceMesh::LoadMeta() {
    //std::string assetPath = GetAssetFilePath();
    //if (assetPath.empty()) {
    //    LOG("ERROR: Asset path not set for mesh");
    //    return;
    //}

    //std::string metaPath = assetPath + ".meta";

    //if (!fs->Exists(metaPath.c_str())) {
    //    LOG("WARNING: Meta file does not exist: %s", metaPath.c_str());
    //    return;
    //}

    //nlohmann::json meta = fs->LoadJSON(metaPath.c_str());

    //// Load UUID
    //if (meta.contains("uuid")) SetUUID(meta["uuid"]);
    //

    //// Load type
    //if (meta.contains("type")) {
    //    std::string type = meta["type"];
    //    if (type != "mesh") {
    //        LOG("WARNING: Meta file type mismatch. Expected 'mesh', got '%s'", type.c_str());
    //    }
    //}

    //
    //uint savedVertexCount = meta.contains("vertexCount") ? meta["vertexCount"].get<uint>() : 0;
    //uint savedIndexCount = meta.contains("indexCount") ? meta["indexCount"].get<uint>() : 0;
    //uint savedTextureCount = meta.contains("textureCount") ? meta["textureCount"].get<uint>() : 0;

    //LOG("Mesh meta loaded: %s (UUID: %llu, vertices: %d, indices: %d, textures: %d)",
    //    metaPath.c_str(), GetUUID(), savedVertexCount, savedIndexCount, savedTextureCount);
}



