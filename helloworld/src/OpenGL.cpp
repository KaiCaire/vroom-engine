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

	/*normalShader = new Shader("")*/

	std::cout << "OpenGL initialized successfully" << std::endl;


	/*If you declare a uniform that isn't used anywhere in your GLSL code
	the compiler will silently remove the variable from the compiled version
	is the cause for several frustrating errors; keep this in mind!*/
	//3D transformation matrices  --> Vclip = Mprojection⋅Mview⋅Mmodel⋅Vlocal

	//glm::mat4 modelMat = glm::mat4(1.0f);
	//modelMat = glm::rotate(modelMat, glm::radians(45.0f), glm::vec3(0.0f, -1.0f, 0.0f)); //transforms vertex coordinates into world coordinates.
	////^rotates on the x axis so it looks like laying on the floor
	///*modelMat = glm::scale(modelMat, glm::vec3(0.05, 0.05, 0.05));*/

	//texCoordsShader->Use();
	//uint modelMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "model");
	//glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	//viewMat = glm::mat4(1.0f);
	//// translate scene in the reverse direction of moving direction
	//viewMat = glm::translate(viewMat, glm::vec3(0.0f, -2.0f, -15.0f));

	////OpenGL = righthanded system --> move cam in  positive z-axis (= translate scene towards negative z-axis)
	//texCoordsShader->Use();
	//uint viewMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "view");
	//glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));


	////projection mat = perspective (FOV, aspectRatio, nearPlane, farPlane)
	//int windowW, windowH;
	//Application::GetInstance().window.get()->GetSize(windowW, windowH);

	//projectionMat = glm::mat4(1.0f);
	//projectionMat = glm::perspective(glm::radians(45.0f), (float)windowW / windowH, 0.1f, 100.0f);
	//texCoordsShader->Use();
	//uint projectionMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "projection");
	//glUniformMatrix4fv(projectionMatLoc, 1, GL_FALSE, glm::value_ptr(projectionMat));

	glEnable(GL_DEPTH_TEST);

	texCoordsShader->Use();
	viewMat = glm::mat4(1.0f);


	Application::GetInstance().sceneManager->LoadDefaultScene();

	return true;
}

bool OpenGL::Update(float dt) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // dark bluish background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	glDisable(GL_CULL_FACE); //if defined clockwise, will not render



	////grid

	//glUseProgram(texCoordsShader->ID);

	////use shader's line color instead of texture
	//glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), true);
	//glUniform4f(glGetUniformLocation(texCoordsShader->ID, "lineColor"), 1.0f, 1.0f, 1.0f, 0.5f); //white grid

	//Application::GetInstance().render.get()->DrawGrid(*texCoordsShader);

	//// Restore to normal texture mode
	//glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), false);

	//viewMat = Application::GetInstance().camera->viewMat;
	//projectionMat = Application::GetInstance().camera->projectionMat;

	//texCoordsShader->Use();
	//texCoordsShader->setMat4("model", modelMat);
	//texCoordsShader->setMat4("view", viewMat);
	//texCoordsShader->setMat4("projection", projectionMat);

	//draw all meshes
	
	//TODO:
	//for (int i = 0; i < Application::GetInstance().render.get()->modelsToDraw.size(); i++) {
	//	Application::GetInstance().render.get()->modelsToDraw[i]->Draw(*texCoordsShader);
	//}



	// Let Render module handle all drawing
	Application::GetInstance().render.get()->RenderFrame(*texCoordsShader);

	return true;


	return true;

}


bool OpenGL::CleanUp() {
	glDeleteVertexArrays(1, &VAO);

	Assimp::DefaultLogger::kill();
	LOG("Assimp Logger Shutdown in OpenGL::CleanUp()");

	return true;
}




