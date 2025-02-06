#pragma once
#include "GraphicsDevice.hpp"

namespace Kong
{
    class VulkanGraphicsDevice : public GraphicsDevice
    {
        public:
        VulkanGraphicsDevice();
        virtual ~VulkanGraphicsDevice();

        void Init() override;
        void Destroy() override;
        void BeginFrame() override;
        void EndFrame() override;
    };
    
}
