#include "Scene.h"
#include "Application.h"
#include "FileSystem.h"
#include "ResourceManager.h"
#include "ModelImporter.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include "Log.h"
#include "Importer.h"

Scene::Scene(const std::string& name) : Module(), sceneName(name) {
    // Create scene root
    root = std::make_shared<GameObject>("Scene Root");
    root->AddComponent(ComponentType::TRANSFORM);
    allGameObjects.push_back(root);

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

    // If no parent, set to root
    if (!go->GetParent()) {
        go->SetParent(root);
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


std::shared_ptr<GameObject> Scene::ImportModel(const std::string& modelPath) {
    LOG("Scene: Importing model '%s'", modelPath.c_str());

    // Call SceneImporter (renamed from ModelImporter::ImportScene)
  
    
    std::shared_ptr<GameObject> sceneGO = Application::GetInstance().importer.get()->modelImporter->ImportScene(modelPath.c_str());

    if (!sceneGO) {
        LOG("ERROR: Failed to import model");
        return nullptr;
    }

    // Add to scene
    AddGameObject(sceneGO);

    // Collect all children recursively and add them
    CollectAllGameObjects(sceneGO);

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

    nlohmann::json sceneMeta;
    sceneMeta["name"] = sceneName;
    sceneMeta["gameObjects"] = nlohmann::json::array();

    // Serialize all GameObjects
    for (auto& go : allGameObjects) {
        if (go == root) continue; // Skip root, it's implicit
        sceneMeta["gameObjects"].push_back(SerializeGameObject(go));
    }

    // Save to file
    FileSystem* fs = Application::GetInstance().fileSystem.get();
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
    if (sceneMeta.contains("name")) {
        sceneName = sceneMeta["name"];
    }

    // Recreate root
    root = std::make_shared<GameObject>("Scene Root");
    root->AddComponent(ComponentType::TRANSFORM);
    allGameObjects.push_back(root);

    // Deserialize GameObjects
    if (sceneMeta.contains("gameObjects")) {
        for (auto& metaGO : sceneMeta["gameObjects"]) {
            auto go = DeserializeGameObject(metaGO);
            if (go) {
                allGameObjects.push_back(go);
            }
        }
    }

    LOG("Scene loaded successfully: %d GameObjects", (int)allGameObjects.size());
    return true;
}

nlohmann::json Scene::SerializeGameObject(std::shared_ptr<GameObject> go) {
    nlohmann::json goMeta;

    goMeta["name"] = go->GetName();
    goMeta["active"] = go->IsActive();

    // Serialize Transform
    auto transformComp = go->GetComponent(ComponentType::TRANSFORM);
    if (transformComp) {
        auto transform = std::dynamic_pointer_cast<TransformComponent>(transformComp);
        goMeta["transform"]["position"] = {
            transform->GetPosition().x,
            transform->GetPosition().y,
            transform->GetPosition().z
        };
        goMeta["transform"]["rotation"] = {
            transform->GetRotation().w,
            transform->GetRotation().x,
            transform->GetRotation().y,
            transform->GetRotation().z
        };
        goMeta["transform"]["scale"] = {
            transform->GetScale().x,
            transform->GetScale().y,
            transform->GetScale().z
        };
    }

    // Serialize MeshRenderer (store mesh UUID reference)
    auto rendererComp = go->GetComponent(ComponentType::MESH_RENDERER);
    if (rendererComp) {
        auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);
        goMeta["meshRenderer"]["meshUUID"] = renderer->GetMeshUUID();
    }

    // Serialize Material
    auto materialComp = go->GetComponent(ComponentType::MATERIAL);
    if (materialComp) {
        auto material = std::dynamic_pointer_cast<MaterialComponent>(materialComp);
        goMeta["material"]["diffuseColor"] = {
            material->GetDiffuseColor().r,
            material->GetDiffuseColor().g,
            material->GetDiffuseColor().b,
            material->GetDiffuseColor().a
        };
        goMeta["material"]["shininess"] = material->GetShininess();
        goMeta["material"]["metallic"] = material->GetMetallic();
        goMeta["material"]["roughness"] = material->GetRoughness();
       

        // Store textures UUID if they exist
        if (material->GetDiffuseMap()) {
            VroomUUID diffuseTexUUID = material->GetDiffuseMap()->GetUUID();
            if(diffuseTexUUID!= 0)
            goMeta["material"]["diffuseMapUUID"] = diffuseTexUUID;
        }

        if (material->GetNormalMap()) {
            VroomUUID normalTexUUID = material->GetNormalMap()->GetUUID();
            if (normalTexUUID != 0)
                goMeta["material"]["normalMapUUID"] = normalTexUUID;
        }

        if (material->GetMetallicMap()) {
            VroomUUID metallicTexUUID = material->GetMetallicMap()->GetUUID();
            if (metallicTexUUID != 0)
                goMeta["material"]["metallicMapUUID"] = metallicTexUUID;
        }

        if (material->GetRoughnessMap()) {
            VroomUUID roughTexUUID = material->GetRoughnessMap()->GetUUID();
            if (roughTexUUID != 0)
                goMeta["material"]["roughnessMapUUID"] = roughTexUUID;
        }

        if (material->GetAOMap()) {
            VroomUUID aoTexUUID = material->GetAOMap()->GetUUID();
            if (aoTexUUID != 0)
                goMeta["material"]["AOMapUUID"] = aoTexUUID;
        }
    }

    // Serialize children recursively
    goMeta["children"] = nlohmann::json::array();
    for (auto& child : go->GetChildren()) {
        goMeta["children"].push_back(SerializeGameObject(child));
    }

    return goMeta;
}

std::shared_ptr<GameObject> Scene::DeserializeGameObject(const nlohmann::json& goMeta) {
    std::string name = goMeta.value("name", "GameObject");
    auto go = std::make_shared<GameObject>(name);

    if (goMeta.contains("active")) {
        go->SetActive(goMeta["active"]);
    }

    auto transformComp = go->AddComponent(ComponentType::TRANSFORM);

    if (goMeta.contains("transform")) {
        auto transform = std::dynamic_pointer_cast<TransformComponent>(transformComp);
        const auto& t = goMeta["transform"];

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


    if (goMeta.contains("meshRenderer")) {
        auto& mr = goMeta["meshRenderer"];
        auto rendererComp = go->AddComponent(ComponentType::MESH_RENDERER);
        auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);

        VroomUUID meshUUID = mr["meshUUID"];
        renderer->SetMeshUUID(meshUUID);

        // Request mesh from ResourceManager
        auto mesh = Application::GetInstance().resourceManager.get()->RequestResource(meshUUID);
        renderer->SetMesh(std::dynamic_pointer_cast<ResourceMesh>(mesh));
    }


    if (goMeta.contains("material")) {
        auto& m = goMeta["material"];
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
            auto tex = Application::GetInstance().resourceManager.get()->RequestResource(id);
            material->SetDiffuseMap(std::static_pointer_cast<ResourceTexture>(tex));
        }

        if (m.contains("normalMapUUID")) {
            VroomUUID id = m["normalMapUUID"];
            auto tex = Application::GetInstance().resourceManager.get()->RequestResource(id);
            material->SetNormalMap(std::static_pointer_cast<ResourceTexture>(tex));
        }

        if (m.contains("metallicMapUUID")) {
            VroomUUID id = m["metallicMapUUID"];
            auto tex = Application::GetInstance().resourceManager.get()->RequestResource(id);
            material->SetMetallicMap(std::static_pointer_cast<ResourceTexture>(tex));
        }

        if (m.contains("roughnessMapUUID")) {
            VroomUUID id = m["roughnessMapUUID"];
            auto tex = Application::GetInstance().resourceManager.get()->RequestResource(id);
            material->SetRoughnessMap(std::static_pointer_cast<ResourceTexture>(tex));
        }

        if (m.contains("AOMapUUID")) {
            VroomUUID id = m["AOMapUUID"];
            auto tex = Application::GetInstance().resourceManager.get()->RequestResource(id);
            material->SetAOMap(std::static_pointer_cast<ResourceTexture>(tex));
        }
    }


    if (goMeta.contains("children")) {
        for (auto& childJson : goMeta["children"]) {
            auto child = DeserializeGameObject(childJson);
            if (child) {
                child->SetParent(go);
            }
        }
    }

    return go;
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