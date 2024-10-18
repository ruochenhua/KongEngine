#include "LightComponent.h"
#include "Component/Mesh/MeshComponent.h"
#include "Actor.h"
#include "render.h"
#include "Scene.h"
#include "glm/gtx/euler_angles.hpp"
#include "Shader/Shader.h"
#include "Shader/ShadowMapShader.h"

using namespace Kong;
using namespace glm;
const float SHADOWMAP_NEAR_PLANE = 0.1f;
const float SHADOWMAP_FAR_PLANE = 30.0f;

CLightComponent::CLightComponent(ELightType in_type)
    : light_type(in_type)
{
   // shadowmap_shader = make_shared<ShadowMapShader>();
}

GLuint CLightComponent::GetShadowMapTexture() const
{
    return shadowmap_texture;
}

CDirectionalLightComponent::CDirectionalLightComponent()
    : CLightComponent(ELightType::directional_light)
{
    shadowmap_shader = ShaderManager::GetShader("directional_light_shadowmap");
    assert(shadowmap_shader.get(), "fail to get shadow map shader");
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
    if(!b_make_shadow)
    {
        return;
    }
#if USE_CSM
    light_space_matrices = GetLightSpaceMatrices();
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    auto actors = CScene::GetActors();
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
#if USE_CSM
        for(int i = 0; i < light_space_matrices.size(); ++i)
        {
            // if(i == 0)
            // {
            // mat4 light_proj = ortho(-20.f, 20.f, -20.f, 20.f, SHADOWMAP_NEAR_PLANE, SHADOWMAP_FAR_PLANE);
            //
            // vec3 light_pos = light_dir * -5.f;
            // mat4 light_view = lookAt(light_pos, vec3(0,0,0), vec3(0, 1, 0));
            // light_space_mat = light_proj * light_view;
            //    light_space_matrices[i] = light_space_mat;
            // }
            
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
        
        render_obj->SimpleDraw();
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
    b_make_shadow = b_turn_on;

    if(b_make_shadow)
    {
        glGenFramebuffers(1, &shadowmap_fbo);
        
        GLfloat border_color[] = {1.0, 1.0, 1.0, 1.0};
#if USE_CSM
        camera_near_far = CRender::GetNearFar();
        float far_plane = camera_near_far.y;
        // csm_distances = {far_plane/100};
        
        csm_distances = {far_plane/500, far_plane/250, far_plane/100, far_plane/50, far_plane/10};
        glGenTextures(1, &csm_texture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, csm_texture);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, SHADOW_RESOLUTION, SHADOW_RESOLUTION, (int)csm_distances.size()+1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);

        glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, csm_texture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
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
    auto camera = CRender::GetRender()->GetCamera();
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
    if(!b_make_shadow)
    {
        return;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    auto actors = CScene::GetActors();
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
            const SVertex& render_vertex = mesh.m_RenderInfo.vertex;
            glBindVertexArray(render_vertex.vertex_array_id);	// 绑定VAO
		
            mat4 model_mat = actor->GetModelMatrix();
            // mat4 mvp = projection_mat * mainCamera->GetViewMatrix() * model_mat; //
            UpdateShadowMapInfo(model_mat, vec2(SHADOWMAP_NEAR_PLANE, SHADOWMAP_FAR_PLANE));
                        // Draw the triangle !
            // if no index, use draw array
            if(render_vertex.index_buffer == GL_NONE)
            {
                glDrawArrays(GL_TRIANGLES, 0, render_vertex.vertex_size / render_vertex.stride_count); // Starting from vertex 0; 3 vertices total -> 1 triangle	
            }
            else
            {		
                glDrawElements(GL_TRIANGLES, render_vertex.indices_count, GL_UNSIGNED_INT, 0);
            }
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
    b_make_shadow = b_turn_on;
    if(b_make_shadow)
    {
        glGenFramebuffers(1, &shadowmap_fbo);
        // 创建点光源阴影贴图
        glGenTextures(1, &shadowmap_texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmap_texture);
        for(GLuint i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
    
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    

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
