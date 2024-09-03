#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
#include "/common/brdf_common.glsl"

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

uniform sampler2D shadow_map;
uniform samplerCube shadow_map_pointlight[4];
uniform sampler2D ssao_result_texture;
// 计算阴影
float ShadowCalculation_DirLight(vec4 pos_light_space, vec3 to_light_dir, vec3 frag_normal)
{
    // 转换到-1,1的范围，再转到0,1的范围
    vec3 proj_coords = pos_light_space.xyz / pos_light_space.w;
    proj_coords = proj_coords* 0.5 + 0.5;
    // 超出阴影贴图范围，就只返回0
    if(proj_coords.z > 1.0 || proj_coords.z < -1.0)
        return 0.0;

//    float closet_depth = texture(shadow_map, proj_coords.xy).r;
    float current_depth = proj_coords.z;
    float bias = 0.0f; //max(0.005 * (1.0 - dot(frag_normal, to_light_dir)), 0.005);
//    float shadow = (current_depth - bias) > closet_depth ? 1.0 : 0.0;
    // 采用pcf柔和阴影锯齿边界
    // todo: 更多优化方法
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texelSize).r;
            shadow += (current_depth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
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
    vec4 frag_pos_lightspace = light_info_ubo.directional_light.light_space_mat * vec4(frag_pos, 1.0);
    float shadow = ShadowCalculation_DirLight(frag_pos_lightspace, to_light_dir, normal);
    return CalcLight(light_color, to_light_dir, normal, view, material) * (1.0 - shadow);
    //return vec3(shadow);
}

vec3 CalcPointLight(PointLight point_light, int light_index, 
                    vec3 normal, vec3 view, vec3 in_frag_pos, BRDFMaterial material, bool calc_shadow)
{
    float kc = 0.2;
    float kl = 0.1;
    float kq = 0.0;
    vec3 light_color = point_light.light_color.xyz;
    vec3 to_light_dir = normalize(point_light.light_pos.xyz - in_frag_pos);

    vec3 point_light_color = CalcLight(light_color, to_light_dir, normal, view, material);

    float shadow = calc_shadow ? ShadowCalculation_pointlight(light_index, in_frag_pos, point_light.light_pos.xyz) : 0;
    
    float distance = length(point_light.light_pos.xyz - in_frag_pos);
    float attenuation = 1.0 / (kc + kl*distance + kq*distance*distance);	//衰减和点光源的参数可控，这里先简单弄个
    //return vec3(shadow);
    return point_light_color * attenuation * (1.0 - shadow);
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
    for(int i = 0; i < light_info_ubo.point_light_count.x; ++i)
    {
        bool calc_shadow = (i < POINT_LIGHT_SHADOW_MAX);
        point_light_color += CalcPointLight(light_info_ubo.point_lights[i], i, frag_normal, view, frag_pos, material, calc_shadow);
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
        ambient = (kD*env_diffuse + env_specular)*ao;
    }
    else
    {
        ambient = F0 *env_albedo.xyz *ao;
    }

    float occlusion = texture(ssao_result_texture, TexCoords).x;
    vec3 color = ambient + (dir_light_color + point_light_color);

    // FragColor = GetAlbedo();
    // FragColor = vec4((frag_normal+1)/2, 1.0);
    // FragColor = vec4(vec3(env_roughness), 1.0);
    // FragColor = vec4(vec3(env_metallic), 1.0);
    // FragColor = skybox_color;
    FragColor = vec4(color, 1.0);
//    FragColor = vec4(texture(ssao_result_texture, TexCoords).xxx, 1.0);
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

}