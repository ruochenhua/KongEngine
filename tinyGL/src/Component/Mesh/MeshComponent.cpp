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
		auto& render_vertex = mesh.m_RenderInfo.vertex;
		if(simple_draw_shader)
		{
			if(use_override_material)
			{
				simple_draw_shader->SetVec4("albedo", override_render_info.material.albedo);	
			}
			else
			{
				simple_draw_shader->SetVec4("albedo", mesh.m_RenderInfo.material.albedo);		
			}
			
		}
		glBindVertexArray(render_vertex.vertex_array_id);
		// Draw the triangle !
		// if no index, use draw array
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

void CMeshComponent::Draw()
{
	if (shader_data)
		shader_data->Use();
	
	for(auto& mesh : mesh_resource->mesh_list)
	{
		auto& render_vertex = mesh.m_RenderInfo.vertex;
		
		glBindVertexArray(render_vertex.vertex_array_id);
		if (shader_data)
		{
			if(use_override_material)
			{
				shader_data->UpdateRenderData(override_render_info.material);
			}
			else
			{
				shader_data->UpdateRenderData(mesh.m_RenderInfo.material);
			}
		}
		// Draw the triangle !
		// if no index, use draw array
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

void CMeshComponent::InitRenderInfo()
{
	// compile shader map
	for(auto& mesh : mesh_resource->mesh_list)
	{
		// 构建默认的shader数据结构，数据齐全，但是冗余
		auto& render_vertex = mesh.m_RenderInfo.vertex;
		std::vector<Vertex> mesh_vertices = mesh.vertices;
		
		glGenVertexArrays(1, &render_vertex.vertex_array_id);
		glBindVertexArray(render_vertex.vertex_array_id);

		//init vertex buffer
		glGenBuffers(1, &render_vertex.vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_vertex.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh_vertices.size(), &mesh_vertices[0], GL_STATIC_DRAW);

		//vertex buffer
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			sizeof(Vertex),                  // stride
			(void*)offsetof(Vertex, position)       // array buffer offset
		);
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
		std::vector<unsigned int> indices = mesh.m_Index;
		if(!indices.empty())
		{
			glGenBuffers(1, &render_vertex.index_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_vertex.index_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
		}

		glBindVertexArray(GL_NONE);

	
		render_vertex.vertex_size = mesh_vertices.size();
		render_vertex.indices_count = indices.size();
	}
}

bool CMeshComponent::IsBlend()
{
	return shader_data->bIsBlend;
}

int CMeshComponent::ImportObj(const std::string& model_path)
{
	mesh_resource = ResourceManager::GetOrLoadMesh(model_path);
	
	return 0;
}

