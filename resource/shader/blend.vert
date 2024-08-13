#version 450 compatibility

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

// out vec3 normal_world;
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
    out_texcoord = in_texcoord;
}