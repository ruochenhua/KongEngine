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

namespace Kong
{
	const string RESOURCE_PATH = "../resource/";
	constexpr  int SHADOW_RESOLUTION = 2048;
}