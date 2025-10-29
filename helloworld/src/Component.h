#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include <string>
#include <memory>
#include <type_traits>
#include <vector>

class GameObject;

enum class ComponentType {
	NONE,
	TRANSFORM,
	MESH_RENDERER,
	MATERIAL

};

class Component {
public:

	virtual ~Component() = default;

	virtual void Update() {}
	virtual void Enable() {}
	virtual void Disable() {}

	virtual void OnEditor() {}

	ComponentType GetType() const { return type; }
	bool IsActive() const { return active; }
	GameObject* GetOwner() const { return owner; }

	void SetActive(bool isActive);

protected:
	GameObject* owner;
	ComponentType type;
	bool active;


};


#endif // !_COMPONENT_H_

