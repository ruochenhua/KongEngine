/**
 * @file GLTexture.hpp
 * @brief OpenGL 实现的 ITexture，RHI 纹理接口。
 * @ingroup RenderAbstraction
 */

#pragma once

#include "Render/Abstraction/ITexture.hpp"
#include "Render/Abstraction/Types.hpp"
#include <cstdint>

namespace Kong
{
    /** OpenGL 后端 ITexture 实现 */
    class GLTexture : public ITexture
    {
    public:
        explicit GLTexture(const TextureDesc& desc);
        ~GLTexture() override;

        int  GetWidth()  const override { return m_width; }
        int  GetHeight() const override { return m_height; }
        int  GetDepth()  const override { return m_depth; }
        void Bind(uint32_t slot, void* commandList = nullptr) override;

    private:
        unsigned int m_texId {0};
        int          m_width {1};
        int          m_height {1};
        int          m_depth {1};
    };
}
