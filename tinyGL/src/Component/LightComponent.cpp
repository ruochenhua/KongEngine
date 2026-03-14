#include "LightComponent.h"
#include "Component/Mesh/MeshComponent.h"
#include "Actor.hpp"
#include "Render/RenderModule.hpp"
#include "Scene.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "Shader/OpenGL/OpenGLShader.h"

using namespace Kong;
using namespace glm;
const float SHADOWMAP_NEAR_PLANE = 0.1f;
const float SHADOWMAP_FAR_PLANE = 30.0f;
struct VulkanShadowMapUbo
{
    // 后面改用csm会传入多个mat
    glm::mat4 light_space_mat;
};

CLightComponent::CLightComponent(ELightType in_type)
    : light_type(in_type)
{
   // shadowmap_shader = make_shared<ShadowMapShader>();
}

CLightComponent::~CLightComponent()
{
#ifdef RENDER_IN_VULKAN
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    vkDestroyFramebuffer(device, m_shadowFrameBuffer, nullptr);
#endif
}

GLuint CLightComponent::GetShadowMapTexture() const
{
    return shadowmap_texture;
}

CDirectionalLightComponent::CDirectionalLightComponent()
    : CLightComponent(ELightType::directional_light)
{
#ifndef RENDER_IN_VULKAN
    shadowmap_shader = ShaderManager::GetShader("directional_light_shadowmap");
    assert(shadowmap_shader.get(), "fail to get shadow map shader");
#endif
}


GLuint CDirectionalLightComponent::GetShadowMapTexture() const
{
#if USE_CSM
    return csm_texture;
#else
    return shadowmap_texture;
#endif
}

glm::vec3 CDirectionalLightComponent::GetLightDir() const
{
    return light_dir;
}

void CDirectionalLightComponent::RenderShadowMap()
{
    if(!enable_shadowmap)
    {
        return;
    }
#if USE_CSM
    light_space_matrices = GetLightSpaceMatrices();
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);

    
    auto actors = KongSceneManager::GetActors();
    for(auto actor : actors)
    {
        auto render_obj = actor->GetComponent<CMeshComponent>();
        if(!render_obj)
        {
            continue;
        }
        // 光源actor里面的mesh就不要渲染shadowmap了
        auto light_component = actor->GetComponent<CLightComponent>();
        if(light_component)
        {
            continue;
        }

        shadowmap_shader->Use();
        mat4 model_mat = actor->GetModelMatrix();
        shadowmap_shader->SetMat4("model", model_mat);
        shadowmap_shader->SetFloat("light_intensity", light_intensity);
#if USE_CSM
        for(int i = 0; i < light_space_matrices.size(); ++i)
        {
            stringstream ss;
            ss << "light_space_matrix[" << i << "]";
            shadowmap_shader->SetMat4(ss.str(), light_space_matrices[i]);
        }
        
#else
        mat4 light_proj = ortho(-20.f, 20.f, -20.f, 20.f, SHADOWMAP_NEAR_PLANE, SHADOWMAP_FAR_PLANE);
        
        vec3 light_pos = light_dir * -5.f;
        mat4 light_view = lookAt(light_pos, vec3(0,0,0), vec3(0, 1, 0));
        light_space_mat = light_proj * light_view;
        
        shadowmap_shader->SetMat4("light_space_mat[0]", light_space_mat);
#endif
        
        render_obj->DrawShadowInfo(shadowmap_shader);
    }
    // 渲染rsm信息
    if(enable_rsm)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, rsm_fbo);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        auto actors = KongSceneManager::GetActors();
        for(auto actor : actors)
        {
            auto render_obj = actor->GetComponent<CMeshComponent>();
            if(!render_obj)
            {
                continue;
            }
            // 光源actor里面的mesh就不要渲染shadowmap了
            auto light_component = actor->GetComponent<CLightComponent>();
            if(light_component)
            {
                continue;
            }

            rsm_shader->Use();
            mat4 model_mat = actor->GetModelMatrix();
            rsm_shader->SetMat4("model", model_mat);
            rsm_shader->SetFloat("light_intensity", light_intensity);
            
            mat4 light_proj = ortho(-20.f, 20.f, -20.f, 20.f, SHADOWMAP_NEAR_PLANE, SHADOWMAP_FAR_PLANE);
            vec3 light_pos = light_dir * -10.f;    // 用相机位置更新平行光的位置
            mat4 light_view = lookAt(light_pos, vec3(0,0,0), vec3(0, 1, 0));
            light_space_mat = light_proj * light_view;
            rsm_shader->SetMat4("light_space_mat", light_space_matrices[0]);
        
            render_obj->DrawShadowInfo(rsm_shader);
        }
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void CDirectionalLightComponent::SetLightDir(const glm::vec3& rotation)
{
    light_dir.x = sin(radians(rotation.x)) * cos(radians(rotation.y));
    light_dir.y = sin(radians(rotation.x)) * sin(radians(rotation.y));
    light_dir.z = cos(radians(rotation.x));
    light_dir = normalize(light_dir);
}

