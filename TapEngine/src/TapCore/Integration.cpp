#include "stdafx.h"
#include "Intergration.h"
#include "RigidBody.h"

namespace Tap
{
	//for now only g force applied
	const float G_FORCE = -9.81f * 0.1f;

	void CIntegration::EulerIntegration(CRigidBody* rb, double dt)
	{				
		//Ðý×ª		
		if (rb->GetRigidBodyType() != RIGIDBODY_DYNAMIC)
			return;

		// TODO: apply damping later

		//Î»ÖÃ
		glm::vec3 pos = rb->m_Transform.GetPosition();
		//printf("rb %d pos %f %f %f\n", rb->GetID(), pos.x, pos.y, pos.z);
		rb->m_Transform.SetPosition(pos + rb->m_LinearMomentom*(float)dt * rb->m_MassInv);

		rb->m_Force.y += G_FORCE;
		rb->m_LinearMomentom += rb->m_Force*(float)dt;
		
		glm::quat orient = rb->m_Transform.GetRotation();
		glm::quat q_ang(rb->m_AngularMomentom*(float)dt * rb->m_MassInv);		
		rb->m_Transform.SetRotation(orient*q_ang);
		
 		//printf("=========================\n");
 		//printf("rb %d rot %f %f %f %f\n", rb->GetID(), dt, rb->m_Transform.GetRotation().y, rb->m_Transform.GetRotation().z, rb->m_Transform.GetRotation().w);
 		//printf("rb %d ang %f %f %f\n", rb->GetID(), rb->m_AngularMomentom.x, rb->m_AngularMomentom.y, rb->m_AngularMomentom.z);		
	}
}