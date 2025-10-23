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

	Texture texture1, texture2;
	int width, height, nChannels;
	glm::mat4 modelMat, viewMat, projectionMat;


	Shader* texCoordsShader;
	Model* warriorModel;

	float fullRadRot = 2 * (float)SDL_PI_F;

	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;

};
