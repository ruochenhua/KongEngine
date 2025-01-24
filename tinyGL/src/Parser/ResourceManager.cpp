#include "ResourceManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "RenderCommon.h"
#include "stb_image.h"
#include "texture.h"
using namespace Kong;
using namespace glm;

ResourceManager* g_resourceManager = new ResourceManager;

shared_ptr<MeshResource> ResourceManager::GetOrLoadMesh(const std::string& model_path)
{
    // 找到现有资源
    return g_resourceManager->GetMesh(model_path);
}

GLuint ResourceManager::GetOrLoadTexture(const std::string& texture_path, bool flip_uv)
{
    return g_resourceManager->GetTexture(texture_path, flip_uv);
}

GLuint ResourceManager::GetTexture(const std::string& texture_path, bool flip_uv)
{
    if(texture_cache.find(texture_path) != texture_cache.end())
    {
        return texture_cache[texture_path];
    }

	GLuint texture_id = 0;
	if (texture_path.empty())
	{
		return texture_id;		
	}
	stbi_set_flip_vertically_on_load(flip_uv);
	int width, height, nr_component;
	auto data = stbi_load(texture_path.c_str(), &width, &height, &nr_component, 0);
	assert(data, "load texture failed");

	GLenum format = GL_BGR;
	switch(nr_component)
	{
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		break;
	}
	
	TextureBuilder::CreateTexture2D(texture_id, width, height, format, data);
	
	texture_cache.emplace(texture_path, texture_id);
	// release memory
	stbi_image_free(data);
    return texture_id;
}


shared_ptr<MeshResource> ResourceManager::GetMesh(const std::string& model_path)
{
	if(mesh_cache.find(model_path) != mesh_cache.end())
	{
		return mesh_cache[model_path];
	}
	
	// 加载资源
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(model_path,
		aiProcess_CalcTangentSpace
		|aiProcess_Triangulate
		|aiProcess_OptimizeMeshes
		|aiProcess_GenSmoothNormals
		|aiProcess_OptimizeGraph
		|aiProcess_JoinIdenticalVertices
		);

	auto mesh_resource = make_shared<MeshResource>();
	mesh_resource->directory = model_path.substr(0, model_path.find_last_of('/'));

	ProcessAssimpNode(scene->mRootNode, scene, mesh_resource);
	mesh_cache.emplace(model_path, mesh_resource);
	
	return mesh_resource;
}

void ResourceManager::ProcessAssimpNode(const aiNode* model_node,
                                        const aiScene* scene,
                                        const shared_ptr<MeshResource>& mesh_resource)
{
    // process node mesh
    for(unsigned i = 0; i < model_node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[model_node->mMeshes[i]];
        ProcessAssimpMesh(mesh, scene, mesh_resource);
    }

    // process child node
    for(unsigned int i = 0; i < model_node->mNumChildren; ++i)
    {
        ProcessAssimpNode(model_node->mChildren[i], scene, mesh_resource);
    }
}

void ResourceManager::ProcessAssimpMesh(aiMesh* mesh,
	const aiScene* scene, const shared_ptr<MeshResource>& mesh_resource)
{
    // assimp允许一个模型顶点有8组纹理坐标，暂时我们只关心第一组
	bool has_uv = mesh->mTextureCoords[0];
	bool has_normal = mesh->HasNormals();
	bool has_tangent = mesh->HasTangentsAndBitangents();
	string directory = mesh_resource->directory;
	
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

	new_mesh.m_RenderInfo.vertex.vertex_size = mesh->mNumVertices;
	
	for(unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			new_mesh.m_Index.push_back(face.mIndices[j]);
		}
	}
	new_mesh.m_RenderInfo.vertex.indices_count = mesh->mNumFaces;

	if(mesh->mMaterialIndex > 0)
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		auto& mesh_material = new_mesh.m_RenderInfo.material;
		mesh_material.name = material->GetName().C_Str();
		
		aiReturn ret = aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &mesh_material.roughness);
		ret = aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &mesh_material.metallic);
				
		aiColor4D ambient_color;
		ret = aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient_color);
		aiGetMaterialFloat(material, AI_MATKEY_SPECULAR_FACTOR, &mesh_material.specular_factor);

		aiColor4D base_color;
		aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &base_color);
		mesh_material.albedo = vec4(base_color.r, base_color.g, base_color.b, base_color.a);
		
		unsigned base_color_count = material->GetTextureCount(aiTextureType_BASE_COLOR);
		unsigned diffuse_count = material->GetTextureCount(aiTextureType_DIFFUSE);
		if(base_color_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_BASE_COLOR, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			mesh_material.diffuse_tex_id = GetOrLoadTexture(tex_path);
		}
		else if(diffuse_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			mesh_material.diffuse_tex_id = GetOrLoadTexture(tex_path);
		}
		

		// 暂时还不支持coat效果，先屏蔽掉对应的mesh
		if(mesh_material.name == "coat")
		{
			return;
		}

		unsigned height_count = material->GetTextureCount(aiTextureType_HEIGHT);
		if(height_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_HEIGHT, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			mesh_material.normal_tex_id = GetOrLoadTexture(tex_path);
		}
		unsigned normal_count = material->GetTextureCount(aiTextureType_NORMALS);
		if(normal_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_NORMALS, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			mesh_material.normal_tex_id = GetOrLoadTexture(tex_path);
		}
		
		// unsigned clear_coat_count = material->GetTextureCount(aiTextureType_CLEARCOAT);
		// if(clear_coat_count > 0)
		// {
		// 	// aiString tex_str;
		// 	// material->GetTexture(aiTextureType_CLEARCOAT, 0, &tex_str);
		// 	// string tex_path = directory + "/" + tex_str.C_Str();
		// 	// new_mesh.m_RenderInfo.diffuse_tex_id = CRender::LoadTexture(tex_path);
		// }
		
		unsigned roughness_count = material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);
		if(roughness_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			mesh_material.roughness_tex_id = GetOrLoadTexture(tex_path);
		}

		unsigned metallic_count = material->GetTextureCount(aiTextureType_METALNESS);
		if(metallic_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_METALNESS, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			mesh_material.metallic_tex_id = GetOrLoadTexture(tex_path);
		}

		unsigned ambient_count = material->GetTextureCount(aiTextureType_AMBIENT);
		if(ambient_count > 0)
		{
			aiString tex_str;
			material->GetTexture(aiTextureType_AMBIENT, 0, &tex_str);
			string tex_path = directory + "/" + tex_str.C_Str();
			mesh_material.ao_tex_id = GetOrLoadTexture(tex_path);
		}
	}
	mesh_resource->mesh_list.push_back(new_mesh);
}
