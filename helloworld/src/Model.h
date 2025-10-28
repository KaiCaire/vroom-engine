#pragma once
#include "Shader.h"
#include "Mesh.h"
#include "Textures.h"
#include <vector>

using namespace std;


class Model
{
public:
    Model(const char* path)
    {
        loadModel(path);
    }

    ~Model() {};
    void Draw(Shader& shader);
public:
    // model data
    int processedMeshes = 0;
    vector<Mesh> meshes;
    std::string directory;

    std::string fullPath;

    void loadModel(string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};
