#version 330 core

layout(location=0) in vec3 in_pos;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_texcoord;
layout(location=3) in mat4 instance_model_mat;
 
// out vec3 normal_world;
out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_uv;
//out vec4 ShadowCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 depth_bias_mvp;
uniform mat3 normal_model_mat;


void main(){
	gl_Position = proj * view * instance_model_mat * vec4(in_pos, 1.0);
    frag_pos = (instance_model_mat * vec4(in_pos, 1.0)).xyz;
	
	// 法线没有位移，不需要w向量，且还需要一些特殊处理来处理不等比缩放时带来的问题
    frag_normal = normalize(normal_model_mat * in_normal);
	frag_uv = in_texcoord;
		
	//ShadowCoord = depth_bias_mvp * vec4(vertexPosition_modelspace, 1);
}