#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
// tessellation primitive generator没有shader代码， 它根据tcs的输出数据和tes的输入数据结构设定计算
// 如下方(quads,fractional_odd_spacing, ccw)代表了细分primitive的设置
layout(quads, fractional_odd_spacing, ccw) in;

uniform sampler2D height_map;
// patch的四个uv数据
in vec2 TextureCoord[];
out float tes_height;

void main(){
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = TextureCoord[0];
    vec2 t01 = TextureCoord[1];
    vec2 t10 = TextureCoord[2];
    vec2 t11 = TextureCoord[3];

    // 根据细分的uv，lerp得到tesscoord对应的贴图上的uv坐标
    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    tes_height = texture(height_map, texCoord).y * 64.0 - 16.0;

    // 获取patch的四个顶点
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    // 得出法线
    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );

    // lerp得到细分的p点，并根据高度图和法线得到实际的高度（原P点的y为0）
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0 + normal * tes_height;

    gl_Position = matrix_ubo.projection * matrix_ubo.view * p;
}