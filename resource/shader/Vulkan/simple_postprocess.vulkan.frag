#version 450

//layout(push_constant) uniform Push{
//    float camera_exposure;
//    int use_bloom;
//} push;

layout(set=0, binding=0) uniform sampler2D scene_texture;
layout(set=1, binding=0) uniform PostProcessUbo{
    float exposure;
    int bloom;
} pp_ubo;

layout(location=0) in vec2 fragUV;
layout(location=0) out vec4 outColor;

void main()
{
    vec3 scene_color = texture(scene_texture, fragUV).rgb;
    //    outColor = vec4(0.1, 0.3, 0.1, 1.0);
    // reinhard色调映射
    vec3 mapped = vec3(1.0)-exp(-scene_color * pp_ubo.exposure);

    const float gamma = 2.2;
    mapped = pow(mapped, vec3(1.0/gamma));

    outColor = vec4(mapped, 1.0);
    outColor = vec4(scene_color, 1.0);
}