#include "BlendShader.h"

#include "render.h"
#include "Scene.h"

using namespace Kong;


void BlendShader::UpdateRenderData(const SMaterial& render_material, const SSceneLightInfo& scene_render_info)
{
    // 材质属性
    SetVec4("albedo", render_material.albedo);
    
    GLuint null_tex_id = CRender::GetNullTexId();
    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_tex_id = render_material.diffuse_tex_id != 0 ? render_material.diffuse_tex_id : null_tex_id;
    glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);
}

void BlendShader::InitDefaultShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/blend.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/blend.frag")},
    };
    shader_id = Shader::LoadShaders(shader_path_map);
    
    assert(shader_id, "Shader load failed!");
    bIsBlend = true;
    Use();
    SetInt("diffuse_texture", 0);
}
