#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;

// out vec3 normal_world;
out vec3 out_pos;
out vec3 out_normal;
//out vec4 ShadowCoord;

uniform mat4 MVP;
uniform mat4 depth_bias_mvp;
uniform mat3 normal_model_mat;

void main(){
	gl_Position = MVP * vec4(in_pos, 1.0); 	
    // out_pos = gl_Position.xyz;
    out_pos = in_pos;
    out_normal = in_normal;
	// uv = vertexTextureCoord;	
	// 法线没有位移，不需要w向量，且还需要一些特殊处理来处理不等比缩放时带来的问题
	// normal_world = normal_model_mat * vertexNormal_modelspace;
		
	//ShadowCoord = depth_bias_mvp * vec4(vertexPosition_modelspace, 1);
}