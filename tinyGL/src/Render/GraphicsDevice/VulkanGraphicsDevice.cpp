#include "VulkanGraphicsDevice.hpp"

using namespace Kong;

#if RENDER_IN_VULKAN

VulkanGraphicsDevice::VulkanGraphicsDevice()
{
    m_API = GraphicsAPI::VULKAN;
}

VulkanGraphicsDevice::~VulkanGraphicsDevice()
{
}

void VulkanGraphicsDevice::Init()
{
}

void VulkanGraphicsDevice::Destroy()
{
}

void VulkanGraphicsDevice::BeginFrame()
{
}

void VulkanGraphicsDevice::EndFrame()
{
}

#endif