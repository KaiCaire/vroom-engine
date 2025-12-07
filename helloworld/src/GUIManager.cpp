#include "Application.h"
#include "Window.h"
#include "GUIManager.h"
#include "GUIElement.h"
#include "OpenGL.h"
#include "Log.h"
#include "FileSystem.h"
#include <SDL3/SDL_opengl.h>
#include <vector>
#include "imgui.h"
#include "imgui_internal.h" 
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include "Camera.h"
#include "SceneManager.h"
#include "Importer.h"
#include "TextureImporter.h"
#include "Scene.h"
#include "MaterialComponent.h"
#include "RenderMeshComponent.h"

GUIManager::GUIManager() : Module(), AdditionalElements(ElementType::Additional, this), Menu(ElementType::MenuBar, this), selectedObject(nullptr)
{
	name = "guiManager";
}

//Destructor
GUIManager::~GUIManager() 
{
}

//Called before manager is available
bool GUIManager::Awake()
{
	LOG("Set up ImGui context");
	bool ret = true;

	//prepare ui elements
	WindowElements = LoadElements();
	//AdditionalElements = GUIElement(ElementType::Additional);

	//Setup version
	const char* glsl_version{ "#version 140" };
	
	//Setup imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO();
	io->IniFilename = nullptr;

	io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	//Setup Backends
	ImGui_ImplSDL3_InitForOpenGL(Application::GetInstance().window.get()->GetWindow(), Application::GetInstance().window.get()->GetContext());
	ImGui_ImplOpenGL3_Init(glsl_version);
	return ret;
}

std::vector<GUIElement> GUIManager::LoadElements() 
{
	std::vector<GUIElement> elements;

	elements.push_back(GUIElement(ElementType::Console, this));
	elements.push_back(GUIElement(ElementType::Config, this));
	elements.push_back(GUIElement(ElementType::Hierarchy, this));
	elements.push_back(GUIElement(ElementType::Inspector, this));
	elements.push_back(GUIElement(ElementType::AssetsViewer, this));

	return elements;
}

//Called before first frame
bool GUIManager::Start()
{
	return true;
}

//Called for each loop
bool GUIManager::PreUpdate()
{
	return true;
}

//void GUIManager::AddGameObject(Model* obj) {
//	for (auto o : obj->gameObjects) {
//		sceneObjects.push_back(o);
//	}
//}

bool GUIManager::Update(float dt)
{
	//initialize game object list

	//if (!objectsInitialized) {
	//	for (auto m : Application::GetInstance().openGL.get()->modelObjects) {
	//		sceneObjects.push_back(m->rootGameObject);
	//	}
	//	objectsInitialized = true;
	//}

	
	//Start the ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	//setup docking space
	ImGuiWindowFlags dockingSpaceFlags = ImGuiWindowFlags_MenuBar |
		                                 ImGuiWindowFlags_NoDocking;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	dockingSpaceFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 
		               ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	ImGui::Begin("DockSpace", nullptr, dockingSpaceFlags);
	ImGui::PopStyleVar(2);

	if (!dockInitialized) InitDock();

	//menu setup
	Menu.ElementSetUp();

	//handle popups
	if (showAboutPopup)
	{
		ImGui::OpenPopup("About");
		showAboutPopup = false; 
	}

	//setup popups
	AdditionalElements.ElementSetUp();

	ImGuiID dockingSpaceID = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockingSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	//main element setup loop
	for (GUIElement e : WindowElements) {
		e.ElementSetUp();
	}

	//process resource deletion
	if (!resourceDeleteQueue.empty()) {
		for (VroomUUID uuid : resourceDeleteQueue) {
			Application::GetInstance().resourceManager->DeleteResource(uuid);
		}
		resourceDeleteQueue.clear();
	}
	
	Application::GetInstance().sceneManager.get()->GetActiveScene()->CleanUpDestroyedObjects();

	return true;
}


// CHECKER TEXTURE HANDLING
void GUIManager::ShowCheckerTexture(std::shared_ptr<GameObject> go) {
	if (!go) return;

	auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(
		go->GetComponent(ComponentType::MATERIAL)
	);

	auto renderComp = std::dynamic_pointer_cast<RenderMeshComponent>(
		go->GetComponent(ComponentType::MESH_RENDERER)
	); 
	auto mesh = renderComp ? renderComp->GetMesh() : nullptr;

	if (!materialComp || !renderComp) {
		LOG("GameObject '%s' has no Material or RenderMesh Component", go->GetName().c_str());
		return;
	}

	// Save the current texture (if not already saved)
	if (originalTextures.find(go) == originalTextures.end()) {
		auto currentTex = materialComp->GetDiffuseMap();
		if (currentTex) {
			originalTextures[go] = currentTex;
			LOG("Saved original texture for '%s' (UUID: %llu)",
				go->GetName().c_str(), currentTex->GetUUID());
		}
	}

	// Load checker texture
	std::string checkerPath = Application::GetInstance().importer.get()->defaultTexDir;
	auto checkerTex = std::dynamic_pointer_cast<ResourceTexture>(Application::GetInstance().resourceManager.get()->RequestResource(checkerPath.c_str()));
	/*auto checkerTex = Application::GetInstance().importer.get()->textureImporter->Import(checkerPath);*/

	if (checkerTex) {
		materialComp->SetDiffuseMap(checkerTex);  // Store UUID, not pointer

		if (mesh->textures.empty()) {
			mesh->textures.push_back(checkerTex); 
		}
		else {
			mesh->textures[0] = checkerTex; 
		}
		LOG("Applied checker texture to '%s'", go->GetName().c_str());
	}
	else {
		LOG("ERROR: Failed to load checker texture");
	}
}

