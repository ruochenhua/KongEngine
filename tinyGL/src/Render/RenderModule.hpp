#pragma once
#include "Component/CameraComponent.h"
#include "Common.h"
#include "RenderSystem/DeferRenderSystem.hpp"
#include "RenderSystem/SSReflectionRenderSystem.hpp"
#include "RenderSystem/WaterRenderSystem.hpp"
#include "RenderSystem/PostProcessRenderSystem.hpp"
#include "RenderSystem/RenderSystem.hpp"
#include "RenderSystem/SkyboxRenderSystem.hpp"
#include "Shader/OpenGL/OpenGLShader.h"

namespace Kong
{
	class VulkanPostprocessSystem;
}

namespace Kong
{
	class VulkanSwapChain;
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
			glm::mat4 projectionView {1.f};
			glm::vec4 lightDirection = glm::normalize(glm::vec4{-1.f, 3.f, -1.0f, 0.0});
			glm::vec4 cameraPosition = glm::normalize(glm::vec4{1.f, 0.f, 0.f, 1.f});

			// SceneLightInfo sceneLightInfo;
            
		};
		
		static KongRenderModule& GetRenderModule();
		static GLuint GetNullTexId();
		static KongTexture* GetNullTex();
		static glm::vec2 GetNearFar();
		static shared_ptr<CQuadShape> GetScreenShape();
		
		KongRenderModule() = default;
		~KongRenderModule();

		KongRenderModule(const KongRenderModule&) = delete;
		KongRenderModule& operator=(const KongRenderModule&) = delete;
		
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

#ifdef RENDER_IN_VULKAN
		
		std::unique_ptr<VulkanDescriptorPool> m_descriptorPool{};
		// vulkan的全局descriptorset，也是用于保存场景的基础信息
		std::unique_ptr<VulkanDescriptorSetLayout> m_descriptorLayout;
		std::vector<std::unique_ptr<VulkanBuffer>> m_uniformBuffers;
		std::vector<VkDescriptorSet> m_descriptorSets;

		int GetFrameIndex() const;

		void BeginFrame();
		void EndFrame();

		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

		bool IsFrameInProgress() const {return m_isFrameStarted;}
		VkCommandBuffer GetCurrentCommandBuffer() const;

		VkRenderPass GetSwapChainRenderPass() const;
		float GetAspectRatio() const;
		
		uint32_t m_currentImageIndex {0};
		int m_currentFrameIndex {0};
		bool m_isFrameStarted {false};

		// todo: 放到private
		std::unique_ptr<SimpleVulkanRenderSystem> m_simpleRenderSystem{nullptr};
		std::unique_ptr<VulkanPostprocessSystem> m_vulkanPostProcessSystem{nullptr};
#endif
		/* 矩阵UBO，保存场景基础的矩阵信息
		 */
		UBOHelper matrix_ubo;
	private:
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
		friend class KongSceneManager;
		
		GLuint m_renderToBuffer {0};    // 渲染到的buffer
		GLuint m_renderToRbo {0};

		weak_ptr<KongTexture> m_nullTex;

		// todo: 删掉，统一用m_nullTex;
		GLuint null_tex_id			= GL_NONE;
		shared_ptr<OpenGLShader> shadowmap_debug_shader;
		
#if SHADOWMAP_DEBUG
		GLuint m_QuadVAO = GL_NONE;
		GLuint m_QuadVBO = GL_NONE;
#endif

#ifdef RENDER_IN_VULKAN
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void RecreateSwapChain();

		std::unique_ptr<VulkanSwapChain> m_swapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;

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
