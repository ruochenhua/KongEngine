#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_scalar_block_layout : enable

#include "common.glsl"

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

layout(location=0) out vec3 cubeUV;

// descriptor set
layout(set=0, binding=0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 cameraPositon;

    SceneLightInfo sceneLightInfo;
} ubo;

void main()
{
    mat4 view_no_translation = ubo.view;
    // 先列再行
	view_no_translation[3] = vec4(0,0,0,1);

	mat4 mvp = ubo.projection * view_no_translation;

    vec4 pos = mvp * vec4(position, 1.0);

	gl_Position = pos.xyww;
    cubeUV = position;
    // Convert cubemap coordinates into Vulkan coordinate space
//	cubeUV.xy *= -1.0;
}