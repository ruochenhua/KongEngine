#version 450

layout(location=0) in vec3 fragColor;
layout(location=1) in vec2 fragUV;

layout(location=0) out vec4 outColor;

layout(push_constant) uniform Push{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

layout(set=0, binding=1) uniform sampler2D diffuse_texture;


void main()
{
    vec4 texColor = texture(diffuse_texture, fragUV);
    outColor = vec4(fragColor,1);
    outColor = texColor;
}