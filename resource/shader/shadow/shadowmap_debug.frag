#version 450 compatibility
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DArray shadow_map;
//uniform float near_plane;
//uniform float far_plane;
//
//// required when using a perspective projection matrix
//float LinearizeDepth(float depth)
//{
//    float z = depth * 2.0 - 1.0; // Back to NDC
//    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
//}

void main()
{
    float depth_value = texture(shadow_map, vec3(TexCoords,2)).r;
    //FragColor = vec4(vec3(LinearizeDepth(depth_value) / far_plane), 1.0); // perspective
    FragColor = vec4(vec3(depth_value), 1.0); // orthographic
}