#include "renderobj.h"
#include "OBJ_Loader.h"
#include "tgaimage.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "render.h"
#include "shader.h"

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

	auto specular_map_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::specular_map);
	if(specular_map_path_iter != texture_paths.end())
	{
		m_RenderInfo.specular_map_tex_id = CRender::LoadTexture(specular_map_path_iter->second);
	}

	m_RenderInfo.color = render_resource_desc.color;
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
	objl::Loader obj_loader;

	bool loadout = obj_loader.LoadFile(model_path);
	if (!loadout)
		return -1;

	auto load_meshes = obj_loader.LoadedMeshes;
	auto load_vertices = obj_loader.LoadedVertices;
	auto load_materials = obj_loader.LoadedMaterials;

	m_vIndex = obj_loader.LoadedIndices;

	size_t vertex_count = load_vertices.size();

	m_vVertex.clear();
	m_vVertex.resize(vertex_count);

	m_vTexCoord.clear();
	m_vTexCoord.resize(vertex_count);

	m_vNormal.clear();
	m_vNormal.resize(vertex_count);

	for (size_t i = 0; i < m_vVertex.size(); ++i)
	{
		auto &pos = load_vertices[i].Position;
		m_vVertex[i] = vec3(pos.X, pos.Y, pos.Z);

		auto tex_coord = load_vertices[i].TextureCoordinate;
		m_vTexCoord[i] = vec2(tex_coord.X, tex_coord.Y);

		auto normal = load_vertices[i].Normal;
		m_vNormal[i] = vec3(normal.X, normal.Y, normal.Z);
	}

	return 0;
}


std::vector<unsigned int> CRenderObj::GetIndices() const
{
	return m_vIndex;
}

void CRenderObj::LoadRenderResource(const SRenderResourceDesc& render_res_desc)
{
	
}

