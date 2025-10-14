#pragma once
#include "Shader.h"
#include "Mesh.h"
#include <vector>

using namespace std;

class Model
{
public:
    Model(char* path)
    {
        loadModel(path);
    }

    ~Model();
    void Draw(Shader& shader);
public:
    // model data
    vector<Mesh> meshes;
    string directory;

    void loadModel(string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};
