#include "BoxShape.h"

#include "render.h"
#include "Shader/BRDFShader.h"

using namespace tinyGL;
using namespace std;
vector<float> CBoxShape::s_vBoxVertices = {
    // positions          // normals           // texture coords
    0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,

    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,

    0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, 0.5f, 0.5f,

    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,

    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f
};

vector<float> CBoxShape::s_vBoxNormals = {
    // positions          // normals           // texture coords
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,

    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,

    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,

    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,

    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,

    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
};

vector<float> CBoxShape::s_vBoxTexCoords = {
    // positions          // normals           // texture coords
    1.0f, 0.0f,
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,

    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,

    1.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f
};

CBoxShape::CBoxShape(const SRenderResourceDesc& render_resource_desc)
    : CMeshComponent(render_resource_desc)
{
    InitBoxData(render_resource_desc);
    //InitRenderInfo(render_resource_desc);
}

void CBoxShape::Draw(const SSceneRenderInfo& scene_render_info)
{
    shader_data->Use();
    auto& render_info = mesh_list[0].m_RenderInfo;
    
    glBindVertexArray(render_info.vertex_array_id);
    shader_data->UpdateRenderData(mesh_list[0], scene_render_info);
    if(render_info.instance_buffer != GL_NONE)
    {
        // Starting from vertex 0; 3 vertices total -> 1 triangle
        glDrawArraysInstanced(GL_TRIANGLES, 0,
            render_info.vertex_size / render_info.stride_count,
            render_info.instance_count);
    }
    else
    {
        // Starting from vertex 0; 3 vertices total -> 1 triangle
        glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / render_info.stride_count); 	
    }
    glBindVertexArray(GL_NONE);	// 解绑VAO
}

void CBoxShape::InitBoxData(const SRenderResourceDesc& render_resource_desc)
{
    CMesh mesh;
    mesh.m_Vertex = s_vBoxVertices;
    mesh.m_Normal = s_vBoxNormals;
    mesh.m_TexCoord = s_vBoxTexCoords;
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
    

    render_info.material = render_resource_desc.material;

    mesh_list.push_back(mesh);

}
