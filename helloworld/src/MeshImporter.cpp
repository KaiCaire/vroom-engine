#include "Application.h"
#include "Log.h"
#include "ResourceTexture.h"
#include <glm/glm.hpp>
#include "MeshImporter.h"
#include "TextureImporter.h"
#include "ResourceMesh.h"
#include "ResourceManager.h"



std::shared_ptr<ResourceMesh> MeshImporter::Import(aiMesh* aiMesh, const aiScene* scene, const std::string& modelPath, VroomUUID cachedUUID) {
    if (!aiMesh) {
        LOG("ERROR: MeshImporter received null aiMesh");
        return nullptr;
    }

    LOG("MeshImporter: Processing mesh '%s'", aiMesh->mName.C_Str());
    FileSystem* fs = Application::GetInstance().fileSystem.get();

    
    VroomUUID meshUUID = cachedUUID;

    // If we have a cached UUID, try to load from Library
    //if (meshUUID != 0) {
    //    std::string libraryPath = std::string(Paths::MESH_LIB_DIR) + std::to_string(meshUUID) + ".vroommesh";

    //    if (fs->Exists(libraryPath.c_str())) {
    //        LOG("Loading mesh from cache: %s", libraryPath.c_str());

    //        auto mesh = std::make_shared<ResourceMesh>();
    //        mesh->SetUUID(meshUUID);
    //        mesh->SetName(aiMesh->mName.C_Str());

    //        LOG("Mesh loaded from cache successfully");
    //        return mesh;
    //    }
    //    else {
    //        LOG("Cache file missing, will reimport");
    //    }
    //}

    // No cached UUID or cache miss - import fresh
    meshUUID = (meshUUID == 0) ? UUIDGen::GenerateUUID() : meshUUID;
    LOG("Importing mesh (UUID: %llu)", meshUUID);

    // Process mesh data
    auto vertices = ProcessVertices(aiMesh);
    auto indices = ProcessIndices(aiMesh);
    auto textures = ProcessTextures(aiMesh, scene, modelPath);

    // Create mesh resource
    auto mesh = std::make_shared<ResourceMesh>(vertices, indices, textures);
    mesh->SetUUID(meshUUID);
    mesh->SetName(aiMesh->mName.C_Str());

    //bounding box calculation
    if (!vertices.empty()) {
        glm::vec3 min = vertices[0].Position;
        glm::vec3 max = vertices[0].Position;

        //iterate over all vertices to find the local space extremes
        for (const auto& vertex : vertices) {
            min = glm::min(min, vertex.Position);
            max = glm::max(max, vertex.Position);
        }

        mesh->minAABB = min;
        mesh->maxAABB = max;
        LOG("Calculated Local AABB for %s: Min(%.2f, %.2f, %.2f) Max(%.2f, %.2f, %.2f)",
            aiMesh->mName.C_Str(), min.x, min.y, min.z, max.x, max.y, max.z);
    }

    // Save to Library
    std::string libraryPath = Paths::MESH_LIB_DIR + std::to_string(meshUUID) + ".vroommesh";
    mesh->SetLibraryFilePath(libraryPath);
    mesh->SaveBin();

    Application::GetInstance().resourceManager->RegisterResource(mesh);
    

    LOG("MeshImporter: Successfully imported '%s' (UUID: %llu)", mesh->GetName().c_str(), meshUUID);

    return mesh;
}

std::vector<Vertex> MeshImporter::ProcessVertices(aiMesh* aiMesh) {
    std::vector<Vertex> vertices;
    vertices.reserve(aiMesh->mNumVertices);

    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
        Vertex vertex;

        // Position
        vertex.Position = glm::vec3(
            aiMesh->mVertices[i].x,
            aiMesh->mVertices[i].y,
            aiMesh->mVertices[i].z
        );

        // Normals
        if (aiMesh->HasNormals()) {
            vertex.Normal = glm::vec3(
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            );
        }

        // Texture coordinates
        if (aiMesh->mTextureCoords[0]) {
            vertex.texCoord = glm::vec2(
                aiMesh->mTextureCoords[0][i].x,
                aiMesh->mTextureCoords[0][i].y
            );
        }
        else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    return vertices;
}

std::vector<unsigned int> MeshImporter::ProcessIndices(aiMesh* aiMesh) {
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
        aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return indices;
}

