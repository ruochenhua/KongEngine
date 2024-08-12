#include "MeshComponent.h"
//#include "OBJ_Loader.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "render.h"
#include "Shader/Shader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm/gtc/random.hpp"

using namespace tinyGL;
using namespace glm;

mat4 CTransformComponent::GetModelMatrix() const
{
	mat4 model = mat4(1.0);
	model = translate(model, location);
	model *= eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
	model = glm::scale(model, scale);
	return model;
}

mat4 CTransformComponent::GenInstanceModelMatrix() const
{
	mat4 model = mat4(1.0);

	vec3 ins_location, ins_rotation, ins_scale;
	ins_location = glm::linearRand(instancing_info.location_min, instancing_info.location_max);
	ins_rotation = glm::linearRand(instancing_info.rotation_min, instancing_info.rotation_max);
	ins_scale	 = glm::linearRand(instancing_info.scale_min, instancing_info.scale_max);

	model = translate(model, ins_location);
	model *= eulerAngleXYZ(ins_rotation.x, ins_rotation.y, ins_rotation.z);
	model = glm::scale(model, ins_scale);
	
	return model;
}

void CTransformComponent::InitInstancingData()
{
	unsigned count = instancing_info.count;
	if(count == 0)
	{
		return;
	}
		
	instancing_model_mat.resize(count+1);
	
	instancing_model_mat[0] = GetModelMatrix();
	for(unsigned i = 1; i <= count; ++i)
	{
		instancing_model_mat[i] = GenInstanceModelMatrix();
	}
}

void CTransformComponent::BindInstancingToMesh(weak_ptr<CMeshComponent> mesh_comp)
{
	auto mesh_ptr = mesh_comp.lock();

	CMesh& mesh = mesh_ptr->mesh_list[0];
	auto& render_info = mesh.m_RenderInfo;
	glGenBuffers(1, &render_info.instance_buffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, render_info.instance_buffer);
	render_info.instance_count = instancing_info.count+1;
	glBufferData(GL_ARRAY_BUFFER, render_info.instance_count * sizeof(glm::mat4), &instancing_model_mat[0], GL_STATIC_DRAW);
	
	glBindVertexArray(render_info.vertex_array_id);
	GLsizei vec4_size = sizeof(glm::vec4);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(vec4_size));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(vec4_size*2));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(vec4_size*3));
	
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	
	glBindVertexArray(0);
}

const mat4& CTransformComponent::GetInstancingModelMat(unsigned idx) const
{
	if(idx >= instancing_model_mat.size())
	{
		return mat4(1.0);
	}

	return instancing_model_mat[idx];
}

CMeshComponent::CMeshComponent(const SRenderResourceDesc& render_resource_desc)
{
	InitRenderInfo(render_resource_desc);
}

void CMeshComponent::BeginPlay()
{
	CComponent::BeginPlay();
}

void CMeshComponent::InitRenderInfo(const SRenderResourceDesc& render_resource_desc)
{
	// compile shader map
	if(render_resource_desc.shader_type.empty())
	{
		shader_data = make_shared<Shader>(render_resource_desc);
	}
	else
	{
		shader_data = ShaderManager::GetShader(render_resource_desc.shader_type);
	}
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
		|aiProcess_FlipUVs
		);

	mesh_list.clear();

	directory = model_path.substr(0, model_path.find_last_of('/'));

	ProcessAssimpNode(scene->mRootNode, scene);
	return 0;
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

	for(unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			new_mesh.m_Index.push_back(face.mIndices[j]);
		}
	}

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
		mesh_material.albedo = vec3(base_color.r, base_color.g, base_color.b);
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
			new_mesh.m_RenderInfo.diffuse_roughness_tex_id = CRender::LoadTexture(tex_path);
		}

		aiReturn ret = aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &mesh_material.roughness);
		
		ret = aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &mesh_material.metallic);
		aiColor4D ambient_color;
		ret = aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient_color);
		aiGetMaterialFloat(material, AI_MATKEY_SPECULAR_FACTOR, &mesh_material.specular_factor);
		
	}
	mesh_list.push_back(new_mesh);
}

