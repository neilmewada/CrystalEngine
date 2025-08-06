#include "FusionCore.h"

namespace CE
{
	u64 FViewport::ComputeMemoryFootprint()
	{
		u64 size = Super::ComputeMemoryFootprint();

		for (int i = 0; i < frames.GetSize(); ++i)
		{
			if (frames[i] != nullptr)
			{
				size += frames[i]->GetWidth() * frames[i]->GetHeight() * sizeof(u32);
			}
		}

        return size;
	}

    FViewport::FViewport()
    {
        m_ImageFormat = Format::B8G8R8A8_UNORM;
        m_SampleCount = 1;
    }

    FViewport::~FViewport()
    {
        
    }

    void FViewport::Release()
    {
        for (int i = 0; i < frames.GetSize(); ++i)
        {
            delete frames[i]; frames[i] = nullptr;

            RPISystem::Get().QueueDestroy(frameViews[i]);
            frameViews[i] = nullptr;
        }

        RPISystem::Get().QueueDestroy(textureSrg);
        textureSrg = nullptr;
    }

    void FViewport::OnBeginDestroy()
    {
	    Super::OnBeginDestroy();

        if (IsDefaultInstance())
            return;

        FusionApplication::Get()->DeregisterViewport(this);

        Release();
    }

    void FViewport::Construct()
    {
        Super::Construct();

        auto app = FusionApplication::Get();

        RHI::ShaderResourceGroupLayout textureSrgLayout = app->GetFusionShader2()->GetDefaultVariant()->GetSrgLayout(RHI::SRGType::PerDraw);
        if (textureSrg == nullptr)
        {
            textureSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(textureSrgLayout);
        }

        FusionApplication::Get()->RegisterViewport(this);
    }

    void FViewport::OnFusionPropertyModified(const CE::Name& propertyName)
    {
        ZoneScoped;

	    Super::OnFusionPropertyModified(propertyName);

        static const CE::Name imageFormat = "ImageFormat";
        static const CE::Name sampleCount = "SampleCount";

        if (propertyName == imageFormat || propertyName == sampleCount)
        {
            RecreateFrameBuffer();
        }
    }

    void FViewport::OnPostComputeLayout()
    {
	    Super::OnPostComputeLayout();

        if ((int)computedSize.x != currentSize.x || (int)computedSize.y != currentSize.y)
        {
            currentSize = computedSize.ToVec2i();

            RecreateFrameBuffer();
        }
    }

    void FViewport::RecreateFrameBuffer()
    {
        ZoneScoped;

        if (GetContext() == nullptr)
        {
	        return;
        }

        if (currentSize.x <= 0 || currentSize.y <= 0)
        {
            currentSize = Vec2i(512, 512);
        }

        scene->SetPrimaryViewportSize(currentSize);

		bool isFirstTime = true;

        for (int i = 0; i < frames.GetSize(); ++i)
        {
            if (frames[i] != nullptr)
            {
                isFirstTime = false;
            }

            delete frames[i]; frames[i] = nullptr;

            RPI::RPISystem::Get().QueueDestroy(frameViews[i]); frameViews[i] = nullptr;
        }

        RPI::TextureDescriptor textureDescriptor{};

        f32 scaling = GetContext()->GetScaling();

        textureDescriptor.texture.bindFlags = TextureBindFlags::Color | TextureBindFlags::ShaderRead | TextureBindFlags::ShaderWrite;
        textureDescriptor.texture.arrayLayers = 1;
        textureDescriptor.texture.mipLevels = 1;
        textureDescriptor.texture.sampleCount = m_SampleCount;
        textureDescriptor.texture.format = m_ImageFormat;
        textureDescriptor.texture.defaultHeapType = MemoryHeapType::Default;
        textureDescriptor.texture.width = (u32)(currentSize.x * scaling);
        textureDescriptor.texture.height = (u32)(currentSize.y * scaling);
        textureDescriptor.texture.depth = 1;
        textureDescriptor.texture.dimension = Dimension::Dim2DArray;

	    RHI::TextureViewDescriptor textureViewDescriptor{};
	    textureViewDescriptor.format = textureDescriptor.texture.format;
	    textureViewDescriptor.arrayLayerCount = 1;
	    textureViewDescriptor.mipLevelCount = 1;
	    textureViewDescriptor.baseArrayLayer = 0;
	    textureViewDescriptor.baseMipLevel = 0;
	    textureViewDescriptor.dimension = Dimension::Dim2D;

        textureDescriptor.samplerDesc.addressModeU =
            textureDescriptor.samplerDesc.addressModeV =
            textureDescriptor.samplerDesc.addressModeW =
            SamplerAddressMode::ClampToBorder;
        textureDescriptor.samplerDesc.borderColor = SamplerBorderColor::FloatOpaqueBlack;
        textureDescriptor.samplerDesc.enableAnisotropy = false;
        textureDescriptor.samplerDesc.samplerFilterMode = FilterMode::Linear;

        for (int i = 0; i < frames.GetSize(); ++i)
        {
            textureDescriptor.texture.name = GetName().GetString() + " FrameBuffer " + i;
            frames[i] = new RPI::Texture(textureDescriptor);

            if (isFirstTime)
            {
                frames[i]->TransitionResourceTo(RHI::ResourceState::Undefined, RHI::ResourceState::FragmentShaderResource);
            }

            textureViewDescriptor.texture = frames[i]->GetRhiTexture();
            frameViews[i] = gDynamicRHI->CreateTextureView(textureViewDescriptor);
        }

        auto app = FusionApplication::Get();

        for (int i = 0; i < frames.GetSize(); ++i)
        {
            textureSrg->Bind(i, "_Texture", frames[i]->GetRhiTexture());
            textureSrg->Bind(i, "_TextureSampler", frames[i]->GetSamplerState());
        }

        textureSrg->FlushBindings();

        MarkDirty();

        m_OnFrameBufferRecreated();
    }

    void FViewport::OnPaint(FPainter* painter)
    {
	    Super::OnPaint(painter);

        painter->SetPen(FPen());
        painter->SetBrush(FBrush());

        if (frames[0] != nullptr)
        {
            painter->DrawViewport(Rect::FromSize(computedPosition, computedSize), this);
        }
    }

    void FViewport::OnSceneChanged()
    {
	    Super::OnSceneChanged();

        RecreateFrameBuffer();
    }
}

