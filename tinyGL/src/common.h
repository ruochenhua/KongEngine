#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GLM/glm.hpp"
#include "GLM/gtc/matrix_transform.hpp"
using namespace std;
namespace tinyGL
{
	class TGAImage;

	struct SMaterial
	{
		float _shininess = 0.8f;
	};

	// ��Ⱦ��Ϣ
	struct SRenderInfo
	{
		// vertex buffer id(vbo)
		GLuint vertexBuffer = 0;
		// ibo
		GLuint indexBuffer = 0;
		// vao
		GLuint vertexArrayId = 0;
		GLuint _texture_buffer = 0;

		GLuint _normal_buffer = 0;

		SMaterial _material;
		// ����Ⱦ��λ��shader����
		GLuint _program_id = 0;
		unsigned _vertex_size = 0;
		unsigned _stride_count = 1;
		unsigned _indices_count = 0;
		// ��ͼid
		GLuint diffuse_tex_id = 0;
		GLuint specular_map_tex_id = 0;
	};

	const std::string RESOURCE_PATH = "../../../../resource/";

	// inline static GLFWwindow* GetWindowPtr()
	// {
	// 	return g_render_window;
	// }
}