#pragma once

namespace CE
{
    CLASS()
    class ENGINE_API MainRenderPipeline : public CE::RenderPipeline
    {
        CE_CLASS(MainRenderPipeline, CE::RenderPipeline)
    public:

        MainRenderPipeline();
        virtual ~MainRenderPipeline();

    protected:

        void OnAfterConstruct() override;

        void OnBeforeDestroy() override;

        void ConstructPipeline() override;

        using uint = u32;

        struct alignas(4) CullingParams
        {
            uint     tileSizeX;                 // e.g., 16
            uint     tileSizeY;                 // e.g., 16
            uint     numLights;                 // _LocalLights count
            uint     lightIndexPoolCapacity;    // max uints in _LightIndexPool

            // NDC depth range for your API (D3D: 0..1 ; Vulkan/GL: -1..1)
            float    ndcNear;                   // D3D: 0,  Vulkan/GL: -1
            float    ndcFar;                    // D3D: 1,  Vulkan/GL:  1
        };

        StaticArray<RHI::Buffer*, RHI::Limits::MaxSwapChainImageCount> cullingParamsBuffer;
        Ref<CE::ComputeShader> tileCullingComputeShader;
    };
    
} // namespace CE

#include "MainRenderPipeline.rtti.h"