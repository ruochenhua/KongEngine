#version 450 compatibility
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene_texture;

uniform int size = 5;   // 形状采样的尺寸，数值越大模糊越大，消耗更大
uniform float separation = 1.0; // 采样间隔，数值越大模糊越大，质量降低

// 模糊采样的颜色和原始颜色的mix上下限
float min_threshold = 0.1;
float max_threshold = 0.3;

void main()
{
    vec2 tex_size = vec2(textureSize(scene_texture, 0).xy);
    FragColor = texture(scene_texture, TexCoords);

    // 后续size开放可调
    if(size <= 0) return;
    float mx = 0.0;
    vec4 cmx = FragColor;

    for(int i = -size; i <= size; ++i)
    {
        for(int j = -size; j <= size; ++j)
        {
            // dilate的形状可以多样，如圆形，矩形等等，根据采样点的形状来决定
            // 这里使用圆形的dilate
            if(distance(vec2(i,j), vec2(0)) > size) continue;

            // 采样区域内点的颜色，不要越界出去了
            vec2 sample_coord = TexCoords + vec2(i, j)*separation/tex_size;
            if(sample_coord.x > 1.0 || sample_coord.x < 0.0 || sample_coord.y > 1.0 || sample_coord.y < 0.0)
                    continue;

            vec4 c = texture(scene_texture, sample_coord);

            // 和目标颜色做点乘，得到一个灰度值
            float mxt = dot(c.rgb, vec3(0.3, 0.59, 0.11));

            // 保存区域内灰度值最大的颜色
            if(mxt > mx)
            {
                mx = mxt;
                cmx = c;
            }
        }
    }

    // 最终颜色是原来颜色和区域内灰度值最大的采样颜色的混合，有上下限做限制
    FragColor.rgb = mix(FragColor.rgb, cmx.rgb, smoothstep(min_threshold, max_threshold, mx));
}