#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <SDL3/SDL_opengl.h>
#include "SDL3/SDL.h"

#include "Module.h"
#include <vector>
#include "FileSystem.h"
#include "GUIElement.h"

//class GUIElement;

class GUIManager : public Module 
{
public:
	GUIManager();

	//Destructor
	virtual ~GUIManager();

	//Called before manager is available
	bool Awake();

	//Setup windows
	std::vector<GUIElement> LoadElements();

	//Called before first frame
	bool Start();

	//Called for each loop
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	//initialize the docking space
	void InitDock();

	//Within update process events
	void ProcessEvents(SDL_Event event);

	//Called before quit
	bool CleanUp();
private:
	ImGuiIO* io = nullptr;
	std::vector<GUIElement> WindowElements;
	GUIElement AdditionalElements;
	GUIElement Menu;

	bool dockInitialized = false;

public:
	bool showAboutPopup = false;
	bool showConsole = true;
	bool showConfig = false;
	bool showHierarchy = false;
	bool showInspector = false;
};