#include "skybox.h"
#include "render.h"
#include "Scene.h"
#include "stb_image.h"
#include "Shader/Shader.h"

using namespace tinyGL;

#define USE_CUBE_MAP 0
void CSkyBox::Init()
{
	map<EShaderType, string> skybox_shader = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox.frag")}
	};
	
	shader_id = Shader::LoadShaders(skybox_shader);
	assert(shader_id, "load skybox shader failed");
	box_mesh = make_shared<CBoxShape>(SRenderResourceDesc());
	// 这里begin play一下会创建一下对应的顶点buffer等数据
	box_mesh->BeginPlay();
	
#if USE_CUBE_MAP
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


	//create texture
	glGenTextures(1, &cube_map_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
	for (int i = 0; i < 6; ++i)
	{
		int width, height, nr_component;
		auto data = stbi_load(tex_path_vec[i].c_str(), &width, &height, &nr_component, 0);
		assert(data);

		GLenum format = GL_RGB;
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
		
		glTexImage2D(tex_type_vec[i], 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#else
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	
	map<EShaderType, string> sphere_to_cube_shader = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/sphere_to_cube.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/sphere_to_cube.frag")}
	};
	
	sphere_to_cube_shader_id = Shader::LoadShaders(sphere_to_cube_shader);
	assert(shader_id, "load skybox shader failed");
	
	// 贴图球形映射转换为立方体贴图映射
	glGenFramebuffers(1, &sphere_to_cube_fbo);
	glGenRenderbuffers(1, &sphere_to_cube_rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, sphere_to_cube_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, sphere_to_cube_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sphere_to_cube_rbo);
	stbi_set_flip_vertically_on_load(true);
	string sphere_map_path = CSceneLoader::ToResourcePath("sky_box/newport_loft.hdr");
	int width, height, nr_component;
	
	auto data = stbi_loadf(sphere_map_path.c_str(), &width, &height, &nr_component, 0);
	assert(data);

	// 创建球形映射的贴图
	glGenTextures(1, &sphere_map_texture);
	glBindTexture(GL_TEXTURE_2D, sphere_map_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	stbi_image_free(data);
		
	// 创建立方体贴图
	glGenTextures(1, &cube_map_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
	for (int i = 0; i < 6; ++i)
	{
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);




	auto& render_info = box_mesh->mesh_list[0].m_RenderInfo;
	
	vec3 scene_center = vec3(0);
	vector<mat4> skybox_views;
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(1,0,0), vec3(0,-1,0)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(-1,0,0), vec3(0,-1,0)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(0,1,0), vec3(0,0,1)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(0,-1,0), vec3(0,0,-1)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(0,0,1), vec3(0,-1,0)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(0,0,-1), vec3(0,-1,0)));
	
	mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	
	glUseProgram(sphere_to_cube_shader_id);
	glUniform1i(glGetUniformLocation(sphere_to_cube_shader_id, "sphere_map"), 0);
	glUniformMatrix4fv(glGetUniformLocation(sphere_to_cube_shader_id, "projection"), 1, GL_FALSE, &projection[0][0]);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphere_map_texture);

	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, sphere_to_cube_fbo);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(sphere_to_cube_shader_id, "view"), 1, GL_FALSE, &skybox_views[i][0][0]);
	
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cube_map_id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
		//renderCube(); // renders a 1x1 cube
		glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  

#endif
}

void CSkyBox::Render(const glm::mat4& mvp)
{
	auto& render_info = box_mesh->mesh_list[0].m_RenderInfo;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
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
