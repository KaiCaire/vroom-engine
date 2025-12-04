#include "ModelImporter.h"
#include "Application.h"
#include "OpenGL.h"
#include "FileSystem.h"

#include "MeshImporter.h"
#include "TextureImporter.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include "SceneManager.h"


#include <algorithm>
#include "ResourceMesh.h"
#include "assimp/importer.hpp"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "stb_image.h"
#include "ResourceTexture.h"
#include "Log.h"
#include "GUIManager.h"


using namespace std;


std::shared_ptr<GameObject> ModelImporter::ImportScene(const char* path) {

    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOG("ERROR::ASSIMP::%s", import.GetErrorString());
        return nullptr;
    }

    FileSystem* fs = Application::GetInstance().fileSystem.get();
    fullPath = fs->NormalizePath(path);
    fileName = fs->GetFileNameFromPath(path);

    // Try to load model meta for caching
    nlohmann::json* modelMeta = LoadModelMeta(path);

    if (modelMeta) LOG("Using cached model data");
    else LOG("Importing model from scratch");
     
     

    stbi_set_flip_vertically_on_load(fileExtension == "obj");

    modelRootGO = make_shared<GameObject>(std::string(fileName));
    Application::GetInstance().sceneManager.get()->GetActiveScene()->AddGameObject(modelRootGO);
    /*Application::GetInstance().guiManager.get()->sceneObjects.push_back(modelRootGO);*/
    modelRootGO->AddComponent(ComponentType::TRANSFORM);

    // Process scene - pass the meta pointer
    for (int i = 0; i < scene->mRootNode->mNumChildren; i++) {
        processNodeWithGameObjects(scene->mRootNode->mChildren[i], scene, modelRootGO, modelMeta);
    }

    // Save/update model meta
    SaveModelMeta(path);

    LOG("=== MODEL LOADING SUMMARY ===");
    LOG("Total GameObjects created: %d", (int)gameObjects.size());
    LOG("Total Meshes processed: %d", (int)meshes.size());

    return modelRootGO;
}

ModelImporter::ModelImporter(std::shared_ptr<ResourceMesh> sharedMesh) {
    auto gameObject = make_shared<GameObject>();
    gameObjects.push_back(gameObject);
    modelRootGO = gameObject;
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
    //auto sharedMesh = make_shared<Mesh>(mesh);
    meshes.push_back(sharedMesh);

    // Add RenderMeshComponent and set the mesh
    auto meshComp = gameObject->AddComponent(ComponentType::MESH_RENDERER);
    auto modelMesh = static_cast<RenderMeshComponent*>(meshComp.get());
    modelMesh->SetMesh(sharedMesh); 

    LOG("  - Added RenderMeshComponent with mesh");

    auto materialComp = gameObject->AddComponent(ComponentType::MATERIAL);
    auto modelMat = static_cast<MaterialComponent*>(materialComp.get());

    //load and assign default material texture
    string checkersTexDir = Application::GetInstance().importer.get()->defaultTexDir;
    string checkersTexName = checkersTexDir.substr(checkersTexDir.find_last_of('/') + 1);
    

    std::shared_ptr<ResourceTexture> defaultColorTex = GetOrLoadTexture(checkersTexDir, checkersTexName, "texture_diffuse");
    modelMesh->GetMesh().get()->textures.push_back(defaultColorTex);

    //modelMat->SetDiffuseMap(std::make_shared<Texture>(defaultColorTex));
    modelMat->SetDiffuseMap(defaultColorTex);

    LOG("  - Added Material component with default texture");
     
}

ModelImporter::ModelImporter() {
    //create root
    modelRootGO = std::make_shared<GameObject>(std::string("EmptyObject"));
    gameObjects.push_back(modelRootGO);
    modelRootGO->AddComponent(ComponentType::TRANSFORM);

    LOG("Empty Object created successfully");
}

