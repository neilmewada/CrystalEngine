#pragma once

namespace CE
{
    CLASS()
    class FUSION_API FGridViewModel : public Object
    {
        CE_CLASS(FGridViewModel, Object)
    protected:

        FGridViewModel();
        
    public:

        virtual ~FGridViewModel();

    };
    
} // namespace CE

#include "FGridViewModel.rtti.h"
