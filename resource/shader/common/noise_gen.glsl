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


///////////////////////////
mat3 m = mat3( 0.00,  0.80,  0.60,
              -0.80,  0.36, -0.48,
              -0.60, -0.48,  0.64);

float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}

///
/// Noise function
///
float noise(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f * f * (3.0 - 2.0 * f);

    float n = p.x + p.y * 57.0 + 113.0 * p.z;

    float res = mix(mix(mix(hash(n +   0.0), hash(n +   1.0), f.x),
                        mix(hash(n +  57.0), hash(n +  58.0), f.x), f.y),
                    mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
                        mix(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
    return res;
}

float PerlinNoise3D(vec3 p, float frequency, int octaves)
{
    float noise_val = 0.0;
    float amp = 0.5;
    float freq = frequency;
    for(int i = 0; i < octaves; ++i)
    {
        noise_val += noise(p*freq) * amp;
        amp*= 0.5;
        freq *= 2.0;
    }
    return noise_val;
}
//////////////////

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