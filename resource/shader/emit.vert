#version 430 core

layout(location = 0) in vec3 in_pos;

// out vec3 normal_world;
out vec3 out_pos;
//out vec4 ShadowCoord;

layout(std140, binding=0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cam_pos;
} matrix_ubo;


void main(){
    gl_Position = matrix_ubo.projection * matrix_ubo.view * matrix_ubo.model * vec4(in_pos, 1.0);
}