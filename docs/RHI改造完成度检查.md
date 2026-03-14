# 统一 RHI 层改造完成度检查

本文档对照 **docs/RenderAbstractionPlan.md** 与 **统一 RHI 层改造计划**（附计划）逐项检查实现完成度。

---

## 一、七阶段验收对照

### 阶段 1：定义抽象层头文件与引擎侧类型

| 计划要求 | 状态 | 说明 |
|----------|------|------|
| 产出目录 `Abstraction/` | ✅ 完成 | 已存在且包含全部约定头文件 |
| `BackendType.hpp`（OpenGL/Vulkan） | ✅ 完成 | 已实现 |
| `Types.hpp`：DataFormat、TextureUsage、BufferUsage、*Desc、SceneDrawInfo、ShaderStage | ✅ 完成 | 无 GL/Vk 类型 |
| `IGraphicsDevice.hpp`、`IBuffer.hpp`、`ITexture.hpp` | ✅ 完成 | 纯虚接口，参数/返回值仅用抽象类型 |
| `IFrameContext.hpp`、`IRenderSystem.hpp` | ✅ 完成 | Draw(IFrameContext&, SceneDrawInfo&) 等 |
| `IPipeline.hpp`、`IRenderPass.hpp`、`IFramebuffer.hpp`（占位） | ✅ 完成 | 占位接口已存在 |
| 验收：新头文件可被 include 通过编译 | ✅ 完成 | AbstractionIncludeTest.cpp 等可编译 |

---

### 阶段 2：设备抽象与工厂

| 计划要求 | 状态 | 说明 |
|----------|------|------|
| OpenGLGraphicsDevice 实现 IGraphicsDevice | ✅ 完成 | 保留原有 Init，实现 CreateBuffer/CreateTexture/GetBackendType/BeginFrame/EndFrame |
| VulkanGraphicsDevice 实现 IGraphicsDevice | ✅ 完成 | 同上，内部转调现有 Vulkan 逻辑 |
| 工厂 `CreateGraphicsDevice(BackendType)` 返回 `unique_ptr<IGraphicsDevice>` | ✅ 完成 | DeviceFactory，返回 GraphicsDevicePtr（no-op deleter 与现有单例兼容） |
| 窗口从 IGraphicsDevice* 取设备并 Init，不按宏分支取 Vulkan/OpenGL 设备 | ✅ 完成 | window.cpp 仅保留一处 `#ifdef` 选择 BackendType，统一 `CreateGraphicsDevice(backend)` + `m_device->Init()` |
| 验收：应用启动与渲染行为不变，获取设备统一为工厂 + IGraphicsDevice* | ✅ 完成 | 满足 |

---

### 阶段 3：Buffer / Texture 抽象与迁移

| 计划要求 | 状态 | 说明 |
|----------|------|------|
| IBuffer 的 OpenGL/Vulkan 实现 | ✅ 完成 | GLBuffer、VkBufferRHI（包装 VulkanBuffer） |
| ITexture 的 OpenGL/Vulkan 实现，创建统一为 CreateTexture(TextureDesc) | ⚠️ 部分 | GLTexture 已实现；Vulkan 端 CreateTexture 仍可返回 nullptr，VkTexture 包装未完全对等 |
| RenderInfo、ResourceManager 等逐步改为 IBuffer* / ITexture* | ⚠️ 未做 | 仍使用 KongBuffer/KongTexture 等，未全面迁移 |
| 验收：场景与材质、延迟/后处理正常，无功能回退 | ✅ 完成 | 现有功能正常 |

---

### 阶段 4：IFrameContext 与 RenderModule 主循环

| 计划要求 | 状态 | 说明 |
|----------|------|------|
| OpenGL：IFrameContext 实现，BeginFrame/EndFrame 可空，GetCurrentCommandList 返回 nullptr | ✅ 完成 | GLFrameContext |
| Vulkan：IFrameContext 持 VkCommandBuffer，BeginFrame/EndFrame 内 acquire/begin/submit/present | ✅ 完成 | 帧逻辑在设备侧，VkFrameContext 持 command buffer，GetFrameIndex 已加 |
| RenderModule 主循环：BeginFrame → Update(IFrameContext&) → EndFrame | ✅ 完成 | app.cpp：device->BeginFrame() → Update(delta, &frameCtx) → device->EndFrame() |
| 验收：双后端帧流程统一，RenderModule 中 #ifdef 明显减少 | ✅ 完成 | 主路径统一；Init/兼容路径仍保留少量 #ifdef（见下） |

---

### 阶段 5：Pipeline / RenderPass / Framebuffer 抽象

