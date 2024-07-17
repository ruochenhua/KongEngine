#version 330 core

in vec3 out_pos;
in vec3 out_normal;

out vec3 color;

uniform float shininess;
uniform vec3 light_color;
uniform vec3 light_dir;

uniform vec3 cam_pos;

void main()
{
    float ka = 0.1;
    float kd = 1.0;
    float ks = 0.5;
    vec3 box_color = vec3(1.0f, 0.5f, 0.31f);

	vec3 ambient = ka * light_color;

	//diffuse 分量计算
	float ln = max(0.0, dot(light_dir, out_normal));

	vec3 diffuse = kd * light_color * ln;

	//specular 分量计算
	vec3 v = normalize(cam_pos - out_pos);
	vec3 h = normalize(light_dir + v);
	
	vec3 reflect_dir = reflect(-light_dir, out_normal);
	float spec = pow(max(dot(v, reflect_dir), 0.0), 32);
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

	
	color = (diffuse + specular) * box_color;
}