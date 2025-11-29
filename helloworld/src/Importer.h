#pragma once
#include "Module.h"
#include <vector>
#include <memory> 
#include <string>


class MeshImporter;
class TextureImporter;
class Texture;

class Importer : public Module {
public:
	Importer();
	~Importer();

	bool Start();

	bool CleanUp();

	MeshImporter* meshImporter;
	TextureImporter* textureImporter;


	std::vector<std::shared_ptr<Texture>> textures_loaded;
	std::string defaultTexDir = "../Assets/Textures/checkers.jpg";
};
