# KongEngine 统一渲染接口架构与实施计划

目标：用**一套与 API 无关的渲染接口**对接 OpenGL 与 Vulkan，上层（RenderModule、场景、组件）只依赖抽象层，通过配置或编译选择后端，不再在业务代码里写 `#ifdef RENDER_IN_VULKAN`。

---

## 一、现状简要分析

| 层次 | 现状 | 问题 |
|------|------|------|
| **设备** | `GraphicsDevice` 仅声明 `Init()`，返回 `GLFWwindow*`；OpenGL 几乎无其它接口，Vulkan 暴露 `GetDevice()`、`CreateBuffer`、`CreateImageWithInfo`、`BeginSingleTimeCommands` 等 | 无统一设备抽象，各处直接调 `OpenGLGraphicsDevice::GetGraphicsDevice()` 或 `VulkanGraphicsDevice::GetGraphicsDevice()` |
| **缓冲** | `KongBuffer` 有基类 + `Initialize/Bind`，但 OpenGL 用 `OpenGLBuffer`+顶点属性，Vulkan 用 `VulkanBuffer`+Map/Flush | 创建与使用方式不统一，Vulkan 路径大量直接用 `VulkanGraphicsDevice` 创建 buffer |
| **纹理** | `KongTexture` 基类 + `OpenGLTexture` / `VulkanTexture` | 创建参数不统一：`TextureCreateInfo` 用 GL 枚举，Vulkan 用 `VkImageCreateInfo`；`Bind(location)` 语义在 Vulkan 下对应 descriptor，未抽象 |
| **管线/渲染通道** | 无公共抽象 | OpenGL 用 shader program + FBO；Vulkan 用 `VkPipeline`、`VkRenderPass`、`VkFramebuffer`，全部在各自 RenderSystem 内部 |
| **渲染系统** | `OpenGLRenderSystem::Draw(delta, RenderResultInfo, KongRenderModule*)` vs `VulkanRenderSystem::Draw(FrameInfo)`，参数与返回值类型完全不同 | RenderModule 内大量 `#ifdef` 分支，无法用同一指针数组驱动两套系统 |
| **公共类型** | `RenderResultInfo` 用 `GLuint`；`EShaderType` 用 `GL_VERTEX_SHADER` 等；`Vertex` 在 Vulkan 下带 `VkVertexInput*Description` | 公共头文件掺杂 GL/Vk 类型，无法做到“只包含抽象头文件即可编译任一端” |

结论：要达成“一套接口、双后端”，需要从**设备 + 资源 + 帧/命令**到**管线与 RenderSystem 接口**做分层抽象，并引入**与 API 无关的描述符（Format、Usage、Desc 等）**。

---

## 二、目标架构：分层与职责

```
┌─────────────────────────────────────────────────────────────────┐
│  RenderModule / Scene / Components（只依赖抽象接口，无 #ifdef）   │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│  统一渲染接口层（IRenderSystem, IFrameContext, IDevice, I*）     │
│  + 引擎侧描述符（TextureDesc, BufferDesc, PipelineDesc 等）       │
└─────────────────────────────────────────────────────────────────┘
                                  │
              ┌───────────────────┴───────────────────┐
              ▼                                       ▼
┌─────────────────────────────┐         ┌─────────────────────────────┐
│  OpenGL 实现层               │         │  Vulkan 实现层               │
│  OpenGLDevice, GLBuffer,     │         │  VulkanDevice, VkBuffer,      │
│  GLTexture, GLRenderSystem…  │         │  VkTexture, VkRenderSystem…   │
└─────────────────────────────┘         └─────────────────────────────┘
```

- **上层**：只看到 `IGraphicsDevice`、`IBuffer`、`ITexture`、`IPipeline`、`IRenderPass`、`IFrameContext`、`IRenderSystem` 等接口，以及引擎自己的 `*Desc`、`DataFormat`、`TextureUsage` 等枚举/结构。
- **实现层**：每个后端在单独目录中实现上述接口，内部可随意使用 GL/Vk 类型，不暴露给上层。

---

## 三、核心抽象与 API 无关类型

