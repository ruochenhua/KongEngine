#pragma once
#include <unordered_map>
#include "RigidBody.h"
#include "CollisionSolver.h"

namespace Tap
{
    // manage physics body, for now only rigid body
    class CBodyManager
    {
    public:
        CBodyManager();

		//
		int AddRigidBody();
		CRigidBody* GetRigidBody(int id);		
		void RemoveRigidBody(int id);

		//
		void ProcessIntegration(double dt);
		//
		void DetectCollision();
		//
		void ClearForce();

		bool GetCollisionManifolds(std::vector<SCollisionManifold>& manifolds);
    private:
        int m_nextID;
        std::unordered_map<int, RigidBodyUPtr> m_mRigidBodyMap;  //rigid body vector
		std::unique_ptr<CCollisionSolver> m_pCollisionSolver;	//处理rigid body之间的碰撞
    };
};