void CDirectionalLightComponent::TurnOnShadowMap(bool b_turn_on)
{
    enable_shadowmap = b_turn_on;

    if(enable_shadowmap)
    {
        glGenFramebuffers(1, &shadowmap_fbo);
        
        GLfloat border_color[] = {1.0, 1.0, 1.0, 1.0};
#if USE_CSM
        camera_near_far = KongRenderModule::GetNearFar();
        float far_plane = camera_near_far.y;
        // csm_distances = {far_plane/100};
        
        csm_distances = {far_plane/300, far_plane/100, far_plane/50, far_plane/10};
        Texture3DCreateInfo tex_create_info {
            GL_TEXTURE_2D_ARRAY, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT,
            SHADOW_RESOLUTION, SHADOW_RESOLUTION,
            GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER,
            GL_NEAREST, GL_NEAREST, nullptr, static_cast<int>(csm_distances.size() + 1)
        };
        TextureBuilder::CreateTexture3D(csm_texture, tex_create_info);

        glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, csm_texture, 0);
        // glDrawBuffer(GL_NONE);
        // glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#else
        
        glGenTextures(1, &shadowmap_texture);
        glBindTexture(GL_TEXTURE_2D, shadowmap_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	   
        glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowmap_texture, 0);
	   
        // 我们需要的只是在从光的透视图下渲染场景的时候深度信息，所以颜色缓冲没有用。
        // 然而，不包含颜色缓冲的帧缓冲对象是不完整的，所以我们需要显式告诉OpenGL我们不适用任何颜色数据进行渲染。
        // 我们通过将调用glDrawBuffer和glReadBuffer把读和绘制缓冲设置为GL_NONE来做这件事。
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

#endif
        
    }
    else
    {
        // 删掉阴影资源
        glDeleteBuffers(1, &shadowmap_fbo);
        glDeleteTextures(1, &shadowmap_texture);
    }
}

void CDirectionalLightComponent::TurnOnReflectiveShadowMap(bool b_turn_on)
{
    enable_rsm = b_turn_on;
    
    if(enable_shadowmap && enable_rsm)
    {
        map<EShaderType, string> shader_path_map = {
            {EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadow/reflective_shadowmap.vert")},
            {EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadow/reflective_shadowmap.frag")}
        };
        rsm_shader = make_shared<OpenGLShader>(shader_path_map);
        
        glGenFramebuffers(1, &rsm_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, rsm_fbo);

        TextureCreateInfo texture_create_info {
            GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT,
            SHADOW_RESOLUTION, SHADOW_RESOLUTION,
            GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
            GL_NEAREST, GL_NEAREST
        };
        // 位置数据
        TextureBuilder::CreateTexture(rsm_world_position, texture_create_info);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rsm_world_position, 0);
        
        // 法线数据
        TextureBuilder::CreateTexture(rsm_world_normal, texture_create_info);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, rsm_world_normal, 0);
        
        // flux数据
        TextureBuilder::CreateTexture(rsm_world_flux, texture_create_info);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, rsm_world_flux, 0);

        // 生成renderbuffer
        glGenRenderbuffers(1, &rsm_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, rsm_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rsm_depth);
        glEnable(GL_DEPTH_TEST);
        
        GLuint g_attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2}; 
        glDrawBuffers(3, g_attachments);
        // Check that the framebuffer is complete.
        if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
            printf("[Deferred Rendering] Framebuffer not complete!");
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    }
}
#ifdef RENDER_IN_VULKAN
void CDirectionalLightComponent::InitShadowMap(VkRenderPass renderPass,
            VulkanDescriptorPool* descriptorPool,
            const std::vector<std::unique_ptr<VulkanDescriptorSetLayout>>& descriptorSetLayout)
{
    // 创建深度图像
    CreateTextures();
    // 创建framebuffer,依赖传入render pass
    CreateFramebuffer(renderPass);
    CreateDescriptorBuffer();
    // 创建descriptor set,依赖传入set layout和descriptor pool
    // CreateDescriptorSet(descriptorSetLayout, descriptorPool);
}

