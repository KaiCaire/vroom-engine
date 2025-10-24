#pragma once
#include "Module.h"
#include "Camera.h"

Camera::Camera() : Module()
{
	name = "camera";
}

bool Camera::Start()
{
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

	glm::mat4 view;
	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	return true;
}

// Destructor
Camera::~Camera()
{
}

bool Camera::Update(float dt)
{

	const float radius = 10.0f;
	float camX = (float)sin(SDL_GetTicks() / 1000) * radius;
	float camZ = (float)cos(SDL_GetTicks() / 1000) * radius;
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	return true;
}