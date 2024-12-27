#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

//in vec3 out_pos;
//in vec3 out_normal;
in vec2 out_texcoord;

in vec4 clip_space;

layout (location=0) out vec4 FragColor;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
uniform sampler2D dudv_map;
uniform float move_factor;

const float wave_strength = 0.02;

void main()
{
	vec2 ndc = clip_space.xy / clip_space.w;
	ndc = ndc / 2.0 + vec2(0.5);
	vec2 reflection_coord = vec2(ndc.x, -ndc.y);
	vec2 refraction_coord = ndc;

	vec2 distortion1 = (texture(dudv_map, out_texcoord + move_factor).rg * 2.0 - 1.0) * wave_strength;
	vec2 distortion2 = (texture(dudv_map, out_texcoord*-1 - move_factor*2.0).rg * 2.0 - 1.0) * wave_strength;

	vec2 total_distort = distortion1 + distortion2;
	reflection_coord += total_distort;
	reflection_coord.x = clamp(reflection_coord.x, 0.001, 0.999);
	reflection_coord.y = clamp(reflection_coord.y, -0.999, -0.001);

	refraction_coord += total_distort;
	refraction_coord = clamp(refraction_coord, 0.001, 0.999);

	vec4 reflection_color = vec4(texture(reflection_texture, reflection_coord).xyz, 1.0);
	vec4 refraction_color = vec4(texture(refraction_texture, refraction_coord).xyz, 1.0);


	vec4 blue_color = vec4(0.0, 0.2, 0.6, 1.0);	// add some blue color
	FragColor = mix(mix(reflection_color, refraction_color, 0.5), blue_color, 0.1);
}