#include "Water.h"

#include "QuadShape.h"
#include "Render/RenderModule.hpp"
#include "Scene.hpp"
#ifdef RENDER_IN_VULKAN
#include "Render/GraphicsAPI/Vulkan/VulkanBuffer.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanRenderInfo.hpp"
#endif

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

void Water::InitRenderInfo()
{
    // 水面用 XZ 平面四边形（法线 +Y），与 CQuadShape 的 XY 平面区分，避免被背面剔除导致整片消失
    std::vector<Vertex> quadVertexArray = {
        {{-1.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}},
        {{ 1.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}},
        {{-1.f, 0.f,  1.f}, {0.f, 1.f, 0.f}, {0.f, 1.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}},
        {{ 1.f, 0.f,  1.f}, {0.f, 1.f, 0.f}, {1.f, 1.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}},
    };

    mesh_resource = make_shared<MeshResource>();
    auto quadMesh = make_shared<CMesh>();

#ifndef RENDER_IN_VULKAN
    auto vertex_buffer = make_unique<OpenGLBuffer>();
    vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(Vertex), quadVertexArray.size(), quadVertexArray.data());
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
    vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(Vertex), quadVertexArray.size(), quadVertexArray.data());
#endif

    quadMesh->m_RenderInfo->vertex_buffer = std::move(vertex_buffer);
    quadMesh->m_RenderInfo->vertices = quadVertexArray;
    mesh_resource->mesh_list.push_back(quadMesh);
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
