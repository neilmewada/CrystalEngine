#include "FusionCore.h"

namespace CE
{

    FImageAtlas::FImageAtlas()
    {

    }

    FImageAtlas::~FImageAtlas()
    {
        
    }

    Vec2 FImageAtlas::GetWhitePixelUV() const
    {
        return whitePixel.uvMin + (whitePixel.uvMax - whitePixel.uvMin) * 0.5f;
    }

    Vec2 FImageAtlas::GetTransparentPixelUV() const
    {
        return transparentPixel.uvMin + (transparentPixel.uvMax - transparentPixel.uvMin) * 0.5f;
    }

    FImageAtlas::ImageItem FImageAtlas::FindImage(const Name& imageName)
    {
        if (!imagesByName.KeyExists(imageName))
            return {};

        return imagesByName[imageName];
    }

    static RPI::TextureDescriptor MakeAtlasTextureDescriptor(u32 atlasSize, int frameIndex)
    {
        RPI::TextureDescriptor desc{};
        desc.texture.name = String::Format("Fusion Atlas Layer {}", frameIndex);
        desc.texture.width = desc.texture.height = atlasSize;
        desc.texture.sampleCount = 1;
        desc.texture.depth = 1;
        desc.texture.dimension = Dimension::Dim2D;
        desc.texture.arrayLayers = 1;
        desc.texture.mipLevels = 1;
        desc.texture.bindFlags = TextureBindFlags::ShaderRead;
        desc.texture.format = Format::R8G8B8A8_UNORM;
        desc.texture.defaultHeapType = MemoryHeapType::Upload;

        desc.samplerDesc.addressModeU = SamplerAddressMode::ClampToBorder;
        desc.samplerDesc.addressModeV = SamplerAddressMode::ClampToBorder;
        desc.samplerDesc.addressModeW = SamplerAddressMode::ClampToBorder;
        desc.samplerDesc.borderColor = SamplerBorderColor::FloatTransparentBlack;
        desc.samplerDesc.enableAnisotropy = false;
        desc.samplerDesc.samplerFilterMode = FilterMode::Linear;
        return desc;
    }

    int FImageAtlas::RegisterAtlasLayer(FAtlasImage* atlas, int layerIndex)
    {
        auto renderService = FApplication::Get()->GetService<FRenderService>();

        StaticArray<RHI::Texture*, RHI::Limits::MaxSwapChainImageCount> rhiTextures{};

        for (int i = 0; i < (int)atlas->textures.GetSize(); ++i)
        {
            atlas->textures[i] = new RPI::Texture(MakeAtlasTextureDescriptor(atlasSize, i));
            rhiTextures[i] = atlas->textures[i]->GetRhiTexture();
        }

        for (int i = 0; i < atlas->dirty.GetSize(); i++)
        {
            atlas->dirty[i] = true;
        }
        
        return renderService->RegisterTexture(rhiTextures);
    }

    void FImageAtlas::Init()
    {
        ZoneScoped;

        if (atlasLayers.NotEmpty())
            return;

        auto renderService = FApplication::Get()->GetService<FRenderService>();

        Ptr<FAtlasImage> atlas = new FAtlasImage(atlasSize);
        atlas->layerIndex = 0;

        atlasLayers.Add(atlas);

        pages.Add(String::Format("Page {}", pages.GetSize()));

        atlas->textureSlot = RegisterAtlasLayer(atlas.Get(), 0);

        UpdateImageAtlasItems();

        RHI::BufferDescriptor stagingDesc{};
        stagingDesc.name = "Staging Buffer";
        stagingDesc.bindFlags = RHI::BufferBindFlags::StagingBuffer;
        stagingDesc.bufferSize = atlasSize * atlasSize * sizeof(u32) * atlasLayers.GetSize();
        stagingDesc.defaultHeapType = RHI::MemoryHeapType::Upload;

        stagingBuffer = RHI::gDynamicRHI->CreateBuffer(stagingDesc);
        stagingBufferFence = RHI::gDynamicRHI->CreateFence(true);

        stagingCommandQueue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
        stagingCommandList = RHI::gDynamicRHI->AllocateCommandList(stagingCommandQueue);

	    {
		    u8 pixels[16];
        	for (int i = 0; i < COUNTOF(pixels); ++i)
        	{
        		pixels[i] = NumericLimits<u8>::Max();
        	}
        	CAImage image = CAImage::LoadRawImageFromMemory(pixels, 4, 4, CMImageFormat::R8, CMImageSourceFormat::None, 8, 8);
        	whitePixel = AddImage("__WhitePixel", image);
	    }

	    {
		    u32 pixels[16];
        	for (int i = 0; i < COUNTOF(pixels); ++i)
        	{
        		pixels[i] = 0;
        	}
            CAImage image = CAImage::LoadRawImageFromMemory((u8*)pixels, 4, 4, CMImageFormat::RGBA8, CMImageSourceFormat::None, 8, 8 * 4);
            transparentPixel = AddImage("__TransparentPixel", image);
	    }
    }

