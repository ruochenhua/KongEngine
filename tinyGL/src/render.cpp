#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Actor.h"
#include "Component/CameraComponent.h"
#include "Engine.h"
#include "Component/LightComponent.h"
#include "Component/Mesh/MeshComponent.h"
#include "message.h"
#include "Scene.h"
#include "Shader/Shader.h"
#include "stb_image.h"
#include "Component/Mesh/QuadShape.h"

using namespace Kong;
using namespace glm;
using namespace std;

map<string, GLuint> texture_manager;

CRender* g_render = new CRender;

void UBOHelper::Init(GLuint in_binding)
{
	binding = in_binding;
	// 创建UBO
	glGenBuffers(1, &ubo_idx);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_idx);
	glBufferData(GL_UNIFORM_BUFFER, next_offset, NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_idx);
	glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);
}

void UBOHelper::Bind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_idx);
}

void UBOHelper::EndBind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void DeferBuffer::Init(unsigned width, unsigned height)
{
	glGenFramebuffers(1, &g_buffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_);
	// 将当前视野的数据用贴图缓存
	// 位置数据
	glGenTextures(1, &g_position_);
	glBindTexture(GL_TEXTURE_2D, g_position_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position_, 0);
	
	// 法线数据
	glGenTextures(1, &g_normal_);
	glBindTexture(GL_TEXTURE_2D, g_normal_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal_, 0);
	
	// 顶点颜色数据
	glGenTextures(1, &g_albedo_);
	glBindTexture(GL_TEXTURE_2D, g_albedo_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_albedo_, 0);

	// orm数据（ao，roughness，metallic）
	glGenTextures(1, &g_orm_);
	glBindTexture(GL_TEXTURE_2D, g_orm_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, g_orm_, 0);

	// 生成renderbuffer
	glGenRenderbuffers(1, &g_rbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, g_rbo_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_rbo_);
	glEnable(GL_DEPTH_TEST);
	
	unsigned int attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	glDrawBuffers(4, attachments);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	quad_shape = make_shared<CQuadShape>(SRenderResourceDesc());
	quad_shape->InitRenderInfo();

	defer_render_shader = make_shared<DeferredBRDFShader>();
	defer_render_shader->InitDefaultShader();
}

CRender* CRender::GetRender()
{
	return g_render;
}
GLuint CRender::GetNullTexId()
{
	return g_render->null_tex_id;
}

GLuint CRender::GetSkyboxTexture() const
{
	return m_SkyBox.GetSkyBoxTextureId();
}

GLuint CRender::GetSkyboxDiffuseIrradianceTexture() const
{
	return m_SkyBox.GetDiffuseIrradianceTexture();
}

GLuint CRender::GetSkyboxPrefilterTexture() const
{
	return m_SkyBox.GetPrefilterTexture();
}

GLuint CRender::GetSkyboxBRDFLutTexture() const
{
	return m_SkyBox.GetBRDFLutTexture();
}

int CRender::Init()
{
	render_window = Engine::GetRenderWindow();
	InitCamera();
	
	m_SkyBox.Init();
#if SHADOWMAP_DEBUG
	map<EShaderType, string> debug_shader_paths = {
		{EShaderType::vs, CSceneLoader::ToResourcePath("shader/shadowmap_debug.vert")},
		{EShaderType::fs, CSceneLoader::ToResourcePath("shader/shadowmap_debug.frag")}
	};
	shadowmap_debug_shader = make_shared<Shader>();

	shadowmap_debug_shader->Init(debug_shader_paths);
	shadowmap_debug_shader->Use();
	shadowmap_debug_shader->SetInt("shadow_map", 0);
#endif
	
	// load null texture
	string null_tex_path = RESOURCE_PATH + "Engine/null_texture.png";
	null_tex_id = LoadTexture(null_tex_path);

	InitUBO();
	post_process.Init();

	int width = Engine::GetEngine().GetWindowWidth();
	int height = Engine::GetEngine().GetWindowHeight();
	
	defer_buffer_.Init(width, height);
	return 0;
}

