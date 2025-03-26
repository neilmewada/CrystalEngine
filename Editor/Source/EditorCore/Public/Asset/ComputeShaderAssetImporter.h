#pragma once

namespace CE
{
    CLASS()
    class EDITORCORE_API ComputeShaderAssetImporter : public Object
    {
        CE_CLASS(ComputeShaderAssetImporter, Object)
    protected:

        ComputeShaderAssetImporter();
        
    public:

        virtual ~ComputeShaderAssetImporter();

    };
    
} // namespace CE

#include "ComputeShaderAssetImporter.rtti.h"
