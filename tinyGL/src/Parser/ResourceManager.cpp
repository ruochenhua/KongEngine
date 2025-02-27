#include "ResourceManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Render/RenderCommon.hpp"
#include "stb_image.h"

#include "Render/GraphicsAPI/OpenGL/OpenGLBuffer.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanBuffer.hpp"
#include "Render/Resource/Texture.hpp"
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

weak_ptr<KongTexture> ResourceManager::GetOrLoadTexture_new(const std::string& texture_path, bool filp_uv)
{
	return g_resourceManager->GetTexture_new(texture_path, filp_uv);
}

stbi_uc* ConvertRGB8ToRGBA8(stbi_uc* rgbData, size_t pixelCount) {
	stbi_uc* rgbaData = new stbi_uc[pixelCount*4];
	
	// 检查内存是否分配成功
	assert(rgbaData != nullptr && "Failed to allocate memory for RGBA8 texture");

	// 转换 RGB 到 RGBA
	for (int i = 0; i < pixelCount; ++i) {
		rgbaData[i * 4 + 0] = rgbData[i * 3 + 0]; // R
		rgbaData[i * 4 + 1] = rgbData[i * 3 + 1]; // G
		rgbaData[i * 4 + 2] = rgbData[i * 3 + 2]; // B
		rgbaData[i * 4 + 3] = 255; // A (alpha 通道设为 255)
	}

	// 原始数据还可以在这里释放，如果不再使用
	delete[] rgbData;

	return rgbaData; // 返回新的 RGBA8 数据
}

weak_ptr<KongTexture> ResourceManager::GetTexture_new(const std::string& texture_path, bool flip_uv)
{
	if(texture_cache_new.find(texture_path) != texture_cache_new.end())
	{
		return texture_cache_new[texture_path];
	}

	GLuint texture_id = 0;
	if (texture_path.empty())
	{
		return weak_ptr<KongTexture>{};		
	}
	stbi_set_flip_vertically_on_load(flip_uv);
	int width, height, nr_component;
	auto data = stbi_load(texture_path.c_str(), &width, &height, &nr_component, 0);
	assert(data, "load texture failed");
	
	// todo: opengl和vulkan这里要做区分
#ifdef RENDER_IN_VULKAN
	
	auto new_tex = make_shared<VulkanTexture>();

	// rgb8在GPU上的支持可能不够，转换成rgba
	if (nr_component == 3)
	{
		data = ConvertRGB8ToRGBA8(data, width * height);
		nr_component = 4;
	}
	
	new_tex->CreateTexture(width, height, nr_component, data);
	texture_cache_new.emplace(texture_path, new_tex);
	
#else
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
	
	auto new_tex = make_shared<OpenGLTexture>();
	TextureBuilder::CreateTexture2D(new_tex->m_texId, width, height, format, data);
	texture_cache_new.emplace(texture_path, new_tex);
#endif
	
	// release memory
	stbi_image_free(data);
	
	return texture_cache_new[texture_path];
}