### 3.1 设备与工厂

- **`IGraphicsDevice`**（替代当前 `GraphicsDevice` 仅有的 `Init`）
  - `Init(width, height) -> void*`（窗口句柄仍可由上层转给 GLFW，或再包一层 `IWindow`）
  - `CreateBuffer(BufferDesc) -> IBuffer*`
  - `CreateTexture(TextureDesc) -> ITexture*`
  - `CreatePipeline(PipelineDesc) -> IPipeline*`（或由 Pipeline 工厂持有 Device 引用）
  - Vulkan 特有：`BeginSingleTimeCommands() / EndSingleTimeCommands()` 可放在 `IFrameContext` 或保留在 `IVulkanDevice` 扩展接口中，由 Vulkan 实现内部使用。
- **后端选择**：编译期（宏/模板）或运行期（配置 + 工厂函数），例如 `CreateGraphicsDevice(Backend::OpenGL)` / `CreateGraphicsDevice(Backend::Vulkan)` 返回 `std::unique_ptr<IGraphicsDevice>`。

### 3.2 资源描述（无 GL/Vk 类型）

- **`BufferDesc`**：usage（Vertex / Index / Uniform / Staging）、size、instanceCount、optional initialData。
- **`TextureDesc`**：width, height, depth/layers；format（引擎枚举，如 `R8G8B8A8_UNORM`、`R32G32B32A32_SFLOAT`、`D24_UNORM_S8_UINT`）；usage（ColorAttachment、DepthStencil、Sampled、TransferDst 等）；filter/wrap 用引擎枚举。
- **`DataFormat`**：枚举所有用到的像素/顶点格式，在实现层映射到 `GL_*` / `VkFormat`。

### 3.3 资源接口

- **`IBuffer`**：`Upload(data, size)`、`Bind(slot)`（或由 Pipeline/Descriptor 层统一 Bind）；Vulkan 实现可保留 `Map/Unmap/Flush`，通过接口暴露或仅内部使用。
- **`ITexture`**：`GetWidth/Height`、`Bind(slot)`、可选 `TransitionLayout`（Vulkan 需要，OpenGL 可空实现）；创建统一走 `IGraphicsDevice::CreateTexture(TextureDesc)`。

### 3.4 帧与命令提交（统一“一帧”的语义）

- **`IFrameContext`**（或叫 `ICommandContext`）
  - 语义：表示“当前帧的提交上下文”。OpenGL 下可以是轻量封装（无操作或只记状态）；Vulkan 下对应当前帧的 `VkCommandBuffer`。
  - 接口建议：`BeginFrame()`、`EndFrame()`、`GetCurrentCommandList()`（返回不透明指针，Vulkan 即 command buffer，GL 可为 nullptr）。
  - 可选：`SubmitSingleTime(callback)`，在 Vulkan 下封装 `BeginSingleTimeCommands` / 执行 callback / `EndSingleTimeCommands`，在 OpenGL 下直接执行 callback。
- **RenderModule** 只调用 `device->BeginFrame()` → 各系统 `Draw(frameContext)` → `device->EndFrame()`，不再根据宏分支“是 Vulkan 就 BeginFrame/EndFrame，否则 SwapBuffers”。

### 3.5 管线与渲染通道

- **`IPipeline`**：由 `PipelineDesc` 创建（shader 路径或字节码 + 顶点布局 + 混合/深度等状态）；`Bind(IFrameContext*)`；不暴露 GL program 或 VkPipeline。
- **`IRenderPass`**：描述一次渲染通道的附件与 load/store；OpenGL 对应“绑定 FBO + 设置 clear”等逻辑，Vulkan 对应 `VkRenderPass`。
- **`IFramebuffer`**：可渲染目标集合（颜色/深度等）。OpenGL 即 FBO；Vulkan 即 `VkFramebuffer`。创建可由 Device 或 RenderPass 工厂完成。

这样，延迟、后处理、天空盒、阴影等都可以用“一个 IRenderPass + 一个/多个 IPipeline + IFramebuffer”描述，具体由后端实现。

### 3.6 描述符 / 绑定（Uniform、纹理槽）

