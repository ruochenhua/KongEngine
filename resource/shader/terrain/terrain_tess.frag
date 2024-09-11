#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

in vec2 frag_texcoord;
in vec3 frag_pos;
in vec3 frag_normal;

in float tes_height;
out vec4 FragColor;

in mat3 TBN;

uniform sampler2D grass_texture;
uniform sampler2D rock_texture;
uniform sampler2D sand_texture;
uniform sampler2D rock_normal_texture;

int trans = 30;
void main()
{
    float grass_scale = 12.f;
    float rock_scale = 10.f;
    float sand_scale = 10.f;

    vec4 grass_color = texture(grass_texture, frag_texcoord*grass_scale);
    vec4 rock_color = texture(rock_texture, frag_texcoord*rock_scale);
    vec4 sand_color = texture(sand_texture, frag_texcoord*sand_scale);


    vec4 color;
    vec3 normal = frag_normal;
    if(tes_height <= trans)
    {
        color = sand_color;
    }
    else if(tes_height <= 3*trans)
    {
        color = mix(sand_color, grass_color, min(1, (tes_height-trans)/trans));
    }
    else
    {
        color = mix(grass_color, rock_color, min(1, (tes_height-3*trans)/trans));
        vec3 rock_normal = texture(rock_normal_texture, frag_texcoord*rock_scale).xyz;
        rock_normal = normalize(TBN * (rock_normal*2.0-1.0));
        normal = rock_normal;
    }

    if(light_info_ubo.has_dir_light.x > 0)
    {
        vec3 light_dir = -light_info_ubo.directional_light.light_dir.xyz;
        vec4 light_color = light_info_ubo.directional_light.light_color;

        vec3 view = matrix_ubo.cam_pos.xyz - frag_pos;
        float diffuse_factor = max(0, dot(light_dir, normal));
        color = diffuse_factor * light_color * color;
    }

    FragColor = vec4(color.xyz, 1.0);
    // FragColor = vec4(normal, 1.0);
}