#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
#include "/common/perlin_noise.glsl"
// tessellation primitive generator没有shader代码， 它根据tcs的输出数据和tes的输入数据结构设定计算
// 如下方(quads,fractional_odd_spacing, ccw)代表了细分primitive的设置
layout(quads, fractional_odd_spacing, ccw) in;

uniform sampler2D height_map;
uniform int octaves;
uniform float amplitude;
uniform float freq;
uniform float power;
uniform float height_scale;
uniform float height_shift;

// patch的四个uv数据
in vec2 TextureCoord[];
out float tes_height;
out vec2 frag_texcoord;
out vec3 frag_pos;
out vec3 frag_normal;
out mat3 TBN;

void main(){
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = TextureCoord[0];
    vec2 t01 = TextureCoord[1];
    vec2 t10 = TextureCoord[2];
    vec2 t11 = TextureCoord[3];
    vec2 t_range = t11 - t00;

    // 根据细分的uv，lerp得到tesscoord对应的贴图上的uv坐标
    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    // 获取patch的四个顶点
    vec3 p00 = gl_in[0].gl_Position.xyz;
    vec3 p01 = gl_in[1].gl_Position.xyz;
    vec3 p10 = gl_in[2].gl_Position.xyz;
    vec3 p11 = gl_in[3].gl_Position.xyz;
    vec3 p_range = p11-p00;

    // 得出法线
    vec3 uVec = p01 - p00;
    vec3 vVec = p10 - p00;
    vec3 normal = normalize( cross(vVec.xyz, uVec.xyz));
    
    // lerp得到细分的p点，并根据高度图和法线得到实际的高度（原P点的y为0）
    vec3 p0 = (p01 - p00) * u + p00;    // 当前点和patch上边界相交的点
    vec3 p1 = (p11 - p10) * u + p10;    // 当前点和patch下边界相交的点
    vec3 p;

    // 有高度图就读取高度图
    if(textureSize(height_map, 0).x > 2)
    {
        int resolution = 10;
        tes_height = texture(height_map, texCoord).y * height_scale - height_shift;
        p = (p1 - p0) * v + p0 + normal * tes_height;

        // 取两个点    
        vec3 pu = p + vec3(p_range.x / resolution, 0, 0);
        vec3 pv = p + vec3(0, 0, p_range.z / resolution);     
        // 取这两个点的高度
        pu.y = texture(height_map, texCoord+vec2(t_range.x/resolution, 0)).y * height_scale - height_shift;        
        pv.y = texture(height_map, texCoord+vec2(0, t_range.y/resolution)).y * height_scale - height_shift;        
         
        uVec = normalize(pu-p);                
        vVec = normalize(pv-p);
        normal = normalize(cross(vVec, uVec));

        TBN = mat3(uVec, vVec, normal);
    }
    else
    // 没有高度图就通过柏林噪音生成
    {
        p = (p1 - p0) * v + p0;
        p.y = Perlin(p.x, p.z, amplitude, octaves, freq, power);
        
        // 找到附近的两个点，计算高度并计算出normal
        vec3 pu = p + vec3(1, 0, 0);
        vec3 pv = p + vec3(0, 0, 1);

        pu.y = Perlin(pu.x, pu.z, amplitude, octaves, freq, power);       
        pv.y = Perlin(pv.x, pv.z, amplitude, octaves, freq, power);
        
        tes_height = p.y;
                 
        uVec = normalize(pu-p);                
        vVec = normalize(pv-p);
        normal = normalize(cross(vVec, uVec));

        TBN = mat3(uVec, vVec, normal);
    }
    
    gl_Position = matrix_ubo.projection * matrix_ubo.view * vec4(p, 1.0);

    //frag_texcoord = texCoord;
    frag_texcoord = gl_TessCoord.xy;
    frag_pos = p;
    
    frag_normal = normal;
}