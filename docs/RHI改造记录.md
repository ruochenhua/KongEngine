# 统一 RHI 层改造记录

本文档记录按 `docs/RenderAbstractionPlan.md` 七阶段设计完成的 RHI 抽象层改造过程与结果。

---

## 改造目标与约束

- **目标**：构造与 API 无关的 RHI（Rendering Hardware Interface）层，上层只依赖抽象接口，通过配置或编译选择 OpenGL/Vulkan 后端。
- **约束**：Vulkan 暂不支持的功能（水体、SSR、独立非延迟 Pass 等）在对应 Pass 中空实现或跳过，不崩溃；代码符合项目风格并补充注释。

---

## 阶段 1：定义抽象层头文件与引擎侧类型

- **产出**：`tinyGL/src/Render/Abstraction/` 目录。
- **新增**：
  - `BackendType.hpp`：`enum class BackendType { OpenGL, Vulkan }`。
  - `Types.hpp`：`DataFormat`、`TextureUsage`、`BufferUsage`、`TextureDesc`、`BufferDesc`、`PipelineDesc`、`RenderPassDesc`、`SceneDrawInfo`、`ShaderStage` 等，无 GL/Vk 类型。
  - `IGraphicsDevice.hpp`、`IBuffer.hpp`、`ITexture.hpp`、`IFrameContext.hpp`、`IRenderSystem.hpp`、`IPipeline.hpp`、`IRenderPass.hpp`、`IFramebuffer.hpp`（占位）。
- **验收**：新头文件可被 include 通过编译，不改变现有行为。

---

## 阶段 2：设备抽象与工厂

- **实现**：`OpenGLGraphicsDevice`、`VulkanGraphicsDevice` 继承并实现 `IGraphicsDevice`；`DeviceFactory::CreateGraphicsDevice(BackendType)` 返回 `GraphicsDevicePtr`；`window.cpp` 通过工厂获取设备并调用 `Init`，仅保留一处 `#ifdef` 用于选择 BackendType。
- **验收**：应用启动与现有渲染行为不变，设备获取统一为工厂 + `IGraphicsDevice*`。

---

## 阶段 3：Buffer / Texture 抽象与迁移

- **实现**：`GLBuffer`、`GLTexture` 实现 `IBuffer`/`ITexture`；`VkBufferRHI` 包装现有 `VulkanBuffer` 实现 `IBuffer`；设备 `CreateBuffer`/`CreateTexture` 返回抽象类型。
- **迁移**：`RenderInfo`、`ResourceManager` 等仍可继续使用原有 Kong* 类型，逐步迁移；部分路径已使用抽象接口。
- **验收**：场景与材质加载、延迟/后处理等仍正常。

---

## 阶段 4：IFrameContext 与 RenderModule 主循环

- **实现**：`GLFrameContext`（BeginFrame/EndFrame 空实现，GetCurrentCommandList 返回 nullptr）；`VkFrameContext` 持有当前帧 `VkCommandBuffer`，`GetCurrentCommandList()` 返回该 buffer；设备负责 `BeginFrame()`/`EndFrame()`，Vulkan 帧状态（swap chain、command buffer、frame index）委托给设备。
- **主循环**：`app.cpp` 统一为 `device->BeginFrame()` → `m_RenderModule.Update(delta, &frameCtx)` → `device->EndFrame()`。
- **验收**：双后端帧流程统一；RenderModule 内与“当前是 GL 还是 Vulkan”相关的分支减少。

---

## 阶段 5：Pipeline / RenderPass / Framebuffer 抽象

- **实现**：`IGraphicsDevice` 增加默认实现（返回 nullptr）的 `CreatePipeline(PipelineDesc)`、`CreateRenderPass(RenderPassDesc)`、`CreateFramebuffer(RenderPassDesc, ITexture*, ITexture*)`；`IPipeline`、`IRenderPass`、`IFramebuffer` 占位接口已存在。
- **验收**：接口就绪，后端可按需实现；至少一个 Pass 走抽象接口可在后续迭代中完成。

---

## 阶段 6：IRenderSystem 统一与 RenderModule 去分支

