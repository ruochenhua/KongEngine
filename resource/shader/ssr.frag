#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
#include "/common/brdf_common.glsl"

layout(location = 0) out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene_position;
uniform sampler2D scene_normal;
uniform sampler2D scene_color;
uniform sampler2D orm_texture;
vec2 tex_size = textureSize(scene_position, 0).xy;
float thickness = 0.05;

mat4 inv_vp = inverse(matrix_ubo.projection * matrix_ubo.view);

float rand(float seed)
{
    return fract(sin(seed) * 43758.43463452);
}

vec3 randVec3(float seed)
{
    return normalize(vec3(rand(seed), rand(seed+1), rand(seed+2)));
}

bool campareDepth(vec4 start_screen, vec4 end_screen, vec3 start_world, vec3 end_world, float sample_t, out vec2 uv)
{
    vec3 cam_pos = matrix_ubo.cam_pos.xyz;
    vec4 screen_p = mix(start_screen, end_screen, sample_t);
    uv = (screen_p.xy/screen_p.w)*0.5 + 0.5;   // 转换为[0,1]
    //vec2 uv = screen_p * tex_size;
    if(uv.x < 0.0 || uv.y < 0.0 || uv.x > 1.0 || uv.y > 1.0)
    {
        return false;
    }

    // 获取到屏幕空间这个点的深度值
    float sample_depth = texture(scene_normal, uv).w;

    // 世界空间的点通过当前的采样比例映射
    vec3 sample_pos = mix(start_world, end_world, sample_t);
    float p_depth = distance(cam_pos, sample_pos);
    if (p_depth > sample_depth  && p_depth < sample_depth + thickness)
    {
        return true;
    }

    return false;
}

bool compareDepth(vec4 ndc_coord, vec2 uv)
{
//    vec4 ndc_coord = vec4(mix(start_ndc, end_ndc, current_percentage), 1.0);
    vec3 cam_pos = matrix_ubo.cam_pos.xyz;

    // NDC转换回世界坐标
    // 推到过程参考：https://blog.csdn.net/forcsdn_tang/article/details/125989895
    float clip_w = 1.0 / (inv_vp * ndc_coord).w;
    // 计算出来的的光线步进在世界上的位置
    vec4 raymacrh_world = inv_vp * ndc_coord * clip_w;

    // 获取到屏幕空间这个点的深度值
    float sample_depth = texture(scene_normal, uv).w;
    // 相机到光线步进的距离
    float p_depth = distance(cam_pos, raymacrh_world.xyz);
    return (p_depth > sample_depth && p_depth < sample_depth  + thickness);
}



#define USE_SCREEN_SPACE_MARCH 1

