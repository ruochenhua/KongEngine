//
//  TapCorePriv.hpp
//  TapCore
//
//  Created by Ruochen Hua on 2018/12/23.
//  Copyright © 2018 Ruochen Hua. All rights reserved.
//

/* The classes below are not exported */
#pragma GCC visibility push(hidden)
namespace Tap
{
    class TapCorePriv
    {
        public:
        void HelloWorldPriv(const char *);
    };
};
#pragma GCC visibility pop
