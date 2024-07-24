#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <sstream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GLM/glm.hpp"
#include "GLM/gtc/matrix_transform.hpp"
using namespace std;
namespace tinyGL
{
	struct SMaterial
	{
		float shininess = 0.8f;
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
		GLuint diffuse_tex_id = 0;
		GLuint specular_map_tex_id = 0;
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
			specular_map
		};
		
		map<EShaderType, string> shader_paths;
		map<ETextureType, string> texture_paths;

		string model_path;
	};
	
	const string RESOURCE_PATH = "../../../../resource/";

	// inline static GLFWwindow* GetWindowPtr()
	// {
	// 	return g_render_window;
	// }
}