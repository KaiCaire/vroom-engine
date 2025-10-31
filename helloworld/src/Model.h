#pragma once
#include "Shader.h"
#include "Mesh.h"
#include "Textures.h"
#include "GameObject.h"
#include <vector>
#include <string>

class Model {
public:
    Model(std::string path) 
    { 
        loadModel(path);
    }
    ~Model();

    
    void Draw(Shader& shader); 
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::shared_ptr<GameObject> rootGameObject;
    std::vector<std::shared_ptr<GameObject>> gameObjects;

    std::vector<std::shared_ptr<GameObject>>& GetGameObjects() { return gameObjects; }

    shared_ptr<GameObject> GetRootGameObject() const { return rootGameObject; }
    
    std::string directory;
    int processedMeshes = 0;

    std::string fullPath;

    void loadModel(std::string path);

    

   /* void processNode(aiNode* node, const aiScene* scene);*/
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    
    

    void processNodeWithGameObjects(aiNode* node, const aiScene* scene, std::shared_ptr<GameObject> parent);
    void createComponentsForMesh(std::shared_ptr<GameObject> gameObject, aiMesh* aiMesh, const aiScene* scene);

    
   
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

    void LogGameObjectHierarchy(std::shared_ptr<GameObject>  go, int depth);

    
    
    

    
};