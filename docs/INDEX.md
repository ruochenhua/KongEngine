# KongEngine 项目索引

个人 3D 渲染引擎，主渲染后端为 **OpenGL**，可选 **Vulkan**。本索引供后续维护与扩展时快速定位代码与资源。

---

## 1. 项目概览

| 项目 | 说明 |
|------|------|
| **主程序** | 可执行目标 `KongEngine`，源码在 `tinyGL/src/` |
| **渲染 API** | OpenGL（默认）、Vulkan（`USE_VULKAN=ON` 时） |
| **构建** | CMake 3.13+，C++17，Windows 下运行 `build.bat` → 在 `project_build/` 生成 sln |
| **场景描述** | YAML（主）/ JSON，见 `resource/scene/*.yaml` |
| **UI** | ImGui |
| **物理** | Tap Engine（规划中，README 提及，当前仓库未接入） |

---

## 2. 根目录与构建

```
KongEngine/
├── CMakeLists.txt          # 主 CMake：OpenGL/Vulkan 开关、KongEngine 可执行文件
├── build.bat               # Windows 构建入口（生成 project_build）
├── README.md               # 项目说明与能力清单
├── docs/                   # 文档
│   ├── INDEX.md            # 本索引（维护用）
│   └── AtmosRender.md      # 大气/体积渲染参考
├── tinyGL/                 # 引擎主源码
├── resource/               # 运行时资源（场景、Shader、模型等）
├── 3rdparty/               # 第三方库
└── project_build/         # 构建输出（git 可忽略）
```

### 构建与渲染后端

- **默认（OpenGL）**：`build.bat` 中未传 `USE_VULKAN`，不定义 `RENDER_IN_VULKAN`。
- **启用 Vulkan**：CMake 需 `-DUSE_VULKAN=ON`，且 `find_package(Vulkan)` 成功时会：
  - `add_definitions(-DRENDER_IN_VULKAN)`
  - `target_link_libraries(KongEngine Vulkan::Vulkan)`

源码中所有 Vulkan 专有路径均用 `#ifdef RENDER_IN_VULKAN` 包裹。

---

## 3. 引擎源码结构（tinyGL/src）

### 3.1 入口与应用

| 文件 | 职责 |
|------|------|
| `app.cpp` / `app.hpp` | `KongApp`：初始化、主循环、按 delta 调用 `KongRenderModule::Update`、Vulkan 时 `BeginFrame`/`EndFrame` |
| `window.cpp` / `window.hpp` | `KongWindow`：GLFW 窗口创建与尺寸等 |
| `ui.cpp` / `ui.h` | `KongUIManager`：ImGui 初始化与渲染，OpenGL/ImGui_ImplVulkan 分支 |

### 3.2 场景与 Actor

| 文件 | 职责 |
|------|------|
| `Scene.cpp` / `Scene.hpp` | 场景容器、加载/卸载、Vulkan 时 `vkDeviceWaitIdle` 等 |
| `Actor.cpp` / `Actor.hpp` | 场景中的实体，挂载 Component |
| `Parser/YamlParser.cpp`、`YamlParser.h` | 场景 YAML 解析与反序列化 |
| `Parser/ResourceManager.cpp`、`ResourceManager.h` | 资源加载与缓存（纹理、模型等） |

### 3.3 组件（Component）

| 目录/文件 | 职责 |
|-----------|------|
| `Component/Component.cpp(.h)` | 组件基类 |
| `Component/CameraComponent.*` | 相机 |
| `Component/LightComponent.*` | 光源 |
| `Component/Mesh/` | 网格相关：`ModelMeshComponent`（Assimp）、`BoxShape`、`QuadShape`、`SphereShape`、`Terrain`、`Water`、`VolumetricCloud` 等 |

### 3.4 渲染核心

