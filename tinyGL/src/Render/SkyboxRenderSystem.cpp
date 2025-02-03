#include "SkyboxRenderSystem.hpp"

#include "Utils.hpp"
#include "RenderModule.hpp"
#include "Scene.hpp"
#include "stb_image.h"
#include "Texture.hpp"
#include "Window.hpp"
#include "Component/Mesh/QuadShape.h"
#include "Shader/Shader.h"

using namespace Kong;

constexpr int CUBE_MAP_RES = 1024;

#define USE_HDR_SKYBOX 1

void SkyboxRenderSystem::Init()
{
	box_mesh = make_shared<CBoxShape>();
	// 这里begin play一下会创建一下对应的顶点buffer等数据
	box_mesh->BeginPlay();
	
	quad_shape = make_shared<CQuadShape>();
	quad_shape->BeginPlay();
	
	skybox_shader = make_shared<SkyboxShader>();
	atmosphere_shader = make_shared<AtmosphereShader>();
	equirectangular_to_cubemap_shader = make_shared<EquirectangularToCubemapShader>();
	irradiance_calculation_shader = make_shared<IrradianceCalculationShader>();
	prefilter_calculation_shader = make_shared<PrefilterCalculationShader>();
	brdf_lut_calculation_shader = make_shared<BRDFLutCalculationShader>();
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
	
	////////// 创建skybox预处理帧缓冲
	glGenFramebuffers(1, &preprocess_fbo);
	glGenRenderbuffers(1, &preprocess_rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, preprocess_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, preprocess_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CUBE_MAP_RES, CUBE_MAP_RES);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, preprocess_rbo);
	
	////////// 创建立方体贴图
	TextureCreateInfo cubemap_create_info {
		GL_TEXTURE_CUBE_MAP, GL_RGB32F, GL_RGB,
		GL_FLOAT, CUBE_MAP_RES, CUBE_MAP_RES,
				GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
				GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR
		};

	TextureBuilder::CreateTexture(cube_map_id, cubemap_create_info);
	
	// ////////// 创建辐照度立方体贴图
	TextureCreateInfo irradiance_create_info {cubemap_create_info};
	irradiance_create_info.width = irradiance_create_info.height = 32;
	irradiance_create_info.minFilter = GL_LINEAR;

	TextureBuilder::CreateTexture(irradiance_tex_id, irradiance_create_info);

	////////// 预滤波环境贴图
	TextureCreateInfo prefilter_create_info {cubemap_create_info};
	prefilter_create_info.width = prefilter_create_info.height = 128;
	TextureBuilder::CreateTexture(prefilter_map_id, prefilter_create_info);

	////////// brdf预计算贴图
	TextureCreateInfo brdf_lut_map_create_info {};
	brdf_lut_map_create_info.width = brdf_lut_map_create_info.height = CUBE_MAP_RES;
	brdf_lut_map_create_info.wrapS = brdf_lut_map_create_info.wrapR = brdf_lut_map_create_info.wrapT = GL_CLAMP_TO_EDGE;
	brdf_lut_map_create_info.minFilter = brdf_lut_map_create_info.magFilter = GL_LINEAR;
	brdf_lut_map_create_info.format = GL_RG;
	brdf_lut_map_create_info.data_type = GL_FLOAT;
	brdf_lut_map_create_info.internalFormat = GL_RG16F;

	TextureBuilder::CreateTexture(brdf_lut_map_id, brdf_lut_map_create_info);

	skybox_res_list.emplace_back(CSceneLoader::ToResourcePath("sky_box/newport_loft.hdr"));
	skybox_res_list.emplace_back(CSceneLoader::ToResourcePath("sky_box/illovo_beach_balcony_4k.hdr"));
	
	PreprocessIBL(skybox_res_list[current_skybox_idx]);
	
#endif
	
	// init volumetric cloud
	volumetric_cloud_ = make_shared<VolumetricCloud>();
}

RenderResultInfo SkyboxRenderSystem::Draw(double delta, const RenderResultInfo& render_result_info, KongRenderModule* render_module)
{
	if (render_module == nullptr || render_module->render_sky_env_status == 0)
	{
		return RenderResultInfo{};
	}

	if (render_result_info.frameBuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, render_result_info.frameBuffer);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	}
	
	Render(render_module->render_sky_env_status, render_result_info.resultDepth);
	return render_result_info;
}

