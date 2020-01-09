#include "stdafx.h"
#include "TapBodyManager.h"
#include "Intergration.h"

namespace Tap
{
    CBodyManager::CBodyManager()
    : m_nextID(0)
    {
		m_mRigidBodyMap.clear();
		m_pCollisionSolver.reset(new CCollisionSolver);
    }

	int CBodyManager::AddRigidBody()
	{
		RigidBodyUPtr tmp_bdy;
		tmp_bdy.reset(new CRigidBody(m_nextID));
		m_mRigidBodyMap.emplace(m_nextID++, std::move(tmp_bdy));

		return m_nextID - 1;
	}

	CRigidBody* CBodyManager::GetRigidBody(int id)
	{
		auto itr = m_mRigidBodyMap.find(id);
		if (itr != m_mRigidBodyMap.end())
		{
			return itr->second.get();
		}

		return nullptr;
	}
	
	void CBodyManager::RemoveRigidBody(int id)
	{
		m_mRigidBodyMap.erase(id);		
	}

	void CBodyManager::ProcessIntegration(double dt)
	{
		// 简单对所有map里面的进行迭代
		for (auto& rb_iter : m_mRigidBodyMap)
		{
			//simple eular
			CIntegration::EulerIntegration(rb_iter.second.get(), dt);
		}
	}

	void CBodyManager::DetectCollision()
	{
		if (m_pCollisionSolver)
		{
			m_pCollisionSolver->Detect(m_mRigidBodyMap);
			m_pCollisionSolver->Solve();
		}
	}

	void CBodyManager::ClearForce()
	{
		for (auto& rb : m_mRigidBodyMap)
		{
			rb.second->SetForce(glm::vec3(0.0f));
		}
	}

	bool CBodyManager::GetCollisionManifolds(std::vector<SCollisionManifold>& manifolds)
	{
		if (m_pCollisionSolver)
		{
			manifolds = m_pCollisionSolver->GetCollisionManifold();
			return true;
		}
		return false;
	}
};