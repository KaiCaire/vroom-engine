#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 texCoord;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

void main()
{
	// matrix multiplication works right to left!
	gl_Position = projection * view * model * vec4(aPos, 1.0f); //turns it into a homogeneous coordinate so it can be transformed in any way
	ourColor = aColor;
	texCoord = aTexCoord;
}