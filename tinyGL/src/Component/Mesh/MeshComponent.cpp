#include "MeshComponent.h"
//#include "OBJ_Loader.h"

#include "Render/RenderModule.hpp"
#include "Shader/Shader.h"

#include "glm/gtc/random.hpp"
#include "Parser/ResourceManager.h"

using namespace Kong;
using namespace glm;

void CMeshComponent::BeginPlay()
{
	CComponent::BeginPlay();
	InitRenderInfo();
}

void CMeshComponent::DrawShadowInfo(shared_ptr<Shader> simple_draw_shader)
{
	for(auto& mesh : mesh_resource->mesh_list)
	{
		auto& render_vertex = mesh->m_RenderInfo.vertex;
		if(simple_draw_shader)
		{
			if(use_override_material)
			{
				simple_draw_shader->SetVec4("albedo", override_render_info.material.albedo);	
			}
			else
			{
				simple_draw_shader->SetVec4("albedo", mesh->m_RenderInfo.material.albedo);		
			}
			
		}
		glBindVertexArray(render_vertex.vertex_array_id);
		// Draw the triangle !
		// if no index, use draw array
		if(!mesh->index_buffer)
		{
			if(render_vertex.instance_buffer != GL_NONE)
			{
				// Starting from vertex 0; 3 vertices total -> 1 triangle
				glDrawArraysInstanced(GL_TRIANGLES, 0,
					mesh->vertices.size(),render_vertex.instance_count);
			}
			else
			{
				// Starting from vertex 0; 3 vertices total -> 1 triangle
				glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size()); 	
			}
		}
		else
		{
			if(render_vertex.instance_buffer != GL_NONE)
			{
				glDrawElementsInstanced(GL_TRIANGLES, mesh->m_Index.size(), GL_UNSIGNED_INT, 0, render_vertex.instance_count);
			}
			else
			{
				glDrawElements(GL_TRIANGLES, mesh->m_Index.size(), GL_UNSIGNED_INT, 0);
			}
		}
		
		glBindVertexArray(GL_NONE);	// 解绑VAO
	}
}

void CMeshComponent::Draw(void* commandBuffer)
{

	if (shader_data)
		shader_data->Use();
	
	for(auto& mesh : mesh_resource->mesh_list)
	{
		auto& render_vertex = mesh->m_RenderInfo.vertex;
		
		glBindVertexArray(render_vertex.vertex_array_id);
		if (shader_data)
		{
			if(use_override_material)
			{
				shader_data->UpdateRenderData(override_render_info.material);
			}
			else
			{
				shader_data->UpdateRenderData(mesh->m_RenderInfo.material);
			}
		}
		// Draw the triangle !
		// if no index, use draw array
		if(!mesh->index_buffer)
		{
			if(render_vertex.instance_buffer != GL_NONE)
			{
				// Starting from vertex 0; 3 vertices total -> 1 triangle
				glDrawArraysInstanced(GL_TRIANGLES, 0,
					mesh->vertices.size(),render_vertex.instance_count);
			}
			else
			{
				// Starting from vertex 0; 3 vertices total -> 1 triangle
				glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size()); 	
			}
		}
		else
		{
			if(render_vertex.instance_buffer != GL_NONE)
			{
				glDrawElementsInstanced(GL_TRIANGLES, mesh->m_Index.size(), GL_UNSIGNED_INT, 0, render_vertex.instance_count);
			}
			else
			{
				glDrawElements(GL_TRIANGLES, mesh->m_Index.size(), GL_UNSIGNED_INT, 0);
			}
		}
		
		glBindVertexArray(GL_NONE);	// 解绑VAO
	}
}

void CMeshComponent::InitRenderInfo()
{
	// compile shader map
	for(auto mesh : mesh_resource->mesh_list)
	{
		// 构建默认的shader数据结构，数据齐全，但是冗余
		auto& render_vertex = mesh->m_RenderInfo.vertex;
		std::vector<Vertex>& mesh_vertices = mesh->vertices;

#ifdef RENDER_IN_VULKAN
		mesh->vertex_buffer = make_unique<VulkanBuffer>();
		mesh->vertex_buffer->Initialize(VERTEX_BUFFER, sizeof(Vertex), mesh_vertices.size(), &mesh_vertices[0]);
		
		std::vector<unsigned int> indices = mesh->m_Index;
		if(!indices.empty())
		{
			mesh->index_buffer = make_unique<VulkanBuffer>();
			mesh->index_buffer->Initialize(INDEX_BUFFER, sizeof(unsigned int), indices.size(), &indices[0]);
		}
		
#else
		glGenVertexArrays(1, &render_vertex.vertex_array_id);
		glBindVertexArray(render_vertex.vertex_array_id);
		
		// //init vertex buffer
		mesh->vertex_buffer.Initialize(VERTEX_BUFFER, sizeof(Vertex), mesh_vertices.size(), &mesh_vertices[0]);
		
		//vertex buffer
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);

		//normal buffer
		glVertexAttribPointer(1, 3,GL_FLOAT,GL_FALSE,sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);

		// texcoord
		glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex, uv));
		glEnableVertexAttribArray(2);

		// tangent
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glEnableVertexAttribArray(3);
		
		// bitangent
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
		glEnableVertexAttribArray(4);

		// index buffer
		std::vector<unsigned int> indices = mesh->m_Index;
		if(!indices.empty())
		{
			mesh->index_buffer.Initialize(INDEX_BUFFER, sizeof(unsigned int), indices.size(), &indices[0]);
		}

		glBindVertexArray(GL_NONE);
#endif
	}
}

bool CMeshComponent::IsBlend()
{
	return shader_data->bIsBlend;
}

#ifdef RENDER_IN_VULKAN

void CMeshComponent::VkDraw(VkCommandBuffer commandBuffer)
{
	for (auto& mesh : mesh_resource->mesh_list)
	{

		if (!mesh->vertex_buffer || !mesh->index_buffer)
			continue;

		mesh->vertex_buffer->Bind(commandBuffer);
		if (mesh->index_buffer)
		{
			mesh->index_buffer->Bind(commandBuffer);

			std::vector<unsigned int> indices = mesh->m_Index;
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		}
		else
		{
			auto vertices = mesh->vertices;
			vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		}
	}
}
#endif

int CMeshComponent::ImportObj(const std::string& model_path)
{
	mesh_resource = ResourceManager::GetOrLoadMesh(model_path);
	
	return 0;
}

