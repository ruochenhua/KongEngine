#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_tex;

out vec2 out_tex;
uniform vec3 cam_pos;

void main(){
    gl_Position = vec4(in_pos, 1.0);
    out_tex = in_tex;
}