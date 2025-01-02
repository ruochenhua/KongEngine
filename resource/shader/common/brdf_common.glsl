#ifndef _BRDF_COMMON_GLSL_
#define _BRDF_COMMON_GLSL_

struct BRDFMaterial
{
    vec4 albedo;    // color
    float specular_factor;
    float metallic;
    float roughness;
    float ao;
};

// 完整的Cook-Torrance specular BRDF: DFG / 4*dot(N,V)*dot(N,L)

// 菲涅尔方程F
// Fresnel-Schlick近似法接收一个参数F0，被称为0°入射角的反射率，或者说是直接(垂直)观察表面时有多少光线会被反射。
// 这个参数F0会因为材料不同而不同，而且对于金属材质会带有颜色。
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// 和BRDF LUT相关的取值，
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// note: 根据迪士尼公司给出的观察以及后来被Epic Games公司采用的光照模型，在几何遮蔽函数和法线分布函数中采用粗糙度的平方会让光照看起来更加自然。

// 法线分布函数D
// 从统计学上近似地表示了与某些（半程）向量h取向一致的微平面的比率。
// 当粗糙度很低（也就是说表面很光滑）的时候，与半程向量取向一致的微平面会高度集中在一个很小的半径范围内。
// 由于这种集中性，NDF最终会生成一个非常明亮的斑点。但是当表面比较粗糙的时候，微平面的取向方向会更加的随机
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}
// 几何遮蔽函数G
// 从统计学上近似的求得了微平面间相互遮蔽的比率，这种相互遮蔽会损耗光线的能量。
// 几何函数采用一个材料的粗糙度参数作为输入参数，粗糙度较高的表面其微平面间相互遮蔽的概率就越高。
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1  = GeometrySchlickGGX(NdotV, roughness);
    float ggx2  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 CalcLight_BRDF(vec3 light_color, vec3 to_light_dir, vec3 normal, vec3 view, BRDFMaterial material)
{
    vec3 h = normalize(to_light_dir + view);
    vec3 radiance = light_color * 180.0 / PI;   //修正一下light_color的值，免得需要的值太大了

    float NDF = DistributionGGX(normal, h, material.roughness);
    float G = GeometrySmith(normal, view, to_light_dir, material.roughness);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, material.albedo.xyz, material.metallic);

    vec3 F = FresnelSchlick(clamp(dot(h, view), 0.0, 1.0), F0);
    //vec3 F = FresnelSchlick(clamp(dot(h, to_light_dir), 0.0, 1.0), F0);

    vec3 numerator = NDF*G*F;
    float demoninator = 4.0 * max(dot(normal, view), 0.0) * max(dot(normal, to_light_dir), 0.0) + 0.0001; //加一个小数避免处于0的情况出现
    vec3 specular = numerator / demoninator * material.specular_factor;
    // KS就是菲涅尔的值
    vec3 KS = F;
    // 根据能量守恒定律，KS+KD不大于1
    vec3 KD = vec3(1.0) - KS;
    // 非金属材质才有漫反射量，这里乘以一下(1-metallic)
    KD *= 1.0 - material.metallic;
    // 取在观测方向上的一个分量
    float NdotL = max(dot(normal, to_light_dir), 0.0);

    // return (KD*obj_albedo / PI)*radiance*NdotL;
    //return material.specular_factor;
    return (KD*material.albedo.xyz / PI + specular)*radiance*NdotL;
    //return specular;
}
#endif  // _BRDF_COMMON_GLSL_
