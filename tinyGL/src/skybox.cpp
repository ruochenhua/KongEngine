#include "skybox.h"
#include "render.h"
#include "Scene.h"
#include "stb_image.h"
#include "Shader/Shader.h"

using namespace tinyGL;

void CSkyBox::Init()
{
	std::vector<std::string> tex_path_vec = {
		CSceneLoader::ToResourcePath("sky_box/dark_sky/darkskies_bk.tga"),
		CSceneLoader::ToResourcePath("sky_box/dark_sky/darkskies_dn.tga"),
		CSceneLoader::ToResourcePath("sky_box/dark_sky/darkskies_ft.tga"),
		CSceneLoader::ToResourcePath("sky_box/dark_sky/darkskies_lf.tga"),
		CSceneLoader::ToResourcePath("sky_box/dark_sky/darkskies_rt.tga"),
		CSceneLoader::ToResourcePath("sky_box/dark_sky/darkskies_up.tga"),
	};
	std::vector<unsigned int> tex_type_vec = {
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	};

	map<EShaderType, string> skybox_shader = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox_vert.shader")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox_frag.shader")}
	};
	
	shader_id = Shader::LoadShaders(skybox_shader);
	
	box_mesh = make_shared<CBoxShape>(SRenderResourceDesc());
	// 这里begin play一下会创建一下对应的顶点buffer等数据
	box_mesh->BeginPlay();

	//create texture
	glGenTextures(1, &cube_map_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
	for (int i = 0; i < 6; ++i)
	{
		int width, height, nr_component;
		auto data = stbi_load(tex_path_vec[i].c_str(), &width, &height, &nr_component, 0);
		assert(data);

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
		
		glTexImage2D(tex_type_vec[i], 0, format, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		stbi_image_free(data);
	}
}

void CSkyBox::Render(const glm::mat4& mvp)
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	
	auto& render_info = box_mesh->mesh_list[0].m_RenderInfo;
	glUseProgram(shader_id);
	glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
	//Shader::SetMat4(m_BoxMesh.shader_id, "MVP", mvp);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "MVP"), 1, GL_FALSE, &mvp[0][0]);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
	
	glUniform1i(glGetUniformLocation(shader_id, "skybox"), 0);
	
	//glDrawArrays(GL_TRIANGLES, 0, mesh_info.vertex_size / 3);
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
		glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(GL_NONE);
}
