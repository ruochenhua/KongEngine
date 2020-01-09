#pragma once
#include "glm/glm.hpp"
namespace Tap
{
	class CRigidBody;

	class CIntegration
	{
	public:
		//简单的欧拉计算方法 semi implicit
		static void EulerIntegration(CRigidBody* rb, double dt);
	private:
	};
}