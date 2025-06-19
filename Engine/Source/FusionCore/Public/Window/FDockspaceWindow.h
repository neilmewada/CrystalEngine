#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockspaceWindow : public FWindow
    {
        CE_CLASS(FDockspaceWindow, FWindow)
    protected:

        FDockspaceWindow();

        void Construct() override;

    public: // - Public API -

        Ref<FDockspace> GetDockspace() const { return dockspace; }

    protected: // - Internal -

        FStyledWidget* borderWidget = nullptr;
        FOverlayStack* rootBox = nullptr;
        FImage* maximizeIcon = nullptr;
        FImage* minimizeIcon = nullptr;
        Ref<FTitleBar> titleBar;
        FHorizontalStack* titleBarContainer = nullptr;
        FStyledWidget* projectLabelParent = nullptr;
        FLabel* projectLabel = nullptr;

        FImage* logo = nullptr;
        FWindowControlButton* minimizeButton = nullptr;
        FWindowControlButton* maximizeButton = nullptr;
        FWindowControlButton* closeButton = nullptr;

        Ref<FDockspace> dockspace;

    public: // - Fusion Properties - 

        FIELD()
        SubClass<FDockspace> dockspaceClass;

        FUSION_PROPERTY_WRAPPER(DockspaceType, dockspace);

        FUSION_WIDGET;
    };
    
}

#include "FDockspaceWindow.rtti.h"
