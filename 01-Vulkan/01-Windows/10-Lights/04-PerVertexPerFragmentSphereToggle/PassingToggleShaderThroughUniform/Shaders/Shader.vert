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

  // Material related uniform
  vec4 materialAmbient;
  vec4 materialDiffuse;  
  vec4 materialSpecular;
  float materialShininess;

  // Key pressed related uniform
  uint lKeyIsPressed;
  uint perFragmentLighting;

} uMyuniformData;

layout(location = 0) out vec3 outTransformedNormal;
layout(location = 1) out vec3 outLightDirection;
layout(location = 2) out vec3 outViewerVector;
layout(location = 3) out vec3 outFongADSLight;

void main(void)
{
    // Code
    gl_Position = uMyuniformData.projectionMatrix * uMyuniformData.viewMatrix * uMyuniformData.modelMatrix * vPosition;
    
    if(1 == uMyuniformData.lKeyIsPressed)
    {
      vec4 eyeCoordinates = uMyuniformData.viewMatrix * uMyuniformData.modelMatrix * vPosition;
      mat3 normalMatrix = mat3(transpose(inverse(uMyuniformData.viewMatrix * uMyuniformData.modelMatrix)));
      if(1 == uMyuniformData.perFragmentLighting)
      {  
        outTransformedNormal = normalMatrix * vNormal;
        outLightDirection = vec3(uMyuniformData.lightPosition) - eyeCoordinates.xyz;
        outViewerVector = -eyeCoordinates.xyz;
      }
      else
      {
        vec3 transformedNormal = normalMatrix * vNormal;
        vec3 lightDirection = vec3(uMyuniformData.lightPosition) - eyeCoordinates.xyz;
        vec3 viewerVector = -eyeCoordinates.xyz;

        vec3 normalizedTransformedNormal = normalize(transformedNormal);
        vec3 normalizedLightDirection = normalize(lightDirection);
        vec3 normalizedViewerVector = normalize(viewerVector);

        vec3 ambientLight = vec3(uMyuniformData.lightAmbient) * vec3(uMyuniformData.materialAmbient);
        vec3 diffuseLight = vec3(uMyuniformData.lightDiffuse) * vec3(uMyuniformData.materialDiffuse) * max(dot(normalizedLightDirection, normalizedTransformedNormal), 0.0);
        vec3 reflectionVector = reflect(-normalizedLightDirection, normalizedTransformedNormal);
        vec3 specularLight = vec3(uMyuniformData.lightSpecular) * vec3(uMyuniformData.materialSpecular) * pow(max(dot(reflectionVector, normalizedViewerVector), 0.0), uMyuniformData.materialShininess);
        outFongADSLight = ambientLight + diffuseLight + specularLight; 
      }
    } 
    else
    {
      if(1 == uMyuniformData.perFragmentLighting)
      {
        outFongADSLight = vec3(1.0, 1.0, 1.0);
      }
    }   
}
