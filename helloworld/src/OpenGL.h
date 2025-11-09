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

	/*Texture texture1, texture2;*/
	int width, height, nChannels;
	glm::mat4 modelMat, viewMat, projectionMat;
	glm::vec3* cubePositions = new glm::vec3[10];

	bool drawZbuffer = false;
	Shader* texCoordsShader;
	Shader* singleColorShader;

	Model* ourModel;
	vector<Model*> modelObjects;
	void SetUpVertShader(Shader* shader);
	void SetUpViewProjection(Shader* shader);

	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;

	Model* CreateCube();
};
