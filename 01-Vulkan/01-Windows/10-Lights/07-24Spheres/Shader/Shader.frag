#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec3 outTransformedNormal;
layout(location = 1) in vec3 outLightDirection;
layout(location = 2) in vec3 outViewerVector;

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

layout(push_constant) uniform PushConstantData_Material
{
  // Material related uniform
  vec4 materialDiffuse;
  vec4 materialAmbient;
  vec4 materialSpecular;
  float materialShininess;
} pushConstantData_Material;

void main(void)
{
    // Code
    vec3 fongADS_light;
    if(1 == uMyuniformData.lKeyIsPressed)
    {
        vec3 normalizedTransformedNormal = normalize(outTransformedNormal);
        vec3 normalizedLightDirection = normalize(outLightDirection);
        vec3 normalizedViewerVector = normalize(outViewerVector);

        vec3 ambientLight = vec3(uMyuniformData.lightAmbient) * vec3(pushConstantData_Material.materialAmbient);
        vec3 diffuseLight = vec3(uMyuniformData.lightDiffuse) * vec3(pushConstantData_Material.materialDiffuse) * max(dot(normalizedLightDirection, normalizedTransformedNormal), 0.0);
        vec3 reflectionVector = reflect(-normalizedLightDirection, normalizedTransformedNormal);
        vec3 specularLight = vec3(uMyuniformData.lightSpecular) * vec3(pushConstantData_Material.materialSpecular) * pow(max(dot(reflectionVector, normalizedViewerVector), 0.0), pushConstantData_Material.materialShininess);
        fongADS_light = ambientLight + diffuseLight + specularLight;
    }
    else
    {
        fongADS_light = vec3(1.0, 1.0, 1.0);
    }

    FragColor = vec4(fongADS_light, 1.0);  // Lighting
}
