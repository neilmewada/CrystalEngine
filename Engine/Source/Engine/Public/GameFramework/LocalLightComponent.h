#pragma once

namespace CE
{
    CLASS(Abstract)
    class ENGINE_API LocalLightComponent : public LightComponent
    {
        CE_CLASS(LocalLightComponent, LightComponent)
    protected:

        LocalLightComponent();
        
    public:


    protected:

        RPI::LocalLightHandle lightHandle{};

    };
    
} // namespace CE

#include "LocalLightComponent.rtti.h"
