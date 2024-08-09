#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl" 
#include "/common/brdf_common.glsl"

out vec4 FragColor;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in mat3 TBN;
in vec4 frag_pos_lightspace;

layout(std140, binding=1) uniform LIGHT_INFO_UBO {
	ivec4 has_dir_light;
    DirectionalLight directional_light;
	ivec4 point_light_count;
    PointLight point_lights[POINT_LIGHT_MAX];
} light_info_ubo;

layout(std140, binding=0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cam_pos;
} matrix_ubo;

uniform vec3 albedo;    // color
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
uniform sampler2D normal_texture;
uniform sampler2D shadow_map;
uniform samplerCube shadow_map_pointlight;


vec3 GetAlbedo()
{
    float texture_size = textureSize(diffuse_texture, 0).x;
    if(texture_size > 1.0)
    {
        vec3 texture_albedo = texture(diffuse_texture, frag_uv).rgb;
        // sRGB空间转换到线性空间
        return pow(texture_albedo, vec3(2.2));
    }

    return albedo;
}

vec3 GetNormal()
{
    float texture_size = textureSize(normal_texture, 0).x;
    if(texture_size > 1.0)
    {
        vec3 texture_normal = texture(normal_texture, frag_uv).rgb;
        // 从[0,1]映射到[-1,1]
        vec3 tex_normal = normalize(texture_normal*2.0 - 1.0);
        return normalize(TBN * tex_normal);
    }

    return frag_normal;
}

// 计算阴影
float ShadowCalculation_DirLight(vec4 pos_light_space, vec3 to_light_dir)
{
    // 转换到-1,1的范围，再转到0,1的范围
    vec3 proj_coords = pos_light_space.xyz / pos_light_space.w;
    proj_coords = proj_coords* 0.5 + 0.5;
    // 超出阴影贴图范围，就只返回0
    if(proj_coords.z > 1.0)
        return 0.0;

//    float closet_depth = texture(shadow_map, proj_coords.xy).r;
    float current_depth = proj_coords.z;
    float bias = max(0.0001 * (1.0 - dot(frag_normal, to_light_dir)), 0.0001);
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

float ShadowCalculation_pointlight(vec3 in_frag_pos, vec3 light_pos)
{
    vec3 frag_to_light = in_frag_pos - light_pos;
    // 立方体贴图采样
    float close_depth = texture(shadow_map_pointlight, frag_to_light).r;
    // 从0-1映射到0-farplane
    float far_plane = 30.f;
    close_depth *= far_plane;
    // 这里计算的是真实光源到像素点世界位置的距离
    float current_depth = length(frag_to_light);
    float bias = 0.0001;
    // 进行比较，如果贴图上的距离比真实距离小，则代表在阴影当中
    float shadow = (current_depth - bias) > close_depth ? 1.0 : 0.0;
    
    return shadow;

}

vec3 CalcLight(vec3 light_color, vec3 to_light_dir, vec3 normal, vec3 view)
{
    BRDFMaterial material;
    material.roughness = roughness;
    material.metallic = metallic;
    material.albedo = GetAlbedo();
    material.ao = ao;

    return CalcLight_BRDF(light_color, to_light_dir, normal, view, material);
}

// calculate color causes by directional light
vec3 CalcDirLight(DirectionalLight dir_light, vec3 normal, vec3 view)
{
    vec3 light_color = dir_light.light_color.xyz;
    vec3 to_light_dir = -dir_light.light_dir.xyz;

    float shadow = ShadowCalculation_DirLight(frag_pos_lightspace, to_light_dir);
    return CalcLight(light_color, to_light_dir, normal, view)  * (1.0 - shadow);
    //return vec3(shadow);
}

vec3 CalcPointLight(PointLight point_light, vec3 normal, vec3 view, vec3 in_frag_pos)
{
    vec3 light_color = point_light.light_color.xyz;
    vec3 to_light_dir = normalize(point_light.light_pos.xyz - in_frag_pos);

    vec3 point_light_color = CalcLight(light_color, to_light_dir, normal, view);

    float shadow = ShadowCalculation_pointlight(frag_pos, point_light.light_pos.xyz);
    float distance = length(point_light.light_pos.xyz - in_frag_pos);
    float attenuation = 1.0 / (distance * distance);	//衰减和点光源的参数可控，这里先简单弄个
    return point_light_color * attenuation * (1.0 - shadow);
}

void main()
{
    vec3 view = normalize(matrix_ubo.cam_pos - frag_pos);  //to_view
    vec3 obj_normal = GetNormal();

    vec3 dir_light_color = vec3(0,0,0);
	if(light_info_ubo.has_dir_light.x > 0)
	{
		dir_light_color = CalcDirLight(light_info_ubo.directional_light, obj_normal, view);
	}

    vec3 point_light_color = vec3(0,0,0);
    for(int i = 0; i <  min(light_info_ubo.point_light_count.x,4); ++i)
    {
        point_light_color += CalcPointLight(light_info_ubo.point_lights[i], obj_normal, view, frag_pos);
    }


    vec3 ambient = vec3(0.03)*GetAlbedo()*ao;
    vec3 color = ambient + (dir_light_color + point_light_color);
    // 伽马校正（Reinhard）
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}