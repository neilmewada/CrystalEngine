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
        for (int i = 0; i < vertEncodersBySetNumber.size(); i++)
        {
            vertEncodersBySetNumber[i] = nil;
            fragEncodersBySetNumber[i] = nil;
        }
    }

    void GraphicsPipeline::Create()
    {
        pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        
        pipelineDescriptor.label = [NSString stringWithCString:pipelineDesc.name.GetString().GetCString()];
        
        pipelineDescriptor.rasterSampleCount = pipelineDescriptor.sampleCount = pipelineDesc.multisampleState.sampleCount;
        
        pipelineDescriptor.rasterizationEnabled = YES;
        
        SetupShaderStages();
        SetupAttachments();
        SetupDepthStencilState();
        SetupVertexInput();
        
        mtlPipeline = [device->GetHandle() newRenderPipelineStateWithDescriptor:pipelineDescriptor error:nil];
    }

    void GraphicsPipeline::SetupShaderStages()
    {
        for (int i = 0; i < vertEncodersBySetNumber.size(); i++)
        {
            vertEncodersBySetNumber[i] = nil;
            fragEncodersBySetNumber[i] = nil;
        }
        
        for (const auto& shaderStageDesc : pipelineDesc.shaderStages)
        {
            NSString* entryPoint = [NSString stringWithCString:shaderStageDesc.entryPoint.GetCString()];
            
            auto shaderModule = (Metal::ShaderModule*)shaderStageDesc.shaderModule;
            
            if (shaderStageDesc.shaderModule->GetShaderStage() == ShaderStage::Vertex)
            {
                id<MTLFunction> vertFunc = [shaderModule->GetMtlLibrary() newFunctionWithName:entryPoint];
                pipelineDescriptor.vertexFunction = vertFunc;
                
                for (const auto& srgLayout : pipelineDesc.srgLayouts)
                {
                    int setNumber = (int)srgLayout.srgType;
                    
                    bool hasVertexStage = false;
                    
                    for (const auto& var : srgLayout.variables)
                    {
                        if (EnumHasFlag(var.shaderStages, RHI::ShaderStage::Vertex))
                        {
                            hasVertexStage = true;
                            break;
                        }
                    }
                    
                    if (hasVertexStage)
                        vertEncodersBySetNumber[setNumber] = [vertFunc newArgumentEncoderWithBufferIndex:setNumber];
                }
            }
            else if (shaderStageDesc.shaderModule->GetShaderStage() == ShaderStage::Fragment)
            {
                id<MTLFunction> fragFunc = [shaderModule->GetMtlLibrary() newFunctionWithName:entryPoint];
                pipelineDescriptor.fragmentFunction = fragFunc;
                
                for (const auto& srgLayout : pipelineDesc.srgLayouts)
                {
                    int setNumber = (int)srgLayout.srgType;
                    
                    bool hasFragmentStage = false;
                    
                    for (const auto& var : srgLayout.variables)
                    {
                        if (EnumHasFlag(var.shaderStages, RHI::ShaderStage::Fragment))
                        {
                            hasFragmentStage = true;
                            break;
                        }
                    }
                    
                    if (hasFragmentStage)
                        fragEncodersBySetNumber[setNumber] = [fragFunc newArgumentEncoderWithBufferIndex:setNumber];
                }
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
    
    inline MTLCompareFunction ToMtlCompareFunction(RHI::CompareOp compareOp)
    {
        switch (compareOp)
        {
            case RHI::CompareOp::Never:
                return MTLCompareFunctionNever;
            case RHI::CompareOp::Less:
                return MTLCompareFunctionLess;
            case RHI::CompareOp::Equal:
                return MTLCompareFunctionEqual;
            case RHI::CompareOp::LEqual:
                return MTLCompareFunctionLessEqual;
            case RHI::CompareOp::Greater:
                return MTLCompareFunctionGreater;
            case RHI::CompareOp::NotEqual:
                return MTLCompareFunctionNotEqual;
            case RHI::CompareOp::GEqual:
                return MTLCompareFunctionGreaterEqual;
            case RHI::CompareOp::Always:
                return MTLCompareFunctionAlways;
        }
        
        return MTLCompareFunctionLessEqual;
    }
    
    inline MTLCullMode ToMtlCullMode(RHI::CullMode cullMode)
    {
        switch (cullMode)
        {
            case RHI::CullMode::Back:
                return MTLCullModeBack;
            case RHI::CullMode::Front:
                return MTLCullModeFront;
            case RHI::CullMode::All:
                return MTLCullModeBack;
        }
        
        return MTLCullModeNone;
    }
    
    inline MTLTriangleFillMode ToMtlFillMode(RHI::FillMode fillMode)
    {
        switch (fillMode)
        {
            case RHI::FillMode::Solid:
                return MTLTriangleFillModeFill;
            case RHI::FillMode::Wireframe:
                return MTLTriangleFillModeLines;
        }
        
        return MTLTriangleFillModeFill;
    }
    
    void GraphicsPipeline::SetupRenderEncoder(id<MTLRenderCommandEncoder> encoder)
    {
        if (mtlDepthStencilState != nil)
        {
            [encoder setDepthStencilState:mtlDepthStencilState];
        }
        
        [encoder setCullMode:ToMtlCullMode(pipelineDesc.rasterState.cullMode)];
        
        [encoder setTriangleFillMode:ToMtlFillMode(pipelineDesc.rasterState.fillMode)];
    }

    void GraphicsPipeline::SetupAttachments()
    {
        if (pipelineDesc.renderPassLayout.subpasses.IsEmpty())
        {
            RHI::RenderPassSubpassLayout subpassLayout{};
            
            for (int i = 0; i < pipelineDesc.renderPassLayout.attachmentLayouts.GetSize(); i++)
            {
                const RHI::RenderPassAttachmentLayout& attachmentLayout = pipelineDesc.renderPassLayout.attachmentLayouts[i];
                
                switch (attachmentLayout.attachmentUsage)
                {
                    case RHI::ScopeAttachmentUsage::Color:
                        subpassLayout.colorAttachments.Add(i);
                        break;
                    case RHI::ScopeAttachmentUsage::DepthStencil:
                        subpassLayout.depthStencilAttachment.Add(i);
                        break;
                    case RHI::ScopeAttachmentUsage::Resolve:
                        subpassLayout.resolveAttachments.Add(i);
                        break;
                    case RHI::ScopeAttachmentUsage::SubpassInput:
                        subpassLayout.subpassInputAttachments.Add(i);
                        break;
                    default:
                        continue;
                }
            }
            
            pipelineDesc.renderPassLayout.subpasses.Add(subpassLayout);
        }
        
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
            
            if (IsStencilMtlFormat(pipelineDescriptor.depthAttachmentPixelFormat))
            {
                pipelineDescriptor.stencilAttachmentPixelFormat = ToMtlFormat(attachmentLayout.format);
            }
            else
            {
                pipelineDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
            }
        }
    }
    
    void GraphicsPipeline::SetupDepthStencilState()
    {
        if (!pipelineDesc.depthStencilState.depthState.enable)
        {
            mtlDepthStencilState = nil;
            return;
        }
        
        MTLDepthStencilDescriptor* depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
        
        depthStencilDesc.depthWriteEnabled = pipelineDesc.depthStencilState.depthState.writeEnable;
        depthStencilDesc.depthCompareFunction = ToMtlCompareFunction(pipelineDesc.depthStencilState.depthState.compareOp);
        
        mtlDepthStencilState = [device->GetHandle() newDepthStencilStateWithDescriptor:depthStencilDesc];
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

