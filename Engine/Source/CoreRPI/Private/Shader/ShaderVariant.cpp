#include "CoreRPI.h"

namespace CE::RPI
{

	ShaderVariant::ShaderVariant(const ShaderVariantDescriptor& desc)
		: reflectionInfo(desc.reflectionInfo)
	{
		variantId = 0;

		pipelineDesc = {};
		pipelineDesc.name = desc.shaderName;

		bool blendFactorFound = false;
		EnumType* blendFactorEnum = GetStaticEnum<RHI::BlendFactor>();
		EnumType* blendOpEnum = GetStaticEnum<RHI::BlendOp>();
		EnumType* depthOpEnum = GetStaticEnum<RHI::CompareOp>();
		EnumType* cullModeEnum = GetStaticEnum<RHI::CullMode>();

		RHI::ColorBlendState colorBlend = {};

		bool depthOnly = false;
		if (desc.TagExists("DrawListTag") && 
			(desc.GetTagValue("DrawListTag") == "depth" || desc.GetTagValue("DrawListTag") == "shadow"))
		{
			depthOnly = true;
		}
		
		if (desc.TagExists("Blend"))
		{
			String value = desc.GetTagValue("Blend").RemoveWhitespaces();
			Array<String> splits = value.Split(',');
			if (splits.GetSize() == 2)
			{
				EnumConstant* srcBlend = blendFactorEnum->FindConstantWithName(splits[0]);
				EnumConstant* dstBlend = blendFactorEnum->FindConstantWithName(splits[1]);
				
				if (srcBlend != nullptr && dstBlend != nullptr)
				{
					colorBlend.srcColorBlend = (RHI::BlendFactor)srcBlend->GetValue();
					colorBlend.dstColorBlend = (RHI::BlendFactor)dstBlend->GetValue();
					colorBlend.srcAlphaBlend = RHI::BlendFactor::One;
					colorBlend.dstAlphaBlend = RHI::BlendFactor::Zero;
					blendFactorFound = true;
				}
			}
		}

		if (desc.TagExists("BlendOp"))
		{
			String value = desc.GetTagValue("BlendOp").RemoveWhitespaces();
			EnumConstant* enumConstant = blendOpEnum->FindConstantWithName(value);
			
			if (enumConstant)
			{
				colorBlend.colorBlendOp = (RHI::BlendOp)enumConstant->GetValue();
				blendFactorFound = true;
			}
		}

		if (!blendFactorFound)
		{
			colorBlend = RHI::ColorBlendState(); // Default value
		}

		if (!depthOnly)
		{
			pipelineDesc.blendState.colorBlends.Add(colorBlend);
		}
		
		pipelineDesc.srgLayouts = desc.reflectionInfo.srgLayouts;
		for (auto& srgLayout : pipelineDesc.srgLayouts)
		{
			if (srgLayout.srgType != SRGType::PerScene &&
				srgLayout.srgType != SRGType::PerView)
			{
				continue;
			}

			for (auto& variable : srgLayout.variables)
			{
				variable.shaderStages |= ShaderStage::Compute;
			}
		}

		pipelineDesc.multisampleState.sampleCount = 1;

		pipelineDesc.depthStencilState.depthState.enable = true;
		pipelineDesc.depthStencilState.depthState.testEnable = true;
		pipelineDesc.depthStencilState.depthState.writeEnable = false;

		if (desc.TagExists("ZWrite"))
		{
			String zwriteValue = desc.GetTagValue("ZWrite");
			
			if (zwriteValue == "On")
			{
				pipelineDesc.depthStencilState.depthState.writeEnable = true;
			}
			else
			{
				pipelineDesc.depthStencilState.depthState.writeEnable = false;
			}
		}

		if (desc.TagExists("ZTest"))
		{
			String ztestValue = desc.GetTagValue("ZTest");
			EnumConstant* enumConstant = depthOpEnum->FindConstantWithName(ztestValue);

			if (enumConstant)
			{
				pipelineDesc.depthStencilState.depthState.testEnable = true;
				pipelineDesc.depthStencilState.depthState.compareOp = (RHI::CompareOp)enumConstant->GetValue();
			}
			else
			{
				pipelineDesc.depthStencilState.depthState.testEnable = false;
			}
		}

		if (!pipelineDesc.depthStencilState.depthState.testEnable)
		{
			pipelineDesc.depthStencilState.depthState.enable = false;
		}

		// TODO: Add support for stencil state
		pipelineDesc.depthStencilState.stencilState.enable = false;

		if (desc.TagExists("Cull"))
		{
			String cullValue = desc.GetTagValue("Cull");
			EnumConstant* enumConstant = cullModeEnum->FindConstantWithName(cullValue);

			if (enumConstant)
			{
				pipelineDesc.rasterState.cullMode = (RHI::CullMode)enumConstant->GetValue();
			}
			else
			{
				pipelineDesc.rasterState.cullMode = RHI::CullMode::Back;
			}
		}

		if (desc.interleaveVertexData)
		{
			u32 offset = 0;

			RHI::VertexInputSlotDescriptor inputSlotDesc{};
			inputSlotDesc.inputSlot = 0;
			inputSlotDesc.inputRate = RHI::VertexInputRate::PerVertex;

			for (int i = 0; i < desc.reflectionInfo.vertexInputs.GetSize(); i++)
			{
				RHI::ShaderSemantic shaderSemantic = RHI::ShaderSemantic::Parse(desc.reflectionInfo.vertexInputs[i].GetString());

				RHI::VertexAttributeDescriptor vertexAttrib{};
				vertexAttrib.inputSlot = 0;
				vertexAttrib.location = i;
				vertexAttrib.offset = offset;
				vertexAttrib.dataType = desc.reflectionInfo.vertexInputTypes[i];

				switch (vertexAttrib.dataType)
				{
				case VertexAttributeDataType::Float:
					inputSlotDesc.stride += sizeof(f32);
					offset += sizeof(f32);
					break;
				case VertexAttributeDataType::Float2:
					inputSlotDesc.stride += sizeof(Vec2);
					offset += sizeof(Vec2);
					break;
				case VertexAttributeDataType::Float4:
					inputSlotDesc.stride += sizeof(Vec4);
					offset += sizeof(Vec4);
					break;
				case VertexAttributeDataType::Int:
					inputSlotDesc.stride += sizeof(int);
					offset += sizeof(int);
					break;
				case VertexAttributeDataType::Int2:
					inputSlotDesc.stride += sizeof(Vec2i);
					offset += sizeof(Vec2i);
					break;
				case VertexAttributeDataType::Int4:
					inputSlotDesc.stride += sizeof(Vec4i);
					offset += sizeof(Vec4i);
					break;
				case VertexAttributeDataType::UInt:
					inputSlotDesc.stride += sizeof(u32);
					offset += sizeof(u32);
					break;
				case VertexAttributeDataType::UInt2:
					inputSlotDesc.stride += sizeof(Vec2i);
					offset += sizeof(Vec2i);
					break;
				case VertexAttributeDataType::UInt4:
					inputSlotDesc.stride += sizeof(Vec4i);
					offset += sizeof(Vec4i);
					break;
				case VertexAttributeDataType::Char4:
				case VertexAttributeDataType::UChar4:
					inputSlotDesc.stride += sizeof(u32);
					offset += sizeof(u32);
					break;
				default:
					continue;
				}

				pipelineDesc.vertexAttributes.Add(vertexAttrib);
			}

			pipelineDesc.vertexInputSlots.Add(inputSlotDesc);
		}
		else
		{
			for (int i = 0; i < desc.reflectionInfo.vertexInputs.GetSize(); i++)
			{
				RHI::ShaderSemantic shaderSemantic = RHI::ShaderSemantic::Parse(desc.reflectionInfo.vertexInputs[i].GetString());

				RHI::VertexInputSlotDescriptor inputSlotDesc{};
				inputSlotDesc.inputSlot = i;
				inputSlotDesc.inputRate = RHI::VertexInputRate::PerVertex;

				RHI::VertexAttributeDescriptor vertexAttrib{};
				vertexAttrib.inputSlot = i;
				vertexAttrib.location = i;
				vertexAttrib.offset = 0;
				vertexAttrib.dataType = desc.reflectionInfo.vertexInputTypes[i];

				switch (vertexAttrib.dataType)
				{
				case VertexAttributeDataType::Float:
					inputSlotDesc.stride = sizeof(float);
					break;
				case VertexAttributeDataType::Float2:
					inputSlotDesc.stride = sizeof(Vec2);
					break;
				case VertexAttributeDataType::Float4:
					inputSlotDesc.stride = sizeof(Vec4);
					break;
				case VertexAttributeDataType::Int:
					inputSlotDesc.stride = sizeof(int);
					break;
				case VertexAttributeDataType::Int2:
					inputSlotDesc.stride = sizeof(Vec2i);
					break;
				case VertexAttributeDataType::Int4:
					inputSlotDesc.stride = sizeof(Vec4i);
					break;
				case VertexAttributeDataType::UInt:
					inputSlotDesc.stride = sizeof(u32);
					break;
				case VertexAttributeDataType::UInt2:
					inputSlotDesc.stride = sizeof(Vec2i);
					break;
				case VertexAttributeDataType::UInt4:
					inputSlotDesc.stride = sizeof(Vec4i);
					break;
				case VertexAttributeDataType::Char4:
				case VertexAttributeDataType::UChar4:
					inputSlotDesc.stride = sizeof(u32);
					break;
				default:
					continue;
				}

				pipelineDesc.vertexInputSlots.Add(inputSlotDesc);
				pipelineDesc.vertexAttributes.Add(vertexAttrib);
			}
		}

		modulesByStage.Clear();

		for (int i = 0; i < desc.moduleDesc.GetSize(); i++)
		{
			if (depthOnly && desc.moduleDesc[i].stage != RHI::ShaderStage::Vertex)
			{
				continue;
			}

			auto moduleDesc = desc.moduleDesc[i];
			moduleDesc.debugName = desc.shaderName;

			auto module = RHI::gDynamicRHI->CreateShaderModule(moduleDesc);
			modulesByStage[moduleDesc.stage] = module;
			
			pipelineDesc.shaderStages.Add({});
			pipelineDesc.shaderStages.Top().entryPoint = desc.entryPoints[i];
			pipelineDesc.shaderStages.Top().shaderModule = module;
		}

		// Default RenderTarget Layout
		RHI::RenderAttachmentLayout colorAttachment{};
		colorAttachment.format = RHI::Format::R8G8B8A8_UNORM;
		colorAttachment.attachmentUsage = ScopeAttachmentUsage::Color;
		colorAttachment.attachmentId = "Color";
		colorAttachment.multisampleState.sampleCount = 1;
		colorAttachment.loadAction = AttachmentLoadAction::Clear;
		colorAttachment.storeAction = AttachmentStoreAction::Store;
		pipelineDesc.rtLayout.attachmentLayouts.Add(colorAttachment);

		RHI::RenderAttachmentLayout depthStencilAttachment{};
		depthStencilAttachment.attachmentId = "DepthStencil";
		depthStencilAttachment.format = RHI::gDynamicRHI->GetAvailableDepthStencilFormats()[0];
		depthStencilAttachment.attachmentUsage = ScopeAttachmentUsage::DepthStencil;
		depthStencilAttachment.multisampleState.sampleCount = 1;
		depthStencilAttachment.loadAction = AttachmentLoadAction::Load;
		depthStencilAttachment.storeAction = AttachmentStoreAction::Store;
		if (pipelineDesc.depthStencilState.depthState.enable)
		{
			if (pipelineDesc.depthStencilState.depthState.writeEnable)
			{
				depthStencilAttachment.loadAction = AttachmentLoadAction::Clear;
			}
			else if (pipelineDesc.depthStencilState.depthState.testEnable)
			{
				depthStencilAttachment.loadAction = AttachmentLoadAction::Load;
			}

			pipelineDesc.rtLayout.attachmentLayouts.Add(depthStencilAttachment);
		}

		pipelineDesc.rootConstantLayout = desc.reflectionInfo.rootConstantLayout;
		pipelineDesc.rootConstantShaderStages = desc.reflectionInfo.rootConstantStages;
		
		pipelineCollection = new RHI::GraphicsPipelineCollection(pipelineDesc);
	}

