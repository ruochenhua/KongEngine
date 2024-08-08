#version 450 compatibility

layout(location=0) in vec3 in_pos;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_texcoord;
layout(location=3) in vec3 in_tangent;
layout(location=4) in vec3 in_bitangent;

// out vec3 normal_world;
out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_uv;
out mat3 TBN;
out vec4 frag_pos_lightspace;

layout(std140, binding=0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cam_pos;
} matrix_ubo;

uniform mat3 normal_model_mat;
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

#define POINT_LIGHT_MAX 4
layout(std140, binding=1) uniform LIGHT_INFO_UBO {
	ivec4 has_dir_light;
    DirectionalLight directional_light;
	ivec4 point_light_count;
    PointLight point_lights[POINT_LIGHT_MAX];
} light_info_ubo;


void main(){
    mat4 model = matrix_ubo.model;
    gl_Position = matrix_ubo.projection * matrix_ubo.view * model * vec4(in_pos, 1.0);
    frag_pos = (model * vec4(in_pos, 1.0)).xyz;

    // 法线没有位移，不需要w向量，且还需要一些特殊处理来处理不等比缩放时带来的问题
    frag_normal = normalize(normal_model_mat * in_normal);
    frag_uv = in_texcoord;
    frag_pos_lightspace = light_info_ubo.directional_light.light_space_mat * vec4(frag_pos, 1.0);

    vec3 tangent = in_tangent;
    
    vec3 T = normalize(vec3(model*vec4(in_tangent, 0.0)));
    vec3 B = normalize(vec3(model*vec4(in_bitangent, 0.0)));
    vec3 N = normalize(vec3(model*vec4(frag_normal, 0.0)));
    
    TBN = mat3(T, B, N);

    //ShadowCoord = depth_bias_mvp * vec4(vertexPosition_modelspace, 1);
}