#include "Application.h"
#include "Window.h"
#include "GUIElement.h"
#include "Log.h"
#include "FileSystem.h"
#include "GUIManager.h"
#include "SystemInfo.h"
#include "OpenGL.h"
#include "Model.h"

#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include "Textures.h"
#include "Window.h"

#include <SDL3/SDL_opengl.h>
#include <glm/glm.hpp>
#include <assimp/version.h>
#include <fmt/core.h>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>

GUIElement::GUIElement(ElementType t, GUIManager* m)
{
	type = t;
	manager = m;
}

GUIElement:: ~GUIElement() 
{

}

void GUIElement::ElementSetUp() 
{
	switch (type) {
	case ElementType::Additional:
		//additional handles popup windows
		AboutSetUp();
		break;
	case ElementType::MenuBar:
		MenuBarSetUp();
		break;
	case ElementType::Console:
		if(Application::GetInstance().guiManager.get()->showConsole) ConsoleSetUp(&Application::GetInstance().guiManager.get()->showConsole);
		break;
	case ElementType::Config:
		if (Application::GetInstance().guiManager.get()->showConfig) ConfigSetUp(&Application::GetInstance().guiManager.get()->showConfig);
		break;
	case ElementType::Hierarchy:
		if (Application::GetInstance().guiManager.get()->showHierarchy) HierarchySetUp(&Application::GetInstance().guiManager.get()->showHierarchy);
		break;
	case ElementType::Inspector:
		if (Application::GetInstance().guiManager.get()->showInspector) InspectorSetUp(&Application::GetInstance().guiManager.get()->showInspector);
		break;
	default:
		LOG("No GUIType detected.");
		break;
	}
}

