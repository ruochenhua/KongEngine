#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene_texture;
uniform sampler2D dilate_texture;
uniform sampler2D position_texture;

// 模糊采样的颜色和原始颜色的mix上下限
uniform float focus_distance = 3.0;
uniform vec2 focus_threshold;
float min_dist = focus_threshold.x;
float max_dist = focus_threshold.y;

void main()
{
    vec4 focus_color = vec4(texture(scene_texture, TexCoords).xyz, 1.0);    //todo：redo cloud calculation
    vec4 out_of_focus_color = texture(dilate_texture, TexCoords);
    vec3 scene_position = texture(position_texture, TexCoords).xyz; 
    
    vec3 cam_pos = matrix_ubo.cam_pos.xyz;
    float blur_amout = smoothstep(min_dist, max_dist, abs(focus_distance - distance(scene_position, cam_pos)));
    
    FragColor = mix(focus_color, out_of_focus_color, blur_amout);
}