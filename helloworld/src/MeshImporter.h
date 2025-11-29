#pragma once
#include <assimp/scene.h>
#include <memory>
#include <vector>
#include <string>
#include "Importer.h"

// Forward declarations
class Mesh;
struct Vertex;
class Texture;

class MeshImporter {
public:
    // Main import function: aiMesh → Mesh (which is now a Resource)
    static std::shared_ptr<Mesh> Import(aiMesh* aiMesh, const aiScene* scene, const std::string& modelPath);

private:
    // Helper to process vertices
    static std::vector<Vertex> ProcessVertices(aiMesh* aiMesh);

    // Helper to process indices
    static std::vector<unsigned int> ProcessIndices(aiMesh* aiMesh);

    // Helper to process textures
    static std::vector<std::shared_ptr<Texture>> ProcessTextures(aiMesh* aiMesh, const aiScene* scene, const std::string& modelPath);



};