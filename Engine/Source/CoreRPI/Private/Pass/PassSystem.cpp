#include "CoreRPI.h"

namespace CE::RPI
{

    void PassSystem::Initialize()
    {
        Bundle* transient = GetTransient(MODULE_NAME);

		// - Pass Templates -

        // - Depth Stencil Pass -
        {
            RasterPass* depthPass = CreateObject<RasterPass>(transient, "DepthStencilPass");

            PassSlot depthStencilOutput{};
            depthStencilOutput.name = "DepthStencilOutput";
            depthStencilOutput.slotType = PassSlotType::Output;
            depthStencilOutput.attachmentUsage = RHI::ScopeAttachmentUsage::DepthStencil;
            depthStencilOutput.dimensions = { RHI::Dimension::Dim2D };
            depthStencilOutput.formats = { RHI::Format::D32_SFLOAT_S8_UINT, RHI::Format::D24_UNORM_S8_UINT, RHI::Format::D16_UNORM_S8_UINT };
            depthStencilOutput.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Clear;
            depthStencilOutput.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
            depthStencilOutput.loadStoreAction.loadActionStencil = RHI::AttachmentLoadAction::Clear;
            depthStencilOutput.loadStoreAction.storeActionStencil = RHI::AttachmentStoreAction::Store;
            depthStencilOutput.loadStoreAction.clearValueDepth = 1.0f;
            depthStencilOutput.loadStoreAction.clearValueStencil = 0;

            depthPass->AddSlot(depthStencilOutput);

            RegisterTemplate(depthPass);
        }

        // - Depth Pass -
        {
            RasterPass* depthPass = CreateObject<RasterPass>(transient, "DepthPass");

            PassSlot depthOutput{};
            depthOutput.name = "DepthOutput";
            depthOutput.slotType = PassSlotType::Output;
            depthOutput.attachmentUsage = RHI::ScopeAttachmentUsage::DepthStencil;
            depthOutput.dimensions = { RHI::Dimension::Dim2D };
            depthOutput.formats = { RHI::Format::D32_SFLOAT, RHI::Format::D32_SFLOAT_S8_UINT, RHI::Format::D24_UNORM_S8_UINT, RHI::Format::D16_UNORM_S8_UINT };
            depthOutput.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Clear;
            depthOutput.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
            depthOutput.loadStoreAction.loadActionStencil = RHI::AttachmentLoadAction::DontCare;
            depthOutput.loadStoreAction.storeActionStencil = RHI::AttachmentStoreAction::DontCare;
            depthOutput.loadStoreAction.clearValueDepth = 1.0f;

            depthPass->AddSlot(depthOutput);

            RegisterTemplate(depthPass);
        }

        // - Directional Shadow Pass -
        {
            RasterPass* shadowPass = CreateObject<RasterPass>(transient, "DirectionalShadowPass");

            PassSlot depthOutput{};
            depthOutput.name = "DepthOutput";
            depthOutput.slotType = PassSlotType::Output;
            depthOutput.attachmentUsage = RHI::ScopeAttachmentUsage::DepthStencil;
            depthOutput.dimensions = { RHI::Dimension::Dim2D };
            depthOutput.formats = { RHI::Format::D32_SFLOAT, RHI::Format::D16_UNORM };
            depthOutput.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Clear;
            depthOutput.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
            depthOutput.loadStoreAction.loadActionStencil = RHI::AttachmentLoadAction::DontCare;
            depthOutput.loadStoreAction.storeActionStencil = RHI::AttachmentStoreAction::DontCare;
            depthOutput.loadStoreAction.clearValueDepth = 1.0f;

            shadowPass->AddSlot(depthOutput);

            RegisterTemplate(shadowPass);
        }

        // - Skybox Pass -
        {
            RasterPass* skyboxPass = CreateObject<RasterPass>(transient, "SkyboxPass");

            PassSlot colorOutput{};
            colorOutput.name = "ColorOutput";
            colorOutput.slotType = PassSlotType::Output;
            colorOutput.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
            colorOutput.dimensions = {RHI::Dimension::Dim2D };
            colorOutput.formats = {};
            colorOutput.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Clear;
            colorOutput.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
            colorOutput.loadStoreAction.clearValue = Vec4(0, 0, 0, 0);

            skyboxPass->AddSlot(colorOutput);

            RegisterTemplate(skyboxPass);
        }

        // - Opaque Pass -
        {
            RasterPass* opaquePass = CreateObject<RasterPass>(transient, "OpaquePass");

            PassSlot depthInput{};
            depthInput.name = "DepthInput";
            depthInput.slotType = PassSlotType::Input;
            depthInput.attachmentUsage = RHI::ScopeAttachmentUsage::DepthStencil;
            depthInput.dimensions = {RHI::Dimension::Dim2D };
            depthInput.formats = { RHI::Format::D32_SFLOAT, RHI::Format::D32_SFLOAT_S8_UINT, RHI::Format::D24_UNORM_S8_UINT, RHI::Format::D16_UNORM_S8_UINT };
            depthInput.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
            depthInput.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
            depthInput.loadStoreAction.loadActionStencil = RHI::AttachmentLoadAction::DontCare;
            depthInput.loadStoreAction.storeActionStencil = RHI::AttachmentStoreAction::DontCare;
            
            PassSlot colorOutput{};
            colorOutput.name = "ColorOutput";
            colorOutput.slotType = PassSlotType::InputOutput;
            colorOutput.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
            colorOutput.dimensions = {RHI::Dimension::Dim2D };
            colorOutput.formats = { };
            colorOutput.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
            colorOutput.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;

            opaquePass->AddSlot(depthInput);
            opaquePass->AddSlot(colorOutput);

            RegisterTemplate(opaquePass);
        }

        // - Resolve Pass -
        {
            RasterPass* resolvePass = CreateObject<RasterPass>(transient, "ResolvePass");

            PassSlot colorSlot{};
            colorSlot.name = "ColorMSAA";
            colorSlot.slotType = PassSlotType::InputOutput;
            colorSlot.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
            colorSlot.dimensions = {RHI::Dimension::Dim2D };
            colorSlot.formats = {};
            colorSlot.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
            colorSlot.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;

            PassSlot resolveSlot{};
            resolveSlot.name = "Resolve";
            resolveSlot.slotType = PassSlotType::Output;
            resolveSlot.attachmentUsage = RHI::ScopeAttachmentUsage::Resolve;
            resolveSlot.dimensions = {RHI::Dimension::Dim2D };
            resolveSlot.formats = {};
            resolveSlot.loadStoreAction.loadAction = RHI::AttachmentLoadAction::DontCare;
            resolveSlot.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;

            resolvePass->AddSlot(colorSlot);
            resolvePass->AddSlot(resolveSlot);

            RegisterTemplate(resolvePass);
        }

        // - Texture to Texture Copy Pass -
        {
            CopyPass* copyPass = CreateObject<CopyPass>(transient, "CopyPass");

			PassSlot inputSlot{};
            inputSlot.name = "Input";
            inputSlot.slotType = PassSlotType::Input;
            inputSlot.attachmentUsage = RHI::ScopeAttachmentUsage::Copy;
            inputSlot.dimensions = { RHI::Dimension::Dim2D };
            inputSlot.formats = {};
            inputSlot.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
            inputSlot.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;

            PassSlot outputSlot{};
            outputSlot.name = "Output";
            outputSlot.slotType = PassSlotType::Output;
            outputSlot.attachmentUsage = RHI::ScopeAttachmentUsage::Copy;
            outputSlot.dimensions = { RHI::Dimension::Dim2D };
            outputSlot.formats = {};
            outputSlot.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
            outputSlot.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;

            copyPass->AddSlot(inputSlot);
            copyPass->AddSlot(outputSlot);

            RegisterTemplate(copyPass);
        }
    }

