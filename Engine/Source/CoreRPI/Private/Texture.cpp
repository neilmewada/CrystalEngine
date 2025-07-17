#include "CoreRPI.h"

namespace CE::RPI
{
    static u64 CalculateTotalTextureSize(u32 width, u32 height, u32 bitsPerPixel, u32 arrayCount = 1, u32 mipLevelCount = 1)
    {
        u64 size = 0;

        for (int mip = 0; mip < mipLevelCount; mip++)
        {
            u32 power = (u32)pow(2, mip);
            u64 curSize = width / power * height / power * bitsPerPixel / 8 * arrayCount;
            size += curSize;
        }

        return size;
    }
    
    Texture::Texture(const TextureDescriptor& desc)
    {
        width = desc.texture.width;
        height = desc.texture.height;
        depth = desc.texture.depth;

        texture = RHI::gDynamicRHI->CreateTexture(desc.texture);

		RHI::TextureViewDescriptor viewDesc{};
		viewDesc.arrayLayerCount = desc.texture.arrayLayers;
		viewDesc.mipLevelCount = desc.texture.mipLevels;
        viewDesc.baseMipLevel = 0;
		viewDesc.baseArrayLayer = 0;
		viewDesc.dimension = desc.texture.dimension;
		viewDesc.format = desc.texture.format;
		viewDesc.texture = texture;

		//textureView = RHI::gDynamicRHI->CreateTextureView(viewDesc);

        samplerState = RPISystem::Get().FindOrCreateSampler(desc.samplerDesc);

        if (desc.source == nullptr)
        {
            return;
        }

        UploadData(desc.source->GetDataPtr(), desc.source->GetDataSize());
    }

    Texture::Texture(RHI::Texture* texture, const RHI::SamplerDescriptor& samplerDesc)
        : texture(texture)
    {
        if (texture)
	    {
		    width = texture->GetWidth();
	    	height = texture->GetHeight();
	    	depth = texture->GetDepth();
	    }

        samplerState = RPISystem::Get().FindOrCreateSampler(samplerDesc);
    }

    Texture::Texture(RHI::TextureView* textureView, const RHI::SamplerDescriptor& samplerDesc)
        : texture(nullptr)
        , textureView(textureView)
    {
        if (textureView)
        {
            width = textureView->GetTexture()->GetWidth();
            height = textureView->GetTexture()->GetHeight();
            depth = textureView->GetTexture()->GetDepth();
        }

        samplerState = RPISystem::Get().FindOrCreateSampler(samplerDesc);
    }

    Texture::Texture(const CMImage& sourceImage, const RHI::SamplerDescriptor& samplerDesc)
    {
        if (!sourceImage.IsValid())
            return;

        u8* data = (u8*)sourceImage.GetDataPtr();
        u32 dataSize = sourceImage.GetDataSize();

        RHI::TextureDescriptor desc{};
        desc.name = "CMImage";
        desc.bindFlags = RHI::TextureBindFlags::ShaderRead;
        desc.mipLevels = 1;
        desc.arrayLayers = 1;
        desc.width = sourceImage.GetWidth();
        desc.height = sourceImage.GetHeight();
        desc.depth = 1;
        desc.dimension = Dimension::Dim2D;
        desc.sampleCount = 1;

        width = desc.width;
        height = desc.height;

        switch (sourceImage.GetFormat())
        {
        case CMImageFormat::R8:
            desc.format = Format::R8_UNORM;
            break;
        case CMImageFormat::RG8:
            desc.format = Format::R8G8_UNORM;
            break;
        case CMImageFormat::RGB8:
            desc.format = Format::R8G8B8_UNORM;
            break;
        case CMImageFormat::RGBA8:
            desc.format = Format::R8G8B8A8_UNORM;
            break;
        case CMImageFormat::R16:
            desc.format = Format::R16_SFLOAT;
            break;
        case CMImageFormat::RG16:
            desc.format = Format::R16G16_SFLOAT;
            break;
        case CMImageFormat::RGBA16:
            desc.format = Format::R16G16B16A16_SFLOAT;
            break;
        default:
            return;
        }

        texture = RHI::gDynamicRHI->CreateTexture(desc);

        samplerState = RPISystem::Get().FindOrCreateSampler(samplerDesc);

        UploadData(data, dataSize);
    }

