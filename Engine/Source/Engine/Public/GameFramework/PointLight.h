#pragma once

namespace CE
{
    CLASS()
    class ENGINE_API PointLight : public Light
    {
        CE_CLASS(PointLight, Light)
    protected:

        PointLight();
        
    public:

        Ref<PointLightComponent> GetPointLightComponent() const { return pointLightComponent; }

    private:

        FIELD()
        Ref<PointLightComponent> pointLightComponent;

    };
    
} // namespace CE

#include "PointLight.rtti.h"
