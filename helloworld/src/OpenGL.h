#pragma once
#include "Module.h"
#include "Shader.h"

#include "ModelImporter.h"

class OpenGL : public Module {

public:
	OpenGL();
	~OpenGL();

	SDL_GLContext glContext;

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	Shader* GetActiveShader() { return activeShader; }

	bool drawZbuffer = false;


	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;

	Shader* texCoordsShader, * depthBufferShader;

private:
	Shader* activeShader;

};
