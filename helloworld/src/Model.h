#pragma once
#include <vector>
#include <memory>
#include <string>
#include <map>
#include "GameObject.h"
#include "ResourceMesh.h"
#include "Shader.h"
#include "UUID.h"
#include <assimp/scene.h>


// Forward declarations
class ResourceTexture;

class Model {
public:
    // Storage for meshes (Mesh is now a Resource)
    std::vector<std::shared_ptr<ResourceMesh>> meshes;

    // GameObjects
    std::vector<std::shared_ptr<GameObject>> gameObjects;
    std::shared_ptr<GameObject> rootGameObject;

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
    void processNodeWithGameObjects(const aiNode* node, const aiScene* scene, std::shared_ptr<GameObject> parent);

    void createComponentsForMesh(std::shared_ptr<GameObject> gameObject, aiMesh* aiMesh, const aiScene* scene);

    std::shared_ptr<ResourceTexture> GetOrLoadTexture(const std::string& fullPath, const std::string& fileName, const std::string& typeName);
    void AssignDefaultTexture(std::vector<std::shared_ptr<ResourceTexture>>& textures);

public:
    // Constructors
    Model();
    Model(std::shared_ptr<ResourceMesh> sharedMesh);
    ~Model();

    // Import scene from file
    void ImportScene(const char* path);

    // Rendering
    void Draw(Shader& shader);

    // GameObject management
    std::shared_ptr<GameObject> CreateEmptyGameObject(const std::string& name,
        std::shared_ptr<GameObject> parent = nullptr);

    std::shared_ptr<GameObject> CreateGameObject(const std::string& name, VroomUUID meshUID, std::shared_ptr<GameObject> parent,
        const glm::vec3& position,
        const glm::quat& rotation,
        const glm::vec3& scale);

    void DestroyGameObject(std::shared_ptr<GameObject> gameObject);
    void CleanUpDestroyedObjects();

    // Hierarchy debugging
    void LogGameObjectHierarchy(std::shared_ptr<GameObject> go, int depth);

    // Getters
    std::shared_ptr<GameObject> GetRootGameObject() const { return rootGameObject; }
    const std::vector<std::shared_ptr<GameObject>>& GetGameObjects() const { return gameObjects; }
    const std::vector<std::shared_ptr<ResourceMesh>>& GetMeshes() const { return meshes; }

    // Texture toggle
    void SetUseDefaultTexture(bool use) { useDefaultTexture = use; }
    bool GetUseDefaultTexture() const { return useDefaultTexture; }
};