# 目标渲染架构达成路线图（详细）

本文描述与你**最终期望**一致的终点形态，以及从**当前仓库**到达该形态所需工作。  
更短的任务表与 Todo ID 见 [`rhi-todo-backlog.md`](./rhi-todo-backlog.md)；分层原则见 [`render-logic-rhi-and-api.md`](./render-logic-rhi-and-api.md)。

---

## 0. 既定开发策略（已采纳）

**表述**：**RHI 契约按 Vulkan 思维定，实现先以 OpenGL 跑通关键路径，再补全 Vulkan。**

| 层面 | 做法 |
|------|------|
| **契约 / 语义** | 在 `IRHICommandList`、`IFramebuffer`、`Types.hpp` 与架构文档中，以 **Render Pass 边界、附件、屏障/布局（Vulkan 概念）** 为「规范」；OpenGL 实现允许 **no-op 或即时模式等价**，但**不得**把「隐式 GL 全局状态」提升为契约。 |
| **实现顺序** | 每条新能力：**先在 OpenGL 后端实现并接一条可运行的关键管线**（如主场景 MRT → 单步 fullscreen），逻辑 Pass 只调 RHI；**再在 Vulkan 后端对齐同一套调用序列**（`arch-vk-tex`、`arch-vk-fbo-tex` 等）。 |
| **禁止误解** | OpenGL 代码**不必**结构上模仿 Vulkan；只需 **满足同一 RHI 接口语义**。 |

**对路线图各阶段的影响**：

- **阶段 B（Pass 迁移）** 可与 **阶段 A 的 GL 部分**并行推进；Vulkan 侧 **A.1～A.3** 在对应 Pass 需要绑纹理/多 subpass 时再集中补齐，而不必阻塞第一条「API 无关 Pass」在 GL 上跑通。  
- **阶段 D（统一 Host）** 仍以「Pass 内无 `Gl*`/`Vk*`」为前提；**不**要求 Vulkan 先于 OpenGL 全部完成。

---

## 1. 目标架构：验收标准（Definition of Done）

满足下列条款，可认为达到「统一 Host + API 无关 Pass」的设计目标：

### 1.1 单一 Pass 宿主（Host）

- 工程中**不再**存在 `OpenGLRenderPassHost` / `VulkanRenderPassHost` 两个并列类作为长期形态。
- 取而代之是**一个**与 API 无关的宿主，例如：
  - **`RenderPassHost`**（或 `DefaultRenderPassHost`），仅依赖：
    - `IGraphicsDevice*`
    - `IRenderModuleBackend*`（若仍需要 Vulkan Descriptor / 全局 UBO 等「后端服务」）
    - 抽象资源接口（`IBuffer` / `ITexture` / `IFramebuffer` / `IPipeline`）
  - 初始化时序通过**抽象能力**表达，例如：
    - `IGraphicsDevice::OnRenderModuleAttached(KongRenderModule&)`  
    - 或 `IRenderPassHost::OnResourcesReady(Phase)`  
    而不是「GL 在 OnAttached、VK 在 AfterVulkanDescriptorReady」硬编码在两个类里。

**允许的中间态**：保留极薄的 **`IRenderPassHostPlatform`** 或 **`IGraphicsDevice::CreateRenderPassHost()`** 返回**同一逻辑类**的不同构造参数（由工厂注入 swapchain、descriptor pool 等），但**拓扑注册代码只有一份**。

### 1.2 每一个逻辑 Pass（`IRenderSystem`）与 Render API 无关

- 所有逻辑 Pass 源码位于**独立目录**（建议例如 `Render/Logic/Passes/` 或 `Render/Passes/`），头文件**不得** include：
  - `glad`、`GL/*`、`vulkan.h`、`vulkan_*`
  - `GraphicsAPI/OpenGL/*`、`GraphicsAPI/Vulkan/*` 中**具体实现**头（实现文件可在 `GraphicsAPI` 内，但 Pass 算法不在此）
- Pass 内**只**使用：
  - `IRHICommandList`、`IFrameContext`
  - `SceneDrawInfo`（并逐步去掉 `uintptr_t` / `GLuint` 主路径）
  - `IGraphicsDevice` 查询（如需要创建 transient 资源，应通过设备工厂而非直接 new `GLTexture`）
- **删除或降级** `RenderSystemAdapter` 包裹「整段 lambda 里调 `GlDefer`」的模式；Adapter 仅保留为**测试桩**或删除。

### 1.3 数据与结果类型 API 无关

