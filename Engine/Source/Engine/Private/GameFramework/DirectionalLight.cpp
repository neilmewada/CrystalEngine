#include "Engine.h"

namespace CE
{

    DirectionalLight::DirectionalLight()
    {
        directionalLightComponent = CreateDefaultSubobject<DirectionalLightComponent>("DirectionalLightComponent");
        lightComponent = directionalLightComponent;
        SetRootComponent(directionalLightComponent);
    }
    
} // namespace CE
