#pragma once

#include "Globals.h"
#include <vector>

class Module;
class Window;
class Renderer;

class Application
{
public:

	Window* window;
	Renderer* renderer;
	
private:

	std::vector<Module*> list_modules;
	uint64 frame_count = 0;

public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

private:

	void AddModule(Module* module);
};
