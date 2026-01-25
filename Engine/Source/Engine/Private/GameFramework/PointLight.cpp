#include "Engine.h"

namespace CE
{

    PointLight::PointLight()
    {
        pointLightComponent = CreateDefaultSubobject<PointLightComponent>("PointLightComponent");
        lightComponent = pointLightComponent.Get();
        SetRootComponent(pointLightComponent.Get());
    }
    
} // namespace CE

