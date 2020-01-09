#include "stdafx.h"
#include "RigidBody.h"
#include "SphereShape.h"
#include "BoxShape.h"

namespace Tap
{
	CRigidBody::CRigidBody(int id)
		: m_ID(id)
		, m_Mass(1.0f), m_MassInv(1.0f)
		, m_AngularMomentom(0.0f, 0.0f, 0.0f)
		, m_LinearMomentom(0.0f, 0.0f, 0.0f)
		, m_Force(0.0f, 0.0f, 0.0f)
		, m_Torque(0.0f, 0.0f, 0.0f)
		, m_eRigidbodyType(RIGIDBODY_DYNAMIC)
		, m_InertiaTensor(glm::mat3(1.0))
    {
		
    }

	bool CRigidBody::AttachShape(SShapeDesc* shape_desc)
	{
		SHAPE_TYPE type = shape_desc->shape_type;
		switch (type)
		{
		case Tap::SHAPE_NONE:
			return false;
		case Tap::SHAPE_SPHERE:
		{
			auto sph_desc = dynamic_cast<SSphereShapeDesc*>(shape_desc);
			if (!sph_desc)
				return false;

			m_Shape.reset(new CSphereShape(sph_desc));
			return true;
		}		
		case Tap::SHAPE_BOX:
		{
			auto box_desc = dynamic_cast<SBoxShapeDesc*>(shape_desc);
			if (!box_desc)
				return false;

			m_Shape.reset(new CBoxShape(box_desc));
			return true;
		}

		case Tap::SHAPE_PLANE:
			return false;
		}

		return false;
	}

	bool CRigidBody::DeleteShape()
	{
		if (!m_Shape)
			return false;

		m_Shape.reset();
		return true;
	}

	int CRigidBody::GetID() const
	{
		return m_ID;
	}

	float CRigidBody::GetMass() const 
	{
		return m_Mass;
	}

	CTransform& CRigidBody::GetTransform()
	{
		return m_Transform;
	}

	SMaterial& CRigidBody::GetMaterial()
	{
		return m_material;
	}

	glm::vec3 CRigidBody::GetLinearMomentom() const
	{
		return m_LinearMomentom;
	}

	glm::vec3 CRigidBody::GetAngularMomentom() const
	{
		return m_AngularMomentom;
	}

	glm::vec3 CRigidBody::GetForce() const
	{
		return m_Force;
	}

	RIGIDBODY_TYPE CRigidBody::GetRigidBodyType() const
	{
		return m_eRigidbodyType;
	}

	void CRigidBody::SetMass(float mass)
	{
		if (mass == 0.0f)
			return;

		m_LinearMomentom = m_LinearMomentom / m_Mass * mass;
		m_AngularMomentom = m_AngularMomentom / m_Mass * mass;
		m_Mass = mass;
		m_MassInv = 1.0f / mass;
	}

	void CRigidBody::SetRigidBodyType(RIGIDBODY_TYPE type)
	{
		m_eRigidbodyType = type;
	}

	void CRigidBody::SetForce(const glm::vec3& force)
	{
		m_Force = force;
	}
};