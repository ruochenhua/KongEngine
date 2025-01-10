#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

in vec3 uv;
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

uniform samplerCube skybox;

void main() {
    vec4 color = textureLod(skybox, uv, 0); //vec4(1,0.5,0.5,1);
    FragColor = color;

    vec3 luminance_mask = vec3(0.2126, 0.7152, 0.0722); //人眼对不同颜色的敏感度不同
    float brightness = dot(color.xyz, luminance_mask);
    if(brightness > 2.0)
    {
        BrightColor = vec4(color.xyz, 1.0);
    }
    else
    {
        // 低于阈值的要设置成黑色（或者其他背景色）
        // 否则在blend开启的情况下会导致alpha为0的时候被遮挡的高亮穿透模型在场景中显现
        BrightColor = vec4(0,0,0,1);
    }
}
