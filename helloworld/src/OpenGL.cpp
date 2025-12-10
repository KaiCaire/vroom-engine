#include "OpenGL.h"
#include <iostream>
#include "Log.h"
#include "Application.h"
#include "Window.h"
#include "Render.h"
#include "SceneManager.h"
#include "ResourceTexture.h"
#include "stb_image.h"
#include "ModelImporter.h"
#include "Input.h"
#include "Camera.h"
#include "GUIManager.h"

#include "RenderMeshComponent.h"
#include "TransformComponent.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

OpenGL::OpenGL() : Module()
{
	std::cout << "OpenGL Constructor" << std::endl;
	VAO = 0;
	VBO = 1;
	EBO = 2;
	/*shaderProgram = 3;*/
	glContext = NULL;
	outlineShader = nullptr;
}

// Destructor
OpenGL::~OpenGL()
{

}

bool OpenGL::Start() {

	//context = environment in which all OpenGL commands operate
	//we create the context by passing a framebuffer, AKA a block of pixels displayable on a surface

	int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress));

	// … check for errors
	if (version == 0) {
		LOG("Error loading the glad library");
		return false;
	}

	glm::mat4 viewMat = Application::GetInstance().camera->viewMat;
	glm::mat4 projectionMat = Application::GetInstance().camera->projectionMat;

	/*stbi_set_flip_vertically_on_load(true);*/

	Assimp::DefaultLogger::create("Assimp_Log.txt", Assimp::Logger::VERBOSE);
	LOG("Assimp Logger Initialized in OpenGL::Start()");

	texCoordsShader = new Shader("TexCoordsShader.vert", "TexCoordsShader.frag");
	depthBufferShader = new Shader("TexCoordsShader.vert", "DepthBufferShader.frag");

	// Shader para el contorno (simple color)
	outlineShader = new Shader("Outline.vert", "Outline.frag");

	std::cout << "OpenGL initialized successfully" << std::endl;

	glEnable(GL_DEPTH_TEST);

	Application::GetInstance().sceneManager->LoadDefaultScene();

	return true;
}

bool OpenGL::Update(float dt) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE); //if defined clockwise, will not render

	Shader* activeShader = nullptr;

	if (drawZbuffer) {
		activeShader = depthBufferShader;
	}
	else {
		activeShader = texCoordsShader;
	}

	activeShader->Use();

	// Render everything
	Application::GetInstance().render.get()->RenderFrame(*activeShader);

	return true;

}


bool OpenGL::CleanUp() {
	glDeleteVertexArrays(1, &VAO);

	Assimp::DefaultLogger::kill();
	LOG("Assimp Logger Shutdown in OpenGL::CleanUp()");

	return true;
}


// Implementación de RenderOutline basada en LearnOpenGL - Stencil Testing
void OpenGL::RenderOutline(std::shared_ptr<GameObject> selectedObj, const glm::vec3& color, float scale) {
	if (!selectedObj) return;
	if (!texCoordsShader || !outlineShader) return;

	// Obtener componentes necesarios
	auto rendererComp = std::dynamic_pointer_cast<RenderMeshComponent>(selectedObj->GetComponent(ComponentType::MESH_RENDERER));
	if (!rendererComp) return;

	auto mesh = rendererComp->GetMesh();
	if (!mesh) return;

	auto transformComp = std::dynamic_pointer_cast<TransformComponent>(selectedObj->GetComponent(ComponentType::TRANSFORM));
	if (!transformComp) return;

	// Cámara
	auto camera = Application::GetInstance().camera.get();
	if (!camera) return;

	// ----- Primera pasada: dibujar el objeto y escribir 1 en el stencil -----
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF); // permitir escritura en el stencil

	glEnable(GL_DEPTH_TEST);
	texCoordsShader->Use();
	texCoordsShader->setMat4("view", camera->viewMat);
	texCoordsShader->setMat4("projection", camera->projectionMat);

	glm::mat4 model = transformComp->GetModelMatrix();
	texCoordsShader->setMat4("model", model);

	// Dibujar mesh con shader normal para que llene stencil y depth
	mesh->Draw(*texCoordsShader);

	// ----- Segunda pasada: dibujar contorno donde stencil != 1 -----
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00); // deshabilitar escritura en stencil
	glDisable(GL_DEPTH_TEST); // para que el outline sobresalga

	outlineShader->Use();
	outlineShader->setMat4("view", camera->viewMat);
	outlineShader->setMat4("projection", camera->projectionMat);

	// Modelo escalado para el contorno (escala alrededor del origen del modelo)
	glm::mat4 scaledModel = glm::scale(model, glm::vec3(scale));
	outlineShader->setMat4("model", scaledModel);

	// enviar color (usar glad_glGetUniformLocation para compatibilidad con el resto)
	int colorLoc = glad_glGetUniformLocation(outlineShader->ID, "outlineColor");
	if (colorLoc >= 0) {
		glUniform3f(colorLoc, color.r, color.g, color.b);
	}

	// Dibujar mesh con shader de color plano
	mesh->Draw(*outlineShader);

	// Restaurar estado GL
	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}




