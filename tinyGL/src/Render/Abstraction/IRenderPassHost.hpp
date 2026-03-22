/**
 * @file IRenderPassHost.hpp
 * @brief 渲染 Pass 与子系统宿主：由各图形后端实现，KongRenderModule 仅持有接口指针，不区分 OpenGL/Vulkan。
 */

#pragma once

#include "Render/Abstraction/RenderSubsystemTypes.hpp"
#include "Render/Abstraction/RHIRenderSubsystems.hpp"

#include <cstdint>
#include <memory>

namespace Kong
{
    class KongRenderModule;
    class IGraphicsDevice;
    class IRHICommandList;
    class IFramebuffer;
    class OpenGLRenderSystem;
    struct SceneLightInfo;
    class AActor;

    /**
     * 持有后端子系统、主场景 FBO（OpenGL）、注册 IRenderSystem 拓扑。
     * OpenGL：OnAttached 内完成 FBO/UBO/Gl* 初始化并 RegisterDefaultRenderPasses。
     * Vulkan：OnAttached 可为空；AfterVulkanDescriptorReady 内创建 Vk* 并 PushRenderSystem。
     */
    class IRenderPassHost
    {
    public:
        virtual ~IRenderPassHost() = default;

        /** 在 KongRenderModule::Init 早期调用（OpenGL：FBO/UBO/子系统 + 注册 Pass） */
        virtual void OnAttached(KongRenderModule& module, IGraphicsDevice* device) = 0;

        /** 注册拓扑（OpenGL 可在 OnAttached 末尾调用；Vulkan 在 AfterVulkanDescriptorReady 调用） */
        virtual void RegisterDefaultRenderPasses(KongRenderModule& module) = 0;

        virtual OpenGLRenderSystem* TryGetOpenGLSubsystem(RenderSystemType type)
        {
            (void)type;
            return nullptr;
        }

        virtual IRHIRenderSubsystem* TryGetRHISubsystem(RHISubsystemKind kind)
        {
            (void)kind;
            return nullptr;
        }

        virtual IFramebuffer* GetMainSceneFramebuffer(KongRenderModule& module)
        {
            (void)module;
            return nullptr;
        }

        virtual void OnSceneLightInfoUpdated(KongRenderModule& module, const SceneLightInfo& lightInfo)
        {
            (void)module;
            (void)lightInfo;
        }

        virtual void DrawMainScene(KongRenderModule& module, IRHICommandList* cmd)
        {
            (void)module;
            (void)cmd;
        }

        virtual void DrawSubsystemUI(KongRenderModule& module) { (void)module; }

        /** OpenGL 等：各子系统 ImGui 面板 */
        virtual bool HasSubsystemUI() const { return false; }

        virtual void DrawFallbackOpenGL(KongRenderModule& module, double delta)
        {
            (void)module;
            (void)delta;
        }

        virtual void DrawFallbackVulkan(KongRenderModule& module, double delta)
        {
            (void)module;
            (void)delta;
        }

        virtual void OnWindowResize(KongRenderModule& module, int width, int height)
        {
            (void)module;
            (void)width;
            (void)height;
        }

        virtual void SetRenderWater(KongRenderModule& module, const std::weak_ptr<AActor>& water)
        {
            (void)module;
            (void)water;
        }

        /** Vulkan：RenderModuleBackendVulkan::Init 末尾调用，vkBackend 为 RenderModuleBackendVulkan* */
        virtual void AfterVulkanDescriptorReady(KongRenderModule& module, void* vkBackendOpaque)
        {
            (void)module;
            (void)vkBackendOpaque;
        }

        virtual void OnVulkanReloadScene(KongRenderModule& module, void* descriptorPoolOpaque)
        {
            (void)module;
            (void)descriptorPoolOpaque;
        }

        /** OpenGL 主 FBO 名；非 OpenGL 返回 0 */
        virtual uint32_t GetMainFBONativeHandle() const { return 0; }

        virtual uint32_t GetMainColorTextureNativeHandle(unsigned index) const
        {
            (void)index;
            return 0;
        }

    protected:
        IRenderPassHost() = default;
    };
}
