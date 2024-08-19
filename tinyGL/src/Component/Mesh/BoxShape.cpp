#include "BoxShape.h"

#include "render.h"

using namespace tinyGL;
using namespace std;
string CBoxShape::box_model_path = "Engine/box/box.obj";

CBoxShape::CBoxShape(const SRenderResourceDesc& render_resource_desc)
    : CMeshComponent(render_resource_desc)
{
    ImportObj(CSceneLoader::ToResourcePath(box_model_path));
    LoadOverloadTexture(render_resource_desc);
}

void CBoxShape::Draw(const SSceneRenderInfo& scene_render_info)
{
    CMeshComponent::Draw(scene_render_info);
}

void CBoxShape::Draw()
{
    for(auto& mesh : mesh_list)
    {
        glBindVertexArray(mesh.m_RenderInfo.vertex_array_id);
        auto& render_info = mesh.m_RenderInfo;
        if(render_info.index_buffer == GL_NONE)
        {
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
        }
        else
        {
            if(render_info.instance_buffer != GL_NONE)
            {
                glDrawElementsInstanced(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0, render_info.instance_count);
            }
            else
            {
                glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
            }
        }
		
        glBindVertexArray(GL_NONE);	// 解绑VAO
    }
}
