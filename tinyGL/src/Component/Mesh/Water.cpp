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

    shader_data = make_shared<OpenGLShader>(shader_path_map);

    shader_data->Use();
    shader_data->SetInt("reflection_texture", 0);
    shader_data->SetInt("refraction_texture", 1);
    shader_data->SetInt("dudv_map", 2);
    shader_data->SetInt("normal_map", 3);
}

void Water::DrawShadowInfo(shared_ptr<OpenGLShader> simple_draw_shader)
{
    CQuadShape::DrawShadowInfo(simple_draw_shader);
}

void Water::Draw(void* commandBuffer)
{
    shader_data->Use();
    auto& render_info = mesh_resource->mesh_list[0]->m_RenderInfo;

    if (auto tex = dudv_texture.lock()) tex->Bind(2);
    if (auto tex = normal_texture.lock()) tex->Bind(3);

    render_info->vertex_buffer->Bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Water::LoadDudvMapTexture(const string& texture_path)
{
    // 类型对opengl没区别
    dudv_texture = ResourceManager::GetOrLoadTexture_new(diffuse, CSceneLoader::ToResourcePath(texture_path));
}

void Water::LoadNormalTexture(const string& texture_path)
{
    // 类型对opengl没区别
    normal_texture = ResourceManager::GetOrLoadTexture_new(normal, CSceneLoader::ToResourcePath(texture_path));
}
