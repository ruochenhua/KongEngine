#version 450 compatibility
in float tes_height;
out vec4 FragColor;

void main()
{
    float h = (tes_height + 16) / 64.f;
    FragColor = vec4(h, h, h, 1.0);
}