- `RenderResultInfo`：**主路径**为 `IFramebuffer*`、`ITexture*`（及可选 `IRHI` 句柄）；`GLuint` / `VkImage` 仅允许在 **GraphicsAPI 实现 .cpp** 内存在。
- 场景级 GPU 数据（相机矩阵、光照、`near_far` 等）：统一为 **`IBuffer*`**（或集中结构体 `SceneGpuResources` 内全是抽象指针），**不再**在 `KongRenderModule` 或 Pass 中直接操作 `UBOHelper` / `glBindBufferBase`（GL 实现藏在 `GLBuffer` / command list 内）。
- `KongRenderModule.hpp` **不再** include `OpenGLRenderSystem.hpp` 等仅服务 GL 类型的头；若仍需编辑器/工具查询子系统，通过 **`ISubsystemRegistry`** 或 `RHISubsystemKind` 等抽象枚举。

### 1.4 后端职责边界清晰

- **`IGraphicsDevice` + `IRHICommandList` 实现类**：唯一的 `gl*` / `vkCmd*` 聚集地（外加各 `GL*` / `Vk*` 资源包装）。
- **`IRenderModuleBackend`**（若保留）：只做 **Vulkan 全局 UBO、Descriptor Pool、与交换链相关的每帧服务**，**不**再负责「注册一整条与 GL 重复的 Pass 链」。长期可收缩为「设备扩展」而非独立心智模型。
- **`KongRenderModule`**：维护相机、场景光信息、`PushRenderSystem` 列表、`Update` 驱动顺序；**不**持有任何 API 特有资源句柄。

### 1.5 工程与可选目标

- **CI / 脚本**：对逻辑 Pass 目录做 include 白名单检查（见 backlog `arch-guard`）。
- **（可选）单二进制双后端**（`arch-dual`）：运行时选设备；与「统一 Host」天然契合。

---

## 2. 当前结构 vs 目标：差距一览

| 维度 | 当前（典型现状） | 目标 |
|------|------------------|------|
| Host | `OpenGLRenderPassHost` / `VulkanRenderPassHost` 两套，各持 `Gl*` / `Vk*` 子系统 | **单一**逻辑 Host + 工厂/设备注入后端差异 |
| Pass 实现 | `GlDefer*`、`VkDefer*`、`RenderSystemAdapter` + lambda 调旧系统 | **一套** `ShadowPass`、`DeferredGeometryPass` 等，仅 RHI |
| 主场景 FBO | GL：`GLFramebuffer` + `GLuint` 遗留；VK：多为传统 `VkFramebuffer` 路径 | 统一 **`IFramebuffer`**，创建自 `IGraphicsDevice` |
| 纹理绑定 | GL：`ITexture::Bind`；VK：`BindTexture` 未完整 | 两侧 **`IRHICommandList::BindTexture`**（或 `IDescriptorSet`）语义一致 |
| Module 头文件 | 仍可能依赖 GL 类型（如 `RenderResultInfo`） | 仅抽象类型 + 前向声明 |
| 兼容路径 | `IRHICommandList == nullptr`、`GetCurrentCommandList` 裸指针 | 收敛为「始终有 CommandList」或明确单一兼容模式后删除 |

---

## 3. 分阶段路线图（建议顺序与依赖）

下列阶段**按依赖排序**；阶段内条目可并行，但**不要跳过「Pass 内已无 API 调用」**去做「统一 Host」（否则统一 Host 里仍会塞满 `ifdef`）。

> **与 §0 策略一致**：**阶段 B 可与阶段 A 在时间上交错**——先在 OpenGL 上按 RHI 契约跑通 Pass；**阶段 A 作为 Vulkan 侧「对齐同一契约」**，不要求先于阶段 B 全部完成。

### 阶段 A — RHI 在 Vulkan 侧「可绑可画」（与 GL 实现并行对齐）

**目标**：逻辑层若只写 `IRHICommandList`，在 **Vulkan** 上也能完成「绑 FB + 清 + 绑管线 + 绑 VB/IB + 绑纹理/UBO + Draw」闭环，与已在 GL 上验证过的调用序列一致。

| 序号 | 工作项 | 说明 / 涉及面 | 完成判据 |
|------|--------|----------------|----------|
| A.1 | **`VulkanRHICommandList::BindTexture` 落地** | 定策略：per-frame descriptor set、push descriptor、或 bindless；与现有 `VkDefer` 的 layout 对齐 | 至少一条最小管线能只通过 RHI 完成纹理采样 |
| A.2 | **`VulkanFramebufferRHI` 与 `ITexture*` 桥接** | 颜色/深度附件可查询为 `ITexture*`，或与 `VulkanTexture` 共享生命周期 | `GetColorAttachment(i)` 非空（在配置好的 FB 上） |
| A.3 | **`USE_VULKAN=ON` 全量编译 + 验证层** | 动态状态、render pass 与现有子系统兼容 | CI 或本地脚本一键通过 |
| A.4 | （按需）**`IRenderPass` / `CreateFramebuffer` 工厂** | 减少手写 `VkRenderPassBeginInfo` | 新建离屏目标不复制粘贴 Vk 样板 |

