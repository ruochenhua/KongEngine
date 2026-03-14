#pragma once
#include "BoxShape.h"
#include "Transform.h"


namespace Tap
{
	struct SCollisionManifold;
	class CBoxCollisionHelper
	{
	public:
		CBoxCollisionHelper(CBoxShape *s0,
			const CTransform& t0,
			CBoxShape *s1,
			const CTransform& t1);

		bool IsCollide(SCollisionManifold& manifold);

	private:
		CTransform m_t0, m_t1;	//transform��Ϣ
		CBoxShape *m_s0, *m_s1;	//shape��Ϣ

	private:
		//�������boxͶӰ��ĳ�����ϵ�overlap���,��ֵ������Ƿֿ���
		float PenetrationOnAxis(const glm::vec3& axis);

		//ͶӰbox��ĳ������,���ذ볤�ĳ���
		float ProjectToAxis(const glm::vec3& axis, CBoxShape* shape, const CTransform& trans);
	};
};