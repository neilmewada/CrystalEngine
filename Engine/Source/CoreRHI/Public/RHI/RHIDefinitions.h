#pragma once

#include "ImGui/ImGuiDefinitions.h"

namespace CE::RHI
{

	/// Only Vulkan is supported
    enum class GraphicsBackend
    {
        None,
        Vulkan,
    };

    enum class ResourceType
    {
        None,
        Buffer,
        Texture,
        TextureView,
        Sampler,
        ShaderModule,
        Pipeline,
        PipelineState,
		ShaderResourceGroup,

		MemoryHeap,
        RenderTarget,
        RenderTargetBuffer,
        Viewport,
		SwapChain,
        DeviceLimits,

        Fence,
		CommandQueue,
        CommandList
    };

    enum class DeviceObjectType
    {
        None,
        Buffer,
        Texture
    };

    struct FontDesc
    {
        SIZE_T byteSize = 0;
        u32 pointSize = 14;
        String fontName = "";
		b8 isCompressed = false;

        void* fontData = nullptr;
    };

    struct FontPreloadConfig
    {
        u32 preloadFontCount = 0;
        FontDesc* preloadFonts = nullptr;
    };

	struct MultisampleState
	{
		u16 sampleCount = 1;
		f32 quality = 1.0f;

        inline SIZE_T GetHash() const
        {
            SIZE_T hash = CE::GetHash(sampleCount);
            CombineHash(hash, quality);
            return hash;
        }
	};

    /*
    *   Buffer
    */

    ENUM()
	enum class IndexFormat
	{
		Uint16 = 0,
		Uint32
	};
    ENUM_CLASS(IndexFormat);

	ENUM(Flags)
    enum class BufferBindFlags
    {
        Undefined = 0,
        VertexBuffer = BIT(0),
        IndexBuffer = BIT(1),
        ConstantBuffer = BIT(2),
        StructuredBuffer = BIT(3),
        // Internal usage only!
        StagingBuffer = BIT(4),
    };
    ENUM_CLASS_FLAGS(BufferBindFlags);

    enum class BufferAllocMode
    {
        Default = 0,
        /// @brief Use memory shared with CPU & GPU
        SharedMemory,
        /// @brief Use only GPU visible memory
        GpuMemory,
    };
    ENUM_CLASS_FLAGS(BufferAllocMode);

    struct BufferData
    {
        u64 dataSize;
        u64 startOffsetInBuffer;
        const void* data = nullptr;
    };

    /*
    *   Texture
    */

	ENUM()
    enum class Dimension
    {
        Dim2D = 0,
        //! Exactly same as Dim2D but is reserved for CubeMaps.
        DimCUBE,
        Dim3D,
        Dim1D,
        Dim2DArray
    };

	ENUM()
    enum class Format
    {
        Undefined = 0,
        R8_UNORM,
        R8_SNORM,
        R8_SRGB,
        R8G8_UNORM,
        R8G8_SRGB,
        R8G8B8A8_SRGB,
        R8G8B8A8_UNORM,
        R8G8B8A8_SNORM,
        B8G8R8A8_SRGB,
        B8G8R8A8_UNORM,
        B8G8R8A8_SNORM,
        R8G8B8_UNORM,
        R8G8B8_SNORM,
        R8G8B8_SRGB,
        B8G8R8_UNORM,
        B8G8R8_SNORM,
        B8G8R8_SRGB,
        R5G6B5_UNORM,
        B5G6R5_UNORM,
        R16_UNORM,
        R16_SNORM,
        R16_SFLOAT,
		R16G16_UNORM,
		R16G16_SNORM,
		R16G16_UINT,
		R16G16_SINT,
		R16G16_SFLOAT,
        R16G16B16A16_SFLOAT,
        R32_UINT,
        R32_SINT,
        R32_SFLOAT,
		R32G32_UINT,
		R32G32_SINT,
		R32G32_SFLOAT,
        R32G32B32A32_SFLOAT,
        D16_UNORM_S8_UINT,
        D16_UNORM,
		D24_UNORM_S8_UINT,
        D32_SFLOAT_S8_UINT,
		D32_SFLOAT,
		// Compressed formats
        BC1_RGB_UNORM,
        BC1_RGB_SRGB,
        BC1_RGBA_UNORM,
        BC1_RGBA_SRGB,
        BC3_UNORM,
        BC3_SRGB,
        BC4_UNORM,
        BC5_UNORM,
        BC7_UNORM,
        BC7_SRGB,
        BC6H_UFLOAT,
        BC6H_SFLOAT,
    };
	ENUM_CLASS(Format);

