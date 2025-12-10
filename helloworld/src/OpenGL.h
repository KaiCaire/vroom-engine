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

	Shader* texCoordsShader;
	Shader* depthBufferShader;
	Shader* outlineShader;

	bool drawZbuffer = false;


	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;

	// Dibuja un contorno alrededor de selectedObj usando stencil testing.
	// color: color del contorno; scale: factor de escala del mesh para generar contorno.
	void RenderOutline(std::shared_ptr<GameObject> selectedObj, const glm::vec3& color = glm::vec3(1.0f), float scale = 1.05f);

};
