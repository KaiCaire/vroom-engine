#include "Window.h"
#include "Log.h"
#include "Application.h"

Window::Window() : Module()
{
	window = NULL;
	glContext = NULL;
	name = "window";
	
	
}

// Destructor
Window::~Window()
{
}

// Called before render is available
bool Window::Awake()
{
	LOG("Init SDL window & surface");
	bool ret = true;

	if (SDL_Init(SDL_INIT_VIDEO) != true)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		// Create window
		Uint32 flags = 0;
		bool fullscreen = false;
		bool borderless = false;
		bool resizable = true;
		bool fullscreen_window = false;

		if (fullscreen == true)        flags |= SDL_WINDOW_FULLSCREEN;
		if (borderless == true)        flags |= SDL_WINDOW_BORDERLESS;
		if (resizable == true)         flags |= SDL_WINDOW_RESIZABLE;
									   flags |= SDL_WINDOW_OPENGL;

		// SDL3: SDL_CreateWindow(title, w, h, flags). Set position separately.
		
		window = SDL_CreateWindow("Vroom Engine", width, height, flags);
		glContext = SDL_GL_CreateContext(window);

		if (window == NULL)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			if (fullscreen_window == true)
			{
				SDL_SetWindowFullscreenMode(window, nullptr); // use desktop resolution
				SDL_SetWindowFullscreen(window, true);
			}
			SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			SDL_ShowWindow(window);
			
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); //we need 3 floats cuz we save the 4th for a stencil buffer

		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);//Use 4

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);//Use 6
	}

	return ret;
}

// Called before quitting
bool Window::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	// Destroy window
	if (window != NULL)
	{
		SDL_DestroyWindow(window);
	}

	// Quit SDL subsystems
	SDL_Quit();
	return true;
}

// Set new window title
void Window::SetTitle(const char* new_title)
{
	//title.create(new_title);
	SDL_SetWindowTitle(window, new_title);
}

void Window::GetSize(int& width, int& height) const
{
	width = this->width;
	height = this->height;
}

int Window::GetScale() const
{
	return scale;
}

void Window::SetFullScreen(bool set) {
	isFullscreen = set;

	if (isFullscreen) {
		SDL_SetWindowFullscreenMode(window, nullptr); // use desktop resolution
		SDL_SetWindowFullscreen(window, true);
		
	}
	else {
		SDL_SetWindowFullscreen(window, false);
	}

	SDL_GetWindowSize(window, &width, &height);
}

void Window::SetWindowSize(glm::vec2 size) {
	currentRes = size;
	width = size.x;
	height = size.y;
	SDL_SetWindowSize(window, size.x, size.y);
}