#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
in vec2 TexCoords;
out float FragColor;

uniform sampler2D position_texture;
uniform sampler2D normal_texture;
uniform sampler2D noise_texture;

uniform mat4 ssao_proj;
uniform vec3 samples[64];

const vec2 noise_scale = vec2(1024.0 / 4.0, 768.0 / 4.0);   // todo 屏幕分辨率传入

void main()
{
    vec3 frag_pos = texture(position_texture, TexCoords).xyz;
    vec3 normal = texture(normal_texture, TexCoords).xyz;
    vec3 random_vec = texture(noise_texture, TexCoords*noise_scale).xyz;

    // TBN
    vec3 tangent = normalize(random_vec - normal*dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    float radius = 1.0;
    float occlusion = 0.0;
    float kernel_size = 64.0;
    for(int i = 0; i < kernel_size; ++i)
    {
        vec3 tmp_sample = TBN * samples[i];
        tmp_sample = frag_pos + tmp_sample * radius;

        vec4 offset = vec4(tmp_sample, 1.0);
        offset = ssao_proj * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sample_depth = -texture(position_texture, offset.xy).w;
        float range_check = smoothstep(0.0, 1.0, radius/abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth > tmp_sample.z ? 1.0 : 0.0);
    }

    occlusion = 1.0 - (occlusion / kernel_size);
    FragColor = occlusion;
}