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

// 使用opengl 4.5的(Direct State Access)特性，减少绑定和解绑操作，优化性能
#define USE_DSA 1

namespace Kong
{
	const string RESOURCE_PATH = "../resource/";
	constexpr  int SHADOW_RESOLUTION = 2048;
}