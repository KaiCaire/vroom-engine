#pragma once

#include "Module.h"
#include "SDL3/SDL.h"
#include "FileSystem"
#include "RenderMeshComponent.h"


class ResourceTexture;
class ResourceManager;

#define MAX_KEYS 300
#define NUM_MOUSE_BUTTONS 5

enum EventWindow
{
	WE_QUIT = 0,
	WE_HIDE = 1,
	WE_SHOW = 2,
	WE_COUNT
};

enum KeyState
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

class Input : public Module
{

public:

	Input();

	// Destructor
	virtual ~Input();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();

	// Called before quitting
	bool CleanUp();

	// Check key states (includes mouse and joy buttons)
	KeyState GetKey(int id) const
	{
		return keyboard[id];
	}

	KeyState GetMouseButtonDown(int id) const
	{
		return mouseButtons[id - 1];
	}

	void ProcessDroppedFile(std::string sourcePath);

	glm::vec3 MouseRay(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view);

	glm::vec3 ViewportMouseRay(int mouseX, int mouseY, int viewportW, int viewportH, const glm::mat4& projection, const glm::mat4& view);

	// Check if a certain window event happened
	bool GetWindowEvent(EventWindow ev);

	// Get mouse / axis position
	SDL_FPoint GetMousePosition();
	SDL_FPoint GetMouseMotion();
	
	


	int GetMouseWheelDeltaY() const { return mouseWheelY; }

	void SetMouseWheelDeltaY(int mouse) { mouseWheelY = mouse; }

public:
	int	mouseMotionX;
	int mouseMotionY;
	int mouseX;
	int mouseY;
	int mouseWheelY;

private:
	bool windowEvents[WE_COUNT];
	KeyState* keyboard;
	int* numkeys;

	KeyState mouseButtons[NUM_MOUSE_BUTTONS];

	const char* droppedFileDir;
	/*std::vector<std::shared_ptr<GameObject>> selectedObjects;*/

	void ApplyTextureToSelectedObject(const std::string& texturePath);
	

	FileSystem* fs;

	std::string CopyFileToAssets(const std::string sourcePath, const char* destPath, const std::string file);
	std::string CopyToTexFolder(const std::string sourcePath, const std::string file);
	bool UpdateTexturesInModelMeta(std::string metaPath, std::shared_ptr<RenderMeshComponent> renderComp);
	
	

};