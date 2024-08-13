#pragma once
#include "MeshComponent.h"

namespace tinyGL
{
	class CBoxShape : public CMeshComponent
	{
	public:
		CBoxShape(const SRenderResourceDesc& render_resource_desc);
		virtual void Draw(const SSceneRenderInfo& scene_render_info) override;
	private:
		void InitBoxData(const SRenderResourceDesc& render_resource_desc);
		
		static std::vector<float> s_vBoxVertices;
		static std::vector<float> s_vBoxNormals;
		static std::vector<float> s_vBoxTexCoords;
		static std::vector<int> s_vBoxIndices;

		std::string texture_path;
		std::string specular_map_path;

	};
}