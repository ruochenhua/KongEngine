#include "Water.h"

#include "Scene.h"

Kong::Water::Water()
{
    InitRenderInfo();

    map<EShaderType, string> shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/water/water.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/water/water.frag")}
    };

    shader_data = make_shared<Shader>(shader_path_map);

    shader_data->Use();
    shader_data->SetInt("scene_texture", 0);
}

void Kong::Water::SimpleDraw(shared_ptr<Shader> simple_draw_shader)
{
    CQuadShape::SimpleDraw(simple_draw_shader);
}

void Kong::Water::Draw(const SSceneLightInfo& scene_render_info)
{
    shader_data->Use();
    auto& render_info = mesh_resource->mesh_list[0].m_RenderInfo.vertex;
    
    glBindVertexArray(render_info.vertex_array_id);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(GL_NONE);
}
