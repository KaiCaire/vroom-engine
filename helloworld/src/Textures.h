#pragma once

#include "Module.h"
//#include <SDL3/SDL.h>
//#include <SDL3_image/SDL_image.h>


class Textures : public Module
{
public:

	Textures();

	// Destructor
	virtual ~Textures();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before quitting
	bool CleanUp();

	
	uint Textures::TextureFromFile(const char* aiStr, const char* directory);

public:
	
	unsigned int textureIDs[];
};
