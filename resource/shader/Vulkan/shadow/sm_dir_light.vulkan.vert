#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_scalar_block_layout : enable

#include "../common.glsl"

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

//layout(set=0, binding=0) uniform lightInfo
//{
//    mat4 light_space_mat;
//}light_info;

layout(set=0, binding=0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 cameraPosition;
    SceneLightInfo sceneLightInfo;
} ubo;

layout(push_constant) uniform Push{
    mat4 modelMatrix;
}push;

void main()
{
    gl_Position = ubo.sceneLightInfo.directional_light.light_space_mat * push.modelMatrix * vec4(position, 1);
}