std::vector<std::shared_ptr<ResourceTexture>> MeshImporter::ProcessTextures(aiMesh* aiMesh, const aiScene* scene, const std::string& modelPath) {
    std::vector<std::shared_ptr<ResourceTexture>> textures;
    FileSystem* fs = Application::GetInstance().fileSystem.get();

    std::vector<std::shared_ptr<ResourceTexture>> textures_loaded = Application::GetInstance().importer.get()->textures_loaded;

    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];

        auto loadTextures = [&](aiTextureType type, const std::string& typeName) {
            for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
                aiString str;
                material->GetTexture(type, i, &str);


                std::string modelDir = fs->GetDirFromPath(modelPath.c_str());
                std::string texturePath = modelDir + "/" + std::string(str.C_Str());
                texturePath = fs->NormalizePath(texturePath.c_str());

                
                if (!fs->Exists(texturePath.c_str())) {
                    LOG("Texture not found at: %s", texturePath.c_str());

                    
                    // Try same directory as the model
                    std::string fileName = fs->GetFileFromPath(str.C_Str());
                    std::string altPath1 = modelDir + "/" + fileName;

                    // Try textures folder subdirectory
                    std::string altPath2 = modelDir + "/textures/" + fileName;

                    if (fs->Exists(altPath1.c_str())) {
                        texturePath = altPath1;
                        LOG("Found texture in model directory: %s", texturePath.c_str());
                    }
                    else if (fs->Exists(altPath2.c_str())) {
                        texturePath = altPath2;
                        LOG("Found texture in textures subdirectory: %s", texturePath.c_str());
                    }
                    else {
                        LOG("WARNING: Texture not found in any location. Using default.");
                        continue; // use default checkers texture
                    }
                }
                // Check if texture is already loaded
                
                bool found = false;
                for (auto& loadedTex : textures_loaded) {
                    if (loadedTex.get()->path == texturePath) {
                        textures.push_back(loadedTex);
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    std::shared_ptr<ResourceTexture> tex = Application::GetInstance().importer.get()->textureImporter->Import(texturePath);
                    
                    if (tex == nullptr) {
                        LOG("WARNING: Texture import failed for path '%s'. Skipping texture.", texturePath.c_str());
                        continue; // Skip the rest of the loop iteration
                    }
                    
                    tex.get()->mapType = typeName;
                    tex.get()->path = texturePath;
                    textures_loaded.push_back(tex);
                    textures.push_back(tex);
                }
            }
            };

        // Load different texture types
        loadTextures(aiTextureType_DIFFUSE, "texture_diffuse");
        loadTextures(aiTextureType_SPECULAR, "texture_specular");
        loadTextures(aiTextureType_NORMALS, "texture_normal");
        loadTextures(aiTextureType_METALNESS, "texture_metallic");
        loadTextures(aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
        loadTextures(aiTextureType_AMBIENT_OCCLUSION, "texture_ao");
    }

    // Assign default texture if none found
    if (textures.empty()) {
        std::string defaultTexPath = Application::GetInstance().importer->defaultTexDir;

        

        // Check cache first
        auto& textures_loaded = Application::GetInstance().importer->textures_loaded;
        bool found = false;
        for (auto& loadedTex : textures_loaded) {
            if (loadedTex.get()->path == defaultTexPath) {
                textures.push_back(loadedTex);
                found = true;
                break;
            }
        }

        if (!found) {

            /*std::string fullDefaultTexPath = defaultTexPath + fileName;*/
            std::shared_ptr<ResourceTexture> defaultTex = Application::GetInstance().importer.get()->textureImporter->Import(defaultTexPath);
            /*defaultTex.TextureFromFile(defaultTexPath, fileName.c_str());*/

            if (defaultTex == nullptr) {
                LOG("FATAL ERROR: Failed to load default texture! Skipping.");
                // We cannot assign a default texture, so we return with no textures.
                return textures;
            }
            
            defaultTex.get()->mapType = "texture_diffuse";
            defaultTex.get()->path = defaultTexPath;
            textures_loaded.push_back(defaultTex);
            textures.push_back(defaultTex);
        }

        LOG("MeshImporter: Assigned default texture");
    }

    return textures;
}