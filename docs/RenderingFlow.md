# KongEngine 渲染流程梳理

本文档梳理当前引擎在 **OpenGL** 与 **Vulkan** 两套后端下的整帧渲染流程，便于回忆与维护。

---

## 一、主循环（两后端共用前半段）

入口：`KongApp::Run()`（`tinyGL/src/app.cpp`）

```
每帧 (delta > 1/60 秒时):
  1. glfwPollEvents()
  2. m_UIManager.PreRenderUpdate(delta)
  3. m_SceneManager.PreRenderUpdate(delta)
  4. [仅 Vulkan] m_RenderModule.BeginFrame()     // 获取 swapchain 图像、开始录制 command buffer
  5. m_RenderModule.Update(delta)               // 本帧所有渲染逻辑
  6. m_UIManager.PostRenderUpdate()
  7. [仅 Vulkan] m_RenderModule.EndFrame()      // 结束录制、提交队列、Present
     [仅 OpenGL] glfwSwapBuffers(...)           // 交换前后缓冲
```

- **OpenGL**：没有显式的 BeginFrame/EndFrame，每帧就是 Update 里直接发 GL 命令，最后 SwapBuffers。
- **Vulkan**：BeginFrame 里 `AcquireNextImage` + `vkBeginCommandBuffer`；Update 里所有 Draw 只往当前帧的 command buffer 里**录制**；EndFrame 里 `vkEndCommandBuffer`、提交、Present。

---

## 二、OpenGL 渲染流程

### 2.1 初始化（KongRenderModule::Init，非 Vulkan 分支）

1. 创建主 FBO：`InitMainFBO()` → `m_renderToBuffer` + 3 个颜色附件 `m_renderToTextures[0..2]` + 深度 RBO `m_renderToRbo`。
2. 初始化 UBO：`matrix_ubo`（view、projection、cam_pos、near_far）、`scene_light_ubo`（光照）。
3. 初始化各 RenderSystem：  
   `m_skyboxRenderSystem.Init()`、`m_deferRenderSystem.Init()`、`m_postProcessRenderSystem.Init()`、`m_ssReflectionRenderSystem.Init()`、`m_waterRenderSystem.Init()`。

### 2.2 每帧 Update(delta) 内的顺序

```
KongRenderModule::Update(delta):
  1. mainCamera->Update(delta)
  2. UpdateSceneRenderInfo()                    // 从场景收集光源，更新 scene_light_ubo / 矩阵 UBO
  3. RenderShadowMap()                          // 见下
  4. [OpenGL 分支]
     a. m_skyboxRenderSystem.PreRenderUpdate()
     b. matrix_ubo 更新 view / projection / cam_pos
     c. latestRenderResult = RenderSceneObject()           // 见下
     d. latestRenderResult = m_waterRenderSystem.Draw(...) // 水体画到当前 result
     e. latestRenderResult = m_postProcessRenderSystem.Draw(...)  // 后处理到主 FBO/屏幕
     f. RenderUI(delta)                          // ImGui 等
```

### 2.3 RenderShadowMap()（OpenGL）

- 设置：`glCullFace(GL_FRONT)`、viewport 设为阴影分辨率。
- 遍历场景光源，对每个有阴影的光源调用其 `RenderShadowMap()`：
  - 平行光：`scene_render_info.scene_dirlight.lock()->RenderShadowMap()`
  - 点光源：`light.lock()->RenderShadowMap()`
- 阴影贴图由各 `CLightComponent` / `CDirectionalLightComponent` 等自己管理 FBO 并绘制。

### 2.4 RenderSceneObject()（OpenGL，得到“场景 + 天空盒 + 可选 SSR”的结果）

渲染目标：`target_fbo == GL_NONE` 时用主 FBO `m_renderToBuffer`，否则用传入的 FBO。  
输出写入 `m_renderToTextures[0..2]` 等，即 `RenderResultInfo.resultColor` 等。

顺序：

1. **关闭混合**，viewport 设为窗口大小，`render_result_info.frameBuffer = m_renderToBuffer`。
2. **延迟渲染**：`m_deferRenderSystem.Draw(...)`  
   - 几何 Pass：写 G-Buffer（position/normal/albedo 等）到多渲染目标。  
   - 光照 Pass：读 G-Buffer，输出到 `resultColor`（及 resultPosition 等）。
3. **开启混合**（`GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`）。
4. **非延迟物体**：`RenderNonDeferSceneObjects()`  
   - 遍历 Actor，对非延迟、非地形、非水体的 Mesh 用各自 shader 绘制（如需要会设置 `b_render_skybox` 等）。
