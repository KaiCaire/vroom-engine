#version 460 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;


//The fragment shader should also have access to the texture object, passed with a uniform

uniform sampler2D ourTexture;

void main()
{
	//built in OpenGL function!
	//vec4 texture(sampler2D sampler, vec2 coord);

	FragColor = texture(ourTexture, texCoord); 
}
