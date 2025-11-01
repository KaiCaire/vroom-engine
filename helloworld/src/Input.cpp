#include "Application.h"
#include "Input.h"
#include "Window.h"
#include "GUIManager.h"
#include "Log.h"
#include "OpenGL.h"

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Model.h"
#include "Render.h"

#include "SDL3/SDL.h"
#include <vector>


using namespace std;


Input::Input() : Module()
{
	name = "input";

	keyboard = new KeyState[MAX_KEYS];
	numkeys = new int[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KeyState) * MAX_KEYS);
	memset(mouseButtons, KEY_IDLE, sizeof(KeyState) * NUM_MOUSE_BUTTONS);

}


//TODO: DRAG AND DROP FUNCTION THAT IMPORTS * file type * TO SCENE

// Destructor
Input::~Input()
{
	delete[] keyboard;
}

// Called before render is available
bool Input::Awake()
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

// Called before the first frame
bool Input::Start()
{
	SDL_StopTextInput(Application::GetInstance().window.get()->window);
	return true;
}

// Called each loop iteration
bool Input::PreUpdate()
{
	static SDL_Event event;

	int numkeys = 0; // will receive the number of keys
	const bool* keys = SDL_GetKeyboardState(&numkeys);

	for (int i = 0; i < MAX_KEYS; ++i)
	{
		if (keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE)
				keyboard[i] = KEY_DOWN;
			else
				keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
				keyboard[i] = KEY_UP;
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
	{
		if (mouseButtons[i] == KEY_DOWN)
			mouseButtons[i] = KEY_REPEAT;

		if (mouseButtons[i] == KEY_UP)
			mouseButtons[i] = KEY_IDLE;
	}
	int w, h = 0;
	while (SDL_PollEvent(&event) != 0)
	{
		Application::GetInstance().guiManager.get()->ProcessEvents(event);
		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			windowEvents[WE_QUIT] = true;
			break;

			/*case SDL_EVENT_WINDOW_RESIZED:*/
		case SDL_EVENT_WINDOW_RESIZED:
			//handle opengl window on resize
			w = event.window.data1;
			h = event.window.data2;
			glViewport(0,0,w, h);
			break;
			/*case SDL_WINDOWEVENT_LEAVE:*/
		case SDL_EVENT_WINDOW_HIDDEN:
		case SDL_EVENT_WINDOW_MINIMIZED:
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			windowEvents[WE_HIDE] = true;
			break;

			//case SDL_WINDOWEVENT_ENTER:
		case SDL_EVENT_WINDOW_SHOWN:
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
		case SDL_EVENT_WINDOW_MAXIMIZED:
		case SDL_EVENT_WINDOW_RESTORED:
			windowEvents[WE_SHOW] = true;
			break;



		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			mouseButtons[event.button.button - 1] = KEY_DOWN;
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			mouseButtons[event.button.button - 1] = KEY_UP;

			
			break;


		case SDL_EVENT_DROP_FILE:
			/*windowID = Application::GetInstance().window.get()->GetWindowID();*/
			droppedFileDir = event.drop.data;
			
			LOG("Dropped File Directory = %s", droppedFileDir);

			ProcessDroppedFile(droppedFileDir);
			
			
			
			//not needed in SDL3, the new allocated memory created  gets freed automatically
			/*SDL_free(&droppedFileDir);*/
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			mouseWheelY = event.wheel.y;
			break;

		case SDL_EVENT_MOUSE_MOTION:
			int scale = Application::GetInstance().window.get()->GetScale();
			mouseMotionX = event.motion.xrel / scale;
			mouseMotionY = event.motion.yrel / scale;
			mouseX = event.motion.x / scale;
			mouseY = event.motion.y / scale;
			break;


		}
	}

	return true;
}

// Called before quitting
bool Input::CleanUp()
{
	LOG("Quitting SDL event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}

void Input::ProcessDroppedFile(const std::string sourcePath) {


	//find last dot of directory to get file extension (.fbx, .obj, .png, .jpg, etc)

	//handle model files
	string fileExtension = sourcePath.substr(sourcePath.find_last_of(".") + 1);
	if (fileExtension == "fbx" || fileExtension == "FBX" || fileExtension == "obj") {
		importedModel = new Model(droppedFileDir);
		Application::GetInstance().render.get()->AddModel(*importedModel);
	}

	//handle image files
	else if (fileExtension == "png" || fileExtension == "jpg" || fileExtension == "tga") {
		//detect if mouse is over a mesh and which one
		glm::mat4 projMat = Application::GetInstance().openGL.get()->projectionMat;
		glm::mat4 viewMat = Application::GetInstance().openGL.get()->viewMat;
		glm::vec3 mouseRayDir = MouseRay(mouseX, mouseY, projMat, viewMat);
		
		/*Application::GetInstance().render.get()->modelsToDraw()[]*/
		
		//if it is, change current material's texture for the dropped texture
	}

	//try {
	//	std::filesystem::copy(sourcePath, destPath);
	//}
	
	
}

bool Input::GetWindowEvent(EventWindow ev)
{
	return windowEvents[ev];
}


glm::vec3 Input::MouseRay(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view) {

	int windowW, windowH;
	Application::GetInstance().window.get()->GetSize(windowW,windowH);

	float normalizedX = (2.0f * mouseX) / windowW - 1.0f;
	float normalizedY = 1.0f - (2.0f * mouseY) / windowH; 
	//y in 2D is greater the further down you go, but in 3D, it's the other way around! flip it by 1-(normalized2Dy)

	glm::vec4 clipCoords = glm::vec4(normalizedX, normalizedY, -1.0f, 1.0f);  
	// -1.0 = nearPlane in normalized depth space | w = 1.0 --> homogeneous coord

	/*
	mouse coords are in screen space, but we want a ray in world space (that starts at camera and goes into the scene)
	we need to convert (mouseX, mouseY) multiplying by (inverse) projection matrix, and then (inverse) view matrix
	*/

	glm::vec4 viewCoords = glm::inverse(projection) * clipCoords; //screen space --> cam (local) space
	viewCoords = glm::vec4(viewCoords.x, viewCoords.y, -1.0f, 0.0f);
	// z = -1 --> forward dir | w = 0 --> applies no translation
	// just keeps it as a direction (we just need to rotate, otherwise the direction vector will get offseted)
	 
	glm::vec4 worldCoords = glm::inverse(view) * viewCoords;  //cam (local) space --> world space
	
	glm::vec3 rayDirection = glm::normalize(glm::vec3(worldCoords)); 
	//normalize vector (we don't care about its lenght, just direction)
	

	return rayDirection;

}


SDL_FPoint Input::GetMousePosition()
{
	return { (float)mouseX, (float)mouseY };
}

SDL_FPoint Input::GetMouseMotion()
{
	return {(float)(mouseMotionX, (float)mouseMotionY)};
}

