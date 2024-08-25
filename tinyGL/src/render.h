#pragma once
#include "Component/CameraComponent.h"
#include "Common.h"
#include "postprocess.h"
#include "skybox.h"
#include "Shader/Shader.h"

namespace Kong
{
	class FinalPostprocessShader;
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

		GLuint GetSkyboxTexture() const;
		GLuint GetSkyboxDiffuseIrradianceTexture() const;
		GLuint GetSkyboxPrefilterTexture() const;
		GLuint GetSkyboxBRDFLutTexture() const;
		int Init();
		int Update(double delta);
		void PostUpdate();
		CCamera* GetCamera() {return mainCamera;}
		void ChangeSkybox();
		// load image file and create texture 
		static GLuint LoadTexture(const std::string& texture_path, bool flip_uv = true);
		
		PostProcess post_process;
		int render_sky_env_status = 0;
	private:
		int InitCamera();
		void InitUBO();
		void RenderSkyBox();
		void RenderScene() const;

		// 预先处理一下场景中的光照。目前场景只支持一个平行光和四个点光源，后续需要根据object的位置等信息映射对应的光源
		void CollectLightInfo();
		void RenderSceneObject();
		
		void RenderShadowMap();
	private:
		
		CSkyBox m_SkyBox;
		GLuint null_tex_id			= GL_NONE;
		
		shared_ptr<Shader> shadowmap_debug_shader;
		GLuint m_QuadVAO = GL_NONE;
		GLuint m_QuadVBO = GL_NONE;


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
