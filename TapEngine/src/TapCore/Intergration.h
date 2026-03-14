#pragma once
#include "glm/glm.hpp"
namespace Tap
{
	class CRigidBody;

	class CIntegration
	{
	public:
		//�򵥵�ŷ�����㷽�� semi implicit
		static void EulerIntegration(CRigidBody* rb, double dt);
	private:
	};
}