**参考 Todo ID**：`arch-vk-tex`、`arch-vk-fbo-tex`、`arch-vk-build`、`arch-rhi-optional`（部分）。

---

### 阶段 B — 逐 Pass 迁出 `Gl*`/`Vk*`，形成 API 无关的 `IRenderSystem`

**目标**：每个逻辑 Pass **一份**源码，内部零 `gl`/`vk`。

| 序号 | 工作项 | 当前典型位置 | 迁移要点 |
|------|--------|----------------|----------|
| B.1 | **Shadow / Depth** | `RenderShadowMap`、Light 组件内 GL、Vulkan shadow 系统 | 输出统一为 `ITexture*`（depth）或 `IFramebuffer*`；Pass 只写 `cmd` |
| B.2 | **Deferred Geometry（GBuffer）** | `GlDeferRenderSystem`、`VkDeferRenderSystem` | MRT 用 `IFramebuffer`；几何阶段 `IPipeline`；实例/材质数据走 `IBuffer` |
| B.3 | **Deferred Lighting** | 同上系统内后半段 | 读 GBuffer 附件为 `ITexture`，全屏或屏幕三角；屏障用 `IRHICommandList` |
| B.4 | **Skybox** | `GlSkyboxRenderSystem`、`VulkanSkyBoxRenderSystem` | 立方体 `ITexture` + 深度行为用 RHI 状态 |
| B.5 | **PostProcess** | `GlPostProcessRenderSystem`、`VulkanPostprocessSystem` | 链式 RT：`CreateTexture`/`IFramebuffer`；每步 `IPipeline` |
| B.6 | **SSR / Water 等** | `GlSSReflectionRenderSystem`、`GlWaterRenderSystem` 等 | 反射/折射 FBO 抽象化；相机复制逻辑留在场景层或 Pass 参数，不碰 GL API |

**收敛兼容路径**：

- 删除「无 `frameContext` 时整段不走 RHI」的双轨，或明确仅保留 **Headless/编辑器** 一条文档化路径。
- `GetCurrentCommandList()`：最终仅用于 **调试/与 ImGui 交互**，或删除，统一 `GetRHICommandList()`。

**参考 Todo ID**：`arch-pass-1`～`arch-pass-4`。

---

### 阶段 C — 数据面与 `KongRenderModule` 去 GL/VK 化

| 序号 | 工作项 | 说明 |
|------|--------|------|
| C.1 | **`RenderResultInfo` 主路径 RHI 化** | `rhiFramebuffer`、`rhiResultColor/Depth` 为主；`GLuint` 删除或仅限实现 .cpp |
| C.2 | **`SceneDrawInfo` 清理** | `currentColorRT` 等 `uintptr_t` 逐步废弃 |
| C.3 | **矩阵 / 光照 UBO** | `UBOHelper` 迁出 Host 对外的「逻辑视图」，改为 `IBuffer` + 小服务类（仅 Abstraction 可见的更新接口） |
| C.4 | **`RenderNonDeferSceneObjects` 等** | 改为 `IPipeline` + `IRHICommandList` 绘制，或迁入命名清晰的 `ForwardOpaquePass` |

**参考 Todo ID**：`arch-data-1`、`arch-data-2`。

---

### 阶段 D — 合并为「统一 RenderPassHost」

**前置条件（必须满足）**：

- 阶段 B 完成：**Pass 注册代码不再引用 `GlDeferRenderSystem` 类型名**（注册的是 `std::make_unique<DeferredGeometryPass>(...)` 这类）。
- 阶段 B/C 足够支撑：**主场景与 GBuffer 创建**可通过 `IGraphicsDevice` 或 Host 只持 `IFramebuffer*` / `IBuffer*`；若需双端发布，阶段 A 在合并 Host 前应达到与 GL 行为一致的 Vulkan 闭环。

**工作项**：

| 序号 | 工作项 | 说明 |
|------|--------|------|
| D.1 | 抽象 **「资源就绪」回调** | 把 `AfterVulkanDescriptorReady` 泛化为 `OnBackendReady(void* opaque)` 或 `BackendCapabilities` |
| D.2 | **单一 `RegisterDefaultRenderPasses`** | 仅一份拓扑；后端差异在「资源从哪来」由 `IGraphicsDevice` 注入 |
| D.3 | **删除** `OpenGLRenderPassHost` / `VulkanRenderPassHost` | 或保留为 50 行以内的 **deprecated 转发** 一个版本周期后删 |
| D.4 | **`CreateRenderPassHost()`** | 返回统一类；构造参数来自 `OpenGLGraphicsDevice` / `VulkanGraphicsDevice` |

