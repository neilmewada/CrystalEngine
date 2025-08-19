#pragma once

namespace CE::RHI
{
	namespace Limits
	{
		constexpr u32 MaxRenderTargetResolution = 4096*2;

		constexpr u32 ModelLodCountMax = 8;

		constexpr u64 MaxConstantBufferSize = 16_KB; // 16384 bytes

#if PLATFORM_DESKTOP
		constexpr u64 MaxStructuredBufferSize = 512_MB;
#else
		constexpr u64 MaxStructuredBufferSize = 128_MB; // 134217728 bytes
#endif

		//! @brief Max number of back buffers (images) that can be used in swap chain.
		constexpr u32 MaxSwapChainImageCount = 2;

		namespace Pipeline
		{
			//! @brief Max number of vertex attributes
			constexpr u32 MaxVertexAttribCount = 16;

			//! @brief Max number of vertex buffer input slots.
			constexpr u32 MaxVertexInputSlotCount = 16;

			//! @brief Max number of color attachment counts.
#if PLATFORM_ANDROID
			constexpr u32 MaxColorAttachmentCount = 4;
#else
			constexpr u32 MaxColorAttachmentCount = 8;
#endif

			//! @brief Max number of output attachments. Resolve + Color + DepthStencil
			constexpr u32 MaxRenderAttachmentCount = 2 * MaxColorAttachmentCount + 1;

			//! @brief Max number of subpasses a render pass can have
			constexpr u32 MaxSubPassCount = 8;

			/// @brief Max number of DrawList Tags
			constexpr u32 DrawListTagCount = 128;

			/// @brief Max number of shader resource groups that can be bound
			constexpr u32 MaxShaderResourceGroupCount = 8;

			/// @brief Max byte size of root constants
			constexpr u8 MaxRootConstantSize = 128;
		}
	}
} // namespace CE::RPI
