#include "BlendShader.h"

#include "Render/RenderModule.hpp"
#include "Scene.hpp"

using namespace Kong;

BlendShader::BlendShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/blend.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/blend.frag")},
    };
    shader_id = OpenGLShader::LoadShaders(shader_path_map);
    
    assert(shader_id, "Shader load failed!");
    bIsBlend = true;
    Use();
    SetInt("diffuse_texture", 0);
}

void BlendShader::UpdateRenderData(shared_ptr<RenderMaterialInfo> render_material)
{
    // 材质属性
    SetVec4("albedo", render_material->albedo);
    render_material->BindTextureByType(diffuse, 0);
}

