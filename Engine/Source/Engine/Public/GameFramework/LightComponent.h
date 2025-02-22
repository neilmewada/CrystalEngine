#pragma once

namespace CE
{
    CLASS(Abstract)
    class ENGINE_API LightComponent : public SceneComponent
    {
        CE_CLASS(LightComponent, SceneComponent)
    public:

        LightComponent();

        virtual ~LightComponent();

        const Color& GetLightColor() const { return lightColor; }

        void SetLightColor(const Color& value) { lightColor = value; }

        f32 GetIntensity() const { return intensity; }

        void SetIntensity(f32 value) { intensity = value; }

    protected:

        FIELD(EditAnywhere, Category = "Light")
        Color lightColor{};

        FIELD(EditAnywhere, Category = "Light")
        f32 intensity = 1.0f;

        FIELD(EditAnywhere, Category = "Light")
        f32 temperature = 0.0f;

        FIELD(EditAnywhere, Category = "Light")
        b8 enableShadows = true;

    private:

    };
    
} // namespace CE

#include "LightComponent.rtti.h"