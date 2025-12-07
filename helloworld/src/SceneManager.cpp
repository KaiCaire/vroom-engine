#include "SceneManager.h"
#include "ResourceManager.h"
#include "Application.h"
#include "RenderMeshComponent.h"


using namespace std;

SceneManager::SceneManager() {

}

SceneManager::~SceneManager() {

}

bool SceneManager::Start() {
    return true;
}

bool SceneManager::Update(float dt) {
    /*currentScene->DebugDrawTree();*/

    currentScene->Update(static_cast<float>(dt));
    return true;
}

bool SceneManager::CleanUp() {

    return true;
}


// In SceneManager.cpp
void SceneManager::LoadDefaultScene() {
    LOG("SceneManager: Loading default scene");

    // Create a new scene
    const std::string sceneName = "DefaultScene";
    auto defaultScene = std::make_shared<Scene>(sceneName);
    scenes.push_back(defaultScene);
    SetActiveScene(sceneName);

    // Import the model through ResourceManager
    std::string modelPath = "../Assets/Models/Kodama/Kodama_LOW.fbx";
    auto ourScene = currentScene->ImportModel(modelPath);

    if (ourScene) {
        LOG("Default scene loaded successfully");
    }
    
}



void SceneManager::SetActiveScene(const std::string& name)
{
    for (const auto& scene : scenes) {
        if (scene->GetName() == name) {
            currentScene = scene;
            currentScene->Start();
            return;
        }
    }
}

std::shared_ptr<Scene> SceneManager::GetActiveScene() const
{
    return currentScene;
}



std::shared_ptr<GameObject> SceneManager::CreateGameObject(const std::string& name) {

    auto go = std::make_shared<GameObject>(name);
    go->AddComponent(ComponentType::TRANSFORM);
    go->SetParent(currentScene->GetRoot());

    currentScene->AddGameObject(go);

    LOG("Created GameObject '%s' in scene '%s'", name.c_str(), currentScene->GetName().c_str());
    return go;
}

std::shared_ptr<GameObject> SceneManager::CreateEmptyGameObject(const std::string& name, std::shared_ptr<GameObject> parent) {
    LOG("Creating empty GameObject: '%s'", name.c_str());

    // Crear GameObject vacío
    auto newGameObject = std::make_shared<GameObject>(name);

    // Añadir Transform (todos los GameObjects necesitan Transform)
    newGameObject->AddComponent(ComponentType::TRANSFORM);

    // Establecer parent
    if (parent) {
        newGameObject->SetParent(parent);
        LOG("  - Parent set to '%s'", parent->GetName().c_str());
    }
    else if (currentScene->GetRoot()) {
        // Si no se especifica parent, usar el root
        newGameObject->SetParent(currentScene->GetRoot());
        LOG("  - Parent set to root");
    }

    // Añadir a la lista
    /*allGameObjects.push_back(newGameObject);*/
    currentScene->AddGameObject(newGameObject);

    int gameObjectsSize = currentScene->GetAllGameObjects().size();

    LOG("Empty GameObject '%s' created successfully with UUID %llu (Total GameObjects: %d)", name.c_str(), newGameObject->GetUUID(),  gameObjectsSize);

    return newGameObject;
}


void SceneManager::DestroyGameObject(std::shared_ptr<GameObject> gameObject) {
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

// --------------- PRIMITIVES CREATION ---------------

std::shared_ptr<GameObject> SceneManager::CreateCube() {
    if (!currentScene) {
        LOG("ERROR: Cannot create cube, no active scene found.");
        return nullptr;
    }


    ResourceManager* resourceManager = Application::GetInstance().resourceManager.get();
    if (!resourceManager) {
        LOG("FATAL ERROR: ResourceManager not available. Cannot create primitive.");
        return nullptr;
    }


    std::shared_ptr<ResourceMesh> cubeMesh = resourceManager->GetPrimitiveMesh(PrimitiveType::CUBE);
    

    if (!cubeMesh) {
        LOG("ERROR: Failed to retrieve or generate cube mesh resource from ResourceManager.");
        return nullptr;
    }

    std::shared_ptr<GameObject> cubeGO = CreateEmptyGameObject("Cube", currentScene->GetRoot());

    if (cubeGO) {
        // Render Component
        auto renderComp = std::dynamic_pointer_cast<RenderMeshComponent>(cubeGO->AddComponent(ComponentType::MESH_RENDERER));

       

        
        if (renderComp) {
            renderComp->SetMesh(cubeMesh);
            VroomUUID meshUUID = cubeMesh->GetUUID();
            if (meshUUID == 0) UUIDGen::GenerateUUID();
            /*renderComp->SetMeshUUID(meshUUID);*/
            resourceManager->AddReference(meshUUID);

            LOG("GameObject '%s' created and linked to cube mesh (UUID: %llu)", "Cube", meshUUID);
        }

        // Material Component
        cubeGO->AddComponent(ComponentType::MATERIAL);
    }

    return cubeGO;
}




