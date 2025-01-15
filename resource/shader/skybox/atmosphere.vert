#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out mat4 inv_view;
out mat4 inv_proj;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);

    inv_view = inverse(matrix_ubo.view);
    inv_proj = inverse(matrix_ubo.projection);
}