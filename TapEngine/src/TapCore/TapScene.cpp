//
//  TapScene.cpp
//  TapCore
//
//  Created by Ruochen Hua on 2018/12/23.
//  Copyright © 2018 Ruochen Hua. All rights reserved.
//
#include "stdafx.h"
#include "TapScene.hpp"
namespace Tap
{
    int CTapScene::InitScene()
    {
		printf("init tap scene\n");
		m_bodyMgr.reset(new CBodyManager);
        
        return 0;
    }

	int CTapScene::Step(double dt)
	{
		if (m_bodyMgr)
		{
			m_bodyMgr->ProcessIntegration(dt);
			m_bodyMgr->DetectCollision();
			m_bodyMgr->ClearForce();
		}
		return 0;
	}

	int CTapScene::AddRigidBody()
	{
		if (m_bodyMgr)
		{
			return m_bodyMgr->AddRigidBody();
		}
		
		return -1;
	}

	CRigidBody* CTapScene::GetRigidBody(int id)
	{
		if (m_bodyMgr)
		{
			return m_bodyMgr->GetRigidBody(id);
		}

		return nullptr;
	}

	bool CTapScene::GetCollisionManifolds(std::vector<SCollisionManifold>& manifolds)
	{
		if (m_bodyMgr)
		{
			return m_bodyMgr->GetCollisionManifolds(manifolds);
		}
		return false;
	}
};