#pragma once

#include "Component.h"
#include "TransformComponent.h"
#include "Mesh.h"
#include <vector>


class GameObject {

public:

	void Update();
	
private:
	std::vector<Component> Components;


};