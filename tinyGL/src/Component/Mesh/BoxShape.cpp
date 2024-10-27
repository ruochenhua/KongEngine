#include "BoxShape.h"

#include "render.h"
#include "Scene.h"

using namespace Kong;
using namespace std;
string CBoxShape::box_model_path = "Engine/box/box.obj";

CBoxShape::CBoxShape()
{
    ImportObj(CSceneLoader::ToResourcePath(box_model_path));
}

// LoadOverloadTexture(render_resource_desc);

void CBoxShape::Draw(const SSceneRenderInfo& scene_render_info)
{
    CMeshComponent::Draw(scene_render_info);
}

void CBoxShape::Draw()
{
    for(auto& mesh : mesh_resource->mesh_list)
    {
        auto& render_vertex = mesh.m_RenderInfo.vertex;
     
        glBindVertexArray(render_vertex.vertex_array_id);
        if(render_vertex.index_buffer == GL_NONE)
        {
            if(render_vertex.instance_buffer != GL_NONE)
            {
                // Starting from vertex 0; 3 vertices total -> 1 triangle
                glDrawArraysInstanced(GL_TRIANGLES, 0,
                    render_vertex.vertex_size / render_vertex.stride_count,
                    render_vertex.instance_count);
            }
            else
            {
                // Starting from vertex 0; 3 vertices total -> 1 triangle
                glDrawArrays(GL_TRIANGLES, 0, render_vertex.vertex_size / render_vertex.stride_count); 	
            }
        }
        else
        {
            if(render_vertex.instance_buffer != GL_NONE)
            {
                glDrawElementsInstanced(GL_TRIANGLES, render_vertex.indices_count, GL_UNSIGNED_INT, 0, render_vertex.instance_count);
            }
            else
            {
                glDrawElements(GL_TRIANGLES, render_vertex.indices_count, GL_UNSIGNED_INT, 0);
            }
        }
		
        glBindVertexArray(GL_NONE);	// 解绑VAO
    }
}
