#include "Water.h"

#include "render.h"
#include "Scene.h"

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
}

void Water::SimpleDraw(shared_ptr<Shader> simple_draw_shader)
{
    CQuadShape::SimpleDraw(simple_draw_shader);
}

void Water::Draw(const SSceneLightInfo& scene_render_info)
{
    shader_data->Use();
    auto& render_info = mesh_resource->mesh_list[0].m_RenderInfo.vertex;

    glActiveTexture(GL_TEXTURE2);   // 前面有reflection和refraction texture，这里从texture2开始
    glBindTexture(GL_TEXTURE_2D, dudv_texture > 0 ? dudv_texture : CRender::GetNullTexId());
    
    glBindVertexArray(render_info.vertex_array_id);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(GL_NONE);
}

void Water::LoadDudvMapTexture(const string& texture_path)
{
    dudv_texture = ResourceManager::GetOrLoadTexture(CSceneLoader::ToResourcePath(texture_path));
}