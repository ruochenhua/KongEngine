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

struct SceneLightInfo
{
    ivec4 has_dir_light;
    DirectionalLight directional_light;
    ivec4 point_light_count;
    PointLight point_lights[512];
    ivec4 point_light_shadow_index;
};