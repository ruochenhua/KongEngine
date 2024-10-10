#ifndef _NOISE_GEN_GLSL_
#define _NOISE_GEN_GLSL_
vec2 seed = vec2(1,1);

float Random2D(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(31.091145, 87.289) + seed)) * 28524.238524);
}

float Hash(int n)
{
    return fract(sin(float(n) + 1.951) * 43758.434536);
}

float Noise(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(vec3(3.0) - vec3(2.0)*f);
    float a = 57.0;
    float b = 129.0;
    float c = a + b;
    float n = p.x + p.y*a + p.z*b;
    return mix(
            mix(
                mix(Hash(int(n)), Hash(int(n+1)), f.x),
                mix(Hash(int(n+a)), Hash(int(n+a+1)), f.x),
                f.y),
            mix(
                mix(Hash(int(n+b)), Hash(int(n+b+1)), f.x),
                mix(Hash(int(n+c)), Hash(int(n+c+1)), f.x),
                f.y),
        f.z
    );
}

float Cells(vec3 p, float cell_count)
{
    vec3 p_cell = p * cell_count;
    float d = 1.0e10;
    for(int xo = -1; xo < 2; ++xo)
    {
        for(int yo = -1; yo < 2; ++yo)
        {
            for(int zo = -1; zo < 2; ++zo)
            {
                vec3 tp = floor(p_cell) + vec3(xo, yo, zo);
                tp = p_cell - tp - Noise(mod(tp, cell_count));
                d = min(d, dot(tp, tp));
            }
        }
    }

    d = clamp(d, 0.0, 1.0);
    return d;
}

float WorleyNoise3D(vec3 p, float cell_count)
{
    return Cells(p, cell_count);
}


// From GLM (gtc/noise.hpp & detail/_noise.hpp)
vec4 Mod289(vec4 x)
{
	return x - floor(x * vec4(1.0) / vec4(289.0)) * vec4(289.0);
}

vec4 Permute(vec4 x)
{
	return Mod289(((x * 34.0) + 1.0) * x);
}

vec4 TaylorInvSqrt(vec4 r)
{
	return vec4(1.79284291400159) - vec4(0.85373472095314) * r;
}

vec4 Fade(vec4 t)
{
	return (t * t * t) * (t * (t * vec4(6) - vec4(15)) + vec4(10));
}

