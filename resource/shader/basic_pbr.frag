#version 330 core
struct DirectionalLight
{
	vec3 light_dir;
	vec3 light_color;
};

struct PointLight
{
	vec3 light_pos;
	vec3 light_color;
};

#define POINT_LIGHT_MAX 4

in vec3 out_pos;
in vec3 out_normal;
in vec2 out_texcoord;

out vec3 color;

uniform float shininess;
uniform DirectionalLight directional_light;
uniform PointLight point_lights[POINT_LIGHT_MAX];
uniform int point_light_count;

uniform vec3 cam_pos;
uniform vec3 obj_color;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_map_texture;

float ka = 0.2;
float kd = 0.5;
float ks = 1;

// calculate color causes by directional light
vec3 CalcDirLight(DirectionalLight dir_light, vec3 normal, vec3 view)
{
	vec3 light_color = dir_light.light_color;
	vec3 light_dir = dir_light.light_dir;

	// ambient light
	vec3 ambient = ka * light_color;

	// diffuse light
	float ln = max(0, dot(light_dir, normal));

	vec3 diffuse = kd* light_color * ln;
	vec3 diffuse_color = vec3(0);
	float texture_size = textureSize(diffuse_texture, 0).x;
	if(texture_size > 1.0)
	{
		diffuse_color = diffuse * texture(diffuse_texture, out_texcoord).rgb;
	}
	else
	{
		diffuse_color = diffuse * obj_color;
	}
	 
	
	// specular light
	vec3 h = normalize(light_dir + view);
	float spec = pow(max(dot(h, normal), 0), 256);
	vec3 specular = ks * spec * light_color;

	float spec_tex_size = textureSize(specular_map_texture, 0).x;
	vec3 spec_basic_color = vec3(0);
	if(spec_tex_size > 1.0)
	{
		spec_basic_color = texture(specular_map_texture, out_texcoord).rgb;
	}
	else
	{
		spec_basic_color = obj_color;
	}

	vec3 specular_color = specular * spec_basic_color;

	return ambient + diffuse_color + specular_color;
}

vec3 CalcPointLight(PointLight point_light, vec3 normal, vec3 view, vec3 frag_pos)
{
	vec3 light_color = point_light.light_color;
	vec3 light_dir = normalize(point_light.light_pos - frag_pos);

	// ambient light
	vec3 ambient = ka * light_color;

	// diffuse light
	float ln = max(0, dot(light_dir, normal));

	vec3 diffuse = kd* light_color * ln;
	vec3 diffuse_color = vec3(0);
	float texture_size = textureSize(diffuse_texture, 0).x;
	if(texture_size > 1.0)
	{
		diffuse_color = diffuse * texture(diffuse_texture, out_texcoord).rgb;
	}
	else
	{
		diffuse_color = diffuse * obj_color;
	}
	 	
	// specular light
	vec3 h = normalize(light_dir + view);
	float spec = pow(max(dot(h, normal), 0), 32);
	vec3 specular = ks * spec * light_color;
	float spec_tex_size = textureSize(specular_map_texture, 0).x;
	vec3 spec_basic_color = vec3(0);
	if(spec_tex_size > 1.0)
	{
		spec_basic_color = texture(specular_map_texture, out_texcoord).rgb;
	}
	else
	{
		spec_basic_color = obj_color;
	}
	vec3 specular_color = specular * spec_basic_color;

	float distance = length(point_light.light_pos - frag_pos);
	float attenuation = 1.0 / (1 + distance);	//衰减和点光源的参数可控，这里先简单弄个
	return (ambient + diffuse_color + specular_color) * attenuation;
}


void main()
{
	vec3 view = normalize(cam_pos - out_pos);
	
	vec3 dir_light_color = CalcDirLight(directional_light, out_normal, view);
	vec3 point_light_color = vec3(0,0,0);
	for(int i = 0; i < point_light_count; ++i)
	{
		point_light_color += CalcPointLight(point_lights[i], out_normal, view, out_pos);
	}

	color = dir_light_color + point_light_color;

}