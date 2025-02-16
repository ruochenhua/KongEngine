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
    shader_id = Shader::LoadShaders(shader_path_map);
    
    assert(shader_id, "Shader load failed!");
    bIsBlend = true;
    Use();
    SetInt("diffuse_texture", 0);
}

void BlendShader::UpdateRenderData(const SMaterialInfo& render_material)
{
    // 材质属性
    SetVec4("albedo", render_material.albedo);
    
    GLuint null_tex_id = KongRenderModule::GetNullTexId();
    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_tex_id = render_material.diffuse_tex_id != 0 ? render_material.diffuse_tex_id : null_tex_id;
    glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);
}

