#include "Engine.h"

namespace CE
{

    MainRenderPipeline::MainRenderPipeline()
    {
        msaa = MSAA4;
    }

    MainRenderPipeline::~MainRenderPipeline()
    {
	    
    }

    void MainRenderPipeline::ConstructPipeline()
    {
	    Super::ConstructPipeline();

	    RPI::PassTree* passTree = renderPipeline->passTree;
    	renderPipeline->mainViewTag = "MainCamera";

	    RPI::ParentPass* rootPass = passTree->GetRootPass();

    	RPI::PassAttachment* pipelineOutput = renderPipeline->FindAttachment("PipelineOutput");

        // -------------------------------
        // Transient Attachments
        // -------------------------------

		// - Depth Stencil -

	    RPI::PassAttachment* depthStencilAttachment;
	    {
            RPI::PassImageAttachmentDesc depthStencilAttachmentDesc{};
            depthStencilAttachmentDesc.name = "DepthStencil";
            depthStencilAttachmentDesc.lifetime = RHI::AttachmentLifetimeType::Transient;
            depthStencilAttachmentDesc.sizeSource.source = pipelineOutput->name;

            depthStencilAttachmentDesc.imageDescriptor.format = RHI::Format::D32_SFLOAT_S8_UINT;
            depthStencilAttachmentDesc.imageDescriptor.arrayLayers = 1;
            depthStencilAttachmentDesc.imageDescriptor.mipCount = 1;
            depthStencilAttachmentDesc.imageDescriptor.dimension = RHI::Dimension::Dim2D;
            depthStencilAttachmentDesc.imageDescriptor.bindFlags = RHI::TextureBindFlags::DepthStencil;
            depthStencilAttachmentDesc.fallbackFormats = { RHI::Format::D24_UNORM_S8_UINT, RHI::Format::D16_UNORM_S8_UINT };

            switch (msaa)
            {
            case MSAA1:
                depthStencilAttachmentDesc.imageDescriptor.sampleCount = 1;
                break;
            case MSAA2:
                depthStencilAttachmentDesc.imageDescriptor.sampleCount = 2;
                break;
            case MSAA4:
                depthStencilAttachmentDesc.imageDescriptor.sampleCount = 4;
                break;
            case MSAA8:
                depthStencilAttachmentDesc.imageDescriptor.sampleCount = 8;
                break;
            }

            depthStencilAttachment = renderPipeline->AddAttachment(depthStencilAttachmentDesc);
	    }

    	// - Color MSAA -
    	
        RPI::PassAttachment* colorMsaa;
	    {
            RPI::PassImageAttachmentDesc colorMsaaAttachmentDesc{};
            colorMsaaAttachmentDesc.name = "ColorMSAA";
            colorMsaaAttachmentDesc.lifetime = RHI::AttachmentLifetimeType::Transient;
            colorMsaaAttachmentDesc.sizeSource.source = pipelineOutput->name;

            colorMsaaAttachmentDesc.imageDescriptor.format = RHI::Format::Undefined;
            colorMsaaAttachmentDesc.imageDescriptor.mipCount = 1;
            colorMsaaAttachmentDesc.imageDescriptor.arrayLayers = 1;
            colorMsaaAttachmentDesc.imageDescriptor.dimension = RHI::Dimension::Dim2D;
            colorMsaaAttachmentDesc.imageDescriptor.bindFlags = RHI::TextureBindFlags::Color;

            colorMsaaAttachmentDesc.formatSource = pipelineOutput->name;
            colorMsaaAttachmentDesc.fallbackFormats = { RHI::Format::R8G8B8A8_UNORM, RHI::Format::B8G8R8A8_UNORM };

            switch (msaa)
            {
            case MSAA1:
                colorMsaaAttachmentDesc.imageDescriptor.sampleCount = 1;
	            break;
            case MSAA2:
                colorMsaaAttachmentDesc.imageDescriptor.sampleCount = 2;
	            break;
            case MSAA4:
                colorMsaaAttachmentDesc.imageDescriptor.sampleCount = 4;
	            break;
            case MSAA8:
                colorMsaaAttachmentDesc.imageDescriptor.sampleCount = 8;
	            break;
            }

            colorMsaa = renderPipeline->AddAttachment(colorMsaaAttachmentDesc);
	    }

    	// - Directional Shadow Map -

        PassAttachment* directionalShadowMap;
	    {
            RPI::PassImageAttachmentDesc directionalShadowMapDesc{};
            directionalShadowMapDesc.name = "DirectionalShadowMap";
            directionalShadowMapDesc.lifetime = RHI::AttachmentLifetimeType::Transient;
            directionalShadowMapDesc.sizeSource.fixedSizes = Vec3i(directionalShadowResolution, directionalShadowResolution, 1);

            directionalShadowMapDesc.imageDescriptor.format = Format::D32_SFLOAT;
            directionalShadowMapDesc.imageDescriptor.mipCount = 1;
            directionalShadowMapDesc.imageDescriptor.arrayLayers = 1;
            directionalShadowMapDesc.imageDescriptor.dimension = Dimension::Dim2D;
            directionalShadowMapDesc.imageDescriptor.sampleCount = 1;
            directionalShadowMapDesc.imageDescriptor.bindFlags = TextureBindFlags::Depth | TextureBindFlags::ShaderRead;
            directionalShadowMapDesc.fallbackFormats = { Format::D32_SFLOAT_S8_UINT, Format::D24_UNORM_S8_UINT, Format::D16_UNORM, Format::D16_UNORM_S8_UINT };

            directionalShadowMap = renderPipeline->AddAttachment(directionalShadowMapDesc);
	    }

        // -------------------------------
        // Passes
        // -------------------------------

        // - Depth Pass -

        RPI::RasterPass* depthPass = (RPI::RasterPass*)RPI::PassSystem::Get().CreatePass(this, "DepthPass");
    	depthPass->SetViewTag(mainViewTag);
        depthPass->SetDrawListTag(GetBuiltinDrawListTag(RPI::BuiltinDrawItemTag::Depth));
	    {
            RPI::PassAttachmentBinding outputBinding{};
            outputBinding.slotType = RPI::PassSlotType::Output;
            outputBinding.attachment = depthStencilAttachment;
            outputBinding.attachmentUsage = RHI::ScopeAttachmentUsage::DepthStencil;
            outputBinding.name = "DepthOutput";
            outputBinding.connectedBinding = outputBinding.fallbackBinding = nullptr;

            depthPass->AddAttachmentBinding(outputBinding);
            
            rootPass->AddChild(depthPass);
	    }

        // - Skybox Pass -

	    RPI::RasterPass* skyboxPass = (RPI::RasterPass*)RPI::PassSystem::Get().CreatePass(this, "SkyboxPass");
    	skyboxPass->SetViewTag(mainViewTag);
        skyboxPass->SetDrawListTag(GetBuiltinDrawListTag(RPI::BuiltinDrawItemTag::Skybox));
	    {
            RPI::PassAttachmentBinding colorOutputBinding{};
            colorOutputBinding.slotType = RPI::PassSlotType::Output;
            colorOutputBinding.attachment = colorMsaa;
            colorOutputBinding.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
            colorOutputBinding.name = "ColorOutput";
            colorOutputBinding.connectedBinding = colorOutputBinding.fallbackBinding = nullptr;

            skyboxPass->AddAttachmentBinding(colorOutputBinding);

            rootPass->AddChild(skyboxPass);
	    }

        // - Directional Shadow Pass -

        RPI::RasterPass* directionalShadowPass = (RPI::RasterPass*)RPI::PassSystem::Get().CreatePass(this, "DirectionalShadowPass");
    	directionalShadowPass->SetViewTag("DirectionalLightShadow");
        directionalShadowPass->SetDrawListTag(GetBuiltinDrawListTag(BuiltinDrawItemTag::Shadow));
	    {
            // Directional Shadow Map
            {
                PassAttachmentBinding shadowMapListBinding{};
                shadowMapListBinding.slotType = PassSlotType::Output;
                shadowMapListBinding.attachmentUsage = ScopeAttachmentUsage::DepthStencil;
                shadowMapListBinding.attachment = directionalShadowMap;
                shadowMapListBinding.name = "DepthOutput";
                shadowMapListBinding.connectedBinding = shadowMapListBinding.fallbackBinding = nullptr;

                directionalShadowPass->AddAttachmentBinding(shadowMapListBinding);
            }

            rootPass->AddChild(directionalShadowPass);
	    }

        // - Opaque Pass -

        auto opaquePass = CreateObject<RPI::RasterPass>(this, "OpaquePass");
    	opaquePass->SetViewTag(mainViewTag);
        opaquePass->SetDrawListTag(GetBuiltinDrawListTag(RPI::BuiltinDrawItemTag::Opaque));
	    {
            // DepthInput
            {
	            RPI::PassSlot depthSlot{};
                depthSlot.name = "DepthInput";
                depthSlot.slotType = RPI::PassSlotType::Input;
                depthSlot.attachmentUsage = RHI::ScopeAttachmentUsage::DepthStencil;
                depthSlot.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
                depthSlot.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;

                opaquePass->AddSlot(depthSlot);

                RPI::PassAttachmentBinding depthSlotBinding{};
                depthSlotBinding.name = "DepthInput";
                depthSlotBinding.slotType = RPI::PassSlotType::Input;
                depthSlotBinding.attachmentUsage = RHI::ScopeAttachmentUsage::DepthStencil;
                depthSlotBinding.connectedBinding = depthPass->FindOutputBinding("DepthOutput");

                opaquePass->AddAttachmentBinding(depthSlotBinding);
            }

            // ColorMSAA
            {
	            RPI::PassSlot colorSlot{};
                colorSlot.name = "ColorMSAA";
                colorSlot.slotType = RPI::PassSlotType::InputOutput;
                colorSlot.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
                colorSlot.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
                colorSlot.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;

                opaquePass->AddSlot(colorSlot);

                RPI::PassAttachmentBinding colorBinding{};
                colorBinding.name = "ColorMSAA";
                colorBinding.slotType = RPI::PassSlotType::InputOutput;
                colorBinding.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
                colorBinding.connectedBinding = skyboxPass->FindOutputBinding("ColorOutput");

                opaquePass->AddAttachmentBinding(colorBinding);
            }

            // DirectionalShadowMap
            {
                RPI::PassSlot shadowMapSlot{};
                shadowMapSlot.name = "DirectionalShadowMap";
                shadowMapSlot.slotType = RPI::PassSlotType::Input;
                shadowMapSlot.attachmentUsage = ScopeAttachmentUsage::Shader;
                shadowMapSlot.loadStoreAction.loadAction = AttachmentLoadAction::Load;
                shadowMapSlot.loadStoreAction.storeAction = AttachmentStoreAction::Store;
                shadowMapSlot.shaderInputName = "_DirectionalShadowMap";

                opaquePass->AddSlot(shadowMapSlot);

                RPI::PassAttachmentBinding shadowMapBinding{};
                shadowMapBinding.name = "DirectionalShadowMap";
                shadowMapBinding.slotType = RPI::PassSlotType::Input;
                shadowMapBinding.attachmentUsage = ScopeAttachmentUsage::Shader;
                shadowMapBinding.connectedBinding = directionalShadowPass->FindOutputBinding("DepthOutput");

                opaquePass->AddAttachmentBinding(shadowMapBinding);
            }

            rootPass->AddChild(opaquePass);
	    }

        // - Resolve Pass -

        auto resolvePass = (RPI::RasterPass*)RPI::PassSystem::Get().CreatePass(this, "ResolvePass");
    	resolvePass->SetViewTag(mainViewTag);
	    {
		    {
                RPI::PassAttachmentBinding colorBinding{};
                colorBinding.name = "ColorMSAA";
                colorBinding.slotType = RPI::PassSlotType::InputOutput;
                colorBinding.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
                colorBinding.connectedBinding = opaquePass->FindInputOutputBinding("ColorMSAA");
                colorBinding.fallbackBinding = nullptr;

                resolvePass->AddAttachmentBinding(colorBinding);
		    }

		    {
                RPI::PassAttachmentBinding resolveBinding{};
                resolveBinding.name = "Resolve";
                resolveBinding.slotType = RPI::PassSlotType::Output;
                resolveBinding.attachmentUsage = RHI::ScopeAttachmentUsage::Resolve;
                resolveBinding.attachment = pipelineOutput;
                resolveBinding.connectedBinding = resolveBinding.fallbackBinding = nullptr;

                resolvePass->AddAttachmentBinding(resolveBinding);
		    }

            rootPass->AddChild(resolvePass);
	    }

    	// - Test Compute Pass -

    	auto assetManager = AssetManager::Get();
    	Ref<CE::ComputeShader> computeShader = assetManager->LoadAssetAtPath<CE::ComputeShader>("/Engine/Assets/Sandbox/TestCompute");

    	if (false)
    	{
    		auto computePass = CreateObject<RPI::ComputePass>(this, "TestComputePass");
    		{
    			Vec3i invocationSize = computeShader->GetReflection().invocationSize;

    			computePass->SetShader(computeShader->GetRpiShader(0));
    			computePass->dispatchSizeSource.source = pipelineOutput->name;
    			computePass->dispatchSizeSource.sizeMultipliers = Vec3(1.0f / invocationSize.x, 1.0f / invocationSize.y, 1.0f / invocationSize.z);

    			// _Texture
    			{
    				RPI::PassSlot textureSlot{};
    				textureSlot.name = "Texture";
    				textureSlot.slotType = RPI::PassSlotType::InputOutput;
    				textureSlot.shaderInputName = "_TextureTest";
    				textureSlot.attachmentUsage = ScopeAttachmentUsage::Shader;
    				textureSlot.loadStoreAction.loadAction = AttachmentLoadAction::Load;
    				textureSlot.loadStoreAction.storeAction = AttachmentStoreAction::Store;

    				computePass->AddSlot(textureSlot);

    				RPI::PassAttachmentBinding textureBinding{};
    				textureBinding.name = "Texture";
    				textureBinding.slotType = RPI::PassSlotType::InputOutput;
    				textureBinding.attachmentUsage = ScopeAttachmentUsage::Shader;
    				textureBinding.connectedBinding = resolvePass->FindOutputBinding("Resolve");
    				textureBinding.fallbackBinding = nullptr;

    				computePass->AddAttachmentBinding(textureBinding);
    			}

    			rootPass->AddChild(computePass);
    		}
    	}

        // -------------------------------
        // Apply Shader Layout (SRG_PerPass)
        // -------------------------------

        Ref<CE::Shader> standardShader = gEngine->GetAssetManager()->LoadAssetAtPath<CE::Shader>("/Engine/Assets/Shaders/PBR/Standard");
        RPI::ShaderCollection* shaderCollection = standardShader->GetShaderCollection();

        renderPipeline->ApplyShaderLayout(shaderCollection);
    }

} // namespace CE
