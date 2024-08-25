#include "MeshComponent.h"
//#include "OBJ_Loader.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "render.h"
#include "Shader/Shader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm/gtc/random.hpp"

using namespace Kong;
using namespace glm;

CMeshComponent::CMeshComponent(const SRenderResourceDesc& render_resource_desc)
{
	if(render_resource_desc.shader_type.empty())
	{
		shader_data = make_shared<Shader>(render_resource_desc);
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
	for(auto& mesh : mesh_list)
	{
		glBindVertexArray(mesh.m_RenderInfo.vertex_array_id);
		shader_data->UpdateRenderData(mesh, scene_render_info);
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
	for(auto& mesh : mesh_list)
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
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(model_path,
		aiProcess_CalcTangentSpace
		|aiProcess_Triangulate
		|aiProcess_OptimizeMeshes
		|aiProcess_GenSmoothNormals
		|aiProcess_OptimizeGraph
		|aiProcess_JoinIdenticalVertices
		//|aiProcess_FlipUVs
		);

	mesh_list.clear();

	directory = model_path.substr(0, model_path.find_last_of('/'));

	ProcessAssimpNode(scene->mRootNode, scene);
	return 0;
}

void CMeshComponent::LoadOverloadTexture(const SRenderResourceDesc& render_resource_desc)
{
	for(auto& mesh : mesh_list)
	{
		auto& render_info = mesh.m_RenderInfo;
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

		auto metallic_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::metallic);
		if (metallic_path_iter != texture_paths.end())
		{
			render_info.metallic_tex_id = CRender::LoadTexture(metallic_path_iter->second);
		}

		auto roughness_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::roughness);
		if (roughness_path_iter != texture_paths.end())
		{
			render_info.roughness_tex_id = CRender::LoadTexture(roughness_path_iter->second);
		}

		auto ao_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::ambient_occlusion);
		if (ao_path_iter != texture_paths.end())
		{
			render_info.ao_tex_id = CRender::LoadTexture(ao_path_iter->second);
		}

		if(render_resource_desc.bOverloadMaterial)
		{
			render_info.material = render_resource_desc.material;
		}
	}
}

void CMeshComponent::ProcessAssimpNode(aiNode* model_node, const aiScene* scene)
{
	// process node mesh
	for(unsigned i = 0; i < model_node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[model_node->mMeshes[i]];
		ProcessAssimpMesh(mesh, scene);
	}

	// process child node
	for(unsigned int i = 0; i < model_node->mNumChildren; ++i)
	{
		ProcessAssimpNode(model_node->mChildren[i], scene);
	}
}

