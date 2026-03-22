# RHI 与渲染架构 — 待办清单（Backlog）

本文档与 Cursor **Todos** 对齐，汇总「已完成」与「未完成」项，并给出建议推进顺序。  
- 分层与接口说明：[`render-logic-rhi-and-api.md`](./render-logic-rhi-and-api.md)  
- **目标架构验收标准 + 分阶段详细路线图**：[`rhi-target-roadmap.md`](./rhi-target-roadmap.md)

---

## 1. 建议推进顺序（依赖大致自上而下）

**策略**（已定）：**RHI 契约按 Vulkan 思维定；实现先 GL 跑通关键路径，再补 Vulkan。** 详见 [`rhi-target-roadmap.md`](./rhi-target-roadmap.md) §0。

1. **契约与接口** — 扩展/文档化 `IRHI*` 时优先问「Vulkan 下是否合理」，再在 `OpenGLCommandList` 等实现。  
2. **Pass 迁移（`arch-pass-1` → `arch-pass-4`）** — 在 **OpenGL** 上先把 API 无关 Pass + RHI 调用链跑通；缺命令再补抽象。  
3. **数据面**（`arch-data-1`、`arch-data-2`）— 与 Pass 迁移可交错。  
4. **Vulkan 对齐** — `arch-vk-tex`、`arch-vk-fbo-tex`、`arch-vk-build`：按已在 GL 上稳定的 RHI 序列实现 VK，而非先阻塞在 VK。  
5. **RHI 扩展 / GL 管线状态**（`arch-rhi-optional`、`arch-gl-pipeline-state`）— 按 Pass 需要增量加接口。  
6. **统一 Host**（`arch-host-unify`）— Pass 逻辑已无 `Gl*`/`Vk*` 后合并 Host；**不要求** VK 先于 GL 全部完成。  
7. **工程约束与可选架构**（`arch-guard`、`arch-registry`、`arch-dual`）— 稳定后再上 CI 或双后端二进制。

---

## 2. 已完成（Todos 中已标记 completed）

| ID | 说明 |
|----|------|
| `arch-rhi-1` | RHI 能力补全：`IFramebuffer` 多附件元数据、`IPipeline::BindGraphics`、`IRHICommandList` 的 `EndRenderPass` / 深度开关等 |
| `arch-rhi-2` | OpenGL：`GLFramebuffer` + `GLTextureView`、`OpenGLCommandList`、`OpenGLGraphicsPipeline`、主场景 FBO 与 `IFramebuffer` 对接 |
| `arch-rhi-3` | Vulkan：`VulkanFramebufferRHI`、`VulkanGraphicsPipeline`、`VulkanRHICommandList` 的 viewport/scissor/bind FB/clear/draw/部分深度动态状态 |

---

## 3. 未完成 — Pass 与数据（核心迁移）

| ID | 状态 | 内容 |
|----|------|------|
| `arch-pass-1` | pending | Shadow / 深度 Pass 仅通过 RHI + `ITexture` / `IBuffer` |
| `arch-pass-2` | pending | Deferred Geometry（GBuffer）迁 RHI；`GlDefer` / `VkDefer` 收缩为薄实现或删除重复逻辑 |
| `arch-pass-3` | pending | Deferred Lighting、Skybox、PostProcess 依次迁 RHI；顺序与 `RenderPassCatalog` 一致 |
| `arch-pass-4` | pending | SSR、水等特效迁 RHI；收敛 `IRHICommandList == nullptr` 与 `GetCurrentCommandList` 兼容路径 |
| `arch-data-1` | pending | `RenderResultInfo` / Module 出口以 `IFramebuffer*`、`ITexture*` 为主，废弃 `GLuint` 主路径 |
| `arch-data-2` | pending | 全局矩阵/光照等场景 GPU 数据统一为 `IBuffer*`（或 `SceneGpuResources`），Module 无 API 特有类型 |

---

## 4. 未完成 — Vulkan RHI 缺口（近期实现遗留）

| ID | 状态 | 内容 |
|----|------|------|
| `arch-vk-tex` | pending | `VulkanRHICommandList::BindTexture`：descriptor / push descriptor / bindless 策略并实现 |
| `arch-vk-fbo-tex` | pending | `VulkanFramebufferRHI`：`GetColorAttachment` / 深度 与 `VulkanTexture`（或 `VkTextureRHI`）桥接 |
| `arch-vk-build` | pending | `USE_VULKAN=ON` 全量编译与运行层验证（动态状态、RenderPass 与现有 `VkDefer` 等兼容） |

---

## 5. 未完成 — RHI 与 OpenGL 增强（可选）

| ID | 状态 | 内容 |
|----|------|------|
| `arch-rhi-optional` | pending | `IRHICommandList`：多色独立 Clear、`ColorWriteMask`、显式 `TextureBarrier` 枚举与 Vulkan 实现 |
| `arch-gl-pipeline-state` | pending | `OpenGLGraphicsPipeline`（或独立状态对象）：混合、深度、裁剪、模板等固定功能状态 |

---

## 6. 未完成 — 统一 Host（目标架构里程碑）

| ID | 状态 | 内容 |
|----|------|------|
| `arch-host-unify` | pending | 合并 `OpenGLRenderPassHost` / `VulkanRenderPassHost` 为**单一** API 无关 `RenderPassHost`；拓扑注册只保留一份。**依赖**：Pass RHI 化与 Vulkan 绑资源闭环（见路线图阶段 A–C） |

---

## 7. 未完成 — 工程与可选目标

| ID | 状态 | 内容 |
|----|------|------|
| `arch-registry` | pending | （可选）`ISubsystemRegistry`：替代分散的 `GetOpenGLSubsystem` / `GetRHISubsystem`，YAML/编辑器只依赖抽象 |
| `arch-guard` | pending | 逻辑 Pass 目录禁止 include `glad` / `vulkan` / 具体 `GraphicsAPI` 实现；CI 或脚本检查 |
| `arch-dual` | pending | （可选）单二进制双后端：CMake 同时编 GL+VK，运行时选择 `IGraphicsDevice` |

---

## 8. 维护说明

- **与 Todos 同步**：在 Cursor 中增删任务时，请在本文件对应小节更新表格（或于里程碑结束时整体重扫一遍代码与 Todos）。  
- **与架构文关系**：[`render-logic-rhi-and-api.md`](./render-logic-rhi-and-api.md) 描述目标分层与接口；[`rhi-target-roadmap.md`](./rhi-target-roadmap.md) 描述**如何到达**目标；本文件跟踪 **Todo ID** 与**完成状态**。

---

*文档生成自仓库当前 RHI 改造规划；最后更新：与 Todos 中 `arch-*` 系列 ID 一致。*