5. **天空盒**：`m_skyboxRenderSystem.Draw(...)`，画到当前 FBO。
6. **可选 SSR**：若 `use_screen_space_reflection`，`m_ssReflectionRenderSystem.Draw(...)`，结果合入后处理用的附件（如第三个 color attachment）。

之后该帧的 `latestRenderResult` 再交给水体、后处理使用。

### 2.5 小结：OpenGL 一帧数据流

```
阴影 Pass（各光源 FBO）
        ↓
主 FBO ← 延迟几何 + 光照
        ↓
主 FBO ← 非延迟物体 + 天空盒 + [SSR]
        ↓
主 FBO ← 水体
        ↓
后处理（读主 FBO，可写回主 FBO 或默认帧缓冲）
        ↓
ImGui
        ↓
glfwSwapBuffers
```

---

## 三、Vulkan 渲染流程

### 3.1 初始化（KongRenderModule::Init，RENDER_IN_VULKAN 分支）

1. 创建 **DescriptorPool**（UBO、SampledImage、CombinedImageSampler、InputAttachment、StorageImage 等）。
2. **CreateCommandBuffers()**：为 `MAX_FRAMES_IN_FLIGHT` 分配 primary command buffer。
3. **RecreateSwapChain()**：创建或重建 SwapChain（含 RenderPass、Framebuffer）。
4. **InitUBO**：  
   - 全局 descriptor set layout（set 0：Uniform Buffer）。  
   - 每帧一个 `VulkanBuffer` 的 UBO，写入 `GlobalVulkanUbo`（projection、view、cameraPosition、sceneLightInfo）。  
   - 每帧一个 `VkDescriptorSet`（m_descriptorSets）。
5. 创建各 Vulkan RenderSystem（顺序与依赖）：
   - `m_vkShadowMapSystem`（VkShadowMapRenderSystem）
   - 若 `VK_DEFER`：  
     - `m_vkDeferRenderSystem`（VkDeferRenderSystem），CreateMeshDescriptorSet  
     - Skybox 用 defer 的 color/depth 作为 input：`m_vkSkyboxSystem`（VulkanSkyBoxCreateInfo 里传 defer 的 GetColorTexture/GetDepthTexture）  
     - 后处理用 swapchain + descriptorPool + defer 的 color 等：`m_vkPostProcessSystem`  
   - 若非 VK_DEFER（简单前向）：  
     - `m_vkSimpleRenderSystem`，CreateMeshDescriptorSet  
     - Skybox / Postprocess 用 Simple 的 color、depth 作为输入，同理。

### 3.2 每帧 BeginFrame / Update / EndFrame

**BeginFrame()**

- `m_swapChain->AcquireNextImage(&m_currentImageIndex)`（若 OUT_OF_DATE 则 RecreateSwapChain）。
- `vkBeginCommandBuffer(m_commandBuffers[m_currentFrameIndex], ...)`。
- `m_isFrameStarted = true`。

**Update(delta)**（在已 BeginFrame 的前提下）

1. `mainCamera->Update(delta)`。
2. `UpdateSceneRenderInfo()`：收集光源、填 `SceneLightInfo`，写入当前帧的 **GlobalVulkanUbo**（每帧 UBO + Flush）。
3. `RenderShadowMap()`：  
   - OpenGL 分支里会调光源的 RenderShadowMap；Vulkan 下 **不** 在 RenderShadowMap() 里画阴影，而是由下面的 **m_vkShadowMapSystem->Draw(frameInfo)** 统一画。
4. 构造 `FrameInfo`（frameIndex, frameTime, commandBuffer）。
5. **按顺序录制到当前 command buffer**：
   - `m_vkShadowMapSystem->Draw(frameInfo)`  
     - 各光源的阴影贴图（平行光/点光）由该系统在独立 RenderPass 中绘制。
   - 若 `VK_DEFER`：  
     - `m_vkDeferRenderSystem->UpdateMeshUBO(frameInfo)`  
     - `m_vkDeferRenderSystem->Draw(frameInfo)`  
     - （几何 subpass → next subpass → 光照 subpass，画到 defer 的 offscreen FBO。）
   - 否则：  
     - `m_vkSimpleRenderSystem->UpdateMeshUBO(frameInfo)`  
     - `m_vkSimpleRenderSystem->Draw(frameInfo)`  
   - `m_vkSkyboxSystem->Draw(frameInfo)`  
     - 使用 defer/simple 的 color、depth 作为输入，渲染到自己的 FBO（或等效目标）。
   - `m_vkPostProcessSystem->Draw(frameInfo)`  
     - 使用 **SwapChain 的 RenderPass + Framebuffer**（即渲染到当前帧的 swapchain 图像）；  
     - 内部 `BeginRenderPass` 即开启“画到屏幕”的 pass，最后再画 ImGui（ImGui_ImplVulkan_RenderDrawData）。

