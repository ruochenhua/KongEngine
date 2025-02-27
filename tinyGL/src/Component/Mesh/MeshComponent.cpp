#include "MeshComponent.h"
//#include "OBJ_Loader.h"

#include "Render/RenderModule.hpp"
#include "Shader/OpenGL/OpenGLShader.h"

#include "glm/gtc/random.hpp"
#include "Parser/ResourceManager.h"
#include "Render/GraphicsAPI/Vulkan/VulkanRenderInfo.hpp"

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

#ifdef RENDER_IN_VULKAN
void CMeshComponent::Draw(const FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout)
{
	for(auto& mesh : mesh_resource->mesh_list)
	{
		auto& render_vertex = mesh->m_RenderInfo;

		shared_ptr<VulkanMaterialInfo> vulkanMaterial;
		if (use_override_material)
		{
			vulkanMaterial = dynamic_pointer_cast<VulkanMaterialInfo>(override_render_info->material);
		}
		else
		{
			vulkanMaterial = dynamic_pointer_cast<VulkanMaterialInfo>(mesh->m_RenderInfo->material);	
		}
		if (!vulkanMaterial)
		{
			continue;
		}
		
		vkCmdBindDescriptorSets(
		    frameInfo.commandBuffer,
		    VK_PIPELINE_BIND_POINT_GRAPHICS,
		    pipelineLayout,
		    0, 1,
		    &vulkanMaterial->m_discriptorSets[frameInfo.frameIndex][VulkanDescriptorSetLayout::BasicData]
		    	, 0, nullptr);
		
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			1, 1,
			&vulkanMaterial->m_discriptorSets[frameInfo.frameIndex][VulkanDescriptorSetLayout::Texture]
				, 0, nullptr);
		
		render_vertex->Draw(frameInfo.commandBuffer);
	}
}

void CMeshComponent::UpdateMeshUBO(const FrameInfo& frameInfo)
{
	for(auto& mesh : mesh_resource->mesh_list)
	{
		shared_ptr<VulkanMaterialInfo> vulkanMaterial;
		if (use_override_material)
		{
			vulkanMaterial = dynamic_pointer_cast<VulkanMaterialInfo>(override_render_info->material);
		}
		else
		{
			vulkanMaterial = dynamic_pointer_cast<VulkanMaterialInfo>(mesh->m_RenderInfo->material);	
		}
		 
		if (!vulkanMaterial)
		{
			continue;
		}

		auto camera = KongRenderModule::GetRenderModule().GetCamera();
		SimpleVulkanRenderSystem::SimpleVulkanUbo ubo{};
		ubo.projectionView =camera->GetProjectionMatrix() * camera->GetViewMatrix();
		ubo.cameraPosition = vec4(camera->GetPosition(), 1.0);
		ubo.use_texture = vulkanMaterial->GetTextureByType(diffuse) ? 1 : 0;
      
		vulkanMaterial->m_uboBuffers[frameInfo.frameIndex]->WriteToBuffer(&ubo);
		vulkanMaterial->m_uboBuffers[frameInfo.frameIndex]->Flush();
	}
}

void CMeshComponent::CreateMeshDescriptorSet(const std::vector<std::unique_ptr<VulkanDescriptorSetLayout>>& descriptorSetLayout, VulkanDescriptorPool* descriptorPool)
{
	for(auto& mesh : mesh_resource->mesh_list)
	{
		VulkanMaterialInfo* vulkanMaterial = nullptr;
		if (use_override_material)
		{
			vulkanMaterial = dynamic_pointer_cast<VulkanMaterialInfo>(override_render_info->material).get();
			if (!vulkanMaterial)
			{
				continue;
			}
			for (auto& layout : descriptorSetLayout)
			{
				vulkanMaterial->CreateDescriptorSet(layout.get(), descriptorPool);	
			}
			break;
		}
		else
		{
			vulkanMaterial = dynamic_pointer_cast<VulkanMaterialInfo>(mesh->m_RenderInfo->material).get();
			if (!vulkanMaterial)
			{
				continue;
			}
			for (auto& layout : descriptorSetLayout)
			{
				vulkanMaterial->CreateDescriptorSet(layout.get(), descriptorPool);	
			}
		}
	}
	
}
#endif

int CMeshComponent::ImportObj(const std::string& model_path)
{
	mesh_resource = ResourceManager::GetOrLoadMesh(model_path);
	
	return 0;
}

