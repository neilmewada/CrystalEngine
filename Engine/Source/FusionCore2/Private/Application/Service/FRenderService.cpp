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
                RHI::ShaderStage::Fragment,
                16
            )
        );
        sceneSrgLayout.TryAdd(
            RHI::SRGVariableDescriptor(
				"_Textures",
                1,
                RHI::ShaderResourceType::Texture2D,
                RHI::ShaderStage::Fragment,
                0
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
            	EndRender();
            }
        }
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

