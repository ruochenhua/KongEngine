#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_scalar_block_layout : enable

#include "../common.glsl"

layout(location=0) in vec2 TexCoords;
layout(set=0, binding=0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 cameraPosition;
    SceneLightInfo sceneLightInfo;
} ubo;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inPosition;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inNormal;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput inAlbedo;
layout(input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput inOrm;
/*
sampler2DShadow：专门用于 阴影贴图（深度纹理） 的采样，存储的是单通道深度值（而非颜色）
*/
layout(set = 1, binding = 4) uniform sampler2DShadow shadowmap_tex;

layout(location=0) out vec4 outColor;

// 计算阴影
float ShadowCalculation_DirLight(vec4 frag_world_pos)
{
    mat4 light_space_mat = ubo.sceneLightInfo.directional_light.light_space_mat;
    // 转换到-1,1的范围，再转到0,1的范围
    vec4 frag_pos_light_space = light_space_mat * frag_world_pos;
    // perform perspective divide
    vec3 proj_coord = frag_pos_light_space.xyz / frag_pos_light_space.w;
    // transform to [0,1] range
    // !!!注意注意，这里只对xy做[0,1]映射，不能修改z，否则深度比较会出错
    proj_coord.xy = proj_coord.xy * 0.5 + 0.5;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (proj_coord.z > 1.0 || proj_coord.z < 0.0) return 1.0;

    float shadow = 0.0f;

    // PCF
    vec2 texel_size = 1.0 / vec2(textureSize(shadowmap_tex, 0));
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            // * shadowmap_tex使用sampler2DShadowmap时vulkan中会利用硬件加速自动比较深度
            shadow += texture(shadowmap_tex, vec3(proj_coord.xy + vec2(x, y) * texel_size, proj_coord.z-0.01));
            // * shadowmap_tex用sampler2D使用下面这种
//            float pcf_depth = texture(shadowmap_tex, proj_coord.xy + vec2(x, y) * texel_size).r;
//            shadow += pcf_depth > (proj_coord.z - 0.005) ? 0.0 : 1.0;
        }
    }
    shadow /= 9.0;
    
    // 为了统一sampler2D和sampler2DShadow的返回，使用sampler2D时这里处理一下
//    shadow = (1.0 - shadow)

    return shadow;
}


const float PI = 3.14159265359;

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

void main()
{
    // 这里要用subpassLoad，而不是texture，而且不需要uv
    vec3 fragPos = subpassLoad(inPosition).xyz;
    vec3 objNormal = subpassLoad(inNormal).xyz;
    vec4 orm = subpassLoad(inOrm);

    vec3 view = normalize(ubo.cameraPosition.xyz - fragPos);

    DirectionalLight dirLight = ubo.sceneLightInfo.directional_light;

    vec3 toLightDir = -dirLight.light_dir.xyz;

    vec3 lightColor = dirLight.light_color.xyz;
    vec3 albedoColor = subpassLoad(inAlbedo).rgb;

//    float shadowColor = texture(shadowmap_tex, TexCoords).r;
//    outColor = vec4(vec3(shadowColor), 1.0);
//    return;

    float metallic = orm.z;
    float specularScalar = orm.w;
    float roughness = orm.y;

    // 简单计算一下光照（BRDF）
    vec3 h = normalize(toLightDir + view);
    vec3 radiance = lightColor;   //修正一下light_color的值，免得需要的值太大了

    float NDF = DistributionGGX(objNormal, h, roughness);
    float G = GeometrySmith(objNormal, view, toLightDir, roughness);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedoColor, metallic);

    vec3 F = FresnelSchlick(clamp(dot(h, view), 0.0, 1.0), F0);

    vec3 numerator = NDF*G*F;
    float demoninator = 4.0 * max(dot(objNormal, view), 0.0) * max(dot(objNormal, toLightDir), 0.0) + 0.0001; //加一个小数避免处于0的情况出现
    vec3 specular = numerator / demoninator * specularScalar;
    // KS就是菲涅尔的值
    vec3 KS = F;
    // 根据能量守恒定律，KS+KD不大于1
    vec3 KD = vec3(1.0) - KS;
    // 非金属材质才有漫反射量，这里乘以一下(1-metallic)
    KD *= 1.0 - metallic;
    // 取在观测方向上的一个分量
    float NdotL = max(dot(objNormal, toLightDir), 0.0);

    // return (KD*obj_albedo / PI)*radiance*NdotL;
    //return material.specular_factor;
    vec3 ambient = albedoColor * 0.1;
    vec3 dir_light = (KD*albedoColor / PI + specular)*radiance*NdotL;

    float shadow = ShadowCalculation_DirLight(subpassLoad(inPosition));
    dir_light *= shadow;
    outColor = vec4(ambient + dir_light, 1.0);
}