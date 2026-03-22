#pragma once
#include "Component/CameraComponent.h"
#include "Common.h"
#include "GraphicsAPI/OpenGL/RenderSystem/OpenGLRenderSystem.hpp"
#include "Render/RenderCommon.hpp"

#include "Render/Abstraction/IRenderPassHost.hpp"
#include "Render/Abstraction/IRenderSystem.hpp"
#include "Render/Abstraction/RenderSubsystemTypes.hpp"
#include "Render/Abstraction/RHIRenderSubsystems.hpp"
#include "Render/Abstraction/Types.hpp"

#include <vector>
#include <memory>

namespace Kong
{
	class IFrameContext;
	class IRenderModuleBackend;
	class IGraphicsDevice;
	class CQuadShape;
	class CCamera;
	class OpenGLRenderPassHost;

	class KongRenderModule
	{
	public:
		struct GlobalVulkanUbo
		{
			glm::mat4 projection {1.f};
			glm::mat4 view {1.f};

			glm::vec4 cameraPosition = glm::normalize(glm::vec4{1.f, 0.f, 0.f, 1.f});

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

		int Init(IGraphicsDevice* device = nullptr);
		int Update(double delta);
		int Update(double delta, IFrameContext* frameContext);
		void RenderUI(double delta);

		void PushRenderSystem(std::unique_ptr<IRenderSystem> sys);

		IRenderModuleBackend* GetRenderBackend() { return m_backend.get(); }

		/** 后端 Pass 宿主（OpenGL/Vulkan 实现）；无图形设备时为空 */
		IRenderPassHost* GetRenderPassHost() { return m_passHost.get(); }

		OpenGLRenderSystem* GetOpenGLSubsystem(RenderSystemType type);
		IRHIRenderSubsystem* GetRHISubsystem(RHISubsystemKind kind);

		shared_ptr<CCamera> GetCamera() const { return mainCamera; }

		void OnWindowResize(int width, int height);

		void SetRenderWater(const weak_ptr<AActor>& water_actor);
		void OnReloadScene();

		GLuint GetMainFBO();
		GLuint GetMainColorTexture(unsigned index);
		IFramebuffer* GetMainSceneFramebuffer();

		void RenderUIBeforeSystems();

		double render_time = 0.0;
		RenderResultInfo RenderSceneObject(GLuint target_fbo = GL_NONE);

		bool use_screen_space_reflection = true;

		SSceneLightInfo scene_render_info;

		std::unique_ptr<IRenderModuleBackend> m_backend;

	private:
		friend class OpenGLRenderPassHost;

		void UpdateSceneRenderInfo();
		void RenderNonDeferSceneObjects(int skybox_render_sky_env_status) const;
		void RenderShadowMap();

		RenderResultInfo latestRenderResult{};

		friend class CYamlParser;
		friend class KongSceneManager;

		weak_ptr<KongTexture> m_nullTex;

		shared_ptr<CCamera> mainCamera{};

		shared_ptr<CQuadShape> m_quadShape;

		std::vector<std::unique_ptr<IRenderSystem>> m_renderSystems;

		IGraphicsDevice* m_device {nullptr};

		std::unique_ptr<IRenderPassHost> m_passHost;
	};
}
