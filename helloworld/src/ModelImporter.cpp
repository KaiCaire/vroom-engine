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
std::string ModelImporter::lastExternalSourcePath = "";

std::shared_ptr<GameObject> ModelImporter::ImportScene(const char* path, const std::string& sourcePath, bool addToScene) {

    Assimp::Importer import;
    /*const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);*/
    unsigned int flags = aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_LimitBoneWeights |
        aiProcess_JoinIdenticalVertices |
        aiProcess_FlipUVs;

    FileSystem* fs = Application::GetInstance().fileSystem.get();
    fileExtension = fs->GetExtensionFromPath(path);
    
    if (fileExtension == "obj") {
        flags &= ~aiProcess_FlipUVs;
    }
    import.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_WEIGHTS, false); //some models with complex rigs are blowing up on import, so adding this
    const aiScene* scene = import.ReadFile(path, flags);

    
    this->meshes.clear();
    this->meshMetaInfo.clear();

    
    // If it's an external path, remember it forever!
    if (sourcePath.find(":") != std::string::npos) {
        lastExternalSourcePath = fs->GetDirFromPath(sourcePath.c_str());
        /*this->ogSourcePath = sourcePath;*/
    }
    // storing it to use it later in createComponentsForMesh instead of passing it down thru signature
    // (I'm not modifying the method signatures for the millionth time istg)
    
    
    fullPath = fs->NormalizePath(path);
    fileName = fs->GetFileNameFromPath(path);
    LOG("DEBUG: fileName extracted = '%s' from path = '%s'", fileName.c_str(), path);

    modelRootGO = std::make_shared<GameObject>(fileName);  
    //gameObjects.push_back(modelRootGO); 


    nlohmann::json* modelMeta = LoadModelMeta(path);
    
   

    if (modelMeta) {
        LOG("Reimporting model, using cached uuids");
    }
    else {
        LOG("Importing model from scratch");
    }
        
     
     
    fileExtension = fs->GetExtensionFromPath(fullPath.c_str());

    modelRootGO->AddComponent(ComponentType::TRANSFORM);

    // Process scene - pass the meta pointer
    for (int i = 0; i < scene->mRootNode->mNumChildren; i++) {
        processNodeWithGameObjects(scene->mRootNode->mChildren[i], scene, modelRootGO, modelMeta);
    }

    // Save/update model meta
    SaveModelMeta(path);

    //Clearing it after creating everything to avoid stale data
    ogSourcePath = "";
    

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
    string checkersTexDir = Application::GetInstance().resourceManager.get()->checkersTexDir;
   

    //// 2. Use the Resource Manager to request it (this handles the cache and GPU)
    auto res = Application::GetInstance().resourceManager->RequestResource(checkersTexDir);
    auto defaultColorTex = std::dynamic_pointer_cast<ResourceTexture>(res);

    if (defaultColorTex) {
        // 3. Assign to the material
        modelMat->SetDiffuseMap(defaultColorTex);
        /*modelMesh->GetMesh()->textures.push_back(defaultColorTex);*/
    }
    else {
        LOG("ERROR: Default white texture not initialized!");
    }

    LOG("  - Added Material component with default texture");
     
}

ModelImporter::ModelImporter() {

}

