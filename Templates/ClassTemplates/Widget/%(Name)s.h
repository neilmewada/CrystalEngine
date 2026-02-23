#pragma once

namespace %(NameSpace)s
{
    CLASS()
    class %(Module_API)s %(Name)s : public FWidget
    {
        CE_CLASS(%(Name)s, FWidget)
    protected:

        %(Name)s();

        void Construct() override;

    public: // - Public API -


    protected: // - Internal -


    public: // - Fusion Properties - 


        FUSION_WIDGET;
    };
    
}

#include "%(Name)s.rtti.h"
