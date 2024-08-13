#include "QuadShape.h"

#include "render.h"

using namespace tinyGL;
using namespace std;

CQuadShape::CQuadShape(const SRenderResourceDesc& render_resource_desc)
    : CMeshComponent(render_resource_desc)
{
    InitQuadData(render_resource_desc);
    
    // 屏幕mesh
    float quadVertices[] = {
        // positions         // normals         // texture Coords
        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    };
    
    auto& render_info = mesh_list[0].m_RenderInfo;
    // 初始化屏幕相关的 VAO
    glGenVertexArrays(1, &render_info.vertex_array_id);
    glGenBuffers(1, &render_info.vertex_buffer);
    glBindVertexArray(render_info.vertex_array_id);
    glBindBuffer(GL_ARRAY_BUFFER, render_info.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
}

void CQuadShape::Draw(const SSceneRenderInfo& scene_render_info)
{
    shader_data->Use();
    auto& render_info = mesh_list[0].m_RenderInfo;
    
    glBindVertexArray(render_info.vertex_array_id);
    shader_data->UpdateRenderData(mesh_list[0], scene_render_info);
   
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(GL_NONE);
}

void CQuadShape::InitQuadData(const SRenderResourceDesc& render_resource_desc)
{
    CMesh mesh;

    auto& render_info = mesh.m_RenderInfo;
    // // load texture map
    const auto& texture_paths = render_resource_desc.texture_paths;
    auto diffuse_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::diffuse);
    if (diffuse_path_iter != texture_paths.end())
    {
        render_info.diffuse_tex_id = CRender::LoadTexture(diffuse_path_iter->second);
    }

    auto specular_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::specular);
    if (specular_path_iter != texture_paths.end())
    {
        render_info.specular_tex_id = CRender::LoadTexture(specular_path_iter->second);
    }

    auto normal_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::normal);
    if (normal_path_iter != texture_paths.end())
    {
        render_info.normal_tex_id = CRender::LoadTexture(normal_path_iter->second);
    }

    auto tangent_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::tangent);
    if (tangent_path_iter != texture_paths.end())
    {
        render_info.tangent_tex_id = CRender::LoadTexture(tangent_path_iter->second);
    }

    render_info.material = render_resource_desc.material;

    mesh_list.push_back(mesh);

}