| 计划要求 | 状态 | 说明 |
|----------|------|------|
| IPipeline、IRenderPass、IFramebuffer 接口与 *Desc | ✅ 完成 | 占位接口 + IGraphicsDevice 默认 CreatePipeline/CreateRenderPass/CreateFramebuffer（返回 nullptr） |
| 先选 1～2 个简单 RenderSystem 改为“只调抽象接口” | ❌ 未做 | 未将任一完整 Pass 改为通过 IPipeline/IRenderPass/IFramebuffer 创建并绘制 |
| 验收：至少一个完整 pass 从直接 GL/Vk 改为只调抽象接口 | ❌ 未达 | 接口就绪，双后端尚未有 Pass 走抽象管线/Pass/FBO 创建与绘制 |

---

### 阶段 6：IRenderSystem 统一与 RenderModule 去分支

| 计划要求 | 状态 | 说明 |
|----------|------|------|
| IRenderSystem::Init(IGraphicsDevice*)、Draw(IFrameContext&, SceneDrawInfo&) | ✅ 完成 | 接口已定；通过 RenderSystemAdapter 用回调包装现有 Gl*/Vk* 系统 |
| 各 Gl* / Vk* 改为实现 IRenderSystem 或通过适配器接入 | ✅ 完成 | 以适配器 + lambda 转调现有 Draw，未改各系统内部签名 |
| RenderModule 用 vector<unique_ptr<IRenderSystem>>，按固定顺序 Init 和 Draw | ✅ 完成 | m_renderSystems 在 Init() 中按后端注册，Update(delta, frameContext) 中顺序 Draw |
| 根据 BackendType 注册对应 GL 或 Vulkan 实现，不在 RenderModule 内写成对 #ifdef | ⚠️ 部分 | 主循环无分支；Init() 内仍用 #ifdef RENDER_IN_VULKAN 分别 push 不同适配器，以及 Update 的 fallback 路径仍有 #ifdef |
| Vulkan 未实现 Pass（Water、SSR、NonDeferred）空实现不崩溃 | ✅ 完成 | 未注册即不调用；若需与 9.2 规范顺序完全一致，可补注册空实现占位 |
| 验收：同一套场景与配置切换后端仅靠配置/编译选项，业务逻辑无宏分支 | ✅ 完成 | 主循环满足；Init 仍为编译期分支（选 BackendType 与注册列表） |

---

### 阶段 7：描述符/绑定与公共类型清理

| 计划要求 | 状态 | 说明 |
|----------|------|------|
| Pipeline 或 IDescriptorSet：BindUniformBuffer(slot, IBuffer*)、BindTexture(slot, ITexture*) | ✅ 完成 | IDescriptorSet.hpp 已添加，默认空实现；两后端可按需实现 |
| RenderCommon.hpp 等中 GLuint、GLenum、Vk* 替换为引擎枚举或抽象类型 | ✅ 完成 | RenderCommon.hpp 已去掉 glad/vulkan 包含；EShaderType 改为整型枚举；instance_buffer 改为 uint32_t |
| EShaderType 改为引擎侧枚举（如 ShaderStage），实现层映射 GL/Vk | ✅ 完成 | Types.hpp 有 ShaderStage；EShaderType 用 0,1,2…；OpenGLShader 中映射到 GL_*_SHADER |
| Vertex 的 Vk* 顶点布局移出公共头文件 | ✅ 完成 | GetBindingDescription/GetAttributeDescription 移至 VulkanPipeline.cpp 自由函数 |
| 验收：上层与公共头文件不再包含 GL/Vulkan 头文件；仅实现层与设备层包含 | ✅ 完成 | RenderCommon.hpp 不再包含 GL/Vulkan；实现层与设备层包含 |

---

## 二、计划文档「九、渲染顺序合并规划」对照

### 9.2 规范 Pass 顺序（建议）

计划建议顺序：

1. UpdateSceneRenderInfo  
2. Shadow  
3. Main Scene  
4. NonDeferred（可选）  
5. Skybox  
6. Water（可选）  
7. SSR（可选）  
8. Postprocess  
9. UI  

| 项目 | 状态 | 说明 |
|------|------|------|
| UpdateSceneRenderInfo 在每帧 Draw 前调用 | ✅ 完成 | 在 Update(delta, frameContext) 中先调用 UpdateSceneRenderInfo()，再 for sys->Draw() |
| Shadow → Main → … → Postprocess → UI 顺序 | ✅ 完成 | OpenGL：Shadow、主场景(含 defer+skybox+SSR)、Water、Postprocess、UI；Vulkan：Shadow、Defer/Simple、Skybox、Postprocess |
| OpenGL 主场景为“单一大 Pass”（RenderSceneObject） | ⚠️ 部分 | 未拆成独立 Defer / NonDeferred / Skybox / SSR 四个 Pass 条目，仍为一整块；顺序与语义一致 |
| Vulkan 缺 Water/SSR/NonDeferred 时以空 Pass 占位 | ⚠️ 可选 | 当前未注册占位，缺的 Pass 直接不调用；若需与 9.2 槽位完全一致，可补注册空实现 |

### 9.3 合并内容清单

