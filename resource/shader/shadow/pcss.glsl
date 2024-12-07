#ifndef _PCSS_GLSL_
#define _PCSS_GLSL_

// PCSS算法的一些辅助函数

// 双线性滤波采样阴影图（提高深度采样精度）
float TextureProjBilinear(sampler2DArray shadowMap, vec2 uv)
{

    // 获取纹理坐标的整数部分和小数部分
    vec2 texture_size = vec2(textureSize(shadowMap, 0));
    vec2 texel_size = 1.0 / texture_size;

    vec2 f = fract(uv * texture_size)/texture_size;
    uv -= f;
    
    // 双线性插值采样
    float tl = texture(shadowMap, vec3(uv, 0)).r;
    float tr = texture(shadowMap, vec3(uv + vec2(texel_size.x, 0.0), 0)).r;
    float bl = texture(shadowMap, vec3(uv + vec2(0.0, texel_size.y), 0)).r;
    float br = texture(shadowMap, vec3(uv + texel_size, 0)).r;

    return mix(mix(tl, tr, f.x), mix(bl, br, f.x), f.y);
}

// 计算遮挡物平均深度（用于估计遮挡物范围）
//float FindBlockerDepth(sampler2D shadowMap, vec2 uv, float z_receiver, float radius)
//{
//    return 0.0;
//}
float FindBlockerDepth(sampler2DArray shadowmap, vec2 uv, float d_receiver, float radius)
{
    float blocker_depth_sum = 0.0;
    int blocker_count = 0;
    
    // 以当前像素为中心,半径为radius的范围采样
    for (float y = -radius; y <= radius; y++) {
        for (float x = -radius; x <= radius; x++) {
            vec2 offset = vec2(x, y) * 1.0 / vec2(textureSize(shadowmap, 0));
            float sampleDepth = TextureProjBilinear(shadowmap, uv + offset);
            if (sampleDepth < d_receiver) {
                blocker_depth_sum += sampleDepth;
                blocker_count++;
            }
        }
    }
    return blocker_count > 0? blocker_depth_sum / float(blocker_count) : 0.0;
}

// 计算遮挡物范围半径（基于相似三角形原理）
float EstimateBlockerSearchRadius(vec2 uv, float d_receiver, float d_blocker, float light_size)
{
    if (d_blocker == 0.0) return 0.0;
//    return 2.0;
    return (d_receiver - d_blocker) * (light_size / d_blocker);
}

float CalculatePCFShadow(float current_depth, sampler2DArray shadow_map, int layer, vec2 uv, int radius)
{
    float shadow = 0.0;
    vec2 texel_size = 1.0 / vec2(textureSize(shadow_map, 0));
    for (int x = -radius; x <= radius; ++x)
    {
        for (int y = -radius; y <= radius; ++y)
        {
            float pcf_depth = texture(shadow_map, vec3(uv + vec2(x, y) * texel_size, layer)).r;
            shadow += current_depth > pcf_depth ? 1.0 : 0.0;
        }
    }
    shadow /= pow((radius+2),2.0);
    return shadow;
}

#endif // _PCSS_GLSL_