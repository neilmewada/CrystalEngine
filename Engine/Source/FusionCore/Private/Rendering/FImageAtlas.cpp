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

    void FImageAtlas::Init()
    {
        ZoneScoped;

        if (atlasLayers.NotEmpty())
            return;

        Ptr<FAtlasImage> atlas = new FAtlasImage(atlasSize);
        atlas->layerIndex = 0;

        atlasLayers.Add(atlas);

        Array<String> pages = GetPages();
        pages.Add(String::Format("Page {}", pages.GetSize()));
        SetPages(pages);

        UpdateImageAtlasItems();

        auto fusionShader = FusionApplication::Get()->GetFusionShader2();

        auto perDrawSrgLayout = fusionShader->GetDefaultVariant()->GetSrgLayout(RHI::SRGType::PerDraw);
        textureSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(perDrawSrgLayout);

        RPI::TextureDescriptor textureDescriptor{};
        textureDescriptor.texture.width = textureDescriptor.texture.height = atlasSize;
        textureDescriptor.texture.sampleCount = 1;
        textureDescriptor.texture.depth = 1;
        textureDescriptor.texture.dimension = Dimension::Dim2DArray;
        textureDescriptor.texture.arrayLayers = atlasLayers.GetSize();
        textureDescriptor.texture.mipLevels = 1;
        textureDescriptor.texture.bindFlags = TextureBindFlags::ShaderRead;
        textureDescriptor.texture.format = Format::R8G8B8A8_UNORM;
        textureDescriptor.texture.defaultHeapType = MemoryHeapType::Upload;

        textureDescriptor.samplerDesc.addressModeU = SamplerAddressMode::ClampToBorder;
        textureDescriptor.samplerDesc.addressModeV = SamplerAddressMode::ClampToBorder;
        textureDescriptor.samplerDesc.addressModeW = SamplerAddressMode::ClampToBorder;
        textureDescriptor.samplerDesc.borderColor = SamplerBorderColor::FloatTransparentBlack;
        textureDescriptor.samplerDesc.enableAnisotropy = false;
        textureDescriptor.samplerDesc.samplerFilterMode = FilterMode::Linear;

        for (int i = 0; i < atlasTexturesPerFrame.GetSize(); ++i)
        {
            textureDescriptor.texture.name = String::Format("Fusion Image Atlas {}", i);

            atlasTexturesPerFrame[i] = new RPI::Texture(textureDescriptor);

            textureSrg->Bind(i, "_Texture", atlasTexturesPerFrame[i]->GetRhiTexture());
            textureSrg->Bind(i, "_TextureSampler", atlasTexturesPerFrame[i]->GetSamplerState());
        }

        RHI::BufferDescriptor stagingDesc{};
        stagingDesc.name = "Staging Buffer";
        stagingDesc.bindFlags = RHI::BufferBindFlags::StagingBuffer;
        stagingDesc.bufferSize = atlasSize * atlasSize * sizeof(u32) * atlasLayers.GetSize();
        stagingDesc.defaultHeapType = RHI::MemoryHeapType::Upload;

        stagingBuffer = RHI::gDynamicRHI->CreateBuffer(stagingDesc);
        stagingBufferFence = RHI::gDynamicRHI->CreateFence(true);

        stagingCommandQueue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
        stagingCommandList = RHI::gDynamicRHI->AllocateCommandList(stagingCommandQueue);

        textureSrg->FlushBindings();

	    {
		    u8 pixels[16];
        	for (int i = 0; i < COUNTOF(pixels); ++i)
        	{
        		pixels[i] = NumericLimits<u8>::Max();
        	}
        	CMImage image = CMImage::LoadRawImageFromMemory(pixels, 4, 4, CMImageFormat::R8, CMImageSourceFormat::None, 8, 8);
        	whitePixel = AddImage("__WhitePixel", image);
	    }

	    {
		    u32 pixels[16];
        	for (int i = 0; i < COUNTOF(pixels); ++i)
        	{
        		pixels[i] = 0;
        	}
            CMImage image = CMImage::LoadRawImageFromMemory((u8*)pixels, 4, 4, CMImageFormat::RGBA8, CMImageSourceFormat::None, 8, 8 * 4);
            transparentPixel = AddImage("__TransparentPixel", image);
	    }
    }

    void FImageAtlas::Shutdown()
    {
        RHI::gDynamicRHI->DestroyShaderResourceGroup(textureSrg);
        textureSrg = nullptr;

        RHI::gDynamicRHI->DestroyBuffer(stagingBuffer);
        stagingBuffer = nullptr;

        RHI::gDynamicRHI->DestroyFence(stagingBufferFence);
        stagingBufferFence = nullptr;

        RHI::gDynamicRHI->FreeCommandLists(1, &stagingCommandList);
        stagingCommandList = nullptr;
        stagingCommandQueue = nullptr;

        for (int i = 0; i < atlasTexturesPerFrame.GetSize(); ++i)
        {
            delete atlasTexturesPerFrame[i];
            atlasTexturesPerFrame[i] = nullptr;
        }

    	atlasLayers.Clear();
        imagesByName.Clear();
    }

    void FImageAtlas::UpdateImageAtlasItems()
    {
	    for (int i = 0; i < atlasLayers.GetSize(); ++i)
	    {
            imagesByName[String::Format("__ImageAtlas_{}", i)] = ImageItem{
                .layerIndex = i,
                .uvMin = Vec2(0, 0),
                .uvMax = Vec2(1, 1),
                .width = atlasSize,
                .height = atlasSize
            };
	    }
    }

    void FImageAtlas::Flush(u32 imageIndex)
    {
        ZoneScoped;

        if (!flushRequiredPerImage[imageIndex])
            return;

        if (atlasTexturesPerFrame[imageIndex]->GetRhiTexture()->GetArrayLayerCount() != atlasLayers.GetSize())
        {
            RPI::TextureDescriptor textureDescriptor{};
            textureDescriptor.texture.width = textureDescriptor.texture.height = atlasSize;
            textureDescriptor.texture.sampleCount = 1;
            textureDescriptor.texture.depth = 1;
            textureDescriptor.texture.dimension = Dimension::Dim2DArray;
            textureDescriptor.texture.arrayLayers = atlasLayers.GetSize();
            textureDescriptor.texture.name = String::Format("Fusion Image Atlas {}", imageIndex);
            textureDescriptor.texture.mipLevels = 1;
            textureDescriptor.texture.bindFlags = TextureBindFlags::ShaderRead;
            textureDescriptor.texture.format = Format::R8G8B8A8_UNORM;
            textureDescriptor.texture.defaultHeapType = MemoryHeapType::Upload;

            textureDescriptor.samplerDesc.addressModeU = SamplerAddressMode::ClampToBorder;
            textureDescriptor.samplerDesc.addressModeV = SamplerAddressMode::ClampToBorder;
            textureDescriptor.samplerDesc.addressModeW = SamplerAddressMode::ClampToBorder;
            textureDescriptor.samplerDesc.borderColor = SamplerBorderColor::FloatTransparentBlack;
            textureDescriptor.samplerDesc.enableAnisotropy = false;
            textureDescriptor.samplerDesc.samplerFilterMode = FilterMode::Linear;

            delete atlasTexturesPerFrame[imageIndex];

            atlasTexturesPerFrame[imageIndex] = new RPI::Texture(textureDescriptor);
            
            textureSrg->Bind(imageIndex, "_Texture", atlasTexturesPerFrame[imageIndex]->GetRhiTexture());
            textureSrg->Bind(imageIndex, "_TextureSampler", atlasTexturesPerFrame[imageIndex]->GetSamplerState());
        }

        textureSrg->FlushBindings();
        
        // Copy data to image
        {
            RPI::Texture* atlasTexture = atlasTexturesPerFrame[imageIndex];
            RHI::Texture* atlasRhiTexture = atlasTexture->GetRhiTexture();

            auto commandList = stagingCommandList;

            commandList->Begin();
            {
                ResourceBarrierDescriptor barrier{};
                barrier.resource = stagingBuffer;
                barrier.fromState = ResourceState::Undefined;
                barrier.toState = ResourceState::CopySource;
                commandList->ResourceBarrier(1, &barrier);

                barrier.resource = atlasRhiTexture;
                barrier.fromState = ResourceState::Undefined;
                barrier.toState = ResourceState::CopyDestination;
                commandList->ResourceBarrier(1, &barrier);

                RHI::BufferToTextureCopy copy{};
                copy.dstTexture = atlasRhiTexture;
                copy.mipSlice = 0;
                copy.baseArrayLayer = 0;
                copy.layerCount = atlasLayers.GetSize();

            	copy.srcBuffer = stagingBuffer;
                copy.bufferOffset = 0;

                commandList->CopyTextureRegion(copy);

                barrier.resource = atlasRhiTexture;
                barrier.fromState = ResourceState::CopyDestination;
                barrier.toState = ResourceState::FragmentShaderResource;
                commandList->ResourceBarrier(1, &barrier);
            }
            commandList->End();

            stagingBufferFence->Reset();
            stagingCommandQueue->Execute(1, &commandList, stagingBufferFence);
            stagingBufferFence->WaitForFence();
        }

        flushRequiredPerImage[imageIndex] = false;
    }

    FImageAtlas::ImageItem FImageAtlas::AddImage(const Name& name, const CMImage& imageSource)
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
                atlas->root->DefragmentSlow();
                
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

            Array<String> pages = GetPages();
            pages.Add(String::Format("Page {}", pages.GetSize()));
            SetPages(pages);

            UpdateImageAtlasItems();

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

        ImageItem item = { .layerIndex = foundAtlas->layerIndex, .uvMin = uvMin, .uvMax = uvMax };
        item.width = textureSize.width;
        item.height = textureSize.height;

        imagesByName[name] = item;

        for (int i = 0; i < flushRequiredPerImage.GetSize(); ++i)
        {
            flushRequiredPerImage[i] = true;
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
                    u8* r = dstPixel;
                    u8* g = dstPixel + 1;
                    u8* b = dstPixel + 2;
                    u8* a = dstPixel + 3;

                    memset(dstPixel, 0, sizeof(u32));
                }
            }

            stagingBuffer->Unmap();
        }

        atlas->root->usedArea -= node->rect.GetAreaInt();
        node->ClearImage();
        atlas->root->Defragment();

        for (int i = 0; i < flushRequiredPerImage.GetSize(); ++i)
        {
            flushRequiredPerImage[i] = true;
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

    bool FImageAtlas::BinaryNode::DefragmentSlow()
    {
        if (child[0] != nullptr && child[1] != nullptr)
        {
            bool leftValid = child[0]->DefragmentSlow();
            bool rightValid = child[1]->DefragmentSlow();

            if (!leftValid && !rightValid)
            {
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
                    nodeToMove->parent = child[1]->child[1];

                    if (nodeToMove->IsHeightSpan() && !nodeToMove->child[0]->IsValidRecursive())
                    {
                    	Ptr<BinaryNode> contentNode = nodeToMove->child[1];
                        child[1]->child[0]->rect.max.y = contentNode->rect.min.y - 1;
                        child[1]->child[1] = contentNode;
                        contentNode->parent = child[1]->child[1];
                    }
                }
            }

            return true;
        }

        return IsValid();
    }
} // namespace CE

