#include "FusionCore.h"

namespace CE
{
    extern RawData Get_Shaders_Fusion_vert_spv();
    extern RawData Get_Shaders_Fusion_frag_spv();
    extern RawData Get_Shaders_Fusion_vert_msl();
    extern RawData Get_Shaders_Fusion_frag_msl();
    extern RawData Get_Shaders_Fusion_vert_json();
    extern RawData Get_Shaders_Fusion_frag_json();

    FRenderService::FRenderService()
    {
        textureDirty.Set();
    }

    void FRenderService::OnStart()
    {
	    Super::OnStart();

#if PLATFORM_MAC
        RawData vertexShader = Get_Shaders_Fusion_vert_msl();
        RawData fragmentShader = Get_Shaders_Fusion_frag_msl();
#else
        RawData vertexShader = Get_Shaders_Fusion_vert_spv();
        RawData fragmentShader = Get_Shaders_Fusion_frag_spv();
#endif

        ShaderReflection reflection{};
        reflection.vertexInputTypes.AddRange({
            RHI::VertexAttributeDataType::Float2,
            RHI::VertexAttributeDataType::Float2,
            RHI::VertexAttributeDataType::UChar4,
            RHI::VertexAttributeDataType::UInt
        });
        reflection.vertexInputs.AddRange({
			"POSITION",
			"TEXCOORD0",
			"COLOR0",
			"TEXCOORD1"
        });
        
    	reflection.srgLayouts.Add({});
        auto& sceneSrgLayout = reflection.srgLayouts.GetLast();
        sceneSrgLayout.srgType = SRGType::PerScene;
        sceneSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
                "_Samplers",
                0,
                RHI::ShaderResourceType::SamplerState,
                RHI::ShaderStage::Fragment | RHI::ShaderStage::Compute,
                16
            )
        );
        sceneSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
				"_Textures",
                1,
                RHI::ShaderResourceType::Texture2D,
                RHI::ShaderStage::Fragment | RHI::ShaderStage::Compute,
                MaxTextureCount
            )
        );

        sceneSrg = RHI::gDynamicRHI->CreateShaderResourceGroup({ "SceneSrg", sceneSrgLayout });

        reflection.srgLayouts.Add({});
        auto& viewSrg = reflection.srgLayouts.GetLast();
        viewSrg.srgType = SRGType::PerView;
        viewSrg.TryAdd(
            RHI::SRGVariableDescriptor(
				"_PerViewData",
                0,
                RHI::ShaderResourceType::ConstantBuffer,
                RHI::ShaderStage::Vertex | RHI::ShaderStage::Fragment
            )
        );

        reflection.srgLayouts.Add({});
        auto& subPassSrgLayout = reflection.srgLayouts.GetLast();
        subPassSrgLayout.srgType = SRGType::PerSubPass;
        subPassSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
				"_LayerData",
                0,
                RHI::ShaderResourceType::ConstantBuffer,
                RHI::ShaderStage::Vertex | RHI::ShaderStage::Fragment
            )
        );

        this->subPassSrgLayout = subPassSrgLayout;

        reflection.srgLayouts.Add({});
        auto& objectSrgLayout = reflection.srgLayouts.GetLast();
        objectSrgLayout.srgType = SRGType::PerObject;
        objectSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
                "_DrawItems",
                0,
                RHI::ShaderResourceType::StructuredBuffer,
                RHI::ShaderStage::Fragment
            )
        );

        objectSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
				"_ClipItems",
                1,
                RHI::ShaderResourceType::StructuredBuffer,
                RHI::ShaderStage::Fragment
            )
        );

        objectSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
				"_GradientStops",
                2,
                RHI::ShaderResourceType::StructuredBuffer,
                RHI::ShaderStage::Fragment
            )
        );

        this->objectSrgLayout = objectSrgLayout;

        mainShader = FShader::Create("FusionShader", reflection, vertexShader, fragmentShader, this);
    }

    void FRenderService::OnShutdown()
    {
	    Super::OnShutdown();

        delete sceneSrg; sceneSrg = nullptr;
    }

    void FRenderService::TickService(FServiceTickPhase tickPhase)
    {
        Ref<FApplication> application = this->application.Lock();
        if (!application)
            return;

        ZoneScoped;

        if (tickPhase == FServiceTickPhase::RenderPrepare)
        {
            RenderPrepare();

            for (int i = 0; i < application->GetSurfaceCount(); i++)
            {
                application->GetSurface(i)->RenderFrame(GetCurrentFrameIndex());
            }
        }
		else if (tickPhase == FServiceTickPhase::Render)
        {
            if (BeginRender())
            {
                int frameIndex = GetCurrentFrameIndex();

                if (textureDirty.Test(frameIndex))
                {
                    textureDirty.Set(frameIndex, false);

                    for (int i = 0; i < texturesBySlot.GetCount(); i++)
                    {
	                    
                    }
                }

            	EndRender();
            }
        }
    }

    int FRenderService::RegisterTexture(RHI::Texture* rhiTexture)
    {
        return RegisterTexture(TextureImpl{ .rhiTexture = rhiTexture });
    }

    int FRenderService::RegisterTexture(RHI::TextureView* rhiTextureView)
    {
        return RegisterTexture(TextureImpl{ .rhiTextureView = rhiTextureView });
    }

    int FRenderService::RegisterTexture(RPI::Texture* rpiTexture)
    {
        return RegisterTexture(TextureImpl{ .rpiTexture = rpiTexture });
    }

    int FRenderService::RegisterTexture(const TextureImpl& texture)
    {
        if (!texture.IsValid())
            return -1;

        textureDirty.Set();

        if (freeSlots.IsEmpty())
        {
            texturesBySlot.Insert(texture);
            return (int)texturesBySlot.GetCount() - 1;
        }

        int slot = freeSlots.Last();
        freeSlots.RemoveLast();

        texturesBySlot[slot] = texture;

        return slot;
    }

    void FRenderService::DeregisterTexture(int slot)
    {
        if (slot < 0 || slot >= texturesBySlot.GetCount())
            return;
        if (!texturesBySlot[slot].IsValid())
            return;

        freeSlots.Insert(slot);
        texturesBySlot[slot] = {};

        textureDirty.Set();
    }

    void FRenderService::UpdateDrawListMask(RHI::DrawListMask& drawListMask)
    {
        Ref<FApplication> application = this->application.Lock();
        if (!application)
            return;

        ZoneScoped;

		for (int i = 0; i < application->GetSurfaceCount(); i++)
        {
            if (Ref<FSurface> surface = application->GetSurface(i))
            {
                surface->GetDrawListMask(drawListMask);
            }
        }
    }

} // namespace CE

