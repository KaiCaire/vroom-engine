#include "OpenGL.h"
#include <iostream>
#include "Log.h"
#include "Application.h"
#include "Window.h"
#include "Render.h"

OpenGL::OpenGL() : Module()
{
	std::cout << "OpenGL Constructor" << std::endl;
	VAO = 0;
	VBO = 1;
	EBO = 2;
	shaderProgram = 3;
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
	
	//vertex shader
	/*
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
	*/

	texCoordsShader = new Shader("../Assets/Shaders/TexCoordsShader.vert", "../Assets/Shaders/TexCoordsShader.frag");

	//Shader constructor already gets source and compiles
	/*unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &ourShader, NULL);
	glCompileShader(vertexShader);*/
	// 2nd param = how many const chars are you passing
	// 4th param --> glint length = array of string lengths, NULL if strings are null-terminated

	// Vertex Data of a triangle
	float vertices[] = { //must be defined counterclockwise
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f

		//x,y,z,r,g,b
	};

	//rectangle --> repeated vertices

	float rectVertices[] = {
		// first triangle
		 0.5f,  0.5f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f,  // bottom right 
		-0.5f,  0.5f, 0.0f,  // top left 
		// second triangle
		 0.5f, -0.5f, 0.0f,  // bottom right --> duplicate
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left --> duplicate
	};

	//instead make a EBO (vertex + indices)

	float rectVerticesOpt[] = {
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // top right, red
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right, green
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left, blue
		-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f  // top left, white
	};
	unsigned int rectIndicesOpt[] = {  // used to indicate drawing order
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	//Generate & bind VAO

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	


	//Generate & bind VBO

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectVerticesOpt), rectVerticesOpt, GL_STATIC_DRAW);

	//Generate & bind EBO

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndicesOpt), rectIndicesOpt, GL_STATIC_DRAW);

	//Configure vertex attributes

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	//Enable vertex attribute above
	glEnableVertexAttribArray(0);

	//color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	
	glBindVertexArray(0);

	

	std::cout << "OpenGL initialized successfully" << std::endl;

	
	/*
	//Fragment Shader
	const char* fragmentShaderSource = "#version 460 core\n"
		"out vec4 FragColor;\n"
		"uniform vec4 ourColor;\n"
		"void main()\n"
		"{\n"
		"	FragColor = ourColor;\n"
		"}\0";
	*/
	

	/*If you declare a uniform that isn't used anywhere in your GLSL code 
	the compiler will silently remove the variable from the compiled version 
	is the cause for several frustrating errors; keep this in mind!*/
	
	//already done in shader constructor
	/*
	
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);


	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	
	*/

	glDisable(GL_CULL_FACE); //if defined clockwise, will not render

	// DEBUG
	int currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
	std::cout << "Current shader program: " << currentProgram << std::endl;

	int attribCount;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attribCount);
	std::cout << "Max vertex attribs: " << attribCount << std::endl;
	//DEBUG END



	return true;
}

bool OpenGL::Update(float dt) {

	glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // dark bluish background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	float timeValue = SDL_GetTicks() / 1000.0f; // normal GetTicks() returns milliseconds! that's too fast, we want seconds
	float greenValue = (sin(timeValue) / 2.0f) + 0.5f; // green intensity changes (0 to 1 and back) in a 3s interval
	int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
	//glUseProgram(shaderProgram);
	glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
	texCoordsShader->Use();

	/*
	Finding the uniform location does not require you to use the shader program first, 
	but updating a uniform does require you to first use the program (by calling glUseProgram), 
	because it sets the uniform on the currently active shader program.
	*/

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	

	return true;
}

bool OpenGL::CleanUp() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	return true;
}