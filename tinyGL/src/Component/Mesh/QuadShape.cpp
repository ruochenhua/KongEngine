#include "QuadShape.h"

#include "Render/RenderModule.hpp"

using namespace Kong;
using namespace std;

CQuadShape::CQuadShape()
{
    InitRenderInfo();
}

void CQuadShape::Draw(void* commandBuffer)
{
    if (shader_data)
        shader_data->Use();
    
    if (shader_data)
    {
        if(use_override_material)
        {
            shader_data->UpdateRenderData(override_render_info.material);
        }
        else
        {
            shader_data->UpdateRenderData(mesh_resource->mesh_list[0]->m_RenderInfo->material);
        }
    }
    
    mesh_resource->mesh_list[0]->m_RenderInfo->vertex_buffer->Bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
    // mesh_resource->mesh_list.push_back(CMesh());
    auto quadMesh = make_shared<CMesh>();
    
#ifndef RENDER_IN_VULKAN
    auto vertex_buffer = make_unique<OpenGLBuffer>();
    vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(float), sizeof(quadVertices), &quadVertices);
    std::vector<OpenGLVertexAttribute> vertexAttributes = {
        {3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0},
        {3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float))},
        {2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float))},
    };
    vertex_buffer->AddAttribute(vertexAttributes);
#else
    auto vertex_buffer = make_unique<VulkanBuffer>();
    vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(float), sizeof(quadVertices), &quadVertices);
    
#endif
    
    quadMesh->m_RenderInfo->vertex_buffer = std::move(vertex_buffer);
    mesh_resource->mesh_list.push_back(quadMesh);
}

