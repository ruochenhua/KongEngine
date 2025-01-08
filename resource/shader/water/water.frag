#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

in vec3 out_pos;
in vec3 out_normal;
in vec2 out_texcoord;

in vec4 clip_space;

layout (location=0) out vec4 FragColor;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
uniform sampler2D dudv_map;
uniform sampler2D normal_map;

uniform float move_factor;

const float wave_strength = 0.05;

void main()
{
	vec2 ndc = clip_space.xy / clip_space.w;
	ndc = ndc / 2.0 + vec2(0.5);
	vec2 reflection_coord = vec2(ndc.x, -ndc.y);
	vec2 refraction_coord = ndc;

//	FragColor = vec4(texture(refraction_texture, refraction_coord).xyz/2.0, 1.0);
//	FragColor = vec4(texture(reflection_texture, reflection_coord).xyz/2.0, 1.0);
//	return;
//	vec2 distortion1 = (texture(dudv_map, out_texcoord + move_factor).rg * 2.0 - 1.0) * wave_strength;
//	vec2 distortion2 = (texture(dudv_map, out_texcoord*-1 - move_factor*2.0).rg * 2.0 - 1.0) * wave_strength;
//	vec2 total_distort = distortion1 + distortion2;
	
	vec2 distorted_texcoords = texture(dudv_map, vec2(out_texcoord.x + move_factor, out_texcoord.y)).rg * 0.1;
	distorted_texcoords = out_texcoord + vec2(distorted_texcoords.x, distorted_texcoords.y + move_factor);
	vec2 total_distort = (texture(dudv_map, distorted_texcoords).rg * 2.0 - 1.0) * wave_strength;
	
	reflection_coord += total_distort;
	reflection_coord.x = clamp(reflection_coord.x, 0.001, 0.999);
	reflection_coord.y = clamp(reflection_coord.y, -0.999, -0.001);

	refraction_coord += total_distort;
	refraction_coord = clamp(refraction_coord, 0.001, 0.999);

	// fixme 为什么这里在terrain是延迟渲染的时候需要将颜色除以二？
	float color_factor = 0.5;

	vec4 reflection_color = vec4(texture(reflection_texture, reflection_coord).xyz*color_factor, 1.0);
	vec4 refraction_color = vec4(texture(refraction_texture, refraction_coord).xyz*color_factor, 1.0);
	
	vec3 view_vector = normalize(matrix_ubo.cam_pos.xyz-out_pos);
	float fresnel_blend = clamp(dot(out_normal, view_vector), 0.0, 1.0);
	fresnel_blend = pow(fresnel_blend, 2.2);
	
	vec3 specular_highlights = vec3(0);
	if(light_info_ubo.has_dir_light.r != 0)
	{
		vec4 normal_map_color = texture(normal_map, distorted_texcoords);
		vec3 normal = normalize(vec3(normal_map_color.r * 2.0 - 1.0, normal_map_color.b, normal_map_color.g * 2.0 - 1.0));

//		fresnel_blend = clamp(dot(normal, view_vector), 0.0, 1.0);

		vec3 light_dir = light_info_ubo.directional_light.light_dir.xyz;
		vec3 light_color = light_info_ubo.directional_light.light_color.xyz;

		vec3 reflected_light = reflect(light_dir, normal);
		float specular = max(dot(reflected_light, view_vector), 0.0);
		float shine_damper = 100.0;
		specular = pow(specular, shine_damper);
		float reflectivity = 0.5;
		specular_highlights = light_color * specular * reflectivity;
	}
	
	vec4 blue_color = vec4(0.1, 0.2, 0.6, 1.0);	// add some blue color
	FragColor = mix(
		mix(reflection_color, refraction_color, fresnel_blend)		//*0.5	// 稍微暗一点dim a little bit
		, blue_color
		, 0.1) + vec4(specular_highlights, 0.0);
}