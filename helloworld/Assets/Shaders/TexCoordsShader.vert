
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
out vec3 ourColor;

void main()
{
	gl_Position = vec4(position.x, position.y, position.z, 1.0f); //turns it into a homogeneous coordinate so it can be transformed in any way
	ourColor = color;
}