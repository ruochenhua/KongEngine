#version 450 compatibility
out vec4 FragColor;

in vec2 TexCoords;
uniform float exposure;

uniform sampler2D scene_texture;
uniform sampler2D bright_texture;

uniform bool bloom;
const float offset = 1.0 / 300.0;
void main()
{
    vec3 scene_value = texture(scene_texture, TexCoords).rgb;
    //FragColor = vec4(scene_value, 1.0);
    // Reinhard色调映射
    vec3 hdr_color = scene_value;
//    vec3 hdr_color = mix(scene_value,  reflection_value.rgb, reflection_value.a);
    if(bloom)
    {
        vec3 bloom_value = texture(bright_texture, TexCoords).rgb;
        hdr_color += bloom_value;
    }

//    vec3 mapped = hdr_color / (hdr_color + vec3(1.0));
//    // exposure
//    float exposure = 1.0;
    vec3 mapped = vec3(1.0) - exp(-hdr_color * exposure);
//    // gamma校正
    const float gamma = 2.2;
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1.0);

    // FragColor = vec4(scene_value, 1.0);
    // 黑白效果
//    vec3 scene_value = texture(scene_texture, TexCoords).rgb;
//    float average = scene_value.x + scene_value.y + scene_value.z;
//    average /= 3.0;
//    FragColor = vec4(vec3(average), 1.0);

    // 核效果
//    vec2 offsets[9] = vec2[](
//        vec2(-offset,  offset), // 左上
//        vec2( 0.0f,    offset), // 正上
//        vec2( offset,  offset), // 右上
//        vec2(-offset,  0.0f),   // 左
//        vec2( 0.0f,    0.0f),   // 中
//        vec2( offset,  0.0f),   // 右
//        vec2(-offset, -offset), // 左下
//        vec2( 0.0f,   -offset), // 正下
//        vec2( offset, -offset)  // 右下
//    );
//
//    float kernel[9] = float[](
//        -1, -1, -1,
//        -1,  9, -1,
//        -1, -1, -1
//    );
//
//    vec3 sampleTex[9];
//    for(int i = 0; i < 9; i++)
//    {
//        sampleTex[i] = vec3(texture(scene_texture, TexCoords.st + offsets[i]));
//    }
//    vec3 col = vec3(0.0);
//    for(int i = 0; i < 9; i++)
//    col += sampleTex[i] * kernel[i];
//
//    FragColor = vec4(col, 1.0);
//    
}