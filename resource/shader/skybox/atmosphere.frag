#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D depth_map;

uniform int render_sky_status;

uniform float iTime;

uniform sampler3D cloud;
uniform sampler3D worley32;
uniform sampler2D weatherTex;
uniform mat4 inv_view;
uniform mat4 inv_proj;

uniform vec2 iResolution = vec2(1024.0, 768.0);

uniform float coverage_multiplier = 1.0;
uniform float cloudSpeed;
uniform float crispiness = 0.4;

uniform vec3 cloudColorTop = vec3(0.95, 0.88, 0.88);
uniform vec3 cloudColorBottom =  vec3(0.35, 0.37, 0.36);

#define CLOUDS_AMBIENT_COLOR_TOP cloudColorTop
#define CLOUDS_AMBIENT_COLOR_BOTTOM cloudColorBottom

vec3 cameraPosition = matrix_ubo.cam_pos.xyz;

// Cone sampling random offsets
uniform vec3 noiseKernel[6u] = vec3[]
(
vec3( 0.38051305,  0.92453449, -0.02111345),
vec3(-0.50625799, -0.03590792, -0.86163418),
vec3(-0.32509218, -0.94557439,  0.01428793),
vec3( 0.09026238, -0.27376545,  0.95755165),
vec3( 0.28128598,  0.42443639, -0.86065785),
vec3(-0.16852403,  0.14748697,  0.97460106)
);

int iSteps = 32;
int jSteps = 16;


// Cloud types height density gradients
const vec4 STRATUS_GRADIENT = vec4(0.0, 0.1, 0.2, 0.3);
const vec4 STRATOCUMULUS_GRADIENT = vec4(0.02, 0.2, 0.48, 0.625);
const vec4 CUMULUS_GRADIENT = vec4(0.00, 0.1625, 0.88, 0.98);


uniform float earthRadius = 600000.0;
uniform float sphereInnerRadius = 5000.0;
uniform float sphereOuterRadius = 17000.0;

const float SPHERE_INNER_RADIUS = earthRadius + sphereInnerRadius;
const float SPHERE_OUTER_RADIUS = SPHERE_INNER_RADIUS + sphereOuterRadius;
//const float SPHERE_DELTA  = float(SPHERE_OUTER_RADIUS - SPHERE_INNER_RADIUS);


const float CLOUDS_MIN_TRANSMITTANCE = 1e-1;
const float CLOUDS_TRANSMITTANCE_THRESHOLD = 1.0 - CLOUDS_MIN_TRANSMITTANCE;

const vec3 SUN_DIR = -light_info_ubo.directional_light.light_dir.xyz;
const vec3 SUN_COLOR = light_info_ubo.directional_light.light_color.xyz*vec3(1.1,1.1,0.95);
vec3 sphereCenter = vec3(0.0, -earthRadius, 0.0);


float HG( float sundotrd, float g) {
    float gg = g * g;
    return (1. - gg) / pow( 1. + gg - 2. * g * sundotrd, 1.5);
}

bool raySphereintersection(vec3 ro, vec3 rd, float radius, out vec3 startPos)
{
    float t;

    sphereCenter.xz = cameraPosition.xz;

    float radius2 = radius*radius;

    vec3 L = ro - sphereCenter;
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, L);
    float c = dot(L,L) - radius2;

    float discr = b*b - 4.0 * a * c;
    if(discr < 0.0) return false;
    t = max(0.0, (-b + sqrt(discr))/2);
    if(t == 0.0){
        return false;
    }
    startPos = ro + rd*t;

    return true;
}

uniform vec3 skyColorBottom;
uniform vec3 skyColorTop;

vec3 getSun(const vec3 d, float powExp){
    float sun = clamp( dot(SUN_DIR,d), 0.0, 1.0 );
    vec3 col = 0.8*vec3(1.0,.6,0.1)*pow( sun, powExp );
    return col;
}

float Random2D(in vec3 st)
{
    return fract(sin(iTime*dot(st.xyz, vec3(12.9898, 78.233, 57.152))) * 43758.5453123);
}

float getHeightFraction(vec3 inPos){
    return (length(inPos - sphereCenter) - SPHERE_INNER_RADIUS)/(SPHERE_OUTER_RADIUS - SPHERE_INNER_RADIUS);
}


