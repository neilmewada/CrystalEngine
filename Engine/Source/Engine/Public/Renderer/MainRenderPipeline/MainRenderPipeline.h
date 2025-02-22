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

        void ConstructPipeline() override;
        
    };
    
} // namespace CE

#include "MainRenderPipeline.rtti.h"