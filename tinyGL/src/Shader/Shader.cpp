#include "Shader.h"

#include "BRDFShader.h"
#include "EmitShader.h"
#include "LightComponent.h"
#include "render.h"
#include "ShadowMapShader.h"

using namespace tinyGL;
using namespace glm;

ShaderManager* g_shader_manager = new ShaderManager;

Shader::Shader(const SRenderResourceDesc& render_resource_desc)
{
    Init(render_resource_desc.shader_paths);
}

GLuint Shader::LoadShaders(const map<EShaderType, string>& shader_path_map)
{
    // include 文件名需要以“/”开头，要不然会报错，为什么？？
    string include_name_str = "/common/common.glsl";
    string full_path = CSceneLoader::ToResourcePath("/shader"+include_name_str);
    string include_content_str = Engine::ReadFile(full_path);
	
    
    vector<GLuint> shader_id_list;
    for(auto& shader_path_pair : shader_path_map)
    {
    	auto shader_type = shader_path_pair.first;
    	auto shader_path = shader_path_pair.second;

    	// EShaderType对照GL_XXXX_SHADER
    	GLuint shader_id = glCreateShader(shader_type);

    	// Read the Shader code from the file
    	std::string shader_code = Engine::ReadFile(shader_path);

    	GLint result = GL_FALSE;
    	int info_log_length;

    	// Compile Shader
    	printf("Compiling shader : %s\n", shader_path.c_str());
    	char const * shader_string_ptr = shader_code.c_str();
    	glShaderSource(shader_id, 1, &shader_string_ptr, NULL);
    	glNamedStringARB(GL_SHADER_INCLUDE_ARB,
    		include_name_str.size(),
    		include_name_str.c_str(),
    		include_content_str.size(),
    		include_content_str.c_str());
    	
    	glCompileShader(shader_id);

    	// Check Shader
    	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    	if (info_log_length > 0) {
    		std::vector<char> error_msg(info_log_length + 1);
    		glGetShaderInfoLog(shader_id, info_log_length, NULL, &error_msg[0]);
    		printf("%s\n", &error_msg[0]);
    		assert(0);
    	}
		shader_id_list.push_back(shader_id);
    }

	// Link the program
	printf("Linking program\n");
	GLuint prog_id = glCreateProgram();
    for(auto shader_id : shader_id_list)
    {
    	glAttachShader(prog_id, shader_id);
    }
	glLinkProgram(prog_id);

    GLint result = GL_FALSE;
    int info_log_length;
	// Check the program
	glGetProgramiv(prog_id, GL_LINK_STATUS, &result);
	glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> prog_error_msg(info_log_length + 1);
		glGetProgramInfoLog(prog_id, info_log_length, NULL, &prog_error_msg[0]);
		printf("%s\n", &prog_error_msg[0]);
		assert(0);
	}

    for(auto shader_id : shader_id_list)
    {
    	glDetachShader(prog_id, shader_id);
		glDeleteShader(shader_id);
    }

	return prog_id;
}



void Shader::Init(const map<EShaderType, string>& shader_path_cache)
{
    shader_id = Shader::LoadShaders(shader_path_cache);
}

void Shader::Use() const
{
    assert(shader_id, "Shader not loaded yet!");
    glUseProgram(shader_id);
}

void Shader::SetupData(CMesh& mesh)
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

	Use();
	SetInt("diffuse_texture", 0);
	SetInt("specular_texture", 1);
	SetInt("normal_texture", 2);
	SetInt("shadow_map", 3);
	SetInt("shadow_map_pointlight", 4);
}

void Shader::UpdateRenderData(const CMesh& mesh, const glm::mat4& actor_model_mat,
                              const SSceneRenderInfo& scene_render_info)
{
	auto& render_info = mesh.m_RenderInfo;
    glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO

	// 材质属性
	SetVec3("albedo", render_info.material.albedo);
	SetFloat("metallic", render_info.material.metallic);
	SetFloat("roughness", render_info.material.roughness);
	SetFloat("ao", render_info.material.ao);

	/*
	法线矩阵被定义为「模型矩阵左上角3x3部分的逆矩阵的转置矩阵」
	Normal = mat3(transpose(inverse(model))) * aNormal;
	 */
	mat3 normal_model_mat = transpose(inverse(actor_model_mat));
	SetMat3("normal_model_mat", normal_model_mat);

	GLuint null_tex_id = CRender::GetNullTexId();
	glActiveTexture(GL_TEXTURE0);
	GLuint diffuse_tex_id = render_info.diffuse_tex_id != 0 ? render_info.diffuse_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);

	glActiveTexture(GL_TEXTURE1);
	GLuint specular_map_id = render_info.specular_tex_id != 0 ? render_info.specular_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, specular_map_id);

	glActiveTexture(GL_TEXTURE2);
	GLuint normal_map_id = render_info.normal_tex_id != 0 ? render_info.normal_tex_id : null_tex_id;
	glBindTexture(GL_TEXTURE_2D, normal_map_id);
}

shared_ptr<Shader> ShaderManager::GetShader(const string& shader_name)
{
	return g_shader_manager->GetShaderFromTypeName(shader_name);
}

shared_ptr<Shader> ShaderManager::GetShaderFromTypeName(const string& shader_name)
{
	auto find_iter = shader_cache.find(shader_name);
	if(find_iter != shader_cache.end())
	{
		return find_iter->second;
	}

	// create shader
	if(shader_name == "brdf")
	{
		auto shader_data = make_shared<BRDFShader>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "brdf_normalmap")
	{
		auto shader_data = make_shared<BRDFShader_NormalMap>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "emit")
	{
		auto shader_data = make_shared<EmitShader>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "point_light_shadowmap")
	{
		auto shader_data = make_shared<PointLightShadowMapShader>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else if(shader_name == "directional_light_shadowmap")
	{
		auto shader_data = make_shared<DirectionalLightShadowMapShader>();
		shader_data->InitDefaultShader();
		shader_cache.emplace(shader_name, shader_data);
		return shader_data;
	}
	else
	{
		assert(0, "shader type not supported");
	}

	
	return nullptr;
}
