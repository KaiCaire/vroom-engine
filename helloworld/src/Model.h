#pragma once
#include "Shader.h"
#include "Mesh.h"
#include "Textures.h"
#include "GameObject.h"
#include <vector>

class Model {
public:
    Model(char* path) { loadModel(path); }
    ~Model();

    
    void Draw(Shader& shader); 
    vector<Mesh> meshes;        

    
    GameObject* GetRootGameObject() const { return rootGameObject; }
    const vector<GameObject*>& GetGameObjects() const { return gameObjects; }

private:
    
    std::string directory;
    int processedMeshes = 0;

    
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    
    GameObject* rootGameObject = nullptr;
    vector<GameObject*> gameObjects;
    void processNodeWithGameObjects(aiNode* node, const aiScene* scene, GameObject* parent);

    
    void loadModel(string path);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};