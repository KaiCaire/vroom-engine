#pragma once
#include <assimp/scene.h>
#include <memory>
#include <vector>
#include <string>
#include "Importer.h"

// Forward declarations
class ResourceMesh;
struct Vertex;
class ResourceTexture;

class MeshImporter {
public:
    // Main import function: aiMesh → Mesh (which is now a Resource)
    static std::shared_ptr<ResourceMesh> Import(aiMesh* aiMesh, const aiScene* scene, const std::string& modelPath);

private:
    // Helper to process vertices
    static std::vector<Vertex> ProcessVertices(aiMesh* aiMesh);

    // Helper to process indices
    static std::vector<unsigned int> ProcessIndices(aiMesh* aiMesh);

    // Helper to process textures
    static std::vector<std::shared_ptr<ResourceTexture>> ProcessTextures(aiMesh* aiMesh, const aiScene* scene, const std::string& modelPath);



};