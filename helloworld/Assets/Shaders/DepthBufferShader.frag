#version 460 core
out vec4 FragColor;


uniform float near = 0.1f;
uniform float far = 100.0f;

float LinearizeDepth(float depth){
    float z = depth * 2.0f - 1.0f;
    return ((2.0 * near * far) / (far + near - z * (far - near))); 
}

void main()
{
    float depth = LinearizeDepth(gl_FragCoord.z) / far;
       
    //depth = grayscale intensity (R, G, and B are all 'depth')
    FragColor = vec4(vec3(depth), 1.0);
}