//type set ups
void GUIElement::MenuBarSetUp() 
{
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit")) {
				//handle exit
				Application::GetInstance().requestExit = true;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			//handle view
			if (ImGui::MenuItem("Console", nullptr, Application::GetInstance().guiManager.get()->showConsole)) {
				bool set = !Application::GetInstance().guiManager.get()->showConsole;
				Application::GetInstance().guiManager.get()->showConsole = set;
			}
			if (ImGui::MenuItem("Configuration", nullptr, Application::GetInstance().guiManager.get()->showConfig)) {
				bool set = !Application::GetInstance().guiManager.get()->showConfig;
				Application::GetInstance().guiManager.get()->showConfig = set;
			}
			if (ImGui::MenuItem("Hierarchy", nullptr, Application::GetInstance().guiManager.get()->showHierarchy)) {
				bool set = !Application::GetInstance().guiManager.get()->showHierarchy;
				Application::GetInstance().guiManager.get()->showHierarchy = set;
			}
			if (ImGui::MenuItem("Inspector", nullptr, Application::GetInstance().guiManager.get()->showInspector)) {
				bool set = !Application::GetInstance().guiManager.get()->showInspector;
				Application::GetInstance().guiManager.get()->showInspector = set;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("Documentation")) {
				//handle documentation
				SDL_OpenURL("https://github.com/KaiCaire/vroom-engine?tab=readme-ov-file#readme");

			}
			if (ImGui::MenuItem("Report a Bug")) {
				//handle report 
				SDL_OpenURL("https://github.com/KaiCaire/vroom-engine/issues");
			}
			if (ImGui::MenuItem("Latest Release")) {
				//handle release
				SDL_OpenURL("https://github.com/KaiCaire/vroom-engine/releases");
			}
			if (ImGui::MenuItem("About")) {
				//handle about window
				Application::GetInstance().guiManager.get()->showAboutPopup = true;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void GUIElement::AboutSetUp() {
	if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		
		//Text
		ImGui::Text("VroomEngine v.1");
		ImGui::Separator();
		ImGui::Text("Developed by:");
		ImGui::BulletText("Ivan Alvarez");
		ImGui::BulletText("Kai Caire");
		ImGui::BulletText("Lara Guevara");
		ImGui::BulletText("Bernat Loza");
		ImGui::Separator();
		ImGui::Text("Developed using:");
		ImGui::BulletText("assimp");
		ImGui::BulletText("fmt");
		ImGui::BulletText("glad");
		ImGui::BulletText("glm");
		ImGui::BulletText("imgui");
		ImGui::BulletText("OpenGL");
		ImGui::BulletText("sdl3");
		ImGui::NewLine();
		ImGui::Text("MIT License Copyright(c) 2025");

		ImGui::Spacing();
		ImGui::Separator();

		//Close button
		if (ImGui::Button("Close Window", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void GUIElement::ConsoleSetUp(bool* show) {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	//initialize for scroll button
	bool scrollToBottom = false;

	//initial states
	ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

	//check if we should show it
	if (!ImGui::Begin("Console", show, window_flags))
	{
		//if not -> end here
		ImGui::End();
		return;
	}

	//Button controls
	//Clear logs
	if (ImGui::Button("Clear")) ClearLogs();
	ImGui::SameLine();

	//Scroll to the bottom
	if (ImGui::Button("Go to Bottom")) scrollToBottom = true;
	ImGui::Separator();

	//log messages area
	//create area for the console
	ImGui::BeginChild("ScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	auto logs = GetLogBuffer();
	//show log messages
	for (const auto& line : logs) ImGui::TextUnformatted(line.c_str());

	if (scrollToBottom) {
		ImGui::SetScrollHereY(1.0f);
		scrollToBottom = false;
	}

	ImGui::EndChild();
	ImGui::End();
}

void GUIElement::ConfigSetUp(bool* show) {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	//initial states
	ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

	//check if we should show it
	if (!ImGui::Begin("Configuration", show, window_flags))
	{
		//if not -> end here
		ImGui::End();
		return;
	}

	//show fps
	//ImGui::Text("FPS: %d", Application::GetInstance().GetFPS());
	ImGui::Text("FPS: %.1f (%.1f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Separator();

	//variable config
	ImGui::Text("Variables:");
	//window full screen
	//check if window is fullscreen
	bool fullscreen = Application::GetInstance().window.get()->isFullscreen;
	if (ImGui::Checkbox("Fullscreen", &fullscreen)) {
		Application::GetInstance().window.get()->SetFullScreen(fullscreen);
	}
	//window resolution
	if (!fullscreen) {
		//get resolutions and current resolution from window
		vector<glm::vec2> options = Application::GetInstance().window.get()->resolutions;
		glm::vec2 current = Application::GetInstance().window.get()->currentRes;

		//find index of current resolution
		int index = 0;

		//setup dropdown menu
		ImGui::Text("Resolution");
		ImGui::SameLine();
		if (ImGui::BeginCombo("##Resolution", (std::to_string((int)current.x) + "x" + std::to_string((int)current.y)).c_str())) {
			for (int i = 0; i < options.size(); i++) {
				//find selected resoltion
				bool selected = (current == options[i]);
				
				//create option label
				std::string label = (std::to_string((int)options[i].x) + "x" + std::to_string((int)options[i].y));
				if (ImGui::Selectable(label.c_str(), selected)) {
					//apply resolution
					current = options[i];
					Application::GetInstance().window.get()->SetWindowSize(current);
				}

				if (selected) ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
	}
	ImGui::Separator();

	//hardware and memory consuption
	ImGui::Text("Hardware and Memory Information:");
	ImGui::BulletText("Memory Consumption: %.2f MB", GetMemoryUsageMB());
	ImGui::BulletText("CPU Cores: %u", GetCPUCoreCount());
	ImGui::Separator();

	//software versions 
	ImGui::Text("Software Versions:");
	ImGui::BulletText("SDL3: %d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION);

	// OpenGL
	const char* glVer = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	ImGui::BulletText("OpenGL: %s", glVer ? glVer : "Unknown");
	
	ImGui::BulletText("ImGui: %s", IMGUI_VERSION);
	ImGui::BulletText("GLM: %d.%d.%d", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_PATCH);
	ImGui::BulletText("Assimp: %d.%d.%d", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
	ImGui::BulletText("fmt: %d.%d.%d", FMT_VERSION / 10000, (FMT_VERSION / 100) % 100, FMT_VERSION % 100);
	
	ImGui::End();
}

void GUIElement::HierarchySetUp(bool* show)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	//initial states
	ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

	//check if we should show it
	if (!ImGui::Begin("Hierarchy", show, window_flags))
	{
		//if not -> end here
		ImGui::End();
		return;
	}

	//create objects (minim cube)
	if (ImGui::BeginMenu("Create...")) {
		if (ImGui::MenuItem("Empty")) {
			//Create empty function

		}
		if (ImGui::MenuItem("Cube")) {
			//Create cube function

		}
		ImGui::EndMenu();
	}

	ImGui::Separator();

	//game objects
	//get root level objects
	for (auto& obj : manager->sceneObjects)
	{
		//check for game objects with no parent
		if (obj->IsActive() && !obj->GetParent()) DrawNode(obj, manager->selectedObject);
	}

	ImGui::End();
}

void GUIElement::DrawNode(const std::shared_ptr<GameObject>& obj, std::shared_ptr<GameObject>& selected) {
	//setup tree structure (add arrows to expandable objects, make it so they show as selected)
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
		(obj == selected ? ImGuiTreeNodeFlags_Selected : 0) |
		(obj->GetChildren().empty() ? ImGuiTreeNodeFlags_Leaf : 0);

	//create node and check if opened
	bool opened = ImGui::TreeNodeEx((void*)obj.get(), flags, "%s", obj->GetName().c_str());

	//check if object has been selected
	if (ImGui::IsItemClicked()) selected = obj;

	//show children 
	if (opened)
	{
		for (auto& child : obj->GetChildren()) DrawNode(child, selected);
		ImGui::TreePop();
	}
}

void GUIElement::InspectorSetUp(bool* show)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	//initial states
	ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

	//check if we should show it
	if (!ImGui::Begin("Inspector", show, window_flags))
	{
		//if not -> end here
		ImGui::End();
		return;
	}

	//check if a game object is selected
	auto selected = manager->selectedObject;

	if (selected) {
		//show game object name
		char buffer[128];
		strcpy(buffer, selected->GetName().c_str());
		if (ImGui::InputText("##hidden", buffer, sizeof(buffer))) selected->SetName(buffer);

	    //transform
		//get transform component
		auto transform = std::dynamic_pointer_cast<TransformComponent>(selected->GetComponent(ComponentType::TRANSFORM));
		if (transform) {
			//check if header is open
			if (ImGui::CollapsingHeader("Transform")) {
				//get values
				glm::vec3 pos = transform.get()->GetWorldPosition();
				glm::quat rot = glm::degrees(glm::eulerAngles(transform->GetWorldRotation()));
				glm::vec3 scale = transform.get()->GetWorldScale();

				//display values
				ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
				ImGui::Text("Rotation: %.2f, %.2f, %.2f", rot.x, rot.y, rot.z);
				ImGui::Text("Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
			}
		}
		
		//mesh
		//get mesh component
		auto meshComponent = std::dynamic_pointer_cast<RenderMeshComponent>(selected->GetComponent(ComponentType::MESH_RENDERER));
		//get texture for next step
		vector<Texture> textureComponent;

		if (meshComponent) {
			std::shared_ptr<Mesh> mesh = meshComponent.get()->GetMesh();
			if(mesh) textureComponent = mesh.get()->textures;

			//check if header is open
			if (ImGui::CollapsingHeader("Mesh")) {
				//get values
				std::shared_ptr<Mesh> mesh = meshComponent.get()->GetMesh();
				vector<Vertex> vert = mesh.get()->vertices;
				vector<unsigned int> ind = mesh.get()->indices;

				//display values
				ImGui::Text("Vertices: %d", vert.size());
				ImGui::Text("Indices: %d", ind.size());

				//show normals 
				//handle button colors (commented until show normal function is created)
				//if (showNormals) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green when active
				//else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));

				if (ImGui::Button("Show Normals")) {
					//handle showing normals

					//handle state
					//showNormals = !showNormals; //commented until show normal function is created
				}

				//activate the button color change (commented until show normal function is created)
				//ImGui::PopStyleColor();
			}

			//texture
			if (ImGui::CollapsingHeader("Texture")) {
				for (auto t : textureComponent) {
					ImGui::Text("Texture %u", t.id);
					ImGui::BulletText("Path: %s", t.path.c_str());
					ImGui::BulletText("Width: %d", t.texW);
					ImGui::BulletText("Height: %d", t.texH);
				}

				//checker texture toggle
				//handle button colors (commented until show checker function is created)
				//if (checkerTex) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green when active
				//else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));

				if (ImGui::Button("Show Checker Texture")) {
					//handle checker texture

					//handle state
					//checkerTex = !checkerTex; //commented until show checker function is created
				}

				//activate the button color change (commented until show checker function is created)
				//ImGui::PopStyleColor();
			}
		}
	}
	else {
		ImGui::Text("No GameObject selected.");
	}

	ImGui::End();
}