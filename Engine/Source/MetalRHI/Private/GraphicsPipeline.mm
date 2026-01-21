#include "MetalRHIPrivate.h"

#include "TextureFormats.inl"

namespace CE::Metal
{
    
    GraphicsPipeline::GraphicsPipeline(Device* device, const RHI::GraphicsPipelineDescriptor& desc)
        : Pipeline(device, desc), pipelineDesc(desc)
    {
        Create();
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        
    }

    void GraphicsPipeline::Create()
    {
        pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        
        pipelineDescriptor.label = [NSString stringWithCString:pipelineDesc.name.GetString().GetCString()];
        
        pipelineDescriptor.rasterSampleCount = pipelineDescriptor.sampleCount = pipelineDesc.multisampleState.sampleCount;
        
        pipelineDescriptor.rasterizationEnabled = YES;
        
        SetupShaderStages();
        SetupColorAttachments();
        SetupVertexInput();
        
        mtlPipeline = [device->GetHandle() newRenderPipelineStateWithDescriptor:pipelineDescriptor error:nil];
    }

    void GraphicsPipeline::SetupShaderStages()
    {
        for (const auto& shaderStageDesc : pipelineDesc.shaderStages)
        {
            NSString* entryPoint = [NSString stringWithCString:shaderStageDesc.entryPoint.GetCString()];
            
            auto shaderModule = (Metal::ShaderModule*)shaderStageDesc.shaderModule;
            
            if (shaderStageDesc.shaderModule->GetShaderStage() == ShaderStage::Vertex)
            {
                pipelineDescriptor.vertexFunction = [shaderModule->GetMtlLibrary() newFunctionWithName:entryPoint];
            }
            else if (shaderStageDesc.shaderModule->GetShaderStage() == ShaderStage::Fragment)
            {
                pipelineDescriptor.fragmentFunction = [shaderModule->GetMtlLibrary() newFunctionWithName:entryPoint];
            }
        }
    }

    inline MTLBlendOperation RHIBlendOpToMtl(RHI::BlendOp blendOp)
    {
        switch (blendOp)
        {
            case RHI::BlendOp::Add:
                return MTLBlendOperationAdd;
            case RHI::BlendOp::Subtract:
                return MTLBlendOperationSubtract;
            case RHI::BlendOp::ReverseSubtract:
                return MTLBlendOperationReverseSubtract;
            case RHI::BlendOp::Min:
                return MTLBlendOperationMin;
            case RHI::BlendOp::Max:
                return MTLBlendOperationMax;
        }
        
        return MTLBlendOperationAdd;
    }

    inline MTLBlendFactor RHIBlendFactorToMtl(RHI::BlendFactor blendFactor)
    {
        switch (blendFactor)
        {
            case RHI::BlendFactor::Zero:
                return MTLBlendFactorZero;
            case RHI::BlendFactor::One:
                return MTLBlendFactorOne;
            case RHI::BlendFactor::SrcColor:
                return MTLBlendFactorSourceColor;
            case RHI::BlendFactor::OneMinusSrcColor:
                return MTLBlendFactorOneMinusSourceColor;
            case RHI::BlendFactor::DstColor:
                return MTLBlendFactorDestinationColor;
            case RHI::BlendFactor::OneMinusDstColor:
                return MTLBlendFactorOneMinusDestinationColor;
            case RHI::BlendFactor::SrcAlpha:
                return MTLBlendFactorSourceAlpha;
            case RHI::BlendFactor::OneMinusSrcAlpha:
                return MTLBlendFactorOneMinusSourceAlpha;
            case RHI::BlendFactor::DstAlpha:
                return MTLBlendFactorDestinationAlpha;
            case RHI::BlendFactor::OneMinusDstAlpha:
                return MTLBlendFactorOneMinusDestinationAlpha;
        }
        
        return MTLBlendFactorZero;
    }

    inline MTLRenderPipelineColorAttachmentDescriptor* GetMtlColorAttachmentDescriptor(const RHI::ColorBlendState& colorBlend)
    {
        MTLRenderPipelineColorAttachmentDescriptor* desc = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];
        
        desc.blendingEnabled = colorBlend.blendEnable;
        
        desc.rgbBlendOperation = RHIBlendOpToMtl(colorBlend.colorBlendOp);
        desc.alphaBlendOperation = RHIBlendOpToMtl(colorBlend.alphaBlendOp);
        
        desc.sourceRGBBlendFactor = RHIBlendFactorToMtl(colorBlend.srcColorBlend);
        desc.sourceAlphaBlendFactor = RHIBlendFactorToMtl(colorBlend.srcAlphaBlend);
        
        desc.destinationRGBBlendFactor = RHIBlendFactorToMtl(colorBlend.dstColorBlend);
        desc.destinationAlphaBlendFactor = RHIBlendFactorToMtl(colorBlend.dstAlphaBlend);
        
        desc.writeMask = MTLColorWriteMaskNone;
        
        if (EnumHasFlag(colorBlend.componentMask, RHI::ColorComponentMask::A))
        {
            desc.writeMask |= MTLColorWriteMaskAlpha;
        }
        if (EnumHasFlag(colorBlend.componentMask, RHI::ColorComponentMask::R))
        {
            desc.writeMask |= MTLColorWriteMaskRed;
        }
        if (EnumHasFlag(colorBlend.componentMask, RHI::ColorComponentMask::G))
        {
            desc.writeMask |= MTLColorWriteMaskGreen;
        }
        if (EnumHasFlag(colorBlend.componentMask, RHI::ColorComponentMask::B))
        {
            desc.writeMask |= MTLColorWriteMaskBlue;
        }
        
