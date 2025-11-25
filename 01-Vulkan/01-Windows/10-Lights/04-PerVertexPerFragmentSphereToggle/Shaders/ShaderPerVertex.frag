#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec3 fongADS_light;

layout(location = 0) out vec4 FragColor;

void main(void)
{
    // Code
    FragColor = vec4(fongADS_light, 1.0);  // Lighting
}