float GlmPerlin4D(vec4 Position, vec4 rep)
{
		vec4 Pi0 = mod(floor(Position), rep);	// Integer part for indexing
		vec4 Pi1 = mod(Pi0 + vec4(1), rep);		// Integer part + 1
		//Pi0 = mod(Pi0, vec4(289));
		//Pi1 = mod(Pi1, vec4(289));
		vec4 Pf0 = fract(Position);	// Fractional part for interpolation
		vec4 Pf1 = Pf0 - vec4(1);		// Fractional part - 1.0
		vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
		vec4 iy = vec4(Pi0.y, Pi0.y, Pi1.y, Pi1.y);
		vec4 iz0 = vec4(Pi0.z);
		vec4 iz1 = vec4(Pi1.z);
		vec4 iw0 = vec4(Pi0.w);
		vec4 iw1 = vec4(Pi1.w);

		vec4 ixy = Permute(Permute(ix) + iy);
		vec4 ixy0 = Permute(ixy + iz0);
		vec4 ixy1 = Permute(ixy + iz1);
		vec4 ixy00 = Permute(ixy0 + iw0);
		vec4 ixy01 = Permute(ixy0 + iw1);
		vec4 ixy10 = Permute(ixy1 + iw0);
		vec4 ixy11 = Permute(ixy1 + iw1);

		vec4 gx00 = ixy00 / vec4(7);
		vec4 gy00 = floor(gx00) / vec4(7);
		vec4 gz00 = floor(gy00) / vec4(6);
		gx00 = fract(gx00) - vec4(0.5);
		gy00 = fract(gy00) - vec4(0.5);
		gz00 = fract(gz00) - vec4(0.5);
		vec4 gw00 = vec4(0.75) - abs(gx00) - abs(gy00) - abs(gz00);
		vec4 sw00 = step(gw00, vec4(0.0));
		gx00 -= sw00 * (step(vec4(0), gx00) - vec4(0.5));
		gy00 -= sw00 * (step(vec4(0), gy00) - vec4(0.5));

		vec4 gx01 = ixy01 / vec4(7);
		vec4 gy01 = floor(gx01) / vec4(7);
		vec4 gz01 = floor(gy01) / vec4(6);
		gx01 = fract(gx01) - vec4(0.5);
		gy01 = fract(gy01) - vec4(0.5);
		gz01 = fract(gz01) - vec4(0.5);
		vec4 gw01 = vec4(0.75) - abs(gx01) - abs(gy01) - abs(gz01);
		vec4 sw01 = step(gw01, vec4(0.0));
		gx01 -= sw01 * (step(vec4(0), gx01) - vec4(0.5));
		gy01 -= sw01 * (step(vec4(0), gy01) - vec4(0.5));

		vec4 gx10 = ixy10 / vec4(7);
		vec4 gy10 = floor(gx10) / vec4(7);
		vec4 gz10 = floor(gy10) / vec4(6);
		gx10 = fract(gx10) - vec4(0.5);
		gy10 = fract(gy10) - vec4(0.5);
		gz10 = fract(gz10) - vec4(0.5);
		vec4 gw10 = vec4(0.75) - abs(gx10) - abs(gy10) - abs(gz10);
		vec4 sw10 = step(gw10, vec4(0));
		gx10 -= sw10 * (step(vec4(0), gx10) - vec4(0.5));
		gy10 -= sw10 * (step(vec4(0), gy10) - vec4(0.5));

		vec4 gx11 = ixy11 / vec4(7);
		vec4 gy11 = floor(gx11) / vec4(7);
		vec4 gz11 = floor(gy11) / vec4(6);
		gx11 = fract(gx11) - vec4(0.5);
		gy11 = fract(gy11) - vec4(0.5);
		gz11 = fract(gz11) - vec4(0.5);
		vec4 gw11 = vec4(0.75) - abs(gx11) - abs(gy11) - abs(gz11);
		vec4 sw11 = step(gw11, vec4(0.0));
		gx11 -= sw11 * (step(vec4(0), gx11) - vec4(0.5));
		gy11 -= sw11 * (step(vec4(0), gy11) - vec4(0.5));

		vec4 g0000 = vec4(gx00.x, gy00.x, gz00.x, gw00.x);
		vec4 g1000 = vec4(gx00.y, gy00.y, gz00.y, gw00.y);
		vec4 g0100 = vec4(gx00.z, gy00.z, gz00.z, gw00.z);
		vec4 g1100 = vec4(gx00.w, gy00.w, gz00.w, gw00.w);
		vec4 g0010 = vec4(gx10.x, gy10.x, gz10.x, gw10.x);
		vec4 g1010 = vec4(gx10.y, gy10.y, gz10.y, gw10.y);
		vec4 g0110 = vec4(gx10.z, gy10.z, gz10.z, gw10.z);
		vec4 g1110 = vec4(gx10.w, gy10.w, gz10.w, gw10.w);
		vec4 g0001 = vec4(gx01.x, gy01.x, gz01.x, gw01.x);
		vec4 g1001 = vec4(gx01.y, gy01.y, gz01.y, gw01.y);
		vec4 g0101 = vec4(gx01.z, gy01.z, gz01.z, gw01.z);
		vec4 g1101 = vec4(gx01.w, gy01.w, gz01.w, gw01.w);
		vec4 g0011 = vec4(gx11.x, gy11.x, gz11.x, gw11.x);
		vec4 g1011 = vec4(gx11.y, gy11.y, gz11.y, gw11.y);
		vec4 g0111 = vec4(gx11.z, gy11.z, gz11.z, gw11.z);
		vec4 g1111 = vec4(gx11.w, gy11.w, gz11.w, gw11.w);

		vec4 norm00 = TaylorInvSqrt(vec4(dot(g0000, g0000), dot(g0100, g0100), dot(g1000, g1000), dot(g1100, g1100)));
		g0000 *= norm00.x;
		g0100 *= norm00.y;
		g1000 *= norm00.z;
		g1100 *= norm00.w;

		vec4 norm01 = TaylorInvSqrt(vec4(dot(g0001, g0001), dot(g0101, g0101), dot(g1001, g1001), dot(g1101, g1101)));
		g0001 *= norm01.x;
		g0101 *= norm01.y;
		g1001 *= norm01.z;
		g1101 *= norm01.w;

		vec4 norm10 = TaylorInvSqrt(vec4(dot(g0010, g0010), dot(g0110, g0110), dot(g1010, g1010), dot(g1110, g1110)));
		g0010 *= norm10.x;
		g0110 *= norm10.y;
		g1010 *= norm10.z;
		g1110 *= norm10.w;

		vec4 norm11 = TaylorInvSqrt(vec4(dot(g0011, g0011), dot(g0111, g0111), dot(g1011, g1011), dot(g1111, g1111)));
		g0011 *= norm11.x;
		g0111 *= norm11.y;
		g1011 *= norm11.z;
		g1111 *= norm11.w;

		float n0000 = dot(g0000, Pf0);
		float n1000 = dot(g1000, vec4(Pf1.x, Pf0.y, Pf0.z, Pf0.w));
		float n0100 = dot(g0100, vec4(Pf0.x, Pf1.y, Pf0.z, Pf0.w));
		float n1100 = dot(g1100, vec4(Pf1.x, Pf1.y, Pf0.z, Pf0.w));
		float n0010 = dot(g0010, vec4(Pf0.x, Pf0.y, Pf1.z, Pf0.w));
		float n1010 = dot(g1010, vec4(Pf1.x, Pf0.y, Pf1.z, Pf0.w));
		float n0110 = dot(g0110, vec4(Pf0.x, Pf1.y, Pf1.z, Pf0.w));
		float n1110 = dot(g1110, vec4(Pf1.x, Pf1.y, Pf1.z, Pf0.w));
		float n0001 = dot(g0001, vec4(Pf0.x, Pf0.y, Pf0.z, Pf1.w));
		float n1001 = dot(g1001, vec4(Pf1.x, Pf0.y, Pf0.z, Pf1.w));
		float n0101 = dot(g0101, vec4(Pf0.x, Pf1.y, Pf0.z, Pf1.w));
		float n1101 = dot(g1101, vec4(Pf1.x, Pf1.y, Pf0.z, Pf1.w));
		float n0011 = dot(g0011, vec4(Pf0.x, Pf0.y, Pf1.z, Pf1.w));
		float n1011 = dot(g1011, vec4(Pf1.x, Pf0.y, Pf1.z, Pf1.w));
		float n0111 = dot(g0111, vec4(Pf0.x, Pf1.y, Pf1.z, Pf1.w));
		float n1111 = dot(g1111, Pf1);

		vec4 fade_xyzw = Fade(Pf0);
		vec4 n_0w = mix(vec4(n0000, n1000, n0100, n1100), vec4(n0001, n1001, n0101, n1101), fade_xyzw.w);
		vec4 n_1w = mix(vec4(n0010, n1010, n0110, n1110), vec4(n0011, n1011, n0111, n1111), fade_xyzw.w);
		vec4 n_zw = mix(n_0w, n_1w, fade_xyzw.z);
		vec2 n_yzw = mix(vec2(n_zw.x, n_zw.y), vec2(n_zw.z, n_zw.w), fade_xyzw.y);
		float n_xyzw = mix(n_yzw.x, n_yzw.y, fade_xyzw.x);
		return float(2.2) * n_xyzw;
}

