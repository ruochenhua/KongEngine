#include "stdafx.h"
#include "CollisionSolver.h"
#include "SphereShape.h"
#include "BoxShape.h"
#include "CollisionHelper.h"

namespace Tap
{
	bool CCollisionSolver::Detect(std::unordered_map<int, RigidBodyUPtr>& rb_map)
	{
		m_vCollisionManifold.clear();
		if (rb_map.size() < 2)
			return true;

		auto iter_0 = rb_map.begin();
		for (; iter_0 != rb_map.end(); ++iter_0)
		{
			auto iter_1 = iter_0; ++iter_1;
			for (; iter_1 != rb_map.end(); ++iter_1)
			{
				DetectRigidBodyCollision(iter_0->second.get(), iter_1->second.get());
			}
		}

		return true;
	}

	bool CCollisionSolver::Solve()
	{
		for (auto& manifold : m_vCollisionManifold)
		{
			//apply linear momentum
			//for first rigidbody
			auto& rb_1 = manifold._first_rb;
			auto& rb_2 = manifold._second_rb;

			glm::vec3 relative_vel = rb_1->m_LinearMomentom*rb_1->m_MassInv 
				- rb_2->m_LinearMomentom*rb_2->m_MassInv;
			glm::vec3& normal = manifold._hit_normal;

			//calculate impulse
			//lower part
			//惯性张量的逆矩阵
			glm::mat3 inertia_inv_1 = glm::inverse(rb_1->m_InertiaTensor);
			glm::mat3 inertia_inv_2 = glm::inverse(rb_2->m_InertiaTensor);
			//刚体的质心到接触点的向量			
			glm::vec3 to_contact_1 = manifold._hit_pos - rb_1->m_Transform.GetPosition();
			glm::vec3 to_contact_2 = manifold._hit_pos - rb_2->m_Transform.GetPosition();
			//normal用统一一个相对于第一个刚体的
			glm::vec3 p1 = inertia_inv_1 * (glm::cross(glm::cross(to_contact_1, normal), to_contact_1));
			glm::vec3 p2 = inertia_inv_2 * (glm::cross(glm::cross(to_contact_2, normal), to_contact_2));

			//如果不是动态类型，mass是无穷大，mass inv就是0
			float mass_inv_1 = (rb_1->GetRigidBodyType() == RIGIDBODY_DYNAMIC) ? rb_1->m_MassInv : 0;
			float mass_inv_2 = (rb_2->GetRigidBodyType() == RIGIDBODY_DYNAMIC) ? rb_2->m_MassInv : 0;

			float lower_part = mass_inv_1 + mass_inv_2 + glm::dot(p1+p2, normal);

			if (manifold._first_rb->GetRigidBodyType() == RIGIDBODY_DYNAMIC)
			{
				//apply impulse to dynamic rigidbody
				ApplyImpulse(manifold._first_rb, relative_vel, normal, to_contact_1, lower_part);
			}
			//for second rigidbody
			if (manifold._second_rb->GetRigidBodyType() == RIGIDBODY_DYNAMIC)
			{
				//apply impulse
				ApplyImpulse(manifold._second_rb, relative_vel, -normal, to_contact_2, lower_part);
			}			
		}

		return true;
	}

	const std::vector<SCollisionManifold>& CCollisionSolver::GetCollisionManifold() const
	{
		return m_vCollisionManifold;
	}

	bool CCollisionSolver::DetectRigidBodyCollision(CRigidBody* rb_0, CRigidBody* rb_1)
	{
		//目前先做两个球体的
		if (rb_0->m_Shape->GetShapeType() == SHAPE_SPHERE && rb_1->m_Shape->GetShapeType() == SHAPE_SPHERE)
		{
			SCollisionManifold collision;
			if (CCDA::SolveSphereToSphereCollision(rb_0, rb_1, collision))
			{
				m_vCollisionManifold.push_back(collision);
				return true;
			}
		}

		if (rb_0->m_Shape->GetShapeType() == SHAPE_BOX && rb_1->m_Shape->GetShapeType() == SHAPE_BOX)
		{
			SCollisionManifold collision;
			if (CCDA::SolveBoxToBoxCollision(rb_0, rb_1, collision))
			{
				m_vCollisionManifold.push_back(collision);
				return true;
			}

		}

		return false;
	}

	const double COLLISION_EPSILON = 0.00001f;

