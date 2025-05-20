#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FPopup : public FStyledWidget
    {
        CE_CLASS(FPopup, FStyledWidget)
    public:

        FPopup();

        // - Public API -

        void SetInitialSize(Vec2 size) { initialSize = size; }

        void ClosePopup();

        virtual void OnPopupClosed();

        bool IsShown() const { return isShown; }

        void SetContextWidget(Ref<FWidget> widget);
        Ref<FWidget> GetContextWidget();

    protected:

        void HandleEvent(FEvent* event) override;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(bool, BlockInteraction);
        FUSION_PROPERTY(bool, AutoClose);

        FUSION_EVENT(ScriptEvent<void(FPopup*)>, OnClosed);

    private:

        Vec2 initialPos;
        Vec2 initialSize;
        Vec2 controlSize;
        bool isNativePopup = false;
        bool isShown = false;
        bool positionFound = false;

        // The widget that created this popup. For ex: a button that opened a popup menu!
        WeakRef<FWidget> contextWidget;

        FUSION_WIDGET;
    };
    
}


#include "FPopup.rtti.h"