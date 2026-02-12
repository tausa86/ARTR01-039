#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec2 vPosition;

layout(binding = 0) uniform MyUniformData
{
    mat4 mvpMatrix;
}myUniformData;

void main(void)
{
    // Code
    gl_Position = myUniformData.mvpMatrix * vec4(vPosition, 0.0, 1.0);
    //gl_Position = vec4(vPosition, 0.0, 1.0);
}
