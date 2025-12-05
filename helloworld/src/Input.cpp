#include "Application.h"
#include "Input.h"
#include "Window.h"
#include "GUIManager.h"
#include "Log.h"
#include "OpenGL.h"
#include "GameObject.h"
#include "Component.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "ModelImporter.h"
#include "Render.h"
#include "Importer.h"
#include "TextureImporter.h"
#include "ResourceManager.h"
#include "ResourceTexture.h"

#include "SDL3/SDL.h"
#include <vector>


using namespace std;
class RenderMeshComponent;


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
			w = event.window.data1;
			h = event.window.data2;

			//make sure window values are set accordingly
			Application::GetInstance().window.get()->width = w;
			Application::GetInstance().window.get()->height = h;

			//handle opengl window on resize
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

void Input::ProcessDroppedFile(std::string sourcePath) {
	/*std::replace(sourcePath.begin(), sourcePath.end(), '\\', '/');*/

	FileSystem* fs = Application::GetInstance().fileSystem.get();
	std::string path = fs->NormalizePath(sourcePath.c_str());
	
	LOG("Dropped File Directory = %s", path.c_str());
	
	std::string fileExtension = fs->GetExtensionFromPath(path.c_str());

	// Handle model files (FBX, OBJ)
	if (fileExtension == "fbx" || fileExtension == "obj") {
		ImportModelFile(path);
	}
	// Handle texture files (PNG, JPG, TGA, DDS)
	else if (fileExtension == "png" || fileExtension == "jpg" || fileExtension == "tga" || fileExtension == "dds") {
		ApplyTextureToSelectedObject(path);
	}
	else {
		LOG("WARNING: Unsupported file type: %s", fileExtension.c_str());
	}
}

void Input::ImportModelFile(const std::string& modelPath) {
	auto modelImporter = Application::GetInstance().importer->modelImporter;

	// ImportScene returns the root GameObject and adds it to sceneObjects
	auto rootGameObject = modelImporter->ImportScene(modelPath.c_str());

	if (!rootGameObject) {
		LOG("ERROR: Failed to import model from: %s", modelPath.c_str());
		return;
	}

	LOG("Successfully imported model: %s", rootGameObject->GetName().c_str());
}

void Input::ApplyTextureToSelectedObject(const std::string& texturePath) {
	GUIManager* guiManager = Application::GetInstance().guiManager.get();
	auto selectedObj = guiManager->selectedObject;

	if (!selectedObj) {
		LOG("No GameObject selected. Select a GameObject in the hierarchy and try again");
		return;
	}

	// Verify the object has a mesh renderer
	auto meshComp = std::dynamic_pointer_cast<RenderMeshComponent>(
		selectedObj->GetComponent(ComponentType::MESH_RENDERER)
	);

	if (!meshComp) {
		LOG("Selected GameObject '%s' has no mesh. Select a GameObject with a mesh and try again",
			selectedObj->GetName().c_str());
		return;
	}

	// Get or create MaterialComponent
	auto materialComp = std::dynamic_pointer_cast<MaterialComponent>(
		selectedObj->GetComponent(ComponentType::MATERIAL)
	);

	if (!materialComp) {
		LOG("No MaterialComponent found on '%s', creating one", selectedObj->GetName().c_str());
		auto newMatComp = selectedObj->AddComponent(ComponentType::MATERIAL);
		materialComp = std::dynamic_pointer_cast<MaterialComponent>(newMatComp);

		if (!materialComp) {
			LOG("ERROR: Failed to create MaterialComponent");
			return;
		}
	}

	// Load texture through ResourceManager (which handles caching)
	ResourceManager* resourceManager = Application::GetInstance().resourceManager.get();
	auto newTexture = std::dynamic_pointer_cast<ResourceTexture>(
		resourceManager->RequestResource(texturePath)
	);

	if (!newTexture) {
		LOG("ERROR: Failed to load texture: %s", texturePath.c_str());
		return;
	}

	if (newTexture) {
		newTexture->mapType = "texture_diffuse";
		newTexture->path = texturePath;
	}

	// Apply texture to MaterialComponent
	materialComp->SetDiffuseMap(newTexture);

	// Update mesh textures (replaces existing textures on THIS mesh only)
	auto meshPtr = meshComp->GetMesh();
	if (meshPtr) {
		meshPtr->textures.clear();
		meshPtr->textures.push_back(newTexture);
	}

	// Make sure checker texture is disabled when applying a new texture
	guiManager->RestoreOGTexture(selectedObj);

	FileSystem* fs = Application::GetInstance().fileSystem.get();
	std::string fileName = fs->GetFileFromPath(texturePath.c_str());

	LOG("Texture '%s' (UUID: %llu) applied to '%s'",
		fileName.c_str(), newTexture->GetUUID(), selectedObj->GetName().c_str());
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

