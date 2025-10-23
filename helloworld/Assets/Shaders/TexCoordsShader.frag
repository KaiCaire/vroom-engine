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



void main()
{
	//built in OpenGL function! -> vec4 texture(sampler2D sampler, vec2 coord);

 	//vec4 diffuse = texture(material.texture_diffuse1, TexCoords);
	FragColor = texture(material.texture_diffuse1, texCoord);

	/*
		// trying to change alpha overtime, isn't working for some reason
		vec4 color1 = texture(tex1, texCoord);
		vec4 color2 = texture(tex2, texCoord);

		FragColor = vec4(
			mix(color1.rgb, color2.rgb, blend),  // blend colors
			mix(color1.a, color2.a, blend)       // blend alpha
		);
	*/

}

//bernat si estas leyendo esto: eres un copion >:v