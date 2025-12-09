#pragma once
#include <assimp/scene.h>
#include <memory>
#include <vector>
#include <string>
#include "Importer.h"
#include "FileSystem.h"



// Forward declarations
class ResourceMesh;
struct Vertex;
class ResourceTexture;

class MeshImporter {
public:
    
    static std::shared_ptr<ResourceMesh> Import(aiMesh* aiMesh, const aiScene* scene, const std::string& modelPath, VroomUUID cachedUUID = 0);

private:
    FileSystem* fs;
    static std::vector<Vertex> ProcessVertices(aiMesh* aiMesh);
    static std::vector<unsigned int> ProcessIndices(aiMesh* aiMesh);
    static std::vector<std::shared_ptr<ResourceTexture>> ProcessTextures(aiMesh* aiMesh, const aiScene* scene, const std::string& modelPath);

};