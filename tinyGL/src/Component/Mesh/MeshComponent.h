#pragma once
#include "Common.h"
#include "Component/Component.h"
#include "Parser/ResourceManager.h"
#include "Render/GraphicsAPI/OpenGL/OpenGLBuffer.hpp"
#include "Render/GraphicsAPI/Vulkan/VulkanBuffer.hpp"
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
				
		// 覆盖原有材质 
		std::unique_ptr<RenderInfo> override_render_info;
		bool use_override_material = false;
	
	protected:
		// import obj model
		int ImportObj(const std::string& model_path);
	};
}
