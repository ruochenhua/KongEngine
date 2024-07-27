#include "renderobj.h"
//#include "OBJ_Loader.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "render.h"
#include "shader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace tinyGL;
using namespace glm;

mat4 SceneObject::GetModelMatrix() const
{
	mat4 model = mat4(1.0);
	model = translate(model, location);
	model *= eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
	model = glm::scale(model, scale);
	return model;
}

CRenderObj::CRenderObj(const SRenderResourceDesc& render_resource_desc)
{
	// compile shader map
	m_RenderInfo.program_id = Shader::LoadShaders(render_resource_desc.shader_paths);

	// load texture map
	const auto& texture_paths = render_resource_desc.texture_paths;
	auto diffuse_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::diffuse);
	if(diffuse_path_iter != texture_paths.end())
	{
		m_RenderInfo.diffuse_tex_id = CRender::LoadTexture(diffuse_path_iter->second);
	}

	auto specular_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::specular);
	if(specular_path_iter != texture_paths.end())
	{
		m_RenderInfo.specular_tex_id = CRender::LoadTexture(specular_path_iter->second);
	}

	auto normal_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::normal);
	if(normal_path_iter != texture_paths.end())
	{
		m_RenderInfo.normal_tex_id = CRender::LoadTexture(normal_path_iter->second);
	}
	
	m_RenderInfo.material = render_resource_desc.material;
}

std::vector<float> CRenderObj::GetVertices() const
{
	std::vector<float> vertices(m_vVertex.size() * 3);

	for (size_t i = 0; i < m_vVertex.size(); ++i)
	{
		vertices[3 * i] = m_vVertex[i].x;
		vertices[3 * i + 1] = m_vVertex[i].y;
		vertices[3 * i + 2] = m_vVertex[i].z;
	}

	return vertices;
}

std::vector<float> CRenderObj::GetTextureCoords() const
{
	std::vector<float> tex_coords(m_vTexCoord.size() * 2);
	for (size_t i = 0; i < m_vVertex.size(); ++i)
	{
		tex_coords[2 * i] = m_vTexCoord[i].x;
		tex_coords[2 * i + 1] = 1.0f - m_vTexCoord[i].y;	//to opengl, invert y coord
	}

	return tex_coords;
}

std::vector<float> CRenderObj::GetNormals() const
{
	std::vector<float> normals(m_vNormal.size() * 3);
	for (size_t i = 0; i < m_vNormal.size(); ++i)
	{
		normals[3 * i] = m_vNormal[i].x;
		normals[3 * i + 1] = m_vNormal[i].y;
		normals[3 * i + 2] = m_vNormal[i].z;
	}

	return normals;
}

int CRenderObj::ImportObj(const std::string& model_path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(model_path,
		aiProcess_CalcTangentSpace|
		aiProcess_Triangulate|
		aiProcess_JoinIdenticalVertices|
		aiProcess_SortByPType);

	m_vVertex.clear();
	m_vTexCoord.clear();
	m_vNormal.clear();
	ProcessAssimpNode(scene->mRootNode, scene);
	//
	// objl::Loader obj_loader;
	//
	// bool loadout = obj_loader.LoadFile(model_path);
	// if (!loadout)
	// 	return -1;
	//
	// auto load_meshes = obj_loader.LoadedMeshes;
	// auto load_vertices = obj_loader.LoadedVertices;
	// auto load_materials = obj_loader.LoadedMaterials;
	//
	// m_vIndex = obj_loader.LoadedIndices;
	//
	// size_t vertex_count = load_vertices.size();
	//
	// m_vVertex.clear();
	// m_vVertex.resize(vertex_count);
	//
	// m_vTexCoord.clear();
	// m_vTexCoord.resize(vertex_count);
	//
	// m_vNormal.clear();
	// m_vNormal.resize(vertex_count);
	//
	// for (size_t i = 0; i < m_vVertex.size(); ++i)
	// {
	// 	auto &pos = load_vertices[i].Position;
	// 	m_vVertex[i] = vec3(pos.X, pos.Y, pos.Z);
	//
	// 	auto tex_coord = load_vertices[i].TextureCoordinate;
	// 	m_vTexCoord[i] = vec2(tex_coord.X, tex_coord.Y);
	//
	// 	auto normal = load_vertices[i].Normal;
	// 	m_vNormal[i] = vec3(normal.X, normal.Y, normal.Z);
	// }
	//
	return 0;
}

void CRenderObj::ProcessAssimpNode(aiNode* model_node, const aiScene* scene)
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

void CRenderObj::ProcessAssimpMesh(aiMesh* mesh, const aiScene* scene)
{
	// assimp允许一个模型顶点有8组纹理坐标，暂时我们只关心第一组
	bool has_uv = mesh->mTextureCoords[0];
	for(unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		const auto& vert = mesh->mVertices[i];
		glm::vec3 vertex = vec3(vert.x, vert.y, vert.z);
		m_vVertex.push_back(vertex);

		const auto& norm = mesh->mNormals[i];
		glm::vec3 normal = vec3(norm.x, norm.y, norm.z);
		m_vNormal.push_back(normal);

		if(has_uv)
		{
			const auto& tex_uv = mesh->mTextureCoords[0][i];
			glm::vec2 uv = vec2(tex_uv.x, tex_uv.y);
			m_vTexCoord.push_back(uv);
		}
		else
		{
			m_vTexCoord.push_back(vec2(0.f));
		}
	}

	for(unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			m_vIndex.push_back(face.mIndices[j]);
		}
	}

	
	
}


std::vector<unsigned int> CRenderObj::GetIndices() const
{
	return m_vIndex;
}

void CRenderObj::LoadRenderResource(const SRenderResourceDesc& render_res_desc)
{
	
}

