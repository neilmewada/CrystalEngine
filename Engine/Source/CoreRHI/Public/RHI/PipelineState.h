#pragma once

namespace CE::RHI
{
	class RenderTarget;

	enum class PipelineStateType
	{
		Graphics,
		Compute,
		RayTracing,
		COUNT
	};

	ENUM()
	enum class VertexAttributeDataType
	{
		Undefined,
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		UInt, UInt2, UInt3, UInt4,
		Char4,
		UChar4
	};
	ENUM_CLASS(VertexAttributeDataType);

	struct VertexAttributeDescriptor
	{
		VertexAttributeDataType dataType = VertexAttributeDataType::Undefined;
		u32 location = 0;
		u32 offset = 0;
		u32 inputSlot = 0;

		SIZE_T GetHash() const;
	};

	ENUM()
	enum class VertexInputRate
	{
		PerVertex = 0,
		PerInstance
	};
	ENUM_CLASS(VertexInputRate);

	struct VertexInputSlotDescriptor
	{
		u32 inputSlot = 0;
		u32 stride = 0;
		VertexInputRate inputRate = VertexInputRate::PerVertex;

		inline SIZE_T GetHash() const
		{
			SIZE_T hash = CE::GetHash(inputSlot);
			CombineHash(hash, stride);
			CombineHash(hash, inputRate);
			return hash;
		}
	};

	struct ShaderStageDescriptor
	{
		RHI::ShaderModule* shaderModule = nullptr;
		Name entryPoint = "";

		CORERHI_API RHI::ShaderStage GetShaderStage() const;

		CORERHI_API SIZE_T GetHash() const;
	};

