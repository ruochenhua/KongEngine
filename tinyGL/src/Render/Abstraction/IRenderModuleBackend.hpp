/**
 * @file IRenderModuleBackend.hpp
 * @brief RenderModule 后端抽象：初始化、UBO/Descriptor 更新、Pass 注册与兼容路径绘制，无 GL/Vk 类型。
 * @ingroup RenderAbstraction
 */

#pragma once

#include "Render/Abstraction/BackendType.hpp"
#include <memory>

namespace Kong
{
    class IGraphicsDevice;
    class KongRenderModule;

    /**
     * 与 API 无关的 RenderModule 后端接口。
     * 负责：各 API 特有的资源初始化（如 Vulkan Descriptor/全局 UBO）、UpdateSceneRenderInfo、OnReloadScene、DrawFallback。
     * 渲染 Pass 的注册与 Gl*、Vk* 子系统实例由 KongRenderModule 统一管理。
     */
    class IRenderModuleBackend
    {
    public:
        virtual ~IRenderModuleBackend() = default;

        /** 初始化后端特有资源（不注册 Pass；Pass 由 KongRenderModule 注册） */
        virtual void Init(KongRenderModule* renderModule, IGraphicsDevice* device) = 0;

        /** 将 renderModule 的 scene_render_info / mainCamera 写入 UBO 或 Descriptor */
        virtual void UpdateSceneRenderInfo(KongRenderModule* renderModule) = 0;

        /** 场景重载后刷新后端资源（如 Vulkan descriptor set 重建） */
        virtual void OnReloadScene(KongRenderModule* renderModule) = 0;

        /** 兼容路径：无 frameContext 时执行整帧绘制（Shadow + 主场景 + 后处理 + UI） */
        virtual void DrawFallback(KongRenderModule* renderModule, double delta) = 0;

        /** 主场景绘制（延迟 + 非延迟 + 天空盒 + 可选 SSR），由后端实现；默认空实现 */
        virtual void DrawMainScene(KongRenderModule* renderModule) {}

        /** 各 RenderSystem 的 UI 绘制（如 ImGui 面板），默认空实现 */
        virtual void DrawUI(KongRenderModule* renderModule) { (void)renderModule; }

        /** 窗口尺寸变化时由 Module 转发，默认空实现 */
        virtual void OnWindowResize(int width, int height) { (void)width; (void)height; }

        /** 设置水体 Actor，默认空实现 */
        virtual void SetRenderWater(const std::weak_ptr<class AActor>& actor) { (void)actor; }

    protected:
        IRenderModuleBackend() = default;
    };

    /** 根据后端类型创建对应 Backend 实现；未编译的后端返回 nullptr */
    std::unique_ptr<IRenderModuleBackend> CreateRenderModuleBackend(BackendType type);
}
