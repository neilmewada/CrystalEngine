#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
    static VkBlendOp RHIBlendOpToVk(RHI::BlendOp blendOp)
    {
        switch (blendOp)
        {
        case RHI::BlendOp::Add:
            return VK_BLEND_OP_ADD;
        case RHI::BlendOp::Subtract:
            return VK_BLEND_OP_SUBTRACT;
        case RHI::BlendOp::ReverseSubtract:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case RHI::BlendOp::Min:
            return VK_BLEND_OP_MIN;
        case RHI::BlendOp::Max:
            return VK_BLEND_OP_MAX;
        }
        return {};
    }

    static VkBlendFactor RHIBlendFactorToVk(RHI::BlendFactor blendFactor)
    {
        switch (blendFactor)
        {
        case RHI::BlendFactor::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case RHI::BlendFactor::One:
            return VK_BLEND_FACTOR_ONE;
        case RHI::BlendFactor::SrcColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case RHI::BlendFactor::OneMinusSrcColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;

        case RHI::BlendFactor::DstColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case RHI::BlendFactor::OneMinusDstColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;

        case RHI::BlendFactor::SrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case RHI::BlendFactor::OneMinusSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

        case RHI::BlendFactor::DstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case RHI::BlendFactor::OneMinusDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        }

        return {};
    }

    static VkCompareOp RHICompareOpToVk(RHI::CompareOp compareOp)
    {
        switch (compareOp)
        {
        case RHI::CompareOp::Never:
            return VK_COMPARE_OP_NEVER;
        case RHI::CompareOp::Less:
            return VK_COMPARE_OP_LESS;
        case RHI::CompareOp::Equal:
            return VK_COMPARE_OP_EQUAL;
        case RHI::CompareOp::LessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case RHI::CompareOp::Greater:
            return VK_COMPARE_OP_GREATER;
        case RHI::CompareOp::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case RHI::CompareOp::GreaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case RHI::CompareOp::Always:
            return VK_COMPARE_OP_ALWAYS;
        }
        return {};
    }

    static VkStencilOp RHIStencilOpToVk(RHI::StencilOp stencilOp)
    {
        switch (stencilOp)
        {
        case RHI::StencilOp::Keep:
            return VK_STENCIL_OP_KEEP;
        case RHI::StencilOp::Zero:
            return VK_STENCIL_OP_ZERO;
        case RHI::StencilOp::Replace:
            return VK_STENCIL_OP_REPLACE;
        case RHI::StencilOp::IncrementAndClamp:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case RHI::StencilOp::DecrementAndClamp:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case RHI::StencilOp::Invert:
            return VK_STENCIL_OP_INVERT;
        case RHI::StencilOp::Increment:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case RHI::StencilOp::Decrement:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        }
        return {};
    }

    GraphicsPipeline::GraphicsPipeline(VulkanDevice* device, const RHI::GraphicsPipelineDescriptor& desc)
        : Pipeline(device, desc)
        , desc(desc)
    {
        pipelineType = PipelineStateType::Graphics;

        SetupColorBlendState();
        SetupDepthStencilState();
        SetupShaderStages();
        SetupRasterState();
        SetupMultisampleState();
        SetupVertexInputState();

        auto rpCache = device->GetRenderPassCache();

        if (desc.renderTarget != nullptr)
        {
            renderPass = ((Vulkan::RenderTarget*)desc.renderTarget)->GetRenderPass();
            if (renderPass)
            {
                FindOrCompile(renderPass, desc.subpass);
            }
        }
        else
        {
            RenderPass::Descriptor rpDesc{};
            rpDesc.name = name;
            RenderPass::BuildDescriptor(desc.rtLayout, rpDesc);
            renderPass = rpCache->FindOrCreate(rpDesc);
            if (renderPass)
            {
                FindOrCompile(renderPass, desc.subpass);
            }
        }

        hash = desc.GetHash();
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        LockGuard<SharedMutex> lock{ pipelineMutex };

        for (auto [_, pipelineCache] : pipelineCachesByHash)
        {
            vkDestroyPipelineCache(device->GetHandle(), pipelineCache, VULKAN_CPU_ALLOCATOR);
        }
        pipelineCaches.Clear();
        pipelineCachesByHash.Clear();

        for (auto [_, pipeline] : pipelinesByHash)
        {
            vkDestroyPipeline(device->GetHandle(), pipeline, VULKAN_CPU_ALLOCATOR);
        }
        pipelines.Clear();
        pipelinesByHash.Clear();
    }

    VkPipeline GraphicsPipeline::FindOrCompile(RenderPass* renderPass, u32 subpass, u32 numViewports, u32 numScissors)
    {
        if (renderPass == nullptr || subpass >= RHI::Limits::Pipeline::MaxSubPassCount)
            return nullptr;

        PipelineVariant variant{};
        variant.pass = renderPass;
        variant.subpass = subpass;
        variant.numViewports = numViewports;
        variant.numScissors = numScissors;

        SIZE_T instanceHash = this->hash;
        CombineHash(instanceHash, variant);

        pipelineMutex.Lock();
        if (pipelinesByHash[instanceHash] != nullptr)
        {
            VkPipeline pipeline = pipelinesByHash[instanceHash];
            pipelineMutex.Unlock();
            return pipeline;
        }
        pipelineMutex.Unlock();

        return CompileInternal(renderPass, subpass, numViewports, numScissors);
    }

    VkPipeline GraphicsPipeline::CompileInternal(RenderPass* renderPass, u32 subpass, u32 numViewports, u32 numScissors)
    {
        VkGraphicsPipelineCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        createInfo.renderPass = renderPass->GetHandle();
        createInfo.subpass = subpass;

        const auto& renderPassDesc = renderPass->GetDescriptor();

        // - Color Blend State -

        while (colorBlendAttachments.GetSize() < renderPassDesc.subpasses[subpass].colorAttachments.GetSize())
        {
            colorBlendAttachments.Add(colorBlendAttachments.GetLast());
        }

        colorBlendState.attachmentCount = renderPassDesc.subpasses[subpass].colorAttachments.GetSize();
        colorBlendState.pAttachments = colorBlendAttachments.GetData();

        createInfo.pColorBlendState = &colorBlendState;

        // - Dynamic State -
        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_STENCIL_REFERENCE };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = COUNTOF(dynamicStates);
        dynamicState.pDynamicStates = dynamicStates;

        createInfo.pDynamicState = &dynamicState;

        // - Depth Stencil -
        createInfo.pDepthStencilState = &depthStencilState;

        // - Shader Stages -
        createInfo.stageCount = shaderStages.GetSize();
        createInfo.pStages = shaderStages.GetData();

        // - Raster State -
        createInfo.pRasterizationState = &rasterState;

        // - Viewport & Scissor -
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.scissorCount = numScissors;
        viewportState.pScissors = nullptr;
        viewportState.viewportCount = numViewports;
        viewportState.pViewports = nullptr;

        createInfo.pViewportState = &viewportState;

        // - Input Assembly -
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        createInfo.pInputAssemblyState = &inputAssembly;

        // - Vertex Input -
        VkPipelineVertexInputStateCreateInfo vertexInputState{};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.vertexBindingDescriptionCount = vertexBindingDescriptions.GetSize();
        vertexInputState.pVertexBindingDescriptions = vertexBindingDescriptions.GetData();
        vertexInputState.vertexAttributeDescriptionCount = vertexInputDescriptions.GetSize();
        vertexInputState.pVertexAttributeDescriptions = vertexInputDescriptions.GetData();

        createInfo.pVertexInputState = &vertexInputState;

        // - Multisample State -
        createInfo.pMultisampleState = &multisampleState;

        // - Pipeline Layout -
        createInfo.layout = pipelineLayout;

        PipelineVariant variant{};
        variant.pass = renderPass;
        variant.subpass = subpass;
        variant.numViewports = numViewports;
        variant.numScissors = numScissors;

        // Pipeline cache is a combination of hashes of: GraphicsPipelineDescriptor + Variant
        SIZE_T instanceHash = hash;
        CombineHash(instanceHash, variant);

        VkResult result;

        VkPipelineCache pipelineCache = nullptr;

	    {
		    LockGuard lock{ pipelineMutex };

        	if (pipelineCaches[variant] == nullptr)
        	{
        		VkPipelineCacheCreateInfo cacheCI{};
        		cacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        		cacheCI.initialDataSize = 0;
        		cacheCI.pInitialData = nullptr;

        		result = vkCreatePipelineCache(device->GetHandle(), &cacheCI, VULKAN_CPU_ALLOCATOR, &pipelineCache);
        		if (result == VK_SUCCESS)
        		{
        			pipelineCaches[variant] = pipelineCache;
        			pipelineCachesByHash[instanceHash] = pipelineCache;
        		}
        	}

        	pipelineCache = pipelineCaches[variant];
	    }

        VkPipeline pipeline = nullptr;
        result = vkCreateGraphicsPipelines(device->GetHandle(), pipelineCache, 1, &createInfo, VULKAN_CPU_ALLOCATOR, &pipeline);

        if (result != VK_SUCCESS)
        {
            CE_LOG(Error, All, "Failed to create Graphics Pipeline: {}", name);
            return nullptr;
        }

	    {
		    LockGuard lock{ pipelineMutex };

        	pipelines[variant] = pipeline;
        	pipelinesByHash[instanceHash] = pipeline;
	    }

        return pipeline;
    }

    void GraphicsPipeline::SetupColorBlendState()
    {
        colorBlendState = {};
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        colorBlendState.logicOpEnable = VK_FALSE;
        colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;

        colorBlendAttachments.Clear();

        for (int i = 0; i < desc.blendState.colorBlends.GetSize(); i++)
        {
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.blendEnable = desc.blendState.colorBlends[i].blendEnable ? VK_TRUE : VK_FALSE;
            colorBlendAttachment.colorBlendOp = RHIBlendOpToVk(desc.blendState.colorBlends[i].colorBlendOp);
            colorBlendAttachment.srcColorBlendFactor = RHIBlendFactorToVk(desc.blendState.colorBlends[i].srcColorBlend);
            colorBlendAttachment.dstColorBlendFactor = RHIBlendFactorToVk(desc.blendState.colorBlends[i].dstColorBlend);
            colorBlendAttachment.alphaBlendOp = RHIBlendOpToVk(desc.blendState.colorBlends[i].alphaBlendOp);
            colorBlendAttachment.srcAlphaBlendFactor = RHIBlendFactorToVk(desc.blendState.colorBlends[i].srcAlphaBlend);
            colorBlendAttachment.dstAlphaBlendFactor = RHIBlendFactorToVk(desc.blendState.colorBlends[i].dstAlphaBlend);
            colorBlendAttachment.colorWriteMask = 0;
            auto mask = desc.blendState.colorBlends[i].componentMask;
            if (EnumHasFlag(mask, RHI::ColorComponentMask::R))
                colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
            if (EnumHasFlag(mask, RHI::ColorComponentMask::G))
                colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
            if (EnumHasFlag(mask, RHI::ColorComponentMask::B))
                colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
            if (EnumHasFlag(mask, RHI::ColorComponentMask::A))
                colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;

            colorBlendAttachments.Add(colorBlendAttachment);
        }

        if (colorBlendAttachments.IsEmpty())
        {
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

            colorBlendAttachments.Add(colorBlendAttachment);
        }

        if (desc.renderTarget != nullptr)
        {
            Vulkan::RenderTarget* rt = (Vulkan::RenderTarget*)desc.renderTarget;
            RenderPass* rp = rt->GetRenderPass();

            u32 numColorTargets = rp->desc.subpasses[0].colorAttachments.GetSize();

            while (colorBlendAttachments.GetSize() < numColorTargets)
            {
                colorBlendAttachments.Add(colorBlendAttachments.GetLast());
            }
            while (colorBlendAttachments.GetSize() > numColorTargets) // Remove extra color blend states
            {
                colorBlendAttachments.RemoveAt(colorBlendAttachments.GetSize() - 1);
            }
        }
    }

    static void FillStencilOpState(VkStencilOpState& fill, RHI::StencilOpState state)
    {
        fill.compareOp = RHICompareOpToVk(state.compareOp);
        fill.depthFailOp = RHIStencilOpToVk(state.depthFailOp);
        fill.failOp = RHIStencilOpToVk(state.failOp);
        fill.passOp = RHIStencilOpToVk(state.passOp);
    }

    void GraphicsPipeline::SetupDepthStencilState()
    {
        depthStencilState = {};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        const auto& depthState = desc.depthStencilState.depthState;
        const auto& stencilState = desc.depthStencilState.stencilState;

        depthStencilState.depthCompareOp = RHICompareOpToVk(depthState.compareOp);
        depthStencilState.depthTestEnable = depthState.testEnable ? VK_TRUE : VK_FALSE;
        depthStencilState.depthWriteEnable = depthState.writeEnable ? VK_TRUE : VK_FALSE;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.maxDepthBounds = 1.0f;
        depthStencilState.minDepthBounds = 0.0f;

        depthStencilState.back = {};
        depthStencilState.front = {};
        {
            FillStencilOpState(depthStencilState.back, stencilState.backFace);
            depthStencilState.back.writeMask = stencilState.writeMask;
            depthStencilState.back.reference = 0; // Filled via dyanamic state

            FillStencilOpState(depthStencilState.front, stencilState.frontFace);
            depthStencilState.front.writeMask = stencilState.writeMask;
            depthStencilState.front.reference = 0; // Filled via dynamic state
        }

        depthStencilState.stencilTestEnable = stencilState.enable ? VK_TRUE : VK_FALSE;
    }

    void GraphicsPipeline::SetupShaderStages()
    {
        shaderStages.Clear();

        for (const auto& shaderStageDesc : desc.shaderStages)
        {
            VkPipelineShaderStageCreateInfo shaderStageCI{};
            shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCI.module = ((Vulkan::ShaderModule*)shaderStageDesc.shaderModule)->GetHandle();

            if (shaderStageDesc.GetShaderStage() == RHI::ShaderStage::Vertex)
                shaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
            else if (shaderStageDesc.GetShaderStage() == RHI::ShaderStage::Fragment)
                shaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            else if (shaderStageDesc.GetShaderStage() == RHI::ShaderStage::Geometry)
                shaderStageCI.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            else
                continue;

            shaderStageCI.pName = shaderStageDesc.entryPoint.GetCString();

            shaderStages.Add(shaderStageCI);
        }
    }

    void GraphicsPipeline::SetupRasterState()
    {
        rasterState = {};
        rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        const RHI::RasterState& state = desc.rasterState;

        rasterState.depthClampEnable = state.depthClipEnable ? VK_FALSE : VK_TRUE;
        rasterState.rasterizerDiscardEnable = VK_FALSE;
        rasterState.lineWidth = 1.0f;

        switch (state.fillMode)
        {
        case RHI::FillMode::Solid:
            rasterState.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        case RHI::FillMode::Wireframe:
            rasterState.polygonMode = VK_POLYGON_MODE_LINE;
            break;
        }

        switch (state.cullMode)
        {
        case RHI::CullMode::None:
            rasterState.cullMode = VK_CULL_MODE_NONE;
            break;
        case RHI::CullMode::Back:
            rasterState.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case RHI::CullMode::Front:
            rasterState.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
        case RHI::CullMode::All:
            rasterState.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
            break;
        }

        rasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterState.depthBiasEnable = VK_FALSE;
        rasterState.depthBiasClamp = 0;
        rasterState.depthBiasConstantFactor = 0;
        rasterState.depthBiasSlopeFactor = 0;
    }

    void GraphicsPipeline::SetupMultisampleState()
    {
        multisampleState = {};
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.rasterizationSamples = GetSampleCountFlagBits(desc.multisampleState.sampleCount);
        multisampleState.minSampleShading = 1.0f;
        multisampleState.pSampleMask = nullptr;
        multisampleState.alphaToCoverageEnable = VK_FALSE;
        multisampleState.alphaToOneEnable = VK_FALSE;
    }

    void GraphicsPipeline::SetupVertexInputState()
    {
        vertexBindingDescriptions = {};
        vertexInputDescriptions = {};

        for (int i = 0; i < desc.vertexAttributes.GetSize(); i++)
        {
            const auto& vertexAttrib = desc.vertexAttributes[i];

            VkVertexInputAttributeDescription attrib{};
            attrib.binding = vertexAttrib.inputSlot;
            attrib.location = vertexAttrib.location;
            attrib.offset = vertexAttrib.offset;

            switch (vertexAttrib.dataType)
            {
            case VertexAttributeDataType::Float:
                attrib.format = VK_FORMAT_R32_SFLOAT;
                break;
            case VertexAttributeDataType::Float2:
                attrib.format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case VertexAttributeDataType::Float3:
                attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case VertexAttributeDataType::Float4:
                attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            case VertexAttributeDataType::Int:
                attrib.format = VK_FORMAT_R32_SINT;
                break;
            case VertexAttributeDataType::Int2:
                attrib.format = VK_FORMAT_R32G32_SINT;
                break;
            case VertexAttributeDataType::Int3:
                attrib.format = VK_FORMAT_R32G32B32_SINT;
                break;
            case VertexAttributeDataType::Int4:
                attrib.format = VK_FORMAT_R32G32B32A32_SINT;
                break;
            case VertexAttributeDataType::UInt:
                attrib.format = VK_FORMAT_R32_UINT;
                break;
            case VertexAttributeDataType::UInt2:
                attrib.format = VK_FORMAT_R32G32_UINT;
                break;
            case VertexAttributeDataType::UInt3:
                attrib.format = VK_FORMAT_R32G32B32_UINT;
                break;
            case VertexAttributeDataType::UInt4:
                attrib.format = VK_FORMAT_R32G32B32A32_UINT;
                break;
			case VertexAttributeDataType::Undefined:
				break;
			case VertexAttributeDataType::Char4:
                attrib.format = VK_FORMAT_R8G8B8A8_SNORM;
				break;
			case VertexAttributeDataType::UChar4:
                attrib.format = VK_FORMAT_R8G8B8A8_UNORM;
				break;
            }

            vertexInputDescriptions.Add(attrib);
        }

        for (int i = 0; i < desc.vertexInputSlots.GetSize(); i++)
        {
            const auto& vertexInput = desc.vertexInputSlots[i];

            VkVertexInputBindingDescription inputBinding{};
            inputBinding.binding = vertexInput.inputSlot;
            if (vertexInput.inputRate == RHI::VertexInputRate::PerInstance)
                inputBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            else
                inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            inputBinding.stride = vertexInput.stride;

            vertexBindingDescriptions.Add(inputBinding);
        }
    }

} // namespace CE::Vulkan