void GUIManager::RestoreOGTexture(std::shared_ptr<GameObject> go) {
	if (!go) return;

	auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(go->GetComponent(ComponentType::MATERIAL));

	auto renderComp = std::dynamic_pointer_cast<RenderMeshComponent>(go->GetComponent(ComponentType::MESH_RENDERER));
	auto mesh = renderComp ? renderComp->GetMesh() : nullptr;

	if (!materialComp || !mesh) return;

	// Find saved texture
	auto it = originalTextures.find(go);
	if (it != originalTextures.end()) {
		auto originalTex = it->second;
		
		if (originalTex) {

			if (!originalTex->isLoadedToGPU) {
				LOG("WARNING: Original texture not loaded to GPU, loading now...");
				originalTex->LoadToGPU();
			}
			

			if (originalTex->isLoadedToGPU) {
				materialComp->SetDiffuseMap(originalTex);  // Restore by UUID
				if (!mesh->textures.empty()) {
					mesh->textures[0] = originalTex;
				}
				LOG("Restored original texture for '%s' (UUID: %llu)", go->GetName().c_str(), originalTex->GetUUID());
			}
			
		}

		// Remove from map
		originalTextures.erase(it);
	}
	else {
		LOG("No saved texture found for '%s'", go->GetName().c_str());
	}
}

//void GUIManager::RefreshGUIHierarchy() {
//	sceneObjects.clear();
//
//	auto sceneManager = Application::GetInstance().sceneManager.get();
//	auto scene = sceneManager->GetActiveScene();
//
//	if (scene) {
//		// Get all GameObjects from the active scene
//		sceneObjects = scene->GetAllGameObjects();
//	}
//}

void GUIManager::AddToDeleteQueue(const std::shared_ptr<GameObject>& obj) {
	if (!obj) {
		LOG("Attempting to delete null object.");
		return;
	}

	// Get the active scene
	auto sceneManager = Application::GetInstance().sceneManager.get();
	auto scene = sceneManager->GetActiveScene();

	if (!scene) {
		LOG("ERROR: No active scene found. Cannot delete object '%s'", obj->GetName().c_str());
		return;
	}

	// Queue object for deletion in the scene
	sceneManager->DestroyGameObject(obj);
	LOG("Queued object '%s' for deletion.", obj->GetName().c_str());

	//std::find returns:
	//Iterator to the found element if it exists
	//sceneObjects.end() if NOT found

	//auto it = std::find(sceneObjects.begin(), sceneObjects.end(), obj);
	//if (it != sceneObjects.end()) {
	//	sceneObjects.erase(it);
	//}

	// Clear selection if deleting the selected object
	if (selectedObject == obj) {
		selectedObject = nullptr;
	}
}

void GUIManager::InitDock() {
	//clear any existing layout
	ImGuiID dockspaceID = ImGui::GetID("DockSpace");
	ImGui::DockBuilderRemoveNode(dockspaceID);
	ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetMainViewport()->Size);

	//split dock
	ImGuiID dockMainID = dockspaceID;
	ImGuiID dockBottomID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Down, 0.25f, nullptr, &dockMainID);
	ImGuiID dockLeftID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, 0.25f, nullptr, &dockMainID);
	ImGuiID dockRightID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, 0.35f, nullptr, &dockMainID);

	//assign windows to dock spaces
	ImGui::DockBuilderDockWindow("Console", dockBottomID);
	ImGui::DockBuilderDockWindow("Hierarchy", dockLeftID);
	ImGui::DockBuilderDockWindow("Inspector", dockRightID);

	ImGui::DockBuilderFinish(dockspaceID);
	//only do this once
	dockInitialized = true;
}

void GUIManager::ProcessEvents(SDL_Event event) {
	ImGui_ImplSDL3_ProcessEvent(&event);
}

bool GUIManager::PostUpdate()
{
	//Render
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


	// Update and render additional platform windows 
	if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}

	return true;
}

//Called before quit
bool GUIManager::CleanUp()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();

	ImGui::DestroyContext();

	return true;
}


//Model* GUIManager::FindGameObjectModel(const std::shared_ptr<GameObject>& obj) {
//	//get all models
//	auto& models = Application::GetInstance().openGL.get()->modelObjects;
//
//	//search for the model that contains a specific game object
//	for (auto& model : models)
//	{
//		for (auto& o : model->gameObjects)
//		{
//			if (o == obj) return model;
//		}
//	}
//	//if not found return nullptr
//	return nullptr;
//}