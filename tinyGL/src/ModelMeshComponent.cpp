#include "ModelMeshComponent.h"
#include "render.h"

using namespace glm;
using namespace tinyGL;

CModelMeshComponent::CModelMeshComponent(const SRenderResourceDesc& render_resource_desc)
	:CMeshComponent(render_resource_desc)
{
	ImportObj(render_resource_desc.model_path);
	// for(auto& mesh : mesh_list)
	// {
	// 	shader_data->SetupData(mesh);	
	// }
}

void CModelMeshComponent::Draw(const SSceneRenderInfo& scene_render_info)
{
	CMeshComponent::Draw(scene_render_info);
	//
	// if(render_info.index_buffer == GL_NONE)
	// {
	// 	if(render_info.instance_buffer != GL_NONE)
	// 	{
	// 		// Starting from vertex 0; 3 vertices total -> 1 triangle
	// 		glDrawArraysInstanced(GL_TRIANGLES, 0,
	// 			render_info.vertex_size / render_info.stride_count,
	// 			render_info.instance_count);
	// 	}
	// 	else
	// 	{
	// 		// Starting from vertex 0; 3 vertices total -> 1 triangle
	// 		glDrawArrays(GL_TRIANGLES, 0, render_info.vertex_size / render_info.stride_count); 	
	// 	}
	// }
	// else
	// {		
	// 	glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
	// }
}
