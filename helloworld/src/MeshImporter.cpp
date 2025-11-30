#include "Application.h"
#include "Log.h"
#include "ResourceTexture.h"
#include <glm/glm.hpp>
#include "MeshImporter.h"
#include "TextureImporter.h"
#include "ResourceMesh.h"


std::shared_ptr<ResourceMesh> MeshImporter::Import(aiMesh* aiMesh, const aiScene* scene,
    const std::string& modelPath) {
    if (!aiMesh) {
        LOG("ERROR: MeshImporter received null aiMesh");
        return nullptr;
    }

    LOG("MeshImporter: Processing mesh '%s' (%d vertices, %d faces)",
        aiMesh->mName.C_Str(), aiMesh->mNumVertices, aiMesh->mNumFaces);

    // Process mesh data
    auto vertices = ProcessVertices(aiMesh);
    auto indices = ProcessIndices(aiMesh);
    auto textures = ProcessTextures(aiMesh, scene, modelPath);

    // Create the Mesh object (Mesh is now a Resource)
    std::shared_ptr<ResourceMesh> mesh = std::make_shared<ResourceMesh>(vertices, indices, textures);

    // Set resource metadata
    mesh->SetName(aiMesh->mName.C_Str());
    mesh->SetAssetFilePath(modelPath);

    // TODO: Generate or retrieve UUID from meta file
    // mesh->SetUID(generatedUID);

    LOG("MeshImporter: Successfully created Mesh with %d vertices, %d indices",
        vertices.size(), indices.size());

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

    std::vector<std::shared_ptr<ResourceTexture>> textures_loaded = Application::GetInstance().importer.get()->textures_loaded;

    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];

        // Helper lambda to load textures by type
        auto loadTextures = [&](aiTextureType type, const std::string& typeName) {
            for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
                aiString str;
                material->GetTexture(type, i, &str);

                // Extract directory from full model path
                std::string directory = modelPath.substr(0, modelPath.find_last_of('/') + 1);
                std::string texturePath = directory + std::string(str.C_Str());

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
                    std::shared_ptr<ResourceTexture> tex = Application::GetInstance().importer.get()->textureImporter->Import(modelPath);
                    
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
        std::string fileName = defaultTexPath.substr(defaultTexPath.find_last_of('/') + 1);

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

            std::string fullDefaultTexPath = defaultTexPath + fileName;
            std::shared_ptr<ResourceTexture> defaultTex = Application::GetInstance().importer.get()->textureImporter->Import(fullDefaultTexPath);
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