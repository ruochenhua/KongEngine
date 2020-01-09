#pragma once
#include "Shape.h"

namespace Tap
{
	struct SSphereShapeDesc : public SShapeDesc
	{
		//球状初始化参数,半径
		SSphereShapeDesc(float radius);

		float _radius;
	};

    class CSphereShape : public CShape
    {
    public:
        CSphereShape(SSphereShapeDesc* sphere_desc);
		virtual void TmpFunc() {};

		float GetRadius() const { return m_Radius; }
    private:
        
		float m_Radius;
    };
};