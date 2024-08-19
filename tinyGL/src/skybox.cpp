#include "skybox.h"
#include "render.h"
#include "Scene.h"
#include "stb_image.h"
#include "Component/Mesh/QuadShape.h"
#include "Shader/Shader.h"

using namespace tinyGL;

unsigned CUBE_MAP_RES = 1024;

#define USE_HDR_SKYBOX 1 
void CSkyBox::Init()
{
	map<EShaderType, string> skybox_shader = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/skybox.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/skybox.frag")}
	};
	
	shader_id = Shader::LoadShaders(skybox_shader);
	assert(shader_id, "load skybox shader failed");
	box_mesh = make_shared<CBoxShape>(SRenderResourceDesc());
	// 这里begin play一下会创建一下对应的顶点buffer等数据
	box_mesh->BeginPlay();
	
	quad_shape = make_shared<CQuadShape>(SRenderResourceDesc());
	quad_shape->BeginPlay();
#if !USE_HDR_SKYBOX
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
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/preprocess_common.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/sphere_to_cube.frag")}
	};
	
	sphere_to_cube_shader_id = Shader::LoadShaders(sphere_to_cube_shader);
	assert(sphere_to_cube_shader_id, "load skybox shader failed");
	
	// 贴图球形映射转换为立方体贴图映射
	glGenFramebuffers(1, &preprocess_fbo);
	glGenRenderbuffers(1, &preprocess_rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, preprocess_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, preprocess_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CUBE_MAP_RES, CUBE_MAP_RES);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, preprocess_rbo);
	stbi_set_flip_vertically_on_load(true);
	string sphere_map_path = CSceneLoader::ToResourcePath("sky_box/newport_loft.hdr");
	int width, height, nr_component;
	
	auto data = stbi_loadf(sphere_map_path.c_str(), &width, &height, &nr_component, 0);
	assert(data);

	// 创建球形映射的贴图
	glGenTextures(1, &sphere_map_texture);
	glBindTexture(GL_TEXTURE_2D, sphere_map_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);
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
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, CUBE_MAP_RES, CUBE_MAP_RES, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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

	glViewport(0, 0, CUBE_MAP_RES, CUBE_MAP_RES); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, preprocess_fbo);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(sphere_to_cube_shader_id, "view"), 1, GL_FALSE, &skybox_views[i][0][0]);
	
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cube_map_id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
		glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);

	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 创建mipmap，在预滤波环境贴图的时候使用他的mipmap贴图可以有效减少采样结果异常斑点的问题
	// 转换完成再生成mipmap
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	

	// 辐照度预计算shader
	map<EShaderType, string> irradiance_shader = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/preprocess_common.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/irradiance_map.frag")}
	};
	
	irradiance_shader_id = Shader::LoadShaders(irradiance_shader);
	assert(irradiance_shader_id, "load skybox shader failed");
	
	// 创建辐照度立方体贴图
	glGenTextures(1, &irradiance_tex_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_tex_id);
	for(int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// 预计算辐照度贴图
	glUseProgram(irradiance_shader_id);
	glUniform1i(glGetUniformLocation(irradiance_shader_id, "cube_map"), 0);
	glUniformMatrix4fv(glGetUniformLocation(irradiance_shader_id, "projection"), 1, GL_FALSE, &projection[0][0]);
		
	glBindFramebuffer(GL_FRAMEBUFFER, preprocess_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, preprocess_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
	
	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.

	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(irradiance_shader_id, "view"), 1, GL_FALSE, &skybox_views[i][0][0]);
	
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_tex_id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
		//renderCube(); // renders a 1x1 cube
		glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 预滤波环境贴图计算shader
	map<EShaderType, string> prefilter_shader = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/preprocess_common.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/prefilter_map.frag")}
	};
	
	prefilter_shader_id = Shader::LoadShaders(prefilter_shader);
	assert(prefilter_shader_id, "load skybox shader failed");
	
	// 预滤波环境贴图
	glGenTextures(1, &prefilter_map_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map_id);
	for(unsigned i = 0; i <6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB32F, 128, 128, 0,
			GL_RGB, GL_FLOAT, nullptr);
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	// 启用三线性过滤
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// 和环境光辐照度积分不同，镜面反射依赖于材质表面的粗糙度。镜面反射光的形状，称为镜面波瓣(Specular lobe)依赖于入射方向和粗糙度
	// 采用蒙特卡洛积分和重要性采样（大数定律）
	// 从总体（理论上大小是无限）中挑选样本N生成采样值并求平均
	// 用低差异序列随机采样的蒙特卡洛方法（拟蒙特卡洛积分）+ 重要性采样（可以做到实时求解）
	glUseProgram(prefilter_shader_id);
	glUniform1i(glGetUniformLocation(prefilter_shader_id, "cube_map"), 0);
	glUniformMatrix4fv(glGetUniformLocation(prefilter_shader_id, "projection"), 1, GL_FALSE, &projection[0][0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);

	glBindFramebuffer(GL_FRAMEBUFFER, preprocess_fbo);
	// 启用立方体贴图之间的正确过渡，去除面之间的接缝
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	int map_mip_levels = 5;
	for(int mip = 0; mip < map_mip_levels; ++mip)
	{
		unsigned int mip_width = 128 * pow(0.5, mip);
		unsigned int mip_height = 128 * pow(0.5, mip);
		
		glBindRenderbuffer(GL_RENDERBUFFER, preprocess_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height);
		glViewport(0, 0, mip_width, mip_height);

		// 不同的粗糙度对应不同的mip等级
		float roughness = (float)mip/(float)(map_mip_levels - 1);
		glUniform1f(glGetUniformLocation(prefilter_shader_id, "roughness"), roughness);

		for (unsigned int i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(glGetUniformLocation(prefilter_shader_id, "view"), 1, GL_FALSE, &skybox_views[i][0][0]);
	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
								   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_map_id, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(render_info.vertex_array_id);	// 绑定VAO
			glDrawElements(GL_TRIANGLES, render_info.indices_count, GL_UNSIGNED_INT, 0);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

	// brdf预计算贴图shader
	map<EShaderType, string> brdf_lut_shader = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/skybox/brdf_lut.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/skybox/brdf_lut.frag")}
	};
	
	brdf_lut_shader_id = Shader::LoadShaders(brdf_lut_shader);
	assert(brdf_lut_shader_id, "load skybox shader failed");
	
	// brdf预计算贴图
	glGenTextures(1, &brdf_lut_map_id);
	glBindTexture(GL_TEXTURE_2D, brdf_lut_map_id);
	// 只需要RG两个分量就行
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RG16F, 512, 512, 0,
			GL_RG, GL_FLOAT, 0);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindFramebuffer(GL_FRAMEBUFFER, preprocess_fbo);
	
	glBindRenderbuffer(GL_RENDERBUFFER, preprocess_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut_map_id, 0);
	
	glViewport(0, 0, 512, 512);
	glUseProgram(brdf_lut_shader_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	quad_shape->Draw();
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
#endif
}

void CSkyBox::Render(const glm::mat4& mvp)
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(shader_id);
	//Shader::SetMat4(m_BoxMesh.shader_id, "MVP", mvp);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "MVP"), 1, GL_FALSE, &mvp[0][0]);
	glUniform1i(glGetUniformLocation(shader_id, "skybox"), 0);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
	//glBindTexture(GL_TEXTURE_2D, brdf_lut_map_id);
	//quad_shape->Draw();
	box_mesh->Draw();
}