- **实现**：
  - `RenderSystemAdapter`（`Abstraction/RenderSystemAdapter.hpp`）：类型擦除适配器，`Draw(IFrameContext&, SceneDrawInfo&)` 委托给外部回调。
  - `RenderModule` 持有 `std::vector<std::unique_ptr<IRenderSystem>> m_renderSystems`，在 `Init()` 中按后端注册适配器（OpenGL：Shadow、主场景、Water、后处理、UI；Vulkan：Shadow、Defer/Simple、Skybox、Postprocess）。
  - `Update(double delta, IFrameContext* frameContext)`：当 `frameContext != nullptr` 且 `m_renderSystems` 非空时，构建 `SceneDrawInfo` 并顺序调用各 `sys->Draw(*frameContext, sceneDrawInfo)`；否则走兼容旧路径。
  - `IFrameContext` 增加 `GetFrameIndex()`，OpenGL 返回 0，Vulkan 返回当前帧索引。
- **验收**：同一套主循环下，切换 OpenGL/Vulkan 仅通过配置或编译选项；Vulkan 缺失功能以空 Pass 不崩溃方式处理。

---

## 阶段 7：描述符/绑定与公共类型清理

- **描述符**：新增 `IDescriptorSet.hpp`，提供 `BindUniformBuffer(slot, IBuffer*)`、`BindTexture(slot, ITexture*)` 的默认空实现，供后端按需实现。
- **公共类型清理**：
  - `RenderCommon.hpp` 不再包含 `glad/glad.h` 与 `<vulkan/vulkan_core.h>`。
  - `EShaderType` 改为引擎侧整型枚举（vs=0, fs=1, gs=2, cs=3, tcs=4, tes=5），实现层映射：OpenGL 在 `OpenGLShader.cpp` 中映射到 `GL_*_SHADER`；Vulkan 仍用枚举作 map 键，阶段位由代码写死。
  - `RenderInfo::instance_buffer` 改为 `uint32_t`；OpenGL 使用处以 `!= 0` 判断有效。
  - `Vertex` 的 `GetBindingDescription`/`GetAttributeDescription` 从 `RenderCommon.hpp` 移除，改为 Vulkan 实现层内自由函数 `GetVertexBindingDescription()`/`GetVertexAttributeDescription()`（在 `VulkanPipeline.cpp` 中），避免公共头文件出现 Vk* 类型。
  - `Types.hpp` 增加引擎侧 `ShaderStage` 枚举，与 `EShaderType` 数值对应，供后续统一使用。
- **验收**：上层与公共头文件不再直接包含 GL/Vulkan 头文件；仅实现层与设备层包含。

---

## 目录与文件组织（改造后）

| 位置 | 内容 |
|------|------|
| `tinyGL/src/Render/Abstraction/` | BackendType, Types, IGraphicsDevice, IBuffer, ITexture, IFrameContext, IRenderSystem, IPipeline, IRenderPass, IFramebuffer, IDescriptorSet, DeviceFactory, RenderSystemAdapter |
| `tinyGL/src/Render/GraphicsAPI/OpenGL/` | OpenGLGraphicsDevice, GLBuffer, GLTexture, GLFrameContext, 各 Gl* RenderSystem |
| `tinyGL/src/Render/GraphicsAPI/Vulkan/` | VulkanGraphicsDevice, VkFrameContext, VkBufferRHI, VulkanPipeline（含顶点布局自由函数）, 各 Vk* RenderSystem |
| `tinyGL/src/Render/RenderModule.*` | 持有 `m_renderSystems`，`Update(delta, frameContext)` 统一驱动；仅依赖 Abstraction 与具体系统头文件 |

---

## 与计划文档的对应

- 实施过程对应 `docs/RenderAbstractionPlan.md` 的七阶段；各阶段验收要点已在上述各节记录。
- `docs/INDEX.md` 中「3.5 图形 API 抽象」已更新为 RHI 层目标、Abstraction 目录、双后端实现职责及 RenderModule 只依赖抽象层的说明。

---

## 风险与后续注意点

1. **OpenGL 隐式状态**：抽象层在每帧/每次 Draw 中应显式设置管线、FBO、绑定等，避免状态泄漏。
2. **Shader 与格式**：顶点属性、uniform/descriptor 布局在两后端保持一致；建议固定一套 layout（如 std140）。
3. **性能**：关键路径注意虚调用与状态切换成本，接口稳定后可考虑内联或模板。
4. **增量迁移**：RenderInfo、ResourceManager 等仍可逐步从 Kong* 迁到 IBuffer*/ITexture*；未实现 Vulkan 的 Pass 可后续补全，接口已统一。

---

*改造完成日期与项目当前状态一致，后续若扩展 Dx12 或进一步统一描述符/管线创建，可在此文档或 RenderAbstractionPlan.md 中延续记录。*
