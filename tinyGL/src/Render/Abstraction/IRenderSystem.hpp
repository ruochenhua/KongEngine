/**
 * @file IRenderSystem.hpp
 * @brief RHI 渲染系统统一接口，所有 Pass 实现此接口由 RenderModule 按序驱动。
 * @ingroup RenderAbstraction
 */

#pragma once

#include "Render/Abstraction/IFrameContext.hpp"
#include "Render/Abstraction/Types.hpp"

namespace Kong
{
    class IGraphicsDevice;

    /**
     * 与 API 无关的渲染系统接口。
     * 每个渲染 Pass（Shadow、Defer、Skybox、Postprocess 等）实现此类；
     * RenderModule 持有一组 IRenderSystem*，每帧按固定顺序调用 Draw。
     * 返回值：若需传出本 Pass 输出 RT，可通过 SceneDrawInfo 的 currentColorRT/currentDepthRT 更新，或返回 ITexture*（后续扩展）。
     */
    class IRenderSystem
    {
    public:
        virtual ~IRenderSystem() = default;

        /** 初始化，由 RenderModule 在启动时调用；device 为当前后端设备 */
        virtual void Init(IGraphicsDevice* device) = 0;

        /**
         * 执行本 Pass 的绘制。
         * @param frameContext 当前帧上下文
         * @param sceneDrawInfo 场景与当前 RT 信息，本 Pass 可更新 currentColorRT/currentDepthRT 供后续 Pass 使用
         */
        virtual void Draw(IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) = 0;

        IRenderSystem(const IRenderSystem&) = delete;
        IRenderSystem& operator=(const IRenderSystem&) = delete;

    protected:
        IRenderSystem() = default;
    };
}
