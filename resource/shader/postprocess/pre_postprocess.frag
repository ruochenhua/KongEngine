#version 450 compatibility
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene_texture;
uniform sampler2D added_texture;
uniform int combine_mode;

void main()
{
    FragColor = texture(scene_texture, TexCoords);

    vec4 added_color = texture(added_texture, TexCoords);

    if(combine_mode == 0)
    {
        FragColor += added_color;
    }
    else if(combine_mode == 1)
    {
        FragColor.rgb += added_color.rgb * added_color.a;
    }
}