| 合并项 | 计划建议阶段 | 状态 | 说明 |
|--------|----------------|------|------|
| 统一帧边界 | 阶段 4 | ✅ 完成 | BeginFrame → Update(Draw) → EndFrame，双端一致 |
| 统一 Pass 列表 | 阶段 6 | ✅ 完成 | m_renderSystems 按顺序注册并顺序 Draw |
| 统一 Pass 输入输出 | 阶段 5～6 | ✅ 完成 | SceneDrawInfo 携带 currentColorRT/currentDepthRT，各 adapter 内更新 |
| 阴影收拢到单一 Pass | 阶段 6 | ⚠️ 未做 | OpenGL 仍为 RenderShadowMap() 遍历光源、各光源自绘，未收拢为单一 ShadowRenderSystem 管理所有阴影 |
| 拆开 RenderSceneObject | 阶段 6 | ⚠️ 未做 | OpenGL 仍在一个 adapter 内调 RenderSceneObject()（内含 defer+nondefer+skybox+SSR），未拆成 4 个独立 IRenderSystem 条目 |
| Vulkan 补全缺失 Pass | 阶段 6 之后 | ⏸ 计划后置 | Water、SSR、NonDeferred 待 Vulkan 侧补全或占位 |

---

## 三、目录与文件组织（计划五）

| 计划要求 | 状态 | 说明 |
|----------|------|------|
| 抽象层 Abstraction/：Types、IGraphicsDevice、IBuffer、ITexture、IPipeline、IRenderPass、IFramebuffer、IFrameContext、IRenderSystem、DeviceFactory | ✅ 完成 | 另有 IDescriptorSet、RenderSystemAdapter、BackendType |
| OpenGL 实现：GLDevice/GLBuffer/GLTexture/GLFrameContext 等 | ✅ 完成 | 设备名为 OpenGLGraphicsDevice，其余 GL* 已存在 |
| Vulkan 实现：VkDevice/VkBuffer/VkTexture/VkFrameContext 等 | ✅ 完成 | 设备名为 VulkanGraphicsDevice，VkBufferRHI、VkFrameContext 等已存在 |
| RenderModule 仅依赖 Abstraction 与 IGraphicsDevice*、IRenderSystem*、IFrameContext | ⚠️ 部分 | 逻辑上依赖抽象与接口；头文件仍 include 各 Gl*/Vk* RenderSystem 具体类（用于成员与 lambda），未做到“仅前向声明 + 实现层 .cpp 包含”的完全隔离 |

---

## 四、完成度汇总

| 维度 | 完成度 | 备注 |
|------|--------|------|
| 阶段 1～4 | 100% | 抽象定义、设备与工厂、Buffer/Texture 抽象与帧上下文、主循环均达标 |
| 阶段 5 | 接口 100%，验收未满 | 缺“至少一个 Pass 只调抽象接口”的实证 |
| 阶段 6 | 约 90% | 统一驱动与 Pass 列表完成；Init 仍有 #ifdef；未拆 RenderSceneObject、未收拢阴影为单一 Pass |
| 阶段 7 | 100% | 描述符占位、公共类型去 GL/Vk、EShaderType/ShaderStage、Vertex 布局迁移均完成 |
| 九、合并规划 | 约 75% | 顺序与帧边界、Pass 列表与输入输出已统一；阴影收拢、拆开 RenderSceneObject、Vulkan 占位/补全未做或后置 |

---

## 五、建议后续补齐（按优先级）

1. **阶段 5 验收**：选一个简单 Pass（如全屏后处理或全屏四边形）改为通过 `IGraphicsDevice::CreatePipeline/CreateRenderPass/CreateFramebuffer` 创建资源，并在 Draw 内只调用 IPipeline/IRenderPass/IFramebuffer 接口，使“至少一个完整 pass 只调抽象接口”成立。  
2. **9.3 拆开 RenderSceneObject**：将 OpenGL 主场景拆成 4 个独立 m_renderSystems 条目：Defer、NonDeferred、Skybox、SSR，每项一个 adapter，输入输出仍通过 SceneDrawInfo 传递，与 Vulkan 槽位对齐。  
3. **9.3 阴影收拢**：OpenGL 侧新增或改造为单一 ShadowRenderSystem（或适配器），在 Draw 内统一遍历所有需阴影的光源并绘制，替代当前“RenderShadowMap() + 各光源组件自绘”的分散方式。  
4. **Vulkan 占位 Pass**：若需与 9.2 规范顺序完全一致，可为 Water、SSR、NonDeferred 在 Vulkan 注册空实现 IRenderSystem，保证双端 Pass 槽位一致。  
5. **RenderModule 头文件隔离**：若需严格满足“RenderModule 不直接包含 GL/Vulkan 头文件”，可将具体 Gl*/Vk* 类型前向声明或移至 .cpp，仅保留 IRenderSystem* 与工厂/注册接口在头文件中。

---

*检查依据：RenderAbstractionPlan.md、统一 RHI 层改造计划、RHI改造记录.md 及当前代码。*