---

### 阶段 E — 工程化与可选增强

| 序号 | 工作项 | 说明 |
|------|--------|------|
| E.1 | **Include 守卫 / CI** | 逻辑 Pass 目录扫描 `glad`/`vulkan`（`arch-guard`） |
| E.2 | **`ISubsystemRegistry`** | YAML/编辑器只依赖抽象（`arch-registry`） |
| E.3 | **单二进制双后端**（`arch-dual`） | CMake + 运行时 `CreateGraphicsDevice(BackendType)` |
| E.4 | **OpenGL 固定管线状态对象** | `arch-gl-pipeline-state`，与 `IPipeline` 对齐 |

---

## 4. 统一 Host 依赖关系说明（为何不能现在合并）

合并 Host 的前提是：**Host 内不再出现「仅 GL 或仅 VK 的成员类型」**。  
当前 Host 仍直接持有：

- OpenGL：`GlDeferRenderSystem`、`UBOHelper`、`GLuint` 路径的资源；
- Vulkan：`std::unique_ptr<VkDeferRenderSystem>` 等。

在这些成员消失之前强行合并，只会得到：

- 巨型 `#ifdef RENDER_IN_VULKAN` 类，或
- `std::variant` / `void*` 仓库，可维护性更差。

**正确顺序**：先 **Pass 与资源 RHI 化**（以阶段 B/C 为主；Vulkan 用阶段 A 对齐同一契约），再 **合并 Host**（阶段 D）。

---

## 5. 各现有子系统迁移时的检查清单（逐文件心智）

对每一个 `Gl*` / `Vk*` 系统，迁移时自问：

1. **绘制**是否都能改为 **`cmd->*`** 序列？若不能，缺哪条 RHI 命令？先补抽象（阶段 A / `arch-rhi-optional`）。
2. **资源**是否都能改为 **`device->Create*`** 或 Pass 构造函数注入？若生命周期复杂，是否引入 **`IResourcePool` / 帧临时分配器**？
3. **与场景交互**（相机、水面 Actor、光照）是否仍留在 Pass 内？应**只读** `SceneDrawInfo` / `sceneContext`，避免 Pass 里 `GetOpenGLSubsystem`。
4. **Vulkan 特有**（subpass、input attachment）是否封装进 **`IRenderPass` / `IFramebuffer`**，而不是漏到 Pass 逻辑？

---

## 6. 建议的目录终点（参考）

```
Render/
  Abstraction/          # 仅 API 无关接口与类型（现状延续）
  Logic/
    Passes/             # 仅 IRenderSystem 实现，零 glad/vulkan
    RenderPassCatalog.hpp
  GraphicsAPI/
    OpenGL/             # GLBuffer, GLTexture, OpenGLCommandList, ...
    Vulkan/             # Vk*, VulkanRHICommandList, ...
  RenderPassHost.cpp    # 单一 Host（或放在 Logic/）
  KongRenderModule.*
```

实际命名可按你喜好调整；关键是 **Passes 与 GraphicsAPI 物理分离**。

---

## 7. 验证与回归

| 层级 | 建议 |
|------|------|
| 编译 | OpenGL 默认配置；Vulkan `USE_VULKAN=ON`；若做双后端则两种都编 |
| 运行 | 主场景 + 延迟 + 阴影 + 后处理 + 水面（若启用）各跑一遍 |
| 自动化 | 后续可为 RHI 实现加 **轻量单元测试**（无窗口 mock command list） |
| 架构 | `arch-guard` 脚本失败即阻断合并 |

---

## 8. 与 Cursor Todos 的对应关系

| 路线图阶段 | 主要 Todo ID |
|------------|----------------|
| A | `arch-vk-tex`、`arch-vk-fbo-tex`、`arch-vk-build`、`arch-rhi-optional` |
| B | `arch-pass-1`～`arch-pass-4` |
| C | `arch-data-1`、`arch-data-2` |
| D | （建议在 Todos 中新增 **`arch-host-unify`**） |
| E | `arch-guard`、`arch-registry`、`arch-dual`、`arch-gl-pipeline-state` |

---

*本文档为「目标结构」的详细拆解；任务勾选仍以 [`rhi-todo-backlog.md`](./rhi-todo-backlog.md) 与 Cursor Todos 为准。建议在开始阶段 D 时新增 Todo `arch-host-unify` 并链接到本文 §3.D。*
