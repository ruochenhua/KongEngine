#version 450 compatibility

layout(location = 0) in vec3 aPosition;

out vec3 uv;

uniform mat4 MVP;

void main(){
	gl_Position = (MVP * vec4(aPosition, 1.0)).xyww;
	uv = aPosition;
}