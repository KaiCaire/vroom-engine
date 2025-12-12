#include "SceneManager.h"
#include "ResourceManager.h"
#include "Application.h"
#include "RenderMeshComponent.h"
#include "Input.h"


using namespace std;

SceneManager::SceneManager() {

}

SceneManager::~SceneManager() {

}

bool SceneManager::Start() {
    return true;
    fs = Application::GetInstance().fileSystem.get();
}

bool SceneManager::Update(float dt) {


    currentScene->Update(static_cast<float>(dt));

    bool ctrl = Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LCTRL) || Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_RCTRL);
    bool s = Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_S);
    bool l = Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_L);
    std::string scenesPath = std::string(Paths::SCENE_ASSETS_DIR) + "/SampleScene.vroomscene";

    if (ctrl && s) GetActiveScene()->SaveScene(scenesPath);

    if (ctrl && l) GetActiveScene()->LoadScene(scenesPath);

    return true;
}

bool SceneManager::CleanUp() {

    return true;
}


// In SceneManager.cpp

void SceneManager::LoadDefaultScene() {
    LOG("SceneManager: Loading scenes");

    // Load the default scene
    auto defaultScene = std::make_shared<Scene>("DefaultScene");

    scenes.push_back(defaultScene);
    SetActiveScene("DefaultScene");

    std::string defaultSceneDir = std::string(Paths::SCENE_ASSETS_DIR) + "/DefaultScene.vroomscene";
    std::string sampleSceneDir = std::string(Paths::SCENE_ASSETS_DIR) + "/SampleScene.vroomscene";

    if (!fs->Exists(Paths::LIB_DIR)
        || (fs->IsFolderEmpty(Paths::MESH_LIB_DIR) && fs->IsFolderEmpty(Paths::TEXTURE_LIB_DIR))
        || !fs->Exists(defaultSceneDir.c_str())) {

        LOG("Importing Default Scene from scratch");

        GetActiveScene()->ImportModel("../Assets/Models/Street/Street environment_V01.FBX");
        GetActiveScene()->SaveScene(defaultSceneDir);
        GetActiveScene()->SaveScene(sampleSceneDir);

    }
    else {
        LOG("Loading Scene from Scene Assets file");
        defaultScene->LoadScene(defaultSceneDir);
    }

    LOG("Successfully created DefaultScene");

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




