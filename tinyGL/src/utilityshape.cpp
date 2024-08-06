#include "utilityshape.h"

#include "render.h"

using namespace tinyGL;
std::vector<float> CUtilityBox::s_vBoxVertices = {	
	// positions          // normals           // texture coords
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};

CUtilityBox::CUtilityBox(const SRenderResourceDesc& render_resource_desc)
	: CMeshComponent(render_resource_desc)
{
	box_render_resource_desc = render_resource_desc;
	GenerateRenderInfo();
}


std::vector<float> CUtilityBox::GetVertices() const
{
	return s_vBoxVertices;
}

std::vector<unsigned int> CUtilityBox::GetIndices() const
{
	return std::vector<unsigned int>();
}

void CUtilityBox::GenerateRenderInfo()
{
	std::vector<float> vertices = GetVertices();
	CMesh mesh;
	auto& render_info = mesh.m_RenderInfo;
	// load texture map
	const auto& texture_paths = box_render_resource_desc.texture_paths;
	auto diffuse_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::diffuse);
	if(diffuse_path_iter != texture_paths.end())
	{
		render_info.diffuse_tex_id = CRender::LoadTexture(diffuse_path_iter->second);
	}
	
	auto specular_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::specular);
	if(specular_path_iter != texture_paths.end())
	{
		render_info.specular_tex_id = CRender::LoadTexture(specular_path_iter->second);
	}
	
	auto normal_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::normal);
	if(normal_path_iter != texture_paths.end())
	{
		render_info.normal_tex_id = CRender::LoadTexture(normal_path_iter->second);
	}
	
	auto tangent_path_iter = texture_paths.find(SRenderResourceDesc::ETextureType::tangent);
	if(tangent_path_iter != texture_paths.end())
	{
		render_info.tangent_tex_id = CRender::LoadTexture(tangent_path_iter->second);
	}
	
	render_info.material = box_render_resource_desc.material;
	
	glGenVertexArrays(1, &render_info.vertex_array_id);
	glBindVertexArray(render_info.vertex_array_id);

	//init vertex, normal, texcoord buffer
	glGenBuffers(1, &render_info.vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, render_info.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

	//vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER,  render_info.vertex_buffer);
	glVertexAttribPointer(
		0,                  // attribute 0.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		8*sizeof(float),	// stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(
		1,                  // attribute 1. 
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized
		8*sizeof(float),                  // stride
		(void*)(3*sizeof(float))            // array buffer offset
	);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
	2,                  // attribute 2. 
	2,                  // size
	GL_FLOAT,           // type
	GL_FALSE,           // normalized
	8*sizeof(float),                  // stride
	(void*)(6*sizeof(float))            // array buffer offset
	);
	glEnableVertexAttribArray(2);

	// add tangent and bitangent data(null for now)
	
	glBindVertexArray(GL_NONE);
	
	render_info.vertex_size = vertices.size();
	render_info.stride_count = 8;

	shader_data->Use();
	shader_data->SetInt("diffuse_texture", 0);
	shader_data->SetInt("specular_texture", 1);
	shader_data->SetInt("normal_texture", 2);
	shader_data->SetInt("tangent_texture", 3);
	shader_data->SetInt("shadow_map", 4);
	shader_data->SetInt("shadow_map_pointlight", 5);
	
	mesh_list.push_back(mesh);
}
