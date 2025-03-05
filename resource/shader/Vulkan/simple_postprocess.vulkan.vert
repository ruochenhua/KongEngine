#version 450
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

layout(location=0) out vec2 fragUV;

void main()
{

    fragUV = uv;
    gl_Position = vec4(position, 1.0);
}