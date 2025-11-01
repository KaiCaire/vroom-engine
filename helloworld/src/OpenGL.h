#pragma once
#include "Module.h"
#include "Shader.h"

#include "Model.h"

class OpenGL : public Module {

public:
	OpenGL();
	~OpenGL();

	SDL_GLContext glContext;
	/*unsigned int shaderProgram;*/
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	//Camera
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	glm::vec3 targetPos;

	/*Texture texture1, texture2;*/
	int width, height, nChannels;
	glm::mat4 modelMat, viewMat, projectionMat;
	glm::vec3* cubePositions = new glm::vec3[10];
	bool firstMouse = true;
	float lastX = 400, lastY = 300, xpos, ypos, yaw = -90.0f, pitch = 0.0f, fov = 45.0f, cameraSpeed, distance;

	Shader* texCoordsShader;
	Model* ourModel;

	/*float fullRadRot = 2 * (float)SDL_PI_F;*/

	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;

	void ProcessMouseRotation(float xoffset, float yoffset, float sensitivity);
	void UpdateCameraVectors();
	void ProcessKeyboardMovement(float actualSpeed);
	void ProcessPan(float xoffset, float yoffset);
	void ProcessDolly(float xoffset, float yoffset);
	void ProcessScrollZoom(float delta, bool isMouseScroll);
	void FocusObject();

};
