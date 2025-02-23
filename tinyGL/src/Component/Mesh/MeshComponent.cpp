#include "MeshComponent.h"
//#include "OBJ_Loader.h"

#include "Render/RenderModule.hpp"
#include "Shader/OpenGL/OpenGLShader.h"

#include "glm/gtc/random.hpp"
#include "Parser/ResourceManager.h"

using namespace Kong;
using namespace glm;

CMeshComponent::CMeshComponent()
#ifdef RENDER_IN_VULKAN
 : override_render_info(std::make_unique<VulkanRenderInfo>())
#else
 : override_render_info(std::make_unique<OpenGLRenderInfo>())
#endif
{
}

void CMeshComponent::BeginPlay()
{
	CComponent::BeginPlay();
	InitRenderInfo();
}

void CMeshComponent::DrawShadowInfo(shared_ptr<OpenGLShader> simple_draw_shader)
{
	for(auto& mesh : mesh_resource->mesh_list)
	{
		auto& render_vertex = mesh->m_RenderInfo;
		if(simple_draw_shader)
		{
			if(use_override_material)
			{
				simple_draw_shader->SetVec4("albedo", override_render_info->material->albedo);	
			}
			else
			{
				simple_draw_shader->SetVec4("albedo", mesh->m_RenderInfo->material->albedo);		
			}
			
		}
		
		render_vertex->Draw(nullptr);
	}
}

void CMeshComponent::Draw(void* commandBuffer)
{
	if (shader_data)
		shader_data->Use();
	
	for(auto& mesh : mesh_resource->mesh_list)
	{
		auto& render_vertex = mesh->m_RenderInfo;
		
		if (shader_data)
		{
			if(use_override_material)
			{
				shader_data->UpdateRenderData(override_render_info->material);
			}
			else
			{
				shader_data->UpdateRenderData(mesh->m_RenderInfo->material);
			}
		}
		
		render_vertex->Draw(commandBuffer);
	}
}

void CMeshComponent::InitRenderInfo()
{
	// compile shader map
	for(auto mesh : mesh_resource->mesh_list)
	{
		// 构建默认的shader数据结构，数据齐全，但是冗余
		mesh->m_RenderInfo->InitRenderInfo();
	}
}

bool CMeshComponent::IsBlend()
{
	return shader_data->bIsBlend;
}


int CMeshComponent::ImportObj(const std::string& model_path)
{
	mesh_resource = ResourceManager::GetOrLoadMesh(model_path);
	
	return 0;
}

