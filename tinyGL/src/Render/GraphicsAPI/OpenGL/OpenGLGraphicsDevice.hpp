#pragma once
#include "../GraphicsDevice.hpp"
#include "Render/Abstraction/Types.hpp"
#include "Render/Abstraction/IBuffer.hpp"
#include "Render/Abstraction/ITexture.hpp"
#include "Render/GraphicsAPI/OpenGL/GLBuffer.hpp"
#include "Render/GraphicsAPI/OpenGL/GLTexture.hpp"
#include "Render/GraphicsAPI/OpenGL/GLFrameContext.hpp"
#include "Render/Abstraction/IFrameContext.hpp"
#include <memory>

struct GLFWwindow;

namespace Kong
{
    class OpenGLGraphicsDevice : public GraphicsDevice
    {
    public:
        static OpenGLGraphicsDevice* GetGraphicsDevice();

        OpenGLGraphicsDevice();
        ~OpenGLGraphicsDevice() override;

        void* Init(int width, int height) override;
        std::unique_ptr<IBuffer> CreateBuffer(const BufferDesc& desc) override;
        std::unique_ptr<ITexture> CreateTexture(const TextureDesc& desc) override;
        BackendType GetBackendType() const override { return BackendType::OpenGL; }
        IFrameContext& BeginFrame() override;
        void EndFrame() override;

    private:
        GLFrameContext m_frameContext;
        GLFWwindow*    m_window {nullptr};
    };
}
