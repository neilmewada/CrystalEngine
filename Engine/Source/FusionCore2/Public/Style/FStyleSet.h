#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FStyleSet : public Object
    {
        CE_CLASS(FStyleSet, Object)
    public:

        FStyleSet();

        Ref<FStyle> FindStyle(CE::Name name);

    private:

        HashMap<CE::Name, Ref<FStyle>> stylesByKey;

    };

} // namespace CE

#include "FStyleSet.rtti.h"