int CRender::InitCamera()
{
	mainCamera = new CCamera(vec3(-4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	mainCamera->InitControl();
	return 0;
}


void CRender::InitUBO()
{
	// 初始化UBO数据
	matrix_ubo.AppendData(glm::mat4(), "model");
	matrix_ubo.AppendData(glm::mat4(), "view");
	matrix_ubo.AppendData(glm::mat4(), "projection");
	matrix_ubo.AppendData(glm::vec3(), "cam_pos");
	matrix_ubo.Init(0);

	scene_light_ubo.AppendData(SceneLightInfo(), "light_info");
	scene_light_ubo.Init(1);
}

int CRender::Update(double delta)
{
	mainCamera->Update(delta);		
	// RenderSkyBox();
	CollectLightInfo();
	
	RenderShadowMap();			
	RenderSceneObject();	
	return 1;
}

void CRender::PostUpdate()
{
	// Swap buffers
	glfwSwapBuffers(render_window);
	glfwPollEvents();
}

void CRender::RenderSceneObject()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if !SHADOWMAP_DEBUG
	int width = Engine::GetEngine().GetWindowWidth();
	int height = Engine::GetEngine().GetWindowHeight();

	glViewport(0,0, width, height);
#if USE_DERER_RENDER
	// 渲染到gbuffer上
	glBindFramebuffer(GL_FRAMEBUFFER, defer_buffer_.g_buffer_);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DeferRenderSceneToGBuffer();

#endif
	
	// 渲染到后处理framebuffer上
	glBindFramebuffer(GL_FRAMEBUFFER, post_process.GetScreenFrameBuffer());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
#if USE_DERER_RENDER
	DeferRenderSceneLighting();
	
	// 需要将延迟渲染的深度缓冲复制到后面的后处理buffer上
	glBindFramebuffer(GL_READ_FRAMEBUFFER, defer_buffer_.g_buffer_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, post_process.GetScreenFrameBuffer());
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#endif
	RenderScene();
	RenderSkyBox();
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//post_process.screen_quad_texture[0] = defer_buffer_.g_position_;
	post_process.Draw();
#endif
}

void CRender::ChangeSkybox()
{
	m_SkyBox.ChangeSkybox();
}

GLuint CRender::LoadTexture(const std::string& texture_path, bool flip_uv)
{
	auto texture_iter = texture_manager.find(texture_path);
	if(texture_iter != texture_manager.end())
	{
		return texture_iter->second;
	}
	
	GLuint texture_id = GL_NONE;
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

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// release memory
	stbi_image_free(data);
	// tex_image->read_tga_file(texture_path.c_str());
	texture_manager.emplace(texture_path, texture_id);
	return texture_id;
}


void CRender::RenderSkyBox()
{
	if(render_sky_env_status == 0)
	{
		return;
	}
	
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrixNoTranslate(); //
	//mat4 mvp = projection * mainCamera->GetViewMatrix(); //
	m_SkyBox.Render(mvp, render_sky_env_status);
}

void CRender::RenderScene() const
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	// 更新光源UBO
	SceneLightInfo light_info;
	bool has_dir_light = !scene_render_info.scene_dirlight.expired(); 
	if(has_dir_light)
	{
		light_info.has_dir_light = ivec4(1);
		auto dir_light = scene_render_info.scene_dirlight.lock();
		light_info.directional_light.light_dir = vec4(dir_light->GetLightDir(), 1.0);
		light_info.directional_light.light_color = vec4(dir_light->light_color, 1.0);
		light_info.directional_light.light_space_mat = dir_light->light_space_mat;
	}
	else
	{
		light_info.has_dir_light = ivec4(0);
	}
			
	int point_light_count = 0;
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(point_light_count > 3)
		{
			break;
		}
				
		if(light.expired())
		{
			continue;
		}
		PointLight point_light;
		point_light.light_pos = vec4(light.lock()->GetLightLocation(), 1.0);
		point_light.light_color = vec4(light.lock()->light_color, 1.0);

		light_info.point_lights[point_light_count] = point_light; 				
		++point_light_count;
	}
			
	light_info.point_light_count = ivec4(point_light_count);
	
	scene_light_ubo.Bind();
	scene_light_ubo.UpdateData(light_info, "light_info");
	scene_light_ubo.EndBind();
	
	// 更新UBO里的矩阵数据
	matrix_ubo.Bind();
	matrix_ubo.UpdateData(scene_render_info.camera_view, "view");
	matrix_ubo.UpdateData(scene_render_info.camera_proj, "projection");
	matrix_ubo.UpdateData(scene_render_info.camera_pos, "cam_pos");
	matrix_ubo.EndBind();
	
	auto actors = CScene::GetActors();
	for(auto actor : actors)
	{
		auto mesh_component = actor->GetComponent<CMeshComponent>();
		if(!mesh_component)
		{
			continue;
		}
		auto mesh_shader = mesh_component->shader_data;
		// 跳过延迟渲染的mesh
		if(dynamic_pointer_cast<DeferInfoShader>(mesh_shader))
		{
			continue;
		}
		
		matrix_ubo.Bind();
		matrix_ubo.UpdateData(actor->GetModelMatrix(), "model");
		matrix_ubo.EndBind();

		// 等于1代表渲染skybox，会需要用到环境贴图
		mesh_shader->Use();
		mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
		mesh_component->Draw(scene_render_info);
	}
}

