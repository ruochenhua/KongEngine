#pragma once
#include "Camera.h"
#include "common.h"
#include "skybox.h"

namespace tinyGL
{
	class Light;
	class CRenderObj;
	class CModel;
	class CCamera;

	class CRender
	{
	public:
		GLFWwindow* render_window;
		CRender(){ }

		int Init();
		void InitLights(const vector<shared_ptr<Light>>& lights);
		int Update(double delta);
		void PostUpdate();

		void RenderSceneObject(shared_ptr<CRenderObj> render_obj);
		
		// load image file and create texture 
		static GLuint LoadTexture(const std::string& texture_path);
		
	private:
		int InitRender();
		int InitCamera();
		void RenderSkyBox();
		void RenderShadowMap(const SRenderInfo& render_info);

		//void UpdateLightDir(float delta);
		
	private:
		CSkyBox m_SkyBox;
		//shadow map
		GLuint m_FrameBuffer		= GL_NONE;
		GLuint m_ShadowMapProgramID = GL_NONE;	// ������Ӱ��ͼ��shader
		GLuint m_DepthTexture		= GL_NONE;			// �����ͼ
		GLuint m_DepthMatrixID		= GL_NONE;
		GLuint null_tex_id			= GL_NONE;
		glm::mat4 m_DepthMVP;

		CCamera* mainCamera = nullptr;

		double light_yaw = 0.0;
		double light_pitch = 45.0;

		vector<shared_ptr<Light>> scene_lights;
	};
}
