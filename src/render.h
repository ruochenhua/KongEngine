#pragma once
#include "common.h"
class CModel;
class CRender
{
public:

	int Init();
	int Update();

	bool AddRenderVertices(const std::vector<float>& vertices);

	GLuint LoadShaders(const std::string& vs, const std::string& fs);

private:
	GLFWwindow* m_pWindow;
};