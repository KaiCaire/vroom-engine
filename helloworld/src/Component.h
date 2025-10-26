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
	MATERIAL,
	CAMERA,
	SHADER,

};

class Component {
public:
	explicit Component(GameObject* owner) : owner(owner) {}
	virtual ~Component() = default;
};

template <typename T>
concept IsComponent = std::is_base_of_v<Component, T>;

