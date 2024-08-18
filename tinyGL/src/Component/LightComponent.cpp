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
    if(!shadowmap_shader)
    {
        return GL_NONE;
    }

    return shadowmap_shader->shadowmap_texture;
}

CDirectionalLightComponent::CDirectionalLightComponent()
    : CLightComponent(ELightType::directional_light)
{
    shadowmap_shader = dynamic_pointer_cast<DirectionalLightShadowMapShader>(ShaderManager::GetShader("directional_light_shadowmap"));
    assert(shadowmap_shader.get(), "fail to get shadow map shader");
}

glm::vec3 CDirectionalLightComponent::GetLightDir() const
{
    return light_dir;
}

void CDirectionalLightComponent::RenderShadowMap()
{
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_shader->shadowmap_fbo);
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
}


vec3 CPointLightComponent::GetLightDir() const
{
    // point light has no direction
    return vec3(0);
}

void CPointLightComponent::RenderShadowMap()
{
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_shader->shadowmap_fbo);
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
            shadowmap_shader->UpdateShadowMapRender(GetLightLocation(), model_mat);
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
