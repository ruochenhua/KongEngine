#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
#include "/common/brdf_common.glsl"
#include "/shadow/rsm_sample_points.glsl"
#include "/shadow/pcss.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform vec3 cam_pos;
uniform bool b_render_skybox;

uniform sampler2D position_texture;
uniform sampler2D normal_texture;
uniform sampler2D albedo_texture;
uniform sampler2D orm_texture;

uniform samplerCube skybox_texture;
uniform samplerCube skybox_diffuse_irradiance_texture;
uniform samplerCube skybox_prefilter_texture;
uniform sampler2D skybox_brdf_lut_texture;

uniform sampler2DArray shadow_map;
uniform sampler2D rsm_world_pos;
uniform sampler2D rsm_world_normal;
uniform sampler2D rsm_world_flux;

uniform samplerCube shadow_map_pointlight[4];
uniform mat4 light_space_matrices[16];
uniform float csm_distances[16];
uniform int csm_level_count;

uniform bool use_ssao;
uniform sampler2D ssao_result_texture;

uniform bool use_rsm;
uniform float rsm_intensity;
uniform int rsm_sample_count;
uniform vec4 rsm_samples_and_weights[32];

uniform bool use_pcss = false;
uniform float pcss_radius = 1.0;
uniform float pcss_light_scale = 0.1;
uniform int pcss_sample_count = 36;
// 计算阴影
float ShadowCalculation_DirLight(vec4 frag_world_pos, vec3 to_light_dir, vec3 in_normal, out vec3 env_color)
{
    vec4 frag_pos_view_space = matrix_ubo.view * frag_world_pos;
    float depthValue = abs(frag_pos_view_space.z);

    int layer = -1;
    for (int i = 0; i < csm_level_count; ++i)
    {
        if (depthValue < csm_distances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = csm_level_count;
    }

    // 转换到-1,1的范围，再转到0,1的范围
    vec4 frag_pos_light_space = light_space_matrices[layer] * frag_world_pos;
    // perform perspective divide
    vec3 proj_coord = frag_pos_light_space.xyz / frag_pos_light_space.w;
    // transform to [0,1] range
    proj_coord = proj_coord * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float current_depth = proj_coord.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (current_depth > 1.0)
    {
        return 0.0;
    }
    
    float shadow = 0.0f;
    // PCSS（只在csm最低级别开启）
    if(use_pcss && layer == 0)
    {
        float d_recv = current_depth;

        float d_blocker = FindBlockerDepth(shadow_map, proj_coord.xy, d_recv, pcss_radius);
        float blocker_radius = EstimateBlockerSearchRadius(proj_coord.xy, d_recv, d_blocker, pcss_light_scale);
        vec2 texel_size = 1.0 / vec2(textureSize(shadow_map, 0));
        // 阻挡区域小就回退到PCF方法
        if(blocker_radius < texel_size.x)
        {
//            shadow = d_recv > TextureProjBilinear(shadow_map, proj_coord.xy) ? 1.0 : 0.0;
            shadow = CalculatePCFShadow(current_depth, shadow_map, layer, proj_coord.xy, 1);
        }
        else
        {
            float shadow_sum = 0.0f;
            for(int pcss_i = 0; pcss_i < pcss_sample_count; pcss_i++)
            {
                // 可以使用泊松采样盘等方法获取更自然的采样点位置，这里简单均匀采样
                vec2 offset = vec2(cos(float(pcss_i) * 2.0 * 3.1415926 / float(pcss_sample_count)),
                sin(float(pcss_i) * 2.0 * 3.1415926 / float(pcss_sample_count))) * blocker_radius;

                vec4 sampleLightSpacePos = vec4(proj_coord.xy + offset, proj_coord.z, 1.0);
                float sampleDepth = TextureProjBilinear(shadow_map, proj_coord.xy+offset);
                shadow_sum += sampleDepth < d_recv? 1.0 : 0.0;
            }
            shadow = shadow_sum / pcss_sample_count;
        }
    }
    else
    {
        // PCF
//        vec2 texel_size = 1.0 / vec2(textureSize(shadow_map, 0));
//        for (int x = -1; x <= 1; ++x)
//        {
//            for (int y = -1; y <= 1; ++y)
//            {
//                float pcf_depth = texture(shadow_map, vec3(proj_coord.xy + vec2(x, y) * texel_size, layer)).r;
//                shadow += current_depth > pcf_depth ? 1.0 : 0.0;
//            }
//        }
//        shadow /= 9.0;
        shadow = CalculatePCFShadow(current_depth, shadow_map, layer, proj_coord.xy, 1);
    }
    
    if(use_rsm)
    {
        // 只计算layer是0的rsm效果
        if (layer == 0)
        {
            float texel_size = 1.0 / 2048.0;
//            for (int i = 0; i < N_SAMPLES; ++i) 
//            {
//
//                vec2 uv = proj_coord.xy + R_MAX * rsm_sample_offsets[i];
//                vec3 flux = texture(rsm_world_flux, uv).rgb;
//                vec3 x_p = texture(rsm_world_pos, uv).xyz;
//                vec3 n_p = texture(rsm_world_normal, uv).xyz;
//
//                vec3 r = frag_world_pos.xyz - x_p;
//                float d2 = dot(r, r);
//                vec3 e_p = flux * (max(0.0, dot(n_p, r)) * max(0.0, dot(in_normal, -r)));
//                e_p *= pow(rsm_sample_offsets[i].x / d2, 2);
//                env_color += e_p;
//            }
//
//             env_color *= rsm_intensity;
            float max_sample_radius = 128.;
            for (int i = 0; i < rsm_sample_count; ++i) 
            {
                vec3 rsm_sample_and_weight = rsm_samples_and_weights[i].xyz;
                vec2 uv = proj_coord.xy + max_sample_radius * rsm_sample_and_weight.xy * texel_size;
                vec3 flux = texture(rsm_world_flux, uv).rgb;
                vec3 x_p = texture(rsm_world_pos, uv).xyz;
                vec3 n_p = texture(rsm_world_normal, uv).xyz;

                vec3 r = normalize(frag_world_pos.xyz - x_p);

                float d2 = dot(r, r);
                vec3 e_p = flux * (max(0.0, dot(n_p, r)) * max(0.0, dot(in_normal, -r))) * rsm_sample_and_weight.z;
                //e_p *= pow(rsm_sample_offsets[i].x / d2, 2);
                env_color += e_p;
            }
            env_color /= rsm_sample_count;
        }
    }
    
    return shadow;
}

float ShadowCalculation_pointlight(int light_index, vec3 in_frag_pos, vec3 light_pos)
{
    vec3 frag_to_light = in_frag_pos - light_pos;
    // 立方体贴图采样
    // float close_depth = texture(shadow_map_pointlight[light_index], frag_to_light).r;
    // 从0-1映射到0-farplane
    float far_plane = 30.f;
    // close_depth *= far_plane;
    // 这里计算的是真实光源到像素点世界位置的距离
    float current_depth = length(frag_to_light);
    if(current_depth > far_plane)
    {
        return 0.0;
    }
    float bias = 0.0f;
    // 进行比较，如果贴图上的距离比真实距离小，则代表在阴影当中
    //float shadow = (current_depth - bias) > close_depth ? 1.0 : 0.0;
    float shadow = 0.0;
    vec3 texelSize = vec3(0.001);//1.0 / textureSize(shadow_map_pointlight, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadow_map_pointlight[light_index], frag_to_light + vec3(x, y, 0) * texelSize).r * far_plane;
            shadow += (current_depth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}

vec3 CalcLight(vec3 light_color, vec3 to_light_dir, vec3 normal, vec3 view, BRDFMaterial material)
{
    return CalcLight_BRDF(light_color, to_light_dir, normal, view, material);
}

// calculate color causes by directional light
vec3 CalcDirLight(DirectionalLight dir_light, vec3 normal, vec3 view, vec3 frag_pos, BRDFMaterial material)
{
    vec3 light_color = dir_light.light_color.xyz;
    vec3 to_light_dir = -dir_light.light_dir.xyz;
    vec3 env_color = vec3(0);
    float shadow = ShadowCalculation_DirLight(vec4(frag_pos,1.0), to_light_dir, normal, env_color);
    return (CalcLight(light_color, to_light_dir, normal, view, material) + env_color)* (1.0 - shadow) ;
}

vec3 CalcPointLight(PointLight point_light, int light_index, 
                    vec3 normal, vec3 view, vec3 in_frag_pos, BRDFMaterial material, int shadow_map_index)
{
    float kc = 0.2;
    float kl = 0.1;
    float kq = 0.0;
    vec3 light_color = point_light.light_color.xyz;
    vec3 to_light_dir = normalize(point_light.light_pos.xyz - in_frag_pos);

    vec3 point_light_color = CalcLight(light_color, to_light_dir, normal, view, material);

    float shadow = shadow_map_index>=0 ? ShadowCalculation_pointlight(shadow_map_index, in_frag_pos, point_light.light_pos.xyz) : 0;
    
    float distance = length(point_light.light_pos.xyz - in_frag_pos);
    float attenuation = 1.0 / (kc + kl*distance + kq*distance*distance);	//衰减和点光源的参数可控，这里先简单弄个
    //return vec3(shadow);
    return point_light_color * attenuation * (1.0 - shadow);
}

vec3 ApplyFog(vec3 origin_color, float pixel_dist)
{    
    vec3 fog_color = vec3(0.5,0.6, 0.7);
    float fog_fall_off = 0.0015;
    float fog_amount = 1.0 - exp(-pixel_dist*fog_fall_off);

    return mix(origin_color, fog_color, fog_amount);
}

void main()
{
    vec3 frag_pos = texture(position_texture, TexCoords).xyz;
    vec3 frag_normal = texture(normal_texture, TexCoords).rgb;
    vec4 env_albedo = texture(albedo_texture, TexCoords);

    vec3 orm = texture(orm_texture, TexCoords).rgb;
    float ao = orm.x;
    float env_roughness = orm.y;
    float env_metallic = orm.z;

    BRDFMaterial material;
    material.roughness = env_roughness;
    material.metallic = env_metallic;
    material.albedo = env_albedo;
	material.specular_factor = 1.0;
    material.ao = ao;

    vec3 view = normalize(cam_pos - frag_pos);  //to_view

    vec3 dir_light_color = vec3(0,0,0);
	if(light_info_ubo.has_dir_light.x > 0)
	{
		dir_light_color = CalcDirLight(light_info_ubo.directional_light, frag_normal, view,  frag_pos, material);
	}

    vec3 point_light_color = vec3(0,0,0);
    ivec4 shadow_index = light_info_ubo.point_light_shadow_index;
    for(int i = 0; i < light_info_ubo.point_light_count.x; ++i)
    {
        int shadow_map_index = GetPointLightShadowMapIndex(i, shadow_index);
        point_light_color += CalcPointLight(light_info_ubo.point_lights[i], i, frag_normal, view, frag_pos, material, shadow_map_index);
    }

    // 用IBL的辐照度贴图作为环境光
    vec3 skybox_irradiance = texture(skybox_diffuse_irradiance_texture, frag_normal).xyz;
    vec3 F0 = vec3(0.03);

    vec3 ambient = vec3(0.0);

    if(b_render_skybox)
    {
        F0 = mix(F0, env_albedo.xyz, env_metallic);
        vec3 kS = FresnelSchlickRoughness(max(dot(frag_normal, view), 0.0), F0, env_roughness);
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - env_metallic;
        // IBL漫反射分量
        vec3 env_diffuse = env_albedo.xyz * skybox_irradiance;
        // IBL镜面反射分量
        vec3 reflect_vec = reflect(-view, frag_normal);
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefiltered_color = textureLod(skybox_prefilter_texture, reflect_vec, env_roughness*MAX_REFLECTION_LOD).xyz;

        vec2 env_BRDF = texture(skybox_brdf_lut_texture, vec2(max(dot(frag_normal, view), 0.0), env_roughness)).rg;
        vec3 env_specular = prefiltered_color * (kS * env_BRDF.x + env_BRDF.y);
        ambient = (kD*env_diffuse + env_specular);
    }
    else
    {
        ambient = F0 *env_albedo.xyz;
    }

    // 仅仅作用于ambient light的话太不明显了是什么原因？
    vec3 color = ambient + (dir_light_color + point_light_color);
    if(use_ssao)
    {
        float occlusion = texture(ssao_result_texture, TexCoords).x;
        color *= occlusion;
    }

    float pixel_dist = length(cam_pos - frag_pos);
    vec3 final_color = ApplyFog(color, pixel_dist);

    FragColor = vec4(final_color, 1.0);

    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
    {
        BrightColor = vec4(color, 1.0);
    }
    else
    {
        // 低于阈值的要设置成黑色（或者其他背景色）
        // 否则在blend开启的情况下会导致alpha为0的时候被遮挡的高亮穿透模型在场景中显现
        BrightColor = vec4(0,0,0,1);
    }


//    FragColor = vec4(texture(rsm_world_flux, TexCoords).rgb/100.0, 1.0);
//    FragColor = vec4(texture(rsm_world_pos, TexCoords).rgb, 1.0);
//    FragColor = vec4(texture(rsm_world_normal, TexCoords).rgb*0.5+0.5, 1.0);

    //FragColor = vec4(frag_normal*0.5+0.5,1.0);
}