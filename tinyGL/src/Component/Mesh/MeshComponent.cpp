#include "MeshComponent.h"
//#include "OBJ_Loader.h"

#include "render.h"
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

void CMeshComponent::Draw(const SSceneRenderInfo& scene_render_info)
{
	shader_data->Use();
	for(auto& mesh : mesh_resource->mesh_list)
	{
		auto& render_vertex = mesh.m_RenderInfo.vertex;
		
		glBindVertexArray(render_vertex.vertex_array_id);
		if(use_override_material)
		{
			shader_data->UpdateRenderData(override_render_info.material, scene_render_info);
		}
		else
		{
			shader_data->UpdateRenderData(mesh.m_RenderInfo.material, scene_render_info);
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
		std::vector<float> vertices = mesh.GetVertices();
		
		glGenVertexArrays(1, &render_vertex.vertex_array_id);
		glBindVertexArray(render_vertex.vertex_array_id);

		//init vertex buffer
		glGenBuffers(1, &render_vertex.vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_vertex.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

		//vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER,  render_vertex.vertex_buffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		glEnableVertexAttribArray(0);

		//normal buffer
		std::vector<float> normals = mesh.GetNormals();
		glGenBuffers(1, &render_vertex.normal_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_vertex.normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*normals.size(), &normals[0], GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3,GL_FLOAT,GL_FALSE,0, (void*)0);
		glEnableVertexAttribArray(1);

		// texcoord
		std::vector<float> tex_coords = mesh.GetTextureCoords();
		glGenBuffers(1, &render_vertex.texture_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_vertex.texture_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,(void*)0);
		glEnableVertexAttribArray(2);

		// tangent
		vector<float> tangents = mesh.GetTangents();
		if(!tangents.empty())
		{
			glGenBuffers(1, &render_vertex.tangent_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, render_vertex.tangent_buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tangents.size(), &tangents[0], GL_STATIC_DRAW);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glEnableVertexAttribArray(3);
		}
		
		// bitangent
		vector<float> bitangents = mesh.GetBitangents();
		if(!bitangents.empty())
		{
			glGenBuffers(1, &render_vertex.bitangent_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, render_vertex.bitangent_buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*bitangents.size(), &bitangents[0], GL_STATIC_DRAW);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glEnableVertexAttribArray(4);
		}
		
		// index buffer
		std::vector<unsigned int> indices = mesh.GetIndices();
		if(!indices.empty())
		{
			glGenBuffers(1, &render_vertex.index_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_vertex.index_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
		}

		glBindVertexArray(GL_NONE);

		// m_RenderInfo._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
		render_vertex.vertex_size = vertices.size();
		render_vertex.indices_count = indices.size();
	}
}

bool CMeshComponent::IsBlend()
{
	return shader_data->bIsBlend;
}

std::vector<float> CMesh::GetVertices() const
{
	return m_Vertex;
}

std::vector<float> CMesh::GetTextureCoords() const
{
	return m_TexCoord;
}

std::vector<float> CMesh::GetNormals() const
{
	return m_Normal;
}

vector<unsigned int> CMesh::GetIndices() const
{
	return m_Index;
}

vector<float> CMesh::GetTangents() const
{
	return m_Tangent;
}

vector<float> CMesh::GetBitangents() const
{
	return m_Bitangent;
}

int CMeshComponent::ImportObj(const std::string& model_path)
{
	mesh_resource = ResourceManager::GetOrLoadMesh(model_path);
	
	return 0;
}

