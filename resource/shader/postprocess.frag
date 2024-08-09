#version 450 compatibility
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene_texture;

void main()
{
    vec3 scene_value = texture(scene_texture, TexCoords).rgb;
    FragColor = vec4(scene_value, 1.0); // orthographic
}