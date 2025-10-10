#pragma once

#include <memory>
#include <list>
#include "Module.h"


// Modules
class Window;
class Input;
class Render;
class OpenGL;

//class Physics;

class Application
{
public:

	// Public method to get the instance of the Singleton
	static Application& GetInstance();

	//	
	void AddModule(std::shared_ptr<Module> module);

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	float GetDt() const {
		return dt;
	}

private:

	// Private constructor to prevent instantiation
	// Constructor
	Application();

	// Delete copy constructor and assignment operator to prevent copying
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	// Call modules before each loop iteration
	void PrepareUpdate();

	// Call modules before each loop iteration
	void FinishUpdate();

	// Call modules before each loop iteration
	bool PreUpdate();

	// Call modules on each loop iteration
	bool DoUpdate();

	// Call modules after each loop iteration
	bool PostUpdate();

	//// Load config file
	//bool LoadConfig();

	std::list<std::shared_ptr<Module>> moduleList;

public:

	enum EngineState
	{
		CREATE = 1,
		AWAKE,
		START,
		LOOP,
		CLEAN,
		FAIL,
		EXIT
	};

	// Modules
	std::shared_ptr<Window> window;
	std::shared_ptr<Input> input;
	std::shared_ptr<Render> render;
	std::shared_ptr<OpenGL> openGL;
	


private:

	// Delta time
	float dt;
	//Frames since startup
	int frames;

	// Calculate timing measures
	// required variables are provided:
	/*Timer startupTime;
	PerfTimer frameTime;
	PerfTimer lastSecFrameTime;*/

	int frameCount = 0;
	int framesPerSecond = 0;
	int lastSecFrameCount = 0;

	float averageFps = 0.0f;
	int secondsSinceStartup = 0;

	//Maximun frame duration in miliseconds.
	int maxFrameDuration = 16;

	std::string gameTitle = "Vroom-Engine";

};