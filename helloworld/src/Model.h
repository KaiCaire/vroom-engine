#pragma once
#include "Shader.h"
#include "Mesh.h"
#include "Textures.h"
#include "GameObject.h"
#include <vector>
#include <string>
#include <unordered_map>

class Model {
public:
    Model(std::string path) 
    { 
        loadModel(path);
    }

    Model(Mesh mesh);
    Model();
    ~Model();

    
    std::shared_ptr<Texture> savedTexture;
    
    void Draw(Shader& shader); 
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::shared_ptr<GameObject> rootGameObject;
    std::vector<std::shared_ptr<GameObject>> gameObjects;

    std::vector<std::shared_ptr<GameObject>>& GetGameObjects() { return gameObjects; }

    shared_ptr<GameObject> GetRootGameObject() const 
    { 
        return rootGameObject; 
    }
    
    std::string fileName, fileExtension, directory;
    int processedMeshes = 0;

    //store original texture for later use
    bool useDefaultTexture = false;
    std::unordered_map<std::shared_ptr<Mesh>, std::vector<Texture>> originalTextures;



    std::string fullPath;

    void loadModel(std::string path);
    
   /* void processNode(aiNode* node, const aiScene* scene);*/
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    void processNodeWithGameObjects(aiNode* node, const aiScene* scene, std::shared_ptr<GameObject> parent);
    void createComponentsForMesh(std::shared_ptr<GameObject> gameObject, aiMesh* aiMesh, const aiScene* scene);

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
    Texture GetOrLoadTexture(const string& fullPath, const string& fileName, const string& typeName);
    void AssignDefaultTexture(std::vector<Texture>& textures);

    void LogGameObjectHierarchy(std::shared_ptr<GameObject>  go, int depth);

    void DestroyGameObject(std::shared_ptr<GameObject> gameObject);
    void CleanUpDestroyedObjects();
    std::shared_ptr<GameObject> CreateEmptyGameObject(const std::string& name, std::shared_ptr<GameObject> parent = nullptr);

};