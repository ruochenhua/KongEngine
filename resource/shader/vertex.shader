#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexTextureCoord;
layout(location = 2) in vec3 vertexNormal_modelspace;

out vec2 uv;
out vec3 normal_world;
out vec3 in_pos;

uniform mat4 MVP;

void main(){
	gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0); 	
    in_pos = gl_Position.xyz;

	uv = vertexTextureCoord;	
	
	normal_world = (MVP * vec4(vertexNormal_modelspace, 1.0)).xyz;	
		
}