void CRender::DeferRenderSceneToGBuffer() const
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	
	// 更新UBO里的矩阵数据
	matrix_ubo.Bind();
	matrix_ubo.UpdateData(scene_render_info.camera_view, "view");
	matrix_ubo.UpdateData(scene_render_info.camera_proj, "projection");
	matrix_ubo.UpdateData(scene_render_info.camera_pos, "cam_pos");
	matrix_ubo.EndBind();
	
	auto actors = CScene::GetActors();
	for(auto actor : actors)
	{
		auto mesh_component = actor->GetComponent<CMeshComponent>();
		if(!mesh_component)
		{
			continue;
		}
		auto mesh_shader = mesh_component->shader_data;
		if(!dynamic_pointer_cast<DeferInfoShader>(mesh_shader))
		{
			continue;
		}
		
		matrix_ubo.Bind();
		matrix_ubo.UpdateData(actor->GetModelMatrix(), "model");
		matrix_ubo.EndBind();

		mesh_shader->Use();
		// 等于1代表渲染skybox，会需要用到环境贴图
		mesh_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
		mesh_component->Draw(scene_render_info);
	}
}

void CRender::DeferRenderSceneLighting() const
{
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);
	// glEnable(GL_DEPTH_TEST);
	// 更新光源UBO
	SceneLightInfo light_info;
	bool has_dir_light = !scene_render_info.scene_dirlight.expired(); 
	if(has_dir_light)
	{
		light_info.has_dir_light = ivec4(1);
		auto dir_light = scene_render_info.scene_dirlight.lock();
		light_info.directional_light.light_dir = vec4(dir_light->GetLightDir(), 1.0);
		light_info.directional_light.light_color = vec4(dir_light->light_color, 1.0);
		light_info.directional_light.light_space_mat = dir_light->light_space_mat;
	}
	else
	{
		light_info.has_dir_light = ivec4(0);
	}
			
	int point_light_count = 0;
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(point_light_count > 3)
		{
			break;
		}
				
		if(light.expired())
		{
			continue;
		}
		PointLight point_light;
		point_light.light_pos = vec4(light.lock()->GetLightLocation(), 1.0);
		point_light.light_color = vec4(light.lock()->light_color, 1.0);

		light_info.point_lights[point_light_count] = point_light; 				
		++point_light_count;
	}
			
	light_info.point_light_count = ivec4(point_light_count);
	
	scene_light_ubo.Bind();
	scene_light_ubo.UpdateData(light_info, "light_info");
	scene_light_ubo.EndBind();
	
	// // 更新UBO里的矩阵数据
	matrix_ubo.Bind();
	matrix_ubo.UpdateData(scene_render_info.camera_view, "view");
	matrix_ubo.UpdateData(scene_render_info.camera_proj, "projection");
	matrix_ubo.UpdateData(scene_render_info.camera_pos, "cam_pos");
	matrix_ubo.EndBind();
	
	defer_buffer_.defer_render_shader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_position_);

	// normal map加一个法线贴图的数据
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_normal_);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_albedo_);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, defer_buffer_.g_orm_);

	defer_buffer_.defer_render_shader->SetBool("b_render_skybox", render_sky_env_status == 1);
	defer_buffer_.defer_render_shader->UpdateRenderData(defer_buffer_.quad_shape->mesh_list[0], scene_render_info);
	defer_buffer_.quad_shape->Draw();
}

