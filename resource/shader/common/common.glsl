#ifndef _COMMON_GLSL_
#define _COMMON_GLSL_

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

#define POINT_LIGHT_MAX 32
#define POINT_LIGHT_SHADOW_MAX 4
const float PI = 3.14159265359;
#endif  // _COMMON_GLSL_