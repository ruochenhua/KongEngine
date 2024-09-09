#version 450 compatibility
in float out_height;
out vec4 FragColor;


void main()
{
    float h = (out_height + 16) / 32.f;
    FragColor = vec4(h, h, h, 1.0);
}