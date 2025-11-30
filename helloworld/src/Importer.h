#pragma once
#include "Module.h"
#include <vector>
#include <memory> 
#include <string>


class MeshImporter;
class TextureImporter;
class ResourceTexture;

class Importer : public Module {
public:
	Importer();
	~Importer();

	bool Start();

	bool CleanUp();

	MeshImporter* meshImporter;
	TextureImporter* textureImporter;


	std::vector<std::shared_ptr<ResourceTexture>> textures_loaded;
	std::string defaultTexDir = "../Assets/Textures/checkers.jpg";
};
