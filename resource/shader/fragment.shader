#version 330 core

in vec2 uv;
in vec3 normal_world;
in vec3 in_pos;
in vec4 ShadowCoord;

out vec3 color;

uniform sampler2D texture_sampler;
uniform sampler2DShadow shadowMap;

uniform float shininess;
uniform vec3 light_color;
uniform vec3 light_dir;

uniform vec3 cam_pos;

void main(){
    float ka = 0.1;
    float kd = 1.0;
    float ks = 1.0;

	vec3 ambient = ka * light_color;

	//diffuse 分量计算
	float ln = max(0.0, dot(light_dir, normal_world));

	vec3 diffuse = kd * light_color * ln;

	//specular 分量计算
	vec3 v = normalize(cam_pos - in_pos);
	vec3 h = normalize(light_dir + v);
	
	float spec = pow(max(0.0, dot(normal_world, h)), 32);
	vec3 specular = ks * spec * light_color;

	// vec3 specular = vec3(0, 0, 0);
	// bool back = (dot(v, normal_world) > 0) && (dot(light_dir, normal_world) > 0);
	// if(back)
	// {
	// 	vec3 t = normalize(cross(normal_world, v));
	// 	float a = dot(light_dir, t);
	// 	float b = dot(v, t);
	// 	float c = sqrt(1-pow(a, 2.0)) * sqrt(1-pow(b, 2.0)) - a*b;
	// 	float brdf = ks*pow(c, shininess);

	// 	specular = brdf * light_color * ln;
	// }

	
	color = (diffuse + specular) * texture(texture_sampler, uv).rgb;

	//shadow
	// float visibility = texture( shadowMap, vec3(ShadowCoord.xy, (ShadowCoord.z)/ShadowCoord.w) );
	// color += ambient + diffuse;// + specular;
	// color *= visibility;
}