#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTexCoords;

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

  // Key pressed related uniform
  uint lKeyIsPressed;

} uMyuniformData;

layout(location = 0) out vec3 outTransformedNormal;
layout(location = 1) out vec3 outLightDirection;
layout(location = 2) out vec3 outViewerVector;

void main(void)
{
    // Code
    gl_Position = uMyuniformData.projectionMatrix * uMyuniformData.viewMatrix * uMyuniformData.modelMatrix * vPosition;
    
    if(1 == uMyuniformData.lKeyIsPressed)
    {
      vec4 iCoordinates = uMyuniformData.viewMatrix * uMyuniformData.modelMatrix * vPosition;
      mat3 normalMatrix = mat3(transpose(inverse(uMyuniformData.viewMatrix * uMyuniformData.modelMatrix)));

      outTransformedNormal = normalMatrix * vNormal;
      outLightDirection = vec3(uMyuniformData.lightPosition - iCoordinates);
      outViewerVector = -iCoordinates.xyz;
    }
    
}
