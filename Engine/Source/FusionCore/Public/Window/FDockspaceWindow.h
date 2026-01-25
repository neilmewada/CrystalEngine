#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FDockspaceWindow : public FWindow, public ApplicationMessageHandler
    {
        CE_CLASS(FDockspaceWindow, FWindow)
    protected:

        FDockspaceWindow();

        void Construct() override;

        void OnBeforeDestroy() override;

    public: // - Public API -

        Ref<FDockspace> GetDockspace() const { return dockspace; }

    protected: // - Internal -

        void OnWindowMaximized(PlatformWindow* window) override;
        void OnWindowRestored(PlatformWindow* window) override;
		void OnWindowMinimized(PlatformWindow* window) override;

        virtual void OnClickClose() {}

        FStyledWidget* borderWidget = nullptr;
        //FOverlayStack* rootBox = nullptr;
        //FImage* maximizeIcon = nullptr;
        //FImage* minimizeIcon = nullptr;
        //Ref<FTitleBar> titleBar;
        //FHorizontalStack* titleBarContainer = nullptr;
        //FStyledWidget* projectLabelParent = nullptr;
        //FLabel* projectLabel = nullptr;

        //FImage* logo = nullptr;
        FWindowControlButton* minimizeButton = nullptr;
        FWindowControlButton* maximizeButton = nullptr;
        FWindowControlButton* closeButton = nullptr;
        Ref<FHorizontalStack> controlContainer;

        Ref<FDockspace> dockspace;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(SubClass<FDockspace>, DockspaceClass);

        FUSION_PROPERTY_WRAPPER(DockspaceType, dockspace);

        FUSION_WIDGET;
    };
    
}

#include "FDockspaceWindow.rtti.h"