    void FImageAtlas::Shutdown()
    {
        auto renderService = FApplication::Get()->GetService<FRenderService>();

        RHI::gDynamicRHI->DestroyBuffer(stagingBuffer);
        stagingBuffer = nullptr;

        RHI::gDynamicRHI->DestroyFence(stagingBufferFence);
        stagingBufferFence = nullptr;

        RHI::gDynamicRHI->FreeCommandLists(1, &stagingCommandList);
        stagingCommandList = nullptr;
        stagingCommandQueue = nullptr;

        for (Ptr<FAtlasImage>& layer : atlasLayers)
        {
            if (layer->textureSlot >= 0)
            {
                renderService->DeregisterTexture(layer->textureSlot);
                layer->textureSlot = -1;
            }

            for (int i = 0; i < (int)layer->textures.GetSize(); ++i)
            {
                delete layer->textures[i];
                layer->textures[i] = nullptr;
            }
        }

    	atlasLayers.Clear();
        imagesByName.Clear();
    }

    void FImageAtlas::UpdateImageAtlasItems()
    {
	    for (int i = 0; i < atlasLayers.GetSize(); ++i)
	    {
            ImageItem item{};
            item.layerIndex = i;
            item.textureSlot = atlasLayers[i]->textureSlot;
            item.uvMin = Vec2(0, 0);
            item.uvMax = Vec2(1, 1);
            item.width = atlasSize;
            item.height = atlasSize;
            imagesByName[String::Format("__ImageAtlas_{}", i)] = item;
	    }
    }

    void FImageAtlas::Flush(u32 frameIndex)
    {
        ZoneScoped;

        // Collect layers dirty for this frame
        Array<FAtlasImage*> dirtyLayers;
        for (Ptr<FAtlasImage>& layer : atlasLayers)
        {
            if (layer->dirty[frameIndex])
                dirtyLayers.Add(layer.Get());
        }

        if (dirtyLayers.IsEmpty())
            return;

        auto commandList = stagingCommandList;
        commandList->Begin();

        // Staging buffer: transition once to CopySource
        {
            ResourceBarrierDescriptor barrier{};
            barrier.resource = stagingBuffer;
            barrier.fromState = ResourceState::Undefined;
            barrier.toState = ResourceState::CopySource;
            commandList->ResourceBarrier(1, &barrier);
        }

        // Batch all dirty-layer destination barriers, then copies, then shader-read barriers
        for (FAtlasImage* layer : dirtyLayers)
        {
            RHI::Texture* dst = layer->textures[frameIndex]->GetRhiTexture();

            ResourceBarrierDescriptor barrier{};
            barrier.resource = dst;
            barrier.fromState = ResourceState::Undefined;
            barrier.toState = ResourceState::CopyDestination;
            commandList->ResourceBarrier(1, &barrier);
        }

        for (FAtlasImage* layer : dirtyLayers)
        {
            RHI::Texture* dst = layer->textures[frameIndex]->GetRhiTexture();

            RHI::BufferToTextureCopy copy{};
            copy.dstTexture = dst;
            copy.mipSlice = 0;
            copy.baseArrayLayer = 0;
            copy.layerCount = 1;
            copy.srcBuffer = stagingBuffer;
            copy.bufferOffset = (u64)layer->layerIndex * atlasSize * atlasSize * sizeof(u32);
            commandList->CopyTextureRegion(copy);
        }

        for (FAtlasImage* layer : dirtyLayers)
        {
            RHI::Texture* dst = layer->textures[frameIndex]->GetRhiTexture();

            ResourceBarrierDescriptor barrier{};
            barrier.resource = dst;
            barrier.fromState = ResourceState::CopyDestination;
            barrier.toState = ResourceState::FragmentShaderResource;
            commandList->ResourceBarrier(1, &barrier);
        }

        commandList->End();

        RHI::CommandQueueSubmission submission{};
        submission.numCommandLists = 1;
        submission.commandLists = &commandList;
        submission.signalFence = stagingBufferFence;
        submission.signalFenceValue = stagingBufferFence->NextSignalValue();

        stagingCommandQueue->Submit(submission);
        stagingBufferFence->WaitCPU(submission.signalFenceValue);

        for (FAtlasImage* layer : dirtyLayers)
        {
            layer->dirty[frameIndex] = false;
        }
    }

