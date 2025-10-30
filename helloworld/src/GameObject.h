#pragma once

#include "Component.h"
#include "TransformComponent.h"
#include "Mesh.h"
#include <vector>
#include <string>


class GameObject {

public:
	GameObject(const std::string& name = "GameObject");
	~GameObject();


	void Update();

	Component* AddComponent(ComponentType type);
	Component* GetComponent(ComponentType type);
	void RemoveComponent(ComponentType type);

	void SetParent(GameObject* newParent);
	GameObject* GetParent() const { return parent; }
	const std::vector<GameObject*>& GetChildren() const { return children; }
	void AddChild(GameObject* child);
	void RemoveChild(GameObject* child);

	bool IsActive() const { return active; }
	void SetActive(bool isActive);
	
	const std::string& GetName() const { return name; }
	void SetName(const std::string& n) { name = n; }
	
private:
	std::string name;
	bool active = true;

	GameObject* parent = nullptr;
	std::vector<GameObject*> children;

	std::vector<std::unique_ptr<Component>> Components;

};