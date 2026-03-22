/**
 * @file RenderPassCatalog.hpp
 * @brief 渲染逻辑层 Pass 编目与默认拓扑（与 OpenGL/Vulkan 无关）。
 *
 * 仅定义标识与注释约定；实际注册顺序由 IRenderPassHost（OpenGL：OnAttached/RegisterDefaultRenderPasses，
 * Vulkan：AfterVulkanDescriptorReady）中 PushRenderSystem 决定，Vulkan Descriptor 由 RenderModuleBackendVulkan::Init 先行创建。
 * 应保持与本文件所述拓扑一致，便于双后端对齐。
 */

#pragma once

#include <cstdint>

namespace Kong
{
    /**
     * 逻辑渲染管线中的 Pass 标识。
     * 与 RenderSystemType（旧 OpenGL 枚举）解耦，供新 Pass 与文档引用。
     */
    enum class RenderLogicPassId : uint32_t
    {
        ShadowMaps = 0,          ///< 方向光/点光深度贴图
        DeferredGeometry,        ///< GBuffer：MRT 输出 albedo/normal/depth 等
        DeferredLighting,        ///< 全屏或体积：读 GBuffer，写 HDR/光照缓冲
        Skybox,                  ///< 天空盒（通常深度等于或特殊深度测试）
        ScreenSpaceReflection,   ///< 可选：SSR
        ForwardOverlay,          ///< 非延迟网格、透明、特殊材质
        Water,                   ///< 可选：水面 Pass
        PostProcess,             ///< 色调映射、Bloom、FXAA 等链式后处理
        ImGuiOrEditorUI,         ///< 调试与编辑器 UI（可仍在窗口层混合）
        Count
    };

    /**
     * 默认 Pass 拓扑（有向边：前者完成后后者可读其输出）。
     *
     * ShadowMaps → DeferredGeometry → DeferredLighting → Skybox → ForwardOverlay
     *   → [SSR] → [Water] → PostProcess → UI
     *
     * SSR/Water 可并行条件化插入在 DeferredLighting 之后、PostProcess 之前。
     */
    struct RenderPipelineTopologyDoc
    {};
}
