#include "Scene.h"
#include "Application.h"
#include "FileSystem.h"
#include "ResourceManager.h"
#include "ModelImporter.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "SceneManager.h"
#include "MaterialComponent.h"
#include "CameraComponent.h"
#include "Log.h"
#include "Importer.h"



Scene::Scene(const std::string& name) : Module(), sceneName(name) {
    // Create scene root
    root = std::make_shared<GameObject>("Scene Root");
    root->AddComponent(ComponentType::TRANSFORM);
    allGameObjects.push_back(root);
    std::unordered_set<std::string> reimportedModels = {};

    //initialize world bounds for octree
    worldBounds.min = glm::vec3(-100.0f, -100.0f, -100.0f);
    worldBounds.max = glm::vec3(100.0f, 100.0f, 100.0f);

    //initialize octree (10 objects per node)
    octree = std::make_unique<Octree>(worldBounds, 10, 5);

    LOG("Scene '%s' created", sceneName.c_str());
}

Scene::~Scene() {
    Clear();
}

void Scene::Clear() {
    allGameObjects.clear();
    root.reset();
    LOG("Scene '%s' cleared", sceneName.c_str());
}


void Scene::AddGameObject(std::shared_ptr<GameObject> go) {
    if (!go) return;

    // Check if already in scene
    for (auto& existing : allGameObjects) {
        if (existing == go) {
            LOG("GameObject '%s' already in scene", go->GetName().c_str());
            return;
        }
    }

    allGameObjects.push_back(go);
    go->SetScene(this);

    // If no parent, set to root
    if (!go->GetParent()) {
        go->SetParent(root);
    }

    //insert object to octree
    if (octree) {
        octree->Insert(go);
    }

    LOG("Added GameObject '%s' to scene", go->GetName().c_str());
}

void Scene::RemoveGameObject(std::shared_ptr<GameObject> go) {
    if (!go) return;

    // Remove from parent
    if (auto parent = go->GetParent()) {
        parent->RemoveChild(go);
    }

    // Remove from list
    auto it = std::find(allGameObjects.begin(), allGameObjects.end(), go);
    if (it != allGameObjects.end()) {
        allGameObjects.erase(it);
        LOG("Removed GameObject '%s' from scene", go->GetName().c_str());
    }
}


void Scene::LogGameObjectHierarchy(std::shared_ptr<GameObject> go, int depth) {
    if (!go) return;

    std::string indent(depth * 2, ' ');
    LOG("%s- '%s' (Active: %s, Components: %d, Children: %d)",
        indent.c_str(),
        go->GetName().c_str(),
        go->IsActive() ? "Yes" : "No",
        go->GetComponentCount(),
        (int)go->GetChildren().size());

    for (auto& child : go->GetChildren())
        LogGameObjectHierarchy(child, depth + 1);
}




void Scene::CleanUpDestroyedObjects() {
    size_t beforeCount = allGameObjects.size();

    // Eliminar GameObjects marcados
    allGameObjects.erase(std::remove_if(allGameObjects.begin(), allGameObjects.end(), [](const std::shared_ptr<GameObject>& go) 
        {
            return go && go->IsMarkedForDestroy();
        }), allGameObjects.end());

    size_t afterCount = allGameObjects.size();
    if (beforeCount != afterCount) {
        LOG("Cleanup: Removed %d GameObject(s). Remaining: %d",
            (int)(beforeCount - afterCount), (int)afterCount);
    }
}


std::shared_ptr<GameObject> Scene::ImportModel(const std::string& modelPath, nlohmann::json* modelMeta, bool addToScene) {
    LOG("Scene: Importing model '%s'", modelPath.c_str());

    // Call SceneImporter (renamed from ModelImporter::ImportScene)
  
    
    std::shared_ptr<GameObject> sceneGO = Application::GetInstance().importer.get()->modelImporter->ImportScene(modelPath.c_str());

    if (!sceneGO) {
        LOG("ERROR: Failed to import model");
        return nullptr;
    }

    if (addToScene) {
        // Add to scene
        AddGameObject(sceneGO);

        // Collect all children recursively and add them
        CollectAllGameObjects(sceneGO);

        if (octree) {
            octree->Rebuild(allGameObjects);
        }
    }

    LOG("Model imported successfully: %d GameObjects created", (int)allGameObjects.size());
    return sceneGO;
}

void Scene::CollectAllGameObjects(std::shared_ptr<GameObject> go) {
    if (!go) return;

    for (auto& child : go->GetChildren()) {
        // Check if already added
        bool found = false;
        for (auto& existing : allGameObjects) {
            if (existing == child) {
                found = true;
                break;
            }
        }

        if (!found) {
            allGameObjects.push_back(child);
        }

        // Recurse
        CollectAllGameObjects(child);
    }
}

