#pragma once
#include "Module.h"
#include "Shader.h"

class OpenGL : public Module {

public:
	OpenGL();
	~OpenGL();

	SDL_GLContext glContext;
	unsigned int shaderProgram;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	Shader* texCoordsShader;

	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;

};
