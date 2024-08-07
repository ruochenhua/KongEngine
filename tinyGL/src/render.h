#pragma once
#include "CameraComponent.h"
#include "Common.h"
#include "skybox.h"
#include "Shader/Shader.h"

namespace tinyGL
{
	class CPointLightComponent;
	class CDirectionalLightComponent;
	class CMeshComponent;
	class CModelMeshComponent;
	class CCamera;
	
	class CRender
	{
	public:
		static CRender* GetRender();
		static GLuint GetNullTexId();
		
		GLFWwindow* render_window;
		CRender() = default;

		int Init();
		int Update(double delta);
		void PostUpdate();
		
		// load image file and create texture 
		static GLuint LoadTexture(const std::string& texture_path);
		
	private:
		int InitCamera();
		void RenderSkyBox();
		void RenderScene() const;

		// 预先处理一下场景中的光照。目前场景只支持一个平行光和四个点光源，后续需要根据object的位置等信息映射对应的光源
		void CollectLightInfo();
		void RenderSceneObject();
		// todo:整合一下吧这两个
		void RenderShadowMap();
		
		
	private:
		CSkyBox m_SkyBox;

		GLuint null_tex_id			= GL_NONE;
		
		// debug
		// GLuint m_ShadowMapDebugShaderId = GL_NONE;
		shared_ptr<Shader> shadowmap_debug_shader;
		GLuint m_QuadVAO = GL_NONE;
		GLuint m_QuadVBO = GL_NONE;
		
		glm::mat4 light_space_mat;

		CCamera* mainCamera = nullptr;

		// 场景光源信息
		SSceneRenderInfo scene_render_info;
		
		// 针对场景中的所有渲染物，使用UBO存储基础数据优化性能
		/*
		 *  mat4 model
		 *  mat4 view
		 *  mat4 projection
		 */
        
		GLuint matrix_ubo_idx = GL_NONE;
	};
}
