#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_scalar_block_layout : enable

#include "common.glsl"

layout(location=0) in vec3 cubeUV;

layout(location=0) out vec4 outColor;

layout(set=1, binding=0) uniform samplerCube cubemap;

void main()
{

//    outColor = vec4(0.4, 0.2, 0.1, 1.0);
    outColor = texture(cubemap, normalize(cubeUV));
}