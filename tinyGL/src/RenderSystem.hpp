#pragma once

namespace Kong
{
    class KongRenderModule;

    // 渲染系统，每个渲染效果或者阶段都独立出来
    class KongRenderSystem
    {
    public:
        KongRenderSystem() = default;
        virtual ~KongRenderSystem() = default;

        // 禁止system之间的复制操作
        KongRenderSystem(const KongRenderSystem&) = delete;
        KongRenderSystem& operator=(const KongRenderSystem&) = delete;
        
        virtual void Draw(double delta, KongRenderModule* render_module) = 0;
        virtual void Init() = 0;
    };
}