**EndFrame()**

- `vkEndCommandBuffer(GetCurrentCommandBuffer())`。
- `m_swapChain->SubmitCommandBuffers(&commandBuffer, &m_currentImageIndex)`（提交 + Present）。
- `m_isFrameStarted = false`，`m_currentFrameIndex` 环增。

### 3.3 Vulkan 各 Pass 的 RenderPass / 目标

| 系统               | RenderPass / Framebuffer     | 输出目标           |
|--------------------|------------------------------|--------------------|
| VkShadowMapRenderSystem | 自建 RenderPass + FBO       | 阴影贴图           |
| VkDeferRenderSystem    | 自建 RenderPass + FBO（多附件） | G-Buffer + 光照结果（颜色 + 深度） |
| VkSkyBoxRenderSystem  | 自建 RenderPass + FBO        | 读 defer 的 color/depth，输出到自己的 FBO |
| VkPostprocessRenderSystem | **SwapChain 的 RenderPass** + SwapChain 的 Framebuffer | 当前帧 swapchain 图像（最终显示） |

注意：Vulkan 下 **没有** 在 Update 里显式调用 `BeginSwapChainRenderPass`；进入“画到 swapchain”的 pass 是在 **VkPostprocessRenderSystem::Draw** 里通过 `BeginRenderPass`（用 swapchain 的 renderPass 和 framebuffer）完成的。

### 3.4 小结：Vulkan 一帧数据流

```
BeginFrame: AcquireNextImage, vkBeginCommandBuffer
    ↓
Shadow Pass(es)     → 阴影贴图
    ↓
Defer Pass          → offscreen G-Buffer + 光照结果（color + depth）
    ↓
Skybox Pass         → 读 defer 结果，输出到 skybox FBO
    ↓
Postprocess Pass    → 读 defer 等，渲染到 SwapChain 图像（并画 ImGui）
    ↓
EndFrame: vkEndCommandBuffer, Submit, Present
```

---

## 四、两后端对比简表

| 环节           | OpenGL                               | Vulkan                                                                 |
|----------------|--------------------------------------|------------------------------------------------------------------------|
| 帧开始/结束    | 无 Begin/End，仅 Update 末尾 SwapBuffers | BeginFrame（Acquire + BeginCommandBuffer） / EndFrame（EndCommandBuffer + Submit + Present） |
| 阴影           | 每光源在自己的 FBO，在 RenderShadowMap() 里由光源组件绘制 | 统一由 m_vkShadowMapSystem->Draw() 在一个/多个 Pass 里绘制              |
| 主场景         | 延迟：DeferRenderSystem；非延迟：RenderNonDeferSceneObjects | 延迟：VkDeferRenderSystem（几何 + 光照 subpass）；或 SimpleRenderSystem |
| 天空盒         | GlSkyboxRenderSystem，画到主 FBO     | VulkanSkyBoxRenderSystem，读 defer/simple 的 color/depth，画到独立 FBO |
| 水体           | GlWaterRenderSystem，画到主 FBO      | 当前 Vulkan 路径下未接（可视为 TODO）                                  |
| SSR            | GlSSReflectionRenderSystem，合入主 FBO 的附件 | 当前 Vulkan 路径下未接（可视为 TODO）                                   |
| 后处理         | GlPostProcessRenderSystem，读主 FBO   | VkPostprocessRenderSystem，读 defer 结果，**渲染到 SwapChain**          |
| UI             | RenderUI(delta)，ImGui 画到当前绑定的 FBO/默认帧缓冲 | 在 VkPostprocessRenderSystem::Draw 内 ImGui_ImplVulkan_RenderDrawData   |

---

## 五、相关文件速查

| 用途           | OpenGL | Vulkan |
|----------------|--------|--------|
| 主循环 / 帧控制 | `app.cpp` | `app.cpp`（BeginFrame/EndFrame） + `RenderModule.cpp` |
| 阴影           | `LightComponent.*` 内 RenderShadowMap；RenderModule::RenderShadowMap | `VkShadowMapRenderSystem.*` |
| 延迟           | `GlDeferRenderSystem.*` | `VkDeferRenderSystem.*` |
| 天空盒         | `GlSkyboxRenderSystem.*` | `VkSkyBoxRenderSystem.*` |
| 后处理         | `GlPostProcessRenderSystem.*` | `VkPostprocessRenderSystem.*` |
| 场景绘制入口   | `RenderModule::RenderSceneObject()` | `RenderModule::Update()` 里各 system->Draw(frameInfo) |

以上即为当前引擎 OpenGL 与 Vulkan 的完整渲染流程梳理，便于后续改版或做统一抽象时对照。
