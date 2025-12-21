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
	elements.push_back(GUIElement(ElementType::GameViewport, this));
	elements.push_back(GUIElement(ElementType::SceneViewport, this));

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

	dockingSpaceFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 
		               ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("DockSpace", nullptr, dockingSpaceFlags);
	ImGui::PopStyleVar(2);

	if (!dockInitialized) InitDock();

	static bool firstFrameFocus = true;
	if (firstFrameFocus && dockInitialized) {
		ImGui::SetWindowFocus("Scene");
		firstFrameFocus = false;
	}

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

	//process file deletion
	if (!fileDeleteQueue.empty()) {
		for (const std::string& filePath : fileDeleteQueue) {
			//delete the file/folder directly
			Application::GetInstance().fileSystem->DeleteFile(filePath.c_str());
		}
		fileDeleteQueue.clear();
	}
	
	Application::GetInstance().sceneManager.get()->GetActiveScene()->CleanUpDestroyedObjects();

	return true;
}

VroomUUID GUIManager::GetCheckerTextureUUID() {
	std::string checkerPath = Application::GetInstance().resourceManager.get()->checkersTexDir;
	std::string checkerMetaPath = checkerPath + ".meta";

	// Attempt to load the meta file to get the UUID
	if (Application::GetInstance().fileSystem->Exists(checkerMetaPath.c_str())) {
		return Application::GetInstance().fileSystem->GetUUIDFromMeta(checkerMetaPath.c_str());
	}
	return 0; // Return 0 if the UUID cannot be determined
}

// src/GUIManager.cpp (Actual implementation)




void GUIManager::ShowCheckerTexture(std::shared_ptr<GameObject> go) {
	

	if (!go) return;
	auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(go->GetComponent(ComponentType::MATERIAL));
	if (!materialComp) return;



	// 1. Save OG
	if (originalTextures.find(go) == originalTextures.end()) {
		if (materialComp->GetDiffuseMap()) {
			originalTextures[go] = materialComp->GetDiffuseMap();
		}
		
	}

	// 2. Request Checker
	std::string checkerPath = Application::GetInstance().resourceManager.get()->checkersTexDir;
	auto checkerTex = std::static_pointer_cast<ResourceTexture>(Application::GetInstance().resourceManager->RequestResource(checkerPath.c_str()));

	if (checkerTex) {
		// ONLY change the component, NOT the mesh textures
		materialComp->SetDiffuseMap(checkerTex);
		/*if (originalTextures[go]) {
			originalTextures[go]->RemoveReference();
		}*/
		//Set diffusemap already handles derreferencing the old texture!
	}
}

void GUIManager::RestoreOGTexture(std::shared_ptr<GameObject> go) {
	if (!go) return;

	// 1. Get the Material Component
	auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(go->GetComponent(ComponentType::MATERIAL));
	if (!materialComp) {
		LOG("GameObject '%s' has no Material Component to restore.", go->GetName().c_str());
		return;
	}

	// 2. Find the saved original texture for THIS specific GameObject
	auto it = originalTextures.find(go);
	if (it != originalTextures.end()) {
		auto originalTex = it->second;

		// ... (omitted existing LoadBin/LoadToGPU logic for brevity)

		if (originalTex) {
			if (materialComp->GetDiffuseMap()) {
				/*materialComp->GetDiffuseMap()->RemoveReference();*/
				//Set diffusemap already handles derreferencing the old texture!
			}
			
			materialComp->SetDiffuseMap(originalTex);
			LOG("Restored original texture for '%s' (UUID: %llu)", go->GetName().c_str(), originalTex->GetUUID());
			
		}

		// 3. Remove from map (MUST HAPPEN)
		originalTextures.erase(it);
	}
	else if (materialComp->GetDiffuseMap() && materialComp->GetDiffuseMap()->GetUUID() == GetCheckerTextureUUID()) {

		// Assign nullptr to clear the map. The user can now drag/drop a new texture.
		materialComp->SetDiffuseMap(nullptr);
		LOG("WARNING: Cleared material on '%s' because no original texture was found to restore.", go->GetName().c_str());
	}
	else {
		LOG("No saved texture found for '%s'. It might not have had one or was already restored.",
			go->GetName().c_str());
	}
}

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



	
	LOG("Queued object '%s' for deletion.", obj->GetName().c_str());

	if (selectedObject == obj) {
		selectedObject = nullptr;
	}

	// Queue object for deletion in the scene
	sceneManager->DestroyGameObject(obj);
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
	ImGuiID dockRightDownID = ImGui::DockBuilderSplitNode(dockRightID, ImGuiDir_Down, 0.50f, nullptr, &dockRightID);

	//assign windows to dock spaces
	ImGui::DockBuilderDockWindow("Console", dockBottomID);
	ImGui::DockBuilderDockWindow("Hierarchy", dockLeftID);
	ImGui::DockBuilderDockWindow("Inspector", dockRightID);
	ImGui::DockBuilderDockWindow("Assets Viewer", dockRightDownID);

	//dock scene and game in the middle
	ImGui::DockBuilderDockWindow("Scene", dockMainID);
	ImGui::DockBuilderDockWindow("Game", dockMainID);

	ImGui::DockBuilderFinish(dockspaceID);

	//make sure you start on scene
	ImGui::SetWindowFocus("Scene");
	//only do this once
	dockInitialized = true;
}

void GUIManager::ProcessEvents(SDL_Event event) {
	ImGui_ImplSDL3_ProcessEvent(&event);
}

void GUIManager::HandleExternalFileDrop(const std::string& sourceOSPath) {
	FileSystem* fs = Application::GetInstance().fileSystem.get();

	std::string sourcePath = fs->NormalizePath(sourceOSPath.c_str());
	std::string fileName = Application::GetInstance().fileSystem->GetFileFromPath(sourceOSPath.c_str());
	std::string targetDir;

	ResourceType resType = Application::GetInstance().resourceManager.get()->DetermineResourceType(sourcePath);
	
	if (resType == ResourceType::SCENE) {
		targetDir = std::string(Paths::MODEL_ASSETS_DIR);
	}
	else if (resType == ResourceType::TEXTURE) {
		targetDir = std::string(Paths::TEXTURE_ASSETS_DIR);
	}
	else {
		LOG("Unrecognized file type, copying to general Assets/ folder");
		targetDir = std::string(Paths::ASSETS_DIR);
	}
	
	//build final path
	
	std::string targetPath = targetDir + "/" + fileName;



	Application::GetInstance().fileSystem->CreateDir(targetDir.c_str());

	if (fs->CustomCopyFile(sourcePath.c_str(), targetPath.c_str())) {
		/*std::string assetRelativePath = Paths::ASSETS_DIR + fileName; */
		Application::GetInstance().resourceManager->RequestResource(targetPath, sourcePath);
		LOG("External file '%s' successfully copied to Assets/ and imported.", fileName.c_str());
	}
	else {
		LOG("ERROR: Failed to copy file %s to Assets/ folder.", fileName.c_str());
	}
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

