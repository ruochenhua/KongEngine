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
		CTransform m_t0, m_t1;	//transform信息
		CBoxShape *m_s0, *m_s1;	//shape信息

	private:
		//检查两个box投影到某个轴上的overlap情况,负值则代表是分开的
		float PenetrationOnAxis(const glm::vec3& axis);

		//投影box到某个轴上,返回半长的长度
		float ProjectToAxis(const glm::vec3& axis, CBoxShape* shape, const CTransform& trans);
	};
};