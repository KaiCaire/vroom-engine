#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <SDL3/SDL_opengl.h>
#include "SDL3/SDL.h"

#include "Module.h"
#include "Gameobject.h"
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

	// Check if a GameObject is showing checker texture
	bool IsShowingCheckerTexture(std::shared_ptr<GameObject> go) {
		if (!go) return false;
		return originalTextures.count(go) > 0;
	}

	// Show checker texture for a specific GameObject
	void ShowCheckerTexture(std::shared_ptr<GameObject> go);

	// Restore original texture for a specific GameObject
	void RestoreOGTexture(std::shared_ptr<GameObject> go);



	//queue object for deletion
	void AddToDeleteQueue(const std::shared_ptr<GameObject>& obj);

	/*void RefreshGUIHierarchy();*/

private:
	ImGuiIO* io = nullptr;
	std::vector<GUIElement> WindowElements;
	GUIElement AdditionalElements;
	GUIElement Menu;

	bool dockInitialized = false;
	bool objectsInitialized = false;

public:
	bool showAboutPopup = false;
	bool showConsole = true;
	bool showConfig = false;
	bool showHierarchy = true;
	bool showInspector = true;

	std::shared_ptr<GameObject> selectedObject;

	std::map<std::shared_ptr<GameObject>, std::shared_ptr<ResourceTexture>> originalTextures;

	bool drawFaceNormals = false;
	bool drawVertNormals = false;
};