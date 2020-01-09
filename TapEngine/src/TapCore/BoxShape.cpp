#include "stdafx.h"
#include "BoxShape.h"

namespace Tap
{
	SBoxShapeDesc::SBoxShapeDesc(const glm::vec3& half_extent)
		: SShapeDesc(SHAPE_BOX)
		, _halfExtent(half_extent)
	{

	}

	SBoxShapeDesc::SBoxShapeDesc(float h_x, float h_y, float h_z)
		: SShapeDesc(SHAPE_BOX)
		, _halfExtent(glm::vec3(h_x, h_y, h_z))
	{

	}

    CBoxShape::CBoxShape(SBoxShapeDesc* box_desc)
		: CShape(box_desc), m_HalfExtent(box_desc->_halfExtent)
    {

    }

	glm::vec3 CBoxShape::GetHalfExtent() const
	{
		return m_HalfExtent;
	}
};