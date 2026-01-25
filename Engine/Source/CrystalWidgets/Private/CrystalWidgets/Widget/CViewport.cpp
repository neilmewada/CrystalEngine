#include "CrystalWidgets.h"

namespace CE::Widgets
{

    CViewport::CViewport()
    {
	    
    }

    CViewport::~CViewport()
    {
        for (int i = 0; i < frames.GetSize(); ++i)
        {
            if (frames[i] != nullptr)
            {
                delete frames[i]; frames[i] = nullptr;
            }
        }
    }

    void CViewport::RecreateFrameBuffer()
    {
        if (currentSize.x <= 0 || currentSize.y <= 0)
            return;

        RPI::TextureDescriptor textureDescriptor{};

        BinaryBlob blob{};
        blob.Reserve(currentSize.x * currentSize.y * 4);
        u8* ptr = blob.GetDataPtr();
        for (int y = 0; y < currentSize.y; ++y)
        {
            for (int x = 0; x < currentSize.x; ++x)
            {
                int i = y * currentSize.x * 4 + x * 4;
                u8 bgra[] = { 0, 0, 0, 255 };
                if (x < currentSize.x / 3.0f)
                    bgra[2] = 255;
                else if (x < currentSize.x * 2.0f / 3.0f)
                    bgra[1] = 255;
                else
                    bgra[0] = 255;

                *(ptr + i + 0) = bgra[0];
                *(ptr + i + 1) = bgra[1];
                *(ptr + i + 2) = bgra[2];
                *(ptr + i + 3) = bgra[3];
            }
        }

        textureDescriptor.texture.name = GetName().GetString() + " FrameBuffer";
        textureDescriptor.texture.bindFlags = TextureBindFlags::Color | TextureBindFlags::ShaderRead;
        textureDescriptor.texture.arrayLayers = 1;
        textureDescriptor.texture.mipLevels = 1;
        textureDescriptor.texture.sampleCount = sampleCount;
        textureDescriptor.texture.format = format;
        textureDescriptor.texture.defaultHeapType = MemoryHeapType::Default;
        textureDescriptor.texture.width = currentSize.x;
        textureDescriptor.texture.height = currentSize.y;
        textureDescriptor.texture.dimension = Dimension::Dim2D;

        textureDescriptor.samplerDesc.addressModeU =
            textureDescriptor.samplerDesc.addressModeV =
            textureDescriptor.samplerDesc.addressModeW =
            SamplerAddressMode::ClampToBorder;
        textureDescriptor.samplerDesc.borderColor = SamplerBorderColor::FloatOpaqueBlack;
        textureDescriptor.samplerDesc.enableAnisotropy = false;
        textureDescriptor.samplerDesc.samplerFilterMode = FilterMode::Linear;

        textureDescriptor.source = &blob;

        for (int i = 0; i < frames.GetSize(); ++i)
        {
	        if (frames[i] != nullptr)
	        {
                if (i == 0)
                    RHI::FrameScheduler::Get()->WaitUntilIdle();

                delete frames[i]; frames[i] = nullptr;
	        }

            frames[i] = new RPI::Texture(textureDescriptor);
        }

        blob.Free();

        onFrameBufferRecreated();
    }

    void CViewport::OnPaint(CPaintEvent* paintEvent)
    {
        CPainter* painter = paintEvent->painter;

        Vec2 position = GetComputedLayoutTopLeft();
        Vec2i size = GetComputedLayoutSize().ToVec2i();

        if (size != currentSize)
        {
            currentSize = size;

            RecreateFrameBuffer();
        }

        Vec2 extraSize = Vec2();
        if (IsWindow() && parent == nullptr)
        {
            extraSize = Vec2(rootPadding.left + rootPadding.right, rootPadding.top + rootPadding.bottom);
        }

        painter->SetBrush(CBrush(Colors::White));

        painter->DrawFrameBuffer(Rect::FromSize(position, size.ToVec2() + extraSize), frames);

	    Super::OnPaint(paintEvent);
    }

} // namespace CE::Widgets
