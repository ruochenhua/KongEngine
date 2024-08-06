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
	glGenBuffers(1, &instancing_info.instance_buffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, instancing_info.instance_buffer);
	glBufferData(GL_ARRAY_BUFFER, (instancing_info.count+1) * sizeof(glm::mat4), &instancing_model_mat[0], GL_STATIC_DRAW);
	
	const CMesh& mesh = mesh_ptr->mesh_list[0];
	auto& render_info = mesh.m_RenderInfo;
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
	// compile shader map
	shader_data = make_shared<Shader>();
	shader_data->Init(render_resource_desc.shader_paths);
}

void CMeshComponent::BeginPlay()
{
	CComponent::BeginPlay();
	// 调用一下shader的初始化
	for(auto& mesh : mesh_list)
	{
		shader_data->SetupData(mesh);	
	}
}

std::vector<float> CMesh::GetVertices() const
{
	// std::vector<float> vertices(m_Vertex.size() * 3);
	//
	// for (size_t i = 0; i < m_Vertex.size(); ++i)
	// {
	// 	vertices[3 * i] = m_Vertex[i].x;
	// 	vertices[3 * i + 1] = m_Vertex[i].y;
	// 	vertices[3 * i + 2] = m_Vertex[i].z;
	// }

	//return vertices;
	return m_Vertex;
}

std::vector<float> CMesh::GetTextureCoords() const
{
	// std::vector<float> tex_coords(m_TexCoord.size() * 2);
	// for (size_t i = 0; i < m_Vertex.size(); ++i)
	// {
	// 	tex_coords[2 * i] = m_TexCoord[i].x;
	// 	tex_coords[2 * i + 1] = 1.0f - m_TexCoord[i].y;	//to opengl, invert y coord
	// }
	//
	// return tex_coords;
	return m_TexCoord;
}

std::vector<float> CMesh::GetNormals() const
{
	// std::vector<float> normals(m_Normal.size() * 3);
	// for (size_t i = 0; i < m_Normal.size(); ++i)
	// {
	// 	normals[3 * i] = m_Normal[i].x;
	// 	normals[3 * i + 1] = m_Normal[i].y;
	// 	normals[3 * i + 2] = m_Normal[i].z;
	// }
	//
	// return normals;
	return m_Normal;
}


vector<unsigned int> CMesh::GetIndices() const
{
	return m_Index;
}

vector<float> CMesh::GetTangents() const
{
	// std::vector<float> tangents(m_Tangent.size() * 3);
	// for (size_t i = 0; i < m_Tangent.size(); ++i)
	// {
	// 	tangents[3 * i] = m_Tangent[i].x;
	// 	tangents[3 * i + 1] = m_Tangent[i].y;
	// 	tangents[3 * i + 2] = m_Tangent[i].z;
	// }
	//
	// return tangents;
	return m_Tangent;
}

vector<float> CMesh::GetBitangents() const
{
	// std::vector<float> bitangents(m_Bitangent.size() * 3);
	// for (size_t i = 0; i < m_Bitangent.size(); ++i)
	// {
	// 	bitangents[3 * i] = m_Bitangent[i].x;
	// 	bitangents[3 * i + 1] = m_Bitangent[i].y;
	// 	bitangents[3 * i + 2] = m_Bitangent[i].z;
	// }
	//
	// return bitangents;
	return m_Bitangent;
}

int CMeshComponent::ImportObj(const std::string& model_path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(model_path,
		aiProcess_CalcTangentSpace
		|aiProcess_Triangulate
		|aiProcess_OptimizeMeshes
		|aiProcess_GenNormals
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
	bool has_tangent = mesh->HasTangentsAndBitangents();
	CMesh new_mesh;
	for(unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		// 用emplace_back而不是push_back可以避免构造后的拷贝操作
		const auto& vert = mesh->mVertices[i];
		new_mesh.m_Vertex.emplace_back(vert.x);
		new_mesh.m_Vertex.emplace_back(vert.y);
		new_mesh.m_Vertex.emplace_back(vert.z);

		const auto& norm = mesh->mNormals[i];
		new_mesh.m_Normal.emplace_back(norm.x);
		new_mesh.m_Normal.emplace_back(norm.y);
		new_mesh.m_Normal.emplace_back(norm.z);

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
		unsigned diffuse_count = material->GetTextureCount(aiTextureType_DIFFUSE);
		if(diffuse_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.diffuse_tex_id = CRender::LoadTexture(tex_path);
		}

		unsigned normal_count = material->GetTextureCount(aiTextureType_HEIGHT);
		if(normal_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_HEIGHT, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.normal_tex_id = CRender::LoadTexture(tex_path);
		}

		unsigned specular_count = material->GetTextureCount(aiTextureType_SPECULAR);
		if(specular_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_SPECULAR, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			new_mesh.m_RenderInfo.specular_tex_id = CRender::LoadTexture(tex_path);
		}
		
	}
	mesh_list.push_back(new_mesh);
}

