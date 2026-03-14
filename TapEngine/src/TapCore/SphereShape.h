#pragma once
#include "Shape.h"

namespace Tap
{
	struct SSphereShapeDesc : public SShapeDesc
	{
		//��״��ʼ������,�뾶
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