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
	bool IsShowingCheckerTexture(std::shared_ptr<GameObject> go);
	//	return originalTextures.find(go) != originalTextures.end();
	//}
	VroomUUID GetCheckerTextureUUID();

	// Show checker texture for a specific GameObject
	void ShowCheckerTexture(std::shared_ptr<GameObject> go);

	// Restore original texture for a specific GameObject
	void RestoreOGTexture(std::shared_ptr<GameObject> go);



	//queue object for deletion
	void AddToDeleteQueue(const std::shared_ptr<GameObject>& obj);
	

	//add file to asset viewer
	void HandleExternalFileDrop(const std::string& sourceOSPath);

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
	bool showAssetsViewer = true;
	bool showSceneViewport = true;

	std::shared_ptr<GameObject> selectedObject;
	std::weak_ptr<GameObject> previousSelectedObject;

	std::map<std::shared_ptr<GameObject>, std::shared_ptr<ResourceTexture>> originalTextures;

	//queued resources for deletion
	std::vector<VroomUUID> resourceDeleteQueue;
	std::vector<std::string> fileDeleteQueue;

	//for search bar and filters
	char assetSearchBuffer[256] = "";     
	int selectedFilterType = 0;

	//check if asset viewer is hovered
	bool assetsViewerIsHovered = false;
	//check if scene is hovered
	bool sceneViewportIsHovered = false;

	bool drawFaceNormals = false;
	bool drawVertNormals = false;

	bool drawAABBs = false;
	bool drawRaycast = false;
};