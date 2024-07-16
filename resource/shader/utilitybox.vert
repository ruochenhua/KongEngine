#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

out vec2 uv;
// out vec3 normal_world;
out vec3 in_pos;
//out vec4 ShadowCoord;

uniform mat4 MVP;
uniform mat4 depth_bias_mvp;
uniform mat3 normal_model_mat;

void main(){
	gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0); 	
    in_pos = gl_Position.xyz;

	// uv = vertexTextureCoord;	
	// 法线没有位移，不需要w向量，且还需要一些特殊处理来处理不等比缩放时带来的问题
	// normal_world = normal_model_mat * vertexNormal_modelspace;
		
	//ShadowCoord = depth_bias_mvp * vec4(vertexPosition_modelspace, 1);
}