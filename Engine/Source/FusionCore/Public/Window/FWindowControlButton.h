#pragma once

namespace CE
{
    ENUM()
    enum class FWindowControlType
    {
        Undefined = 0,
        Close,
        Maximize,
        Minimize
    };
    ENUM_CLASS(FWindowControlType);

    CLASS()
    class FUSIONCORE_API FWindowControlButton : public FButton
    {
        CE_CLASS(FWindowControlButton, FButton)
    protected:

        FWindowControlButton();

        void Construct() override;

        void HandleEvent(FEvent* event) override;

    public: // - Public API -

        void SetMaximizedState(bool isMaximized);

        const auto& GetControlGroup() const { return controlGroup; }

        void SetControlGroup(const Array<WeakRef<FWindowControlButton>>& controlGroup) { this->controlGroup = controlGroup; }

    protected: // - Internal -

        Ref<FImage> icon;
        FWindowControlType controlType = FWindowControlType::Undefined;

        Array<WeakRef<FWindowControlButton>> controlGroup;

    public: // - Fusion Properties - 

        FUSION_PROPERTY_WRAPPER2(Background, icon, Icon);
        FUSION_PROPERTY_WRAPPER2(Visible, icon, IconVisible);

        Self& ControlType(FWindowControlType type);

        FWindowControlType ControlType() const { return controlType; }

        FUSION_WIDGET;
    };
    
}

#include "FWindowControlButton.rtti.h"
