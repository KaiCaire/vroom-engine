#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include <algorithm>


GameObject::GameObject(const std::string& name)
    : name(name), active(true), parent(nullptr) {
}

GameObject::~GameObject() {
    // Delete all components
    for (Component* component : Components) {
        delete component;
    }
    Components.clear();

    // Clear all children (they should be managed elsewhere)
    for (GameObject* child : children) {
        if (child) {
            child->parent = nullptr;
        }
    }
}

void GameObject::Update() {
    for (int i = 0; i < Components.size(); i++) {
        Component* comp = Components[i];
        if (comp->IsActive())
            comp->Update();
    }

    for (auto child : children)
        child->Update();
}

Component* GameObject::AddComponent(ComponentType type) {
    Component* newComponent = nullptr;

    switch (type) {
    case ComponentType::TRANSFORM: 
        newComponent = new TransformComponent(this);
        break;
    case ComponentType::MESH_REDERER:
        newComponent = new RenderMeshComponent(this);
        break;
    case ComponentType::MATERIAL:
        newComponent = new MaterialComponent(this);
        break;
    default:
        return nullptr;
    }

    if (newComponent) {
        Components.emplace_back(newComponent);
    }

    return newComponent;

}

Component* GameObject::GetComponent(ComponentType type) {
    for (auto& comp : Components)
        if (comp->GetType() == type)
            return comp.get();
    return nullptr;
}

void GameObject::RemoveComponent(ComponentType type) {
    Components.erase(std::remove_if(Components.begin(), Components.end(),
        [type](const std::unique_ptr<Component>& comp) {
            return comp->GetType() == type;
        }),
        Components.end());
}

