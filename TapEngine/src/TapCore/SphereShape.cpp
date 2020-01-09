#include "stdafx.h"
#include "SphereShape.h"

namespace Tap
{
	SSphereShapeDesc::SSphereShapeDesc(float radius)
		: SShapeDesc(SHAPE_SPHERE)
		, _radius(radius)
	{

	}

    CSphereShape::CSphereShape(SSphereShapeDesc* sphere_desc)
		: CShape(sphere_desc)
		, m_Radius(sphere_desc->_radius)
    {

    }
};