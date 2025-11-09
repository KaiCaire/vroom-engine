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
#include "GUIManager.h"

using namespace std;


void Model::loadModel(string path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }

    

    fullPath = path;
    std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
    LOG("FullPath = %s", fullPath);
    fileExtension = fullPath.substr(fullPath.find_last_of(".") + 1);
    directory = fullPath.substr(0, fullPath.find_last_of('/'));
    fileName = fullPath.substr(fullPath.find_last_of('/') + 1, fullPath.find_last_of('.') - (fullPath.find_last_of('/') + 1));



    stbi_set_flip_vertically_on_load(fileExtension == "obj");

    rootGameObject = make_shared<GameObject>(fileName);
    Application::GetInstance().guiManager.get()->sceneObjects.push_back(rootGameObject);
    rootGameObject.get()->SetOwnerModel(this);

    rootGameObject->AddComponent(ComponentType::TRANSFORM);
    /*processNodeWithGameObjects(scene->mRootNode, scene, rootGameObject);*/
    for (int i = 0; i < scene->mRootNode->mNumChildren; i++) {
        processNodeWithGameObjects(scene->mRootNode->mChildren[i], scene, rootGameObject);
    }

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

Model::Model(Mesh mesh) {
    auto gameObject = make_shared<GameObject>();
    gameObjects.push_back(gameObject);
    rootGameObject = gameObject;
    
    

    /*LOG("Created Cube: '%s' (Parent: '%s')", gameObject->GetName().c_str(), parent ? parent->GetName().c_str() : "NULL");*/

    // Transform component
    auto transformComp = gameObject->AddComponent(ComponentType::TRANSFORM);
    auto transform = static_cast<TransformComponent*>(transformComp.get());

    glm::vec3 position = glm::vec3(0.0f);
    glm::quat rotation = glm::quat(0, 0, 0, 1);
    glm::vec3 scaling = glm::vec3(1.0f);

    transform->SetPosition(position);

    //IMPORTANT! quats in glm are defined as glm::quat(w,x,y,z)
    transform->SetRotation(rotation);
    transform->SetScale(scaling);

    LOG("  - Transform: Pos(%.2f, %.2f, %.2f) Scale(%.2f, %.2f, %.2f)",
        position.x, position.y, position.z,
        scaling.x, scaling.y, scaling.z);


    // Create & store mesh 
    auto sharedMesh = make_shared<Mesh>(mesh);
    meshes.push_back(sharedMesh);

    // Add RenderMeshComponent and set the mesh
    auto meshComp = gameObject->AddComponent(ComponentType::MESH_RENDERER);
    auto modelMesh = static_cast<RenderMeshComponent*>(meshComp.get());
    modelMesh->SetMesh(sharedMesh); 

    LOG("  - Added RenderMeshComponent with mesh");

    auto materialComp = gameObject->AddComponent(ComponentType::MATERIAL);
    auto modelMat = static_cast<MaterialComponent*>(materialComp.get());

    //load and assign default material texture
    string checkersTexDir = Application::GetInstance().textures.get()->defaultTexDir;
    string checkersTexName = checkersTexDir.substr(checkersTexDir.find_last_of('/') + 1);
    

    Texture defaultColorTex = GetOrLoadTexture(checkersTexDir, checkersTexName, "texture_diffuse");
    modelMesh->GetMesh().get()->textures.push_back(defaultColorTex);

    modelMat->SetDiffuseMap(std::make_shared<Texture>(defaultColorTex));

    LOG("  - Added Material component with default texture");
     
}

Model::Model() {
    //create root
    rootGameObject = std::make_shared<GameObject>("EmptyObject");
    gameObjects.push_back(rootGameObject);
    rootGameObject->AddComponent(ComponentType::TRANSFORM);

    LOG("Empty Object created successfully");
}

