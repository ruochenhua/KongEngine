/**
 * @file RenderSystemAdapter.hpp
 * @brief 类型擦除的 IRenderSystem 适配器，用回调实现 Draw，供 RenderModule 统一驱动 GL/Vk 具体系统。
 * @ingroup RenderAbstraction
 */

#pragma once

#include "Render/Abstraction/IRenderSystem.hpp"
#include <functional>

namespace Kong
{
    /**
     * 实现 IRenderSystem，将 Draw 委托给外部回调。
     * RenderModule 按 Pass 顺序注册多个 Adapter，每个回调内部转调现有 Gl* / Vk* 系统。
     */
    class RenderSystemAdapter : public IRenderSystem
    {
    public:
        using DrawFn = std::function<void(IFrameContext&, SceneDrawInfo&)>;

        explicit RenderSystemAdapter(DrawFn drawFn)
            : m_drawFn(std::move(drawFn))
        {}

        void Init(IGraphicsDevice* device) override { (void)device; }

        void Draw(IFrameContext& frameContext, SceneDrawInfo& sceneDrawInfo) override
        {
            if (m_drawFn)
                m_drawFn(frameContext, sceneDrawInfo);
        }

    private:
        DrawFn m_drawFn;
    };
}
