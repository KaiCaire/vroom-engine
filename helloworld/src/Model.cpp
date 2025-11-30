#include "Model.h"
#include "Application.h"
#include "OpenGL.h"
#include "FileSystem.h"

#include "MeshImporter.h"
#include "TextureImporter.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"


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


void Model::ImportScene(const char* path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }

    
    FileSystem* fs = Application::GetInstance().fileSystem.get();
    // Parse path info
    fullPath = fs->NormalizePath(path);
    fileName = fs->GetFileNameFromPath(path);

    // Check for existing meta
    std::string metaPath = std::string(path) + ".meta";
    std::vector<VroomUUID> existingMeshUIDs;
    bool alreadyImported = false;

    if (fs->Exists(metaPath.c_str())) {
        if (fs->IsMetaValid(metaPath.c_str())) {
            // Already imported and valid

            alreadyImported = true;
            LOG("Scene already imported: %s", fullPath);

        }
    }


    stbi_set_flip_vertically_on_load(fileExtension == "obj");

    rootGameObject = make_shared<GameObject>(fileName);
    Application::GetInstance().guiManager.get()->sceneObjects.push_back(rootGameObject);
    rootGameObject.get()->SetOwnerModel(this);

    rootGameObject->AddComponent(ComponentType::TRANSFORM);
    
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

Model::Model(std::shared_ptr<ResourceMesh> sharedMesh) {
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

        //auto transformComp = gameObject->GetComponent(ComponentType::TRANSFORM);
        //if (transformComp) {
        //    auto transform = std::dynamic_pointer_cast<TransformComponent>(transformComp);
        //    // Get the Model Matrix (Global Transform)
        //    glm::mat4 modelMatrix = transform->GetGlobalTransform();

        //    // Pass the matrix to the shader (assuming your shader has a 'model' uniform)
        //    shader.setMat4("model", modelMatrix);
        //}

        //trigger checkerboard texture
        //if (useDefaultTexture) {
        //    //store original texture if not yet stored
        //    if (originalTextures.find(mesh) == originalTextures.end()) {
        //        originalTextures[mesh] = mesh->textures;
        //    }

        //    mesh->textures.clear();

        //    std::string checkersTexDir = Application::GetInstance().importer->defaultTexDir;
        //    std::string checkersTexName = checkersTexDir.substr(checkersTexDir.find_last_of('/') + 1);
        //    std::shared_ptr<ResourceTexture> checkersTex = GetOrLoadTexture(checkersTexDir, checkersTexName, "texture_diffuse");

        //    mesh->textures.push_back(checkersTex);
        //}
        //else {
        //    //restore original texture
        //    auto ogTex = originalTextures.find(mesh);
        //    if (ogTex != originalTextures.end()) {
        //        mesh->textures = ogTex->second;
        //        //originalTextures.erase(ogTex);
        //    }

        //}

        if (!mesh->textures.empty() && mesh->textures[0])
        {
            // Activate Texture Unit 0 (GL_TEXTURE0)
            glActiveTexture(GL_TEXTURE0);

            // Bind the actual OpenGL texture ID. 
            // This assumes ResourceTexture has a method GetID() which returns the GLuint texture ID.
            GLuint textureID = mesh->textures[0]->id;
            glBindTexture(GL_TEXTURE_2D, textureID);

            // Tell the shader sampler 'material.texture_diffuse1' to use the texture bound to unit 0
            // This assumes your Shader class has a setInt method
            shader.setInt("material.texture_diffuse1", 0);
        }

        // Restore default texture unit (good practice)
        glActiveTexture(GL_TEXTURE0);

        //draw the mesh
        renderer->GetMesh()->Draw(shader);
    }
}

void Model::processNodeWithGameObjects(const aiNode* node, const aiScene* scene, shared_ptr<GameObject> parent) {

    aiMatrix4x4 accumulatedTransform;
    const aiNode* currentNode = node;

    while (std::string(currentNode->mName.C_Str()).find("_$AssimpFbx$_") != std::string::npos) {
        accumulatedTransform = accumulatedTransform * currentNode->mTransformation;
        if (currentNode->mNumChildren > 1) {
            LOG("WARNING: FBX dummy node has multiple children");
        }
        currentNode = currentNode->mChildren[0];
    }

    auto gameObject = make_shared<GameObject>(currentNode->mName.C_Str());
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

            meshIndex = currentNode->mMeshes[i];
            aiMesh* aimesh = scene->mMeshes[meshIndex];

            
            auto meshGO = make_shared<GameObject>(nodeName);
            gameObjects.push_back(meshGO);

            meshGO->AddComponent(ComponentType::TRANSFORM);
            meshGO->SetParent(gameObject);

            createComponentsForMesh(meshGO, aimesh, scene);
        }
       

    }
    else if (currentNode->mNumMeshes == 1) {
        meshIndex = currentNode->mMeshes[0]; // ← Global index
        aiMesh* aiMesh = scene->mMeshes[meshIndex];


        createComponentsForMesh(gameObject, aiMesh, scene);
    }
    //else { //no meshes-> create empty GO
    //    auto emptyGO = make_shared<GameObject>(nodeName);
    //    emptyGO->AddComponent(ComponentType::TRANSFORM);
    //    emptyGO->SetParent(gameObject);
    //    gameObjects.push_back(emptyGO);
    //}



    LOG("  - Processing %d children for '%s'", currentNode->mNumChildren, gameObject->GetName().c_str());

    for (unsigned int i = 0; i < currentNode->mNumChildren; i++)
        processNodeWithGameObjects(currentNode->mChildren[i], scene, gameObject);
}

//Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
//    vector<Vertex> vertices;
//    vector<unsigned int> indices;
//    vector<Texture> textures;
//    
//
//    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
//        Vertex vertex;
//        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
//
//        if (mesh->HasNormals())
//            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
//
//        if (mesh->mTextureCoords[0])
//            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
//        else
//            vertex.texCoord = glm::vec2(0.0f, 0.0f);
//
//        vertices.push_back(vertex);
//    }
//
//    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//        aiFace face = mesh->mFaces[i];
//        for (unsigned int j = 0; j < face.mNumIndices; j++)
//            indices.push_back(face.mIndices[j]);
//    }
//
//    if (mesh->mMaterialIndex >= 0) {
//        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
//        auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
//        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//
//        auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
//        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//
//    }
//    else {
//        AssignDefaultTexture(textures);
//        /*string checkersTexDir = Application::GetInstance().textures.get()->defaultTexDir;
//        string checkersTexName = checkersTexDir.substr(0, checkersTexDir.find_last_of('/') + 1);
//        Texture defaultTex = GetOrLoadTexture(checkersTexDir, checkersTexName, "texture_diffuse");
//        textures.push_back(defaultTex);*/
//        
//        
//    }
//
//    processedMeshes++;
//    LOG("Processed mesh %d", processedMeshes);
//    return Mesh(vertices, indices, textures);
//}
//
//vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
//    vector<Texture> textures;
//
//    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
//        aiString str;
//        mat->GetTexture(type, i, &str);
//        
//        textures.push_back(GetOrLoadTexture(fullPath, str.C_Str(), typeName));
//    }
//
//    
//
//    return textures;
//}

void Model::createComponentsForMesh(std::shared_ptr<GameObject> gameObject, aiMesh* aiMesh, const aiScene* scene)
{
    LOG("=== createComponentsForMesh START ===");
    LOG("GameObject: '%s'", gameObject->GetName().c_str());
    LOG("aiMesh vertices: %d, faces: %d", aiMesh->mNumVertices, aiMesh->mNumFaces);

    // MeshImporter returns a Mesh directly
    auto mesh = Application::GetInstance().importer->meshImporter->Import(aiMesh, scene, fullPath);

    if (!mesh) {
        LOG("ERROR: MeshImporter::Import returned nullptr!");
        return;
    }

    LOG("Mesh created: vertices=%d, indices=%d, textures=%d",
        mesh->vertices.size(), mesh->indices.size(), mesh->textures.size());

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

std::shared_ptr<ResourceTexture> Model::GetOrLoadTexture(const std::string& fullPath, const std::string& fileName, const std::string& typeName) {

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

void Model::AssignDefaultTexture(std::vector<std::shared_ptr<ResourceTexture>>& textures) {
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

std::shared_ptr<GameObject> Model::CreateGameObject(const std::string& name, VroomUUID meshUID, std::shared_ptr<GameObject> parent, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) {

    auto go = std::make_shared<GameObject>(name);


    if (parent) {
        go->SetParent(parent);  // Just pass the shared_ptr directly
    }


    // Add Transform component
    auto transformComp = go->AddComponent(ComponentType::TRANSFORM);
    auto transform = std::static_pointer_cast<TransformComponent>(transformComp);
    transform->SetPosition(position);
    transform->SetRotation(rotation);
    transform->SetScale(scale);

    // Add RenderMeshComponent
    auto rendererComp = go->AddComponent(ComponentType::MESH_RENDERER);
    auto renderer = std::static_pointer_cast<RenderMeshComponent>(rendererComp);
    renderer->SetMeshID(meshUID);

    LOG("Created GameObject '%s' with mesh (UUID: %llu)", name.c_str(), meshUID);

    return go;
}