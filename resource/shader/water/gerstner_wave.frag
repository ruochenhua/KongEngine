#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

in vec2 frag_texcoord;
in vec3 frag_pos;
in vec3 frag_normal;

in float tes_height;
in vec4 clip_space;

out vec4 FragColor;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
uniform sampler2D dudv_map;
uniform sampler2D normal_map;
uniform double iTime;

uniform vec3 cam_pos;
const float foam_threshold = 0.5;

void main()
{
    vec2 ndc = clip_space.xy / clip_space.w;
	ndc = ndc / 2.0 + vec2(0.5);
	vec2 reflection_coord = vec2(ndc.x, -ndc.y);
	vec2 refraction_coord = ndc;

	vec3 view_vector = normalize(matrix_ubo.cam_pos.xyz-frag_pos);
	float fresnel_blend = clamp(dot(frag_normal, view_vector), 0.0, 1.0);

	vec3 specular_highlights = vec3(0);

	if(light_info_ubo.has_dir_light.r != 0)
	{
		vec3 light_dir = light_info_ubo.directional_light.light_dir.xyz;
		vec3 light_color = light_info_ubo.directional_light.light_color.xyz;

		vec3 reflected_light = reflect(light_dir, frag_normal);
		float specular = max(dot(reflected_light, view_vector), 0.0);
		float shine_damper = 100.0;
		specular = pow(specular, shine_damper);
		float reflectivity = 0.5;
		specular_highlights = light_color * specular * reflectivity;

		float diffuse_factor = max(0, dot(light_dir, frag_normal));
//		diffuse_factor *= diffuse_factor;
	}

	float move_factor = -float(iTime) * 0.05;
	float wave_strength = 0.05;
	vec2 distorted_texcoords = texture(dudv_map, vec2(frag_texcoord.x + move_factor, frag_texcoord.y)).rg * 0.1;
	distorted_texcoords = frag_texcoord + vec2(distorted_texcoords.x, distorted_texcoords.y + move_factor);
	vec2 dudv_distort = (texture(dudv_map, distorted_texcoords).rg * 2.0 - 1.0) * wave_strength;

	vec2 total_distort = frag_normal.xz * 0.1 + dudv_distort;

	reflection_coord += total_distort;
	reflection_coord.x = clamp(reflection_coord.x, 0.001, 0.999);
	reflection_coord.y = clamp(reflection_coord.y, -0.999, -0.001);

	refraction_coord += total_distort;
	refraction_coord = clamp(refraction_coord, 0.001, 0.999);

	vec4 reflection_color = vec4(texture(reflection_texture, reflection_coord).xyz, 1.0);
	vec4 refraction_color = vec4(texture(refraction_texture, refraction_coord).xyz, 1.0);

	vec4 water_tint_color = vec4(0.2, 0.6, 0.9, 1.0);
	float water_tint_factor = 0.2;
	reflection_color = mix(reflection_color, water_tint_color, water_tint_factor);

	vec4 diffuse_color = mix(reflection_color, refraction_color, fresnel_blend);

    vec4 final_color = diffuse_color + vec4(specular_highlights, 0.0);

//	if(frag_pos.y > foam_threshold)
//	{
//		final_color = mix(vec4(1.0), final_color,
//						  pow(max(0, dot(frag_normal, vec3(0,1,0))), 10.0) );
//	}

	FragColor = final_color;
}