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
		//进行碰撞检测
		bool Detect(std::unordered_map<int, RigidBodyUPtr>& rb_map);
		//解决碰撞
		bool Solve();
		//获取碰撞记录
		const std::vector<SCollisionManifold>& GetCollisionManifold() const;

	private:
		std::vector<SCollisionManifold> m_vCollisionManifold;

	private:
		//两个刚体的碰撞检测
		bool DetectRigidBodyCollision(CRigidBody* rb_0, CRigidBody* rb_1);

		//施加冲量
		//lower part是冲量计算方程式的分母，因为两个刚体这个量是一样的，所以只计算一次
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