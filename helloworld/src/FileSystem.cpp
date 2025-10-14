#include "FileSystem.h"

struct aiLogStream stream;

FileSystem::FileSystem() {

}

FileSystem::~FileSystem() {

}

bool FileSystem::Start() {

	// Stream log messages to Debug window
	
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);


	return true;
}

bool FileSystem::CleanUp() {

	aiDetachAllLogStreams();
	return true;
}

aiMesh FileSystem::LoadFile(const char* filePath) {

	aiMesh* new_mesh;
	const aiScene* scene = aiImportFile(filePath, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene != nullptr && scene->HasMeshes())
	{

		// Use scene->mNumMeshes to iterate on scene->mMeshes array
		for (int i = 0; i < scene->mNumMeshes; i++) {
			//retrieve mesh list from scene
			aiMesh* aiMesh = scene->mMeshes[i];

			//create new mesh struct for each mesh in scene
			Mesh ourMesh;

			//copy vertices
			ourMesh.num_vertices = aiMesh->mNumVertices;
			ourMesh.vertices = new float[ourMesh.num_vertices * 3];
			memcpy(ourMesh.vertices, aiMesh->mVertices, sizeof(float) * ourMesh.num_vertices * 3);
			LOG("New mesh with %d vertices", ourMesh.num_vertices);

			// copy faces
			if (aiMesh->HasFaces())
			{
				ourMesh.num_indices = aiMesh->mNumFaces * 3;
				ourMesh.indices = new uint[ourMesh.num_indices]; // assume each face is a triangle
				for (uint i = 0; i < aiMesh->mNumFaces; ++i)
				{
					if (aiMesh->mFaces[i].mNumIndices != 3)
						LOG("WARNING, geometry face with != 3 indices!");
					else
						memcpy(&ourMesh.indices[i * 3], new_mesh->mFaces[i].mIndices, 3 * sizeof(uint));
				}
			}
		}
		aiReleaseImport(scene);
	}
	else LOG("Error loading scene %s", filePath);


	return *new_mesh;
}

aiMesh FileSystem::SaveFile(const char* filePath) {

	aiMesh* newMesh;
	return *newMesh;
}