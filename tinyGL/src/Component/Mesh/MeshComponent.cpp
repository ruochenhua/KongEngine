#include "MeshComponent.h"
//#include "OBJ_Loader.h"

#include "render.h"
#include "Shader/Shader.h"

#include "glm/gtc/random.hpp"
#include "Parser/ResourceManager.h"

using namespace Kong;
using namespace glm;

CMeshComponent::CMeshComponent(const SRenderResourceDesc& render_resource_desc)
{
	if(render_resource_desc.shader_type.empty())
	{
		shader_data = make_shared<Shader>(render_resource_desc.shader_paths);
	}
	else
	{
		shader_data = ShaderManager::GetShader(render_resource_desc.shader_type);
	}
}

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
		glBindVertexArray(mesh.m_RenderInfo.vertex_array_id);
		if(use_override_material)
		{
			shader_data->UpdateRenderData(override_render_info, scene_render_info);
		}
		else
		{
			shader_data->UpdateRenderData(mesh.m_RenderInfo, scene_render_info);
		}
		// Draw the triangle !
		// if no index, use draw array
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

void CMeshComponent::InitRenderInfo()
{
	// compile shader map
	for(auto& mesh : mesh_resource->mesh_list)
	{
		// 构建默认的shader数据结构，数据齐全，但是冗余
		auto& render_info = mesh.m_RenderInfo;
		std::vector<float> vertices = mesh.GetVertices();

		glGenVertexArrays(1, &render_info.vertex_array_id);
		glBindVertexArray(render_info.vertex_array_id);

		//init vertex buffer
		glGenBuffers(1, &render_info.vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_info.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

		//vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER,  render_info.vertex_buffer);
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
		glGenBuffers(1, &render_info.normal_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_info.normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*normals.size(), &normals[0], GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3,GL_FLOAT,GL_FALSE,0, (void*)0);
		glEnableVertexAttribArray(1);

		// texcoord
		std::vector<float> tex_coords = mesh.GetTextureCoords();
		glGenBuffers(1, &render_info.texture_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, render_info.texture_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,(void*)0);
		glEnableVertexAttribArray(2);
		

		// tangent
		vector<float> tangents = mesh.GetTangents();
		if(!tangents.empty())
		{
			glGenBuffers(1, &render_info.tangent_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, render_info.tangent_buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tangents.size(), &tangents[0], GL_STATIC_DRAW);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glEnableVertexAttribArray(3);
		}
		
		// bitangent
		vector<float> bitangents = mesh.GetBitangents();
		if(!bitangents.empty())
		{
			glGenBuffers(1, &render_info.bitangent_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, render_info.bitangent_buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*bitangents.size(), &bitangents[0], GL_STATIC_DRAW);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glEnableVertexAttribArray(4);
		}
		
		// index buffer
		std::vector<unsigned int> indices = mesh.GetIndices();
		if(!indices.empty())
		{
			glGenBuffers(1, &render_info.index_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_info.index_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
		}

		glBindVertexArray(GL_NONE);

		// m_RenderInfo._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
		render_info.vertex_size = vertices.size();
		render_info.indices_count = indices.size();
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

void CMeshComponent::LoadOverloadTexture(const SRenderResourceDesc& render_resource_desc)
{
	// todo: vbo、vao这些应该拆出来
	override_render_info = mesh_resource->mesh_list[0].m_RenderInfo;
	if(render_resource_desc.bOverloadMaterial)
	{
		use_override_material = true;
		override_render_info.material = render_resource_desc.material;
	}
	
	const auto& texture_paths = render_resource_desc.texture_paths;
	auto diffuse_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::diffuse);
	if (diffuse_path_iter != texture_paths.end())
	{
		override_render_info.diffuse_tex_id = ResourceManager::GetOrLoadTexture(diffuse_path_iter->second);
	}

	auto specular_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::specular);
	if (specular_path_iter != texture_paths.end())
	{
		override_render_info.specular_tex_id = ResourceManager::GetOrLoadTexture(specular_path_iter->second);
	}

	auto normal_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::normal);
	if (normal_path_iter != texture_paths.end())
	{
		override_render_info.normal_tex_id = ResourceManager::GetOrLoadTexture(normal_path_iter->second);
	}

	auto metallic_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::metallic);
	if (metallic_path_iter != texture_paths.end())
	{
		override_render_info.metallic_tex_id = ResourceManager::GetOrLoadTexture(metallic_path_iter->second);
	}

	auto roughness_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::roughness);
	if (roughness_path_iter != texture_paths.end())
	{
		override_render_info.roughness_tex_id = ResourceManager::GetOrLoadTexture(roughness_path_iter->second);
	}

	auto ao_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::ambient_occlusion);
	if (ao_path_iter != texture_paths.end())
	{
		override_render_info.ao_tex_id = ResourceManager::GetOrLoadTexture(ao_path_iter->second);
	}
}
