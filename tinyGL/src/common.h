#pragma once
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <sstream>

#include "GLFW/glfw3.h"
#include "GLM/glm.hpp"
#include "GLM/gtc/matrix_transform.hpp"
using namespace std;
namespace tinyGL
{
	struct SMaterial
	{
		glm::vec3 albedo = glm::vec3(0.f); 	
		float metallic = 0.5f;
		float roughness = 0.5;
		float ao = 0.3f;
	};

	// 渲染信息
	struct SRenderInfo
	{
		// vertex buffer id(vbo)
		GLuint vertex_buffer = 0;
		// ibo
		GLuint index_buffer = 0;
		// vao
		GLuint vertex_array_id = 0;
		GLuint texture_buffer = 0;
		GLuint normal_buffer = 0;

		SMaterial material;
		// shader program
		GLuint program_id = 0;
		unsigned vertex_size = 0;
		unsigned stride_count = 1;
		unsigned indices_count = 0;
		// texture id
		// todo: 总不能一个一个加吧，要支持类型映射
		GLuint diffuse_tex_id = 0;
		GLuint specular_tex_id = 0;
		GLuint normal_tex_id = 0;
	};

	// 渲染资源描述
	struct SRenderResourceDesc
	{
		enum EShaderType
		{
			vs = GL_VERTEX_SHADER,		// vertex shader
			fs = GL_FRAGMENT_SHADER,	// fragment shader
		};

		enum ETextureType
		{
			diffuse = 0,
			specular,
			normal,
			metallic,
			roughness,
			ambient_occlusion,
			glow,
		};
		
		map<EShaderType, string> shader_paths;
		map<ETextureType, string> texture_paths;

		string model_path;
		SMaterial material;
	};
	
	const string RESOURCE_PATH = "../resource/";

	// inline static GLFWwindow* GetWindowPtr()
	// {
	// 	return g_render_window;
	// }
}