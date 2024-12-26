#pragma once
#include "MeshComponent.h"

namespace Kong
{
	class CBoxShape : public CMeshComponent
	{
	public:
		CBoxShape();
		virtual void Draw(const SSceneLightInfo& scene_render_info) override;
		void Draw();	
	//	void InitRenderInfo() override;
	private:
		
		static std::string box_model_path;
	};
}