void CRender::CollectLightInfo()
{
	// todo: scene重新加载的时候处理一次就好,但是transform更新需要及时
	scene_render_info.clear();
	// scene_dirlight.reset();
	// scene_pointlights.clear();
	//
	auto actors = CScene::GetActors();
	for(auto actor: actors)
	{
		auto light_component = actor->GetComponent<CLightComponent>();
		if(!light_component)
		{
			continue;
		}

		auto dir_light = std::dynamic_pointer_cast<CDirectionalLightComponent>(light_component);
		if(dir_light)
		{
			dir_light->SetLightDir(actor->rotation);
			scene_render_info.scene_dirlight = dir_light;
			continue;
		}

		if(scene_render_info.scene_pointlights.size() < 4)
		{
			auto point_light = dynamic_pointer_cast<CPointLightComponent>(light_component);
			if(point_light)
			{
				point_light->SetLightLocation(actor->location);
				scene_render_info.scene_pointlights.push_back(point_light);
			}
		}
	}

	scene_render_info.camera_pos = mainCamera->GetPosition();
	scene_render_info.camera_view = mainCamera->GetViewMatrix();
	scene_render_info.camera_proj = mainCamera->GetProjectionMatrix();
		
}

void CRender::RenderShadowMap()
{
	// shadowmap 需要正面剔除，避免阴影悬浮
	// todo: 处理内部有开口模型或者平面该如何处理？
	// note: 剔除front好像shadow bias不填阴影效果也比较正常？
	glCullFace(GL_FRONT);
	glViewport(0,0, SHADOW_WIDTH, SHADOW_HEIGHT);

	// auto scene_lights = CScene::GetScene()->GetSceneLights();
	if(!scene_render_info.scene_dirlight.expired())
	{
		scene_render_info.scene_dirlight.lock()->RenderShadowMap();
	}
	
	for(auto light : scene_render_info.scene_pointlights)
	{
		if(!light.expired())
		{
			light.lock()->RenderShadowMap();
		}
	}

#if SHADOWMAP_DEBUG
	glCullFace(GL_BACK);
	// 先只画平行光的
	CDirectionalLightComponent* dir_light = scene_render_info.scene_dirlight.lock().get();
	
	if(!dir_light)
		return;
	
	int width = Engine::GetEngine().GetWindowWidth();
	int height = Engine::GetEngine().GetWindowHeight();
	glViewport(0,0,width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shadowmap_debug_shader->Use();
	// glUseProgram(m_ShadowMapDebugShaderId);
	// Shader::SetFloat(m_ShadowMapDebugShaderId, "near_plane", dir_light->near_plane);
	// Shader::SetFloat(m_ShadowMapDebugShaderId, "far_plane", dir_light->far_plane);
	glActiveTexture(GL_TEXTURE0);

	GLuint dir_light_shadowmap_id = dir_light->GetShadowMapTexture();
	glBindTexture(GL_TEXTURE_2D, dir_light_shadowmap_id);

	// renderQuad() renders a 1x1 XY quad in NDC
	// -----------------------------------------
	if (m_QuadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &m_QuadVAO);
		glGenBuffers(1, &m_QuadVBO);
		glBindVertexArray(m_QuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(m_QuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
#endif
}

