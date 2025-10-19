#pragma once

#include "Module.h"

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

public:
	// The window we'll be rendering to
	SDL_Window* window;
	SDL_GLContext glContext;

	std::string title;
	int width = 800;
	int height = 720;
	int scale = 1;
};