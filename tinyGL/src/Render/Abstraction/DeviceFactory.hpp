/**
 * @file DeviceFactory.hpp
 * @brief RHI 设备工厂，根据后端类型创建 IGraphicsDevice。
 * @ingroup RenderAbstraction
 */

#pragma once

#include "Render/Abstraction/BackendType.hpp"
#include "Render/Abstraction/IGraphicsDevice.hpp"
#include <memory>

namespace Kong
{
    /**
     * 根据当前编译或配置创建图形设备。
     * 编译期单后端时，仅与 type 匹配的后端会返回非空；未编译的后端返回 nullptr。
     * 返回的 unique_ptr 使用 no-op deleter（不拥有设备生命周期，与现有单例兼容）。
     */
    using GraphicsDevicePtr = std::unique_ptr<IGraphicsDevice, void(*)(IGraphicsDevice*)>;
    GraphicsDevicePtr CreateGraphicsDevice(BackendType type);
}