    FImageAtlas::ImageItem FImageAtlas::AddImage(const Name& name, const CAImage& imageSource)
    {
        if (!name.IsValid() || !imageSource.IsValid() || imagesByName.KeyExists(name))
        {
            return {};
        }

        switch (imageSource.GetFormat())
        {
        case CMImageFormat::Undefined:
        case CMImageFormat::R16:
        case CMImageFormat::RG16:
        case CMImageFormat::RGB16:
        case CMImageFormat::RGBA16:
        case CMImageFormat::RGB565:
        case CMImageFormat::BC1:
        case CMImageFormat::BC3:
        case CMImageFormat::BC4:
        case CMImageFormat::BC5:
        case CMImageFormat::BC6H:
        case CMImageFormat::BC7:
            return {};
        }

        Vec2i textureSize = Vec2i(imageSource.GetWidth(), imageSource.GetHeight());
        u32 textureArea = textureSize.width * textureSize.height;

        Ptr<FAtlasImage> foundAtlas = nullptr;
        Ptr<BinaryNode> insertNode = nullptr;

        for (int i = 0; i < atlasLayers.GetSize(); ++i)
        {
            Ptr<FAtlasImage> atlas = atlasLayers[i];

            insertNode = atlas->root->Insert(textureSize);

            if (insertNode == nullptr && atlas->root->GetFreeArea() > textureArea * 2)
            {
                atlas->root->Defragment();
                
                insertNode = atlas->root->Insert(textureSize);
            }

            if (insertNode != nullptr)
            {
                foundAtlas = atlas;
	            break;
            }
        }

        if (insertNode == nullptr) // Need to create a new atlas layer
        {
            Ptr<FAtlasImage> atlas = new FAtlasImage(atlasSize);
            atlas->layerIndex = atlasLayers.GetSize();
            atlasLayers.Add(atlas);

            pages.Add(String::Format("Page {}", pages.GetSize()));

            RHI::BufferDescriptor stagingDesc{};
            stagingDesc.name = "Staging Buffer";
            stagingDesc.bindFlags = RHI::BufferBindFlags::StagingBuffer;
            stagingDesc.bufferSize = atlasSize * atlasSize * sizeof(u32) * atlasLayers.GetSize();
            stagingDesc.defaultHeapType = RHI::MemoryHeapType::Upload;

            RHI::Buffer* newStagingBuffer = RHI::gDynamicRHI->CreateBuffer(stagingDesc);

            void* data;
            stagingBuffer->Map(0, stagingBuffer->GetBufferSize(), &data);
            {
                newStagingBuffer->UploadData(data, stagingBuffer->GetBufferSize());
            }
            stagingBuffer->Unmap();

            delete stagingBuffer;
            stagingBuffer = newStagingBuffer;

            atlas->textureSlot = RegisterAtlasLayer(atlas.Get(), atlas->layerIndex);

            UpdateImageAtlasItems();

            insertNode = atlas->root->Insert(textureSize);

            if (insertNode == nullptr)
            {
                return {};
            }

            foundAtlas = atlas;
        }

        insertNode->imageName = name;

        foundAtlas->root->usedArea += textureArea;
        foundAtlas->nodesByImageName[name] = insertNode;

        int posX = Math::RoundToInt(insertNode->rect.min.x);
        int posY = Math::RoundToInt(insertNode->rect.min.y);

        Vec2 uvMin = Vec2((f32)posX / (f32)foundAtlas->atlasSize, (f32)posY / (f32)foundAtlas->atlasSize);
        Vec2 uvMax = Vec2((f32)(posX + textureSize.width) / (f32)foundAtlas->atlasSize, (f32)(posY + textureSize.height) / (f32)foundAtlas->atlasSize);

        void* stagingPtr;
        stagingBuffer->Map((u32)foundAtlas->layerIndex * atlasSize * atlasSize * sizeof(u32),
            atlasSize * atlasSize * sizeof(u32), &stagingPtr);

        for (int y = 0; y < textureSize.y; ++y)
        {
            for (int x = 0; x < textureSize.x; ++x)
	        {
                Vec2i dstPos = Vec2i(posX + x, posY + y);
                
                u8* dstPixel = (u8*)stagingPtr + (foundAtlas->atlasSize * dstPos.y + dstPos.x) * sizeof(u32);
                u8* srcPixel = (u8*)imageSource.GetDataPtr() + (SIZE_T)(textureSize.x * y + x) * (SIZE_T)imageSource.GetBitsPerPixel() / 8;
                u8* r = dstPixel;
                u8* g = dstPixel + 1;
                u8* b = dstPixel + 2;
                u8* a = dstPixel + 3;
                
                switch (imageSource.GetFormat())
                {
                case CMImageFormat::Undefined:
	                break;
                case CMImageFormat::R8:
                    *r = *g = *b = *srcPixel;
                    *a = (u8)255;
	                break;
                case CMImageFormat::RG8:
                    *r = *g = *b = *srcPixel;
                    *a = *(srcPixel + 1);
	                break;
                case CMImageFormat::RGB8:
                    *r = *srcPixel;
                    *g = *(srcPixel + 1);
                    *b = *(srcPixel + 2);
                    *a = (u8)255;
	                break;
                case CMImageFormat::RGBA8:
                    *r = *srcPixel;
                    *g = *(srcPixel + 1);
                    *b = *(srcPixel + 2);
                    *a = *(srcPixel + 3);
	                break;
                case CMImageFormat::R32:
                    *r = *g = *b = static_cast<u8>(*((f32*)srcPixel) * 255.0f);
                    *a = (u8)255;
	                break;
                case CMImageFormat::RG32:
                    *r = *g = *b = static_cast<u8>(*((f32*)srcPixel) * 255.0f);
                    *a = static_cast<u8>(*((f32*)srcPixel + 1) * 255.0f);
	                break;
                case CMImageFormat::RGB32:
                    *r = static_cast<u8>(*((f32*)srcPixel) * 255.0f);
                    *g = static_cast<u8>(*((f32*)srcPixel + 1) * 255.0f);
                    *b = static_cast<u8>(*((f32*)srcPixel + 2) * 255.0f);
                    *a = (u8)255;
	                break;
                case CMImageFormat::RGBA32:
                    *r = static_cast<u8>(*((f32*)srcPixel) * 255.0f);
                    *g = static_cast<u8>(*((f32*)srcPixel + 1) * 255.0f);
                    *b = static_cast<u8>(*((f32*)srcPixel + 2) * 255.0f);
                    *a = static_cast<u8>(*((f32*)srcPixel + 3) * 255.0f);
	                break;
                case CMImageFormat::R16:
	                break;
                case CMImageFormat::RG16:
	                break;
                case CMImageFormat::RGB16:
	                break;
                case CMImageFormat::RGBA16:
	                break;
                }
	        }
        }

        stagingBuffer->Unmap();

        ImageItem item{};
        item.layerIndex = foundAtlas->layerIndex;
        item.textureSlot = foundAtlas->textureSlot;
        item.uvMin = uvMin;
        item.uvMax = uvMax;
        item.width = textureSize.width;
        item.height = textureSize.height;

        imagesByName[name] = item;

        for (int i = 0; i < foundAtlas->dirty.GetSize(); i++)
        {
            foundAtlas->dirty[i] = true;
        }

        return item;
    }

