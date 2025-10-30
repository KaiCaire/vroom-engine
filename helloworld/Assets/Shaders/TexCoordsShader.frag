#version 460 core
out vec4 FragColor;

//in vec3 ourColor;
in vec2 texCoord;

//The fragment shader should also have access to the texture object, passed with a uniform

struct Material 
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
	sampler2D texture_normal1;
    sampler2D texture_roughness1; 
    sampler2D texture_ao1; 
};


uniform Material material;
uniform bool useLineColor;
uniform vec4 lineColor;



void main()
{
	
	if (useLineColor) FragColor = lineColor;
	else FragColor = texture(material.texture_diffuse1, texCoord);

}

