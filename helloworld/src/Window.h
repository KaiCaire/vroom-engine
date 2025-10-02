#pragma once

#include "Module.h"

class Application;

enum WindowEvent
{
	WINDOW_EVENT_QUIT = 0,
	WINDOW_EVENT_HIDE = 1,
	WINDOW_EVENT_SHOW = 2,
	WINDOW_EVENT_RESIZE = 3,
	WINDOW_EVENT_COUNT       // Max window events
};

class Window : public Module
{
public:

	Window(Application* app, bool start_enabled = true);
	// Destructor
	virtual ~Window();

	bool Init();
	bool Awake();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void SetTitle(const char* title);



	// Gather relevant win events
	void GetSize (int& width, int& height) const;

	int GetScale() const;

private:
	uint width;
	uint height;

	bool windowEvents[WINDOW_EVENT_COUNT];
};
