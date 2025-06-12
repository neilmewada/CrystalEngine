#pragma once

namespace CE
{
    class FDockTabWell;
    class FDockTabItem;
    class FDockWindow;

    using FDockId = Uuid;

    ENUM(Flags)
    enum class FDockTypeMask
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

    protected: // - Internal -

        Ref<FDockTabWell> tabWell;

        FDockId dockId;

    public: // - Fusion Properties - 

        FUSION_PROPERTY(bool, AllowDocking);

        FUSION_WIDGET;
    };
    
}

#include "FDockspace.rtti.h"
