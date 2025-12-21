#include "Application.h"
#include "Window.h"
#include "GUIElement.h"
#include "Log.h"
#include "FileSystem.h"
#include "GUIManager.h"
#include "SystemInfo.h"
#include "OpenGL.h"
#include "ModelImporter.h"
#include "SceneManager.h"
#include "Input.h"
#include "Camera.h"
#include "SceneManager.h"

#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include "ResourceTexture.h"
#include "Render.h"
#include "TextureImporter.h"
#include "Importer.h"

#include <SDL3/SDL_opengl.h>
#include <glm/glm.hpp>
#include <assimp/version.h>
#include <fmt/core.h>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ImGuizmo.h"
#include "imgui_internal.h"

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
	case ElementType::AssetsViewer:
		if (Application::GetInstance().guiManager.get()->showAssetsViewer) AssetsViewerSetUp(&Application::GetInstance().guiManager.get()->showAssetsViewer);
		break;
	case ElementType::SceneViewport:
		if (Application::GetInstance().guiManager.get()->showSceneViewport) SceneViewportSetUp(&Application::GetInstance().guiManager.get()->showSceneViewport);
		break;
	case ElementType::GameViewport:
		if (Application::GetInstance().guiManager.get()->showGameViewport) GameViewportSetUp(&Application::GetInstance().guiManager.get()->showGameViewport);
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
			auto activeScene = Application::GetInstance().sceneManager.get()->GetActiveScene();
			std::string scenesPath = std::string(Paths::SCENE_ASSETS_DIR) + "/SampleScene.vroomscene";
			if (ImGui::MenuItem("Save Scene")) {
				
				activeScene->SaveScene(scenesPath);
			}
			if (ImGui::MenuItem("Load Scene")) {
				
				activeScene->LoadScene(scenesPath);
			}
			if (ImGui::MenuItem("Al carrer")) {
				//handle exit
				Application::GetInstance().requestExit = true;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			//handle view
			if (ImGui::MenuItem("Assets Viewer", nullptr, Application::GetInstance().guiManager.get()->showAssetsViewer)) {
				bool set = !Application::GetInstance().guiManager.get()->showAssetsViewer;
				Application::GetInstance().guiManager.get()->showAssetsViewer = set;
			}
			if (ImGui::MenuItem("Console", nullptr, Application::GetInstance().guiManager.get()->showConsole)) {
				bool set = !Application::GetInstance().guiManager.get()->showConsole;
				Application::GetInstance().guiManager.get()->showConsole = set;
			}
			if (ImGui::MenuItem("Configuration", nullptr, Application::GetInstance().guiManager.get()->showConfig)) {
				bool set = !Application::GetInstance().guiManager.get()->showConfig;
				Application::GetInstance().guiManager.get()->showConfig = set;
			}
			if (ImGui::MenuItem("Game View", nullptr, Application::GetInstance().guiManager.get()->showGameViewport)) {
				bool set = !Application::GetInstance().guiManager.get()->showGameViewport;
				Application::GetInstance().guiManager.get()->showGameViewport = set;
			}
			if (ImGui::MenuItem("Hierarchy", nullptr, Application::GetInstance().guiManager.get()->showHierarchy)) {
				bool set = !Application::GetInstance().guiManager.get()->showHierarchy;
				Application::GetInstance().guiManager.get()->showHierarchy = set;
			}
			if (ImGui::MenuItem("Inspector", nullptr, Application::GetInstance().guiManager.get()->showInspector)) {
				bool set = !Application::GetInstance().guiManager.get()->showInspector;
				Application::GetInstance().guiManager.get()->showInspector = set;
			}
			if (ImGui::MenuItem("Scene", nullptr, Application::GetInstance().guiManager.get()->showSceneViewport)) {
				bool set = !Application::GetInstance().guiManager.get()->showSceneViewport;
				Application::GetInstance().guiManager.get()->showSceneViewport = set;
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

		float barWidth = ImGui::GetWindowSize().x;
		ImGui::SetCursorPosX(barWidth * 0.5f - 50.0f);

		Application& app = Application::GetInstance();
		if (app.GetState() == EngineEditState::EDITOR) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); 
			if (ImGui::MenuItem(" PLAY ")) {
				app.SetState(EngineEditState::GAME);
				ImGui::SetWindowFocus("Game");
			}
			ImGui::PopStyleColor();
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); 
			if (ImGui::MenuItem(" STOP ")) {
				app.SetState(EngineEditState::EDITOR);
				ImGui::SetWindowFocus("Scene");
			}
			ImGui::PopStyleColor();
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
		ImGui::BulletText("Marti Mach");
		ImGui::Separator();
		ImGui::Text("Developed using:");
		ImGui::BulletText("vcpkg");
		ImGui::BulletText("assimp");
		ImGui::BulletText("stb");
		ImGui::BulletText("glm");
		ImGui::BulletText("imgui");
		ImGui::BulletText("imguizmo");
		ImGui::BulletText("glad (for OpenGL)");
		ImGui::BulletText("sdl-3 & sdl-3.image");
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
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

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
		std::vector<glm::vec2> options = Application::GetInstance().window.get()->resolutions;
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

	ImGui::Text("Debug Visualization:");
	//toggle Z-buffer visualization
	ImGui::Checkbox("Draw Z-Buffer", &Application::GetInstance().openGL.get()->drawZbuffer);
	//toggle AABB drawing
	ImGui::Checkbox("Show Object AABBs", &Application::GetInstance().guiManager.get()->drawAABBs);
	//toggle raycast drawing
	//ImGui::Checkbox("Show Debug Raycast", &Application::GetInstance().guiManager.get()->drawRaycast);
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
			//Create empty 
			auto empty = std::make_shared<GameObject>();
			Application::GetInstance().sceneManager->GetActiveScene()->AddGameObject(empty);
			
		}
		if (ImGui::MenuItem("Camera")) {
			Application::GetInstance().sceneManager->CreateCameraObject("Camera");
		}
		if (ImGui::MenuItem("Cube")) {
			auto defaultCube = Application::GetInstance().sceneManager->CreateCube();
			/*Application::GetInstance().render->AddModel(defaultCube);*/
		}
		ImGui::EndMenu();
	}

	ImGui::Separator();

	auto scene = Application::GetInstance().sceneManager->GetActiveScene();
	if (!scene) {
		ImGui::Text("No active scene");
		ImGui::End();
		return;
	}

	const auto& sceneRoot = scene->GetRoot();

	// Draw hierarchy
	for (auto& child : sceneRoot->GetChildren())
	{
		if (child && child->IsActive()) 
			DrawNode(child, manager->selectedObject);
	}


	//creating invisible object for drag and drop
	//get avaiable size 
	ImVec2 contentSize = ImGui::GetContentRegionAvail();

	//give area an id and create invisible box
	ImGui::InvisibleButton("##HierarchyDropTraget", contentSize);

	//drag and drop target
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
			std::string droppedPath((const char*)payload->Data);
			//handle instantiation
			InstantiateAsset(droppedPath);
		}
		// Accept hierarchy reparent to root
		if (const ImGuiPayload* payload2 = ImGui::AcceptDragDropPayload("HIERARCHY_GO")) {
			if (payload2->DataSize == sizeof(VroomUUID)) {
				VroomUUID srcUUID = 0;
				std::memcpy(&srcUUID, payload2->Data, sizeof(VroomUUID));
				auto scene = Application::GetInstance().sceneManager.get()->GetActiveScene();
				if (scene) {
					auto srcGO = scene->FindGameObjectByUUID(srcUUID);
					if (srcGO) {
						// Reparent to root
						srcGO->SetParent(scene->GetRoot());
						Application::GetInstance().guiManager.get()->selectedObject = srcGO;
						LOG("Reparented '%s' to scene root", srcGO->GetName().c_str());
					}
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::End();
}

void GUIElement::DrawNode(const std::shared_ptr<GameObject>& obj, std::shared_ptr<GameObject>& selected) {
	//make sure obj is not set for deletion
	if (!obj) return;
	if (obj.get()->IsMarkedForDestroy()) return;

	//setup tree structure (add arrows to expandable objects, make it so they show as selected)
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
		(obj == selected ? ImGuiTreeNodeFlags_Selected : 0) |
		(obj->GetChildren().empty() ? ImGuiTreeNodeFlags_Leaf : 0);

	//create node and check if opened
	bool opened = ImGui::TreeNodeEx((void*)obj.get(), flags, "%s", obj->GetName().c_str());

	// Drag source: start dragging this gameobject (payload = its UUID)
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
		VroomUUID payloadUUID = obj->GetUUID();
		ImGui::SetDragDropPayload("HIERARCHY_GO", &payloadUUID, sizeof(VroomUUID));
		ImGui::Text("%s", obj->GetName().c_str());
		ImGui::EndDragDropSource();
	}

	// Accept drop on this node to reparent the dragged object under 'obj'
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GO")) {
			if (payload->DataSize == sizeof(VroomUUID)) {
				VroomUUID srcUUID = 0;
				std::memcpy(&srcUUID, payload->Data, sizeof(VroomUUID));

				// find the source GameObject
				auto scene = Application::GetInstance().sceneManager.get()->GetActiveScene();
				if (scene) {
					auto srcGO = scene->FindGameObjectByUUID(srcUUID);
					if (srcGO && srcGO != obj) {
						// Prevent reparenting to a descendant of srcGO (would create cycle)
						bool invalid = false;
						auto check = obj;
						while (check) {
							if (check == srcGO) { invalid = true; break; }
							check = check->GetParent();
						}

						if (!invalid) {
							// Perform reparent: SetParent handles removal from old parent and adding to new
							srcGO->SetParent(obj);

							// Keep selection on the moved object
							Application::GetInstance().guiManager.get()->selectedObject = srcGO;

							LOG("Reparented '%s' under '%s'", srcGO->GetName().c_str(), obj->GetName().c_str());
						}
						else {
							LOG("Cannot reparent '%s' under its own descendant '%s'", srcGO->GetName().c_str(), obj->GetName().c_str());
						}
					}
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	//check if object has been selected
	if (ImGui::IsItemClicked()) selected = obj;

	//right click to delete object
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Delete")) {
			manager->AddToDeleteQueue(obj);
			if (selected == obj) selected = nullptr;
		}
		ImGui::EndPopup();
	}

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

	if (auto prev = manager->previousSelectedObject.lock()) {
		// Check if the selection has changed AND the previous object was showing the checker
		if (selected != prev) {
			// Assuming IsShowingCheckerTexture returns true if the object is in the originalTextures map.
			if (manager->originalTextures.count(prev) > 0) {
				manager->RestoreOGTexture(prev);
				LOG("Auto-restored texture for previous selection '%s'.", prev->GetName().c_str());
			}
		}
	}

	manager->previousSelectedObject = selected;

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
				//position
				glm::vec3 pos = transform->GetPosition();
				if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f)) {
					transform->SetPosition(pos);
				}

				//rotation
				glm::vec3 rotation = glm::degrees(glm::eulerAngles(transform->GetRotation()));
				if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.1f)) {
					transform->SetRotation(glm::quat(glm::radians(rotation)));
				}

				//scale
				glm::vec3 scale = transform->GetScale();
				if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f)) {
					transform->SetScale(scale);
				}
			}
		}
		
		//mesh
		//get mesh component
		auto meshComponent = std::dynamic_pointer_cast<RenderMeshComponent>(selected->GetComponent(ComponentType::MESH_RENDERER));
		//get texture for next step
		std::vector<std::shared_ptr<ResourceTexture>> textureComponent;

		bool showFaceNormals = manager->drawFaceNormals;
		bool showVertNormals = manager->drawVertNormals;

		
		if (meshComponent) {
			std::shared_ptr<ResourceMesh> mesh = meshComponent.get()->GetMesh();
			if(mesh) textureComponent = mesh.get()->textures;

			//check if header is open
			if (ImGui::CollapsingHeader("Mesh") && mesh) {
				//get values
				/*std::shared_ptr<ResourceMesh> mesh = meshComponent.get()->GetMesh();*/
				std::vector<Vertex> vert = mesh.get()->vertices;
				std::vector<unsigned int> ind = mesh.get()->indices;

				//display values
				ImGui::Text("Vertices: %d", vert.size());
				ImGui::Text("Indices: %d", ind.size());

				//show normals 
				ImGui::Checkbox("Show Vertex Normals", &mesh.get()->drawFaceNormals);
				ImGui::Checkbox("Show Face Normals", &mesh.get()->drawVertNormals);
			}
			//texture
			if (ImGui::CollapsingHeader("Texture") && mesh) {
				//drag and drop of textures
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
						//check if an object is selected
						if (manager->selectedObject) {
							std::string droppedPath((const char*)payload->Data);
							ApplyTextureToSelection(droppedPath);
						}
					}
					ImGui::EndDragDropTarget();
				}

				auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(
					selected->GetComponent(ComponentType::MATERIAL)
				);

				if (materialComp) {
					// Show current texture info
					auto currentTex = materialComp->GetDiffuseMap();
					if (currentTex) {
						ImGui::Text("Current Texture:");
						ImGui::BulletText("UUID: %llu", currentTex->GetUUID());
						ImGui::BulletText("Path: %s", currentTex->GetAssetFilePath());
						ImGui::BulletText("Size: %dx%d", currentTex->texW, currentTex->texH);
					}
					else {
						ImGui::Text("No texture assigned");
					}

					// Checker texture toggle for THIS GameObject only
					bool isShowingChecker = manager->IsShowingCheckerTexture(selected);

					if (ImGui::Checkbox("Show Checker Texture", &isShowingChecker)) {
						if (isShowingChecker) {
							// SWITCH TO CHECKER
							manager->ShowCheckerTexture(selected);
						}
						else {
							// SWITCH BACK TO ORIGINAL
							manager->RestoreOGTexture(selected);
						}
					}
				}
			}

		}
	}
	else {
		ImGui::Text("No GameObject selected.");
	}

	ImGui::End();
}

