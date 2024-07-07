#pragma once
#include "Camera.h"
#include "common.h"
#include "skybox.h"

namespace tinyGL
{
	class CModel;
	class CCamera;

	class CRender
	{
	public:
		static GLFWwindow* s_pWindow;
		static GLuint LoadShaders(const std::string& vs, const std::string& fs);

	public:
		CRender()
			: m_LightDir(glm::normalize(glm::vec3(-1, -1, 0)))
			, m_LightColor(0.8, 0.8, 0.8)
			, m_LightPos(5, 5, 0)
		{ }

		int Init();
		int Update();

		SRenderInfo AddModel(CModel* model, const std::string shader_paths[2]);

	private:
		int InitRender();
		int InitCameraControl();

		void RenderSkyBox();
		void RenderShadowMap(const SRenderInfo& render_info);
		void RenderModel(const SRenderInfo& render_info) const;

	private:
		std::vector<SRenderInfo> m_vRenderInfo;
		CSkyBox m_SkyBox;

		glm::vec3 m_LightDir;
		glm::vec3 m_LightColor;
		glm::vec3 m_LightPos;

		//shadow map
		GLuint m_FrameBuffer;
		GLuint m_ShadowMapProgramID;
		GLuint m_DepthTexture;
		GLuint m_DepthMatrixID;
		glm::mat4 m_DepthMVP;

		CCamera* mainCamera = nullptr;
	};
}