void ModelImporter::Draw(Shader& shader) {
    for (auto& gameObject : gameObjects) {
        //check if object is active and is not to be destroyed
        if (!gameObject || gameObject->IsMarkedForDestroy() || !gameObject->IsActive())
            continue;

        //get transform component
        auto transformComp = gameObject->GetComponent(ComponentType::TRANSFORM);
        if (!transformComp)
            continue;

        auto transform = std::dynamic_pointer_cast<TransformComponent>(transformComp);
        if (!transform)
            continue;

        glm::mat4 modelMatrix = transform->GetGlobalTransform();
        shader.setMat4("model", modelMatrix);

        //check for mesh renderer
        auto rendererComp = gameObject->GetComponent(ComponentType::MESH_RENDERER);
        if (!rendererComp)
            continue;

        auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);
        if (!renderer || !renderer->GetMesh())
            continue;

        auto mesh = renderer->GetMesh();
        if (!mesh) continue;

        auto materialComp = gameObject->GetComponent(ComponentType::MATERIAL);
        if (!materialComp)
            continue;

        auto material = std::dynamic_pointer_cast<MaterialComponent>(materialComp);
        if (!material) continue;

      

        renderer->GetMesh()->Draw(shader, material.get());
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
    /*gameObjects.push_back(gameObject);*/

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

    glm::vec3 pos(position.x, position.y, position.z);
    glm::quat rot(rotation.w, rotation.x, rotation.y, rotation.z);
    glm::vec3 scale(scaling.x, scaling.y, scaling.z);

    transform->SetPosition(pos);
    transform->SetRotation(rot);
    transform->SetScale(scale);


    if (parent) {
        gameObject->SetParent(parent);
        LOG("  - Set parent to '%s'", parent->GetName().c_str());
    }

    std::string nodeName = currentNode->mName.C_Str();
    uint meshIndex = -1;



    /*LOG("  - Processing %d meshes for '%s'", currentNode->mNumMeshes, gameObject->GetName().c_str());*/
    if (currentNode->mNumMeshes > 1) {
        for (unsigned int i = 0; i < currentNode->mNumMeshes; i++) {
            meshIndex = currentNode->mMeshes[i]; // The unique global index in the FBX
            aiMesh* aimesh = scene->mMeshes[meshIndex];

            // Create a user-friendly name for the Hierarchy so that we don't have multiple meshes named the same 
            std::string friendlyName = std::string(aimesh->mName.C_Str());
            if (friendlyName.empty() || friendlyName == nodeName) {
                friendlyName = nodeName + "_" + std::to_string(i);
            }

            auto meshGO = make_shared<GameObject>(friendlyName);
            meshGO->AddComponent(ComponentType::TRANSFORM);
            meshGO->SetParent(gameObject);

            // 2. Pass the index to createComponents so we can identify the mesh in the .meta
            createComponentsForMesh(meshGO, aimesh, scene, modelMeta, meshIndex);
        }
       

    }
    else if (currentNode->mNumMeshes == 1) {
        meshIndex = currentNode->mMeshes[0]; // ← Global index
        aiMesh* aiMesh = scene->mMeshes[meshIndex];

        createComponentsForMesh(gameObject, aiMesh, scene, modelMeta, meshIndex);
    }


    LOG("  - Processing %d children for '%s'", currentNode->mNumChildren, gameObject->GetName().c_str());

    for (unsigned int i = 0; i < currentNode->mNumChildren; i++)
        processNodeWithGameObjects(currentNode->mChildren[i], scene, gameObject, modelMeta);
}