void CMeshComponent::ProcessAssimpMesh(aiMesh* mesh, const aiScene* scene)
{
	// assimp允许一个模型顶点有8组纹理坐标，暂时我们只关心第一组
	bool has_uv = mesh->mTextureCoords[0];
	bool has_normal = mesh->HasNormals();
	bool has_tangent = mesh->HasTangentsAndBitangents();
	
	CMesh new_mesh;
	new_mesh.name = mesh->mName.C_Str();
	for(unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		// 用emplace_back而不是push_back可以避免构造后的拷贝操作
		const auto& vert = mesh->mVertices[i];
		
		new_mesh.m_Vertex.emplace_back(vert.x);
		new_mesh.m_Vertex.emplace_back(vert.y);
		new_mesh.m_Vertex.emplace_back(vert.z);
		
		if(has_normal)
		{
			const auto& norm = mesh->mNormals[i];
			new_mesh.m_Normal.emplace_back(norm.x);
			new_mesh.m_Normal.emplace_back(norm.y);
			new_mesh.m_Normal.emplace_back(norm.z);
		}
		else
		{
			new_mesh.m_Normal.emplace_back(0);
			new_mesh.m_Normal.emplace_back(1);
			new_mesh.m_Normal.emplace_back(0);
		}
		
		if(has_uv)
		{
			const auto& tex_uv = mesh->mTextureCoords[0][i];
			new_mesh.m_TexCoord.emplace_back(tex_uv.x);
			new_mesh.m_TexCoord.emplace_back(tex_uv.y);
		}
		else
		{
			new_mesh.m_TexCoord.emplace_back(0.0);
			new_mesh.m_TexCoord.emplace_back(0.0);
		}

		if(has_tangent)
		{
			const auto& tangent = mesh->mTangents[i];
			new_mesh.m_Tangent.emplace_back(tangent.x);
			new_mesh.m_Tangent.emplace_back(tangent.y);
			new_mesh.m_Tangent.emplace_back(tangent.z);

			const auto& bitangnet = mesh->mBitangents[i];
			new_mesh.m_Bitangent.emplace_back(bitangnet.x);
			new_mesh.m_Bitangent.emplace_back(bitangnet.y);
			new_mesh.m_Bitangent.emplace_back(bitangnet.z);
		}
		else
		{
			// fixme: 这里先塞空值进去，是否可以做判断(用不同的shader？或者shader里面判断tangent的值？)
			new_mesh.m_Tangent.emplace_back(0);
			new_mesh.m_Tangent.emplace_back(0);
			new_mesh.m_Tangent.emplace_back(0);
			new_mesh.m_Bitangent.emplace_back(0);
			new_mesh.m_Bitangent.emplace_back(0);
			new_mesh.m_Bitangent.emplace_back(0);
		}
	}

	new_mesh.m_RenderInfo.vertex_size = mesh->mNumVertices;
	
	for(unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			new_mesh.m_Index.push_back(face.mIndices[j]);
		}
	}
	new_mesh.m_RenderInfo.indices_count = mesh->mNumFaces;

	if(mesh->mMaterialIndex > 0)
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		auto& mesh_material = new_mesh.m_RenderInfo.material;
		mesh_material.name = material->GetName().C_Str();

		unsigned base_color_count = material->GetTextureCount(aiTextureType_BASE_COLOR);
		unsigned diffuse_count = material->GetTextureCount(aiTextureType_DIFFUSE);
		if(base_color_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_BASE_COLOR, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.diffuse_tex_id = CRender::LoadTexture(tex_path);
		}
		else if(diffuse_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.diffuse_tex_id = CRender::LoadTexture(tex_path);
		}
		
		aiColor4D base_color;
		aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &base_color);
		mesh_material.albedo = vec4(base_color.r, base_color.g, base_color.b, base_color.a);
		// 暂时还不支持coat效果，先屏蔽掉对应的mesh
		if(mesh_material.name == "coat")
		{
			return;
			//mesh_material.albedo = vec3(1,0,0);
		}
		unsigned height_count = material->GetTextureCount(aiTextureType_HEIGHT);
		if(height_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_HEIGHT, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.normal_tex_id = CRender::LoadTexture(tex_path);
		}
		unsigned normal_count = material->GetTextureCount(aiTextureType_NORMALS);
		if(normal_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_NORMALS, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.normal_tex_id = CRender::LoadTexture(tex_path);
		}
		
		//aiTextureType_SHININESS
		unsigned specular_count = material->GetTextureCount(aiTextureType_SPECULAR);
		if(specular_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_SPECULAR, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.specular_tex_id = CRender::LoadTexture(tex_path);
		}

		unsigned shininess_count = material->GetTextureCount(aiTextureType_SHININESS);
		if(shininess_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_SHININESS, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.specular_tex_id = CRender::LoadTexture(tex_path);
		}
				
		unsigned clear_coat_count = material->GetTextureCount(aiTextureType_CLEARCOAT);
		if(clear_coat_count > 0)
		{
			// aiString tex_str;
			// material->GetTexture(aiTextureType_CLEARCOAT, 0, &tex_str);
			// string tex_path = directory + "/" + tex_str.C_Str();
			// new_mesh.m_RenderInfo.diffuse_tex_id = CRender::LoadTexture(tex_path);
		}
		
		unsigned roughness_count = material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);
		if(roughness_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.roughness_tex_id = CRender::LoadTexture(tex_path);
		}

		aiReturn ret = aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &mesh_material.roughness);
		
		ret = aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &mesh_material.metallic);
		aiColor4D ambient_color;
		ret = aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient_color);
		aiGetMaterialFloat(material, AI_MATKEY_SPECULAR_FACTOR, &mesh_material.specular_factor);
		
	}
	mesh_list.push_back(new_mesh);
}