bool Scene::SaveScene(const std::string& filePath) {
    LOG("Saving scene '%s' to '%s'", sceneName.c_str(), filePath.c_str());

    // Extract directory from full path
    FileSystem* fs = Application::GetInstance().fileSystem.get();
    std::string directory = fs->GetDirFromPath(filePath.c_str());

    // Create directory if it doesn't exist (e.g., "Assets/Scenes")
    if (!directory.empty() && !fs->Exists(directory.c_str())) {
        fs->CreateDir(directory.c_str());
    }

    nlohmann::json sceneMeta;
    sceneMeta["1.name"] = sceneName;
    sceneMeta["2.gameObjects"] = nlohmann::json::array();

    // Serialize all GameObjects
    for (auto& go : root->GetChildren()) {
        // No need to check for 'go == root' as we are getting children of root.
        sceneMeta["2.gameObjects"].push_back(SerializeGameObject(go));
    }

    // Save to file
   
    fs->SaveJSON(filePath.c_str(), sceneMeta);

    LOG("Scene saved successfully");
    return true;
}

bool Scene::LoadScene(const std::string& filePath) {
    LOG("Loading scene from '%s'", filePath.c_str());

    FileSystem* fs = Application::GetInstance().fileSystem.get();

    if (!fs->Exists(filePath.c_str())) {
        LOG("ERROR: Scene file not found: %s", filePath.c_str());
        return false;
    }

    nlohmann::json sceneMeta = fs->LoadJSON(filePath.c_str());

    // Clear current scene
    Clear();
   

    // Load scene name
    if (sceneMeta.contains("1.name")) {
        sceneName = sceneMeta["1.name"];
    }

    // Recreate root
    root = std::make_shared<GameObject>("Scene Root");
    root->AddComponent(ComponentType::TRANSFORM);
    allGameObjects.push_back(root);

    // Deserialize GameObjects
    if (sceneMeta.contains("2.gameObjects")) {
        for (auto& metaGO : sceneMeta["2.gameObjects"]) {
            std::string sourceModelName = metaGO["1.name"];
            auto go = DeserializeGameObject(metaGO, sourceModelName);
            if (go) {
                go->SetParent(root); 
                
            }
        }
    }

    for (auto& topLevelObject : root->GetChildren()) {
        CollectAllGameObjects(topLevelObject);
    }

    if (octree) {
        octree->Rebuild(allGameObjects);
    }

    LOG("Scene loaded successfully: %d GameObjects", (int)allGameObjects.size());
    return true;
}

nlohmann::json Scene::SerializeGameObject(std::shared_ptr<GameObject> go) {
    nlohmann::json goMeta;

    goMeta["1.name"] = go->GetName();
    goMeta["2.active"] = go->IsActive();

    // Serialize Transform
    auto transformComp = go->GetComponent(ComponentType::TRANSFORM);
    if (transformComp) {
        auto transform = std::dynamic_pointer_cast<TransformComponent>(transformComp);
        goMeta["3.transform"]["position"] = {
            transform->GetPosition().x,
            transform->GetPosition().y,
            transform->GetPosition().z
        };
        goMeta["3.transform"]["rotation"] = {
            transform->GetRotation().w,
            transform->GetRotation().x,
            transform->GetRotation().y,
            transform->GetRotation().z
        };
        goMeta["3.transform"]["scale"] = {
            transform->GetScale().x,
            transform->GetScale().y,
            transform->GetScale().z
        };
    }

    // Serialize MeshRenderer (store mesh UUID reference)
    auto rendererComp = go->GetComponent(ComponentType::MESH_RENDERER);
    if (rendererComp) {
        auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);
        goMeta["4.meshRenderer"]["meshUUID"] = renderer->GetMeshUUID();
    }

    // Serialize Material
    auto materialComp = go->GetComponent(ComponentType::MATERIAL);
    if (materialComp) {
        auto material = std::dynamic_pointer_cast<MaterialComponent>(materialComp);
        goMeta["5.material"]["diffuseColor"] = {
            material->GetDiffuseColor().r,
            material->GetDiffuseColor().g,
            material->GetDiffuseColor().b,
            material->GetDiffuseColor().a
        };
        goMeta["5.material"]["shininess"] = material->GetShininess();
        goMeta["5.material"]["metallic"] = material->GetMetallic();
        goMeta["5.material"]["roughness"] = material->GetRoughness();


        // Store textures UUID if they exist
        if (material->GetDiffuseMap()) {
            VroomUUID diffuseTexUUID = material->GetDiffuseMap()->GetUUID();
            if (diffuseTexUUID != 0)
                goMeta["5.material"]["diffuseMapUUID"] = diffuseTexUUID;
        }

        if (material->GetNormalMap()) {
            VroomUUID normalTexUUID = material->GetNormalMap()->GetUUID();
            if (normalTexUUID != 0)
                goMeta["5.material"]["normalMapUUID"] = normalTexUUID;
        }

        if (material->GetMetallicMap()) {
            VroomUUID metallicTexUUID = material->GetMetallicMap()->GetUUID();
            if (metallicTexUUID != 0)
                goMeta["5.material"]["metallicMapUUID"] = metallicTexUUID;
        }

        if (material->GetRoughnessMap()) {
            VroomUUID roughTexUUID = material->GetRoughnessMap()->GetUUID();
            if (roughTexUUID != 0)
                goMeta["5.material"]["roughnessMapUUID"] = roughTexUUID;
        }

        if (material->GetAOMap()) {
            VroomUUID aoTexUUID = material->GetAOMap()->GetUUID();
            if (aoTexUUID != 0)
                goMeta["5.material"]["AOMapUUID"] = aoTexUUID;
        }
    }

    // Serialize children recursively
    goMeta["6.children"] = nlohmann::json::array();
    for (auto& child : go->GetChildren()) {
        goMeta["6.children"].push_back(SerializeGameObject(child));
    }

    return goMeta;
}