        return desc;
    }

    void GraphicsPipeline::SetupColorAttachments()
    {
        const RenderPassSubpassLayout& subpass = pipelineDesc.renderPassLayout.subpasses[pipelineDesc.subpass];
        
        for (int i = 0; i < subpass.colorAttachments.GetSize(); i++)
        {
            u32 attachmentIdx = subpass.colorAttachments[i];
            const RenderPassAttachmentLayout& attachmentLayout = pipelineDesc.renderPassLayout.attachmentLayouts[attachmentIdx];
            
            if (i < pipelineDesc.blendState.colorBlends.GetSize())
            {
                pipelineDescriptor.colorAttachments[i] = GetMtlColorAttachmentDescriptor(pipelineDesc.blendState.colorBlends[i]);
            }
            else if (pipelineDesc.blendState.colorBlends.NotEmpty())
            {
                pipelineDescriptor.colorAttachments[i] = GetMtlColorAttachmentDescriptor(pipelineDesc.blendState.colorBlends.GetLast());
            }
            else
            {
                pipelineDescriptor.colorAttachments[i] = GetMtlColorAttachmentDescriptor(RHI::ColorBlendState());
            }
            
            pipelineDescriptor.colorAttachments[i].pixelFormat = ToMtlFormat(attachmentLayout.format);
        }
        
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
        pipelineDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
        
        if (!subpass.depthStencilAttachment.IsEmpty())
        {
            u32 attachmentIdx = subpass.depthStencilAttachment[0];
            const RenderPassAttachmentLayout& attachmentLayout = pipelineDesc.renderPassLayout.attachmentLayouts[attachmentIdx];
            
            pipelineDescriptor.depthAttachmentPixelFormat = ToMtlFormat(attachmentLayout.format);
            pipelineDescriptor.stencilAttachmentPixelFormat = ToMtlFormat(attachmentLayout.format);
        }
    }

    void GraphicsPipeline::SetupVertexInput()
    {
        pipelineDescriptor.vertexDescriptor = [[MTLVertexDescriptor alloc] init];
        
        for (int i = 0; i < pipelineDesc.vertexInputSlots.GetSize(); i++)
        {
            int inputSlot = pipelineDesc.vertexInputSlots[i].inputSlot;
            int bufferIndex = (int)RHI::SRGType::COUNT + inputSlot;
            
            pipelineDescriptor.vertexDescriptor.layouts[bufferIndex].stride = pipelineDesc.vertexInputSlots[i].stride;
            pipelineDescriptor.vertexDescriptor.layouts[bufferIndex].stepRate = 1;
            
            switch (pipelineDesc.vertexInputSlots[i].inputRate)
            {
                case RHI::VertexInputRate::PerVertex:
                    pipelineDescriptor.vertexDescriptor.layouts[bufferIndex].stepFunction = MTLVertexStepFunctionPerVertex;
                    break;
                case RHI::VertexInputRate::PerInstance:
                    pipelineDescriptor.vertexDescriptor.layouts[bufferIndex].stepFunction = MTLVertexStepFunctionPerInstance;
                    break;
            }
        }
        
        for (int i = 0; i < pipelineDesc.vertexAttributes.GetSize(); i++)
        {
            int location = pipelineDesc.vertexAttributes[i].location;
            
            pipelineDescriptor.vertexDescriptor.attributes[location].offset = pipelineDesc.vertexAttributes[i].offset;
            pipelineDescriptor.vertexDescriptor.attributes[location].bufferIndex = (int)RHI::SRGType::COUNT + pipelineDesc.vertexAttributes[i].inputSlot;
            
            switch (pipelineDesc.vertexAttributes[i].dataType)
            {
                case RHI::VertexAttributeDataType::Undefined:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatInvalid;
                    break;
                case RHI::VertexAttributeDataType::Float:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatFloat;
                    break;
                case RHI::VertexAttributeDataType::Float2:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatFloat2;
                    break;
                case RHI::VertexAttributeDataType::Float3:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatFloat3;
                    break;
                case RHI::VertexAttributeDataType::Float4:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatFloat4;
                    break;
                case RHI::VertexAttributeDataType::Int:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatInt;
                    break;
                case RHI::VertexAttributeDataType::Int2:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatInt2;
                    break;
                case RHI::VertexAttributeDataType::Int3:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatInt3;
                    break;
                case RHI::VertexAttributeDataType::Int4:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatInt4;
                    break;
                case RHI::VertexAttributeDataType::UInt:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatUInt;
                    break;
                case RHI::VertexAttributeDataType::UInt2:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatUInt2;
                    break;
                case RHI::VertexAttributeDataType::UInt3:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatUInt3;
                    break;
                case RHI::VertexAttributeDataType::UInt4:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatUInt4;
                    break;
                case RHI::VertexAttributeDataType::Char4:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatChar4;
                    break;
                case RHI::VertexAttributeDataType::UChar4:
                    pipelineDescriptor.vertexDescriptor.attributes[location].format = MTLVertexFormatUChar4;
                    break;
            }
        }
    }


} // namespace CE::Metal

