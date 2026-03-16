#include "FusionCore.h"

namespace CE
{
    extern RawData Get_Shaders_Fusion_vert_spv();
    extern RawData Get_Shaders_Fusion_frag_spv();
    extern RawData Get_Shaders_Fusion_vert_json();
    extern RawData Get_Shaders_Fusion_frag_json();

    FRenderService::FRenderService()
    {

    }

    void FRenderService::OnStart()
    {
	    Super::OnStart();

        RawData vertexShader = Get_Shaders_Fusion_vert_spv();
        RawData fragmentShader = Get_Shaders_Fusion_frag_spv();

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
                for (int i = 0; i < application->GetSurfaceCount(); i++)
                {
                    application->GetSurface(i)->UpdateBuffers(GetCurrentFrameIndex());
                }

                for (int i = 0; i < application->GetSurfaceCount(); i++)
                {
                    application->GetSurface(i)->FlushDrawPackets();
                }

                EndRender();
            }
        }
    }

    void FRenderService::UpdateDrawListMask(RHI::DrawListMask& drawListMask)
    {
        Ref<FApplication> application = this->application.Lock();
        if (!application)
            return;

		for (int i = 0; i < application->GetSurfaceCount(); i++)
        {
            if (Ref<FSurface> surface = application->GetSurface(i))
            {
                surface->GetDrawListMask(drawListMask);
            }
        }
    }

} // namespace CE