std::shared_ptr<GameObject> Scene::DeserializeGameObject(const nlohmann::json& goMeta, const std::string sourceModelName) {
    std::string name = goMeta.value("1.name", "2.GameObject");
    auto go = std::make_shared<GameObject>(name);

    if (goMeta.contains("2.active")) {
        go->SetActive(goMeta["2.active"]);
    }

    auto transformComp = go->AddComponent(ComponentType::TRANSFORM);

    if (goMeta.contains("3.transform")) {
        auto transform = std::dynamic_pointer_cast<TransformComponent>(transformComp);
        const auto& t = goMeta["3.transform"];

        if (t.contains("position")) {
            auto pos = t["position"];
            transform->SetPosition(glm::vec3(pos[0], pos[1], pos[2]));
        }
        if (t.contains("rotation")) {
            auto rot = t["rotation"];
            transform->SetRotation(glm::quat(rot[0], rot[1], rot[2], rot[3]));
        }
        if (t.contains("scale")) {
            auto scale = t["scale"];
            transform->SetScale(glm::vec3(scale[0], scale[1], scale[2]));
        }
    }


    if (goMeta.contains("4.meshRenderer")) {
        auto& mr = goMeta["4.meshRenderer"];
        auto rendererComp = go->AddComponent(ComponentType::MESH_RENDERER);
        LOG("Added MESH_RENDERER component to GameObject '%s'", go->GetName().c_str());
        auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);

        VroomUUID meshUUID = mr["meshUUID"];
        renderer->SetMeshUUID(meshUUID);

        // Request mesh from ResourceManager
        auto resMesh = Application::GetInstance().resourceManager.get()->RequestResource(meshUUID);
       

        if (resMesh == nullptr) {
            if (reimportedModels.find(sourceModelName) == reimportedModels.end()) {

                // Mark the model as reimported to avoid repeating
                reimportedModels.insert(sourceModelName);

                LOG("Mesh missing -> reimporting entire model once: %s", sourceModelName.c_str());

                std::string modelPath = Application::GetInstance().fileSystem->NormalizePath(
                    FindModelInAssetsFolder(sourceModelName).c_str()
                );

                // IMPORT WITH NO META
                //Application::GetInstance().importer->modelImporter->ImportScene(modelPath.c_str());
                Application::GetInstance().sceneManager->GetActiveScene()->ImportModel(modelPath.c_str(), nullptr, false);
                // Traverse importedRoot to find the mesh by UUID

                // Try fetching the mesh again from the ResourceManager
                resMesh = Application::GetInstance().resourceManager->RequestResource(meshUUID);

                if (resMesh != nullptr)
                {
                    // Update JSON with new UUID
                }
                else
                {
                    LOG("ERROR: Mesh still missing after reimport: %llu", meshUUID);
                }

            }
            else
            {
                LOG("Mesh missing but model already reimported. Skipping second reimport.");
            }
        }
        
        auto mesh = std::dynamic_pointer_cast<ResourceMesh>(resMesh);
        renderer->SetMesh(mesh);

        if (mesh && !mesh->isLoadedToGPU) {
            Application::GetInstance().resourceManager.get()->LoadResourceToGPU(mesh);
        }

        //checking for material entry only if mesh exists

        if (goMeta.contains("5.material")) {
            auto& m = goMeta["5.material"];
            auto materialComp = go->AddComponent(ComponentType::MATERIAL);
            auto material = std::dynamic_pointer_cast<MaterialComponent>(materialComp);

            // Diffuse color
            if (m.contains("diffuseColor")) {
                auto c = m["diffuseColor"];
                material->SetDiffuseColor(glm::vec4(c[0], c[1], c[2], c[3]));
            }

            // PBR values
            if (m.contains("shininess")) material->SetShininess(m["shininess"]);
            if (m.contains("metallic"))  material->SetMetallic(m["metallic"]);
            if (m.contains("roughness")) material->SetRoughness(m["roughness"]);

            // Texture maps (UUIDs)
            if (m.contains("diffuseMapUUID")) {
                VroomUUID id = m["diffuseMapUUID"];
                auto resTex = Application::GetInstance().resourceManager.get()->RequestResource(id);
                std::shared_ptr<Resource> textureToAssign = resTex;

                if (!textureToAssign) {
                    std::string checkerPath = Application::GetInstance().importer.get()->defaultTexDir;
                    textureToAssign = Application::GetInstance().resourceManager.get()->RequestResource(checkerPath);
                    LOG("WARNING: Diffuse texture UUID %llu missing. Falling back to checker.", id);
                }

                
                if (textureToAssign) {
                    material->SetDiffuseMap(std::dynamic_pointer_cast<ResourceTexture>(textureToAssign));
                }
                
               
            }

            if (m.contains("normalMapUUID")) {
                VroomUUID id = m["normalMapUUID"];
                auto tex = Application::GetInstance().resourceManager.get()->RequestResource(id);
                /*if (!tex->isLoadedToGPU) Application::GetInstance().resourceManager.get()->LoadResourceToGPU(tex);*/
                material->SetNormalMap(std::static_pointer_cast<ResourceTexture>(tex));
            }

            if (m.contains("metallicMapUUID")) {
                VroomUUID id = m["metallicMapUUID"];
                auto tex = Application::GetInstance().resourceManager.get()->RequestResource(id);
                /*if (!tex->isLoadedToGPU) Application::GetInstance().resourceManager.get()->LoadResourceToGPU(tex);*/
                material->SetMetallicMap(std::static_pointer_cast<ResourceTexture>(tex));
            }

            if (m.contains("roughnessMapUUID")) {
                VroomUUID id = m["roughnessMapUUID"];
                auto tex = Application::GetInstance().resourceManager.get()->RequestResource(id);
                /*if (!tex->isLoadedToGPU) Application::GetInstance().resourceManager.get()->LoadResourceToGPU(tex);*/
                material->SetRoughnessMap(std::static_pointer_cast<ResourceTexture>(tex));
            }

            if (m.contains("AOMapUUID")) {
                VroomUUID id = m["AOMapUUID"];
                auto tex = Application::GetInstance().resourceManager.get()->RequestResource(id);
                /*if (!tex->isLoadedToGPU) Application::GetInstance().resourceManager.get()->LoadResourceToGPU(tex);*/
                material->SetAOMap(std::static_pointer_cast<ResourceTexture>(tex));
            }


        }

        

       
    }

    if (goMeta.contains("6.children")) {
        for (auto& childJson : goMeta["6.children"]) {
            auto child = DeserializeGameObject(childJson, sourceModelName);
            if (child) {
                child->SetParent(go);
            }
        }
    }

    return go;
}

