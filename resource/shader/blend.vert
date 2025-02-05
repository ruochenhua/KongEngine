#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl" 

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

uniform mat4 model;
// out vec3 normal_world;
out vec2 out_texcoord;
//out vec4 ShadowCoord;

void main(){
    gl_Position = matrix_ubo.projection * matrix_ubo.view * model * vec4(in_pos, 1.0);
    out_texcoord = in_texcoord;
}