    Texture::Texture(const Array<CMImage>& sourceImageMips, const RHI::SamplerDescriptor& samplerDesc)
    {
        if (sourceImageMips.IsEmpty())
            return;

        RHI::TextureDescriptor desc{};
        desc.name = "CMImage";
        desc.bindFlags = RHI::TextureBindFlags::ShaderRead;
        desc.mipLevels = sourceImageMips.GetSize();
        desc.arrayLayers = 1;
        desc.width = sourceImageMips[0].GetWidth();
        desc.height = sourceImageMips[0].GetHeight();
        desc.depth = 1;
        desc.dimension = Dimension::Dim2D;
        desc.sampleCount = 1;

        width = desc.width;
        height = desc.height;

        switch (sourceImageMips[0].GetFormat())
        {
        case CMImageFormat::R8:
            desc.format = Format::R8_UNORM;
            break;
        case CMImageFormat::RG8:
            desc.format = Format::R8G8_UNORM;
            break;
        case CMImageFormat::RGB8:
            desc.format = Format::R8G8B8_UNORM;
            break;
        case CMImageFormat::RGBA8:
            desc.format = Format::R8G8B8A8_UNORM;
            break;
        case CMImageFormat::R16:
            desc.format = Format::R16_SFLOAT;
            break;
        case CMImageFormat::RG16:
            desc.format = Format::R16G16_SFLOAT;
            break;
        case CMImageFormat::RGBA16:
            desc.format = Format::R16G16B16A16_SFLOAT;
            break;
        default:
            return;
        }

        texture = gDynamicRHI->CreateTexture(desc);

        samplerState = RPISystem::Get().FindOrCreateSampler(samplerDesc);

        // - Upload Data -

        {
            int totalMipLevels = texture->GetMipLevelCount();

            RHI::CommandQueue* queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
            auto commandList = RHI::gDynamicRHI->AllocateCommandList(queue);
            auto uploadFence = RHI::gDynamicRHI->CreateFence(false);

            RHI::BufferDescriptor stagingDesc{};
            stagingDesc.name = "Staging Buffer";
            stagingDesc.bindFlags = RHI::BufferBindFlags::StagingBuffer;
            stagingDesc.bufferSize = CalculateTotalTextureSize(desc.width, desc.height, sourceImageMips[0].GetBitsPerPixel(), 1, sourceImageMips.GetSize());
            stagingDesc.defaultHeapType = RHI::MemoryHeapType::Upload;

            RHI::Buffer* stagingBuffer = RHI::gDynamicRHI->CreateBuffer(stagingDesc);

            void* data;
            stagingBuffer->Map(0, stagingBuffer->GetBufferSize(), &data);
            {
                u8* mipPtr = (u8*)data;
                u32 curWidth = sourceImageMips[0].GetWidth();
                u32 curHeight = sourceImageMips[0].GetHeight();

	            for (int i = 0; i < sourceImageMips.GetSize(); ++i)
	            {
                    SIZE_T mipByteSize = curWidth * curHeight * sourceImageMips[i].GetBitsPerPixel() / 8;

                    memcpy(mipPtr, sourceImageMips[i].GetDataPtr(), mipByteSize);

                    mipPtr += mipByteSize;
                    curWidth /= 2;
                    curHeight /= 2;
	            }
            }
            stagingBuffer->Unmap();

            commandList->Begin();
            {
                ResourceBarrierDescriptor barrier{};
                barrier.resource = stagingBuffer;
                barrier.fromState = ResourceState::Undefined;
                barrier.toState = ResourceState::CopySource;
                commandList->ResourceBarrier(1, &barrier);

                barrier.resource = texture;
                barrier.fromState = ResourceState::Undefined;
                barrier.toState = ResourceState::CopyDestination;
                commandList->ResourceBarrier(1, &barrier);

                u64 bufferOffset = 0;

                for (int mip = 0; mip < totalMipLevels; mip++)
                {
                    RHI::BufferToTextureCopy copyRegion{};
                    copyRegion.srcBuffer = stagingBuffer;
                    copyRegion.bufferOffset = bufferOffset;
                    copyRegion.dstTexture = texture;
                    copyRegion.baseArrayLayer = 0;
                    copyRegion.layerCount = (u16)texture->GetArrayLayerCount();
                    copyRegion.mipSlice = (u16)mip;

                    commandList->CopyTextureRegion(copyRegion);
                    u32 power = (u32)pow(2, mip);

                    bufferOffset += (u64)texture->GetWidth() / power * (u64)texture->GetHeight() / power * (u64)texture->GetBitsPerPixel() / 8 * copyRegion.layerCount;
                }

                barrier.resource = texture;
                barrier.fromState = ResourceState::CopyDestination;
                barrier.toState = ResourceState::FragmentShaderResource;
                commandList->ResourceBarrier(1, &barrier);
            }
            commandList->End();

            queue->Execute(1, &commandList, uploadFence);
            uploadFence->WaitForFence();

            delete stagingBuffer;
            RHI::gDynamicRHI->FreeCommandLists(1, &commandList);
            delete uploadFence;
        }

    }