void CDirectionalLightComponent::RenderShadowMap(const FrameInfo& frameInfo, VkPipelineLayout pipelineLayout)
{
    auto camera = KongRenderModule::GetRenderModule().GetCamera();
    auto camera_pos = camera->GetPosition();
    vec3 center_pos = vec3(0, 0, 0);
    // center_pos = camera_pos;
    mat4 light_proj = ortho(-20.f, 20.f, -20.f, 20.f, SHADOWMAP_NEAR_PLANE, SHADOWMAP_FAR_PLANE);
                                      
    vec3 light_pos = light_dir * -10.f + center_pos;
    // !注意up的反向
    mat4 light_view = lookAt(light_pos, center_pos, vec3(0, -1, 0));
    light_space_mat = light_proj * light_view;
    
    auto actors = KongSceneManager::GetActors();
    for (auto actor : actors)
    {
        auto mesh_component = actor->GetComponent<CMeshComponent>();
        if (!mesh_component)
        {
            continue;
        }

        auto mesh_shader = mesh_component->shader_data;
        VkModelRenderSystem::SimplePushConstantData pushData {actor->GetModelMatrix()};

        vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(pushData), &pushData);
        
        // VulkanShadowMapUbo ubo{};
        // // camera_near_far = KongRenderModule::GetNearFar();
        // // light_space_mat = CalLightSpaceMatrix(camera_near_far.x, camera_near_far.y);        
        // ubo.light_space_mat = light_space_mat;
        // for (const auto& uniformBuffer : m_uniformBuffers)
        // {
        //     uniformBuffer->WriteToBuffer(&ubo);
        //     uniformBuffer->Flush();
        // }
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &KongRenderModule::GetRenderModule().m_descriptorSets[frameInfo.frameIndex]
                , 0, nullptr);
        
        // vkCmdBindDescriptorSets(
        //     frameInfo.commandBuffer,
        //     VK_PIPELINE_BIND_POINT_GRAPHICS,
        //     pipelineLayout,
        //     0, 1,
        //     
        //     &m_descriptorSets[frameInfo.frameIndex][VulkanDescriptorSetLayout::Default]
        //         , 0, nullptr);
        
        mesh_component->DrawShadow(frameInfo, pipelineLayout);
    }
}

void CDirectionalLightComponent::ConvertDepthTextureLayout(const FrameInfo& frameInfo)
{
    // 创建图像内存屏障
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = m_depthTexture->m_image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    // 调用 vkCmdPipelineBarrier
    vkCmdPipelineBarrier(
        frameInfo.commandBuffer,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier
    );
}

#endif
std::vector<glm::vec4> CDirectionalLightComponent::GetFrustumCornersWorldSpace(const glm::mat4& proj_view)
{
    const auto inv = glm::inverse(proj_view);

    // 顶点的世界坐标在projection和view matrix的转换下的坐标范围是[-1,1]
    // 那么将在[-1,1]这个边界的八个顶点坐标乘以projection和view matrix的逆矩阵则可以得到视锥体边界的顶点的世界坐标
    vector<vec4> frustum_corners;
    for(unsigned int i = 0; i < 2; i++)
    {
        for(unsigned int j = 0; j < 2; j++)
        {
            for(unsigned int k = 0; k < 2; k++)
            {
                const vec4 pt = inv * vec4(2.0f*i-1.0f,2.0f*j-1.0f,2.0f*k-1.0f, 1.0f);
                frustum_corners.push_back(pt / pt.w);
            }
        }   
    }
    
    return frustum_corners;
}

