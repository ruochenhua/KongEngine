#pragma once
#include "Shape.h"
#include "glm/glm.hpp"

namespace Tap
{
	struct SBoxShapeDesc : public SShapeDesc
	{
		//盒状的初始化参数,为长宽高的半长
		SBoxShapeDesc(const glm::vec3& half_extent);
		SBoxShapeDesc(float h_x, float h_y, float h_z);

		glm::vec3 _halfExtent;
	};

    class CBoxShape : public CShape
    {
    public:
        CBoxShape(SBoxShapeDesc* box_desc);
    
		glm::vec3 GetHalfExtent() const;
    private:
        
		glm::vec3 m_HalfExtent;
    };
};