float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
    return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

float getDensityForCloud(float heightFraction, float cloudType)
{
    float stratusFactor = 1.0 - clamp(cloudType * 2.0, 0.0, 1.0);
    float stratoCumulusFactor = 1.0 - abs(cloudType - 0.5) * 2.0;
    float cumulusFactor = clamp(cloudType - 0.5, 0.0, 1.0) * 2.0;

    vec4 baseGradient = stratusFactor * STRATUS_GRADIENT + stratoCumulusFactor * STRATOCUMULUS_GRADIENT + cumulusFactor * CUMULUS_GRADIENT;

    // gradicent computation (see Siggraph 2017 Nubis-Decima talk)
    //return remap(heightFraction, baseGradient.x, baseGradient.y, 0.0, 1.0) * remap(heightFraction, baseGradient.z, baseGradient.w, 1.0, 0.0);
    return smoothstep(baseGradient.x, baseGradient.y, heightFraction) - smoothstep(baseGradient.z, baseGradient.w, heightFraction);

}

float threshold(const float v, const float t)
{
    return v > t ? v : 0.0;
}

const vec3 windDirection = normalize(vec3(0.5, 0.0, 0.1));

vec2 getUVProjection(vec3 p){
     return p.xz/SPHERE_INNER_RADIUS + 0.5;
//    vec3 dir_vector = normalize(p - sphereCenter);
//    return (dir_vector.xz + 1.0)/2.0;
}

#define CLOUD_TOP_OFFSET 750.0
#define SATURATE(x) clamp(x, 0.0, 1.0)
#define CLOUD_SCALE crispiness
#define CLOUD_SPEED cloudSpeed

uniform float curliness;

float sampleCloudDensity(vec3 p, bool expensive, float lod){

    float heightFraction = getHeightFraction(p);
    vec3 animation = heightFraction * windDirection * CLOUD_TOP_OFFSET + windDirection * iTime * CLOUD_SPEED;
    vec2 uv = getUVProjection(p);
    vec2 moving_uv = getUVProjection(p + animation);

    if(heightFraction < 0.0 || heightFraction > 1.0){
        return 0.0;
    }

    vec4 low_frequency_noise = textureLod(cloud, vec3(uv*CLOUD_SCALE, heightFraction), lod);
    float lowFreqFBM = dot(low_frequency_noise.gba, vec3(0.625, 0.25, 0.125));
    float base_cloud = remap(low_frequency_noise.r, -(1.0 - lowFreqFBM), 1., 0.0 , 1.0);

    float density = getDensityForCloud(heightFraction, 1.0);
    base_cloud *= (density/heightFraction);

    vec3 weather_data = texture(weatherTex, moving_uv).rgb;
    float cloud_coverage = weather_data.r*coverage_multiplier;
    float base_cloud_with_coverage = remap(base_cloud , cloud_coverage , 1.0 , 0.0 , 1.0);
    base_cloud_with_coverage *= cloud_coverage;

    //bool expensive = true;

    if(expensive)
    {
        vec3 erodeCloudNoise = textureLod(worley32, vec3(moving_uv*CLOUD_SCALE, heightFraction)*curliness, lod).rgb;
        float highFreqFBM = dot(erodeCloudNoise.rgb, vec3(0.625, 0.25, 0.125));//(erodeCloudNoise.r * 0.625) + (erodeCloudNoise.g * 0.25) + (erodeCloudNoise.b * 0.125);
        float highFreqNoiseModifier = mix(highFreqFBM, 1.0 - highFreqFBM, clamp(heightFraction * 10.0, 0.0, 1.0));

        base_cloud_with_coverage = base_cloud_with_coverage - highFreqNoiseModifier * (1.0 - base_cloud_with_coverage);

        base_cloud_with_coverage = remap(base_cloud_with_coverage*2.0, highFreqNoiseModifier * 0.2, 1.0, 0.0, 1.0);
    }

    return clamp(base_cloud_with_coverage, 0.0, 1.0);
}


float beer(float d){
    return exp(-d);
}

float powder(float d){
    return (1. - exp(-2.*d));
}