- 抽象为 **“绑定槽”**：例如 `IPipeline::BindUniformBuffer(slot, IBuffer*)`、`BindTexture(slot, ITexture*)`。OpenGL：`glBindBufferBase` + `glBindTextureUnit`；Vulkan：更新 descriptor set 后 `vkCmdBindDescriptorSets`。可引入轻量 `IDescriptorSet` 或直接在 Pipeline 上提供 `Bind*`，由实现层封装 GL/Vk 细节。

### 3.7 渲染系统统一接口

- **`IRenderSystem`**
  - `Init(IGraphicsDevice*, ...)` 或由 RenderModule 注入 Device。
  - `Draw(IFrameContext& frameContext, const SceneDrawInfo& sceneInfo)`（或类似参数：相机、光源、需要绘制的物体列表等）。
  - 返回值如需“本 pass 输出到哪张纹理/ framebuffer”，用 `ITexture*` / `IFramebuffer*` 或引擎内 ID，不再用 `GLuint`。
- **RenderModule**：持有 `std::vector<std::unique_ptr<IRenderSystem>>` 和 `IGraphicsDevice*`，按固定顺序调用各系统的 `Draw(frameContext, sceneInfo)`，不再按宏区分 Gl* / Vk*。

---

## 四、实施阶段与顺序

建议按阶段推进，每阶段保持可编译、可运行，优先保证 OpenGL 行为不变，再在 Vulkan 上对齐。

### 阶段 1：定义抽象层头文件与引擎侧类型（不删现有代码）

- **产出**：新目录如 `tinyGL/src/Render/Abstraction/`（或 `Core/`）。
- **内容**：
  - `BackendType.hpp`：`enum class BackendType { OpenGL, Vulkan }`。
  - `Types.hpp`：`DataFormat`、`TextureUsage`、`BufferUsage`、`TextureDesc`、`BufferDesc`、`PipelineDesc`（仅字段，不依赖 GL/Vk）。
  - `IGraphicsDevice.hpp`、`IBuffer.hpp`、`ITexture.hpp`：纯虚接口，方法参数和返回值只用上述类型和接口指针。
  - `IFrameContext.hpp`、`IRenderSystem.hpp`：声明帧上下文与渲染系统接口（参数可先留简版，例如 `Draw(IFrameContext&)`）。
- **不要求**：尚未实现这些接口；现有调用仍走 `OpenGLGraphicsDevice` / `VulkanGraphicsDevice` 和现有 Kong* 类型。
- **验收**：新头文件被少量占位实现或测试 include 通过即可。

### 阶段 2：设备抽象与工厂

- **实现**：
  - `OpenGLGraphicsDevice` 继承并实现 `IGraphicsDevice`（保留原有 `Init` 与内部 GL 状态）。
  - `VulkanGraphicsDevice` 继承并实现 `IGraphicsDevice`（原有 Vulkan 初始化与创建逻辑保留，新接口内部转调现有实现）。
  - 工厂：`CreateGraphicsDevice(BackendType)` 返回 `std::unique_ptr<IGraphicsDevice>`；根据当前 CMake 选项或运行期配置选择后端。
- **窗口**：`window.cpp` 改为从 `IGraphicsDevice*` 取“当前设备”并调用 `Init`，不再在源文件中 `#ifdef RENDER_IN_VULKAN` 分支取 Vulkan/OpenGL 设备。
- **验收**：应用启动、窗口与现有渲染行为不变；仅“获取设备”的调用点统一为工厂 + `IGraphicsDevice*`。

### 阶段 3：Buffer / Texture 抽象与迁移

- **实现**：
  - `IBuffer` 的 OpenGL/Vulkan 实现（包装现有 `OpenGLBuffer` / `VulkanBuffer` 或逐步内联替换）。
  - `ITexture` 的 OpenGL/Vulkan 实现（包装现有 `OpenGLTexture` / `VulkanTexture`），创建统一为 `IGraphicsDevice::CreateTexture(TextureDesc)`，内部将 `TextureDesc` 转为 GL 或 Vk 的创建参数。
