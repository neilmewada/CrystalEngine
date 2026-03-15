#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FShader : public Object
    {
        CE_CLASS(FShader, Object)
    public:

        FShader();

        static Ref<FShader> Create(RawData vertexShader, RawData fragmentShader, Ref<Object> outer = nullptr);

    protected:

        void OnBeforeDestroy() override;

        RPI::Shader* shader = nullptr;

    };
    
} // namespace CE

#include "FShader.rtti.h"