#version 450 

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

layout(location=0) out vec3 fragColor;

layout(push_constant) uniform Push{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

// descriptor set
layout(set=0, binding=0) uniform GlobalUbo {
    mat4 projectionView;
    vec3 directionToLight;
} ubo;

const float AMBIENT = 0.02;

void main()
{
    gl_Position = ubo.projectionView * push.modelMatrix * vec4(position, 1.0);
    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);
    float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);

    fragColor = lightIntensity*vec3(0.2, 0.3, 0.4);
}