#pragma once

namespace CE
{
    CLASS()
    class ENGINE_API DirectionalLightComponent : public LightComponent
    {
        CE_CLASS(DirectionalLightComponent, LightComponent)
    public:

        DirectionalLightComponent();

        ~DirectionalLightComponent();

        RPI::View* GetRpiView() const { return rpiView; }

    protected:

        void OnEnabled() override;

        void OnDisabled() override;

        void Tick(f32 delta) override;

        // - Properties -

        FIELD(EditAnywhere, Category = "Light")
        f32 shadowDistance = 10;

        // - Internal -

        RPI::DirectionalLightHandle lightHandle{};
        RPI::ViewPtr rpiView = nullptr;
    };
    
} // namespace CE

#include "DirectionalLightComponent.rtti.h"