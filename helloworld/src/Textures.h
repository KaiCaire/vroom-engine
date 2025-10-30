#pragma once

#include "Module.h"
#include <string>


class Texture : public Module
{
public:

	Texture();

	
	virtual ~Texture();

	bool Awake();

	bool Start();

	bool CleanUp();

	uint TextureFromFile(std::string directory, const char* filename);


public:
	
	uint id;
	std::string mapType;
	std::string path;
	
};
