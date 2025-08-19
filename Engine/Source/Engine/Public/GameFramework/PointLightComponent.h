#pragma once

namespace CE
{
    CLASS()
    class ENGINE_API PointLightComponent : public LocalLightComponent
    {
        CE_CLASS(PointLightComponent, LocalLightComponent)
    protected:

        PointLightComponent();

        void OnBeginDestroy() override;

        void Tick(f32 delta) override;

        void OnFieldChanged(const Name& fieldName) override;
        void OnFieldEdited(const Name& fieldName) override;

        void OnEnabled() override;

        void OnDisabled() override;

    public:

    private:

        FIELD(EditAnywhere, Category = "Light")
        f32 range = 2;

    public:

        CE_PROPERTY(Range, range);

    };
    
} // namespace CE

#include "PointLightComponent.rtti.h"