void SkyboxRenderSystem::PreprocessIBL(const string& hdr_file_path)
{
	int width, height, nr_component;
	stbi_set_flip_vertically_on_load(true);
	auto data = stbi_loadf(hdr_file_path.c_str(), &width, &height, &nr_component, 0);
	if (data == nullptr)
	{
		throw std::runtime_error("failed to load hdr image for skybox");
	}

	TextureCreateInfo sphere_map_create_info {
	GL_TEXTURE_2D, GL_RGB32F, GL_RGB, GL_FLOAT, width, height,
	GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, data
	};
	
	TextureBuilder::CreateTexture(sphere_map_texture, sphere_map_create_info);
	stbi_image_free(data);

	// projection和view矩阵
	vec3 scene_center = vec3(0);
	vector<mat4> skybox_views;
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(1,0,0), vec3(0,-1,0)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(-1,0,0), vec3(0,-1,0)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(0,1,0), vec3(0,0,1)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(0,-1,0), vec3(0,0,-1)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(0,0,1), vec3(0,-1,0)));
	skybox_views.push_back(lookAt(scene_center, scene_center+vec3(0,0,-1), vec3(0,-1,0)));
	
	mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	// 将等距柱状投影图转换为立方体贴图
	equirectangular_to_cubemap_shader->Use();
	equirectangular_to_cubemap_shader->SetInt("sphere_map", 0);
	equirectangular_to_cubemap_shader->SetMat4("projection", projection);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphere_map_texture);

	glViewport(0, 0, CUBE_MAP_RES, CUBE_MAP_RES); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, preprocess_fbo);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangular_to_cubemap_shader->SetMat4("view", skybox_views[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cube_map_id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		box_mesh->Draw();
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 创建mipmap，在预滤波环境贴图的时候使用他的mipmap贴图可以有效减少采样结果异常斑点的问题
	// 转换完成写入到cube_map贴图后再生成mipmap，否则不会生效
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// 预计算辐照度贴图
	irradiance_calculation_shader->Use();
	irradiance_calculation_shader->SetInt("cube_map", 0);
	irradiance_calculation_shader->SetMat4("projection", projection);
			
	glBindFramebuffer(GL_FRAMEBUFFER, preprocess_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, preprocess_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
	
	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.

	for (unsigned int i = 0; i < 6; ++i)
	{
		irradiance_calculation_shader->SetMat4("view", skybox_views[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_tex_id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		box_mesh->Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 和环境光辐照度积分不同，镜面反射依赖于材质表面的粗糙度。镜面反射光的形状，称为镜面波瓣(Specular lobe)依赖于入射方向和粗糙度
	// 采用蒙特卡洛积分和重要性采样（大数定律）
	// 从总体（理论上大小是无限）中挑选样本N生成采样值并求平均
	// 用低差异序列随机采样的蒙特卡洛方法（拟蒙特卡洛积分）+ 重要性采样（可以做到实时求解）
	prefilter_calculation_shader->Use();
	prefilter_calculation_shader->SetInt("cube_map", 0);
	prefilter_calculation_shader->SetMat4("projection", projection);
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
		float roughness = (float)mip/((float)map_mip_levels - 1.0f);
		prefilter_calculation_shader->SetFloat("roughness", roughness);
		
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilter_calculation_shader->SetMat4("view", skybox_views[i]);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
								   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_map_id, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			box_mesh->Draw();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, preprocess_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, preprocess_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CUBE_MAP_RES, CUBE_MAP_RES);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut_map_id, 0);
	
	glViewport(0, 0, CUBE_MAP_RES, CUBE_MAP_RES);
	brdf_lut_calculation_shader->Use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	quad_shape->Draw();
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void SkyboxRenderSystem::Render(int render_sky_status, GLuint depth_texture)
{
	switch (render_sky_status)
	{
	case 1:
		{
			// 绘制天空盒
			glCullFace(GL_FRONT);
			glDepthFunc(GL_LEQUAL);
			skybox_shader->Use();
			skybox_shader->SetInt("skybox", 0);
#if USE_DSA
			glBindTextureUnit(0, cube_map_id);
#else
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_id);
#endif		
			box_mesh->Draw();
			glCullFace(GL_BACK);
		}
		break;
	case 2:
		// 绘制天空大气
		if (render_cloud)
		{
			// 渲染天空大气的时候需要关掉修改深度缓存，否则在这之后的渲染（如水）会因为深度问题被裁切掉。
			glDepthMask(GL_FALSE);
			RenderCloud(depth_texture);
			glDepthMask(GL_TRUE);
		}
		
		break;
	default:
		break;
	}
}

void SkyboxRenderSystem::RenderCloud(GLuint depth_texture)
{
	// 参照工程是在后处理里面渲染的：https://github.com/fede-vaccaro/TerrainEngine-OpenGL
	// 我们可以直接在skybox里面画
	// volumetric_cloud_->SimpleDraw();
	// 计算体积云
	
	atmosphere_shader->Use();
	auto cloud_model_ = volumetric_cloud_->cloud_model_;
	
	atmosphere_shader->SetFloat("iTime", glfwGetTime());
	//
	atmosphere_shader->SetVec4("cloud_stat_1", vec4(cloud_model_->coverage
		, cloud_model_->cloud_speed
		, cloud_model_->crispiness
		, cloud_model_->curliness));
	
	atmosphere_shader->SetVec2("cloud_stat_2", vec2(cloud_model_->absorption*0.01f, cloud_model_->density));
	atmosphere_shader->SetBool("enablePowder", cloud_model_->enable_powder);

	atmosphere_shader->SetVec3("earth_stat", vec3(cloud_model_->earth_radius, cloud_model_->sphere_inner_radius, cloud_model_->sphere_outer_radius));
	
	atmosphere_shader->SetVec3("cloudColorTop", cloud_model_->cloud_color_top);
	atmosphere_shader->SetVec3("cloudColorBottom", cloud_model_->cloud_color_bottom);
	
	atmosphere_shader->SetVec2("iResolution", KongWindow::GetWindowModule().windowSize);
	
#if USE_DSA
	glBindTextureUnit(0, depth_texture);
	glBindTextureUnit(1, cloud_model_->perlin_texture);
	glBindTextureUnit(2, cloud_model_->worley32_texture);
	glBindTextureUnit(3, cloud_model_->weather_texutre);
#else
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depth_texture);	
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, cloud_model_->perlin_texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, cloud_model_->worley32_texture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, cloud_model_->weather_texutre);
#endif
	quad_shape->Draw();
}

void SkyboxRenderSystem::ChangeSkybox()
{
	current_skybox_idx = (current_skybox_idx + 1) % skybox_res_list.size();
	PreprocessIBL(skybox_res_list[current_skybox_idx]);
}

void SkyboxRenderSystem::PreRenderUpdate()
{
	if(render_cloud)
	{
		volumetric_cloud_->PreRenderUpdate();	
	}
}