    Texture::~Texture()
    {
        if (texture)
        {
            RPISystem::Get().QueueDestroy(texture);
            texture = nullptr;
        }

        if (textureView)
        {
            RPISystem::Get().QueueDestroy(textureView);
            textureView = nullptr;
        }

        samplerState = nullptr;
    }

    RHI::TextureView* Texture::GetOrCreateTextureView()
    {
        if (textureView)
            return textureView;
        if (!texture)
            return nullptr;

        RHI::TextureViewDescriptor viewDesc{};
        viewDesc.texture = texture;
        viewDesc.arrayLayerCount = texture->GetArrayLayerCount();
        viewDesc.mipLevelCount = texture->GetMipLevelCount();
        viewDesc.baseArrayLayer = 0;
        viewDesc.baseMipLevel = 0;
        viewDesc.dimension = texture->GetDimension();
        viewDesc.format = texture->GetFormat();

        textureView = RHI::gDynamicRHI->CreateTextureView(viewDesc);
        return textureView;
    }

    void Texture::UploadData(u8* src, u64 dataSize)
    {
        if (src == nullptr || dataSize == 0)
        {
            return;
        }

        int totalMipLevels = texture->GetMipLevelCount();

        RHI::CommandQueue* queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
        auto commandList = RHI::gDynamicRHI->AllocateCommandList(queue);
        auto uploadFence = RHI::gDynamicRHI->CreateFence(false);

        RHI::BufferDescriptor stagingDesc{};
        stagingDesc.name = "Staging Buffer";
        stagingDesc.bindFlags = RHI::BufferBindFlags::StagingBuffer;
        stagingDesc.bufferSize = dataSize;
        stagingDesc.defaultHeapType = RHI::MemoryHeapType::Upload;

        RHI::Buffer* stagingBuffer = RHI::gDynamicRHI->CreateBuffer(stagingDesc);
        stagingBuffer->UploadData(src, dataSize);

        commandList->Begin();
        {
            ResourceBarrierDescriptor barrier{};
            barrier.resource = stagingBuffer;
            barrier.fromState = ResourceState::Undefined;
            barrier.toState = ResourceState::CopySource;
            commandList->ResourceBarrier(1, &barrier);

            barrier.resource = texture;
            barrier.fromState = ResourceState::Undefined;
            barrier.toState = ResourceState::CopyDestination;
            commandList->ResourceBarrier(1, &barrier);

            u64 bufferOffset = 0;

            for (int mip = 0; mip < totalMipLevels; mip++)
            {
                RHI::BufferToTextureCopy copyRegion{};
                copyRegion.srcBuffer = stagingBuffer;
                copyRegion.bufferOffset = bufferOffset;
                copyRegion.dstTexture = texture;
                copyRegion.baseArrayLayer = 0;
                copyRegion.layerCount = texture->GetArrayLayerCount();
                copyRegion.mipSlice = mip;

                commandList->CopyTextureRegion(copyRegion);
                u32 power = (u32)pow(2, mip);
                
                bufferOffset += texture->GetWidth() / power * texture->GetHeight() / power * texture->GetBitsPerPixel() / 8 * copyRegion.layerCount;
            }

            barrier.resource = texture;
            barrier.fromState = ResourceState::CopyDestination;
            barrier.toState = ResourceState::FragmentShaderResource;
            commandList->ResourceBarrier(1, &barrier);
        }
        commandList->End();

        queue->Execute(1, &commandList, uploadFence);
        uploadFence->WaitForFence();

        delete stagingBuffer;
        RHI::gDynamicRHI->FreeCommandLists(1, &commandList);
        delete uploadFence;
    }

    void Texture::TransitionResourceTo(RHI::ResourceState fromState, RHI::ResourceState toState)
    {
		RHI::CommandQueue* queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
        RHI::CommandList* cmdList = RHI::gDynamicRHI->AllocateCommandList(queue, CommandListType::Direct);
		RHI::Fence* fence = RHI::gDynamicRHI->CreateFence(false);

        cmdList->Begin();
		{
            RHI::ResourceBarrierDescriptor barrier;
            barrier.resource = texture;
            barrier.fromState = fromState;
            barrier.toState = toState;
            barrier.subresourceRange = RHI::SubresourceRange::All();

            cmdList->ResourceBarrier(1, &barrier);
		}
        cmdList->End();

        queue->Execute(1, &cmdList, fence);
        fence->WaitForFence();

    	RHI::gDynamicRHI->FreeCommandLists(1, &cmdList);
        delete fence;
    }
} // namespace CE::RPI
