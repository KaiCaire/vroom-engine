#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "SDL3/SDL.h"
#include <memory>
#include "Module.h"
#include "FileSystem.h"
#include "GameObject.h"

enum ElementType{ Additional, MenuBar, Console, Config, Hierarchy, Inspector, AssetsViewer, SceneViewport, GameViewport};

class ResourceTexture;

class GUIElement {
public:
	GUIElement(ElementType t, GUIManager* m);
	~GUIElement();

	void ElementSetUp();

	//type set ups
	void MenuBarSetUp();
	void AboutSetUp();
	void ConsoleSetUp(bool* show);
	void ConfigSetUp(bool* show);
	void HierarchySetUp(bool* show);
	void InspectorSetUp(bool* show);
	void AssetsViewerSetUp(bool* show);
	void SceneViewportSetUp(bool* show);
	void GameViewportSetUp(bool* show);

	//other
	void DrawNode(const std::shared_ptr<GameObject>& obj, std::shared_ptr<GameObject>& selected);
	void DrawAssetTreeNode(const std::string& directoryPath);
	void InstantiateAsset(const std::string& assetPath);
	void ApplyTextureToSelection(const std::string& assetPath);
	

private:
	ElementType type;
	GUIManager* manager;
};