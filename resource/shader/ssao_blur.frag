#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"
in vec2 TexCoords;
out float FragColor;

uniform sampler2D ssao_texture;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssao_texture, 0));
        float result = 0.0;
        for (int x = -2; x < 2; ++x)
        {
            for (int y = -2; y < 2; ++y)
            {
                vec2 offset = vec2(float(x), float(y)) * texelSize;
                result += texture(ssao_texture, TexCoords + offset).r;
            }
        }
        FragColor = result / (4.0 * 4.0);
}