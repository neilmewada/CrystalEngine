#pragma once

namespace CE
{
    STRUCT()
    struct FUSIONCORE_API FStyleSetEntry final
    {
        CE_STRUCT(FStyleSetEntry)
    public:

        FIELD()
        CE::Name styleKey;

        FIELD()
        Ref<FStyle> style;

    };

    CLASS()
    class FUSIONCORE_API FStyleSet : public Object
    {
        CE_CLASS(FStyleSet, Object)
    public:

        FStyleSet();

        Ref<FStyle> FindStyle(CE::Name name);

    protected:

        void OnAfterDeserialize() override;

    private:

        FIELD()
        Array<FStyleSetEntry> entries;

        HashMap<CE::Name, Ref<FStyle>> stylesByKey;

    };

} // namespace CE

#include "FStyleSet.rtti.h"