    bool FImageAtlas::RemoveImage(const Name& name)
    {
        if (!imagesByName.KeyExists(name))
            return false;

        int layerIndex = imagesByName[name].layerIndex;
        if (layerIndex < 0 || layerIndex >= atlasLayers.GetSize())
            return false;

        Ptr<FAtlasImage> atlas = atlasLayers[layerIndex];

        if (!atlas->nodesByImageName.KeyExists(name))
            return false;

        Ptr<BinaryNode> node = atlas->nodesByImageName[name];
        if (node == nullptr)
            return false;

        // Clean up the image pixels from the atlas (not necessary)
        {
            void* stagingPtr;
            stagingBuffer->Map((u32)atlas->layerIndex * atlasSize * atlasSize * sizeof(u32),
                atlasSize * atlasSize * sizeof(u32), &stagingPtr);

            int posX = node->rect.min.x;
            int posY = node->rect.min.y;
            Vec2i textureSize = node->GetSize();

            for (int x = 0; x < textureSize.x; ++x)
            {
                for (int y = 0; y < textureSize.y; ++y)
                {
                    Vec2i dstPos = Vec2i(posX + x, posY + y);

                    u8* dstPixel = (u8*)stagingPtr + (atlas->atlasSize * dstPos.y + dstPos.x) * sizeof(u32);
                    memset(dstPixel, 0, sizeof(u32));
                }
            }

            stagingBuffer->Unmap();
        }

        atlas->root->usedArea -= node->rect.GetAreaInt();
        node->ClearImage();
        atlas->root->DefragmentFast();

        for (int i = 0; i < atlas->dirty.GetSize(); i++)
        {
            atlas->dirty[i] = true;
        }

        atlas->nodesByImageName.Remove(name);
        imagesByName.Remove(name);

        return true;
    }

