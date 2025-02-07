#pragma once
#include "Component/CameraComponent.h"
#include "Common.h"
#include "RenderSystem/DeferRenderSystem.hpp"
#include "RenderSystem/SSReflectionRenderSystem.hpp"
#include "RenderSystem/WaterRenderSystem.hpp"
#include "RenderSystem/PostProcessRenderSystem.hpp"
#include "RenderSystem/RenderSystem.hpp"
#include "RenderSystem/SkyboxRenderSystem.hpp"
#include "Shader/Shader.h"

namespace Kong
{
	class CCamera;

	// 针对场景中的所有渲染物，使用UBO存储基础数据优化性能
	/*
	 *  mat4 view
	 *  mat4 projection
	 *  vec4 cam_pos;
	 *  vec4 near_far;
	 */
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
	
	class KongRenderModule
	{
	public:
		static KongRenderModule& GetRenderModule();
		static GLuint GetNullTexId();
		static glm::vec2 GetNearFar();
		static shared_ptr<CQuadShape> GetScreenShape();
		
		KongRenderModule() = default;
		
		int Init();
		int Update(double delta);
		void RenderUI(double delta);
		
		shared_ptr<CCamera> GetCamera() const {return mainCamera;}

		void OnWindowResize(int width, int height);

		void SetRenderWater(const weak_ptr<AActor>& water_actor);

		double render_time = 0.0;
		
		// 预先处理一下场景中的光照。目前场景只支持一个平行光和四个点光源，后续需要根据object的位置等信息映射对应的光源
		RenderResultInfo RenderSceneObject(GLuint target_fbo = GL_NONE);
		
		// 启用屏幕空间反射
		bool use_screen_space_reflection = true;

		// 场景光源信息
		SSceneLightInfo scene_render_info;
		
		GLuint m_renderToTextures[FRAGOUT_TEXTURE_COUNT] = {0, 0, 0};

		KongRenderSystem* GetRenderSystemByType(RenderSystemType type);

		/* 矩阵UBO，保存场景基础的矩阵信息
		 */
		UBOHelper matrix_ubo;
	private:
		int InitCamera();
		// 更新场景的渲染信息（光照、相机等等）
		void UpdateSceneRenderInfo();
		void InitUBO();
		void InitMainFBO();
		
		// 渲染不支持延迟渲染的物体
		void RenderNonDeferSceneObjects() const;
		
		void RenderShadowMap();

		RenderResultInfo latestRenderResult{};
	private:
		friend class CYamlParser;
		
		GLuint m_renderToBuffer {0};    // 渲染到的buffer
		GLuint m_renderToRbo {0};
		
		GLuint null_tex_id			= GL_NONE;
		shared_ptr<Shader> shadowmap_debug_shader;
#if SHADOWMAP_DEBUG
		GLuint m_QuadVAO = GL_NONE;
		GLuint m_QuadVBO = GL_NONE;
#endif
		
		shared_ptr<CCamera> mainCamera{};
		

		// 光照UBO，保存场景基础的光照信息
		/*		
		 *	SceneLightInfo light_info
		 */
		UBOHelper scene_light_ubo;

		// 天空盒
		SkyboxRenderSystem m_skyboxRenderSystem;
		// 延迟渲染
		DeferRenderSystem m_deferRenderSystem;
		// 后处理
		PostProcessRenderSystem m_postProcessRenderSystem;
		// 屏幕空间反射
		SSReflectionRenderSystem m_ssReflectionRenderSystem;
		// 水体渲染实现
		WaterRenderSystem m_waterRenderSystem;

		shared_ptr<CQuadShape> m_quadShape;
	};
}
