#pragma once
#include <vector>
#include <memory>
#include "Shape.h"
#include "Transform.h"

namespace Tap
{
	enum RIGIDBODY_TYPE
	{
		RIGIDBODY_STATIC = 0,
		RIGIDBODY_DYNAMIC,
		RIGIDBODY_KINEMATIC,
		RIGIDBODY_COUNT,
	};

	struct SMaterial
	{
		SMaterial()
			: _elastic(0.5f)
		{}

		float _elastic;	//����
	};

    class CRigidBody
    {
    public:
        CRigidBody(int id);

		//�����״,������״��id
		bool AttachShape(SShapeDesc* shape_desc);
		//ɾ����״
		//TODO: Ŀǰֻ֧��һ����״, ����֧�ֶ���״
		bool DeleteShape();

		// getter and setter
		int GetID() const; 
		float GetMass() const;

		CTransform& GetTransform();
		glm::vec3 GetLinearMomentom() const;
		glm::vec3 GetAngularMomentom() const;

		glm::vec3 GetForce() const;		

		RIGIDBODY_TYPE GetRigidBodyType() const;
		SMaterial& GetMaterial();

		void SetMass(float mass);
		void SetRigidBodyType(RIGIDBODY_TYPE type);
		void SetForce(const glm::vec3& force);
    private:
		//Ϊ�˷���,��������ײ��ⶼ����Ϊfriend class
		friend class CIntegration;
		friend class CCollisionSolver;
		friend class CCDA;

        int m_ID;
        //transform
		CTransform m_Transform;

		//�����������
		float m_Mass;
		float m_MassInv;    //inv of mass, need a lot

		glm::vec3 m_LinearMomentom;
		glm::vec3 m_AngularMomentom;

		glm::vec3 m_Force;	// ���Ե���
		glm::vec3 m_Torque;	// ��ת������

		glm::mat3 m_InertiaTensor;	//��������

		RIGIDBODY_TYPE m_eRigidbodyType;

		//��������
		SMaterial m_material;

        //std::vector<CShape> m_shape; //support multiple shape later
		std::unique_ptr<CShape> m_Shape;
    };

    typedef std::unique_ptr<CRigidBody> RigidBodyUPtr; 
}