#pragma once

namespace CE
{
    class FDockTabWell;
    class FDockTabItem;
    class FDockWindow;
    class FDockspaceWindow;
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

        void HandleEvent(FEvent* event) override;

    public: // - Public API -

        FDockId GetDockId() const { return dockId; }

        void UpdateTabs();

        void AddDockWindow(Ref<FDockWindow> dockWindow);

        void AddDockWindow(FDockWindow& dockWindow)
        {
            AddDockWindow(&dockWindow);
        }

        bool CanBeDocked(Ref<FDockWindow> dockWindow);

        bool CanDetach(Ref<FDockTabItem> dockTabItem);

        void SetTabWellWindowHitTest(bool set);

        Ref<FDockspaceSplitView> GetRootSplit() const { return container; }

        void IterateTabWellsRecursively(const Delegate<void(FDockTabWell& tabWell)>& pred);

        Ref<FStackBox> GetRootTabWellParent();

        Ref<FDockTabWell> GetRootTabWell();

    protected: // - Internal -

        Ref<FDockspaceSplitView> container;

        WeakRef<FDockTabItem> curDraggedItem;

        FIELD()
        Vec2i originalWindowSize;

        FDockId dockId;

        FIELD()
        SubClass<FDockspaceWindow> detachedDockspaceWindowClass;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(bool, AllowDocking);
        FUSION_PROPERTY(bool, AllowSplitting);
        FUSION_PROPERTY(bool, DestroyWhenEmpty);

        Self& TabWellOverlayWidget(FWidget& widget);

        FUSION_PROPERTY(FDockTypeMask, DockspaceType);

        FUSION_PROPERTY(Delegate<FDockspace&()>, OnCreateDockspace);
        FUSION_PROPERTY(Delegate<void(Ref<FWindow>, Ref<FDockTabItem>)>, OnWindowSetup);

        FUSION_WIDGET;
        friend class FDockTabWell;
        friend class FDockspaceStyle;
        friend class FDockTabItem;
        friend class FDockspaceSplitView;
    };
    
}

#include "FDockspace.rtti.h"
