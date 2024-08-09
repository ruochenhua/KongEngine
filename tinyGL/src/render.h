#pragma once
#include "CameraComponent.h"
#include "Common.h"
#include "skybox.h"
#include "Shader/Shader.h"

namespace tinyGL
{
	class PostprocessShader;
	class CPointLightComponent;
	class CDirectionalLightComponent;
	class CMeshComponent;
	class CModelMeshComponent;
	class CCamera;

	// 针对场景中的所有渲染物，使用UBO存储基础数据优化性能
	class UBOHelper
	{
	public:
		template <class T>
		void AppendData(T data, const std::string& name);

		template <class T>
		void UpdateData(const T& data, const std::string& name) const;
		
		void Init(GLuint in_binding);
		// 开始绑定
		void Bind() const;
		// 结束绑定
		void EndBind() const;
	private:
		std::map<string, unsigned> data_offset_cache;
		size_t next_offset = 0;
		GLuint binding = GL_NONE;
		GLuint ubo_idx = GL_NONE;
	};

	template <class T>
	void UBOHelper::AppendData(T data, const std::string& name)
	{
		data_offset_cache.emplace(name, next_offset);
		//UpdateStd140Offset(data);
		size_t size = sizeof(T);
		next_offset += size;
	}

	template <class T>
	void UBOHelper::UpdateData(const T& data, const std::string& name) const
	{
		auto find_iter = data_offset_cache.find(name);
		if(find_iter == data_offset_cache.end())
		{
			assert(false, "update data failed");
			return;
		}

		unsigned offset = find_iter->second;
		size_t size = sizeof(T);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &data);
	}

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
		void InitUBO();
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

		shared_ptr<PostprocessShader> postprocess_shader;

		CCamera* mainCamera = nullptr;
		
		// 场景光源信息
		SSceneRenderInfo scene_render_info;
		
		/* 矩阵UBO，保存场景基础的矩阵信息
		 *  mat4 model
		 *  mat4 view
		 *  mat4 projection
		 *  vec3 cam_pos;
		 */
		UBOHelper matrix_ubo;

		// 光照UBO，保存场景基础的光照信息
		/*		
		 *	SceneLightInfo light_info
		 */
		UBOHelper scene_light_ubo;
	};
}
