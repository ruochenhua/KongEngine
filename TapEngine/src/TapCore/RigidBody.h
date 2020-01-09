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

		float _elastic;	//弹性
	};

    class CRigidBody
    {
    public:
        CRigidBody(int id);

		//添加形状,返回形状的id
		bool AttachShape(SShapeDesc* shape_desc);
		//删除形状
		//TODO: 目前只支持一个形状, 后续支持多形状
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
		//为了方便,迭代和碰撞检测都设置为friend class
		friend class CIntegration;
		friend class CCollisionSolver;
		friend class CCDA;

        int m_ID;
        //transform
		CTransform m_Transform;

		//动能数据相关
		float m_Mass;
		float m_MassInv;    //inv of mass, need a lot

		glm::vec3 m_LinearMomentom;
		glm::vec3 m_AngularMomentom;

		glm::vec3 m_Force;	// 线性的力
		glm::vec3 m_Torque;	// 旋转的力矩

		glm::mat3 m_InertiaTensor;	//惯性张量

		RIGIDBODY_TYPE m_eRigidbodyType;

		//材料属性
		SMaterial m_material;

        //std::vector<CShape> m_shape; //support multiple shape later
		std::unique_ptr<CShape> m_Shape;
    };

    typedef std::unique_ptr<CRigidBody> RigidBodyUPtr; 
}