void ModelImporter::createComponentsForMesh(std::shared_ptr<GameObject> gameObject, aiMesh* aiMesh, const aiScene* scene, nlohmann::json* modelMeta, uint currentMeshIndex)
{
    LOG("=== createComponentsForMesh START ===");

    VroomUUID existingUUID = 0;
    static int meshCounter = 0; // Temporary fallback to ensure uniqueness
    FileSystem* fs = Application::GetInstance().fileSystem.get();

    if (modelMeta && modelMeta->contains("meshes")) {
        for (auto& m : (*modelMeta)["meshes"]) {
            // Use index as the primary key for uniqueness
            if (m.contains("meshIndex") && m["meshIndex"] == currentMeshIndex) {
                existingUUID = m["meshUUID"].get<VroomUUID>();
                break;
            }
        }
    }

    // Pass the cached UUID if we have one
    auto mesh = Application::GetInstance().importer->meshImporter->Import(aiMesh, scene, fullPath, existingUUID);

    if (!mesh) {
        LOG("ERROR: Failed to import mesh");
        return;
    }

    // register resource
    Application::GetInstance().resourceManager->RegisterResource(mesh);

    meshMetaInfo.push_back({ aiMesh->mName.C_Str(), mesh->GetUUID(), currentMeshIndex });


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


    renderer->SetMesh(mesh);
    // Verify mesh was added properly
    auto verifyMesh = renderer->GetMesh();
    if (!verifyMesh) {
        LOG("ERROR: After SetMesh, GetMesh returns nullptr!");
    }
    else {
        LOG("SUCCESS: Mesh set in renderer (vertices=%d)", verifyMesh->vertices.size());
    }

    if (mesh && !mesh->isLoadedToGPU) {
        Application::GetInstance().resourceManager->LoadResourceToGPU(mesh);
    }

    auto materialComp = gameObject->AddComponent(ComponentType::MATERIAL);
    auto matComponent = std::dynamic_pointer_cast<MaterialComponent>(materialComp);
    if (!matComponent) return;

    //----- TEXTURE SEARCH ------

    // 1. Initial Default to avoid null texture scenario from the start
    std::string checkersTex = Application::GetInstance().resourceManager->checkersTexDir;
    auto resTex = Application::GetInstance().resourceManager->RequestResource(checkersTex.c_str());
    std::shared_ptr<ResourceTexture> finalTexture = std::dynamic_pointer_cast<ResourceTexture>(resTex);
    

    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
        aiString str;
        std::string textureToLoad = "";
        std::string textureFileName = "";
        std::string modelName = fs->GetFileNameFromPath(fullPath.c_str());

        //TRY THE ASSIMP EMBEDDED NAME FIRST (High Accuracy)
        if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS) {
            textureFileName = fs->GetFileFromPath(str.C_Str());
            LOG("[Texture Search] Assimp found embedded name: %s", textureFileName.c_str());

        }
        //Guess based on model name
        else {
            // We don't know the extension, so we'll have to try them in the loop below
            LOG("[Texture Search] No embedded name. Guessing for: %s_diffuse", modelName.c_str());
        }

        // different paths where we'll look for the file
        std::vector<std::string> searchDirs;
        // If external drop, look at the external folders
        if (!lastExternalSourcePath.empty()) {
            std::string sourceDir = lastExternalSourcePath;
            searchDirs.push_back(sourceDir);
            searchDirs.push_back(sourceDir + "/textures");
            searchDirs.push_back(sourceDir + "/../textures");
        }
        // Always look in the local project folders (Assets/Models/ or Assets/Textures/)

        searchDirs.push_back(fs->GetDirFromPath(fullPath.c_str()));
        searchDirs.push_back(std::string(Paths::TEXTURE_ASSETS_DIR));

        //if we have a textureFileNameFromAssimp:
        if (!textureFileName.empty()) {
            for (const auto& dir : searchDirs) {
                std::string testPath = fs->NormalizePath((dir + "/" + textureFileName).c_str());
                LOG("Searching for texture in %s", testPath.c_str());
                if (fs->Exists(testPath.c_str())) {
                    textureToLoad = testPath;
                    break;
                }
            }
        }
        
        //try naming convention (it might have found a texture in assimp, 
        // but it could have been renamed to match the convention)
        if(textureToLoad.empty() || !fs->Exists(textureToLoad.c_str())){
            
            LOG("[Texture Search] Assimp search failed. Trying Naming Convention for: %s_diffuse", modelName.c_str());
            std::vector<std::string> extensions = { ".png", ".jpg", ".tga", ".dds" };
            for (const auto& dir : searchDirs) {
                if (!textureToLoad.empty()) {
                    break;
                }
                
                for (const auto& ext : extensions) {
                    std::string testPath = fs->NormalizePath((dir + "/" + modelName + "_diffuse" + ext).c_str());
                
                    LOG("Checking for texture at: %s", testPath.c_str());
                    
                    if (fs->Exists(testPath.c_str())) {
                        textureToLoad = testPath;

                        LOG("[Texture Search] Found via naming convention: %s", testPath.c_str());
                        break;
                    }
                }
            }
        }

        // Load and Copy
        if (!textureToLoad.empty()) {
            // If the file we found is EXTERNAL (not in Assets), copy it to Assets/Textures
            if (textureToLoad.find(":") == std::string::npos) {
                std::string fileName = fs->GetFileFromPath(textureToLoad.c_str());
                std::string destPath = std::string(Paths::TEXTURE_ASSETS_DIR) + "/" + fileName;

                if (fs->CustomCopyFile(textureToLoad.c_str(), destPath.c_str())) {
                    textureToLoad = destPath; // Point to the new local version
                }
            }

            // Load the resource
            auto res = Application::GetInstance().resourceManager->RequestResource(textureToLoad.c_str());
            auto tex = std::dynamic_pointer_cast<ResourceTexture>(res);
            if (tex) {
                finalTexture = tex;
                LOG("Successfully applied texture: %s", textureToLoad.c_str());
            }
        }
        else {
            LOG("No texture found for mesh, defaulting to checkers.");
        }

        // 2. LOAD MATERIAL PROPERTIES 
        aiColor4D color;
        if (AI_SUCCESS == aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &color)) {
            matComponent->SetDiffuseColor(glm::vec4(color.r, color.g, color.b, color.a));
        }

        float shininess;
        if (AI_SUCCESS == aiGetMaterialFloat(aiMat, AI_MATKEY_SHININESS, &shininess)) {
            matComponent->SetShininess(shininess);
        }

        //FINAL ASSIGNMENT
        if (finalTexture) {
            matComponent->SetDiffuseMap(finalTexture);
        }
    }

    LOG("=== createComponentsForMesh END ===\n");
}


