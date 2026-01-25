#pragma once

namespace CE
{
    ENUM()
    enum class PhysicsCombineMode
    {
	    Average,
        Minimum,
        Maximum,
        Multiply
    };
    ENUM_CLASS(PhysicsCombineMode);

    CLASS(Config = Engine)
    class COREPHYSICS_API PhysicsMaterial : public DataAsset
    {
        CE_CLASS(PhysicsMaterial, DataAsset)
    protected:

        PhysicsMaterial();
        
    public:

        FIELD(EditAnywhere, Config)
        float staticFriction = 0.5f;

        FIELD(EditAnywhere, Config)
        float dynamicFriction = 0.5f;

        FIELD(EditAnywhere, Config)
        float bounciness = 0;

        FIELD(EditAnywhere, Config)
        PhysicsCombineMode frictionCombine = PhysicsCombineMode::Average;

        FIELD(EditAnywhere, Config)
        PhysicsCombineMode bouncinessCombine = PhysicsCombineMode::Average;

    };
    
} // namespace CE

#include "PhysicsMaterial.rtti.h"
