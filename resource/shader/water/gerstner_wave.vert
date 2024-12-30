#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_tex;

out vec2 out_tex;
uniform vec3 cam_pos;

// 200单位地形平移一次
float cam_offset_threshold = 200.0;
void main(){
    vec3 cam_offset = floor(vec3(cam_pos.x, 0, cam_pos.z)/cam_offset_threshold);
    cam_offset*=cam_offset_threshold;
    gl_Position = vec4(in_pos+cam_offset, 1.0);
    out_tex = in_tex;
}