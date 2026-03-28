#pragma once

namespace CE
{
    CLASS()
    class FUSIONCORE_API FShader : public Object
    {
        CE_CLASS(FShader, Object)
    public:

        FShader();

        static Ref<FShader> Create(Name shaderName, const ShaderReflection& reflection, RawData vertexShader, RawData fragmentShader, Ref<Object> outer = nullptr);

        RHI::PipelineState* GetDefaultPipeline();

        RPI::Shader* GetRpiShader() const { return shader; }

    protected:

        void OnBeforeDestroy() override;

        RPI::Shader* shader = nullptr;

    };
    
} // namespace CE

#include "FShader.rtti.h"