ModelImporter::~ModelImporter() {
   
}


void ModelImporter::SaveModelMeta(const char* modelPath) {
    FileSystem* fs = Application::GetInstance().fileSystem.get();

    nlohmann::json meta;

    // Check if meta already exists to preserve the model UUID
    std::string metaPath = std::string(modelPath) + ".meta";
    VroomUUID modelUUID = 0;

    if (fs->Exists(metaPath.c_str())) {
        meta = fs->LoadJSON(metaPath.c_str());
       
        if (meta.contains("uuid")) {
            modelUUID = meta["uuid"];
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
        meshEntry["meshName"] = meshInfo.name;
        meshEntry["meshUUID"] = meshInfo.uuid;
        meshEntry["meshIndex"] = meshInfo.index;

        //TEXTURES ARE ALREADY SET VIA DIFFUSE MAP UUID IN THE SCENE FILES, THIS IS REDUNDANT AND EVEN DANGEROUS
        //nlohmann::json texturesArray = nlohmann::json::array();
        //for (const auto & texInfo : meshInfo.textures) {
        //    nlohmann::json texEntry;
        //    texEntry["texName"] = texInfo.name;
        //    texEntry["texUUID"] = texInfo.uuid;
        //    texEntry["texType"] = texInfo.texType;

        //    texturesArray.push_back(texEntry);
        //}
        //meshEntry["meshTextures"] = texturesArray;
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

    if (!fs->Exists(metaPath.c_str()) || !fs->IsMetaValid(metaPath.c_str())) {
        LOG("No meta (or invalid meta) file found for model: %s", modelPath);
        return nullptr;
    }

    // Check if model needs reimport
    if (fs->NeedsReimport(metaPath.c_str(), modelPath)) {
        LOG("Model %s was modified. Re-importing while preserving UUIDs...", modelPath);
        // We DON'T return nullptr. We return the meta so we can reuse the IDs!
    }

     //Load and return the meta
    static nlohmann::json meta;  // Static so pointer remains valid
    meta = fs->LoadJSON(metaPath.c_str());

    LOG("Loaded model meta: %s", metaPath.c_str());
    return &meta;
}