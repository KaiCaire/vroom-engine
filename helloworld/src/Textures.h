#pragma once

#include "Module.h"

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

	/*bool Update(float dt);*/

	// Called before quitting
	bool CleanUp();

	unsigned char* LoadTexture(const char* filename, int* w, int* h, int* nChannels, int desiredChannels = 0);

	uint Textures::TextureFromFile(const char* aiStr, const char* directory);

public:
	
	unsigned int textureIDs[];
};
