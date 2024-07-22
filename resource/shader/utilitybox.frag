#version 330 core

in vec3 out_pos;
in vec3 out_normal;
in vec2 out_texcoord;

out vec3 color;

uniform float shininess;
uniform vec3 light_color;
uniform vec3 light_dir;

uniform vec3 cam_pos;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_map_texture;

void main()
{
    float ka = 0.1;
    float kd = 1.0;
    float ks = 3.0;
    vec3 box_color = vec3(1.0, 0.5, 0.31);

	vec3 ambient = ka * light_color;

	//diffuse 分量计算
	float ln = max(0.0, dot(light_dir, out_normal));

	vec3 diffuse = kd * light_color * ln;

	//specular 分量计算
	vec3 v = normalize(cam_pos - out_pos);
	vec3 h = normalize(light_dir + v);	
	
	float spec = pow(max(dot(h, out_normal), 0.0), 256);
    // phong
	// vec3 reflect_dir = reflect(-light_dir, out_normal);
    // float spec = pow(max(dot(v, reflect_dir), 0.0), 32);
    
	vec3 specular = ks * spec * light_color;

	// vec3 specular = vec3(0, 0, 0);
	// bool back = (dot(v, out_normal) > 0) && (dot(light_dir, out_normal) > 0);
	// if(back)
	// {
	// 	vec3 t = normalize(cross(out_normal, v));
	// 	float a = dot(light_dir, t);
	// 	float b = dot(v, t);
	// 	float c = sqrt(1-pow(a, 2.0)) * sqrt(1-pow(b, 2.0)) - a*b;
	// 	float brdf = ks*pow(c, shininess);

	// 	specular = brdf * light_color * ln;
	// }

	
	//color = vec3(out_texcoord, 1.0);// (diffuse + specular) * box_color;
	diffuse_color = diffuse * texture(diffuse_texture, out_texcoord).rgb;
	specular_color = specular * texture(specular_map_texture, out_texcoord).rgb;
	color = diffuse_color +specular_color;
}