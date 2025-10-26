#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "SDL3/SDL.h"

#include "Module.h"
#include "FileSystem.h"

class GUIManager : public Module 
{
	GUIManager();

	//Destructor
	virtual ~GUIManager();

	//Called before manager is available
	bool Awake();

	//Called before first frame
	bool Start();

	//Called for each loop
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	//Called before quit
	bool CleanUp();

};