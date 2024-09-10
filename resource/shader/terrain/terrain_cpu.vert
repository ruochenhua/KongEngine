#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

layout(location=0) in vec3 in_pos;
out float out_height;
out vec3 out_position;

void main(){
    out_height = in_pos.y;    
    out_position = (matrix_ubo.view * vec4(in_pos, 1.0)).xyz;
    gl_Position = matrix_ubo.projection * matrix_ubo.view * vec4(in_pos, 1.0);
}