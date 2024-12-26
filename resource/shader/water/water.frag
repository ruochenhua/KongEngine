#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

in vec3 out_pos;
in vec3 out_normal;
in vec2 out_texcoord;

layout (location=0) out vec4 FragColor;

uniform sampler2D scene_texture;

void main()
{
	float texture_size = textureSize(scene_texture, 0).x;
	if(texture_size > 1.0)
	{
		FragColor = vec4(texture(scene_texture, out_texcoord).xyz, 1.0);
	}
	else
	{
		FragColor = vec4(out_texcoord, 0.0, 1.0);
	}

}