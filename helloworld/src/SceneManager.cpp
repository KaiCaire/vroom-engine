#include "SceneManager.h"
#include "ResourceManager.h"
#include "Application.h"
#include "RenderMeshComponent.h"
#include "GUIManager.h"
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
    std::string scenesPath = std::string(Paths::SCENE_ASSETS_DIR) + "/" + GetActiveScene()->GetName() + ".vroomscene";


    if (ctrl && s) 
        GetActiveScene()->SaveScene(scenesPath);

    if (ctrl && l) 
        GetActiveScene()->LoadScene(scenesPath);

    return true;
}

bool SceneManager::CleanUp() {

    return true;
}

 
// In SceneManager.cpp

void SceneManager::LoadDefaultScene() {
    LOG("SceneManager: Loading scenes");

    auto defaultScene = std::make_shared<Scene>("DefaultScene");
    scenes.push_back(defaultScene);
    SetActiveScene("DefaultScene");

    std::string defaultScenePath = std::string(Paths::SCENE_ASSETS_DIR) + "/" + GetActiveScene()->GetName() + ".vroomscene";

   
    if (!fs->Exists(defaultScenePath.c_str())) {
        LOG("DefaultScene file missing. Importing street environment...");
        GetActiveScene()->ImportModel("../Assets/Models/Street/Street environment_V01.FBX");
        GetActiveScene()->SaveScene(defaultScenePath);
    }
    else {
        LOG("Loading Scene from persistent file: %s", defaultScenePath.c_str());
        defaultScene->LoadScene(defaultScenePath);
    }

    LOG("Successfully initialized DefaultScene");
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


    currentScene->AddGameObject(newGameObject);

    int gameObjectsSize = currentScene->GetAllGameObjects().size();

    LOG("Empty GameObject '%s' created successfully with UUID %llu (Total GameObjects: %d)", name.c_str(), newGameObject->GetUUID(),  gameObjectsSize);

    return newGameObject;
}

std::shared_ptr<GameObject> SceneManager::CreateCameraObject(const std::string& name) {
    if (!currentScene) return nullptr;

    //create the base GameObject
    auto cameraGO = CreateEmptyGameObject(name, currentScene->GetRoot());

    cameraGO->AddComponent(ComponentType::CAMERA);

    LOG("Created Camera GameObject '%s'", name.c_str());
    return cameraGO;
}

void ManualResourceCleanup(const std::shared_ptr<GameObject>& go) {
    
    if (auto rmc = std::dynamic_pointer_cast<RenderMeshComponent>(go->GetComponent(ComponentType::MESH_RENDERER))) {
        // The RMC destructor should do this, but we force it here for synchronous cleanup
        if (auto mesh = rmc->GetMesh()) {
            mesh->RemoveReference();
            rmc->SetMesh(nullptr);
            LOG("  - Mesh ref removed for '%s'", go->GetName().c_str());
        }
    }

   
    if (auto mc = std::dynamic_pointer_cast<MaterialComponent>(go->GetComponent(ComponentType::MATERIAL))) {

        if (auto tex = mc->GetDiffuseMap()) {
            LOG("Texture Reference Count before deletion: %d", tex->GetReferenceCount());
            tex->RemoveReference();
            LOG("Reference to texture %s removed, reference count is now: %d", tex->GetAssetFilePath(), tex->GetReferenceCount());
        }
            
            
        if (auto tex = mc->GetNormalMap()) 
            tex->RemoveReference();
        if (auto tex = mc->GetMetallicMap()) 
            tex->RemoveReference();
        if (auto tex = mc->GetRoughnessMap()) 
            tex->RemoveReference();
        if (auto tex = mc->GetAOMap()) 
            tex->RemoveReference();

        
        mc->SetDiffuseMap(nullptr); 

        LOG("  - Material refs removed for '%s'", go->GetName().c_str());
    }
}

void SceneManager::DestroyGameObject(std::shared_ptr<GameObject> gameObject) {
    if (!gameObject) {
        LOG("WARNING: Attempted to destroy null GameObject");
        return;
    }

    LOG("Destroying GameObject '%s'", gameObject->GetName().c_str());

    ManualResourceCleanup(gameObject);
    // Marcar este GameObject
    gameObject->MarkForDestroy();

    // Lambda recursiva sin std::function
    auto markChildren = [&](auto&& self, std::shared_ptr<GameObject> go) -> void {
        for (auto& child : go->GetChildren()) {
            if (child && !child->IsMarkedForDestroy()) {
                ManualResourceCleanup(child);
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
    if (!resourceManager) return nullptr;

    //get mesh (already has UUID_CUBE assigned from InitializePrimitives)
    std::shared_ptr<ResourceMesh> cubeMesh = resourceManager->GetPrimitiveMesh(PrimitiveType::CUBE);
    if (!cubeMesh) {
        LOG("ERROR: Primitive Cube mesh could not be found or initialized.");
        return nullptr;
    }

    //Create GameObject
    auto cubeGO = std::make_shared<GameObject>("Cube");
    cubeGO->AddComponent(ComponentType::TRANSFORM);
    cubeGO->SetParent(currentScene->GetRoot());

    //setup mesh renderer
    auto renderComp = std::dynamic_pointer_cast<RenderMeshComponent>(cubeGO->AddComponent(ComponentType::MESH_RENDERER));
    if (renderComp) {
        renderComp->SetMesh(cubeMesh);

      
        renderComp->SetMeshUUID(cubeMesh->GetUUID());

        LOG("Cube Mesh linked with Persistent UUID: %llu", cubeMesh->GetUUID());
    }

    // setup material
    auto matComp = std::dynamic_pointer_cast<MaterialComponent>(cubeGO->AddComponent(ComponentType::MATERIAL));
    if (matComp) {
        std::string checkersDir = resourceManager->checkersTexDir;
        auto tex = std::dynamic_pointer_cast<ResourceTexture>(resourceManager->RequestResource(checkersDir));
        if (tex) {
            matComp->SetDiffuseMap(tex);
        }
    }

    //add to scene 
    currentScene->AddGameObject(cubeGO);

    LOG("GameObject 'Cube' successfully added to scene.");
    return cubeGO;
}


