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

        samplerStates.Reserve(MaxSamplerCount);

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
                MaxSamplerCount
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

    }

    bool FRenderService::ReplaceTexture(int slot, int frameIndex, RHI::Texture* rhiTexture)
    {
        if (!rhiTexture)
            return false;
        if (slot < 0 || slot >= texturesBySlotPerFrame[frameIndex].GetCount())
            return false;
        if (texturesBySlotPerFrame[frameIndex][slot] == nullptr) // The slot was not registered.
            return false;

        texturesBySlotPerFrame[frameIndex][slot] = rhiTexture;

        textureDirty.Set();

        return true;
    }

    int FRenderService::RegisterTexture(StaticArray<RHI::Texture*, RHI::Limits::MaxSwapChainImageCount> rhiTexture)
    {
        for (int i = 0; i < rhiTexture.GetSize(); i++)
        {
            if (rhiTexture[i] == nullptr)
                return -1;
        }

        textureDirty.Set();

        if (freeSlots.IsEmpty())
        {
            for (int i = 0; i < rhiTexture.GetSize(); i++)
            {
	            texturesBySlotPerFrame[i].Insert(rhiTexture[i]);
            }
            totalTextures++;
            return (int)texturesBySlotPerFrame[0].GetCount() - 1;
        }

        int slot = freeSlots.Last();
        freeSlots.RemoveLast();

        for (int i = 0; i < rhiTexture.GetSize(); i++)
        {
            texturesBySlotPerFrame[i][slot] = rhiTexture[i];
        }

        totalTextures++;
        return slot;
    }

    int FRenderService::RegisterTexture(RHI::Texture* rhiTexture)
    {
        StaticArray<RHI::Texture*, RHI::Limits::MaxSwapChainImageCount> textures{};
        for (int i = 0; i < textures.GetSize(); i++)
        {
            textures[i] = rhiTexture;
        }
		return RegisterTexture(textures);
    }

    void FRenderService::FlushTextures()
    {
		int frameIndex = GetCurrentFrameIndex();
		bool needsFlush = false;

        if (samplerDirty.Test(frameIndex))
        {
			samplerDirty.Set(frameIndex, false);
			needsFlush = true;

			if (samplerStates.GetSize() > 0)
            {
                sceneSrg->Bind(frameIndex, "_Samplers", 0, samplerStates.GetSize(), samplerStates.GetData());
            }
        }

        if (textureDirty.Test(frameIndex))
        {
        	textureDirty.Set(frameIndex, false);
            needsFlush = true;

            if (totalTextures > 0 && texturesBySlotPerFrame[frameIndex].GetCount() > 0)
            {
                sceneSrg->Bind(frameIndex, "_Textures", 0, texturesBySlotPerFrame[frameIndex].GetCount(), texturesBySlotPerFrame[frameIndex].GetData());
            }
        }

		if (needsFlush)
		{
			sceneSrg->FlushBindings();
		}
    }

    void FRenderService::DeregisterTexture(int slot)
    {
        if (slot < 0 || slot >= texturesBySlotPerFrame[0].GetCount())
            return;
        if (!texturesBySlotPerFrame[0][slot])
            return;

        freeSlots.Insert(slot);
        
        for (int i = 0; i < texturesBySlotPerFrame.GetSize(); i++)
        {
            texturesBySlotPerFrame[i][slot] = RPISystem::Get().GetDefaultAlbedoTex()->GetRhiTexture();
        }

        totalTextures--;

        textureDirty.Set();
    }

    int FRenderService::FindOrCreateSampler(RHI::FilterMode filterMode, RHI::SamplerAddressMode addressU,
	    RHI::SamplerAddressMode addressV, RHI::SamplerBorderColor borderColor)
    {
        if (addressU == SamplerAddressMode::ClampToEdge)
			addressU = SamplerAddressMode::ClampToBorder;
		if (addressV == SamplerAddressMode::ClampToEdge)
			addressV = SamplerAddressMode::ClampToBorder;
		if (addressU == SamplerAddressMode::MirroredRepeat)
            addressU = SamplerAddressMode::Repeat;
        if (addressV == SamplerAddressMode::MirroredRepeat)
			addressV = SamplerAddressMode::Repeat;

		if (addressU == SamplerAddressMode::ClampToBorder || addressV == SamplerAddressMode::ClampToBorder)
        {
            if (borderColor == SamplerBorderColor::FloatTransparentBlack || borderColor == SamplerBorderColor::IntTransparentBlack)
            {
                borderColor = SamplerBorderColor::FloatTransparentBlack;
            }
            else if (borderColor == SamplerBorderColor::FloatOpaqueBlack || borderColor == SamplerBorderColor::IntOpaqueBlack)
            {
                borderColor = SamplerBorderColor::FloatOpaqueBlack;
            }
            else
            {
                borderColor = SamplerBorderColor::FloatOpaqueWhite;
            }
        }
        else
        {
			borderColor = SamplerBorderColor::FloatTransparentBlack;
        }

        FSampleState state{};
		state.filterMode = filterMode;
		state.addressU = addressU;
		state.addressV = addressV;
        state.borderColor = borderColor;

        if (samplerIndicesByState.KeyExists(state))
        {
            return samplerIndicesByState[state];
		}

		RHI::SamplerDescriptor desc{};
		desc.samplerFilterMode = filterMode;
		desc.addressModeU = addressU;
		desc.addressModeV = addressV;
        desc.addressModeW = SamplerAddressMode::Repeat;
		desc.borderColor = borderColor;

        RHI::Sampler* sampler = RPISystem::Get().FindOrCreateSampler(desc);
		int samplerIndex = (int)samplerStates.GetSize();

        samplerStates.Add(sampler);
		samplerIndicesByState[state] = samplerIndex;

		samplerDirty.Set();

        return samplerIndex;
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

