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
}

void Kong::Water::SimpleDraw(shared_ptr<Shader> simple_draw_shader)
{
    CQuadShape::SimpleDraw(simple_draw_shader);
}

void Kong::Water::Draw(const SSceneRenderInfo& scene_render_info)
{
    CQuadShape::Draw(scene_render_info);
}
