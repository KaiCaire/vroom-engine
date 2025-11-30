#pragma once
#include <memory>
#include <string>
#include <vector>
#include "GameObject.h"
#include <nlohmann/json.hpp>
#include "Module.h"

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

    void SetName(const std::string& name) { sceneName = name; }

private:
    std::string sceneName;
    std::shared_ptr<GameObject> root;
    std::vector<std::shared_ptr<GameObject>> allGameObjects;

    // Serialization helpers
    nlohmann::json SerializeGameObject(std::shared_ptr<GameObject> go);
    std::shared_ptr<GameObject> DeserializeGameObject(const nlohmann::json& j);
    void CollectAllGameObjects(std::shared_ptr<GameObject> go);

    void LogGameObjectHierarchy(std::shared_ptr<GameObject> go, int depth);
};