bool GUIManager::IsShowingCheckerTexture(std::shared_ptr<GameObject> go) {
	auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(go->GetComponent(ComponentType::MATERIAL));
	if (!materialComp) return false;

	std::shared_ptr<ResourceTexture> currentTex = materialComp->GetDiffuseMap();
	if (!currentTex) return false;

	VroomUUID checkerUUID = GetCheckerTextureUUID(); // Use the helper

	return currentTex->GetUUID() == checkerUUID;
}

void GUIElement::AssetsViewerSetUp(bool* show) {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(700, 400), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Assets Viewer", show, window_flags))
	{
		ImGui::End();
		return;
	}

	GUIManager* manager = Application::GetInstance().guiManager.get();
	//check if mouse is hovering on the viewer window
	manager->assetsViewerIsHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

	//search bar
	ImGui::Text("Search:");
	ImGui::SameLine();
	ImGui::InputText("##search", manager->assetSearchBuffer, IM_ARRAYSIZE(manager->assetSearchBuffer));

	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 15.0f);

	if (ImGui::TreeNodeEx("Assets", ImGuiTreeNodeFlags_DefaultOpen)) {
		//recursion
		DrawAssetTreeNode("../Assets");
		ImGui::TreePop();
	}
	ImGui::PopStyleVar();

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
			//Handled in process events
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::End();
}

