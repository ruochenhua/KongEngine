#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

uniform mat4 model;

// out vec3 normal_world;
//out vec3 out_pos;
//out vec3 out_normal;
out vec2 out_texcoord;
out vec4 clip_space;

const float tiling = 6.0;

void main(){
	gl_Position = matrix_ubo.projection * matrix_ubo.view * model * vec4(in_pos, 1.0);
    clip_space = gl_Position;
//    out_pos = (model * vec4(in_pos, 1.0)).xyz;
//
//    out_normal = normalize(mat3(transpose(inverse(model))) * in_normal);
	out_texcoord = in_texcoord * tiling;
}