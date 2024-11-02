#version 450 compatibility

layout(location = 0) out vec4 world_pos;
layout(location = 1) out vec4 world_normal;
layout(location = 2) out vec4 world_flux;

in vec4 frag_pos;
in vec3 frag_normal;

uniform vec4 albedo;
uniform float light_intensity;

void main()
{
	world_pos = frag_pos;
    world_normal = vec4(frag_normal, 1);
    world_flux = albedo;// * light_intensity;
}