| 文件 | 职责 |
|------|------|
| `Render/RenderModule.cpp` / `RenderModule.hpp` | **渲染总控**：UBO、主 FBO、各 RenderSystem 的创建与调用；OpenGL/Vulkan 双路径（Vulkan 描述符池、SwapChain、CommandBuffer、各 Vk* 系统） |
| `Render/RenderCommon.hpp` / `RenderCommon.cpp` | 渲染通用类型；`RenderInfo` 在 Vulkan 下为 `VulkanRenderInfo` |
| `Render/Resource/Texture.*` | `KongTexture`，Vulkan 下实现为 `VulkanTexture` |
| `Render/Resource/Buffer.*` | 缓冲抽象 |

### 3.5 图形 API 抽象（RHI 层）

- **目标**：上层（RenderModule、场景、组件）只依赖抽象接口，通过配置或编译选择 OpenGL/Vulkan 后端；无业务层 `#ifdef RENDER_IN_VULKAN` 分支。
- **抽象层**：`Render/Abstraction/`
  - `BackendType.hpp`、`Types.hpp`（DataFormat、BufferDesc、TextureDesc、SceneDrawInfo、ShaderStage 等）、`IGraphicsDevice.hpp`、`IBuffer.hpp`、`ITexture.hpp`、`IFrameContext.hpp`、`IRenderSystem.hpp`、`IPipeline.hpp`、`IRenderPass.hpp`、`IFramebuffer.hpp`、`IDescriptorSet.hpp`、`DeviceFactory.hpp/.cpp`、`RenderSystemAdapter.hpp`。
  - 主循环：`app.cpp` 中 `device->BeginFrame()` → `RenderModule::Update(delta, &frameCtx)` → `device->EndFrame()`；`Update` 内按 `m_renderSystems` 顺序调用各 `IRenderSystem::Draw(IFrameContext&, SceneDrawInfo&)`。
- **OpenGL 实现**：`Render/GraphicsAPI/OpenGL/`
  - `OpenGLGraphicsDevice` 实现 `IGraphicsDevice`；`GLBuffer`/`GLTexture` 实现 `IBuffer`/`ITexture`；`GLFrameContext` 实现 `IFrameContext`；各 Gl* RenderSystem 通过 `RenderSystemAdapter` 包装，由 RenderModule 统一驱动。
- **Vulkan 实现**：`Render/GraphicsAPI/Vulkan/`
  - `VulkanGraphicsDevice` 实现 `IGraphicsDevice`；`VkBufferRHI` 实现 `IBuffer`；`VkFrameContext` 实现 `IFrameContext`；各 Vk* RenderSystem 通过 `RenderSystemAdapter` 包装。暂未实现的 Pass（如 Water、SSR）可注册空实现，不崩溃。
- **公共类型**：`RenderCommon.hpp` 不再包含 GL/Vulkan 头文件；`EShaderType` 为引擎侧整型枚举，`RenderInfo::instance_buffer` 为 `uint32_t`，实现层负责映射。

### 3.6 Shader（OpenGL 侧）

- 位于 `tinyGL/src/Shader/OpenGL/`：  
  `OpenGLShader`、`PBRShader`、`DeferInfoShader`、`PostprocessShader`、`SkyboxShader`、`ShadowMapShader`、`BlendShader`、`EmitShader` 等，与 `resource/shader/` 下 GLSL 对应。

---

## 4. 资源与 Shader 路径（resource/）

- **场景**：`resource/scene/*.yaml`（如 `hello_ssr.yaml`、`hello_terrain.yaml`、`hello_pbr_texture.yaml` 等），入口默认加载 `scene/hello_ssr.yaml`（见 `app.cpp`）。
- **Shader**：
  - **OpenGL**：`resource/shader/` 下按功能分子目录：`defer_pbr.*`、`skybox/`、`shadow/`、`postprocess/`、`water/`、`terrain/`、`volumetric_cloud/`、`ssr.*`、`ssao*` 等。
  - **Vulkan**：`resource/shader/Vulkan/`，使用 `.vulkan.vert.spv` / `.frag.spv` / `.comp.spv` 等预编译 SPIR-V；子目录如 `defer/`、`shadow/`。
