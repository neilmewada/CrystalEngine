#pragma once

namespace CE
{
    class FStyle;

    CLASS()
    class FUSIONCORE_API FStyleSet : public Object
    {
        CE_CLASS(FStyleSet, Object)
    public:

        FStyleSet();

        Ref<FStyle> FindStyle(CE::Name name);

        void SetParent(Ref<FStyleSet> parent);

        void RegisterStyle(CE::Name name, Ref<FStyle> style);
        void DeregisterStyle(CE::Name name);

        void InitDefault();

    protected:

        

    private:

        FIELD()
        WeakRef<FStyleSet> parent;

        HashMap<CE::Name, Ref<FStyle>> stylesByKey;
    };

} // namespace CE

#include "FStyleSet.rtti.h"