- **迁移**：
  - `RenderInfo`、`ResourceManager`、`KongTexture` 使用处逐步改为持有/返回 `IBuffer*`、`ITexture*`；旧类型可保留为实现细节或逐步删除。
- **验收**：现有场景与材质加载、延迟/后处理等仍正常，无功能回退。

### 阶段 4：帧上下文（IFrameContext）与 RenderModule 主循环

- **实现**：
  - OpenGL：实现 `IFrameContext`，`BeginFrame`/`EndFrame` 可空或只做状态重置；`GetCurrentCommandList()` 返回 nullptr；Present 仍在 `EndFrame` 或窗口层调用 `SwapBuffers`。
  - Vulkan：实现 `IFrameContext`，内部持当前帧的 `VkCommandBuffer`，`BeginFrame` 里做 begin command buffer、acquire image 等，`EndFrame` 里 submit、present。
- **RenderModule**：
  - 主循环统一为：`device->BeginFrame()` → 得到 `IFrameContext` → `Update(delta)` 里各系统只接收 `IFrameContext&`（不再传 `VkCommandBuffer` 或 GL 专用结构）→ `device->EndFrame()`。
- **验收**：双后端下帧流程统一，RenderModule 中与“当前是 GL 还是 Vulkan”相关的 `#ifdef` 明显减少或移除。

### 阶段 5：Pipeline / RenderPass / Framebuffer 抽象

- **设计**：`IPipeline`、`IRenderPass`、`IFramebuffer` 接口与 `PipelineDesc`、`RenderPassDesc` 等；OpenGL/Vulkan 各自实现。
- **策略**：先选 1～2 个简单 RenderSystem（例如全屏后处理、或简单几何）改为“通过 Device 创建 IPipeline/IRenderPass/IFramebuffer + 在 Draw 里只调用接口”，验证通过后再推广到延迟、阴影、天空盒等。
- **验收**：至少一个完整 pass 的绘制路径从“直接调 GL/Vk”改为“只调抽象接口”，双后端均可运行。

### 阶段 6：IRenderSystem 统一与 RenderModule 去分支

- **定义**：`IRenderSystem::Init(IGraphicsDevice* device, ...)`、`IRenderSystem::Draw(IFrameContext& ctx, const SceneDrawInfo& info)`；返回值若需要，用 `ITexture*` 或引擎 ID。
- **实现**：每个现有 Gl*RenderSystem / Vk*RenderSystem 改为实现 `IRenderSystem`，内部仍使用本后端的 Pipeline/Pass/FBO 实现类。
- **RenderModule**：用 `std::vector<std::unique_ptr<IRenderSystem>>` 保存所有系统，按固定顺序 Init 和 Draw；根据当前 `BackendType` 注册对应 GL 或 Vulkan 的 RenderSystem 实现，不再在 RenderModule 内写 `#ifdef RENDER_IN_VULKAN` 的成对分支。
- **验收**：同一套场景与配置，切换 OpenGL/Vulkan 仅通过配置或编译选项，业务逻辑无宏分支。

### 阶段 7：描述符/绑定与公共类型清理

- **描述符**：在 Pipeline 或独立 `IDescriptorSet` 上提供 `BindUniformBuffer(slot, IBuffer*)`、`BindTexture(slot, ITexture*)`，两后端分别实现。
- **清理**：`RenderCommon.hpp` 等中的 `GLuint`、`GLenum`、`Vk*` 替换为引擎枚举或抽象类型；`EShaderType` 改为引擎侧枚举（如 `ShaderStage::Vertex`），在实现层再映射到 GL/Vk。
- **验收**：上层与公共头文件不再包含 GL/Vulkan 头文件；仅实现层与设备层包含。

---

## 五、目录与文件组织建议

- **抽象层（仅接口与引擎类型）**  
  `tinyGL/src/Render/Abstraction/`  
  - `Types.hpp`、`IGraphicsDevice.hpp`、`IBuffer.hpp`、`ITexture.hpp`、`IPipeline.hpp`、`IRenderPass.hpp`、`IFramebuffer.hpp`、`IFrameContext.hpp`、`IRenderSystem.hpp`、`DeviceFactory.hpp/.cpp`（可选）。

