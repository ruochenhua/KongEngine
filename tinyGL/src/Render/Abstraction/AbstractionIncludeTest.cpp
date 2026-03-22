/**
 * @file AbstractionIncludeTest.cpp
 * @brief 仅用于验证 RHI 抽象层头文件可被编译，无实际逻辑。
 * @ingroup RenderAbstraction
 */

#include "Render/Abstraction/BackendType.hpp"
#include "Render/Abstraction/Types.hpp"
#include "Render/Abstraction/IBuffer.hpp"
#include "Render/Abstraction/ITexture.hpp"
#include "Render/Abstraction/IGraphicsDevice.hpp"
#include "Render/Abstraction/IFrameContext.hpp"
#include "Render/Abstraction/IRenderSystem.hpp"
#include "Render/Abstraction/IPipeline.hpp"
#include "Render/Abstraction/IRenderPass.hpp"
#include "Render/Abstraction/IFramebuffer.hpp"
#include "Render/Abstraction/IRHICommandList.hpp"
#include "Render/Abstraction/RHIRenderSubsystems.hpp"
#include "Render/Abstraction/ISampler.hpp"
#include "Render/Logic/RenderPassCatalog.hpp"

namespace Kong
{
    void AbstractionIncludeTest_UseTypes()
    {
        (void)BackendType::OpenGL;
        BufferDesc bd{};
        TextureDesc td{};
        SceneDrawInfo sdi{};
        SamplerDesc sd{};
        (void)bd;
        (void)td;
        (void)sdi;
        (void)sd;
        (void)RenderLogicPassId::PostProcess;
        (void)RHISubsystemKind::Deferred;
    }
}