// NDC的坐标和最终屏幕空间坐标的比例保持是一致的
// NDC坐标和世界、视图空间坐标可以相互转换
// 在屏幕空间进行步进，按照屏幕空间的坐标分段，获取采样点的屏幕空间坐标后按比例转换为NDC坐标，后再转换为世界、视图空间坐标来去步进的深度值
void main()
{
    vec2 tex_size = textureSize(scene_position, 0).xy;
    vec2 tex_uv = gl_FragCoord.xy / tex_size;

    vec4 orm = texture(orm_texture, TexCoords);
    float roughness = orm.y;
    float metallic = orm.z;
    vec4 s_color = texture(scene_color, TexCoords);

    FragColor = s_color;

    vec4 normal_depth = texture(scene_normal, TexCoords);
    vec3 world_normal = normalize(normal_depth.xyz + randVec3(fract(TexCoords.x*12.345)*sin(TexCoords.y)*9876.31)*0.2*roughness);
    // 远近平面
    vec2 near_far = matrix_ubo.near_far.xy;
    vec3 world_pos = texture(scene_position, TexCoords).xyz;
    vec3 cam_pos = matrix_ubo.cam_pos.xyz;

    mat4 projection = matrix_ubo.projection;
    mat4 view = matrix_ubo.view;
    mat4 vp = projection * view;    // 世界坐标到裁切坐标的转换矩阵

    // mat4 inv_vp = inverse(vp);

    vec3 view_dir = normalize(world_pos-cam_pos);
    vec3 rd = normalize(reflect(view_dir, world_normal));
    vec3 rand_vec = randVec3(rd.x) * 0.0;
    // rd = normalize(rd + rand_vec * roughness);
    float resolution = 0.5;

   // 往相机方向反射的暂时先不管
//        if(dot(view_dir, rd) > -0.0)
    {
#if USE_SCREEN_SPACE_MARCH==1
        float max_step_dist = 5.0;
#else
        float max_step_dist = 5.0;
#endif
        // 在屏幕空间进行光线步进
        // 起始点和结束点
        vec3 start_pos_world = world_pos  + rd*0.1;
        vec3 end_pos_world = world_pos + max_step_dist*rd;
        // 在屏幕空间上的从起始点到结束点的坐标[0, resolution]
        vec4 start_clip = vp * vec4(start_pos_world, 1.0);
        vec4 end_clip   = vp * vec4(end_pos_world, 1.0);

        vec3 start_ndc  = start_clip.xyz / start_clip.w;
        vec3 end_ndc    = end_clip.xyz / end_clip.w;
        vec3 ndc_diff = end_ndc - start_ndc;

        // ndc->屏幕坐标 [0, resolution.xy]
        vec3 start_screen  = vec3(0);
        start_screen.xy = (start_ndc.xy + 1) / 2 * tex_size;
        start_screen.z = (near_far.y - near_far.x) * 0.5 * start_ndc.z + (near_far.x + near_far.y) * 0.5;
        vec3 end_screen    = vec3(0);
        end_screen.xy = (end_ndc.xy + 1) / 2 * tex_size;
        end_screen.z = (near_far.y - near_far.x) * 0.5 * end_ndc.z + (near_far.x + near_far.y) * 0.5;

#if USE_SCREEN_SPACE_MARCH==1
        int step_count = 32;

        vec3 screen_diff = end_screen - start_screen;
        int sample_count = int(max(abs(screen_diff.x), abs(screen_diff.y)) * resolution) ; // 大于1
        sample_count = min(sample_count, 128);
        vec3 delta_screen = screen_diff / float(sample_count);

        // 如果sample count为10，则每次采样的前进的长度为总长度的1/10
        float percentage_delta = 1.0 / float(sample_count);
        vec3 current_screen = start_screen;
        vec3 last_screen = current_screen;
        float current_percentage = 0.0;
        float last_percentage = 0.0;
#else
        int step_count = 32;
        int sample_count = step_count;
        float delta = 1.0 / sample_count;   // 如果sample count为10，则delta采样为总共的1/10
#endif

        vec4 reflect_color = vec4(0.0);
        vec3 cam_pos = matrix_ubo.cam_pos.xyz;
        for(int i = 0; i < sample_count; i++)
        {
#if USE_SCREEN_SPACE_MARCH==1
            // 采样当前屏幕上的点对应场景世界空间坐标的位置
            vec2 uv = current_screen.xy / tex_size;

            // 测试直接在ndc进行步进
            // vec4 sample_ndc = vec4(mix(start_ndc, end_ndc, current_percentage), 1.0);
            // vec2 uv = (sample_ndc.xy + 1) / 2;

            // 转换为贴图坐标，检查越界
            if(uv.x < 0.0 || uv.y < 0.0 || uv.x > 1.0 || uv.y > 1.0)
            {

            }
            else
            {
                // 延迟渲染存储的屏幕对应的世界位置
                vec3 sample_world = texture(scene_position, uv).xyz;

                // 屏幕空间坐标找到NDC
                // 推导参考：https://stackoverflow.com/questions/42751427/transformations-from-pixels-to-ndc
//                    vec4 sample_ndc = vec4(1);
//                    sample_ndc.xy = current_screen.xy/tex_size * 2.0 - 1.0;
//                    sample_ndc.z = (current_screen.z * 2.0 - near_far.y - near_far.x)/(near_far.y - near_far.x) - 1.0;
                vec4 sample_ndc = vec4(mix(start_ndc, end_ndc, current_percentage), 1.0);
                if(compareDepth(sample_ndc, uv))
                {
//                        reflect_color = texture(scene_color, uv);
                    // 初筛后再二分法检查？目前精度感觉很够了
                    int split_count = 5;
                    while(split_count > 0)
                    {
                        vec3 mid_screen = (last_screen + current_screen) * 0.5;
                        float mid_percentage = (last_percentage + current_percentage) * 0.5;
                        vec4 mid_ndc = vec4(mix(start_ndc, end_ndc, mid_percentage), 1.0);
                        uv = mid_screen.xy / tex_size;
                        if(compareDepth(mid_ndc, uv))
                        {
                            current_screen = mid_screen;
                        }
                        else
                        {
                            last_screen = mid_screen;
                        }
                        split_count--;
                    }

                    reflect_color = texture(scene_color, uv);
                    break;
                }
                else
                {
//                   reflect_color = vec4(0,1,0,1);
                }

                last_screen = current_screen;
                last_percentage = current_percentage;
                current_screen += delta_screen;
                current_percentage += percentage_delta;
            }

#else
            float sample_t = i*delta;
            // 线性插值找到当前采样的屏幕空间的点
//                vec4 screen_p = mix(start_pos_screen, end_pos_screen, sample_t);
            vec2 uv = vec2(0);

            if(campareDepth(start_clip, end_clip, start_pos_world, end_pos_world, sample_t, uv))
            {
                reflect_color = texture(scene_color, uv);
                int split_count = 10;
                float i_divide_pos = 0.5;
                while(split_count > 0)
                {
                    if(campareDepth(start_clip, end_clip, start_pos_world, end_pos_world, (float(i)-i_divide_pos)*delta, uv))
                    {
                        i_divide_pos += i_divide_pos*0.5;
                    }
                    else
                    {
                        i_divide_pos -= i_divide_pos*0.5;
                    }
                    split_count--;
                }

                reflect_color = texture(scene_color, uv);
                break;
            }
#endif
        }

        reflect_color = reflect_color*metallic;
        FragColor.rgb = FragColor.rgb + reflect_color.rgb*reflect_color.a;
    }
}
