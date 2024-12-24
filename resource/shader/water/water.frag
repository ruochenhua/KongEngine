#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

in vec3 out_pos;
in vec3 out_normal;
in vec2 out_texcoord;

layout (location=0) out vec4 FragColor;

void main()
{
	FragColor = vec4(0.2, 0.7, 0.1, 1.0);
}