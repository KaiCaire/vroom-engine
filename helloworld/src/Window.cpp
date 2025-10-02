#pragma once
#include "Module.h"
#include "Globals.h"

#include <limits.h>

class Render : public Module
{
public:
	Render(Application* app, bool start_enabled = true);
	~Render();

	bool Init();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void SetBackgroundColor(Color color);
	bool Draw(Texture2D texture, int x, int y, const Rectangle* section = NULL, double angle = 0, int pivot_x = 0, int pivot_y = 0) const;
	bool DrawText(const char* text, int x, int y, Font font, int spacing, Color tint) const;

public:

	Color background;
	Rectangle camera;
};