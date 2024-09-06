#ifndef _COMMON_GLSL_
#define _COMMON_GLSL_

#define POINT_LIGHT_MAX 512
#define POINT_LIGHT_SHADOW_MAX 4
const float PI = 3.14159265359;

struct DirectionalLight
{
    vec4 light_dir;
    vec4 light_color;
    mat4 light_space_mat;
};

struct PointLight
{
    vec4 light_pos;
    vec4 light_color;
};

layout(std140, binding=0) uniform UBO {
    mat4 view;
    mat4 projection;
    vec4 cam_pos;
    vec4 near_far;
} matrix_ubo;

layout(std140, binding=1) uniform LIGHT_INFO_UBO {
    ivec4 has_dir_light;
    DirectionalLight directional_light;
    ivec4 point_light_count;
    PointLight point_lights[POINT_LIGHT_MAX];
    ivec4 point_light_shadow_index;
} light_info_ubo;

int GetPointLightShadowMapIndex(int i, ivec4 shadow_map_index)
{
    if(i == shadow_map_index.x)
    {
        return 0;
    }

    if(i == shadow_map_index.y)
    {
        return 1;
    }

    if(i == shadow_map_index.z)
    {
        return 2;
    }

    if(i == shadow_map_index.w)
    {
        return 3;
    }

    return -1;
}



#endif  // _COMMON_GLSL_