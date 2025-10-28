#pragma once
#include "Module.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class Camera : public Module 
{
public:

	Camera();

	//Destructor
	virtual ~Camera();

	bool Start();
	bool Update(float dt);
};