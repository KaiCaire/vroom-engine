#include "OpenGL.h"
#include <iostream>
#include "Log.h"
#include "Application.h"
#include "Window.h"
#include "Render.h"
#include "Textures.h"
#include "stb_image.h"

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

	texCoordsShader = new Shader("../Assets/Shaders/TexCoordsShader.vert", "../Assets/Shaders/TexCoordsShader.frag");

	//Shader constructor already gets source and compiles
	

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
	//UPDATE: added texCoords
	float rectVerticesOpt[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
	};
	unsigned int rectIndicesOpt[] = {  // used to indicate drawing order
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};


	//Generate texture
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);


	//Setting various texture parameters:

	//Texture-Wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //S = X axis in texCoords
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); //T = Y axis texCoords

	//default texture repeat mode is GL_REPEAT
	//if using GL_CLAMP_TO_BORDER, specify border color with:
	/*
		float borderColo[] = { 1.0f, 1.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	*/

	//Filtering mode- -> GL_NEAREST = blocky pattern (default) || GL_LINEAR = smoother pattern
	// can be set separately for minifying or magnifying operations:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// mip maps are only implemented in downscaling! don't filter mipmaps with MAG_FILTER 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	//Load texture
	int width, height, nChannels;

	unsigned char* data = Application::GetInstance().textures.get()->LoadTexture("container.jpg", &width, &height, &nChannels);
	
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);


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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	//Enable vertex attribute above
	glEnableVertexAttribArray(0);

	//color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	

	//texCoords attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	glBindVertexArray(0);

	

	std::cout << "OpenGL initialized successfully" << std::endl;

	

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

	/* flickering neon green rect:
	
	float timeValue = SDL_GetTicks() / 1000.0f; // normal GetTicks() returns milliseconds! that's too fast, we want seconds
	float greenValue = (sin(timeValue) / 2.0f) + 0.5f; // green intensity changes (0 to 1 and back) in a 3s interval
	int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
	//glUseProgram(shaderProgram);
	glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

	*/

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

	return true;
}