- **OpenGL 实现**  
  保留/整理在 `tinyGL/src/Render/GraphicsAPI/OpenGL/`，新增或重命名：  
  - `GLDevice.cpp` 实现 `IGraphicsDevice`，内部可保留现有 `OpenGLGraphicsDevice` 逻辑。  
  - `GLBuffer`、`GLTexture`、`GLPipeline`、`GLRenderPass`、`GLFramebuffer`、`GLFrameContext` 等实现对应接口。

- **Vulkan 实现**  
  保留/整理在 `tinyGL/src/Render/GraphicsAPI/Vulkan/`，新增或重命名：  
  - `VkDevice.cpp` 实现 `IGraphicsDevice`。  
  - `VkBuffer`、`VkTexture`（可包装现有 `VulkanBuffer`/`VulkanTexture`）、`VkPipeline`、`VkRenderPass`、`VkFramebuffer`、`VkFrameContext` 等。

- **RenderModule**  
  仅依赖 `Abstraction/` 与 `IGraphicsDevice*`、`IRenderSystem*`、`IFrameContext`，不再直接包含 OpenGL/Vulkan 头文件或具体设备类。

---

## 六、风险与注意点

1. **OpenGL 的“隐式状态”**：管线、FBO、绑定等是全局状态，抽象层要在每帧/每次 Draw 里显式设置，避免状态泄漏或顺序依赖；Vulkan 本身显式，相对容易封装。
2. **Shader 与格式**：顶点属性、uniform/descriptor 布局需在两后端一致；建议先固定一套 layout（如 std140），再在抽象层用同一份 Desc 描述。
3. **性能**：抽象层避免每帧大量虚调用或多余状态切换；关键路径可考虑内联或 template 策略（在保证接口稳定的前提下）。
4. **增量迁移**：每个阶段只动一部分调用点，保留旧路径直到新路径验证通过，再删除旧代码与 `#ifdef`。

---

## 七、与现有 INDEX 的对应

- 实施过程中，**docs/INDEX.md** 中「3.5 图形 API 抽象」一节可逐步更新为：先写“目标：统一接口 IGraphicsDevice / IBuffer / ITexture …”，再随阶段完成补充“Abstraction/ 目录”“OpenGL/Vulkan 实现职责”和“RenderModule 只依赖抽象层”。  
- 本计划作为 **docs/RenderAbstractionPlan.md** 固定引用，便于后续按阶段勾选和记录进度。

---

## 八、OpenGL 与 Vulkan 顺序设计评价

### 8.1 逻辑顺序一致性（优点）

两套后端的**语义顺序是一致的**：

- 先更新场景与光照 UBO；
- 再画阴影；
- 再画主场景（延迟几何 + 光照，或前向）；
- 再天空盒；
- 再后处理；
- 最后 UI。

因此“先阴影、再主场景、再天空盒、再后处理”的管线思路在两边是对齐的，合并时只需定一个**规范顺序**，让两后端都按同一顺序注册/执行 Pass，即可在行为上统一。

### 8.2 OpenGL 顺序设计的优缺点

**优点**

- **单一主 FBO**：延迟、非延迟、天空盒、SSR、水体都画到同一组 `m_renderToBuffer` / `m_renderToTextures`，数据流简单，中间结果用 `RenderResultInfo` 在几个系统之间手递手即可。
- **实现早、功能全**：水体、SSR、非延迟物体、后处理链都已接好，适合作为“功能参考”和合并时的行为基准。

**缺点**

- **阴影分散在光源组件**：`RenderShadowMap()` 里遍历光源并调每个光源的 `RenderShadowMap()`，阴影 FBO 和绘制逻辑挂在 `CLightComponent` / `CDirectionalLightComponent` 上。扩展多光源阴影或 CSM 时要改多处，和“一个阴影系统管所有阴影”的 Vulkan 思路不一致，合并时建议**收拢到统一 Shadow Pass**。
- **RenderSceneObject 职责过重**：延迟、非延迟、天空盒、SSR 全塞在一个函数里，顺序写死，难以插新 Pass 或按配置开关某一步；与 Vulkan 的“每个 Pass 一个 System、Module 只按表驱动”相比，可扩展性差。
- **无显式帧边界**：没有 BeginFrame/EndFrame，状态和“一帧”的边界靠约定，抽象成 `IFrameContext` 时需要在 OpenGL 侧补一层“逻辑帧”封装（例如每帧开始时重置/绑定状态）。

