#include "Application.h"
#include "Window.h"
#include "GUIManager.h"
#include "GUIElement.h"
#include "Log.h"
#include "FileSystem.h"
#include <SDL3/SDL_opengl.h>
#include <vector>
#include "imgui.h"
#include "imgui_internal.h" 
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

GUIManager::GUIManager() : Module(), AdditionalElements(ElementType::Additional), Menu(ElementType::MenuBar)
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

	elements.push_back(GUIElement(ElementType::Console));
	elements.push_back(GUIElement(ElementType::Config));
	elements.push_back(GUIElement(ElementType::Hierarchy));
	elements.push_back(GUIElement(ElementType::Inspector));

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

bool GUIManager::Update(float dt)
{
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


	//test window
	/*ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(200, 200), ImGuiCond_FirstUseEver);
	ImGui::Begin("Holis", nullptr);

	ImGui::Text("Soy gui");
	ImGui::End();*/

	return true;
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

	//assign windows to dock spaces
	ImGui::DockBuilderDockWindow("Console", dockBottomID);

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