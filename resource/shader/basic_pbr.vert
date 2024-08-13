#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl" 

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

// out vec3 normal_world;
out vec3 out_pos;
out vec3 out_normal;
out vec2 out_texcoord;
//out vec4 ShadowCoord;

layout(std140, binding=0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cam_pos;
} matrix_ubo;


void main(){
	gl_Position = matrix_ubo.projection * matrix_ubo.view * matrix_ubo.model * vec4(in_pos, 1.0);
    out_pos = (matrix_ubo.model * vec4(in_pos, 1.0)).xyz;
	
	// 法线没有位移，不需要w向量，且还需要一些特殊处理来处理不等比缩放时带来的问题
    out_normal = normalize(mat3(transpose(inverse(model)))) * in_normal;
	out_texcoord = in_texcoord;
    
	//ShadowCoord = depth_bias_mvp * vec4(vertexPosition_modelspace, 1);
}