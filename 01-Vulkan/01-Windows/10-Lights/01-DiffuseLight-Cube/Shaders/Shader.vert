#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

layout(binding = 0) uniform myuniformData
{
  // Matrices related uniforms
  mat4 modelMatrix;
  mat4 viewMatrix;
  mat4 projectionMatrix;

  // Lighting related uniforms
  // vec4 for alignment or packing as compare to vec3
  vec4 lightDiffuse;
  vec4 lightPosition;
  vec4 materialDiffuse;

  // Key pressed related uniform
  uint lKeyIsPressed;

} uMyuniformData;

layout(location = 0) out vec3 outDiffuseLight;

void main(void)
{
    // Code
    gl_Position = uMyuniformData.projectionMatrix * uMyuniformData.viewMatrix * uMyuniformData.modelMatrix * vPosition;
    
    if(1 == uMyuniformData.lKeyIsPressed)
    {
      vec4 iCoordinates = uMyuniformData.viewMatrix * uMyuniformData.modelMatrix * vPosition;
      mat3 normalMatrix = mat3(transpose(inverse(uMyuniformData.viewMatrix * uMyuniformData.modelMatrix)));
      vec3 transformedNormal = normalize(normalMatrix * vNormal);
      vec3 lightSource = normalize(vec3(uMyuniformData.lightPosition - iCoordinates));

      outDiffuseLight = vec3(uMyuniformData.lightDiffuse) * vec3(uMyuniformData.materialDiffuse) * max(dot(lightSource, transformedNormal), 0.0);
    }
    else
    {
      outDiffuseLight = vec3(1.0, 1.0, 1.0);  // White light
    }
}
