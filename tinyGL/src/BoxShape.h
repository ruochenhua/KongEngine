#pragma once
#include "MeshComponent.h"

namespace tinyGL
{
	class CBoxShape : public CMeshComponent
	{
	public:
		CBoxShape(const SRenderResourceDesc& render_resource_desc);
		virtual void Draw(const SSceneRenderInfo& scene_render_info) override;
	
	//	void InitRenderInfo() override;
	private:
		
		static std::string box_model_path;
	};
}