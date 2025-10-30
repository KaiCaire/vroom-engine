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
	Component(std::shared_ptr<GameObject> owner, ComponentType type);
	virtual ~Component() = default;

	virtual void Update() {}
	virtual void Enable() {}
	virtual void Disable() {}

	virtual void OnEditor() {}

	ComponentType GetType() const { return type; }
	bool IsActive() const { return active; }
	std::shared_ptr<GameObject> GetOwner() const 
	{ 
		return owner.lock(); 
	}

	void SetActive(bool isActive);

protected:
	std::weak_ptr<GameObject> owner;
	ComponentType type;
	bool active;


};


#endif // !_COMPONENT_H_