// Beer's law inverted equation for edges
float powder(float density, float ca)
{
	float f = 1.0 - exp(-density * 2.0);
	return mix(1.0, f, clamp(-ca * 0.5 + 0.5, 0.0, 1.0));
	//return f;
}

float henyeyGreenstein(vec3 l, vec3 v, float g, float ca)
{
	float g2 = g * g;

	return ((1.0 - g2) / pow((1.0 + g2 - 2.0 * g * ca), 1.5 )) * (1.0 / (4.0 * 3.1415));
}

float phase(vec3 inLightVec, vec3 inViewVec, float g) {
    float costheta = dot(inLightVec, inViewVec) / length(inLightVec) / length(inViewVec);
    return HG(costheta, g);
}

vec3 eye = cameraPosition;

uniform float absorption = 0.0035;

// Full cloud light energy equation
float lightEnergy(vec3 l, vec3 v, float ca, float coneDensity)
{
	return 50.0 * beer(coneDensity) * powder(coneDensity, ca) * henyeyGreenstein(l, v, 0.2, ca);
}

float maxRenderDist = 1000000.0f;

float raymarchToLight(vec3 origin_pos, float stepSize, vec3 view_dir, float originalDensity, float lightDotEye)
{
    vec3 startPos = origin_pos;
    float ds = stepSize * 0.7;
    vec3 lightDir = SUN_DIR;
    vec3 rayStep = lightDir * ds;
    const float CONE_STEP = 1.0/6.0;
    float coneRadius = 1.0;
    float density = 0.0;
    float coneDensity = 0.0;
    float invDepth = 1.0/(stepSize*6);
//    float sigma_ds = -ds*absorption;

    float T = 1.0;

    for(int i = 0; i < 6; i++)
    {
        // Get position inside cone
		vec3 posInCone = startPos + lightDir + coneRadius * noiseKernel[i] * float(i);

		float deltaDist = clamp(length(posInCone - cameraPosition) / maxRenderDist, 0.0, 1.0);

		// By advancing towards the light we might go outside the atmosphere
		float heightFraction = getHeightFraction(posInCone);
		if(heightFraction <= 1.0)
		{
			// sample the expensive way if we hare near borders (where density is low, like 0.3 or below)
			float cloudDensity = sampleCloudDensity(posInCone, coneDensity < 0.3, heightFraction);
			if(cloudDensity > 0.0)
			{
				density += cloudDensity;
				float transmittance = 1.0 - (density * invDepth);
				coneDensity += (cloudDensity * transmittance);
			}
		}

		startPos += rayStep;
		coneRadius += CONE_STEP;

        // terrain method
//        vec3 pos = startPos + coneRadius*noiseKernel[i]*float(i);
//
//        float heightFraction = getHeightFraction(pos);
//        if(heightFraction >= 0)
//        {
//
//            float cloudDensity = sampleCloudDensity(pos, bool(density > 0.3), i/16);
//            if(cloudDensity > 0.0)
//            {
//                float Ti = exp(cloudDensity*sigma_ds);
//                T *= Ti;
//                density += cloudDensity;
//            }
//        }
//        startPos += rayStep;
//        coneRadius += CONE_STEP;
    }

    //return 2.0*T*powder((originalDensity));//*powder(originalDensity, 0.0);

    // 1 far sample for shadowing
	origin_pos += (rayStep * 8.0);
	float heightFraction = getHeightFraction(origin_pos);
	float cloudDensity = sampleCloudDensity(origin_pos, false, heightFraction);
	if(cloudDensity > 0.0)
	{
		density += cloudDensity;
		float transmittance = 1.0 - (density * invDepth);
		coneDensity += (cloudDensity * transmittance);
	}

	float ca = dot(lightDir, view_dir);

	// Compute light energy arriving at point
	return lightEnergy(lightDir, view_dir, ca, coneDensity);

    // terrain method
//    return T;
}

