#pragma once
#include "glm/glm.hpp"
#include "RigidBody.h"
#include <unordered_map>

namespace Tap
{
	struct SCollisionManifold 
	{
		CRigidBody* _first_rb;
		CRigidBody* _second_rb;
		glm::vec3 _hit_pos;		
		glm::vec3 _hit_normal;	
		float _hit_depth;

		SCollisionManifold()
			: _hit_pos(0.0f, 0.0f, 0.0f)
			, _first_rb(nullptr), _second_rb(nullptr)
		{
			_hit_normal = { 0.0f,0.0f,0.0f };			
			_hit_depth = 0.0f;			
		}
	};
	
	class CCollisionSolver
	{
	public:
		//������ײ���
		bool Detect(std::unordered_map<int, RigidBodyUPtr>& rb_map);
		//�����ײ
		bool Solve();
		//��ȡ��ײ��¼
		const std::vector<SCollisionManifold>& GetCollisionManifold() const;

	private:
		std::vector<SCollisionManifold> m_vCollisionManifold;

	private:
		//�����������ײ���
		bool DetectRigidBodyCollision(CRigidBody* rb_0, CRigidBody* rb_1);

		//ʩ�ӳ���
		//lower part�ǳ������㷽��ʽ�ķ�ĸ����Ϊ���������������һ���ģ�����ֻ����һ��
		void ApplyImpulse(CRigidBody *rb, const glm::vec3& relative_vel, const glm::vec3& hit_normal, const glm::vec3& to_contact, float lower_part);
	};

	// Class for Collision Detection Algorithm
	class CCDA
	{
	public:
		static bool SolveSphereToSphereCollision(CRigidBody* rb_0, CRigidBody* rb_1, SCollisionManifold& manifold);
		static bool SolveBoxToBoxCollision(CRigidBody* rb_0, CRigidBody* rb_1, SCollisionManifold& mainfold);
	};
}