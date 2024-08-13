#version 450 compatibility
in vec2 out_texcoord;
out vec4 FragColor;

uniform vec4 albedo;    // color
uniform sampler2D diffuse_tex;

void main()
{
    float tex_size = textureSize(diffuse_tex, 0).x;
    if(tex_size > 2.0)
    {
        FragColor = texture(diffuse_tex, out_texcoord);
    }
    else
    {
        FragColor = albedo;
    }
}