    CORERHI_API u32 GetBitsPerPixelForFormat(RHI::Format format);
    CORERHI_API u32 GetBitDepthForFormat(RHI::Format format);
    CORERHI_API u32 GetNumChannelsForFormat(RHI::Format format);

	ENUM(Flags)
    enum class TextureBindFlags
    {
		None = 0,
		/// @brief Use as shader input.
		ShaderRead = BIT(0),
		/// @brief Use as shader output.
		ShaderWrite = BIT(1),
		/// @brief Use with shader read & write access.
		ShaderReadWrite = ShaderRead | ShaderWrite,
		/// @brief Use as color attachment.
		Color = BIT(2),
		/// @brief Use as depth stencil attachment.
		DepthStencil = BIT(3),
		/// @brief Use as depth-only attachment.
		Depth = BIT(4),
		/// @brief Use as a subpass input.
		SubpassInput = BIT(5),
    };
    ENUM_CLASS_FLAGS(TextureBindFlags);

	ENUM()
	enum class FilterMode
	{
		Linear = 0,
		Nearest = 1,
		Cubic = 2
	};
	ENUM_CLASS(FilterMode);

    ENUM()
	enum class FilterModeMask
	{
        None = 0,
		Linear = BIT((u32)FilterMode::Linear),
		Nearest = BIT((u32)FilterMode::Nearest),
		Cubic = BIT((u32)FilterMode::Cubic)
	};
	ENUM_CLASS_FLAGS(FilterModeMask);

    ENUM()
    enum class SamplerAddressMode
    {
        Repeat = 0,
        MirroredRepeat = 1,
        ClampToEdge = 2,
        ClampToBorder = 3,
    };
    ENUM_CLASS(SamplerAddressMode);

    ENUM()
    enum class SamplerBorderColor
    {
        FloatTransparentBlack = 0,
        IntTransparentBlack = 1,
        FloatOpaqueBlack = 2,
        IntOpaqueBlack = 3,
        FloatOpaqueWhite = 4,
        IntOpaqueWhite = 5,
    };
    ENUM_CLASS(SamplerBorderColor);

    struct CORERHI_API SamplerDescriptor
    {
        SamplerAddressMode addressModeU{};
        SamplerAddressMode addressModeV{};
        SamplerAddressMode addressModeW{};
		FilterMode samplerFilterMode{};
        SamplerBorderColor borderColor{};
        b8 enableAnisotropy = false;
        u8 maxAnisotropy = 16;

        SIZE_T GetHash() const;

        bool operator==(const SamplerDescriptor& rhs) const
        {
            return GetHash() == rhs.GetHash();
        }

        bool operator!=(const SamplerDescriptor& rhs) const
        {
            return !operator==(rhs);
        }
    };

    /*
    *   Command List
    */

	struct ViewportState
	{
		ViewportState() = default;

		ViewportState(float x, float y,
			float width, float height,
			float minDepth = 0.0f, float maxDepth = 1.0f) 
			: x(x), y(y) , width(width), height(height), minDepth(minDepth), maxDepth(maxDepth)
		{}

		float x = 0, y = 0;
		float width = 0, height = 0;
		float minDepth = 0.0f, maxDepth = 1.0f;
	};

	struct ScissorState
	{
		ScissorState() = default;

		ScissorState(float x, float y, float width, float height)
			: x(x), y(y), width(width), height(height)
		{}

		float x = 0, y = 0;
		float width = 0, height = 0;
	};

