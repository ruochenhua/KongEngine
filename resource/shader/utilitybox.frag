#version 330 core

in vec3 out_pos;
in vec3 out_normal;

out vec3 color;

uniform float shininess;
uniform vec3 light_color;
uniform vec3 light_dir;

uniform vec3 cam_pos;

void main()
{
    color = out_pos;
}