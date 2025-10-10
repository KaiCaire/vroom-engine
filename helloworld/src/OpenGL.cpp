#include "OpenGL.h"

#include <iostream>
#include "Log.h"
#include "Application.h"
#include "Window.h"

OpenGL::OpenGL() : Module(), glContext(nullptr), shaderProgram(0), VAO(0), VBO(0) 
{
	std::cout << "OpenGL Constructor" << std::endl;
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
	
	//vertex shader
	const char* vertexShaderSource = 
		"#version 460 core\n"  
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec3 color;\n"
		"out vec3 ourColor;\n"
		"void main()\n"
		"{\n"
			"gl_Position = vec4(position.x, position.y, position.z, 1.0f);\n" //turns it into a homogeneous coordinate so it can be transformed in any way
			"ourColor = color;\n"
		"}\0";

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// 2nd param = how many const chars are you passing
	// 4th param --> glint length = array of string lengths, NULL if strings are null-terminated

	//Vertex Data
	float vertices[] = { //must be defined counterclockwise
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f

		//x,y,z,r,g,b
	};

	//Generate & bind VAO

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);


	//Generate & bind VBO

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Configure vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),  (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);



	std::cout << "OpenGL initialized successfully" << std::endl;


	//Check vertex shader compilation
	int vertexSuccess;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
	if (!vertexSuccess) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "ERROR: Vertex Shader Compilation Failed\n" << infoLog << std::endl;
	}
	else {
		std::cout << "Vertex Shader Compilation Successful!\n" << std::endl;
	}

	//Fragment Shader
	const char* fragmentShaderSource = "#version 460 core\n"
		"out vec4 FragColor;\n"
		"in vec3 ourColor;\n"
		"void main()\n"
		"{\n"
		"	FragColor = vec4(ourColor, 1.0f);\n"
		"}\0";

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	int fragSuccess;
	/*char infoLog[512];*/
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragSuccess);
	if (!fragSuccess) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "ERROR: Fragment Shader Compilation Failed\n" << infoLog << std::endl;
	}
	else {
		std::cout << "Fragment Shader Compilation Successful!\n" << std::endl;
	}

	glDisable(GL_CULL_FACE); //if defined clockwise, will not render

	//check for fragment shader initialization success
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check for linking errors
	int programSuccess;
	
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &programSuccess);
	if (!programSuccess) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "ERROR: Shader Program Linking Failed\n" << infoLog << std::endl;
	}
	else {
		std::cout << "Shader Program Compilation Successful!\n" << std::endl;
	}

	// DEBUG
	int currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
	std::cout << "Current shader program: " << currentProgram << std::endl;

	int attribCount;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attribCount);
	std::cout << "Max vertex attribs: " << attribCount << std::endl;
	//DEBUG END

	 
	// Delete shaders after linking
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return true;
}

bool OpenGL::Update(float dt) {


	glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // dark bluish background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	/*SDL_GL_SwapWindow(Application::GetInstance().window->window);*/


	return true;
}

bool OpenGL::CleanUp() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	return true;
}