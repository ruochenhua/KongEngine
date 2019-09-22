#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
class TGAImage;

struct SRenderInfo
{
	GLuint _vertex_buffer = 0;
	GLuint _vertex_array_id = 0;

	GLuint _texture_buffer = 0;
	GLuint _texture_array_id = 0;

	GLuint _program_id = 0;
	unsigned _vertex_size = 0;
	TGAImage* _texture_img = nullptr;
};