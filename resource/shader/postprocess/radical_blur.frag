#version 450 compatibility
out vec4 FragColor;

in vec2 TexCoords;

uniform float blur_amount;

// 用于采样屏幕纹理的 sampler2D，这里假设屏幕图像已经提前渲染好并绑定到这个纹理采样器上
uniform sampler2D bright_texture;

// 光源在屏幕空间的位置（归一化坐标，范围 0 到 1）
uniform vec2 lightPos = vec2(0.5, 0.5);

// 光线散射强度，用于控制 God Ray 效果的明显程度
uniform float scatteringStrength = 0.1;

// 光线最大传播距离，限制光线投射的范围
uniform int maxRayDistance = 200;

// 计算光线方向向量
vec2 calculateRayDirection(vec2 pixelPos) {
    return normalize(lightPos - pixelPos);
}

// 计算光散射强度（简单的线性衰减示例，可替换为更复杂的物理模型）
float calculateScattering(float distance) {
    return scatteringStrength * max(0.0, 1.0 - distance / maxRayDistance);
}

void main()
{
    // 计算纹理单个纹素（texel）的大小，用于后续采样偏移计算
    vec2 texelSize = 1.0 / vec2(textureSize(bright_texture, 0));
    int rayStepSize = 2;
 // 获取当前像素的颜色
    vec4 pixelColor = texture(bright_texture, TexCoords);

    // 计算从当前像素指向光源的光线方向
    vec2 rayDirection = calculateRayDirection(TexCoords);

    // 用于累积光线散射贡献的颜色
    vec4 accumulatedColor = vec4(0.0);
    // 当前光线传播的距离
    float currentDistance = 0.0;

    // 沿着光线方向进行采样
    while (currentDistance < maxRayDistance) {
        // 计算当前采样点的纹理坐标
        vec2 sampleCoords = TexCoords + rayDirection * currentDistance * texelSize;

        // 获取采样点的颜色
        vec4 sampleColor = texture(bright_texture, sampleCoords);

        // 计算当前采样点的光散射强度
        float scattering = calculateScattering(currentDistance);

        // 将采样点颜色根据散射强度累加到累积颜色中
        accumulatedColor += sampleColor * scattering;

        // 更新光线传播距离
        currentDistance += rayStepSize;
    }

    // 将原始像素颜色和累积的光线散射颜色混合
    FragColor = accumulatedColor / maxRayDistance * 10.0;
}