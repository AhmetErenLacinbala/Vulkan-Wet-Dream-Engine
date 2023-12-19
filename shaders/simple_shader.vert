#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo{
    mat4 projectionViewMatrix;
    vec4 ambientLightColor;
    vec3 LightPosition;
    vec4 lightColor;
} ubo;

layout (push_constant) uniform Push {
    mat4 modelMatrix; 
    mat4 normalMatrix;
} push;


void main(){
    //we should convert modelMatrix to position matrix since the point light in the world space
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projectionViewMatrix * positionWorld;
    
    // Calculating the inverse in a shader can be expensive and should be avoided
    //mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
    //vec3 normalWorldSpace = normalize(normalMatrix * normal);

    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);
    
    vec3 directionToLight = ubo.LightPosition - positionWorld.xyz;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); //distance squared

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 diffuceLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)),0);

    fragColor = (diffuceLight + ambientLight) * color;

}