void Model::Draw(Shader& shader) {
    for (auto& gameObject : gameObjects) {
        //check if object is active and is not to be destroyed
        if (!gameObject || gameObject->IsMarkedForDestroy() || !gameObject->IsActive())
            continue;

        //check for mesh renderer
        auto rendererComp = gameObject->GetComponent(ComponentType::MESH_RENDERER);
        if (!rendererComp)
            continue;

        auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);
        if (!renderer || !renderer->GetMesh())
            continue;

        auto mesh = renderer->GetMesh();
        if (!mesh) continue;

        //trigger checkerboard texture
        if (useDefaultTexture) {
            //store original texture if not yet stored
            if (originalTextures.find(mesh) == originalTextures.end()) {
                originalTextures[mesh] = mesh->textures;
            }

            mesh->textures.clear();

            std::string checkersTexDir = Application::GetInstance().textures->defaultTexDir;
            std::string checkersTexName = checkersTexDir.substr(checkersTexDir.find_last_of('/') + 1);
            Texture checkersTex = GetOrLoadTexture(checkersTexDir, checkersTexName, "texture_diffuse");

            mesh->textures.push_back(checkersTex);
        }
        else {
            //restore original texture
            auto ogTex = originalTextures.find(mesh);
            if (ogTex != originalTextures.end()) {
                mesh->textures = ogTex->second;
                //originalTextures.erase(ogTex);
            }

        }

        //draw the mesh
        renderer->Render(&shader);
    }
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
        aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];

        if (node->mNumMeshes > 1) {
            string meshName = string(node->mName.C_Str()) + "_Mesh" + to_string(i);
            auto meshGO = make_shared<GameObject>(meshName);
            gameObjects.push_back(meshGO);

            meshGO->AddComponent(ComponentType::TRANSFORM);
            meshGO->SetParent(gameObject);

            createComponentsForMesh(meshGO, aimesh, scene);

        }
        else {
            createComponentsForMesh(gameObject, aimesh, scene);
        }
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
    else {
        AssignDefaultTexture(textures);
        /*string checkersTexDir = Application::GetInstance().textures.get()->defaultTexDir;
        string checkersTexName = checkersTexDir.substr(0, checkersTexDir.find_last_of('/') + 1);
        Texture defaultTex = GetOrLoadTexture(checkersTexDir, checkersTexName, "texture_diffuse");
        textures.push_back(defaultTex);*/
        
        
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
        
        textures.push_back(GetOrLoadTexture(fullPath, str.C_Str(), typeName));
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
    if(textures.empty()) {
        AssignDefaultTexture(textures);
        std::vector<Texture> diffuseMaps;
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    }

    // --- Create Mesh ---
    auto mesh = std::make_shared<Mesh>(vertices, indices, textures);
    meshes.push_back(mesh); // store shared_ptr

    // --- Add RenderMeshComponent Component ---
    auto rendererComp = gameObject->AddComponent(ComponentType::MESH_RENDERER);
    auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);
    if (renderer)
    {
        renderer->SetMesh(mesh); // pass shared_ptr<Mesh>
        LOG("  - Added RenderMeshComponent component to '%s'", gameObject->GetName().c_str());
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
            matComponent->SetDiffuseColor(glm::vec4(color.r, color.g, color.b, color.a));
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
    if (aiMesh->mMaterialIndex <= 0) {
        AssignDefaultTexture(textures);
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


void Model::DestroyGameObject(std::shared_ptr<GameObject> gameObject) {
    if (!gameObject) {
        LOG("WARNING: Attempted to destroy null GameObject");
        return;
    }

    LOG("Destroying GameObject '%s'", gameObject->GetName().c_str());

    // Marcar este GameObject
    gameObject->MarkForDestroy();

    // Lambda recursiva sin std::function
    auto markChildren = [&](auto&& self, std::shared_ptr<GameObject> go) -> void {
        for (auto& child : go->GetChildren()) {
            if (child && !child->IsMarkedForDestroy()) {
                LOG("  - Marking child '%s' for destruction", child->GetName().c_str());
                child->MarkForDestroy();
                self(self, child);  // recursi�n
            }
        }
        };

    // Llamar con la funci�n y el objeto ra�z
    markChildren(markChildren, gameObject);

    // Desconectar del padre
    if (auto parent = gameObject->GetParent()) {
        parent->RemoveChild(gameObject);
        LOG("  - Disconnected from parent '%s'", parent->GetName().c_str());
    }
}

void Model::CleanUpDestroyedObjects() {
    size_t beforeCount = gameObjects.size();

    // Eliminar GameObjects marcados
    gameObjects.erase(
        std::remove_if(gameObjects.begin(), gameObjects.end(),
            [](const std::shared_ptr<GameObject>& go) {
                return go && go->IsMarkedForDestroy();
            }),
        gameObjects.end()
    );

    size_t afterCount = gameObjects.size();
    if (beforeCount != afterCount) {
        LOG("Cleanup: Removed %d GameObject(s). Remaining: %d",
            (int)(beforeCount - afterCount), (int)afterCount);
    }
}

std::shared_ptr<GameObject> Model::CreateEmptyGameObject(const std::string& name, std::shared_ptr<GameObject> parent) {
    LOG("Creating empty GameObject: '%s'", name.c_str());

    // Crear GameObject vac�o
    auto newGameObject = std::make_shared<GameObject>(name);

    // A�adir Transform (todos los GameObjects necesitan Transform)
    newGameObject->AddComponent(ComponentType::TRANSFORM);

    // Establecer parent
    if (parent) {
        newGameObject->SetParent(parent);
        LOG("  - Parent set to '%s'", parent->GetName().c_str());
    }
    else if (rootGameObject) {
        // Si no se especifica parent, usar el root
        newGameObject->SetParent(rootGameObject);
        LOG("  - Parent set to root");
    }

    // A�adir a la lista
    gameObjects.push_back(newGameObject);

    LOG("Empty GameObject '%s' created successfully (Total GameObjects: %d)",
        name.c_str(), (int)gameObjects.size());

    return newGameObject;
}

Texture Model::GetOrLoadTexture(const string& fullPath, const string& fileName, const string& typeName) {

    auto& textures_loaded = Application::GetInstance().textures.get()->textures_loaded;
    // Check if already loaded
    for (auto& loadedTex : textures_loaded) {
        if (loadedTex.path == fullPath) {
            return loadedTex; // Return the cached texture
        }
    }

    // Not found, load new texture
    Texture texture;
    texture.TextureFromFile(fullPath, fileName.c_str());
    texture.mapType = typeName;
    texture.path = fullPath;
    textures_loaded.push_back(texture);

    return texture;
}

void Model::AssignDefaultTexture(std::vector<Texture>& textures) {
    string fullPath = Application::GetInstance().textures.get()->defaultTexDir;
    string fileName = fullPath.substr(fullPath.find_last_of('/') + 1);
    string directory = fullPath.substr(0, fullPath.find_last_of('/') + 1);

    LOG("AssignDefaultTexture: fullPath=%s, fileName=%s", fullPath.c_str(), fileName.c_str());

    Texture defaultTex = GetOrLoadTexture(fullPath, fileName, "texture_diffuse");

    if (defaultTex.id != 0) {
        textures.push_back(defaultTex);
        LOG("  -> Default texture assigned (ID: %d)", defaultTex.id);
    }
    else {
        LOG("  -> ERROR: Failed to assign default texture!");
    }
}

