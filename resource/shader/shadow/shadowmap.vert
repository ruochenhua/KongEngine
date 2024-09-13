#version 450 compatibility

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 in_pos;

// Values that stay constant for the whole mesh.
uniform mat4 light_space_mat[16];
uniform int cascade_shadow_count;
uniform mat4 model;

void main(){
	gl_Position =  light_space_mat[0] * model * vec4(in_pos, 1);
}

