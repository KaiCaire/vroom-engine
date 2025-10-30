#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include <algorithm>
#include "Log.h"


GameObject::GameObject(const std::string& name)
    : name(name), active(true), parent(nullptr) {
}

GameObject::~GameObject() {
    
    Components.clear(); 

    // Clear all children
    for (GameObject* child : children) {
        if (child) {
            child->parent = nullptr;
        }
    }
}

void GameObject::Update() {
    for (int i = 0; i < Components.size(); i++) {
        Component* comp = Components[i].get();
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
        newComponent = static_cast<Component*>(new TransformComponent(this));
        // AÑADIR AQUÍ:
        LOG("Added TRANSFORM component to GameObject '%s'", name.c_str());
        break;
    case ComponentType::MESH_RENDERER:
        newComponent = static_cast<Component*>(new RenderMeshComponent(this));
        // AÑADIR AQUÍ:
        LOG("Added MESH_RENDERER component to GameObject '%s'", name.c_str());
        break;
    case ComponentType::MATERIAL:
        newComponent = static_cast<Component*>(new MaterialComponent(this));
        // AÑADIR AQUÍ:
        LOG("Added MATERIAL component to GameObject '%s'", name.c_str());
        break;
    default:
        LOG("WARNING: Attempted to add unknown component type to '%s'", name.c_str());
        return nullptr;
    }

    if (newComponent) {
        Components.emplace_back(newComponent);
    }

    return newComponent;
}

Component* GameObject::GetComponent(ComponentType type) {
    for (auto& comp : Components) {
        if (comp && comp->GetType() == type) {
            return comp.get();
        }
    }
    return nullptr;
}

void GameObject::RemoveComponent(ComponentType type) {
    for (auto it = Components.begin(); it != Components.end(); ++it) {
        if ((*it)->GetType() == type) {
            Components.erase(it);  
            return;
        }
    }
}



void GameObject::SetActive(bool isActive) {
    if (active == isActive) return;

    active = isActive;

    // Propagate to children
    for (GameObject* child : children) {
        if (child) {
            child->SetActive(isActive);
        }
    }
}

void GameObject::SetParent(GameObject* newParent) {
    // AÑADIR AQUÍ:
    LOG("Setting parent of '%s' to '%s'",
        name.c_str(),
        newParent ? newParent->GetName().c_str() : "NULL");

    // Remove from current parent
    if (parent) {
        parent->RemoveChild(this);
    }

    parent = newParent;

    // Add to new parent
    if (parent) {
        parent->AddChild(this);
    }
}

void GameObject::AddChild(GameObject* child) {
    if (!child) return;

    // Check if already a child
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) return;

    children.push_back(child);
    child->parent = this;

    // AÑADIR AQUÍ:
    LOG("Added child '%s' to '%s' (Total children: %d)",
        child->GetName().c_str(),
        name.c_str(),
        children.size());
}

void GameObject::RemoveChild(GameObject* child) {
    if (!child) return;

    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        (*it)->parent = nullptr;
        children.erase(it);
    }
}