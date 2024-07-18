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
		GLFWwindow* render_window;
		static GLuint LoadShaders(const std::string& vs, const std::string& fs);

	public:
		CRender()
			: m_LightDir(glm::normalize(glm::vec3(-1, 1, 0)))
			, m_LightColor(1, 1, 1)
			, m_LightPos(5, 5, 5)
		{ }

		int Init();
		int Update(double delta);

		void AddRenderInfo(SRenderInfo render_info, const std::string shader_paths[2]);
		
	private:
		int InitRender();
		int InitCamera();

		void RenderSkyBox();
		void RenderShadowMap(const SRenderInfo& render_info);
		void RenderModel(const SRenderInfo& render_info) const;

		void UpdateLightDir(float delta);
		
	private:
		std::vector<SRenderInfo> m_vRenderInfo;
		CSkyBox m_SkyBox;

		glm::vec3 m_LightDir;
		glm::vec3 m_LightColor;
		glm::vec3 m_LightPos;

		//shadow map
		GLuint m_FrameBuffer;
		GLuint m_ShadowMapProgramID;	// 包含阴影贴图的shader
		GLuint m_DepthTexture;			// 深度贴图
		GLuint m_DepthMatrixID;
		glm::mat4 m_DepthMVP;

		CCamera* mainCamera = nullptr;

		double light_yaw = 0.0;
		double light_pitch = 45.0;
	};
}