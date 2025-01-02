#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
#include "/common/noise_gen.glsl"
// tessellation primitive generator没有shader代码， 它根据tcs的输出数据和tes的输入数据结构设定计算
// 如下方(quads,fractional_odd_spacing, ccw)代表了细分primitive的设置
layout(quads, fractional_odd_spacing, ccw) in;

uniform sampler2D height_map;
//uniform int octaves;
//uniform float amplitude;
//uniform float freq;
//uniform float power;
uniform float height_scale;
uniform float height_shift;
uniform vec3 cam_pos;
uniform double iTime;

uniform int terrain_size;
uniform int terrain_res;

const int wave_num = 8;

// patch的四个uv数据
in vec2 TextureCoord[];
//out float tes_height;
out vec2 frag_texcoord;
out vec3 frag_pos;
out vec3 frag_normal;
out vec4 clip_space;

vec3 GerstnerWave(vec3 p, vec4 wave_variables, float speed_factor, inout vec3 binormal, inout vec3 tangent)
{
    vec2 wave_direction = normalize(wave_variables.xy); //波形方向

    float steepness = wave_variables.z;    // 0~1
    float wave_length = wave_variables.w;

    float k = 2.0 * PI / wave_length;
    //    float wave_speed = 0.5;
    float wave_speed = sqrt(9.8 / k) * speed_factor;
    float _amplitude = steepness / k;


    float f = k * (dot(wave_direction, p.xz) - wave_speed * float(iTime));

//    p.y = _amplitude * sin(f);
//    p.x += _amplitude*cos(f) * wave_direction.x;
//    p.z += _amplitude*cos(f) * wave_direction.y;

//    vec3 tangent = (vec3(1-k*_amplitude*sin(f),
//                             k*_amplitude*cos(f),
//                             0));
    tangent += vec3(-wave_direction.x*wave_direction.x * steepness * sin(f),
                        wave_direction.x*steepness*cos(f),
                        -wave_direction.y*wave_direction.x*steepness*sin(f));

    binormal += vec3(-wave_direction.x*wave_direction.y * steepness * sin(f),
                        wave_direction.y*steepness*cos(f),
                        -wave_direction.y*wave_direction.y*steepness*sin(f));

//    p_normal = normalize(cross(binormal, tangent));

//    p_normal = vec3(-tangent.y, tangent.x, 0);

    return vec3(wave_direction.x * _amplitude * cos(f),
                _amplitude*sin(f),
                wave_direction.y * _amplitude * cos(f));
}

void main(){
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = TextureCoord[0];
    vec2 t10 = TextureCoord[1];
    vec2 t01 = TextureCoord[2];
    vec2 t11 = TextureCoord[3];
    vec2 t_range = t11 - t00;

    // 根据细分的uv，lerp得到tesscoord对应的贴图上的uv坐标
    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    // 获取patch的四个顶点
    vec3 p00 = gl_in[0].gl_Position.xyz;
    vec3 p10 = gl_in[1].gl_Position.xyz;
    vec3 p01 = gl_in[2].gl_Position.xyz;
    vec3 p11 = gl_in[3].gl_Position.xyz ;
    vec3 p_range = p11-p00;

    // 得出法线
    vec3 uVec = p01 - p00;
    vec3 vVec = p10 - p00;
    vec3 normal = normalize( cross(vVec.xyz, uVec.xyz));

    // lerp得到细分的p点，并根据高度图和法线得到实际的高度（原P点的y为0）
    vec3 p0 = (p01 - p00) * u + p00;    // 当前点和patch上边界相交的点
    vec3 p1 = (p11 - p10) * u + p10;    // 当前点和patch下边界相交的点
    vec3 p;

    // 通过noise生成造型
    // Gersnter Wave生成函数！！
    p = (p1 - p0) * v + p0;
    vec3 grid_point = p;
    vec3 tangent = vec3(1, 0, 0);
    vec3 binormal = vec3(0, 0, 1);

    vec4 wave_list[wave_num] = vec4[wave_num](
            // direction.x, direction.y, steepness, wave_length
        vec4(0.5, 0.5, 0.05, 201),
        vec4(6.0, 2.0, 0.10, 153),
        vec4(1.0, 5.0, 0.13, 98),
        vec4(0.5, 1.5, 0.22, 73),
        vec4(3.5, 1.5, 0.23, 52),
        vec4(2.5, 5.0, 0.32, 37),
        vec4(4.5, 2.4, 0.37, 13),
        vec4(0.75, 1.2, 0.08, 3.3)
    );
    float total_steepness = 0.0;
    for(int i = 0; i < wave_num; i++)
    {
        total_steepness += wave_list[i].z;
    }
    for(int i = 0; i < wave_num; i++)
    {
       wave_list[i].z /= total_steepness;
    }

    for(int i = 0; i < wave_num; i++)
    {
        // wave_varibles依次存储着方向、steepness和波长wave_length
        float speed_factor = 1.0;
        p += GerstnerWave(grid_point, wave_list[i], speed_factor, binormal, tangent);
    }
    p.y = p.y * height_scale + height_shift;

    normal = normalize(cross(binormal, tangent));
    // 找到附近的两个点，计算高度并计算出normal
//    vec3 pu = p + vec3(1, 0, 0);
//    vec3 pv = p + vec3(0, 0, 1);
//
//    pu = GernsterWave(pu.xz);
//    pv = GernsterWave(pv.xz);

//    tes_height = p.y;

//    uVec = normalize(pu-p);
//    vVec = normalize(pv-p);
//    normal = normalize(cross(vVec, uVec));

    gl_Position = matrix_ubo.projection * matrix_ubo.view * vec4(p, 1.0);

    vec2 cam_uv_offset = cam_pos.xz / float(terrain_size);
    //frag_texcoord = texCoord;
    frag_texcoord = p.xz / float(terrain_size) * terrain_res;
    //frag_texcoord = gl_TessCoord.xy;

    frag_pos = p;
    clip_space = gl_Position;
    frag_normal = normal;
}