std::shared_ptr<GameObject> Scene::GetModelParentGameObject(std::shared_ptr<GameObject> go)
{
    if (!go) 
        return nullptr;

    auto parent = go->GetParent();

    if (parent == GetRoot())
        return go;

    return GetModelParentGameObject(parent);
}

std::shared_ptr<GameObject> Scene::FindGameObjectByUUID(VroomUUID uuid) {
    
    for (auto object : allGameObjects) {
        if (object->GetUUID() == uuid) {
            return object;
        }
    }
    return nullptr;
}

std::shared_ptr<GameObject> Scene::FindGameObjectByName(const std::string name) {
    for (auto object : allGameObjects) {
        if (object->GetName() == name) {
            return object;
        }
    }
    return nullptr;
}

std::string Scene::FindModelInAssetsFolder(std::string sourceModelName) {

    for (const auto& entry : std::filesystem::recursive_directory_iterator(std::string(Paths::MODEL_ASSETS_DIR))) {

        if (entry.path().extension().string() == ".fbx" || entry.path().extension().string() == ".FBX" || entry.path().extension().string() == ".obj") {
            std::string modelName = entry.path().stem().string();
            if (modelName == sourceModelName) {
                return std::string(entry.path().string());
            }
        }

    }

    LOG("Could not retrieve source model file name");
    return "";
}

std::vector<std::shared_ptr<CameraComponent>> Scene::GetCameras() const {
    std::vector<std::shared_ptr<CameraComponent>> cameras;
    for (auto& go : allGameObjects) {
        if (!go) continue;
        auto cam = std::dynamic_pointer_cast<CameraComponent>(go->GetComponent(ComponentType::CAMERA));
        if (cam) {
            cameras.push_back(cam);
        }
    }
    return cameras;
}