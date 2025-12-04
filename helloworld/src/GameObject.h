#pragma once

#include "Component.h"
#include "TransformComponent.h"
#include "ResourceMesh.h"
#include "ModelImporter.h"
#include <vector>
#include <string>
#include <memory>
#include "UUID.h"


class ModelImporter;

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
    // Constructor
    
    GameObject(const std::string& name, const VroomUUID uuid);
    GameObject(const std::string& name = "GameObject"); 
    ~GameObject(); // Destructor can remain trivial; shared_ptr handles cleanup

    // Update this GameObject and its children
    void Update();

    // Component management
    std::shared_ptr<Component> AddComponent(ComponentType type);
    std::shared_ptr<Component> GetComponent(ComponentType type);
    void RemoveComponent(ComponentType type);
    int GetComponentCount() const { return Components.size(); }

    // Parent/child management
    void SetParent(std::shared_ptr<GameObject> newParent);
    std::shared_ptr<GameObject> GetParent() const;

    const std::vector<std::shared_ptr<GameObject>>& GetChildren() const;
    void AddChild(std::shared_ptr<GameObject> child);
    void RemoveChild(std::shared_ptr<GameObject> child);

    // Active state
    bool IsActive() const { return active; }
    void SetActive(bool isActive);

    bool isSelected = false;

    // Name
    const std::string& GetName() const { return name; }
    void SetName(const std::string& n) { name = n; }

    //UUID
    VroomUUID GetUUID() const { return uuid; }
    void SetUUID(VroomUUID _uuid) { uuid = _uuid; }

    bool IsMarkedForDestroy() const { return markedForDestroy; }
    void MarkForDestroy() { markedForDestroy = true; }
    void SetAsEmpty() {
        isEmpty = true;
    }

    void SetOwnerModel(ModelImporter* model) { 
        ownerModel = model; 
    }

    ModelImporter* GetOwnerModel() { 
        return ownerModel; 
    }

    


private:
    uint64_t uuid;
    std::string name;
    bool active = true;
    bool markedForDestroy = false;
    bool isEmpty = false;

    //store model owner for root objects
    ModelImporter* ownerModel = nullptr;

    std::weak_ptr<GameObject> parent;                          // Weak pointer to parent
    std::vector<std::shared_ptr<GameObject>> children;         // Shared pointers to children
    std::vector<std::shared_ptr<Component>> Components;        // Components are shared_ptr
};
