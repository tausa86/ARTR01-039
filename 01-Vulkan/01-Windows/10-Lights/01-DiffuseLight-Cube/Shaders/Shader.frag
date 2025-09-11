#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec3 outDiffuseLight;

void main(void)
{
    // Code
    FragColor = vec4(outDiffuseLight, 1.0);  // Lighting
}
