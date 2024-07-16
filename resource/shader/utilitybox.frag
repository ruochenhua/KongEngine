#version 330 core

in vec2 uv;
in vec3 normal_world;
in vec3 in_pos;
in vec4 ShadowCoord;

out vec3 color;

uniform float shininess;
uniform vec3 light_color;
uniform vec3 light_dir;

uniform vec3 cam_pos;

void main()
{
    color = in_pos;
}