void GUIElement::DrawAssetTreeNode(const std::string& directoryPath) {
	//get search state
	GUIManager* manager = Application::GetInstance().guiManager.get();
	std::string search_text = manager->assetSearchBuffer;
	std::transform(search_text.begin(), search_text.end(), search_text.begin(), ::tolower);

	//get directory contents
	auto entries = Application::GetInstance().fileSystem->GetDirectoryContents(directoryPath.c_str());

	for (const auto& entry : entries) {
		//check search
		if (!search_text.empty()) {
			std::string entryNameLower = entry.name;
			std::transform(entryNameLower.begin(), entryNameLower.end(), entryNameLower.begin(), ::tolower);

			//skip if the current item doesnt match and not a directory 
			if (entryNameLower.find(search_text) == std::string::npos && !entry.isDirectory) {
				continue;
			}
		}

		bool is_file = !entry.isDirectory;
		ImGui::PushID(entry.fullPath.c_str());
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
		if (!entry.isDirectory) {
			//file doesnt expand
			flags |= ImGuiTreeNodeFlags_Leaf;
		}

		std::string displayName = entry.name;
		if (is_file) {
			std::string metaPath = entry.fullPath + ".meta";
			if (Application::GetInstance().fileSystem->Exists(metaPath.c_str())) {
				VroomUUID uuid = Application::GetInstance().fileSystem->GetUUIDFromMeta(metaPath.c_str());

				if (uuid != 0) {
					std::shared_ptr<Resource> managedResource = Application::GetInstance().resourceManager->GetResourceByUUID(uuid);

					if (managedResource) {
						displayName = entry.name + " (Refs: " + std::to_string(managedResource.get()->GetReferenceCount()) + ")";
					}
				}
			}
		}

		//draw node
		bool opened = ImGui::TreeNodeEx(displayName.c_str(), flags);

		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete")) {
				std::string metaPath = entry.fullPath + ".meta";

				std::string extension = Application::GetInstance().fileSystem->GetExtensionFromPath(entry.fullPath.c_str());
				bool isModelFile = (extension == "fbx" || extension == "obj" || extension == "dae" || extension == "max");

				if (entry.isDirectory || !Application::GetInstance().fileSystem->Exists(metaPath.c_str()) || isModelFile) {
					Application::GetInstance().guiManager->fileDeleteQueue.push_back(entry.fullPath);
				}
				else {
					VroomUUID uuid = Application::GetInstance().fileSystem->GetUUIDFromMeta(metaPath.c_str());
					Application::GetInstance().guiManager->resourceDeleteQueue.push_back(uuid);
				}
			}
			ImGui::EndPopup();
		}

		//drag and drop organization
		if (entry.isDirectory && ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {

				std::string draggedAssetPath((const char*)payload->Data);

				std::string assetFileName = Application::GetInstance().fileSystem->GetFileFromPath(draggedAssetPath.c_str());
				std::string newDirectoryPath = entry.fullPath;

				//create new path
				std::string newAssetPath = newDirectoryPath + "/" + assetFileName;

				//find uuid
				std::string draggedMetaPath = draggedAssetPath + ".meta";
				if (Application::GetInstance().fileSystem->Exists(draggedMetaPath.c_str())) {
					VroomUUID uuid = Application::GetInstance().fileSystem->GetUUIDFromMeta(draggedMetaPath.c_str());

					//move the asset
					Application::GetInstance().resourceManager->MoveAsset(uuid, newAssetPath);
				}
				else {
					Application::GetInstance().fileSystem->MoveFileToNewPath(draggedAssetPath.c_str(), newAssetPath.c_str());
				}
			}
			ImGui::EndDragDropTarget();
		}

		//dragging
		if (!entry.isDirectory && ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("ASSET_PATH", entry.fullPath.c_str(), entry.fullPath.length() + 1);
			ImGui::Text("%s", entry.name.c_str());
			ImGui::EndDragDropSource();
		}

		//recursion loop
		if (entry.isDirectory) {
			if (opened) {
				DrawAssetTreeNode(entry.fullPath);
				ImGui::TreePop();
			}
		}
		else {
			if (opened) ImGui::TreePop(); 
		}

		ImGui::PopID();
	}
}

