#version 450 compatibility
in vec3 out_pos;
out vec4 FragColor;

uniform vec3 albedo;    // color


void main()
{
    FragColor = vec4(albedo, 1.0);
}