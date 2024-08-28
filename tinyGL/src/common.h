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
	const int SHADOW_WIDTH = 2048;
	const int SHADOW_HEIGHT = 2048;
}