    void PassSystem::Shutdown()
    {
        for (auto [name, pass] : passTemplates)
        {
            if (pass)
				pass->BeginDestroy();
        }
        passTemplates.Clear();
    }

    PassSystem& PassSystem::Get()
    {
        static PassSystem instance{};
        return instance;
    }

    void PassSystem::RegisterTemplate(Name templateName, Pass* pass)
    {
        if (passTemplates[templateName] != nullptr)
            return;

        passTemplates[templateName] = pass;
    }

    void PassSystem::RegisterTemplate(Pass* pass)
    {
        if (pass)
        {
            RegisterTemplate(pass->GetName(), pass);
        }
    }

    Pass* PassSystem::CreatePass(Name templateName, Name newPassName)
    {
        return CreatePass(GetTransient(MODULE_NAME), templateName, newPassName);
    }
    
    Pass* PassSystem::CreatePass(Object* outer, Name templateName, Name newPassName)
    {
        if (passTemplates[templateName] == nullptr)
            return nullptr;

        if (!newPassName.IsValid())
            newPassName = templateName;

        Pass* templatePass = passTemplates[templateName];

        Pass* newPass = CreateObject<Pass>(outer, newPassName.GetString(), OF_NoFlags, templatePass->GetClass());
        newPass->drawListTag = templatePass->drawListTag;
        newPass->viewTag = templatePass->viewTag;
        newPass->slots = templatePass->slots;

        return newPass;
    }

} // namespace CE::RPI
