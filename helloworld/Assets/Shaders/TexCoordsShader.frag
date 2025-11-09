#version 460 core
out vec4 FragColor;


in vec2 texCoord;

//The fragment shader should also have access to the texture object, passed with a uniform

struct Material 
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
	sampler2D texture_normal1;
    sampler2D texture_roughness1; 
    sampler2D texture_metallic1; 
    sampler2D texture_ao1; 
};


uniform Material material;


uniform bool drawZbuffer;

float near = 0.1;
float far = 100.0f;

float LinearizeDepth(float depth){
    float z = depth * 2.0f - 1.0f;
    return ((2.0 * near * far) / (far + near - z * (far - near))); 
}

void main()
{
    if(drawZbuffer){
        float depth = LinearizeDepth(gl_FragCoord.z) / far;
        FragColor = vec4(vec3(depth), 1.0);
    } 
    else
    {
        
        FragColor = texture(material.texture_diffuse1, texCoord);
    }


	

}

