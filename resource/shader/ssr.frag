#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D scene_position;
uniform sampler2D scene_normal;
uniform sampler2D scene_color;
uniform sampler2D orm_texture;
vec2 tex_size = 1.0 / vec2(1024, 768);
float bias = 0.01f;
float thickness = 0.1;
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
    if (p_depth > sample_depth - bias  && p_depth < sample_depth + bias + thickness)
    {
        return true;
    }

    return false;
}

void main()
{
    vec4 orm = texture(orm_texture, TexCoords);
    float roughness = orm.y;
    float metallic = orm.z;
    vec4 s_color = texture(scene_color, TexCoords);

    FragColor = s_color;

    // 先只在下面这种情况下的pixel上去做反射
    if(roughness < 0.2 && metallic > 0.8)
    {
        vec4 normal_depth = texture(scene_normal, TexCoords);
        vec3 world_normal = normalize(normal_depth.xyz);

        vec3 world_pos = texture(scene_position, TexCoords).xyz;
        vec3 cam_pos = matrix_ubo.cam_pos.xyz;

        mat4 projection = matrix_ubo.projection;
        mat4 view = matrix_ubo.view;
        mat4 world2screen = projection * view;

        vec3 view_dir = normalize(world_pos-cam_pos);
        vec3 rd = normalize(reflect(view_dir, world_normal));
       // rd = world_normal;
        bool found_intersect = false;

        // 往相机方向反射的暂时先不管
//        if(dot(view_dir, rd) > 0.0)
        {
            int step_count = 64;
            float max_step_dist = 10.0;
            // 在屏幕空间进行光线步进
            // 起始点和结束点
            vec3 start_pos_world = world_pos  + rd*0.1;
            vec3 end_pos_world = world_pos + max_step_dist*rd;
            // 在屏幕空间上的从起始点到结束点的坐标[0, resolution]
            vec4 start_pos_screen = world2screen * vec4(start_pos_world, 1.0);
            vec4 end_pos_screen = world2screen * vec4(end_pos_world, 1.0);
            vec2 screen_diff = abs(end_pos_screen.xy - start_pos_screen.xy);
            // 用长边作为主要的边来划分
            float sample_count = ceil(max(screen_diff.x, screen_diff.y)); // 大于1
            //sample_count *= 3.0;
            //sample_count = min(step_count, sample_count*5.0);
            sample_count = step_count;

            float delta = 1.0 / sample_count;   // 如果sample count为10，则delta采样为总共的1/10
            vec4 reflect_color = vec4(0.0);
            for(int i = 0; i < sample_count; i++)
            {
                float sample_t = i*delta;
                // 线性插值找到当前采样的屏幕空间的点
                vec4 screen_p = mix(start_pos_screen, end_pos_screen, sample_t);
                vec2 uv = vec2(0);
                if(campareDepth(start_pos_screen, end_pos_screen, start_pos_world, end_pos_world, sample_t, uv))
                {
                    reflect_color = texture(scene_color, uv);
                    int split_count = 4;
                    float i_divide_pos = 0.5;
                    while(split_count > 0)
                    {
                        if(campareDepth(start_pos_screen, end_pos_screen, start_pos_world, end_pos_world, (float(i)-i_divide_pos)*delta, uv))
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
            }

            FragColor += reflect_color;
        }
//        if(!found_intersect)
//        {
//            // 红
//            FragColor = vec4(1.0, 0.2, 0.3, 1.0);
//        }
//        else
//        {
//            // 蓝
//           FragColor = vec4(0.2, 0.4, 1.0, 1.0);
//        }

        // float camera_dist = distance(world_pos, cam_pos);
        // WTF? 世界位置到相机的距离要比存的depth要大
        //FragColor = vec4(vec3(camera_dist - normal_depth.w), 1.0);
    }
}