	void CCollisionSolver::ApplyImpulse(CRigidBody *rb, const glm::vec3& relative_vel, const glm::vec3& hit_normal, const glm::vec3& to_contact, float lower_part)
	{
		//calculating impulse
		float jr = glm::max(glm::dot(-(1.0f + rb->GetMaterial()._elastic)*relative_vel, hit_normal), 0.0f);
		jr /= lower_part;

		//linear momentom added
		rb->m_LinearMomentom += jr * hit_normal;

		glm::mat3 inertia_inv = glm::inverse(rb->m_InertiaTensor);
		//angular momentom added
		glm::vec3 ang_cross = glm::cross(to_contact, hit_normal);
		rb->m_AngularMomentom += jr * inertia_inv * glm::cross(to_contact, hit_normal) * rb->m_Mass;

		//calculate friction
		//disable for now
// 		glm::vec3 tangent(0);
// 
// 		if (glm::dot(relative_vel, hit_normal) != 0)
// 		{
// 			glm::vec3 dr = glm::dot(relative_vel, hit_normal)*hit_normal;
// 			double dist = glm::distance(relative_vel, dr);
// 			if (dist > COLLISION_EPSILON)
// 				tangent = glm::normalize(relative_vel - dr);
// 
// 			if(glm::length(tangent) > 0)
// 				printf("tangent 0 %f %f %f\n", tangent.x, tangent.y, tangent.z);
// 		}
// 		else if (glm::dot(rb->m_Force, hit_normal) != 0)
// 		{
// 			glm::vec3 dr = glm::dot(rb->m_Force, hit_normal)*hit_normal;
// 			if (glm::distance(rb->m_Force, dr) > glm::epsilon<float>())
// 				tangent = glm::normalize(rb->m_Force - dr);
// 		}
// 		//test friction factor as 0.1
// 		rb->m_AngularMomentom += -jr * 0.1f*tangent;
	}

	bool CCDA::SolveSphereToSphereCollision(CRigidBody* rb_0, CRigidBody* rb_1, SCollisionManifold& manifold)
	{
		if(rb_0->m_Shape->GetShapeType() != SHAPE_SPHERE 
			|| rb_1->m_Shape->GetShapeType()!=SHAPE_SPHERE)
		{
			return false;
		}

		auto shape_0 = dynamic_cast<CSphereShape*>(rb_0->m_Shape.get());
		auto shape_1 = dynamic_cast<CSphereShape*>(rb_1->m_Shape.get());
		if (!shape_0 || !shape_1)
		{
			return false;
		}

		glm::vec3 pos_0 = rb_0->GetTransform().GetPosition();
		glm::vec3 pos_1 = rb_1->GetTransform().GetPosition();
		
		float dis = glm::distance(pos_0, pos_1);
		float radius_sum = shape_0->GetRadius() + shape_1->GetRadius();

		if (dis > radius_sum)	//或者采用平方
		{
			return false;
		}

		manifold._first_rb = rb_0;
		manifold._second_rb = rb_1;

		
		glm::vec3 p0_2_p1 = pos_1 - pos_0;
		glm::vec3 unit_dir = glm::normalize(p0_2_p1);
		float hit_depth = radius_sum - dis;

		manifold._hit_normal = unit_dir;
		manifold._hit_depth = hit_depth;

		float dis_p0_2_hit = shape_0->GetRadius() - hit_depth * 0.5f;	//p0到碰撞点的距离
		manifold._hit_pos = pos_0 + unit_dir * dis_p0_2_hit;	//碰撞点

		
// 		//debug print
// 		printf("===========================\n");
// 		printf("hit detect %f, %f, %f, %f\n", manifold._hit_pos.x, manifold._hit_pos.y, manifold._hit_pos.z, dis_p0_2_hit);
// 		printf("p0 to p1 %f, %f, %f\n", p0_2_p1.x, p0_2_p1.y, p0_2_p1.z);
		return true;
	}

	bool CCDA::SolveBoxToBoxCollision(CRigidBody* rb_0, CRigidBody* rb_1, SCollisionManifold& manifold)
	{
		if (rb_0->m_Shape->GetShapeType() != SHAPE_BOX
			|| rb_1->m_Shape->GetShapeType() != SHAPE_BOX)
		{
			return false;
		}

		auto shape_0 = dynamic_cast<CBoxShape*>(rb_0->m_Shape.get());
		auto shape_1 = dynamic_cast<CBoxShape*>(rb_1->m_Shape.get());

		if (!shape_0 || !shape_1)
		{
			return false;
		}

		 //根据SAT判断两个box是否碰撞，需要获取两个盒子的三个轴向的方向和长度
		auto& t0 = rb_0->GetTransform();
		auto& t1 = rb_1->GetTransform();

		CBoxCollisionHelper collision_helper(shape_0, t0, shape_1, t1);
		if (collision_helper.IsCollide(manifold))
		{
			manifold._first_rb = rb_0;
			manifold._second_rb = rb_1;
			return true;
		}
		else
			return false;
	}
}