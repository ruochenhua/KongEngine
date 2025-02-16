#include "Water.h"

#include "Render/RenderModule.hpp"
#include "Scene.hpp"

using namespace Kong;

Water::Water()
{
    InitRenderInfo();

    map<EShaderType, string> shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/water/water.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/water/water.frag")}
    };

    shader_data = make_shared<Shader>(shader_path_map);

    shader_data->Use();
    shader_data->SetInt("reflection_texture", 0);
    shader_data->SetInt("refraction_texture", 1);
    shader_data->SetInt("dudv_map", 2);
    shader_data->SetInt("normal_map", 3);
}

void Water::DrawShadowInfo(shared_ptr<Shader> simple_draw_shader)
{
    CQuadShape::DrawShadowInfo(simple_draw_shader);
}

void Water::Draw()
{
    shader_data->Use();
    auto& render_info = mesh_resource->mesh_list[0].m_RenderInfo.vertex;
#if USE_DSA
    glBindTextureUnit(2, dudv_texture > 0 ? dudv_texture : KongRenderModule::GetNullTexId());
    glBindTextureUnit(3, normal_texture > 0 ? normal_texture : KongRenderModule::GetNullTexId());
#else
    glActiveTexture(GL_TEXTURE2);   // 前面有reflection和refraction texture，这里从texture2开始
    glBindTexture(GL_TEXTURE_2D, dudv_texture > 0 ? dudv_texture : CRender::GetNullTexId());

    glActiveTexture(GL_TEXTURE3);   // 前面有reflection和refraction texture，这里从texture2开始
    glBindTexture(GL_TEXTURE_2D, normal_texture > 0 ? normal_texture : CRender::GetNullTexId());
#endif
    glBindVertexArray(render_info.vertex_array_id);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(GL_NONE);
}

void Water::LoadDudvMapTexture(const string& texture_path)
{
    dudv_texture = ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(texture_path));
}

void Water::LoadNormalTexture(const string& texture_path)
{
    normal_texture = ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(texture_path));
}
