#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_scalar_block_layout : enable

#include "../common.glsl"

layout(location=0) in vec3 fragPos;
layout(location=1) in vec3 fragNormal;
layout(location=2) in vec2 fragUV;
layout(location=3) in mat3 TBN;

layout(location=0) out vec4 gPosition;
layout(location=1) out vec4 gNormal;
layout(location=2) out vec4 gAlbedo;
layout(location=3) out vec4 gOrm;

layout(push_constant) uniform Push{
    mat4 modelMatrix;
} push;

layout(set=0, binding=0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 cameraPosition;
    SceneLightInfo sceneLightInfo;
} ubo;

layout(set=1, binding=0) uniform BaiscMaterial{
    vec4 albedo;
    float specular_factor;
    float metallic;
    float roughness;
    float ambient;
}material;

layout(set=2, binding=0) uniform sampler2D diffuse_texture;
layout(set=2, binding=1) uniform sampler2D normal_texture;
layout(set=2, binding=2) uniform sampler2D roughness_texture;
layout(set=2, binding=3) uniform sampler2D metallic_texture;
layout(set=2, binding=4) uniform sampler2D ambient_texture;

const float PI = 3.14159265359;

vec4 GetAlbedo()
{
    float texture_size = textureSize(diffuse_texture, 0).x;
    if(texture_size > 2.0)
    {
        vec4 texture_albedo = texture(diffuse_texture, fragUV);
        // sRGB空间转换到线性空间
//        return texture_albedo;
        return pow(texture_albedo, vec4(2.2));
    }

    return material.albedo;
}

vec3 GetNormal()
{
    float texture_size = textureSize(normal_texture, 0).x;
    if(texture_size > 1.0)
    {
        vec3 texture_normal = texture(normal_texture, fragUV).xyz;
        // 从[0,1]映射到[-1,1]
        vec3 tex_normal = normalize(texture_normal*2.0 - 1.0);
        return normalize(TBN * tex_normal);
    }

    return fragNormal;
}


float GetRoughness()
{
    float texture_size = textureSize(roughness_texture, 0).x;
    if(texture_size > 2.0)
    {
        return texture(roughness_texture, fragUV).r;
    }

    return material.roughness;
}

float GetMetallic()
{
    float texture_size = textureSize(metallic_texture, 0).x;
    if(texture_size > 2.0)
    {
        return texture(metallic_texture, fragUV).r;
    }

    return material.metallic;
}

float GetAO()
{
    float texture_size = textureSize(ambient_texture, 0).x;
    if(texture_size > 2.0)
    {
        return texture(ambient_texture, fragUV).r;
    }

    return material.ambient;
}

void main()
{
    gPosition = vec4(fragPos,1.0);
    vec3 camPos = ubo.cameraPosition.xyz;
    float depth = distance(camPos, fragPos.xyz);
    gNormal = vec4(GetNormal(), depth);
    gAlbedo = GetAlbedo();
    gOrm = vec4(GetAO(), GetRoughness(), GetMetallic(), 1.0);
}