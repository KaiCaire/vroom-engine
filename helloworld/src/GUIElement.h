#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "SDL3/SDL.h"

#include "Module.h"
#include "FileSystem.h"

enum ElementType{ Additional, MenuBar};

class GUIElement {
public:
	GUIElement(ElementType t);
	~GUIElement();

	void ElementSetUp();

	//type set ups
	void MenuBarSetUp();
	void AboutSetUp();

private:
	ElementType type;
};