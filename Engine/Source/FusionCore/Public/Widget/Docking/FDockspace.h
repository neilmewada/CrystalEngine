#pragma once

namespace CE
{
    class FDockTabWell;
    class FDockTabItem;
    class FDockWindow;
    class FDockspaceSplitView;

    using FDockId = Uuid;

    ENUM(Flags)
    enum class FDockTypeMask : u8
    {
        None = 0,
        Major = BIT(0),
        Minor = BIT(1),
        All = Major | Minor,
    };
    ENUM_CLASS_FLAGS(FDockTypeMask);

    STRUCT()
    struct FUSIONCORE_API FDockspaceFilter
    {
        CE_STRUCT(FDockspaceFilter)
    public:

        Self& WithDockTypeMask(FDockTypeMask mask);

        Self& WithAllowedDockspaces(const Array<FDockId>& dockspaceIds);

        FIELD()
        FDockTypeMask allowedDockTypes = FDockTypeMask::None;

        FIELD()
        Array<FDockId> allowedDockspaces;

    };

    CLASS()
    class FUSIONCORE_API FDockspace : public FStyledWidget
    {
        CE_CLASS(FDockspace, FStyledWidget)
    protected:

        FDockspace();

        void Construct() override;

    public: // - Public API -

        FDockId GetDockId() const { return dockId; }

        Ref<FDockTabWell> GetDockTabWell() const { return tabWell; }

        void SetActiveTab(Ref<FDockTabItem> tabItem);

        void UpdateTabs();

        void AddDockWindow(Ref<FDockWindow> dockWindow);

        void AddDockWindow(FDockWindow& dockWindow)
        {
            AddDockWindow(&dockWindow);
        }

        bool CanDetach(Ref<FDockTabItem> dockTabItem);

        Ref<FNativeContext> DetachItem(Ref<FDockTabItem> dockTabItem);

    protected: // - Internal -

        Ref<FDockTabWell> tabWell;
        Ref<FDockspaceSplitView> container;

        Array<Ref<FDockWindow>> tabbedDockWindows;

        Ref<FDockTabItem> selectedTab;

        FIELD()
        SubClass<FWindow> detachedWindowClass;

        FDockId dockId;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(bool, AllowDocking);
        FUSION_PROPERTY(bool, AllowSplitting);

        FUSION_PROPERTY(FDockTypeMask, DockspaceType);

        FUSION_WIDGET;
        friend class FDockTabWell;
        friend class FDockspaceStyle;
    };
    
}

#include "FDockspace.rtti.h"
