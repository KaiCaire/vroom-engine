#include "OpenGL.h"
#include <iostream>
#include "Log.h"
#include "Application.h"
#include "Window.h"
#include "Render.h"
#include "Textures.h"
#include "stb_image.h"
#include "Model.h"

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

	stbi_set_flip_vertically_on_load(true);

	

	texCoordsShader = new Shader("TexCoordsShader.vert", "TexCoordsShader.frag");

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


	//Generate textures

	
	texture1.TextureFromFile("../Assets/Textures", "bernat_ivan_meme.jpg");
	texture2.TextureFromFile("../Assets/Textures", "cat_gay.png");
	
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
	

	glDisable(GL_CULL_FACE); //if defined clockwise, will not render

	// DEBUG
	int currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
	std::cout << "Current shader program: " << currentProgram << std::endl;

	int attribCount;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attribCount);
	std::cout << "Max vertex attribs: " << attribCount << std::endl;
	//DEBUG END

	//transformations
	/*
		glm::mat4() by default creates a zero matrix — all elements = 0.
		glm::mat4(1.0f) creates an identity matrix (sets 1s all across the diagonal)
	*/

	//load model
	//3D transformation matrices  --> Vclip = Mprojection⋅Mview⋅Mmodel⋅Vlocal
	 
	modelMat = glm::mat4(1.0f);
	modelMat = glm::rotate(modelMat, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //transforms vertex coordinates into world coordinates.
	//^rotates on the x axis so it looks like laying on the floor

	texCoordsShader->Use();
	uint modelMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "model");
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
	
	viewMat = glm::mat4(1.0f);
	// translate scene in the reverse direction of moving direction
	viewMat = glm::translate(viewMat, glm::vec3(0.0f, 0.0f, -3.0f));
	//OpenGL = righthanded system --> move cam in  positive z-axis (= translate scene towards negative z-axis)
	texCoordsShader->Use();
	uint viewMatLoc = glad_glGetUniformLocation(texCoordsShader->ID, "view");
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));

	;
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
	 //don't forget to activate/use the shader before setting uniforms!
	glUniform1i(glGetUniformLocation(texCoordsShader->ID, "tex1"), 0);
	// or set it via the texture class
	texCoordsShader->setInt("tex2", 1);


	/*warriorModel = new Model("../Assets/Models/warrior.FBX");*/

	return true;
}

bool OpenGL::Update(float dt) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // dark bluish background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	

	/*warriorModel->Draw(*texCoordsShader);*/

	/*
	Finding the uniform location does not require you to use the shader program first, 
	but updating a uniform does require you to first use the program (by calling glUseProgram), 
	because it sets the uniform on the currently active shader program.
	*/
	
	//float timeValue = SDL_GetTicks() / 1000.0f;
	//trans = glm::mat4(1.0f); //reset the matrix, otherwise you'll accumulate rotations (accelerating)
	//float scalingFactor = sin(timeValue)/2.0f + 1.0f;
	//trans = glm::scale(trans, glm::vec3(scalingFactor, scalingFactor, scalingFactor));
	///*trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));*/
	//trans = glm::rotate(trans, timeValue, glm::vec3(0.0, 0.0, 1.0)); //rotate around Z axis (we're on XY plane)
	///*trans = glm::scale(trans, glm::vec3(2, 2, 2));*/

	//float alphaValue = 1;
	
	//int blendLocation = glGetUniformLocation(texCoordsShader->ID, "blend"); // get uniform location
	//if (blendLocation != -1) {

	//	alphaValue = timeValue * 0.5f;         // adjust speed (0.5 = takes 2s to reach 1)
	//	if (alphaValue > 1.0f) alphaValue = 1.0f;   // clamp to 1.0
	//}

	//uint transformLoc = glad_glGetUniformLocation(texCoordsShader->ID, "transform");
	//
	//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
	//glUniform1f(blendLocation, alphaValue);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1.id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2.id);

	texCoordsShader->Use();

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


	return true;
}


bool OpenGL::CleanUp() {
	glDeleteVertexArrays(1, &VAO);

	return true;
}