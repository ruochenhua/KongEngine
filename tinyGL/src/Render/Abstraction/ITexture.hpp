/**
 * @file ITexture.hpp
 * @brief RHI 纹理接口，双后端统一抽象。
 * @ingroup RenderAbstraction
 */

#pragma once

#include <cstdint>

namespace Kong
{
    /**
     * 与 API 无关的纹理接口。
     * - OpenGL: 对应纹理对象，Bind(slot) 即 glBindTextureUnit。
     * - Vulkan: 对应 VkImage + ImageView + Sampler，Bind 在 descriptor 侧体现。
     * TransitionLayout: Vulkan 需要显式转换，OpenGL 可空实现。
     */
    class ITexture
    {
    public:
        virtual ~ITexture() = default;

        virtual int  GetWidth()  const = 0;
        virtual int  GetHeight() const = 0;
        virtual int  GetDepth()  const { return 1; }

        /** 绑定到纹理槽位，commandList 为 GetCurrentCommandList() 或 nullptr（GL） */
        virtual void Bind(uint32_t slot, void* commandList = nullptr) = 0;

        /**
         * 布局转换（Vulkan 必须，OpenGL 可空实现）。
         * oldLayout/newLayout 为实现层枚举或 0 表示“当前/目标”，由实现定义。
         */
        virtual void TransitionLayout(void* commandList, uint32_t oldLayout, uint32_t newLayout, int arrayLayer = 0) { (void)commandList; (void)oldLayout; (void)newLayout; (void)arrayLayer; }

        ITexture(const ITexture&) = delete;
        ITexture& operator=(const ITexture&) = delete;

    protected:
        ITexture() = default;
    };
}
