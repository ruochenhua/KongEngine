#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_scalar_block_layout : enable

#include "common.glsl"

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

layout(location=0) out vec3 fragPos;
layout(location=1) out vec3 fragNormal;
layout(location=2) out vec2 fragUV;
layout(location=3) out mat3 TBN;

layout(push_constant) uniform Push{
    mat4 modelMatrix;
    int use_texture;
} push;

// descriptor set
layout(set=0, binding=0) uniform GlobalUbo {
    mat4 projectionView;
    vec4 directionToLight;
    vec4 cameraPositon;

    SceneLightInfo sceneLightInfo;
} ubo;

void main()
{
    mat4 model = push.modelMatrix;

    gl_Position = ubo.projectionView * model * vec4(position, 1.0);
    fragPos = (model * vec4(position, 1.0)).xyz;
    fragNormal = normalize(mat3(transpose(inverse(model))) * normal);
    fragUV = uv;

    vec3 T = normalize(vec3(model*vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(model*vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model*vec4(fragNormal, 0.0)));

    TBN = mat3(T, B, N);
}