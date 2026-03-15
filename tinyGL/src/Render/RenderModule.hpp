#pragma once
#include "Component/CameraComponent.h"
#include "Common.h"
#include "GraphicsAPI/OpenGL/RenderSystem/OpenGLRenderSystem.hpp"
#include "Render/Abstraction/IRenderSystem.hpp"
#include "Render/Abstraction/RenderSystemAdapter.hpp"
#include "Render/Abstraction/Types.hpp"
#include "Render/Abstraction/IRenderModuleBackend.hpp"
#include "Render/Abstraction/BackendType.hpp"

#include "Shader/OpenGL/OpenGLShader.h"

#include <vector>
#include <memory>

namespace Kong
{
	class IGraphicsDevice;
	class CQuadShape;
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
		struct GlobalVulkanUbo
		{
			glm::mat4 projection {1.f};
			glm::mat4 view {1.f};
			
			glm::vec4 cameraPosition = glm::normalize(glm::vec4{1.f, 0.f, 0.f, 1.f});

			// SceneLightInfo sceneLightInfo;
            SceneLightInfo sceneLightInfo;
		};
		
		static KongRenderModule& GetRenderModule();
		static KongTexture* GetNullTex();
		static glm::vec2 GetNearFar();
		static shared_ptr<CQuadShape> GetScreenShape();
		
		KongRenderModule() = default;
		~KongRenderModule();

		KongRenderModule(const KongRenderModule&) = delete;
		KongRenderModule& operator=(const KongRenderModule&) = delete;
		
		/** 初始化；传入当前图形设备（由 Window 提供），可为 nullptr 走兼容路径 */
		int Init(IGraphicsDevice* device = nullptr);
		int Update(double delta);
		/** 统一 RHI 路径：传入当前帧上下文，按 m_renderSystems 顺序执行各 Pass */
		int Update(double delta, IFrameContext* frameContext);
		void RenderUI(double delta);

		/** 供 IRenderModuleBackend 注册 Pass 使用 */
		void PushRenderSystem(std::unique_ptr<IRenderSystem> sys);

		/** 当前后端，Vulkan 路径下可转为 RenderModuleBackendVulkan* 取 descriptor pool/sets */
		IRenderModuleBackend* GetBackend() { return m_backend.get(); }
		
		shared_ptr<CCamera> GetCamera() const {return mainCamera;}

		void OnWindowResize(int width, int height);

		void SetRenderWater(const weak_ptr<AActor>& water_actor);
		void OnReloadScene();

		/** OpenGL 主 FBO / 主颜色附件，供后端 DrawMainScene 使用 */
		GLuint GetMainFBO() const { return m_renderToBuffer; }
		GLuint GetMainColorTexture(unsigned index) const { return index < FRAGOUT_TEXTURE_COUNT ? m_renderToTextures[index] : 0; }

		/** ImGui 等与相机/SSR 等相关的 UI，在具体 RenderSystem 的 DrawUI 之前调用 */
		void RenderUIBeforeSystems();

		double render_time = 0.0;
		// 预先处理一下场景中的光照。目前场景只支持一个平行光和四个点光源，后续需要根据object的位置等信息映射对应的光源
		RenderResultInfo RenderSceneObject(GLuint target_fbo = GL_NONE);
		
		// 启用屏幕空间反射
		bool use_screen_space_reflection = true;

		// 场景光源信息
		SSceneLightInfo scene_render_info;
		
		GLuint m_renderToTextures[FRAGOUT_TEXTURE_COUNT] = {0, 0, 0};

		OpenGLRenderSystem* GetRenderSystemByType(RenderSystemType type);

		/** 当前后端状态，由 Init(device) 时创建 */
		std::unique_ptr<IRenderModuleBackend> m_backend;

		/* 矩阵UBO，保存场景基础的矩阵信息（OpenGL 路径使用） */
		UBOHelper matrix_ubo;
	private:
		// 更新场景的渲染信息（光照、相机等等）
		void UpdateSceneRenderInfo();
		void InitUBO();
		void InitMainFBO();
		
		// 渲染不支持延迟渲染的物体；skybox_render_sky_env_status 由 OpenGL 后端传入
		void RenderNonDeferSceneObjects(int skybox_render_sky_env_status) const;
		
		void RenderShadowMap();

		RenderResultInfo latestRenderResult{};
	private:
		friend class CYamlParser;
		friend class KongSceneManager;
		
		GLuint m_renderToBuffer {0};    // 渲染到的buffer
		GLuint m_renderToRbo {0};

		weak_ptr<KongTexture> m_nullTex;

		// todo: 删掉，统一用m_nullTex;
		shared_ptr<OpenGLShader> shadowmap_debug_shader;
		
#if SHADOWMAP_DEBUG
		GLuint m_QuadVAO = GL_NONE;
		GLuint m_QuadVBO = GL_NONE;
#endif

		shared_ptr<CCamera> mainCamera{};
		

		// 光照UBO，保存场景基础的光照信息（OpenGL 路径使用）
		UBOHelper scene_light_ubo;

		shared_ptr<CQuadShape> m_quadShape;

		/** 按固定顺序注册的渲染 Pass，由 Update(delta, frameContext) 统一驱动 */
		std::vector<std::unique_ptr<IRenderSystem>> m_renderSystems;

		/** 当前 RHI 设备，由 Init(device) 设置 */
		IGraphicsDevice* m_device {nullptr};

#ifndef RENDER_IN_VULKAN
		friend class RenderModuleBackendOpenGL;
#endif
#ifdef RENDER_IN_VULKAN
		friend class RenderModuleBackendVulkan;
#endif
	};
}