	ENUM()
	enum class BlendOp
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min, Max
	};
	ENUM_CLASS(BlendOp);

	ENUM()
	enum class BlendFactor
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha
	};
	ENUM_CLASS(BlendFactor);

	ENUM()
	enum class CompareOp : int
	{
		Never = 0,
		Less = 1,
		Equal = 2,
		LessOrEqual = 3,
		LEqual = LessOrEqual,
		Greater = 4,
		NotEqual = 5,
		GreaterOrEqual = 6,
		GEqual = GreaterOrEqual,
		Always = 7,
	};
	ENUM_CLASS(CompareOp);

	ENUM()
	enum class StencilOp
	{
		Keep,
		Zero,
		Replace,
		IncrementAndClamp,
		DecrementAndClamp,
		Invert,
		Increment,
		Decrement
	};
	ENUM_CLASS(StencilOp);

	STRUCT()
	struct CORERHI_API ColorBlendState
	{
		CE_STRUCT(ColorBlendState)
	public:

		FIELD()
		bool blendEnable = true;

		FIELD()
		BlendOp colorBlendOp = BlendOp::Add;

		FIELD()
		BlendFactor srcColorBlend = BlendFactor::SrcAlpha;

		FIELD()
		BlendFactor dstColorBlend = BlendFactor::OneMinusSrcAlpha;

		FIELD()
		BlendOp alphaBlendOp = BlendOp::Add;

		FIELD()
		BlendFactor srcAlphaBlend = BlendFactor::One;

		FIELD()
		BlendFactor dstAlphaBlend = BlendFactor::Zero;

		FIELD()
		ColorComponentMask componentMask = ColorComponentMask::All;

		inline SIZE_T GetHash() const
		{
			SIZE_T hash = CE::GetHash(blendEnable);
			if (!blendEnable)
				return hash;
			CombineHash(hash, colorBlendOp);
			CombineHash(hash, srcColorBlend);
			CombineHash(hash, dstColorBlend);
			CombineHash(hash, alphaBlendOp);
			CombineHash(hash, srcAlphaBlend);
			CombineHash(hash, dstAlphaBlend);
			CombineHash(hash, componentMask);
			return hash;
		}
	};

	STRUCT()
	struct CORERHI_API BlendState
	{
		CE_STRUCT(BlendState)
	public:

		FIELD()
		Array<ColorBlendState> colorBlends{};

		inline SIZE_T GetHash() const
		{
			SIZE_T hash = 0;
			for (int i = 0; i < colorBlends.GetSize(); i++)
			{
				if (i == 0)
					hash = colorBlends[i].GetHash();
				else
					CombineHash(hash, colorBlends[i]);
			}
			return hash;
		}
	};

	STRUCT()
	struct CORERHI_API DepthState
	{
		CE_STRUCT(DepthState)
	public:

		FIELD()
		bool enable = false;

		FIELD()
		CompareOp compareOp = CompareOp::Less;

		FIELD()
		bool testEnable = false;

		FIELD()
		bool writeEnable = false;

		inline SIZE_T GetHash() const
		{
			SIZE_T hash = CE::GetHash(enable);
			if (!enable)
				return hash;
			CombineHash(hash, compareOp);
			CombineHash(hash, testEnable);
			CombineHash(hash, writeEnable);
			return hash;
		}
	};

	STRUCT()
	struct CORERHI_API StencilOpState
	{
		CE_STRUCT(StencilOpState)
	public:

		FIELD()
		StencilOp failOp = StencilOp::Keep;

		FIELD()
		StencilOp depthFailOp = StencilOp::Keep;

		FIELD()
		StencilOp passOp = StencilOp::Keep;

		FIELD()
		CompareOp compareOp = CompareOp::Always;

		SIZE_T GetHash() const;
	};

	struct PipelineDescriptor
	{
		// Used for debugging purposes.
		Name name{};

		Array<ShaderStageDescriptor> shaderStages{};
		Array<ShaderResourceGroupLayout> srgLayouts{};

		Array<ShaderStructMemberType> rootConstantLayout{};
		ShaderStage rootConstantShaderStages = ShaderStage::None;

		SIZE_T GetHash() const;
	};

	STRUCT()
	struct CORERHI_API StencilState
	{
		CE_STRUCT(StencilState)
	public:

		FIELD()
		bool enable = false;

		FIELD()
		u32 readMask = 0xFF;

		FIELD()
		u32 writeMask = 0xFF;

		FIELD()
		StencilOpState frontFace{};

		FIELD()
		StencilOpState backFace{};

		SIZE_T GetHash() const;
	};

	STRUCT()
	struct CORERHI_API DepthStencilState
	{
		CE_STRUCT(DepthStencilState)
	public:

		FIELD()
		DepthState depthState{};

		FIELD()
		StencilState stencilState{};

		SIZE_T GetHash() const;
	};

	STRUCT()
	struct CORERHI_API RasterState
	{
		CE_STRUCT(RasterState)
	public:

		FIELD()
		CullMode cullMode = CullMode::Back;

		FIELD()
		FillMode fillMode = FillMode::Solid;

		FIELD()
		bool multisampleEnable = false;

		FIELD()
		bool depthClipEnable = true;

		SIZE_T GetHash() const;
	};

	struct GraphicsPipelineDescriptor : PipelineDescriptor
	{
		Array<VertexAttributeDescriptor> vertexAttributes{};
		Array<VertexInputSlotDescriptor> vertexInputSlots{};
		
		RasterState rasterState{};
		BlendState blendState{};
		DepthStencilState depthStencilState{};
		MultisampleState multisampleState{};

		RenderTarget* renderTarget = nullptr;
		RenderTargetLayout rtLayout{};

		u32 subpass = 0;

		CORERHI_API SIZE_T GetHash() const;
	};

	struct ComputePipelineDescriptor : PipelineDescriptor
	{

	};

	class IPipelineLayout
	{
	protected:

		virtual ~IPipelineLayout() {}

	public:

		virtual bool IsCompatibleWith(IPipelineLayout* other) = 0;

	};

	class PipelineState : public RHIResource
	{
	public:
		virtual ~PipelineState() = default;

		inline PipelineStateType GetPipelineType() const { return pipelineType; }

		inline bool IsGraphicsPipeline() const { return pipelineType == PipelineStateType::Graphics; }
		inline bool IsComputePipeline() const { return pipelineType == PipelineStateType::Compute; }
		inline bool IsRayTracingPipeline() const { return pipelineType == PipelineStateType::RayTracing; }

		virtual IPipelineLayout* GetPipelineLayout() = 0;
		
		inline const RHI::GraphicsPipelineDescriptor& GetGraphicsDescriptor() const { return graphicsDescriptor; }

	protected:
		PipelineState() : RHIResource(RHI::ResourceType::PipelineState) {}

		PipelineStateType pipelineType{};

		RHI::GraphicsPipelineDescriptor graphicsDescriptor{};
		RHI::ComputePipelineDescriptor computeDescriptor{};
	};
    
} // namespace CE::RHI

#include "PipelineState.rtti.h"