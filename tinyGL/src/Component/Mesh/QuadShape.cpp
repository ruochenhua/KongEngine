#include "QuadShape.h"

#include "Render/RenderModule.hpp"

using namespace Kong;
using namespace std;

CQuadShape::CQuadShape()
{
    InitRenderInfo();
}

void CQuadShape::Draw()
{
    if (shader_data)
        shader_data->Use();
    auto& render_info = mesh_resource->mesh_list[0].m_RenderInfo.vertex;
    
    glBindVertexArray(render_info.vertex_array_id);
    if (shader_data)
    {
        if(use_override_material)
        {
            shader_data->UpdateRenderData(override_render_info.material);
        }
        else
        {
            shader_data->UpdateRenderData(mesh_resource->mesh_list[0].m_RenderInfo.material);
        }
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(GL_NONE);
}

void CQuadShape::InitRenderInfo()
{
    // 屏幕mesh
    float quadVertices[] = {
        // positions         // normals         // texture Coords
        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    };
    
    mesh_resource = make_shared<MeshResource>();
    mesh_resource->mesh_list.push_back(CMesh());
    
    auto& render_info = mesh_resource->mesh_list[0].m_RenderInfo.vertex;
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

void CQuadShape::BindVAO()
{
    glBindVertexArray(mesh_resource->mesh_list[0].m_RenderInfo.vertex.vertex_array_id);
}
