#pragma once
#include "Common.h"
#include "Component/Component.h"
#include "Parser/ResourceManager.h"
#include "Render/GraphicsAPI/OpenGL/OpenGLBuffer.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanRenderInfo.hpp"
#include "Render/GraphicsAPI/Vulkan/RenderSystem/VulkanRenderSystem.hpp"
#include "Shader/OpenGL/OpenGLShader.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace Kong
{
	class CMeshComponent : public CComponent
	{
	public:
		shared_ptr<MeshResource> mesh_resource;
		shared_ptr<OpenGLShader> shader_data;
		
		CMeshComponent();	
		
		void BeginPlay() override;
		// 简单调用一下draw，不管shader（可能用其他的shader）
		virtual void DrawShadowInfo(shared_ptr<OpenGLShader> simple_draw_shader);
		virtual void Draw(void* commandBuffer = nullptr);
		virtual void InitRenderInfo();
		bool IsBlend();

#ifdef RENDER_IN_VULKAN
		void Draw(const FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);
		void UpdateMeshUBO(const FrameInfo& frameInfo);
		void CreateMeshDescriptorSet(const std::vector<std::unique_ptr<VulkanDescriptorSetLayout>>& descriptorSetLayout, VulkanDescriptorPool* descriptorPool);
#endif
		
		// 覆盖原有材质
#ifdef RENDER_IN_VULKAN
		std::unique_ptr<VulkanRenderInfo> override_render_info;
#else
		std::unique_ptr<OpenGLRenderInfo> override_render_info;
#endif
		bool use_override_material = false;
	
	protected:
		// import obj model
		int ImportObj(const std::string& model_path);
	};
}
