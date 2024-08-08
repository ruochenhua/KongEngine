#version 450 compatibility
in vec4 frag_pos;

uniform vec3 light_pos;
uniform float far_plane;

// Ouput data
//layout(location = 0) out float fragmentdepth;


void main(){
	// 深度贴图是像素点距离光源的位置除以远平面的距离
	float light_dist = length(frag_pos.xyz - light_pos);
	light_dist = light_dist / far_plane;	
	
	gl_FragDepth = light_dist;
}