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
		static CRender* GetRender();
		
		GLFWwindow* render_window;
		CRender(){ }

		int Init();
		int Update(double delta);
		void PostUpdate();

		void RenderSceneObject();
		// todo:整合一下吧这两个
		void RenderShadowMap();
		
		// load image file and create texture 
		static GLuint LoadTexture(const std::string& texture_path);
		
	private:
		int InitRender();
		int InitCamera();
		void RenderSkyBox();
		void RenderScene() const;
		
		//void UpdateLightDir(float delta);
		
	private:
		CSkyBox m_SkyBox;

		GLuint null_tex_id			= GL_NONE;
		
		// debug
		GLuint m_ShadowMapDebugShaderId = GL_NONE;
		GLuint m_QuadVAO = GL_NONE;
		GLuint m_QuadVBO = GL_NONE;
		
		glm::mat4 light_space_mat;

		CCamera* mainCamera = nullptr;
	};
}
