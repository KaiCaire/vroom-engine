#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "GameObject.h"
#include <nlohmann/json.hpp>
#include "Module.h"
#include "Octree.h"

class GameObject;

class Scene : public Module {
public:
    Scene(const std::string& name = "SampleScene");
    ~Scene();

    // Scene file operations
    void Clear();
    bool SaveScene(const std::string& filePath);
    bool LoadScene(const std::string& filePath);

    // GameObject management
   
    void AddGameObject(std::shared_ptr<GameObject> go);
    void RemoveGameObject(std::shared_ptr<GameObject> go);
    
    void CleanUpDestroyedObjects();

    // Import 3D model into scene (calls SceneImporter internally)
    std::shared_ptr<GameObject> ImportModel(const std::string& modelPath);

    // Getters
    std::shared_ptr<GameObject> GetRoot() const { return root; }

    const std::vector<std::shared_ptr<GameObject>>& GetAllGameObjects() const { return allGameObjects; }

    std::string GetName() const { return sceneName; }

    Octree* GetOctree() {
        return octree.get();
    }

    void SetName(const std::string& name) { sceneName = name; }

    // Find by UUID 
    std::shared_ptr<GameObject> FindGameObjectByUUID(VroomUUID uuid);

    //Find by name:
    std::shared_ptr<GameObject> FindGameObjectByName(const std::string name);

    std::string FindModelInAssetsFolder(std::string modelName);


private:
    std::string sceneName;
    std::shared_ptr<GameObject> root;
    std::vector<std::shared_ptr<GameObject>> allGameObjects;
    std::unordered_set<std::string> reimportedModels;

    // Serialization helpers
    nlohmann::json SerializeGameObject(std::shared_ptr<GameObject> go);
    std::shared_ptr<GameObject> DeserializeGameObject(const nlohmann::json& j, const std::string sourceModelName);
    void CollectAllGameObjects(std::shared_ptr<GameObject> go);

    void LogGameObjectHierarchy(std::shared_ptr<GameObject> go, int depth);

    std::unique_ptr<Octree> octree;
    AABB worldBounds;
};