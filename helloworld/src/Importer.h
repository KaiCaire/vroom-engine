#pragma once
#include "Module.h"
#include <vector>
#include <memory> 
#include <string>



class MeshImporter;
class TextureImporter;
class ResourceTexture;
class ModelImporter;

class Importer : public Module {
public:
	Importer();
	~Importer();

	bool Start();

	bool CleanUp();

	MeshImporter* meshImporter;
	TextureImporter* textureImporter;
	ModelImporter* modelImporter;

	
	std::vector<std::shared_ptr<ResourceTexture>> textures_loaded;
	
};
