#version 450 compatibility
in vec3 out_pos;
out vec4 FragColor;

uniform vec4 albedo;    // color

void main()
{
    FragColor = albedo;
}