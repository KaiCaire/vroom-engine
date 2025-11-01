#pragma once

#include "Module.h"
#include <vector>

class Window : public Module
{
public:

	Window();

	// Destructor
	virtual ~Window();

	// Called before render is available
	bool Awake();

	// Called before quitting
	bool CleanUp();

	// Changae title
	void SetTitle(const char* title);

	// Retrive window size
	void GetSize(int& width, int& height) const;

	// Retrieve window scale
	int GetScale() const;

	SDL_Window* GetWindow() const {
		return window;
	}

	SDL_GLContext GetContext() const {
		return glContext;
	}

	void SetFullScreen(bool set);
	void SetWindowSize(glm::vec2 size);

public:
	// The window we'll be rendering to
	SDL_Window* window;
	SDL_GLContext glContext;

	std::string title;
	int width = 800;
	int height = 720;
	int scale = 1;

	bool isFullscreen = false;

	//current resolution being used 
	glm::vec2 currentRes = { width, height };

	//list of resolutions for imgui dropdown
	std::vector<glm::vec2> resolutions = {
		{800, 720},
		{1280, 720}
	};

};