mat4 CDirectionalLightComponent::CalLightSpaceMatrix(float near, float far)
{
    auto camera = KongRenderModule::GetRenderModule().GetCamera();
    float aspect_ratio = camera->m_screenInfo._aspect_ratio;
    float fov = camera->m_screenInfo._fov;

    const auto camera_view = camera->GetViewMatrix();
    // 获取小段的视锥体投影矩阵
    const auto proj = perspective(fov, aspect_ratio, near, far);
    const auto corners = GetFrustumCornersWorldSpace(proj * camera_view);

    vec3 center = vec3(0.0f);
    for(const auto& v : corners)
    {
        center += vec3(v);
    }
    center /= corners.size();   // 获取视锥体的中心点

    const auto light_view = lookAt(center-light_dir, center, vec3(0.0f, 1.0f, 0.0f));
    float min_x = std::numeric_limits<float>::max();
    float min_y = std::numeric_limits<float>::max();
    float min_z = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    float max_y = std::numeric_limits<float>::lowest();
    float max_z = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        const auto trf = light_view * v;
        min_x = std::min(min_x, trf.x);
        max_x = std::max(max_x, trf.x);
        min_y = std::min(min_y, trf.y);
        max_y = std::max(max_y, trf.y);
        min_z = std::min(min_z, trf.z);
        max_z = std::max(max_z, trf.z);
    }
    constexpr float z_mult = 10.0f;
    if (min_z < 0)
    {
        min_z *= z_mult;
    }
    else
    {
        min_z /= z_mult;
    }
    if (max_z < 0)
    {
        max_z /= z_mult;
    }
    else
    {
        max_z *= z_mult;
    }
    // mat4 light_proj = ortho(-20.f, 20.f, -20.f, 20.f, SHADOWMAP_NEAR_PLANE, SHADOWMAP_FAR_PLANE);
        
    const mat4 light_projection = ortho(min_x, max_x, min_y, max_y, min_z, max_z);
    return light_projection * light_view;
}

std::vector<glm::mat4> CDirectionalLightComponent::GetLightSpaceMatrices()
{
    vector<mat4> ret;
    for(int i = 0; i < csm_distances.size() + 1; ++i)
    {
        if(i == 0)
        {
            ret.push_back(CalLightSpaceMatrix(camera_near_far.x, csm_distances[i]));
        }
        else if (i < csm_distances.size())
        {
            ret.push_back(CalLightSpaceMatrix(csm_distances[i-1], csm_distances[i]));
        }
        else
        {
            ret.push_back(CalLightSpaceMatrix(csm_distances[i-1], camera_near_far.y));
        }
    }
    return ret;
}

#ifdef RENDER_IN_VULKAN
void CDirectionalLightComponent::CreateDescriptorSet(const std::vector<std::unique_ptr<VulkanDescriptorSetLayout>>& descriptorSetLayout, VulkanDescriptorPool* descriptorPool)
{
    m_descriptorSets.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorSet newSet;
        auto bufferInfo = m_uniformBuffers[i]->DescriptorInfo();
        VulkanDescriptorWriter(*descriptorSetLayout[0], *descriptorPool)
        .WriteBuffer(0, &bufferInfo)
        .Build(newSet);
        m_descriptorSets[i].emplace(VulkanDescriptorSetLayout::DescriptorSetLayoutUsageType::Default, newSet);
    }
}

void CDirectionalLightComponent::CreateFramebuffer(VkRenderPass renderPass)
{
    auto device = VulkanGraphicsDevice::GetGraphicsDevice()->GetDevice();
    
    VkFramebufferCreateInfo framebufferInfo = {};

    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &m_depthTexture->m_imageView;
    // 按照阴影贴图设置大小创建
    framebufferInfo.width = SHADOW_RESOLUTION;
    framebufferInfo.height = SHADOW_RESOLUTION;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
        device,
        &framebufferInfo,
        nullptr,
        &m_shadowFrameBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer");
    }
}

void CDirectionalLightComponent::CreateTextures()
{
    // 创建深度贴图
    VkFormat depthFormat = KongRenderModule::GetRenderModule().GetSwapChain()->FindDepthFormat();
    
    VkImageCreateInfo depthImageInfo = {};
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.extent.width = SHADOW_RESOLUTION;
    depthImageInfo.extent.height = SHADOW_RESOLUTION;
    depthImageInfo.extent.depth = 1;
    
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.format = depthFormat;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;   // 为了渲染深度图，需要增加采样图像用法
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depthImageInfo.flags = 0;
    
    m_depthTexture = make_unique<VulkanTexture>(depthImageInfo);
    m_depthTexture->CreateImageView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    // m_depthTexture->CreateTextureSampler();
    m_depthTexture->CreateDepthTextureSampler();
}

void CDirectionalLightComponent::CreateDescriptorBuffer()
{
    m_uniformBuffers = VulkanRenderSystem::CreateDescriptorBuffer<VulkanShadowMapUbo>();
}
#endif

CPointLightComponent::CPointLightComponent()
    : CLightComponent(ELightType::point_light)
{
    shadowmap_shader = ShaderManager::GetShader("point_light_shadowmap");
    assert(shadowmap_shader.get(), "fail to get shadow map shader");
}


vec3 CPointLightComponent::GetLightDir() const
{
    // point light has no direction
    return vec3(0);
}

