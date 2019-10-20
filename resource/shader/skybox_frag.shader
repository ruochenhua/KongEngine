#version 330 core

in vec3 uv;
out vec3 color;

uniform samplerCube skybox;

void main(){
	color = texture(skybox, uv).rgb;
}