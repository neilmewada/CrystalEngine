#pragma once

#if PAL_TRAIT_BUILD_EDITOR
namespace CE::Editor
{
    class ComputeShaderAssetImportJob;
}
#endif

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

    protected:

        FIELD()
        Array<Name> kernelNames;

        FIELD()
        Array<BinaryBlob> kernels;

        FIELD()
        ShaderReflection reflection;

#if PAL_TRAIT_BUILD_EDITOR
        friend class CE::Editor::ComputeShaderAssetImportJob;
#endif
    };
    
} // namespace CE

#include "ComputeShader.rtti.h"
