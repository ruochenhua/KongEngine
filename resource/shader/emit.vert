#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl" 
layout(location = 0) in vec3 in_pos;

// out vec3 normal_world;
out vec3 out_pos;
//out vec4 ShadowCoord;

void main(){
    gl_Position = matrix_ubo.projection * matrix_ubo.view * matrix_ubo.model * vec4(in_pos, 1.0);
}