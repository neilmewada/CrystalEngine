#pragma once

namespace CE
{

    CLASS()
    class ENGINE_API ComputeShader : public Asset
    {
        CE_CLASS(ComputeShader, Asset)
    protected:

        ComputeShader();
        
    public:

        virtual ~ComputeShader();

        RPI::ComputeShader* GetRpiShader(int kernelIndex);

        int GetNumOfKernels() const { return Math::Min(kernelNames.GetSize(), kernels.GetSize()); }

        CE::Name GetKernelName(int kernelIndex) const;

        int FindKernelIndex(const CE::Name& kernelName) const;

        const ShaderReflection& GetReflection() const { return reflection; }

    protected:

        FIELD()
        Array<Name> kernelNames;

        FIELD()
        Array<BinaryBlob> kernels;

        FIELD()
        ShaderReflection reflection;

        HashMap<int, RPI::ComputeShader*> computeShaders;

#if PAL_TRAIT_BUILD_EDITOR
        friend class CE::Editor::ComputeShaderAssetImportJob;
#endif
    };
    
} // namespace CE

#include "ComputeShader.rtti.h"