#define BAYER_FACTOR 1.0/16.0
uniform float bayerFilter[16u] = float[]
(
0.0*BAYER_FACTOR, 8.0*BAYER_FACTOR, 2.0*BAYER_FACTOR, 10.0*BAYER_FACTOR,
12.0*BAYER_FACTOR, 4.0*BAYER_FACTOR, 14.0*BAYER_FACTOR, 6.0*BAYER_FACTOR,
3.0*BAYER_FACTOR, 11.0*BAYER_FACTOR, 1.0*BAYER_FACTOR, 9.0*BAYER_FACTOR,
15.0*BAYER_FACTOR, 7.0*BAYER_FACTOR, 13.0*BAYER_FACTOR, 5.0*BAYER_FACTOR
);

uniform bool enablePowder = false;

vec3 ambientLight(vec3 bg)
{
    float light_factor = clamp(dot(vec3(0,1,0), SUN_DIR), 0.0, 1.0);
    vec3 real_light_color = SUN_COLOR;
    real_light_color.y *= light_factor * 0.85;
    real_light_color.z *= light_factor * 0.55;

	return mix(real_light_color, bg, 0.65) * light_factor * 0.65;
}

uniform float densityFactor = 0.02;
vec4 raymarchToCloud(vec3 startPos, vec3 endPos, vec3 bg){
    vec3 path = endPos - startPos;
    float len = length(path);

//    const int nSteps = 64;
    const int nSteps = int(ceil(mix(48.0, 96.0, clamp(len/(SPHERE_OUTER_RADIUS-SPHERE_INNER_RADIUS), 0.0, 1.0))));

    float ds = len / float(nSteps-1);
    vec3 view_dir = path/len;
    vec3 dir = view_dir * ds;

    vec4 col = vec4(0.0);

    vec2 fragCoord = gl_FragCoord.xy;
    int a = int(fragCoord.x) % 4;
    int b = int(fragCoord.y) % 4;

    //startPos += dir*abs(Random2D(vec3(a,b,a+b)))*.5;
    vec3 pos = startPos;
    pos += dir * bayerFilter[a * 4 + b];

    float density = 0.0;

    float lightDotEye = dot(normalize(SUN_DIR), normalize(dir));

    float T = 1.0;
    float sigma_ds = -ds*densityFactor;

    for(int i = 0; i < nSteps; ++i)
    {
        float density_sample = sampleCloudDensity(pos, true, 0);
        if(density_sample > 0.)
        {
            density += density_sample;

            float height = getHeightFraction(pos);
            float light_energy = raymarchToLight(pos, ds, view_dir, density_sample, lightDotEye);
            vec3 ambient_c = mix(CLOUDS_AMBIENT_COLOR_BOTTOM, CLOUDS_AMBIENT_COLOR_TOP, height);
            vec3 ambient_color = ambientLight(ambient_c);

			vec4 src = vec4(light_energy*vec3(0.5) + ambient_color, density_sample); // ACCUMULATE
			src.rgb *= src.a;
			col = (1.0 - col.a) * src + col;

			if(col.a >= 0.99) // EARLY EXIT ON FULL OPACITY
				break;

            // 原terrain方法
//            float scattering = mix(HG(lightDotEye, -0.08), HG(lightDotEye, 0.08), clamp(lightDotEye*0.5 + 0.5, 0.0, 1.0));
//            //scattering = 0.6;
//            scattering = max(scattering, 1.0);
//
//            float powderTerm =  powder(density_sample);
//            if(!enablePowder)
//            {
//                powderTerm = 1.0;
//            }
//
//            vec3 S = 0.6*( mix( mix(ambientLight*1.8, bg, 0.2), scattering*SUN_COLOR, powderTerm* light_energy)) * density_sample;
//            float dTrans = exp(density_sample*sigma_ds);
//            vec3 Sint = (S - S * dTrans) * (1. / density_sample);
//            col.rgb += T * Sint;
//            T *= dTrans;
        }

        if( T <= CLOUDS_MIN_TRANSMITTANCE ) break;

        pos += dir;
    }
    //col.rgb += ambientlight*0.02;
//    col.a = 1.0 - T;
    //col = vec4( vec3(getHeightFraction(startPos)), 1.0);

    return col;
}

float computeFogAmount(in vec3 startPos, in float factor){
    float dist = length(startPos - cameraPosition);
    float radius = (cameraPosition.y - sphereCenter.y) * 0.3;
    float alpha = (dist / radius);
    //v.rgb = mix(v.rgb, ambientColor, alpha*alpha);

    return (1.-exp( -dist*alpha*factor));
}

