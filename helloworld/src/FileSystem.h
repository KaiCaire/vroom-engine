#pragma once
#include "Module.h"
#include "Log.h"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"

struct Vertex {
	float position[3];
	float Normal[3];
	float TexCoords[3];
};

struct Mesh {
	uint id_index = 0; // index in VRAM
	uint num_indices = 0;
	uint* indices = nullptr;
	uint id_vertex = 0; // unique vertex in VRAM
	uint num_vertices = 0;
	float* vertices = nullptr;
};

struct VertexBuffers {

};

class FileSystem : public Module {

public:

	FileSystem();
	~FileSystem();

	bool Start() override;
	bool CleanUp() override;

	aiMesh LoadFile(const char* filePath);
	aiMesh SaveFile(const char* filePath);


private:

	

};
