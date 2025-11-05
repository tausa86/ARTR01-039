#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(vertices = 4) out;
layout(binding = 0) uniform MyUniformData
{
    mat4 mvpMatrix;
    vec4 numberOfLineSegments;
    vec4 numberOfLineStrips;
    vec4 lineColor;
}myUniformData;

void main(void)
{
    // Code
    // Take from RTR code
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    gl_TessLevelOuter[0] = myUniformData.numberOfLineStrips.x;
    gl_TessLevelOuter[1] = myUniformData.numberOfLineSegments.x;
}
