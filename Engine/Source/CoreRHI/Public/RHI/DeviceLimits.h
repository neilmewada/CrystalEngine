#pragma once

namespace CE::RHI
{

    class DeviceLimits : RHI::RHIResource
    {
    protected:
        DeviceLimits() : RHI::RHIResource(ResourceType::DeviceLimits) {}
        virtual ~DeviceLimits() = default;

    public:
        
        bool IsFormatSupported(RHI::Format format)
        {
            return imageFormatSupport.KeyExists(format) && imageFormatSupport[format].bindFlags != RHI::TextureBindFlags::None;
        }

        bool IsFormatSupported(RHI::Format format, RHI::TextureBindFlags bindFlags)
        {
            return imageFormatSupport.KeyExists(format) && ((imageFormatSupport[format].bindFlags & bindFlags) == bindFlags);
        }

        inline bool IsFormatSupported(RHI::Format format, RHI::TextureBindFlags bindFlags, RHI::FilterMode filterMode)
        {
            return IsFormatSupported(format, bindFlags) && (imageFormatSupport[format].filterMask & (1 << (u32)filterMode)) != 0;
        }

		inline bool IsRayTracingSupported() const { return isRayTracingSupported; }

        inline u64 GetMaxConstantBufferRange() const { return maxConstantBufferRange; }
        inline u64 GetMaxStructuredBufferRange() const { return maxStructuredBufferRange; }
        
        inline u32 GetMaxRootConstantByteSize() const { return maxRootConstantByteSize; }

        bool IsSparseBindingSupported() const { return sparseBinding; }

        bool IsSparseTexture2DResidencySupported(u32 sampleCount)
        {
	        switch (sampleCount)
	        {
	        case 2:
                return sparseResidency2DSample2;
            case 4:
                return sparseResidency2DSample4;
            case 8:
                return sparseResidency2DSample8;
            case 16:
                return sparseResidency2DSample16;
            default:
                return sparseResidency2D;
	        }
        }

        bool IsSparseTexture3DResidencySupported()
        {
            return sparseResidency3D;
        }

    protected:

        struct FormatEntry
        {
            RHI::TextureBindFlags bindFlags{};
            RHI::FilterModeMask filterMask{};
        };

        HashMap<RHI::Format, FormatEntry> imageFormatSupport{};

        u32 maxConstantBufferRange = 0;
        u32 maxStructuredBufferRange = 0;
        
        u32 maxRootConstantByteSize = 256;

        bool sparseBinding = false;
        bool sparseResidency2D = false;
        bool sparseResidency2DSample2 = false;
        bool sparseResidency2DSample4 = false;
        bool sparseResidency2DSample8 = false;
        bool sparseResidency2DSample16 = false;

        bool sparseResidency3D = false;

        bool isRayTracingSupported = false;
    };
    
} // namespace CE::RHI
