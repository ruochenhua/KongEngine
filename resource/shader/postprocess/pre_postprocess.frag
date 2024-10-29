#version 450 compatibility
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene_texture;
uniform sampler2D reflection_texture;

void main()
{
    FragColor = texture(scene_texture, TexCoords);
    vec4 reflection_color = texture(reflection_texture, TexCoords);

    FragColor.rgb += reflection_color.rgb * reflection_color.a;
}