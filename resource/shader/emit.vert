#version 330 core

layout(location = 0) in vec3 in_pos;

// out vec3 normal_world;
out vec3 out_pos;
//out vec4 ShadowCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;


void main(){
    gl_Position = proj * view * model * vec4(in_pos, 1.0);
}