#pragma once
#include <vector>
#include <memory>
#include <string>
#include <map>
#include "GameObject.h"
#include "ResourceMesh.h"
#include "ResourceManager.h"
#include "Shader.h"
#include "UUID.h"
#include <assimp/scene.h>


// Forward declarations
class ResourceTexture;

class ModelImporter {
public:
    // Storage for meshes (Mesh is now a Resource)
    std::vector<std::shared_ptr<ResourceMesh>> meshes;

     //GameObjects
    std::vector<std::shared_ptr<GameObject>> gameObjects;
    std::shared_ptr<GameObject> modelRootGO;

    //for external file drops
    static std::string lastExternalSourcePath;

    // Model metadata
    std::string fullPath;
    std::string fileName;
    std::string fileExtension;

    // Texture management
    //std::map<std::shared_ptr<ResourceMesh>, std::vector<Texture>> originalTextures;
    std::map<std::shared_ptr<ResourceMesh>, std::vector<std::shared_ptr<ResourceTexture>>> originalTextures;
    bool useDefaultTexture = false;
    std::shared_ptr<ResourceTexture> savedTexture = nullptr;

    // Processing methods
    void processNodeWithGameObjects(const aiNode* node, const aiScene* scene, std::shared_ptr<GameObject> parent, nlohmann::json* modelMeta);

    void createComponentsForMesh(std::shared_ptr<GameObject> gameObject, aiMesh* aiMesh, const aiScene* scene, nlohmann::json* modelMeta, uint meshIndex);

    /*std::shared_ptr<ResourceTexture> GetOrLoadTexture(const std::string& fullPath, const std::string& fileName, const std::string& typeName);*/
    //void AssignDefaultTexture(std::vector<std::shared_ptr<ResourceTexture>>& textures);

public:
    // Constructors
    ModelImporter();
    ModelImporter(std::shared_ptr<ResourceMesh> sharedMesh);
    ~ModelImporter();

    // Import scene from file
    std::shared_ptr<GameObject> ImportScene(const char* path, const std::string& sourcePath = "", bool addToScene = true);

    // Rendering
    void Draw(Shader& shader);

    const std::vector<std::shared_ptr<ResourceMesh>>& GetMeshes() const { return meshes; }

    //struct TexMetaInfo {
    //    std::string name;
    //    VroomUUID uuid;
    //    std::string texType;
    //};

    struct MeshMetaInfo {
        std::string name;
        VroomUUID uuid;
        /*std::vector<TexMetaInfo> textures;*/ //meshes don't store textures anymore, the material does
        uint index; //we need this to identify meshes with sth other than names, in case two are named the same
    };

    std::vector<MeshMetaInfo> meshMetaInfo;
    //td::vector<TexMetaInfo> texMetaInfo; --> should be created for each meshEntry of meshMetaInfo
    
    std::string ogSourcePath;

    void SaveModelMeta(const char* modelPath);
    nlohmann::json* LoadModelMeta(const char* modelPath);  

    // Texture toggle
    void SetUseDefaultTexture(bool use) { useDefaultTexture = use; }
    bool GetUseDefaultTexture() const { return useDefaultTexture; }


};