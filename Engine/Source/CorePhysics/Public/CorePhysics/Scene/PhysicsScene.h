#pragma once

namespace CE
{
    CLASS()
    class COREPHYSICS_API PhysicsScene : public Object
    {
        CE_CLASS(PhysicsScene, Object)
    protected:

        PhysicsScene();
        
    public:

        virtual ~PhysicsScene();

    };
    
} // namespace CE

#include "PhysicsScene.rtti.h"
