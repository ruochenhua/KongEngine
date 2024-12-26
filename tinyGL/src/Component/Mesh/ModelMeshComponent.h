#pragma once
#include "MeshComponent.h"

namespace Kong
{
	class CModelMeshComponent : public CMeshComponent
	{
	public:
		CModelMeshComponent(const string& model_path);
		virtual void Draw(const SSceneLightInfo& scene_render_info) override;
	};
}