### 8.3 Vulkan 顺序设计的优缺点

**优点**

- **Pass 边界清晰**：每个 System 自己 `BeginRenderPass` / `EndRenderPass`，输入输出（如 defer 的 color/depth、skybox 的 input、postprocess 的 swapchain）在 CreateInfo 里显式声明，依赖关系一目了然。
- **阴影集中**：`VkShadowMapRenderSystem` 统一画所有光源的阴影，扩展 CSM/点光阴影时只改一处，合并时建议以这种**集中式 Shadow Pass** 为统一模型。
- **与 Vulkan 模型匹配**：显式 CommandBuffer、RenderPass、Framebuffer，便于做 barrier、多线程录制等，抽象层只需把“当前帧的 context”传给各 System。

**缺点**

- **功能缺口**：当前 Vulkan 路径没有水体、没有 SSR，非延迟物体也未单独成 Pass（若需要需在 VkDefer 或单独 System 里补）。合并时要明确这些是“待实现”而非“设计上不做”。
- **顺序写死在 Update**：虽然每个 Pass 是独立 System，但调用顺序仍在 `Update()` 里用一长串 `m_vk*->Draw(frameInfo)` 写死，未做成“Pass 列表 + 依赖”的配置化驱动，合并时可一并改成**可配置的 Pass 顺序**。

### 8.4 小结

| 维度         | OpenGL                         | Vulkan                          | 合并建议                     |
|--------------|--------------------------------|----------------------------------|------------------------------|
| 阴影         | 分散在各光源组件               | 集中在一个 ShadowMap System     | 统一为“集中式 Shadow Pass”   |
| 主场景结构   | 单函数内顺序写死               | 多 System 顺序写死              | 统一为“Pass 列表 + 按序 Draw”|
| 中间结果     | 单一主 FBO + RenderResultInfo  | 每 Pass 独立 FBO，CreateInfo 链 | 统一为“Pass 输入/输出句柄”  |
| 帧边界       | 无                             | BeginFrame/EndFrame             | 统一用 IFrameContext 表示一帧 |
| 功能覆盖     | 全（含水体、SSR、非延迟）      | 缺水体、SSR、独立非延迟 Pass    | 以 OpenGL 为功能清单补 Vulkan |

---

## 九、渲染顺序合并规划

### 9.1 目标：单一规范顺序 + 双后端实现

- 定义一套**与 API 无关的 Pass 顺序**（及可选 Pass 的开关），RenderModule 只按该顺序调用 `IRenderSystem::Draw(frameContext, sceneInfo)`，不区分 OpenGL/Vulkan。
- OpenGL 与 Vulkan 各自实现同一批“逻辑 Pass”（Shadow、Defer、Skybox、Water、SSR、Postprocess、UI），输入输出用 `ITexture*` / `IFramebuffer*` 或引擎内 ID 表达，不暴露 GL/Vk。

### 9.2 规范 Pass 顺序（建议）

按当前两端的共同逻辑与扩展性，建议规范顺序为：

```
1. UpdateSceneRenderInfo()           // 非 Pass，仅更新 UBO/Descriptor
2. Shadow Pass                        // 所有光源阴影 → 阴影贴图
3. Main Scene Pass                    // 延迟：几何 + 光照 → color + depth；或前向 → color + depth
4. NonDeferred Pass（可选）           // 非延迟物体 → 叠加到 Main Scene 的 color/depth
5. Skybox Pass                        // 读 Main Scene color/depth，写回或写独立 RT
6. Water Pass（可选）                 // 读当前 color/depth，叠加水体
7. SSR Pass（可选）                    // 读 color/depth/等，写反射到指定附件
8. Postprocess Pass                   // 读最终场景 RT，输出到 SwapChain/主 FBO
9. UI Pass                            // ImGui 等，画到当前显示目标
```