    void FImageAtlas::BinaryNode::ClearImage()
    {
        imageName = Name();
        imageId = -1;
    }

    Ptr<FImageAtlas::BinaryNode> FImageAtlas::BinaryNode::FindUsedNode()
    {
        if (IsValid())
            return this;

        if (child[0] != nullptr)
        {
            Ptr<BinaryNode> node = child[0]->FindUsedNode();
            if (node != nullptr)
            {
                return node;
            }
        }

        if (child[1] != nullptr)
        {
            Ptr<BinaryNode> node = child[1]->FindUsedNode();
            if (node != nullptr)
            {
                return node;
            }
        }

        return nullptr;
    }

    Ptr<FImageAtlas::BinaryNode> FImageAtlas::BinaryNode::Insert(Vec2i imageSize)
    {
        using BinaryNode = FImageAtlas::BinaryNode;

        if (child[0] != nullptr)
        {
            // We are not in leaf node
            Ptr<BinaryNode> newNode = child[0]->Insert(imageSize);
            if (newNode != nullptr)
            {
                totalChildren++;
                return newNode;
            }
            if (child[1] == nullptr)
            {
                return nullptr;
            }

            newNode = child[1]->Insert(imageSize);
            if (newNode != nullptr)
            {
                totalChildren++;
            }

            return newNode;
        }
        else // We are in leaf node
        {
            if (IsValid()) // Do not split a valid node
                return nullptr;

            if (GetSize().width < imageSize.width ||
                GetSize().height < imageSize.height)
            {
                return nullptr;
            }

            if (GetSize().width == imageSize.width && GetSize().height == imageSize.height)
            {
                return this;
            }

            // Split the node
            child[0] = new BinaryNode;
            child[1] = new BinaryNode;
            child[0]->parent = this;
            child[1]->parent = this;

            totalChildren = 2;

            int dw = GetSize().width - imageSize.width;
            int dh = GetSize().height - imageSize.height;

            if (dw > dh)
            {
                child[0]->rect = Rect(rect.left, rect.top,
                    rect.left + imageSize.width, rect.bottom);
                child[1]->rect = Rect(rect.left + imageSize.width + 1, rect.top,
                    rect.right, rect.bottom);
            }
            else
            {
                child[0]->rect = Rect(rect.left, rect.top,
                    rect.right, rect.top + imageSize.height);
                child[1]->rect = Rect(rect.left, rect.top + imageSize.height + 1,
                    rect.right, rect.bottom);
            }

            return child[0]->Insert(imageSize);
        }
    }

