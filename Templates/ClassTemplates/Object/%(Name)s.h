#pragma once

namespace CE
{
    CLASS()
    class %(Module_API)s %(Name)s : public Object
    {
        CE_CLASS(%(Name)s, Object)
    protected:

        %(Name)s();
        
    public:

    };
    
} // namespace CE

#include "%(Name)s.rtti.h"