void ModelImporter::Draw(Shader& shader) {
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


        //FIX!
        //trigger checkerboard texture
        if (useDefaultTexture) {
            //store original texture if not yet stored
            if (originalTextures.find(mesh) == originalTextures.end()) {
                originalTextures[mesh] = mesh->textures;
            }

            mesh->textures.clear();

            std::string checkersTexDir = Application::GetInstance().importer->defaultTexDir;
            std::string checkersTexName = checkersTexDir.substr(checkersTexDir.find_last_of('/') + 1);
            std::shared_ptr<ResourceTexture> checkersTex = GetOrLoadTexture(checkersTexDir, checkersTexName, "texture_diffuse");

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
        renderer->GetMesh()->Draw(shader);
    }
}

void ModelImporter::processNodeWithGameObjects(const aiNode* node, const aiScene* scene, std::shared_ptr<GameObject> parent, nlohmann::json* modelMeta)
{

    aiMatrix4x4 accumulatedTransform;
    const aiNode* currentNode = node;

    while (std::string(currentNode->mName.C_Str()).find("_$AssimpFbx$_") != std::string::npos) {
        accumulatedTransform = accumulatedTransform * currentNode->mTransformation;
        if (currentNode->mNumChildren > 1) {
            LOG("WARNING: FBX dummy node has multiple children");
        }
        currentNode = currentNode->mChildren[0];
    }

    auto gameObject = make_shared<GameObject>(std::string(currentNode->mName.C_Str()));
    gameObjects.push_back(gameObject);

    LOG("Created GameObject: '%s' (Parent: '%s')",
        gameObject->GetName().c_str(),
        parent ? parent->GetName().c_str() : "NULL");

    // Transform component
    auto transformComp = gameObject->AddComponent(ComponentType::TRANSFORM);
    auto transform = static_cast<TransformComponent*>(transformComp.get());

    // Combine transforms
    aiMatrix4x4 localTransform = accumulatedTransform * currentNode->mTransformation;

    aiVector3D position, scaling;
    aiQuaternion rotation;
    localTransform.Decompose(scaling, rotation, position);

    // Normalize scale
    float div_scale = std::max(scaling.x, scaling.y);
    div_scale = std::max(div_scale, scaling.z);
    if (div_scale > 0.0f) {
        scaling /= div_scale;
    }

   /* transform->SetPosition(glm::vec3(position.x, position.y, position.z));
    transform->SetRotation(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
    transform->SetScale(glm::vec3(scaling.x, scaling.y, scaling.z));*/

    glm::vec3 pos(position.x, position.y, position.z);
    glm::quat rot(rotation.w, rotation.x, rotation.y, rotation.z);
    glm::vec3 scale(scaling.x, scaling.y, scaling.z);


    if (parent) {
        gameObject->SetParent(parent);
        LOG("  - Set parent to '%s'", parent->GetName().c_str());
    }

    std::string nodeName = currentNode->mName.C_Str();
    uint meshIndex = -1;



    /*LOG("  - Processing %d meshes for '%s'", currentNode->mNumMeshes, gameObject->GetName().c_str());*/
    if (currentNode->mNumMeshes > 1) {
        for (unsigned int i = 0; i < currentNode->mNumMeshes; i++) {

            meshIndex = currentNode->mMeshes[i];
            aiMesh* aimesh = scene->mMeshes[meshIndex];

            
            auto meshGO = make_shared<GameObject>(std::string(nodeName)); //casting just in case
            gameObjects.push_back(meshGO);

            meshGO->AddComponent(ComponentType::TRANSFORM);
            meshGO->SetParent(gameObject);

            createComponentsForMesh(meshGO, aimesh, scene, modelMeta);
        }
       

    }
    else if (currentNode->mNumMeshes == 1) {
        meshIndex = currentNode->mMeshes[0]; // ← Global index
        aiMesh* aiMesh = scene->mMeshes[meshIndex];


        createComponentsForMesh(gameObject, aiMesh, scene, modelMeta);
    }
    //else { //no meshes-> create empty GO
    //    auto emptyGO = make_shared<GameObject>(nodeName);
    //    emptyGO->AddComponent(ComponentType::TRANSFORM);
    //    emptyGO->SetParent(gameObject);
    //    gameObjects.push_back(emptyGO);
    //}



    LOG("  - Processing %d children for '%s'", currentNode->mNumChildren, gameObject->GetName().c_str());

    for (unsigned int i = 0; i < currentNode->mNumChildren; i++)
        processNodeWithGameObjects(currentNode->mChildren[i], scene, gameObject, modelMeta);
}


