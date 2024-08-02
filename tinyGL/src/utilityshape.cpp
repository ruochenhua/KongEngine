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
	: CRenderObj(render_resource_desc)
{
	CMesh new_mesh;
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

	glUseProgram(shader_id);
	glUniform1i(glGetUniformLocation(shader_id, "diffuse_texture"), 0);
	glUniform1i(glGetUniformLocation(shader_id, "specular_texture"), 1);
	glUniform1i(glGetUniformLocation(shader_id, "normal_texture"), 2);
	glUniform1i(glGetUniformLocation(shader_id, "tangent_texture"), 3);
	glUniform1i(glGetUniformLocation(shader_id, "shadow_map"), 4);
	glUniform1i(glGetUniformLocation(shader_id, "shadow_map_pointlight"), 5);

	
	mesh_list.push_back(mesh);
}

void CUtilityBox::InitInstancingData()
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
		GenerateRenderInfo();
	}

	
	
	glGenBuffers(1, &instancing_info.instance_buffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, instancing_info.instance_buffer);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::mat4), &instancing_model_mat[0], GL_STATIC_DRAW);
	for(auto& mesh : mesh_list)
	{
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
}

const mat4& CUtilityBox::GetInstancingModelMat(unsigned idx) const
{
	if(idx >= instancing_model_mat.size())
	{
		return mat4(1.0);
	}

	return instancing_model_mat[idx];
}
