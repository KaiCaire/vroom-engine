#version 460 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;

//The fragment shader should also have access to the texture object, passed with a uniform

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float blend;

void main()
{
	//built in OpenGL function! -> vec4 texture(sampler2D sampler, vec2 coord);

	
	//FragColor = vec4(ourColor, 1.0f);
	//multiplying the texture with the color will result in an overlay
	

	FragColor = mix(texture(tex1, texCoord), texture(tex2, texCoord), 0.5);

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