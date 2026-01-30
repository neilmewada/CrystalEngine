#pragma once

namespace CE::RHI
{
    struct GraphicsPipelineVariant
    {
        RHI::MultisampleState sampleState{};
        Array<RHI::Format> colorFormats{};
        RHI::Format depthStencilFormat = RHI::Format::Undefined;

        SIZE_T GetHash() const
        {
            SIZE_T hash = sampleState.GetHash();
            for (int i = 0; i < colorFormats.GetSize(); i++)
            {
                CombineHash(hash, colorFormats[i]);
            }
            CombineHash(hash, depthStencilFormat);
            return hash;
        }

        bool operator==(const GraphicsPipelineVariant& rhs) const
        {
            return GetHash() == rhs.GetHash();
        }

        bool operator!=(const GraphicsPipelineVariant& rhs) const
        {
            return !operator==(rhs);
        }
    };

    class CORERHI_API GraphicsPipelineCollection
    {
    public:

        GraphicsPipelineCollection(const RHI::GraphicsPipelineDescriptor& desc);

        ~GraphicsPipelineCollection();

        RHI::PipelineState* GetPipeline(const GraphicsPipelineVariant& variant);

        inline RHI::PipelineState* GetPipeline() const { return defaultPipeline; }

        inline const GraphicsPipelineVariant& GetDefaultVariant() const { return defaultVariant; }

    private:

        RHI::GraphicsPipelineDescriptor desc{};

        GraphicsPipelineVariant defaultVariant{};
        RHI::PipelineState* defaultPipeline = nullptr;

        SharedMutex mutex{};
        HashMap<GraphicsPipelineVariant, RHI::PipelineState*> pipelineCollection{};
        Array<RHI::RenderPass*> renderPasses{};
    };
    
} // namespace CE::RPI
