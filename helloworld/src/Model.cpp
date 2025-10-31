#include "Model.h"
#include "Application.h"
#include "OpenGL.h"
#include "FileSystem.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include <string>
#include <vector>
#include <algorithm>
#include "Mesh.h"
#include "assimp/importer.hpp"
#include "stb_image.h"
#include "Textures.h"
#include "Log.h"

using namespace std;

// Static list of loaded textures
vector<Texture> textures_loaded;

void Model::loadModel(string path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }

    fullPath = path;
    string fileExtension = fullPath.substr(fullPath.find_last_of(".") + 1);

    stbi_set_flip_vertically_on_load(fileExtension == "obj");

    directory = fullPath.substr(0, fullPath.find_last_of('/'));

    rootGameObject = make_shared<GameObject>("RootNode");
    processNodeWithGameObjects(scene->mRootNode, scene, nullptr);

    LOG("Finished Loading Model");
    LOG("=== MODEL LOADING SUMMARY ===");
    LOG("Total GameObjects created: %d", (int)gameObjects.size());
    LOG("Total Meshes processed: %d", (int)meshes.size());
    LOG("Root GameObject: '%s'", rootGameObject ? rootGameObject->GetName().c_str() : "NULL");

    if (rootGameObject) {
        LOG("=== HIERARCHY ===");
        // LogGameObjectHierarchy(rootGameObject, 0);
    }
}

void Model::Draw(Shader& shader) {
    for (auto& mesh : meshes)
        mesh->Draw(shader);
}

void Model::processNodeWithGameObjects(aiNode* node, const aiScene* scene, shared_ptr<GameObject> parent) {
    auto gameObject = make_shared<GameObject>(node->mName.C_Str());
    gameObjects.push_back(gameObject);

    LOG("Created GameObject: '%s' (Parent: '%s')",
        gameObject->GetName().c_str(),
        parent ? parent->GetName().c_str() : "NULL");

    // Transform component
    auto transformComp = gameObject->AddComponent(ComponentType::TRANSFORM);
    auto transform = static_cast<TransformComponent*>(transformComp.get());

    aiVector3D position, scaling;
    aiQuaternion rotation;
    node->mTransformation.Decompose(scaling, rotation, position);

    transform->SetPosition(glm::vec3(position.x, position.y, position.z));
    transform->SetRotation(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
    transform->SetScale(glm::vec3(scaling.x, scaling.y, scaling.z));

    LOG("  - Transform: Pos(%.2f, %.2f, %.2f) Scale(%.2f, %.2f, %.2f)",
        position.x, position.y, position.z,
        scaling.x, scaling.y, scaling.z);

    if (parent) {
        gameObject->SetParent(parent);
        LOG("  - Set parent to '%s'", parent->GetName().c_str());
    }

    LOG("  - Processing %d meshes for '%s'", node->mNumMeshes, gameObject->GetName().c_str());
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(make_shared<Mesh>(processMesh(mesh, scene)));
    }

    LOG("  - Processing %d children for '%s'", node->mNumChildren, gameObject->GetName().c_str());
    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNodeWithGameObjects(node->mChildren[i], scene, gameObject);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->HasNormals())
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0])
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        else
            vertex.texCoord = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    processedMeshes++;
    LOG("Processed mesh %d", processedMeshes);
    return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
    vector<Texture> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (auto& loadedTex : textures_loaded) {
            if (strcmp(loadedTex.path.c_str(), fullPath.c_str()) == 0) {
                textures.push_back(loadedTex);
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture;
            texture.TextureFromFile(fullPath, str.C_Str());
            texture.mapType = typeName;
            texture.path = fullPath;
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }

    return textures;
}

void Model::createComponentsForMesh(std::shared_ptr<GameObject> gameObject, aiMesh* aiMesh, const aiScene* scene)
{
    LOG("Creating components for mesh in GameObject '%s'", gameObject->GetName().c_str());
    LOG("  - Vertices: %d, Faces: %d", aiMesh->mNumVertices, aiMesh->mNumFaces);

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // --- Process vertices ---
    for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
    {
        Vertex vertex;

        vertex.Position = glm::vec3(
            aiMesh->mVertices[i].x,
            aiMesh->mVertices[i].y,
            aiMesh->mVertices[i].z
        );

        if (aiMesh->HasNormals())
        {
            vertex.Normal = glm::vec3(
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            );
        }

        if (aiMesh->mTextureCoords[0])
        {
            vertex.texCoord = glm::vec2(
                aiMesh->mTextureCoords[0][i].x,
                aiMesh->mTextureCoords[0][i].y
            );
        }
        else
        {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // --- Process indices ---
    for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
    {
        aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // --- Load material textures ---
    if (aiMesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];

        auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        auto roughnessMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

        auto metallicMaps = loadMaterialTextures(material, aiTextureType_METALNESS, "texture_metallic");
        textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

        auto normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        auto aoMaps = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "texture_ao");
        textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
    }

    // --- Create Mesh ---
    auto mesh = std::make_shared<Mesh>(vertices, indices, textures);
    meshes.push_back(mesh); // store shared_ptr

    // --- Add MeshRenderer Component ---
    auto rendererComp = gameObject->AddComponent(ComponentType::MESH_RENDERER);
    auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);
    if (renderer)
    {
        renderer->SetMesh(mesh); // pass shared_ptr<Mesh>
        LOG("  - Added MeshRenderer component to '%s'", gameObject->GetName().c_str());
    }

    // --- Add Material Component ---
    auto materialComp = gameObject->AddComponent(ComponentType::MATERIAL);
    auto matComponent = std::dynamic_pointer_cast<MaterialComponent>(materialComp);
    if (matComponent && aiMesh->mMaterialIndex >= 0)
    {
        aiMaterial* aiMat = scene->mMaterials[aiMesh->mMaterialIndex];

        aiColor4D color;
        if (AI_SUCCESS == aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &color))
        {
            matComponent->SetColor(glm::vec4(color.r, color.g, color.b, color.a));
            LOG("    - Material color: (%.2f, %.2f, %.2f, %.2f)", color.r, color.g, color.b, color.a);
        }

        float shininess;
        if (AI_SUCCESS == aiGetMaterialFloat(aiMat, AI_MATKEY_SHININESS, &shininess))
        {
            matComponent->SetShininess(shininess);
            LOG("    - Material shininess: %.2f", shininess);
        }

        LOG("  - Added Material component to '%s'", gameObject->GetName().c_str());
    }
}

Model::~Model() {
    // shared_ptr automatically cleans up
}

void Model::LogGameObjectHierarchy(shared_ptr<GameObject> go, int depth) {
    if (!go) return;

    string indent(depth * 2, ' ');
    LOG("%s- '%s' (Active: %s, Components: %d, Children: %d)",
        indent.c_str(),
        go->GetName().c_str(),
        go->IsActive() ? "Yes" : "No",
        go->GetComponentCount(),
        (int)go->GetChildren().size());

    for (auto& child : go->GetChildren())
        LogGameObjectHierarchy(child, depth + 1);
}