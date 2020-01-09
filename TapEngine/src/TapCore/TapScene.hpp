//
//  TapScene.hpp
//  TapCore
//
//  Created by Ruochen Hua on 2018/12/23.
//  Copyright © 2018 Ruochen Hua. All rights reserved.
//

#ifndef TapScene_hpp
#define TapScene_hpp

#include <stdio.h>
#include <memory>
#include "TapBodyManager.h"
#include "CollisionHelper.h"

namespace Tap
{
    class CTapScene
    {
    public:
        int InitScene();
        
        int Step(double dt);
        
		//todo: 暂时用这个别扭一点的方法,先add,再用id去get
        int AddRigidBody();   
		CRigidBody* GetRigidBody(int id);

		bool GetCollisionManifolds(std::vector<SCollisionManifold>& manifolds);
    private:
		std::unique_ptr<CBodyManager> m_bodyMgr;
    };
};

#endif /* TapScene_hpp */
