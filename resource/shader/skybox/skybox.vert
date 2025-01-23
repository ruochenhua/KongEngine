#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

layout(location = 0) in vec3 aPosition;
out vec3 uv;

void main(){
	mat4 view_no_translation = matrix_ubo.view;
	// 先列再行
	view_no_translation[3][0] = 0.0;
	view_no_translation[3][1] = 0.0;
	view_no_translation[3][2] = 0.0;

	mat4 mvp = matrix_ubo.projection * view_no_translation;

	gl_Position = (mvp * vec4(aPosition, 1.0)).xyww;
	uv = aPosition;
}