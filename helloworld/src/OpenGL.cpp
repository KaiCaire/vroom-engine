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
#include "MaterialComponent.h"

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

	//texCoordsShader->Use();
	//viewMat = glm::mat4(1.0f);
	

	return true;
}

bool OpenGL::Update(float dt) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// clean stencil and depth buffer every frame
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glDisable(GL_CULL_FACE); //if defined clockwise, will not render

	activeShader = nullptr;

	if (drawZbuffer) {
		activeShader = depthBufferShader;
		activeShader->Use();
		activeShader->setFloat("near", Application::GetInstance().camera->nearPlane);
		activeShader->setFloat("far", Application::GetInstance().camera->farPlane / 5);
		
	}
	else {
		activeShader = texCoordsShader;
		activeShader->Use();
		/*glUniform1f(glad_glGetUniformLocation(activeShader->ID, "near"), Application::GetInstance().camera->nearPlane);
		glUniform1f(glad_glGetUniformLocation(activeShader->ID, "far"), Application::GetInstance().camera->farPlane);*/
	}
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



