#pragma once
#include "renderobj.h"

namespace tinyGL
{
	class CUtilityBox : public CRenderObj
	{
	public:
		CUtilityBox(const SRenderResourceDesc& render_resource_desc);
		virtual std::vector<float> GetVertices() const;
		virtual std::vector<unsigned int> GetIndices() const;

		void GenerateRenderInfo() override;
		void InitInstancingData();
		const glm::mat4& GetInstancingModelMat(unsigned idx) const override;
	private:
		static std::vector<float> s_vBoxVertices;
		static std::vector<int> s_vBoxIndices;

		std::string texture_path;
		std::string specular_map_path;

		std::vector<glm::mat4> instancing_model_mat;
		SRenderResourceDesc box_render_resource_desc;
	};
}