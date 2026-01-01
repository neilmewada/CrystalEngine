#pragma once

namespace CE::RHI
{
	class Viewport;
	class ShaderResourceGroup;
	class Texture;
	class Buffer;
	class CommandList;
	class IDeviceObject;
	class RenderTarget;
	class RenderTargetBuffer;
	class RayTracingBlas;
	struct AttachmentClearValue;
    class SwapChain;

	enum class CommandListType
	{
		Direct = 0,
		Indirect
	};

	struct SubresourceRange
	{
		u32 baseMipLevel = 0;
		u32 levelCount = 0;
		u32 baseArrayLayer = 0;
		u32 layerCount = 0;

		constexpr static SubresourceRange All()
		{
			SubresourceRange result;
			result.baseMipLevel = result.levelCount = result.baseArrayLayer = result.layerCount = 0;
			return result;
		}

		inline bool IsAll() const
		{
			return baseMipLevel == 0 && levelCount == 0 && baseArrayLayer == 0 && layerCount == 0;
		}
	};

	struct SubresourceLayers
	{
		u32 mipSlice = 0;
		u32 baseArrayLayer = 0;
		u32 layerCount = 0;

		constexpr static SubresourceLayers All()
		{
			SubresourceLayers result;
			result.mipSlice = result.baseArrayLayer = result.layerCount = 0;
			return result;
		}
	};

	struct BlitRegion
	{
		SubresourceLayers srcSubresource{};
		Vec3i srcOffset{};
		Vec3i srcExtent{};
		SubresourceLayers dstSubresource{};
		Vec3i dstOffset{};
		Vec3i dstExtent{};
	};

	struct ResourceBarrierDescriptor
	{
		IDeviceObject* resource;
		ResourceState fromState;
		ResourceState toState;
		SubresourceRange subresourceRange = SubresourceRange::All();
	};

	struct BufferToTextureCopy
	{
		RHI::Buffer* srcBuffer = nullptr;
		u64 bufferOffset = 0;

		RHI::Texture* dstTexture = nullptr;
		u16 mipSlice = 0;
		u16 baseArrayLayer = 0;
		u16 layerCount = 1;
	};

	struct BufferCopy
	{
		RHI::Buffer* srcBuffer = nullptr;
		u64 srcOffset = 0;
		RHI::Buffer* dstBuffer = nullptr;
		u64 dstOffset = 0;
		u64 totalByteSize = 0;
	};

	struct TextureToBufferCopy
	{
		RHI::Texture* srcTexture = nullptr;
		u16 mipSlice = 0;
		u16 baseArrayLayer = 0;
		u16 layerCount = 1;

		RHI::Buffer* dstBuffer = nullptr;
		u64 bufferOffset = 0;
	};

	class CORERHI_API CommandList : public RHIResource
	{
	protected:
		CommandList() : RHIResource(ResourceType::CommandList)
		{}
	public:
		virtual ~CommandList() = default;

		// - Public API -

		inline RHI::CommandListType GetCommandListType() const 
		{
			return commandListType;
		}

		// - Command List API -

		virtual void Begin() = 0;
		virtual void End() = 0;
        
        virtual bool BeginRenderPass(RenderPass* renderPass, RenderPassFrameBuffer* frameBuffer, AttachmentClearValue* clearValuesPerAttachment) = 0;
        
        virtual void RenderPassNextSubpass() = 0;
        
        virtual void EndRenderPass() = 0;

		inline void SetFrameIndex(u32 frameIndex)
		{
			this->currentFrameIndex = frameIndex;
		}

		virtual void ResourceBarrier(u32 count, ResourceBarrierDescriptor* barriers) = 0;

		virtual void SetShaderResourceGroups(const ArrayView<RHI::ShaderResourceGroup*>& srgs) = 0;

		inline void SetShaderResourceGroup(RHI::ShaderResourceGroup* srg)
		{
			if (srg != nullptr)
			{
				SetShaderResourceGroups({ srg });
			}
		}

		virtual void ClearShaderResourceGroups() = 0;

		virtual void SetRootConstants(u32 offset, u32 num32BitValues, const void* srcData) = 0;

		virtual void SetViewports(u32 count, ViewportState* viewports) = 0;
		virtual void SetScissors(u32 count, ScissorState* scissors) = 0;

		virtual void CommitShaderResources() = 0;

		virtual void BindPipelineState(RHI::PipelineState* pipelineState) = 0;

		virtual void BindVertexBuffers(u32 firstInputSlot, u32 count, const RHI::VertexBufferView* bufferViews) = 0;

		virtual void BindIndexBuffer(const RHI::IndexBufferView& bufferView) = 0;

		virtual void DrawIndexed(const DrawIndexedArguments& args) = 0;

		virtual void DrawLinear(const DrawLinearArguments& args) = 0;

		virtual void Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ) = 0;

		virtual void CopyTextureRegion(const BufferToTextureCopy& region) = 0;
		virtual void CopyTextureRegion(const TextureToBufferCopy& region) = 0;

		virtual void CopyBufferRegion(const BufferCopy& copy) = 0;

		// - Ray Tracing -

		virtual void BuildBlas(RHI::RayTracingBlas* blas) = 0;

	protected:

		u32 currentFrameIndex = 0;

		RHI::CommandListType commandListType = RHI::CommandListType::Direct;

	};

} // namespace CE::RHI
