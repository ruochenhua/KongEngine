#include "stdafx.h"
#include "Shape.h"

namespace Tap
{
	SShapeDesc::SShapeDesc(SHAPE_TYPE type)
		: shape_type(type)
	{

	}

	void SShapeDesc::Init()
	{

	}

    CShape::CShape(SShapeDesc* shape_desc)
    : m_ShapeType(shape_desc->shape_type)
    {

    }
};