#ifndef _PERLIN_NOISE_GLSL_
#define _PERLIN_NOISE_GLSL_
vec2 seed = vec2(1,1);

float Random2D(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(31.091145, 87.289) + seed)) * 28524.238524);
}

float InterpolatedNoise(float x, float y)
{
    int integer_x = int(floor(x));
    float fract_x = fract(x);
    int integer_y = int(floor(y));
    float fract_y = fract(y);
    
    vec2 random_input = vec2(integer_x, integer_y);
    float a = Random2D(random_input);
    float b = Random2D(random_input+vec2(1,0));
    float c = Random2D(random_input+vec2(0,1));
    float d = Random2D(random_input+vec2(1,1));
    
    vec2 w = vec2(fract_x, fract_y);
    w=w*w*w*(10.0 + w*(-15 + 6*w));

    float k0 = a, k1 = b-a, k2 = c-a, k3 = d-c-b+a;

    return k0+k1*w.x + k2*w.y + k3*w.x*w.y;
}

float Perlin(float x, float y, float amplitude, int octaves, float freq, float power)
{
    float persistence = 0.5;
    float total = 0, frequency = freq;

    for(int i = 0; i < octaves; ++i)
    {
        frequency *= 2.0f;
        amplitude *= persistence;

        total += InterpolatedNoise(x*frequency, y*frequency) * amplitude;
    }

    return pow(total, power);
}

#endif  // _COMMON_GLSL_