#pragma once

namespace Kong
{
    // 渲染器类
    class Renderer
    {
    public:
        Renderer() = default;
        virtual ~Renderer() = default;

        // 不支持复制和赋值
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
    private:
    };
}