void GUIElement::InstantiateAsset(const std::string& assetPath) {
	Application::GetInstance().input.get()->ProcessDroppedFile(assetPath);
}

void GUIElement::ApplyTextureToSelection(const std::string& assetPath) {
	//make sure an object is selected
	auto go = manager->selectedObject;
	if (!go) {
		LOG("WARNING: Cannot apply texture - no GameObject selected.");
		return;
	}

	auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(go->GetComponent(ComponentType::MATERIAL));
	auto renderComp = std::dynamic_pointer_cast<RenderMeshComponent>(go->GetComponent(ComponentType::MESH_RENDERER));

	//make sure needed components are available
	if (!materialComp || !renderComp || !renderComp->GetMesh()) {
		LOG("WARNING: GameObject '%s' cannot accept texture (missing Material or Mesh component).", go->GetName().c_str());
		return;
	}

	auto mesh = renderComp->GetMesh();

	//request resource
	std::shared_ptr<Resource> resource = Application::GetInstance().resourceManager->RequestResource(assetPath);
	std::shared_ptr<ResourceTexture> newTex = std::dynamic_pointer_cast<ResourceTexture>(resource);

	if (newTex) {
		//apply to MaterialComponent for inspector
		materialComp->SetDiffuseMap(newTex);

		////apply to ResourceMesh for rendering
		//if (mesh->textures.empty()) {
		//	mesh->textures.push_back(newTex);
		//}
		//else {
		//	mesh->textures[0] = newTex;
		//}

		//auto it = manager->originalTextures.find(go);
		//if (it != manager->originalTextures.end()) {
		//	manager->originalTextures.erase(it);
		//}

		LOG("SUCCESS: Applied texture '%s' to GameObject '%s'.", newTex->GetName().c_str(), go->GetName().c_str());
	}
	else {
		LOG("ERROR: Failed to load/cast texture from path: %s", assetPath.c_str());
	}
}

