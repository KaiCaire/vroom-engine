#pragma once

#include <string>
#include <memory>
#include <type_traits>
#include <vector>

class GameObject;

enum class ComponentType {
	NONE,
	TRANSFORM,
	MESH_REDERER,
	MATERIAL

};

class Component {
public:
	
	Component(GameObject* owner, ComponentType type)
		: owner(owner), type(type), active(true) {
	}
	virtual ~Component() = default;

	virtual void Udpdate() {}
	virtual void Enable() {}
	virtual void Disable() {}

	ComponentType GetType() const { return type; }
	bool IsActive() const { return active; }
	GameObject* GetOwner() const { return owner; }

protected:
	GameObject* owner;
	ComponentType type;
	bool active;


};



