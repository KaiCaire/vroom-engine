#include "Application.h"
#include "Window.h"
#include "GUIElement.h"
#include "Log.h"
#include "FileSystem.h"
#include "GUIManager.h"
#include <SDL3/SDL_opengl.h>
#include <vector>

GUIElement::GUIElement(ElementType t) 
{
	type = t;
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
		ImGui::Text("Developed by: Ivan Alvarez, Kai Caire, Lara Guevara, Bernat Loza");
		ImGui::Text("Developed using: ");
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