/**
 * @file IGraphicsDevice.hpp
 * @brief RHI 图形设备接口，负责初始化与资源创建。
 * @ingroup RenderAbstraction
 */

#pragma once

#include "Render/Abstraction/BackendType.hpp"
#include "Render/Abstraction/Types.hpp"
#include "Render/Abstraction/IBuffer.hpp"
#include "Render/Abstraction/ITexture.hpp"
#include "Render/Abstraction/IPipeline.hpp"
#include "Render/Abstraction/IRenderPass.hpp"
#include "Render/Abstraction/IFramebuffer.hpp"
#include "Render/Abstraction/ISampler.hpp"
#include "Render/Abstraction/IRenderPassHost.hpp"

#include <memory>

namespace Kong
{
    class IFrameContext;

    /**
     * 与 API 无关的图形设备接口。
     * 职责：初始化窗口/上下文、创建 Buffer/Texture（及后续 Pipeline/RenderPass/Framebuffer）。
     * 线程安全：Init 与资源创建通常仅在主线程调用。
     */
    class IGraphicsDevice
    {
    public:
        virtual ~IGraphicsDevice() = default;

        /**
         * 初始化图形设备与窗口。
         * @return 窗口句柄（如 GLFWwindow*），由上层转交 GLFW 或其它窗口管理。
         */
        virtual void* Init(int width, int height) = 0;

        /** 创建缓冲，返回抽象缓冲对象；失败可返回 nullptr */
        virtual std::unique_ptr<IBuffer> CreateBuffer(const BufferDesc& desc) = 0;

        /** 创建纹理，返回抽象纹理对象；失败可返回 nullptr */
        virtual std::unique_ptr<ITexture> CreateTexture(const TextureDesc& desc) = 0;

        /** 创建采样器；未实现时返回 nullptr（可用纹理内嵌采样状态代替） */
        virtual std::unique_ptr<ISampler> CreateSampler(const SamplerDesc& desc) { (void)desc; return nullptr; }

        /** 返回当前后端类型 */
        virtual BackendType GetBackendType() const = 0;

        /** 开始本帧，返回本帧的提交上下文；须在 EndFrame 前完成本帧所有绘制 */
        virtual IFrameContext& BeginFrame() = 0;

        /** 结束本帧并提交/呈现 */
        virtual void EndFrame() = 0;

        /** 等待设备空闲（如析构前等待 GPU 完成）。默认空实现；Vulkan 实现中调用 vkDeviceWaitIdle。 */
        virtual void WaitIdle() {}

        /** 创建管线（阶段 5，默认返回 nullptr，由后端实现） */
        virtual std::unique_ptr<IPipeline> CreatePipeline(const PipelineDesc& desc) { (void)desc; return nullptr; }
        /** 创建渲染通道（阶段 5，默认返回 nullptr） */
        virtual std::unique_ptr<IRenderPass> CreateRenderPass(const RenderPassDesc& desc) { (void)desc; return nullptr; }
        /** 创建帧缓冲（阶段 5，默认返回 nullptr） */
        virtual std::unique_ptr<IFramebuffer> CreateFramebuffer(const RenderPassDesc& desc, ITexture* color, ITexture* depth) { (void)desc; (void)color; (void)depth; return nullptr; }

        /** 创建当前后端的 Pass 宿主（子系统 + 拓扑注册）；默认 nullptr */
        virtual std::unique_ptr<IRenderPassHost> CreateRenderPassHost() { return nullptr; }

        IGraphicsDevice(const IGraphicsDevice&) = delete;
        IGraphicsDevice& operator=(const IGraphicsDevice&) = delete;

    protected:
        IGraphicsDevice() = default;
    };
}