void ModelImporter::createComponentsForMesh(std::shared_ptr<GameObject> gameObject, aiMesh* aiMesh, const aiScene* scene, nlohmann::json* modelMeta)
{
    LOG("=== createComponentsForMesh START ===");

    std::shared_ptr<ResourceMesh> mesh;
    VroomUUID meshUUID = 0;

    // Check if we have cached UUID from model meta
    if (modelMeta && modelMeta->contains("meshes")) {
        for (auto& meshEntry : (*modelMeta)["meshes"]) {
            if (meshEntry["name"] == aiMesh->mName.C_Str()) {
                meshUUID = meshEntry["uuid"];
                LOG("Found cached mesh UUID: %llu", meshUUID);
                break;
            }
        }
    }

    // Let MeshImporter handle loading/importing
    // Pass the cached UUID if we have one
    mesh = Application::GetInstance().importer->meshImporter->Import(aiMesh, scene, fullPath, meshUUID);

    if (!mesh) {
        LOG("ERROR: Failed to import mesh");
        return;
    }

    // Store mesh info for model meta
    meshMetaInfo.push_back({aiMesh->mName.C_Str(), mesh->GetUUID(), mesh->vertices.size(), mesh->indices.size() });

    // Store the mesh in the model
    meshes.push_back(mesh);
    LOG("Mesh stored in model (total: %d)", (int)meshes.size());

    // --- Add RenderMeshComponent ---
    auto rendererComp = gameObject->AddComponent(ComponentType::MESH_RENDERER);
    if (!rendererComp) {
        LOG("ERROR: Failed to add MESH_RENDERER component!");
        return;
    }

    auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);
    if (!renderer) {
        LOG("ERROR: Failed to cast to RenderMeshComponent!");
        return;
    }

    // Set the mesh directly
    renderer->SetMesh(mesh);

    // Verify
    auto verifyMesh = renderer->GetMesh();
    if (!verifyMesh) {
        LOG("ERROR: After SetMesh, GetMesh returns nullptr!");
    }
    else {
        LOG("SUCCESS: Mesh set in renderer (vertices=%d)", verifyMesh->vertices.size());
    }

    // --- Add Material Component ---
    auto materialComp = gameObject->AddComponent(ComponentType::MATERIAL);
    auto matComponent = std::dynamic_pointer_cast<MaterialComponent>(materialComp);

    if (matComponent && aiMesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[aiMesh->mMaterialIndex];

        aiColor4D color;
        if (AI_SUCCESS == aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &color)) {
            matComponent->SetDiffuseColor(glm::vec4(color.r, color.g, color.b, color.a));
        }

        float shininess;
        if (AI_SUCCESS == aiGetMaterialFloat(aiMat, AI_MATKEY_SHININESS, &shininess)) {
            matComponent->SetShininess(shininess);
        }
    }

    LOG("=== createComponentsForMesh END ===\n");
}


ModelImporter::~ModelImporter() {
    // shared_ptr automatically cleans up
}



