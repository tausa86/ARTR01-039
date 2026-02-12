#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 outTransformedNormal;
layout(location = 1) in vec3 outLightDirection;
layout(location = 2) in vec3 outViewerVector;
layout(location = 3) in vec2 out_texcoord;

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

layout(binding = 1) uniform sampler2D uTextureSampler;

void main(void)
{
    // Code
    vec3 fongADS_light;
        vec3 normalizedTransformedNormal = normalize(outTransformedNormal);
        vec3 normalizedLightDirection = normalize(outLightDirection);
        vec3 normalizedViewerVector = normalize(outViewerVector);

        vec3 ambientLight = vec3(uMyuniformData.lightAmbient) * vec3(uMyuniformData.materialAmbient);
        vec3 diffuseLight = vec3(uMyuniformData.lightDiffuse) * vec3(uMyuniformData.materialDiffuse) * max(dot(normalizedLightDirection, normalizedTransformedNormal), 0.0);
        vec3 reflectionVector = reflect(-normalizedLightDirection, normalizedTransformedNormal);
        vec3 specularLight = vec3(uMyuniformData.lightSpecular) * vec3(uMyuniformData.materialSpecular) * pow(max(dot(reflectionVector, normalizedViewerVector), 0.0), uMyuniformData.materialShininess);
        fongADS_light = ambientLight + diffuseLight + specularLight;
    
    FragColor = vec4(fongADS_light, 1.0) * texture(uTextureSampler, out_texcoord);  // Texture
}
