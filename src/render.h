#pragma once
#include "common.h"
class CModel;
class CRender
{
public:

	int Init();
	int Update();

	SRenderInfo AddModel(CModel* model, const std::string shader_paths[2]);

	GLuint LoadShaders(const std::string& vs, const std::string& fs);

private:
	void RenderModel(const SRenderInfo& render_info) const;

private:
	GLFWwindow* m_pWindow;
	std::vector<SRenderInfo> m_vRenderInfo;
};