#pragma once

namespace CE
{
    CLASS()
    class ENGINE_API DirectionalLight : public Light
    {
        CE_CLASS(DirectionalLight, Light)
    public:

        DirectionalLight();

        DirectionalLightComponent* GetDirectionalLightComponent() const { return directionalLightComponent; }

    protected:

        FIELD()
        DirectionalLightComponent* directionalLightComponent = nullptr;

    };
    
} // namespace CE

#include "DirectionalLight.rtti.h"