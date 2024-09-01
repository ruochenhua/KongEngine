#ifndef _COMMON_GLSL_
#define _COMMON_GLSL_

#define POINT_LIGHT_MAX 32
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
    mat4 model;
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
} light_info_ubo;




#endif  // _COMMON_GLSL_