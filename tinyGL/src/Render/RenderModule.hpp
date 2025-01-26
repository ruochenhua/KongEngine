#pragma once
#include "Component/CameraComponent.h"
#include "Common.h"
#include "Render/PostProcessRenderSystem.hpp"
#include "Render/RenderSystem.hpp"
#include "Render/SkyboxRenderSystem.hpp"
#include "Component/Mesh/Water.h"
#include "Shader/DeferInfoShader.h"
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
#if USE_DSA
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
		// glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &data);
		glNamedBufferSubData(ubo_idx, offset, size, &data);
	}

#else
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
#endif
	
	// 延迟渲染的数据结构
	struct DeferBuffer
	{
		GLuint g_buffer_	= 0;
		GLuint g_position_	= 0;
		GLuint g_normal_	= 0;
		GLuint g_albedo_	= 0;
		GLuint g_orm_		= 0;	//o:ao, r:roughness, m: metallic
		GLuint g_rbo_		= 0;
		
		void Init(unsigned width, unsigned height);
		void GenerateDeferRenderTextures(int width, int height);
		// 延迟渲染着色器
		shared_ptr<DeferredBRDFShader> defer_render_shader;
	};

	// ssao渲染相关
	struct SSAOHelper
	{
		unsigned ssao_kernel_count = 64;
		unsigned ssao_noise_size = 4;
		vector<glm::vec3> ssao_kernal_samples;
		vector<glm::vec3> ssao_kernal_noises;
		
		GLuint ssao_fbo = GL_NONE;
		GLuint SSAO_BlurFBO = GL_NONE;
		GLuint ssao_noise_texture = GL_NONE;
		GLuint ssao_result_texture = GL_NONE;
		GLuint ssao_blur_texture = GL_NONE;
		shared_ptr<SSAOShader> ssao_shader_;
		shared_ptr<Shader> ssao_blur_shader_;

		void Init(int width, int height);
		void GenerateSSAOTextures(int width, int height);
	};

	// water渲染相关
	struct WaterRenderHelper
	{
		weak_ptr<AActor> water_actor;
		// 反射部分，需要变换相机角度重新渲染
		GLuint water_reflection_fbo = GL_NONE;
		GLuint water_reflection_rbo = GL_NONE;
		GLuint water_reflection_texture = GL_NONE;

		// 折射部分，先使用延迟渲染的结果复制过来
		GLuint water_refraction_fbo = GL_NONE;
		GLuint water_refraction_texture = GL_NONE;
		
		void Init(int width, int height);
		void GenerateWaterRenderTextures(int width, int height);

		float total_move = 0.0f;
		float move_speed = 0.01f;
	};
	
	class KongRenderModule
	{
	public:
		static KongRenderModule& GetRenderModule();
		static GLuint GetNullTexId();
		static glm::vec2 GetNearFar();
		static shared_ptr<CQuadShape> GetScreenShape();
		
		KongRenderModule() = default;

		GLuint GetSkyboxTexture() const;
		GLuint GetSkyboxDiffuseIrradianceTexture() const;
		GLuint GetSkyboxPrefilterTexture() const;
		GLuint GetSkyboxBRDFLutTexture() const;
		
		GLuint GetLatestDepthTexture() const;
		int Init();
		int Update(double delta);
		void RenderUI(double delta);
		
		void DoPostProcess();
		
		CCamera* GetCamera() {return mainCamera;}


		void ChangeSkybox();
		void OnWindowResize(int width, int height);

		void SetRenderWater(const weak_ptr<AActor>& water_actor);

		double render_time = 0.0;
		
		PostProcessRenderSystem post_process;
		int render_sky_env_status = 2;
		// 启用屏幕空间环境光遮蔽
		bool use_ssao = false;
		// 启用反射阴影贴图
		bool use_rsm = false;
		float rsm_intensity = 0.04f;
		int rsm_sample_count = 32;
		
		vector<glm::vec4> rsm_samples_and_weights;
		// 启用PCSS
		bool use_pcss = false;
		float pcss_radius = 1.0f;
		float pcss_light_scale = 0.1f;
		int pcss_sample_count = 36;
		
		// 启用屏幕空间反射
		bool use_screen_space_reflection = true;
		SkyboxRenderSystem m_SkyBox;
		
		// 场景光源信息
		SSceneLightInfo scene_render_info;
	private:
		int InitCamera();
		// 更新场景的渲染信息（光照、相机等等）
		void UpdateSceneRenderInfo();
		void InitUBO();
		void RenderSkyBox(GLuint depth_texture = 0);
		// 渲染不支持延迟渲染的物体
		void RenderNonDeferSceneObjects() const;
		// 延迟渲染，将场景渲染到GBuffer上
		void DeferRenderSceneToGBuffer() const;
		// 利用GBuffer的信息，渲染光照
		void DeferRenderSceneLighting() const;
		// 渲染水
		void RenderWater();
		
		void SSAORender() const;
		void SSReflectionRender() const;
		// 预先处理一下场景中的光照。目前场景只支持一个平行光和四个点光源，后续需要根据object的位置等信息映射对应的光源
		void CollectLightInfo();
		void RenderSceneObject(bool water_reflection);
		
		void RenderShadowMap();
	private:
		
		GLuint null_tex_id			= GL_NONE;
		
		shared_ptr<Shader> shadowmap_debug_shader;
		GLuint m_QuadVAO = GL_NONE;
		GLuint m_QuadVBO = GL_NONE;

		CCamera* mainCamera = nullptr;
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

		// 延迟渲染
		DeferBuffer defer_buffer_;
		// ssao实现
		SSAOHelper ssao_helper_;
		// 水体渲染实现
		WaterRenderHelper water_render_helper_;

		shared_ptr<CQuadShape> quad_shape;
		// 屏幕空间反射
		shared_ptr<SSReflectionShader> ssreflection_shader;

		std::vector<KongRenderSystem> m_renderSystems;
	};
}
