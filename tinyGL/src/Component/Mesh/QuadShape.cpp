#include "QuadShape.h"

#include "Render/RenderModule.hpp"
#ifdef RENDER_IN_VULKAN
#include "Render/GraphicsAPI/Vulkan/VulkanRenderInfo.hpp"
#endif
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
            shader_data->UpdateRenderData(override_render_info->material);
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
    // float quadVertices[] = {
    //     // positions         // normals         // texture Coords
    //     -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    //     -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
    //      1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    //      1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    // };

    std::vector<Vertex> quadVertexArray = {
        {{-1,1, 0}, {0.0, 0.0, 1.0}, {0.0, 1.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}},
        {{-1,-1, 0}, {0.0, 0.0, 1.0}, {0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}},
        {{1,1, 0}, {0.0, 0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}},
        {{1,-1, 0}, {0.0, 0.0, 1.0}, {1.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}}
    };
    
    mesh_resource = make_shared<MeshResource>();
    // mesh_resource->mesh_list.push_back(CMesh());
    auto quadMesh = make_shared<CMesh>();
    
#ifndef RENDER_IN_VULKAN
    auto vertex_buffer = make_unique<OpenGLBuffer>();
    vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(Vertex), quadVertexArray.size(), &quadVertexArray[0]);
    std::vector<OpenGLVertexAttribute> vertexAttributes = {
        {3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)},
        {3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)},
        {2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv)},
        {3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent)},
        {3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent)},
    };
    vertex_buffer->AddAttribute(vertexAttributes);
#else
    auto vertex_buffer = make_unique<VulkanBuffer>();
    vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(Vertex), quadVertexArray.size(), &quadVertexArray[0]);
    
#endif
    
    quadMesh->m_RenderInfo->vertex_buffer = std::move(vertex_buffer);
    mesh_resource->mesh_list.push_back(quadMesh);
}

