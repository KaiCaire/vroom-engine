#include "Textures.h"
#include "Application.h"
#include "Render.h"
#include "Log.h"
#include <string>
#include "FileSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

Textures::Textures() : Module()
{
	name = "textures";
}

// Destructor
Textures::~Textures()
{
}

// Called before render is available
bool Textures::Awake()
{
	
	bool ret = true;

	return ret;
}

// Called before the first frame
bool Textures::Start()
{
	
	bool ret = true;
	return ret;
}

// Called before quitting
bool Textures::CleanUp()
{
	
	return true;
}


unsigned char* Textures::LoadTexture(const char* filename, int* w, int* h, int* nChannels, int desiredChannels) {

	std::string dir = "../Assets/Textures/";
	std::string filePath = dir + filename; 
	return stbi_load(filePath.c_str(), w, h, nChannels, desiredChannels);
}

uint Textures::TextureFromFile(const char* str, const char* directory) {
	//get texture from a file
	return (uint)0;
}

