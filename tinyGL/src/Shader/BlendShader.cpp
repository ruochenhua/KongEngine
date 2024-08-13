#include "BlendShader.h"

#include "render.h"

using namespace tinyGL;


void BlendShader::UpdateRenderData(const CMesh& mesh, const SSceneRenderInfo& scene_render_info)
{
    const SRenderInfo& render_info = mesh.GetRenderInfo();

    // 材质属性
    SetVec4("albedo", render_info.material.albedo);
    
    GLuint null_tex_id = CRender::GetNullTexId();
    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
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
    
    Use();
    SetInt("diffuse_texture", 0);
}