float PerlinNoise3D(vec3 pIn, float frequency, int octaveCount)
{
	float octave_frenquency_factor = 2.0;			// noise frequency factor between octave, forced to 2

	// Compute the sum for each octave
	float sum = 0.0f;
	float weight_sum = 0.0f;
	float weight = 0.5f;
	for (int oct = 0; oct < octaveCount; oct++)
	{
		// Perlin vec3 is bugged in GLM on the Z axis :(, black stripes are visible
		// So instead we use 4d Perlin and only use xyz...
		//glm::vec3 p(x * freq, y * freq, z * freq);
		//float val = glm::perlin(p, glm::vec3(freq)) *0.5 + 0.5;

		vec4 p = vec4(pIn.x, pIn.y, pIn.z, 0.0) * vec4(frequency);
		float val = GlmPerlin4D(p, vec4(frequency));

		sum += val * weight;
		weight_sum += weight;

		weight *= weight;
		frequency *= octave_frenquency_factor;
	}

	float noise = (sum / weight_sum);// *0.5 + 0.5;;
	noise = min(noise, 1.0f);
	noise = max(noise, 0.0f);
	return noise;
}


float InterpolatedNoise(vec2 uv)
{
    vec2 integer = floor(uv);
    vec2 fract_part = fract(uv);

    vec2 random_input = integer;
    float a = Random2D(random_input);
    float b = Random2D(random_input+vec2(1,0));
    float c = Random2D(random_input+vec2(0,1));
    float d = Random2D(random_input+vec2(1,1));
    
    vec2 w = fract_part;
    w=w*w*w*(10.0 + w*(-15 + 6*w));

    float k0 = a, k1 = b-a, k2 = c-a, k3 = d-c-b+a;

    return k0+k1*w.x + k2*w.y + k3*w.x*w.y;
}


const mat2 m2 = mat2(  0.80,  0.60,
                      -0.60,  0.80 );

float Perlin(float x, float y, float amplitude, int octaves, float freq, float power)
{
    float persistence = 0.5;
    float total = 0, frequency = freq;
	vec2 uv = vec2(x, y);
    for(int i = 0; i < octaves; ++i)
    {
        frequency *= 2.0f;
        amplitude *= persistence;

        total += InterpolatedNoise(uv*frequency) * amplitude;

		uv *= m2;
    }

    return pow(total, power);
}



#endif  // _NOISE_GEN_GLSL_