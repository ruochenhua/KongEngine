#include "LightComponent.h"
#include "Component/Mesh/MeshComponent.h"
#include "Actor.h"
#include "Scene.h"
#include "glm/gtx/euler_angles.hpp"
#include "Shader/Shader.h"
#include "Shader/ShadowMapShader.h"

using namespace tinyGL;
using namespace glm;
const float SHADOWMAP_NEAR_PLANE = 1.0f;
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
    shadowmap_shader = dynamic_pointer_cast<DirectionalLightShadowMapShader>(ShaderManager::GetShader("directional_light_shadowmap"));
    assert(shadowmap_shader.get(), "fail to get shadow map shader");

    
    glGenFramebuffers(1, &shadowmap_fbo);
    glGenTextures(1, &shadowmap_texture);
    glBindTexture(GL_TEXTURE_2D, shadowmap_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat border_color[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	   
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowmap_texture, 0);
	   
    // 我们需要的只是在从光的透视图下渲染场景的时候深度信息，所以颜色缓冲没有用。
    // 然而，不包含颜色缓冲的帧缓冲对象是不完整的，所以我们需要显式告诉OpenGL我们不适用任何颜色数据进行渲染。
    // 我们通过将调用glDrawBuffer和glReadBuffer把读和绘制缓冲设置为GL_NONE来做这件事。
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::vec3 CDirectionalLightComponent::GetLightDir() const
{
    return light_dir;
}

void CDirectionalLightComponent::RenderShadowMap()
{
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
        mat4 light_proj = ortho(-20.f, 20.f, -20.f, 20.f, SHADOWMAP_NEAR_PLANE, SHADOWMAP_FAR_PLANE);
        
        vec3 light_pos = light_dir * -5.f;
        mat4 light_view = lookAt(light_pos, vec3(0,0,0), vec3(0, 1, 0));
        light_space_mat = light_proj * light_view;
        shadowmap_shader->SetMat4("light_space_mat", light_space_mat);
                
        for(auto& mesh : render_obj->mesh_list)
        {
            const SRenderInfo& render_info = mesh.GetRenderInfo();
            glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
		
            mat4 model_mat = actor->GetModelMatrix();
            shadowmap_shader->SetMat4("model", model_mat);
            //shadowmap_shader->UpdateShadowMapRender(GetLightDir(), model_mat);
            // Draw the triangle !
            // if no index, use draw array
            if(render_info.index_buffer == GL_NONE)
            {
                glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / render_info.stride_count); // Starting from vertex 0; 3 vertices total -> 1 triangle	
            }
            else
            {		
                glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
            }
        }
        glBindVertexArray(GL_NONE);	// 解绑VAO
    }
	
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void CDirectionalLightComponent::SetLightDir(const glm::vec3& rotation)
{
    glm::mat3 rotation_mat = glm::eulerAngleXYZ(radians(rotation.x), radians(rotation.y), radians(rotation.z));
    
    light_dir = normalize(rotation_mat * vec3(1,0,0));
}

CPointLightComponent::CPointLightComponent()
    : CLightComponent(ELightType::point_light)
{
    shadowmap_shader = dynamic_pointer_cast<PointLightShadowMapShader>(ShaderManager::GetShader("point_light_shadowmap"));
    assert(shadowmap_shader.get(), "fail to get shadow map shader");
    
    glGenFramebuffers(1, &shadowmap_fbo);
    // 创建点光源阴影贴图
    glGenTextures(1, &shadowmap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmap_texture);
    for(GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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


vec3 CPointLightComponent::GetLightDir() const
{
    // point light has no direction
    return vec3(0);
}

void CPointLightComponent::RenderShadowMap()
{
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
        
        for(auto& mesh : render_obj->mesh_list)
        {
            const SRenderInfo& render_info = mesh.GetRenderInfo();
            glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
		
            mat4 model_mat = actor->GetModelMatrix();
            // mat4 mvp = projection_mat * mainCamera->GetViewMatrix() * model_mat; //
            shadowmap_shader->UpdateShadowMapRender(GetLightLocation(), model_mat, vec2(near_plane, far_plane));
                        // Draw the triangle !
            // if no index, use draw array
            if(render_info.index_buffer == GL_NONE)
            {
                glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / render_info.stride_count); // Starting from vertex 0; 3 vertices total -> 1 triangle	
            }
            else
            {		
                glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
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