vec3 computeClipSpaceCoord(vec2 fragCoord){
	vec2 ray_nds = 2.0*vec2(fragCoord.xy)/iResolution.xy - 1.0;
	return vec3(ray_nds, 1.0);
}

vec3 computeSkyboxCloud(vec4 sky_color, vec3 worldDir)
{
    vec3 startPos, endPos;

    vec4 bg = sky_color;
    int case_ = 0;
    //compute raymarching starting and ending point
    vec3 fogRay;
    if(cameraPosition.y < SPHERE_INNER_RADIUS - earthRadius){
        // under the cloud
        raySphereintersection(cameraPosition, worldDir, SPHERE_INNER_RADIUS, startPos);
        raySphereintersection(cameraPosition, worldDir, SPHERE_OUTER_RADIUS, endPos);
        fogRay = startPos;
    }else if(cameraPosition.y > SPHERE_INNER_RADIUS - earthRadius && cameraPosition.y < SPHERE_OUTER_RADIUS - earthRadius){
        // inside the cloud
        startPos = cameraPosition;
        raySphereintersection(cameraPosition, worldDir, SPHERE_OUTER_RADIUS, endPos);
        bool hit = raySphereintersection(cameraPosition, worldDir, SPHERE_INNER_RADIUS, fogRay);
        if(!hit)
        {
            fogRay = startPos;
        }

        case_ = 1;
    }else{
        // on top of the cloud
        raySphereintersection(cameraPosition, worldDir, SPHERE_OUTER_RADIUS, startPos);
        raySphereintersection(cameraPosition, worldDir, SPHERE_INNER_RADIUS, endPos);
        raySphereintersection(cameraPosition, worldDir, SPHERE_OUTER_RADIUS, fogRay);
        case_ = 2;
    }

    //compute fog amount and early exit if over a certain value
    float fogAmount = computeFogAmount(fogRay, 0.0006);


//    if(fogAmount > 0.999965)
//    {
//        fragColor_v = bg;
//        return vec3(1,0,0);
//        return fragColor_v.xyz; //early exit
//    }

    vec4 v = vec4(0.0);
    v = raymarchToCloud(startPos, endPos, bg.rgb);

    v.rgb = v.rgb*1.8 - 0.1; // contrast-illumination tuning

    // apply atmospheric fog to far away clouds
    vec3 ambientColor = bg.rgb;

    // use current position distance to center as action radius
    v.rgb = mix(v.rgb, bg.rgb*v.a, clamp(fogAmount,0.,1.));

    // add sun glare to clouds
    float sun = clamp( dot(SUN_DIR,normalize(endPos - startPos)), 0.0, 1.0 );
    vec3 s = 0.8*vec3(1.0,0.4,0.2)*pow( sun, 256.0 );
    v.rgb += s*v.a;

    // blend clouds and background
    bg.rgb = bg.rgb*(1.0 - v.a) + v.rgb;
    bg.a = 1.0;

//    return vec3(0.2, 0.3, 1.0);
    return bg.xyz;
}

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

void main()
{
    vec4 color = vec4(0,0,0,1);
    vec2 fragCoord = gl_FragCoord.xy;
    float depth_val = texture(depth_map, fragCoord/iResolution).w;
    if(depth_val > 1e-3)
    {
        FragColor = vec4(0);
        return;
    }
    vec4 ray_clip = vec4(computeClipSpaceCoord(fragCoord), 1.0);
	vec4 ray_view = inv_proj * ray_clip;
	ray_view = vec4(ray_view.xy, -1.0, 0.0);
    vec3 worldDir = (inv_view * ray_view).xyz;

    if(light_info_ubo.has_dir_light.x > 0)
    {
        vec3 uSunPos = -light_info_ubo.directional_light.light_dir.xyz;
        color.xyz = atmosphere(
            normalize(worldDir), // normalized ray direction
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
//        color.xyz = normalize(worldDir)*0.5 + 0.5;
        color.xyz = computeSkyboxCloud(color, worldDir);
    }
    FragColor = color;
}
