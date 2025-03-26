#pragma once

namespace CE
{
    CLASS()
    class ENGINE_API ComputeShader : public Object
    {
        CE_CLASS(ComputeShader, Object)
    protected:

        ComputeShader();
        
    public:

        virtual ~ComputeShader();

    };
    
} // namespace CE

#include "ComputeShader.rtti.h"
