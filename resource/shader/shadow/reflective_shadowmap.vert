#version 450 compatibility

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;

// Values that stay constant for the whole mesh.
uniform mat4 light_space_mat;
uniform mat4 model;

out vec4 frag_pos;
out vec3 frag_normal;

void main(){
	gl_Position =  light_space_mat * model * vec4(in_pos, 1);
    frag_pos = model * vec4(in_pos, 1);
	frag_normal = normalize(mat3(transpose(inverse(model))) * in_normal);
}
