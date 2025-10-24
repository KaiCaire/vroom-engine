#pragma once
#include "Module.h"
#include "Log.h"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"

class FileSystem : public Module {

public:

	FileSystem();
	~FileSystem();

	bool Start() override;
	bool CleanUp() override;

	/*aiMesh LoadFile(const char* filePath);
	aiMesh SaveFile(const char* filePath);*/


private:
	

};