- **模型/贴图**：`resource/` 下各子目录（如 `nanosuit/`、`Porsche/`、`african_head/` 等）及 `Engine/box`、`Engine/sphere`。

---

## 5. 第三方依赖（3rdparty/）

由根目录 `CMakeLists.txt` 通过 `add_subdirectory(3rdparty)` 引入，见 `3rdparty/CMakeLists.txt`：

| 库 | 用途 |
|----|------|
| GLFW | 窗口与输入 |
| GLM | 数学库 |
| GLAD | OpenGL 加载 |
| Assimp | 模型加载 |
| yaml-cpp | YAML 解析 |
| imgui | UI（主 CMakeLists 中直接 GLOB 其 cpp 并链接） |

Vulkan 由系统/CMake 的 `find_package(Vulkan)` 提供，非 3rdparty 子目录。

---

## 6. 维护与扩展要点

### 6.1 双后端约定

- 所有 **Vulkan 专用** 代码用 `#ifdef RENDER_IN_VULKAN` 包裹；OpenGL 路径不要依赖 Vulkan 头文件或类型。
- 新增渲染功能时，若需双后端支持，需在 **OpenGL**（`GraphicsAPI/OpenGL/RenderSystem/`、`Shader/OpenGL/`）和 **Vulkan**（`GraphicsAPI/Vulkan/`、`resource/shader/Vulkan/`）各实现一版，并在 `RenderModule` 中按后端分支调用。

### 6.2 关键宏与 CMake 变量

- `RENDER_IN_VULKAN`：由 CMake 在 `Vulkan_FOUND AND USE_VULKAN` 时定义。
- `USE_VULKAN`：CMake 选项，需在配置时 `-DUSE_VULKAN=ON` 才会尝试启用 Vulkan。

### 6.3 与 README 的对应关系

- **渲染**：PBR、延迟、阴影（含 CSM）、IBL、后处理、SSR、地形、水体、体积云等见 README「渲染能力」；多图形 API 中 OpenGL 已实现，Vulkan 部分实现，Dx12 未做。
- **编辑**：场景加载/反序列化已做；序列化、场景增删对象、世界线框等见 README「编辑能力」。
- **物理**：Tap Engine 计划接入，当前未在 CMake/源码中引用。

### 6.4 常用修改入口速查

| 需求 | 建议查看位置 |
|------|----------------|
| 切换默认场景 | `tinyGL/src/app.cpp` 中 `LoadScene("scene/...")` |
| 增加/修改 OpenGL 渲染管线 | `RenderModule` + `GraphicsAPI/OpenGL/RenderSystem/` + `resource/shader/` |
| 增加/修改 Vulkan 渲染管线 | `RenderModule` 中 `#ifdef RENDER_IN_VULKAN` 块 + `GraphicsAPI/Vulkan/RenderSystem/` + `resource/shader/Vulkan/` |
| 场景格式与加载 | `Parser/YamlParser.*`、`resource/scene/*.yaml` |
| 窗口与输入 | `window.*`、GLFW |
| UI 与 ImGui 后端 | `ui.cpp` / `ui.h`（OpenGL/ImGui_ImplVulkan） |

---

## 7. 文档与参考

- **大气/体积渲染**：`docs/AtmosRender.md`（外链课程笔记）。
- **渲染流程梳理**：`docs/RenderingFlow.md`（OpenGL / Vulkan 主循环、各 Pass 顺序与数据流）。
- **统一渲染接口架构与实施计划**：`docs/RenderAbstractionPlan.md`（一套接口对接 OpenGL/Vulkan 的分层设计、抽象接口定义与 7 阶段实施步骤）。
- **能力清单与路线图**：见根目录 `README.md`。

---

*本索引最后更新与项目当前状态一致，后续若增加 Dx12、Tap 物理或大目录重构，建议同步更新此文档。*
