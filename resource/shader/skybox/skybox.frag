#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
in vec3 uv;
out vec4 FragColor;

uniform samplerCube skybox;

int iSteps = 64; 
int jSteps = 32;

uniform int render_sky_status;
        
// 默认球心在原点
vec2 ray_sphere_intersection(vec3 ray_origin, vec3 ray_direction, float sphere_radius)
{
    // ray-sphere intersection that assumes
    // No intersection when result.x > result.y
    float a = dot(ray_direction, ray_direction);
    float b = 2.0 * dot(ray_direction, ray_origin);
    float c = dot(ray_origin, ray_origin) - (sphere_radius * sphere_radius);
    float d = (b*b) - 4.0*a*c;

    // 返回击中结果，y小于x代表无结果
    if (d < 0.0) return vec2(1e10,-1e10);
    // 击中的话有两个相同或者不同的结果
    return vec2(
        (-b - sqrt(d))/(2.0*a),
        (-b + sqrt(d))/(2.0*a)
    );
}

// 获取大气密度
// 传入位置离海平面的高度，以及散射的相关基准高度
// 大气中任意一点的散射系数的计算，简化拆解为散射在海平面的散射系数，乘以基于海平面高度的该散射的大气密度计算公式
float get_atmos_density(float height_to_sea_level, float scale_height)
{
    return exp(-height_to_sea_level / scale_height);
}

// 单次散射模型（太阳光进入大气，只经过一次散射改变方向）
vec3 atmosphere(vec3 ray_dir, vec3 ray_origin,
                vec3 pSun, float iSun,
                float planet_radius, float rAtmos,
                vec3 kRlh, float kMie, float scale_height_rlh, float scale_height_mie, float g)
{
    // Normalize the sun and view directions.
    pSun = normalize(pSun);
    ray_dir = normalize(ray_dir);

    // 视线和大气层大小的尺寸的射线检测
    // x为大气入射点的距离、y为大气出射点的距离（x==y代表光线和大气球体相切，x>y代表光线不经过大气）
    vec2 atmos_hit = ray_sphere_intersection(ray_origin, ray_dir, rAtmos);
    // 未击中，返回0
    if (atmos_hit.x > atmos_hit.y) return vec3(0,0,0);

    // 视线和星球做射线检测，取得近处的检测结果（远处的那个光被星球本体遮挡）
    vec2 planet_hit = ray_sphere_intersection(ray_origin, ray_dir, planet_radius);
    float light_distance = atmos_hit.y;

    // hit the planet
    if(planet_hit.x < planet_hit.y && planet_hit.x > 0.1)
    {
        light_distance = planet_hit.x;
    }

    // light sample length
    float ds = light_distance / float(iSteps);

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 total_scatter_rlh = vec3(0,0,0);
    vec3 total_scatter_mie = vec3(0,0,0);

    // Initialize optical depth accumulators for the primary ray.
    float total_od_rlh = 0.0;
    float total_od_mie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(ray_dir, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    // 瑞利散射相函数
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    // 米氏散射相函数
    float pMie = (1 - gg) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (4*PI));

    // 光线采样
    for (int i = 0; i < iSteps; i++) {

        // Calculate the primary ray sample position.
        // 以视角位置和方向，
        vec3 iPos = ray_origin + ray_dir * (iTime + ds * 0.5);

        // 观察点和星球表面距离（我们这里先默认星球的中心为原点，所以直接用位置的长度就行）
        // Calculate the height of the sample.


        // Calculate the step size of the secondary ray.
        // 在当前点向太阳的位置做射线检测，以大气的半径为球体。.y是代表大气的出射点，j_steps代表采样数
        float jStepSize = ray_sphere_intersection(iPos, pSun, rAtmos).y / float(jSteps);

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // 在当前点到大气入射的举例上，采样计算
        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) {

            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);

            // Calculate the height of the sample.
            float jHeight = length(jPos) - planet_radius;

            // Accumulate the optical depth.
            jOdRlh += get_atmos_density(jHeight, scale_height_rlh) * jStepSize;
            jOdMie += get_atmos_density(jHeight, scale_height_mie) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }
        
        float surface_height = length(iPos) - planet_radius;
        // 计算这一步的散射的光学深度结果
        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float od_step_rlh = get_atmos_density(surface_height, scale_height_rlh) * ds;
        float od_step_mie = get_atmos_density(surface_height, scale_height_mie) * ds;

        // Accumulate optical depth.
        total_od_rlh += od_step_rlh;
        total_od_mie += od_step_mie;
        // 计算衰减系数，光在经过一定距离后衰减剩下来的比例。
        vec3 attn = exp(-(kMie * (total_od_mie + jOdMie) + kRlh * (total_od_rlh + jOdRlh)));

        // Accumulate scattering.
        total_scatter_rlh += od_step_rlh * attn;
        total_scatter_mie += od_step_mie * attn;

        // Increment the primary ray time.
        iTime += ds;
    }

    // Calculate and return the final color.
    return iSun * (pRlh * kRlh * total_scatter_rlh + pMie * kMie * total_scatter_mie);
}

void main() {

    if (render_sky_status == 1)
    {
        FragColor = textureLod(skybox, uv, 0); //vec4(1,0.5,0.5,1);    
    }
    else if (render_sky_status == 2)
    {
        vec3 color = vec3(0);
        if(light_info_ubo.has_dir_light.x > 0)
        {
            vec3 uSunPos = -light_info_ubo.directional_light.light_dir.xyz;
            color = atmosphere(
                normalize(uv), // normalized ray direction
                vec3(0, 6371000, 0), // ray origin
                uSunPos, // position of the sun
                22.0, // intensity of the sun
                6371e3, // radius of the planet in meters
                6471e3, // radius of the atmosphere in meters
                vec3(5.5e-6, 13.0e-6, 22.4e-6), // Rayleigh scattering coefficient
                21e-6, // Mie scattering coefficient
                8500, // Rayleigh scale height
                1200, // Mie scale height
                0.758                           // Mie preferred scattering direction
            );
        }
        FragColor = vec4(color, 1.0);
    }
    else
    {
        FragColor = vec4(0.1, 0.1, 0.1, 1.0);
    }
}