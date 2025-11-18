#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

layout(location = 0) out vec3 outTransformedNormal;
layout(location = 1) out vec3 outLightDirection;
layout(location = 2) out vec3 outViewerVector;
layout(location = 3) out vec2 out_texcoord;

layout(binding = 0) uniform myuniformData
{
  // Matrices related uniforms
  mat4 modelMatrix;
  mat4 viewMatrix;
  mat4 projectionMatrix;

  // Lighting related uniforms
  // vec4 for alignment or packing as compare to vec3
  vec4 lightAmbient;
  vec4 lightDiffuse;
  vec4 lightSpecular;
  vec4 lightPosition;

  // Material related uniform
  vec4 materialAmbient;
  vec4 materialDiffuse;  
  vec4 materialSpecular;
  float materialShininess;

} uMyuniformData;

void main(void)
{
    // Code
    gl_Position = uMyuniformData.projectionMatrix * uMyuniformData.viewMatrix * uMyuniformData.modelMatrix * vPosition;

    vec4 eyeCoordinates = uMyuniformData.viewMatrix * uMyuniformData.modelMatrix * vPosition;
    mat3 normalMatrix = mat3(transpose(inverse(uMyuniformData.viewMatrix * uMyuniformData.modelMatrix)));

    outTransformedNormal = normalMatrix * vNormal;
    outLightDirection = vec3(uMyuniformData.lightPosition) - eyeCoordinates.xyz;
    outViewerVector = -eyeCoordinates.xyz;
   
    out_texcoord = vTexCoord;
}
