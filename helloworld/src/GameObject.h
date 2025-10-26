#pragma once

#include "Component.h"
#include "TransformComponent.h"
#include "Mesh.h"


class GameObject {

public:
    GameObject(const std::string& name = "GameObject");
    ~GameObject();

    GameObject(const GameObject& other);
    GameObject& operator=(const GameObject& other);

    template <IsComponent T, typename... Args>
    T* AddComponent(Args&&... args);

    template <typename T>
    T* GetComponent() const;

    template <IsComponent T>
    void RemoveComponent();

    template <IsComponent T>
    bool HasComponent() const;
};