void GUIElement::SceneViewportSetUp(bool* show) {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	if (ImGui::Begin("Scene", show, window_flags)) {
		//input gate -> check if window is hovered
		manager->sceneViewportIsHovered = ImGui::IsWindowHovered();

		//resizing handling
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		Render* render = Application::GetInstance().render.get();

		int currentWidth = Application::GetInstance().window->width;
		int currentHeight = Application::GetInstance().window->height;

		bool isInvalid = currentWidth <= 0 || currentHeight <= 0;
		bool needsResize = isInvalid || (viewportSize.x != currentWidth || viewportSize.y != currentHeight);

		if(needsResize) {
			if (viewportSize.x > 0 && viewportSize.y > 0) {
				Application::GetInstance().window->width = (int)viewportSize.x;
				Application::GetInstance().window->height = (int)viewportSize.y;

				render->InitSceneFBO((int)viewportSize.x, (int)viewportSize.y);

				Application::GetInstance().camera->RecalculateMatrices((int)viewportSize.x, (int)viewportSize.y);

				LOG("Scene Viewport resized and camera updated to %dx%d.", (int)viewportSize.x, (int)viewportSize.y);
			}
		}

		//render scene texture
		unsigned int sceneTextureID = render->sceneTextureID;
		//LOG("Current Scene Texture ID: %u", sceneTextureID);
		if (sceneTextureID != 0) {
			ImGui::Image((ImTextureID)(intptr_t)sceneTextureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));

			// show gizmo when an object is selected
			ImVec2 imgMin = ImGui::GetItemRectMin();
			ImVec2 imgMax = ImGui::GetItemRectMax();
			float imgWidth = imgMax.x - imgMin.x;
			float imgHeight = imgMax.y - imgMin.y;

			// for safety reasons we only show if image has positive size
			if (imgWidth > 0 && imgHeight > 0) {
				// Begin ImGuizmo frame and configure
				ImGuizmo::BeginFrame();
				ImGuizmo::Enable(true);
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(imgMin.x, imgMin.y, imgWidth, imgHeight);

				// If there's a selection draw the manipulator
				auto selected = manager->selectedObject;
				if (selected) {
					auto transform = std::dynamic_pointer_cast<TransformComponent>(selected->GetComponent(ComponentType::TRANSFORM));
					if (transform) {
						// Get matrices from camera and object
						glm::mat4 modelMat = transform->GetGlobalTransform();
						glm::mat4 viewMat = Application::GetInstance().camera->viewMat;
						glm::mat4 projMat = Application::GetInstance().camera->projectionMat;

						// Prepare float buffers for ImGuizmo (column-major)
						float modelArr[16];
						memcpy(modelArr, glm::value_ptr(modelMat), sizeof(modelArr));

						// Choose imguizmo mode
						ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
						switch (manager->gizmoOperation) {
						case GUIManager::GIZMO_TRANSLATE:
							operation = ImGuizmo::TRANSLATE;
							break;
						case GUIManager::GIZMO_ROTATE:
							operation = ImGuizmo::ROTATE;
							break;
						case GUIManager::GIZMO_SCALE:
							operation = ImGuizmo::SCALE;
							break;
						default:
							operation = ImGuizmo::TRANSLATE;
							break;
						}
						ImGuizmo::MODE mode = ImGuizmo::WORLD;

						// Show the guizmo manipulator
						ImGuizmo::Manipulate(glm::value_ptr(viewMat), glm::value_ptr(projMat), operation, mode, modelArr);

						// apllying the new transform
						if (ImGuizmo::IsUsing()) {
							float matrixTranslation[3], matrixRotation[3], matrixScale[3];
							glm::mat4 modelMatrix = transform->GetModelMatrix();
							ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix), matrixTranslation, matrixRotation, matrixScale);
							transform->SetPosition(glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]));
							glm::vec3 eulerRadians = glm::radians(glm::vec3(matrixRotation[0], matrixRotation[1], matrixRotation[2]));
							transform->SetRotation(glm::quat(eulerRadians));
							transform->SetScale(glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]));

							glm::mat4 newGlobal = glm::make_mat4(modelArr);

							glm::mat4 parentGlobal = glm::mat4(1.0f);
							if (auto parent = selected->GetParent()) {
								auto parentTransform = std::dynamic_pointer_cast<TransformComponent>(parent->GetComponent(ComponentType::TRANSFORM));
								if (parentTransform) parentGlobal = parentTransform->GetGlobalTransform();
							}

							glm::mat4 localMat = glm::inverse(parentGlobal) * newGlobal;

							glm::vec3 translation = glm::vec3(localMat[3]);
							glm::vec3 col0 = glm::vec3(localMat[0]);
							glm::vec3 col1 = glm::vec3(localMat[1]);
							glm::vec3 col2 = glm::vec3(localMat[2]);
							glm::vec3 scale(
								glm::length(col0),
								glm::length(col1),
								glm::length(col2)
							);
							//divide by 0 prevention
							if (scale.x == 0.0f) scale.x = 1.0f;
							if (scale.y == 0.0f) scale.y = 1.0f;
							if (scale.z == 0.0f) scale.z = 1.0f;

							glm::mat3 rotMat(
								col0 / scale.x,
								col1 / scale.y,
								col2 / scale.z
							);

							glm::quat rotation = glm::quat_cast(rotMat);

							// Apply to component 
							transform->SetPosition(translation);
							transform->SetRotation(rotation);
							transform->SetScale(scale);
						}
					}
				}
			}
		}
		else {
			manager->sceneViewportIsHovered = false;
		}
	}
	ImGui::End();
}


