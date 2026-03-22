/**
 * @file RHIRenderSubsystems.hpp
 * @brief 与图形 API 无关的渲染子系统标记接口：OpenGL 的 Gl* 与 Vulkan 的 Vk* 实现同一套查询，
 *        供逻辑层按 Pass 类型获取实例，逐步将绘制迁到 IRHICommandList。
 * @ingroup RenderAbstraction
 *
 * 子系统实例由 KongRenderModule 持有（阶段 1）；IRenderModuleBackend 仅负责 Descriptor/UBO 等后端能力。
 */

#pragma once

#include <cstdint>

namespace Kong
{
    /** 与 RenderPassCatalog / 模块内注册顺序对应的子系统类别 */
    enum class RHISubsystemKind : uint8_t
    {
        Deferred = 0,
        Skybox,
        PostProcess,
        ScreenSpaceReflection,
        Water,
    };

    /**
     * 标记「可通过 RHI 统一驱动」的渲染子系统；具体绘制仍可在实现内调用 gl/vk，直至完全迁入 IRHICommandList。
     */
    class IRHIRenderSubsystem
    {
    public:
        virtual ~IRHIRenderSubsystem() = default;
        virtual RHISubsystemKind GetRHISubsystemKind() const noexcept = 0;

    protected:
        IRHIRenderSubsystem() = default;
        IRHIRenderSubsystem(const IRHIRenderSubsystem&) = delete;
        IRHIRenderSubsystem& operator=(const IRHIRenderSubsystem&) = delete;
    };
}