- **Main Scene**：在“延迟”与“前向”之间二选一（或通过配置选择），与当前 VK_DEFER / 非 VK_DEFER 对应。
- **NonDeferred / Water / SSR**：若某后端暂未实现，对应 System 的 `Draw` 可空实现或直接 return，不破坏顺序。

### 9.3 合并内容清单（与 RenderAbstractionPlan 阶段对应）

| 合并项           | 说明 | 建议阶段 |
|------------------|------|----------|
| **统一帧边界**   | RenderModule 始终调用 BeginFrame → Update(Draw) → EndFrame；OpenGL 实现 IFrameContext 空操作或状态重置，Vulkan 实现为 Acquire + BeginCB / EndCB + Submit + Present。 | 阶段 4 |
| **统一 Pass 列表** | RenderModule 持有一个 `std::vector<std::unique_ptr<IRenderSystem>>`，按规范顺序注册（Shadow、Defer/Forward、NonDeferred、Skybox、Water、SSR、Postprocess、UI）；根据配置或后端能力跳过未实现的 Pass。 | 阶段 6 |
| **统一 Pass 输入输出** | 用 `SceneDrawInfo`（或类似）携带“当前可用的 RT 句柄”（如 sceneColor、sceneDepth、shadowMaps[]）；每个 IRenderSystem::Draw 只读这些句柄并返回本 Pass 产出的句柄供下一 Pass 使用，或由 RenderModule 维护一个小的 “current color/current depth” 槽位，每 Pass 结束后更新。 | 阶段 5～6 |
| **阴影收拢到单一 Pass** | OpenGL 侧新增或改造为“ShadowRenderSystem”：在 Init 时从场景收集所有需要阴影的光源，在 Draw 里统一遍历并绘制到各阴影图，不再在 LightComponent 里各自 RenderShadowMap；Vulkan 侧保持现有 VkShadowMapRenderSystem，实现同一 IRenderSystem 接口。 | 阶段 6 |
| **拆开 RenderSceneObject** | OpenGL 侧将 RenderSceneObject 拆成：DeferRenderSystem.Draw、NonDeferredPass（新或并入现有）、SkyboxRenderSystem.Draw、SSRRenderSystem.Draw；每个的输入输出符合统一 Pass 输入输出约定，便于与 Vulkan 一一对应。 | 阶段 6 |
| **Vulkan 补全缺失 Pass** | 在 Vulkan 侧实现与 OpenGL 对等的 Water、SSR、NonDeferred（若需要），使同一套 Pass 列表在双后端下行为一致。 | 阶段 6 之后 |

### 9.4 合并后的 RenderModule 伪代码（目标形态）

```text
// 初始化时（根据 BackendType 注册不同实现，顺序一致）
m_renderSystems.push_back(CreateShadowRenderSystem(device, ...));
m_renderSystems.push_back(CreateMainSceneRenderSystem(device, ...));  // Defer or Forward
m_renderSystems.push_back(CreateNonDeferredRenderSystem(device, ...)); // 可选
m_renderSystems.push_back(CreateSkyboxRenderSystem(device, ...));
m_renderSystems.push_back(CreateWaterRenderSystem(device, ...));       // 可选
m_renderSystems.push_back(CreateSSRRenderSystem(device, ...));          // 可选
m_renderSystems.push_back(CreatePostprocessRenderSystem(device, ...));
m_renderSystems.push_back(CreateUIRenderSystem(device, ...));

// 每帧
device->BeginFrame(frameContext);
UpdateSceneRenderInfo();
for (auto& sys : m_renderSystems)
    sys->Draw(frameContext, sceneDrawInfo);  // sceneDrawInfo 内更新 color/depth 等句柄
device->EndFrame();
```

这样 OpenGL 与 Vulkan 的“顺序设计”在规范层面完全合并，差异仅剩各 Pass 的内部实现与后端能力（如 Vulkan 补全水体/SSR）。

---

*计划版本：初稿。实施时可按实际工作量将某阶段拆成多步，或合并阶段 5/6 的试点与推广。*