void CPointLightComponent::RenderShadowMap()
{
    if(!enable_shadowmap)
    {
        return;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    auto actors = KongSceneManager::GetActors();
    for(auto actor : actors)
    {
        auto render_obj = actor->GetComponent<CMeshComponent>();
        if(!render_obj)
        {
            continue;
        }
        // 光源actor里面的mesh就不要渲染shadowmap了
        auto light_component = actor->GetComponent<CLightComponent>();
        if(light_component)
        {
            continue;
        }
        
        for(auto& mesh : render_obj->mesh_resource->mesh_list)
        {
		
            mat4 model_mat = actor->GetModelMatrix();
            // mat4 mvp = projection_mat * mainCamera->GetViewMatrix() * model_mat; //
            UpdateShadowMapInfo(model_mat, vec2(SHADOWMAP_NEAR_PLANE, SHADOWMAP_FAR_PLANE));
                        // Draw the triangle !
            // if no index, use draw array
            
            mesh->m_RenderInfo->Draw(nullptr);
            // glBindVertexArray(render_vertex.vertex_array_id);	// 绑定VAO
            // if(!mesh->m_RenderInfo->index_buffer)
            // {
            //     glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size()); // Starting from vertex 0; 3 vertices total -> 1 triangle	
            // }
            // else
            // {		
            //     glDrawElements(GL_TRIANGLES, mesh->m_Index.size(), GL_UNSIGNED_INT, 0);
            // }
        }
        glBindVertexArray(GL_NONE);	// 解绑VAO
    }
	
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

glm::vec3 CPointLightComponent::GetLightLocation() const
{
    return light_location;
}

void CPointLightComponent::SetLightLocation(const glm::vec3& in_light_location)
{
    light_location = in_light_location;
}

void CPointLightComponent::TurnOnShadowMap(bool b_turn_on)
{
    enable_shadowmap = b_turn_on;
    if(enable_shadowmap)
    {
        glGenFramebuffers(1, &shadowmap_fbo);
        // 创建点光源阴影贴图
        TextureCreateInfo shadowmap_tex_create_info {
            GL_TEXTURE_CUBE_MAP, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT,
            SHADOW_RESOLUTION, SHADOW_RESOLUTION, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
            GL_NEAREST, GL_NEAREST
        };

        TextureBuilder::CreateTexture(shadowmap_texture, shadowmap_tex_create_info);
        
        glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmap_texture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        if(shadowmap_fbo) glDeleteFramebuffers(1, &shadowmap_fbo);
        if(shadowmap_texture) glDeleteTextures(1, &shadowmap_texture);

        shadowmap_fbo = shadowmap_texture = 0;
    }
}

void CPointLightComponent::UpdateShadowMapInfo(const mat4& model_mat, const vec2& near_far_plane)
{
    // 点光源的阴影贴图
    GLfloat aspect = (GLfloat)SHADOW_RESOLUTION / (GLfloat)SHADOW_RESOLUTION;
    float near_plane = near_far_plane.x;
    float far_plane = near_far_plane.y;
    mat4 shadow_proj = perspective(radians(90.f), aspect, near_plane, far_plane);
    
    // 方向可以固定是朝向六个方向
    vector<mat4> shadow_transforms;
    shadow_transforms.push_back(shadow_proj * lookAt(light_location, light_location+vec3(1,0,0), vec3(0,-1,0)));
    shadow_transforms.push_back(shadow_proj * lookAt(light_location, light_location+vec3(-1,0,0), vec3(0,-1,0)));
    shadow_transforms.push_back(shadow_proj * lookAt(light_location, light_location+vec3(0,1,0), vec3(0,0,1)));
    shadow_transforms.push_back(shadow_proj * lookAt(light_location, light_location+vec3(0,-1,0), vec3(0,0,-1)));
    shadow_transforms.push_back(shadow_proj * lookAt(light_location, light_location+vec3(0,0,1), vec3(0,-1,0)));
    shadow_transforms.push_back(shadow_proj * lookAt(light_location, light_location+vec3(0,0,-1), vec3(0,-1,0)));

    shadowmap_shader->Use();
    shadowmap_shader->SetFloat("far_plane", far_plane);
    for(int i = 0; i < 6; ++i)
    {
        stringstream shadow_matrices_stream;
        shadow_matrices_stream <<  "shadow_matrices[" << i << "]";
        shadowmap_shader->SetMat4(shadow_matrices_stream.str(), shadow_transforms[i]);
    }
    shadowmap_shader->SetVec3("light_pos", light_location);
    // RenderScene();
    shadowmap_shader->SetMat4("model", model_mat);
}
