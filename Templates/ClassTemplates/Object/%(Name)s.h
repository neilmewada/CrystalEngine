#pragma once

namespace %(NameSpace)s
{
    CLASS()
    class %(Module_API)s %(Name)s : public %(SuperClass)s
    {
        CE_CLASS(%(Name)s, %(SuperClass)s)
    protected:

        %(Name)s();
        
    public:

    };
    
} // namespace CE

#include "%(Name)s.rtti.h"
