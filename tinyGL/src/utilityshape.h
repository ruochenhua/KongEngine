#pragma once
#include "MeshComponent.h"

namespace tinyGL
{
	class CUtilityBox : public CMeshComponent
	{
	public:
		CUtilityBox(const SRenderResourceDesc& render_resource_desc);
		virtual std::vector<float> GetVertices() const;
		
		virtual std::vector<unsigned int> GetIndices() const;
	private:
		static std::vector<float> s_vBoxVertices;
		static std::vector<float> s_vBoxNormals;
		static std::vector<float> s_vBoxTexCoords;
		static std::vector<int> s_vBoxIndices;

		std::string texture_path;
		std::string specular_map_path;

		SRenderResourceDesc box_render_resource_desc;
	};
}