#include "OpenGL.h"
#include <iostream>
#include "Log.h"
#include "Application.h"
#include "Window.h"
#include "Render.h"
#include "Textures.h"
#include "stb_image.h"
#include "Model.h"
#include "Input.h"

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

	/*stbi_set_flip_vertically_on_load(true);*/

	

	texCoordsShader = new Shader("TexCoordsShader.vert", "TexCoordsShader.frag");

	std::cout << "OpenGL initialized successfully" << std::endl;


	/*If you declare a uniform that isn't used anywhere in your GLSL code
	the compiler will silently remove the variable from the compiled version
	is the cause for several frustrating errors; keep this in mind!*/
	//3D transformation matrices  --> Vclip = Mprojection⋅Mview⋅Mmodel⋅Vlocal

	modelMat = glm::mat4(1.0f);
	modelMat = glm::rotate(modelMat, glm::radians(45.0f), glm::vec3(0.0f, -1.0f, 0.0f)); //transforms vertex coordinates into world coordinates.
	//^rotates on the x axis so it looks like laying on the floor
	/*modelMat = glm::scale(modelMat, glm::vec3(0.05, 0.05, 0.05));*/

	texCoordsShader->Use();
	uint modelMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "model");
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	viewMat = glm::mat4(1.0f);
	// translate scene in the reverse direction of moving direction
	viewMat = glm::translate(viewMat, glm::vec3(0.0f, -2.0f, -15.0f));
	
	//OpenGL = righthanded system --> move cam in  positive z-axis (= translate scene towards negative z-axis)
	texCoordsShader->Use();
	uint viewMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "view");
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));


	//projection mat = perspective (FOV, aspectRatio, nearPlane, farPlane)
	int windowW, windowH;
	Application::GetInstance().window.get()->GetSize(windowW, windowH);

	projectionMat = glm::mat4(1.0f);
	projectionMat = glm::perspective(glm::radians(45.0f), (float)windowW / windowH, 0.1f, 100.0f);
	texCoordsShader->Use();
	uint projectionMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "projection");
	glUniformMatrix4fv(projectionMatLoc, 1, GL_FALSE, glm::value_ptr(projectionMat));

	glEnable(GL_DEPTH_TEST);

	texCoordsShader->Use();
	// //don't forget to activate/use the shader before setting uniforms!
	//glUniform1i(glGetUniformLocation(texCoordsShader->ID, "tex1"), 0);
	//// or set it via the texture class
	//texCoordsShader->setInt("tex2", 1);

	std::string modelPath = "../Assets/Models/BakerHouse/BakerHouse.fbx"; 
	

	ourModel = new Model(modelPath.c_str());

	Application::GetInstance().render.get()->AddModel(*ourModel);
	

	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	viewMat = glm::mat4(1.0f);


	return true;
}

bool OpenGL::Update(float dt) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // dark bluish background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	glDisable(GL_CULL_FACE); //if defined clockwise, will not render


	//grid

	glUseProgram(texCoordsShader->ID);

	//use shader's line color instead of texture
	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), true);
	glUniform4f(glGetUniformLocation(texCoordsShader->ID, "lineColor"), 1.0f, 0.0f, 1.0f, 1.0f); 

	Application::GetInstance().render.get()->DrawGrid();
	
	// Restore to normal texture mode
	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "useLineColor"), false);

	
	

	//camera controls
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
		cameraSpeed = 0.10f;
	else
		cameraSpeed = 0.05f;


	xpos = Application::GetInstance().input.get()->GetMousePosition().x;
	ypos = Application::GetInstance().input.get()->GetMousePosition().y;

	if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT) {
		if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
			cameraPos += cameraSpeed * cameraFront;
		if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
			cameraPos -= cameraSpeed * cameraFront;
		if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (firstMouse)
		{
			lastX = Application::GetInstance().input.get()->GetMousePosition().x;
			lastY = Application::GetInstance().input.get()->GetMousePosition().y;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(direction);
	}

	if (!firstMouse && Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
		firstMouse = true;
	}

	int scrollDelta = Application::GetInstance().input.get()->GetMouseWheelDeltaY();

	if (scrollDelta != 0)
	{
		float zoomSpeed = 2.0f;

		fov -= (float)scrollDelta * zoomSpeed;

		if (fov < 1.0f) fov = 1.0f;
		if (fov > 90.0f) fov = 90.0f;
	}
	projectionMat = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
	Application::GetInstance().input.get()->SetMouseWheelDeltaY(0);


	viewMat = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	texCoordsShader->Use();
	texCoordsShader->setMat4("model", modelMat);
	texCoordsShader->setMat4("view", viewMat);
	texCoordsShader->setMat4("projection", projectionMat);


	
	for (int i = 0; i < Application::GetInstance().render.get()->modelsToDraw.size(); i++) {
		Application::GetInstance().render.get()->modelsToDraw[i].Draw(*texCoordsShader);
	}


	return true;

}


bool OpenGL::CleanUp() {
	glDeleteVertexArrays(1, &VAO);

	return true;
}