void GUIElement::GameViewportSetUp(bool* show) {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	if (ImGui::Begin("Game", show, window_flags)) {
		//resizing handling
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		Render* render = Application::GetInstance().render.get();

		int currentWidth = Application::GetInstance().render->gameWidth;
		int currentHeight = Application::GetInstance().render->gameHeight;

		bool isInvalid = currentWidth <= 0 || currentHeight <= 0;
		bool needsResize = isInvalid || (viewportSize.x != currentWidth || viewportSize.y != currentHeight);

		if (needsResize) {
			if (viewportSize.x > 0 && viewportSize.y > 0) {
				Application::GetInstance().render->gameWidth = (int)viewportSize.x;
				Application::GetInstance().render->gameHeight = (int)viewportSize.y;

				render->InitGameFBO((int)viewportSize.x, (int)viewportSize.y);

				Application::GetInstance().camera->RecalculateMatrices((int)viewportSize.x, (int)viewportSize.y);

				LOG("Game Viewport resized and camera updated to %dx%d.", (int)viewportSize.x, (int)viewportSize.y);
			}
		}

		uint32_t texID = Application::GetInstance().render->gameTextureID;
		if (texID) {
			ImGui::Image((ImTextureID)(uintptr_t)texID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
		}

		//drag and drop target
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
				std::string droppedPath((const char*)payload->Data);

				//handle instantiation
				InstantiateAsset(droppedPath);
			}
			ImGui::EndDragDropTarget();
		}
	}
	ImGui::End();
}