    bool FImageAtlas::BinaryNode::DefragmentFast()
    {
        if (child[0] != nullptr && child[1] != nullptr)
        {
            bool leftValid = child[0]->DefragmentFast();
            bool rightValid = child[1]->DefragmentFast();

            if (!leftValid && !rightValid)
            {
                child[0] = nullptr;
                child[1] = nullptr;

                return false;
            }

            // Perform basic defragmentation:
            // Merge two empty consecutive columns OR rows into a single one!
            // This can happen if the 2nd consecutive column/row is the 1st child of 2nd node.
            {
                if (IsWidthSpan() && !leftValid && child[1]->child[0] != nullptr &&
                    child[1]->IsWidthSpan() && !child[1]->child[0]->IsValidRecursive())
                {
                    Ptr<BinaryNode> nodeToMove = child[1]->child[1];
                    child[0]->rect.max.x = child[1]->child[0]->rect.max.x;
                    child[1] = nodeToMove;
                    nodeToMove->parent = this;
                }
                else if (IsHeightSpan() && !leftValid && child[1]->child[0] != nullptr &&
                    child[1]->IsHeightSpan() && !child[1]->child[0]->IsValidRecursive())
                {
                    Ptr<BinaryNode> nodeToMove = child[1]->child[1];
                    child[0]->rect.max.y = child[1]->child[0]->rect.max.y;
                    child[1] = nodeToMove;
                    nodeToMove->parent = this;
                }
            }

            return true;
        }

        return IsValid();
    }

    bool FImageAtlas::BinaryNode::Defragment()
    {
        if (child[0] != nullptr && child[1] != nullptr)
        {
            bool leftValid = child[0]->Defragment();
            bool rightValid = child[1]->Defragment();

            if (!leftValid && !rightValid)
            {
                child[0] = nullptr;
                child[1] = nullptr;
                return false;
            }

            // Perform advanced defragmentation which is very slow:
            {
                if (IsWidthSpan() && !leftValid && child[1]->child[0] != nullptr &&
                    child[1]->IsHeightSpan() && !child[1]->child[0]->IsValidRecursive() &&
                    child[1]->child[1] != nullptr && child[1]->child[1]->IsWidthSpan() &&
                    child[1]->child[1]->child[0] != nullptr && !child[1]->child[1]->child[0]->IsValidRecursive())
                {
                    Ptr<BinaryNode> nodeToMove = child[1]->child[1]->child[1];
                    f32 splitX = child[1]->child[1]->child[0]->rect.max.x;

                    child[0]->rect.max.x = splitX;
                    child[1]->rect.min.x = splitX + 1;
                    child[1]->child[0]->rect.min.x = splitX + 1;
                    child[1]->child[1] = nodeToMove;
                    nodeToMove->parent = child[1].Get();  // child[1]->child[1] is now nodeToMove itself

                    if (nodeToMove->IsHeightSpan() && !nodeToMove->child[0]->IsValidRecursive())
                    {
                    	Ptr<BinaryNode> contentNode = nodeToMove->child[1];
                        child[1]->child[0]->rect.max.y = contentNode->rect.min.y - 1;
                        child[1]->child[1] = contentNode;
                        contentNode->parent = child[1].Get();  // child[1]->child[1] is now contentNode itself
                    }
                }
            }

            return true;
        }

        return IsValid();
    }
} // namespace CE