	ShaderVariant::~ShaderVariant()
	{
		//delete pipeline; pipeline = nullptr;
		delete pipelineCollection; pipelineCollection = nullptr;

		for (auto [stage, module] : modulesByStage)
		{
			if (module == nullptr) 
				continue;
			RHI::gDynamicRHI->DestroyShaderModule(module);
		}
		modulesByStage.Clear();
	}

	RHI::PipelineState* ShaderVariant::GetPipeline(const RHI::GraphicsPipelineVariant& variant)
	{
		return pipelineCollection->GetPipeline(variant);
	}

	RHI::PipelineState* ShaderVariant::GetPipeline(const RHI::MultisampleState& multisampleState)
	{
		RHI::GraphicsPipelineVariant variant = pipelineCollection->GetDefaultVariant();
		variant.sampleState = multisampleState;
		return pipelineCollection->GetPipeline(variant);
	}

	const RHI::ShaderResourceGroupLayout& ShaderVariant::GetSrgLayout(RHI::SRGType srgType)
	{
		for (const auto& srgLayout : pipelineDesc.srgLayouts)
		{
			if (srgLayout.srgType == srgType)
				return srgLayout;
		}
		
		thread_local RHI::ShaderResourceGroupLayout empty{};
		return empty;
	}

	bool ShaderVariant::HasSrgLayout(RHI::SRGType srgType)
	{
		for (const auto& srgLayout : pipelineDesc.srgLayouts)
		{
			if (srgLayout.srgType == srgType)
				return true;
		}

		return false;
	}

} // namespace CE::RPI
