#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

layout(vertices=4) out;
in vec2 out_tex[];
out vec2 TextureCoord[];

const int MIN_TESS_LEVEL = 4;
const int MAX_TESS_LEVEL = 32;
const float MIN_DISTANCE = 20;
const float MAX_DISTANCE = 2500;
void main(){
    mat4 view = matrix_ubo.view;

    // 传输数据，将每个patch 的顶点信息转送
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    TextureCoord[gl_InvocationID] = out_tex[gl_InvocationID];

    // invocation id为0的点控制整个patch的细分等级
    if(gl_InvocationID == 0)
    {
        vec4 eye_space_pos_00 = view * gl_in[0].gl_Position;
        vec4 eye_space_pos_01 = view * gl_in[1].gl_Position;
        vec4 eye_space_pos_10 = view * gl_in[2].gl_Position;
        vec4 eye_space_pos_11 = view * gl_in[3].gl_Position;

        // 根据顶点距离相机的距离，动态设定细分的精度
        float dist_length = MAX_DISTANCE - MIN_DISTANCE;
        float distance_00 = clamp((abs(eye_space_pos_00.z) - MIN_DISTANCE)/dist_length, 0.0, 1.0);
        float distance_01 = clamp((abs(eye_space_pos_01.z) - MIN_DISTANCE)/dist_length, 0.0, 1.0);
        float distance_10 = clamp((abs(eye_space_pos_10.z) - MIN_DISTANCE)/dist_length, 0.0, 1.0);
        float distance_11 = clamp((abs(eye_space_pos_11.z) - MIN_DISTANCE)/dist_length, 0.0, 1.0);

        float tess_level_0 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance_10, distance_00));
        float tess_level_1 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance_00, distance_01));
        float tess_level_2 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance_01, distance_11));
        float tess_level_3 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance_11, distance_10));

        // 决定tesselation的程度
        // outer代表细分的点和其他patch之间的连接细分
        gl_TessLevelOuter[0] = tess_level_0;
        gl_TessLevelOuter[1] = tess_level_1;
        gl_TessLevelOuter[2] = tess_level_2;
        gl_TessLevelOuter[3] = tess_level_3;

        // inner代表patch之内的的点的连接的细分
        gl_TessLevelInner[0] = max(tess_level_1, tess_level_3);
        gl_TessLevelInner[1] = max(tess_level_0, tess_level_2);
    }
}