void ResourceManager::Clean()
{
	if (!g_resourceManager)
	{
		return;
	}

	// 释放资源
	g_resourceManager->mesh_cache.clear();			
	g_resourceManager->texture_cache.clear();
	g_resourceManager->texture_cache_new.clear();	
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
	
	auto new_mesh = make_shared<CMesh>();
	new_mesh->name = mesh->mName.C_Str();
	for(unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		// 用emplace_back而不是push_back可以避免构造后的拷贝操作
		const auto& vert = mesh->mVertices[i];
		Vertex new_vertex;
		
		new_vertex.position = {vert.x, vert.y, vert.z};
		
		if(has_normal)
		{
			const auto& norm = mesh->mNormals[i];

			new_vertex.normal = {norm.x, norm.y, norm.z};
		}
		else
		{
			new_vertex.normal = {0, 1, 0};
		}
		
		if(has_uv)
		{
			const auto& tex_uv = mesh->mTextureCoords[0][i];

			new_vertex.uv = {tex_uv.x, tex_uv.y};
		}
		else
		{
			new_vertex.uv = {0, 0};
		}

		if(has_tangent)
		{
			const auto& tangent = mesh->mTangents[i];
			new_vertex.tangent = {tangent.x, tangent.y, tangent.z};

			const auto& bitangnet = mesh->mBitangents[i];
			new_vertex.bitangent = {bitangnet.x, bitangnet.y, bitangnet.z};
		}
		else
		{
			new_vertex.tangent = {1, 0, 0};
			new_vertex.bitangent = {0, 0, 1};
		}

		new_mesh->m_RenderInfo->vertices.emplace_back(new_vertex);
	}
	
	for(unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			new_mesh->m_RenderInfo->m_Index.push_back(face.mIndices[j]);
		}
	}
	
	if(mesh->mMaterialIndex > 0)
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		auto mesh_material = new_mesh->m_RenderInfo->material;
		if(mesh_material)
		{
			mesh_material->name = material->GetName().C_Str();
		
			aiReturn ret = aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &mesh_material->roughness);
			ret = aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &mesh_material->metallic);
				
			aiColor4D ambient_color;
			ret = aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient_color);
			aiGetMaterialFloat(material, AI_MATKEY_SPECULAR_FACTOR, &mesh_material->specular_factor);

			aiColor4D base_color;
			aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &base_color);
			mesh_material->albedo = vec4(base_color.r, base_color.g, base_color.b, base_color.a);
		
			unsigned base_color_count = material->GetTextureCount(aiTextureType_BASE_COLOR);
			unsigned diffuse_count = material->GetTextureCount(aiTextureType_DIFFUSE);
			if(base_color_count > 0)
			{
				aiString tex_str;
				material->GetTexture(aiTextureType_BASE_COLOR, 0, &tex_str);
				string tex_path = directory + "/" + tex_str.C_Str();

				
				// mesh_material->diffuse_tex_id = GetOrLoadTexture(tex_path);
				mesh_material->textures.emplace(ETextureType::diffuse, GetOrLoadTexture_new(tex_path));
			}
			else if(diffuse_count > 0)
			{
				aiString tex_str;
				material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_str);
				string tex_path = directory + "/" + tex_str.C_Str();
				// mesh_material->diffuse_tex_id = GetOrLoadTexture(tex_path);
				mesh_material->textures.emplace(ETextureType::diffuse, GetOrLoadTexture_new(tex_path));
			}
		

			// 暂时还不支持coat效果，先屏蔽掉对应的mesh
			if(mesh_material->name == "coat")
			{
				return;
			}

			unsigned height_count = material->GetTextureCount(aiTextureType_HEIGHT);
			if(height_count > 0)
			{
				aiString tex_str;
				material->GetTexture(aiTextureType_HEIGHT, 0, &tex_str);
				string tex_path = directory + "/" + tex_str.C_Str();
				// mesh_material->normal_tex_id = GetOrLoadTexture(tex_path);
				mesh_material->textures.emplace(ETextureType::normal, GetOrLoadTexture_new(tex_path));
			}
			unsigned normal_count = material->GetTextureCount(aiTextureType_NORMALS);
			if(normal_count > 0)
			{
				aiString tex_str;
				material->GetTexture(aiTextureType_NORMALS, 0, &tex_str);
				string tex_path = directory + "/" + tex_str.C_Str();
				// mesh_material->normal_tex_id = GetOrLoadTexture(tex_path);
				mesh_material->textures.emplace(ETextureType::normal, GetOrLoadTexture_new(tex_path));
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
				// mesh_material->roughness_tex_id = GetOrLoadTexture(tex_path);
				mesh_material->textures.emplace(ETextureType::roughness, GetOrLoadTexture_new(tex_path));
			}

			unsigned metallic_count = material->GetTextureCount(aiTextureType_METALNESS);
			if(metallic_count > 0)
			{
				aiString tex_str;
				material->GetTexture(aiTextureType_METALNESS, 0, &tex_str);
				string tex_path = directory + "/" + tex_str.C_Str();
				// mesh_material->metallic_tex_id = GetOrLoadTexture(tex_path);
				mesh_material->textures.emplace(ETextureType::metallic, GetOrLoadTexture_new(tex_path));
			}

			unsigned ambient_count = material->GetTextureCount(aiTextureType_AMBIENT);
			if(ambient_count > 0)
			{
				aiString tex_str;
				material->GetTexture(aiTextureType_AMBIENT, 0, &tex_str);
				string tex_path = directory + "/" + tex_str.C_Str();
				// mesh_material->ao_tex_id = GetOrLoadTexture(tex_path);
				mesh_material->textures.emplace(ETextureType::ambient_occlusion, GetOrLoadTexture_new(tex_path));
			}
		}
	}
	mesh_resource->mesh_list.push_back(new_mesh);
}
