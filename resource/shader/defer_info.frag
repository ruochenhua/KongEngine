#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
#include "/common/brdf_common.glsl"

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out vec4 gORM;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in mat3 TBN;

layout(std140, binding=0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cam_pos;
} matrix_ubo;

uniform vec4 albedo;    // color
uniform float specular_factor;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform sampler2D diffuse_texture;
uniform sampler2D normal_texture;
uniform sampler2D roughness_texture;
uniform sampler2D metallic_texture;
uniform sampler2D ao_texture;
uniform samplerCube skybox_texture;
uniform samplerCube skybox_diffuse_irradiance_texture;
uniform samplerCube skybox_prefilter_texture;
uniform sampler2D skybox_brdf_lut_texture;

vec4 GetAlbedo()
{
    float texture_size = textureSize(diffuse_texture, 0).x;
    if(texture_size > 1.0)
    {
        vec4 texture_albedo = texture(diffuse_texture, frag_uv);
        // sRGB空间转换到线性空间
        return pow(texture_albedo, vec4(2.2));
    }

    return albedo;
}

vec3 GetNormal()
{
    float texture_size = textureSize(normal_texture, 0).x;
    if(texture_size > 1.0)
    {
        vec3 texture_normal = texture(normal_texture, frag_uv).rgb;
        // 从[0,1]映射到[-1,1]
        vec3 tex_normal = normalize(texture_normal*2.0 - 1.0);
        return normalize(TBN * tex_normal);
    }

    return frag_normal;
}

float GetRoughness()
{
    float texture_size = textureSize(roughness_texture, 0).x;
    if(texture_size > 1.0)
    {
        return texture(roughness_texture, frag_uv).r;
    }

    return roughness;
}

float GetMetallic()
{
    float texture_size = textureSize(metallic_texture, 0).x;
    if(texture_size > 1.0)
    {
        return texture(metallic_texture, frag_uv).r;
    }

    return metallic;
}

float GetAO()
{
    float texture_size = textureSize(ao_texture, 0).x;
    if(texture_size > 1.0)
    {
        return texture(ao_texture, frag_uv).r;
    }

    return ao;
}

void main()
{
    gPosition = vec4(frag_pos,1.0);
    gNormal = vec4(GetNormal(),1.0);
    gAlbedo = GetAlbedo();
    gORM = vec4(GetAO(), GetRoughness(), GetMetallic(), 1.0);
}