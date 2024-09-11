#pragma once
#include "Common.h"
#include "Component/Component.h"
#include "Parser/ResourceManager.h"
#include "Shader/Shader.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace Kong
{
	class CMeshComponent : public CComponent
	{
	public:
		shared_ptr<MeshResource> mesh_resource;
		shared_ptr<Shader> shader_data;
		
		CMeshComponent() = default;	
		
		void BeginPlay() override;
		// 简单调用一下draw，不管shader（可能用其他的shader）
		virtual void SimpleDraw();
		virtual void Draw(const SSceneRenderInfo& scene_render_info);
		virtual void InitRenderInfo();
		bool IsBlend();

		// 覆盖原有材质 
		SRenderInfo override_render_info;
		bool use_override_material = false;
	protected:
		// import obj model
		int ImportObj(const std::string& model_path);
	};
}