    /*
    *   Shader
    */

	class ShaderModule;

	ENUM()
    enum class ShaderStage
    {
        None = 0,
        Vertex = BIT(0),
        Fragment = BIT(1),
		Tessellation = BIT(2),
		Geometry = BIT(3),
        Compute = BIT(4),

		Default = Vertex | Fragment,
		All = Vertex | Fragment | Tessellation | Geometry,
    };
    ENUM_CLASS_FLAGS(ShaderStage);

	struct ShaderModuleDescriptor
	{
        Name debugName = nullptr;
		String name = "";
		ShaderStage stage = ShaderStage::None;
		void* byteCode = nullptr;
		SIZE_T byteSize = 0;
	};

	ENUM()
	enum class ShaderResourceType
	{
		None = 0,
		ConstantBuffer, // A uniform buffer in vulkan
		StructuredBuffer, // A storage buffer in vulkan
		RWStructuredBuffer, // A RW storage buffer in vulkan
		Texture1D, // A texture1D in vulkan
        Texture2D, // A texture2D in vulkan
        Texture2DArray, // A texture2D in vulkan
		Texture3D, // A texture3D in vulkan
		TextureCube, // A textureCube in vulkan
		RWTexture2D, // An image2D in vulkan
		RWTexture3D, // An image3D in vulkan
        RWTexture2DArray,
		SamplerState, // A sampler in vulkan
		SubpassInput // A SubpassInput in vulkan
	};
	ENUM_CLASS(ShaderResourceType);

    ENUM()
    enum class VertexInputAttribute : u32
    {
        None = 0,
        Position,
        Normal,
        Binormal,
        BlendWeight,
        BlendIndices,
        Tangent,
        Color,
        UV
    };
    ENUM_CLASS_FLAGS(VertexInputAttribute);

    CORERHI_API SIZE_T GetVertexInputTypeSize(VertexInputAttribute input);

    struct CORERHI_API ShaderSemantic
    {
        ShaderSemantic() = default;

        ShaderSemantic(VertexInputAttribute attribute, u8 index = 0) : attribute(attribute), index(index)
        {

        }

        static ShaderSemantic Parse(const String& name);

        String ToString() const;

        VertexInputAttribute attribute = VertexInputAttribute::None;
        u8 index = 0;

        inline bool operator==(const ShaderSemantic& rhs) const
        {
            return attribute == rhs.attribute && index == rhs.index;
        }

        inline bool operator!=(const ShaderSemantic& rhs) const
        {
            return !(*this == rhs);
        }
    };

	ENUM()
	enum class CullMode
	{
		None = 0,
        Off = None,
		Back,
		Front,
		All
	};
	ENUM_CLASS(CullMode);

    ENUM()
    enum class FillMode
    {
        Solid = 0,
        Wireframe
    };
    ENUM_CLASS(FillMode);

    ENUM(Flags)
    enum class ColorComponentMask
    {
        None = 0,
        R = BIT(0),
        G = BIT(1),
        B = BIT(2),
        A = BIT(3),

        RGB = R | G | B,
        RG = R | G,
        RB = R | B,
        GB = G | B,
        RGBA = R | G | B | A,
        All = R | G | B | A,
    };
    ENUM_CLASS(ColorComponentMask);

    enum class ResourceState
    {
        Undefined = 0,
        General = BIT(0),
        CopyDestination = BIT(1),
        CopySource = BIT(2),
        DepthWrite = BIT(3),
        DepthRead = BIT(4),
        // Read only resource
        FragmentShaderResource = BIT(5),
        // Read only resource
        NonFragmentShaderResource = BIT(6),
        ColorOutput = BIT(7),
        VertexBuffer = BIT(8),
        IndexBuffer = BIT(9),
        ConstantBuffer = BIT(10),
        Present = BIT(11),
        // Read/Write resource
        ShaderWrite = BIT(12),
        BlitSource = BIT(13),
        BlitDestination = BIT(14)
    };

} // namespace CE

#include "RHIDefinitions.rtti.h"