std::shared_ptr<ResourceTexture> ModelImporter::GetOrLoadTexture(const std::string& fullPath, const std::string& fileName, const std::string& typeName) {

    auto& textures_loaded = Application::GetInstance().importer.get()->textures_loaded;
    // Check if already loaded
    for (auto& loadedTex : textures_loaded) {
        if (loadedTex.get()->path == fullPath) {
            return loadedTex; // Return the cached texture
        }
    }

    // Not found, load new texture
    std::shared_ptr<ResourceTexture> texture = Application::GetInstance().importer.get()->textureImporter->Import(fullPath);
    /*texture.TextureFromFile(fullPath, fileName.c_str());*/

    if (texture == nullptr) {
        LOG("WARNING: GetOrLoadTexture failed to load texture from path: %s. Returning default texture or nullptr.", fullPath.c_str());
        return nullptr;
    }
    
    texture.get()->mapType = typeName;
    texture.get()->path = fullPath;
    textures_loaded.push_back(texture);

    return texture;
}

void ModelImporter::AssignDefaultTexture(std::vector<std::shared_ptr<ResourceTexture>>& textures) {
    string fullPath = Application::GetInstance().importer.get()->defaultTexDir;
    string fileName = fullPath.substr(fullPath.find_last_of('/') + 1);
    string directory = fullPath.substr(0, fullPath.find_last_of('/') + 1);

    LOG("AssignDefaultTexture: fullPath=%s, fileName=%s", fullPath.c_str(), fileName.c_str());

    std::shared_ptr<ResourceTexture> defaultTex = GetOrLoadTexture(fullPath, fileName, "texture_diffuse");

    if (defaultTex.get()->GetUUID() != 0) {
        textures.push_back(defaultTex);
        LOG("  -> Default texture assigned (ID: %d)", defaultTex.get()->GetUUID());
    }
    else {
        LOG("  -> ERROR: Failed to assign default texture!");
    }
}

void ModelImporter::SaveModelMeta(const char* modelPath) {
    FileSystem* fs = Application::GetInstance().fileSystem.get();

    nlohmann::json meta;

    // Check if meta already exists to preserve the model UUID
    std::string metaPath = std::string(modelPath) + ".meta";
    VroomUUID modelUUID = 0;

    if (fs->Exists(metaPath.c_str())) {
        nlohmann::json existingMeta = fs->LoadJSON(metaPath.c_str());
        if (existingMeta.contains("uuid")) {
            modelUUID = existingMeta["uuid"];
        }
    }

    if (modelUUID == 0) {
        modelUUID = UUIDGen::GenerateUUID();
    }

    meta["uuid"] = modelUUID;
    meta["modTime"] = fs->GetFileModTime(modelPath);
    meta["type"] = "model";

    // Add all mesh info
    nlohmann::json meshesArray = nlohmann::json::array();
    for (const auto& meshInfo : meshMetaInfo) {
        nlohmann::json meshEntry;
        meshEntry["name"] = meshInfo.name;
        meshEntry["uuid"] = meshInfo.uuid;
        meshEntry["vertexCount"] = meshInfo.vertexCount;
        meshEntry["indexCount"] = meshInfo.indexCount;
        meshesArray.push_back(meshEntry);
    }
    meta["meshes"] = meshesArray;

    fs->SaveJSON(metaPath.c_str(), meta);
    LOG("Model meta saved: %s (%d meshes)", metaPath.c_str(), (int)meshMetaInfo.size());

    // Clear for next import
    meshMetaInfo.clear();
}


nlohmann::json* ModelImporter::LoadModelMeta(const char* modelPath) {
    FileSystem* fs = Application::GetInstance().fileSystem.get();
    std::string metaPath = std::string(modelPath) + ".meta";

    if (!fs->Exists(metaPath.c_str())) {
        LOG("No meta file found for model: %s", modelPath);
        return nullptr;
    }

    if (!fs->IsMetaValid(metaPath.c_str())) {
        LOG("Meta file invalid for model: %s", modelPath);
        return nullptr;
    }

    // Check if model needs reimport
    if (fs->NeedsReimport(metaPath.c_str(), modelPath)) {
        LOG("Model has been modified, needs reimport: %s", modelPath);
        return nullptr;
    }

    // Load and return the meta
    static nlohmann::json meta;  // Static so pointer remains valid
    meta = fs->LoadJSON(metaPath.c_str());

    LOG("Loaded model meta: %s", metaPath.c_str());
    return &meta;
}