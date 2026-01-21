
#include "CoreApplication.h"

#if PAL_TRAIT_METAL_SUPPORTED
#   include "MetalRHI.h"
#endif
#if PAL_TRAIT_VULKAN_SUPPORTED
#   include "VulkanRHI.h"
#endif

#include "CoreShader.h"

#include "CoreRHITest.h"

#include <gtest/gtest.h>

using namespace CE;

#define WINDOW_TEST_BEGIN WindowTestBegin()
#define WINDOW_TEST_END WindowTestEnd()

static void WindowTestBegin()
{
    gProjectName = MODULE_NAME;
    gProjectPath = PlatformDirectories::GetLaunchDir();

    ModuleManager::Get().LoadModule("Core");
    ModuleManager::Get().LoadModule("CoreApplication");
    ModuleManager::Get().LoadModule("CoreMedia");
    ModuleManager::Get().LoadModule("CoreRHI");
#if PAL_TRAIT_METAL_SUPPORTED
    ModuleManager::Get().LoadModule("MetalRHI");
#endif
#if PAL_TRAIT_VULKAN_SUPPORTED
    ModuleManager::Get().LoadModule("VulkanRHI");
#endif
    ModuleManager::Get().LoadModule("CoreShader");

    auto app = PlatformApplication::Get();
    app->Initialize();
    
    InputManager::Get().Initialize(app);

    gDefaultWindowWidth = 1000;
    gDefaultWindowHeight = 600;
    
    PlatformWindowInfo windowInfo{
        .maximised = false,
        .fullscreen = false,
        .resizable = true,
        .hidden = false,
        .windowFlags = PlatformWindowFlags::DestroyOnClose
    };
    
    PlatformWindow* mainWindow = app->InitMainWindow(MODULE_NAME, gDefaultWindowWidth, gDefaultWindowHeight, windowInfo);

#if PAL_TRAIT_METAL_SUPPORTED
    RHI::gDynamicRHI = new CE::Metal::MetalRHI();
#endif
#if PAL_TRAIT_VULKAN_SUPPORTED
    RHI::gDynamicRHI = new CE::Vulkan::VulkanRHI();
#endif
    //RHI::gDynamicRHI->AddValidationCallbackHandler(OnValidationMessage, RHI::ValidationMessageType::Warning);

    RHI::gDynamicRHI->Initialize();
    RHI::gDynamicRHI->PostInitialize();
}

static void WindowTestEnd()
{
    auto app = PlatformApplication::Get();

    RHI::gDynamicRHI->PreShutdown();

    InputManager::Get().Shutdown(app);
    app->PreShutdown();

    RHI::gDynamicRHI->Shutdown();

    app->Shutdown();

    delete RHI::gDynamicRHI;
    RHI::gDynamicRHI = nullptr;

    delete app;
    app = nullptr;

    ModuleManager::Get().UnloadModule("CoreShader");
#if PAL_TRAIT_METAL_SUPPORTED
    ModuleManager::Get().UnloadModule("MetalRHI");
#endif
#if PAL_TRAIT_VULKAN_SUPPORTED
    ModuleManager::Get().UnloadModule("VulkanRHI");
#endif
    ModuleManager::Get().UnloadModule("CoreRHI");
    ModuleManager::Get().UnloadModule("CoreMedia");
    ModuleManager::Get().UnloadModule("CoreApplication");
    ModuleManager::Get().UnloadModule("Core");
}

struct alignas(16) ViewDataConstants
{
    Matrix4x4 viewProjectionMatrix;
};

struct alignas(16) ObjectDataConstants
{
    Matrix4x4 modelMatrix;
};

constexpr const char RHI_Triangle_Shader[] = R"(
#include "Core/Macros.hlsli"

cbuffer _ViewData : SRG_PerView(b0)
{
    float4x4 viewProjectionMatrix;
};

cbuffer _ObjectData : SRG_PerObject(b0)
{
    float4x4 modelMatrix;
}

struct VSInput
{
    float3 position : POSITION;
    float3 color : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 color : TEXCOORD0;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput o;
	o.position = mul(mul(float4(input.position, 1.0), modelMatrix), viewProjectionMatrix);
	//o.position = float4(input.position, 1.0);
    o.color = input.color;
    return o;
}
#endif

#if FRAGMENT

float4 FragMain(PSInput input) : SV_TARGET
{
    return float4(input.color, 1);
}

#endif
)";

TEST(RHI, Triangle)
{
    WINDOW_TEST_BEGIN;
    
    PlatformApplication* app = PlatformApplication::Get();
    PlatformWindow* mainWindow = app->GetMainWindow();
    
    u32 width = 0;
    u32 height = 0;
    
    constexpr u32 kNumFrames = RHI::Limits::MaxSwapChainImageCount;
    
    // - SwapChain -
    
    RHI::SwapChain* swapChain = nullptr;
    {
        RHI::SwapChainDescriptor desc{};
        desc.imageCount = 2;
        desc.frameBufferOnly = true;
        
        swapChain = gDynamicRHI->CreateSwapChain(mainWindow, desc);
    }
    
    // - Shader -
    
    ShaderCompiler compiler{};
    
    ShaderReflection reflection{};
    BinaryBlob vertBlob;
    BinaryBlob fragBlob;
    
    ShaderCompilationInfo compileInfo{};
    compileInfo.includeSearchPaths.Add(PlatformDirectories::GetLaunchDir() / "Engine/Shaders");
    compileInfo.outReflection = &reflection;
    
    compileInfo.stages.Add({});
    compileInfo.stages.GetLast().entryPoint = "VertMain";
    compileInfo.stages.GetLast().stage = RHI::ShaderStage::Vertex;
    compileInfo.stages.GetLast().debugName = "VertMain";
    compileInfo.stages.GetLast().extraArgs.AddRange({
        L"-D", L"COMPILE=1",
        L"-D", L"VERTEX=1",
        L"-fspv-preserve-bindings",
        L"-fspv-preserve-interface"
    });
    compileInfo.stages.GetLast().outByteCode = &vertBlob;
    
    compileInfo.stages.Add({});
    compileInfo.stages.GetLast().entryPoint = "FragMain";
    compileInfo.stages.GetLast().stage = RHI::ShaderStage::Fragment;
    compileInfo.stages.GetLast().debugName = "FragMain";
    compileInfo.stages.GetLast().extraArgs.AddRange({
        L"-D", L"COMPILE=1",
        L"-D", L"FRAGMENT=1",
        L"-fspv-preserve-bindings",
        L"-fspv-preserve-interface"
    });
    compileInfo.stages.GetLast().outByteCode = &fragBlob;
    
    auto result = compiler.CompileAuto(RHI_Triangle_Shader, COUNTOF(RHI_Triangle_Shader), compileInfo);
    String errorMsg = compiler.GetErrorMessage();
    EXPECT_EQ(result, ShaderCompiler::ERR_Success);
    
    RHI::ShaderModuleDescriptor vertShaderDesc{};
    vertShaderDesc.name = "Vertex Shader";
    vertShaderDesc.defaultEntryPoint = "VertMain";
    vertShaderDesc.stage = RHI::ShaderStage::Vertex;
    vertShaderDesc.debugName = vertShaderDesc.name;
    vertShaderDesc.byteCode = vertBlob.GetDataPtr();
    vertShaderDesc.byteSize = vertBlob.GetDataSize();
    
    RHI::ShaderModule* vertShader = gDynamicRHI->CreateShaderModule(vertShaderDesc);
    
    RHI::ShaderModuleDescriptor fragShaderDesc{};
    fragShaderDesc.name = "Fragment Shader";
    fragShaderDesc.defaultEntryPoint = "FragMain";
    fragShaderDesc.stage = RHI::ShaderStage::Fragment;
    fragShaderDesc.debugName = fragShaderDesc.name;
    fragShaderDesc.byteCode = fragBlob.GetDataPtr();
    fragShaderDesc.byteSize = fragBlob.GetDataSize();
    
    RHI::ShaderModule* fragShader = gDynamicRHI->CreateShaderModule(fragShaderDesc);
    
    // - Shader Resources -
    
    RHI::ShaderResourceGroup* perViewSrg = nullptr;
    {
        const RHI::ShaderResourceGroupLayout& perViewSrgLayout = reflection.srgLayouts[0];
        
        RHI::ShaderResourceGroupDescriptor perViewSrgDesc{};
        perViewSrgDesc.name = "SRG_PerView";
        perViewSrgDesc.layout = perViewSrgLayout;
        perViewSrgDesc.shaderHint = vertShader;
        
        perViewSrg = gDynamicRHI->CreateShaderResourceGroup(perViewSrgDesc);
    }

    RHI::ShaderResourceGroup* perObjectSrg = nullptr;
    {
	    const RHI::ShaderResourceGroupLayout& perObjectSrgLayout = reflection.srgLayouts[1];
        
        RHI::ShaderResourceGroupDescriptor perObjectSrgDesc{};
        perObjectSrgDesc.name = "SRG_PerObject";
        perObjectSrgDesc.layout = perObjectSrgLayout;
        perObjectSrgDesc.shaderHint = vertShader;
        
		perObjectSrg = gDynamicRHI->CreateShaderResourceGroup(perObjectSrgDesc);
    }
    
    ViewDataConstants viewData{};
    Matrix4x4 projectionMatrix = Matrix4x4::PerspectiveProjection((f32)swapChain->GetWidth() / (f32)swapChain->GetHeight(), 60, 0.1f, 1000.0f);
    Matrix4x4 viewMatrix = Matrix4x4::Translation(Vec3(0, 0, -5));
	viewData.viewProjectionMatrix = projectionMatrix * viewMatrix;

    ObjectDataConstants objectData{};
	objectData.modelMatrix = Matrix4x4::Translation(Vec3(0, 0, 10)) * Matrix4x4::Scale(Vec3(1, 1, 1) * 5);
    
    StaticArray<RHI::Buffer*, RHI::Limits::MaxSwapChainImageCount> perViewDataBuffers{};
    StaticArray<RHI::Buffer*, RHI::Limits::MaxSwapChainImageCount> perObjectDataBuffers{};
    {
        RHI::BufferDescriptor bufferDesc{};
        bufferDesc.name = "PerScene cbuffer";
        bufferDesc.bufferSize = sizeof(ViewDataConstants);
        bufferDesc.bindFlags = RHI::BufferBindFlags::ConstantBuffer;
        bufferDesc.defaultHeapType = RHI::MemoryHeapType::Upload;
        
		for (u32 i = 0; i < kNumFrames; i++)
		{
			perViewDataBuffers[i] = gDynamicRHI->CreateBuffer(bufferDesc);

            perViewDataBuffers[i]->UploadData(&viewData, sizeof(ViewDataConstants));

            perViewSrg->Bind(i, "_ViewData", perViewDataBuffers[i]);
		}

		bufferDesc.name = "PerObject cbuffer";
		bufferDesc.bufferSize = sizeof(ObjectDataConstants);
		
    	for (u32 i = 0; i < kNumFrames; i++)
		{
			perObjectDataBuffers[i] = gDynamicRHI->CreateBuffer(bufferDesc);

            perObjectDataBuffers[i]->UploadData(&objectData, sizeof(ObjectDataConstants));

            perObjectSrg->Bind(i, "_ObjectData", perObjectDataBuffers[i]);
		}
    }

    perViewSrg->FlushBindings();
	perObjectSrg->FlushBindings();
    
    // - Vertex & Index Buffers -

    RHI::VertexBufferView vertexBufferView{};
    u32 numVertices = 0;
    {
        RHI::Buffer* vertexBuffer = nullptr;
        RHI::BufferDescriptor vertexBufferDesc{};
        
        vertexBufferDesc.name = "Vertex Buffer";
        vertexBufferDesc.bufferSize = sizeof(TriangleVertices);
        vertexBufferDesc.bindFlags = RHI::BufferBindFlags::VertexBuffer;
        vertexBufferDesc.defaultHeapType = RHI::MemoryHeapType::Default;
        
        vertexBuffer = gDynamicRHI->CreateBuffer(vertexBufferDesc);
        
        vertexBuffer->UploadData(TriangleVertices, sizeof(TriangleVertices));
        
        vertexBufferView = RHI::VertexBufferView(vertexBuffer, 0, vertexBuffer->GetBufferSize(), sizeof(TriVertex));
        
        numVertices = COUNTOF(TriangleVertices);
    }
    
    // - Render Pass -
    
    RHI::RenderPassLayout rpLayout{};
    RHI::RenderPass* renderPass = nullptr;
    RHI::RenderPassFrameBuffer* frameBuffer = nullptr;
    {
        {
            RHI::RenderPassAttachmentLayout colorAttachmentLayout{};
            colorAttachmentLayout.format = swapChain->GetSwapChainFormat();
            colorAttachmentLayout.attachmentUsage = RHI::ScopeAttachmentUsage::Color;
            colorAttachmentLayout.multisampleState.sampleCount = 1;
            
            colorAttachmentLayout.loadAction = RHI::AttachmentLoadAction::Clear;
            colorAttachmentLayout.storeAction = RHI::AttachmentStoreAction::Store;
            
            rpLayout.attachmentLayouts.Add(colorAttachmentLayout);
        }
        
        {
            RHI::RenderPassSubpassLayout subpass{};
            subpass.colorAttachments.Add(0);
            
            rpLayout.subpasses.Add(subpass);
        }
        
        renderPass = gDynamicRHI->CreateRenderPass(rpLayout);
        
        RHI::RenderPassFrameBufferDescriptor frameBufferDesc{};
        frameBufferDesc.renderPass = renderPass;
        frameBufferDesc.attachments.Add(RenderPassFrameAttachment(swapChain));

        frameBuffer = gDynamicRHI->CreateRenderPassFrameBuffer(frameBufferDesc);
    }
    
    // - Graphics Pipeline -
    
    RHI::PipelineState* pipeline = nullptr;
    {
        RHI::GraphicsPipelineDescriptor desc{};
        
        desc.blendState.colorBlends.Add({});
        
        desc.depthStencilState.depthState.enable = false;
        desc.depthStencilState.stencilState.enable = false;
        
        desc.rasterState.cullMode = RHI::CullMode::Off;
        desc.rasterState.fillMode = RHI::FillMode::Solid;
        
        desc.multisampleState.sampleCount = 1;
        
        desc.renderPassLayout = rpLayout;
        desc.subpass = 0;
        
        desc.vertexInputSlots.Add({});
        desc.vertexInputSlots[0].inputRate = RHI::VertexInputRate::PerVertex;
        desc.vertexInputSlots[0].inputSlot = 0;
        desc.vertexInputSlots[0].stride = sizeof(TriVertex);
        
        desc.vertexAttributes.Add({});
        desc.vertexAttributes[0].location = 0;
        desc.vertexAttributes[0].offset = 0;
        desc.vertexAttributes[0].dataType = RHI::VertexAttributeDataType::Float3;
        desc.vertexAttributes[0].inputSlot = 0;
        
        desc.vertexAttributes.Add({});
        desc.vertexAttributes[1].location = 1;
        desc.vertexAttributes[1].offset = offsetof(TriVertex, color);
        desc.vertexAttributes[1].dataType = RHI::VertexAttributeDataType::Float3;
        desc.vertexAttributes[1].inputSlot = 0;
        
        desc.srgLayouts.Add(perViewSrg->GetLayout());
        desc.srgLayouts.Add(perObjectSrg->GetLayout());
        
        desc.shaderStages.Add({});
        desc.shaderStages[0].entryPoint = "VertMain";
        desc.shaderStages[0].shaderModule = vertShader;
        
        desc.shaderStages.Add({});
        desc.shaderStages[1].entryPoint = "FragMain";
        desc.shaderStages[1].shaderModule = fragShader;
        
        pipeline = gDynamicRHI->CreateGraphicsPipeline(desc);
    }
    
    // - Command Lists -
    
    RHI::CommandQueue* cmdQueue = gDynamicRHI->GetPrimaryGraphicsQueue();
    RHI::CommandList* cmdLists[kNumFrames] = {};
    {
        for (int i = 0; i < kNumFrames; i++)
        {
            cmdLists[i] = gDynamicRHI->AllocateCommandList(cmdQueue);
            cmdLists[i]->SetFrameIndex(i);
        }
    }
    
    int frameIndex = 0;
    int totalFrames = 0;
    uint64_t frameDoneValue[kNumFrames] = {};
    RHI::Fence* graphicsFence = gDynamicRHI->CreateFence();

    auto renderLoop = [&]
        {
            if (frameDoneValue[frameIndex] != 0)
            {
                graphicsFence->WaitCPU(frameDoneValue[frameIndex]);
            }

			perViewDataBuffers[frameIndex]->UploadData(&viewData, sizeof(ViewDataConstants));
			perObjectDataBuffers[frameIndex]->UploadData(&objectData, sizeof(ObjectDataConstants));

            bool imageAcquired = swapChain->AcquireNextImage();
            if (!imageAcquired)
            {
                return;
            }

            uint64_t done = graphicsFence->NextSignalValue();
            frameDoneValue[frameIndex] = done;

            RHI::CommandQueueSubmission submission{};
            submission.numCommandLists = 1;
            submission.commandLists = &cmdLists[frameIndex];

            submission.signalFence = graphicsFence;
            submission.signalFenceValue = done;

            submission.numPresentSwapChains = 1;
            submission.presentSwapChains = &swapChain;

            auto cmdList = cmdLists[frameIndex];

            cmdList->Begin();
            {
                AttachmentClearValue clearValue{};
                clearValue.clearValue = Vec4(0, 0, 0, 1);

                RHI::ResourceBarrierDescriptor barrier{};

                barrier.resource = swapChain;
                barrier.fromState = ResourceState::Undefined;
                barrier.toState = ResourceState::ColorOutput;
                cmdList->ResourceBarrier(1, &barrier);

                cmdList->BeginRenderPass(renderPass, frameBuffer, &clearValue);
                {
                    RHI::ScissorState scissor{};
                    scissor.x = scissor.y = 0;
                    scissor.width = swapChain->GetWidth();
                    scissor.height = swapChain->GetHeight();
                    cmdList->SetScissors(1, &scissor);

                    RHI::ViewportState viewport{};
                    viewport.x = viewport.y = 0;
                    viewport.minDepth = 0; viewport.maxDepth = 1;
                    viewport.width = scissor.width;
                    viewport.height = scissor.height;
                    cmdList->SetViewports(1, &viewport);

                    cmdList->BindPipelineState(pipeline);

                    cmdList->SetShaderResourceGroups({ perViewSrg, perObjectSrg });
                    cmdList->CommitShaderResources();

                    cmdList->BindVertexBuffers(0, 1, &vertexBufferView);

                    cmdList->DrawLinear(RHI::DrawLinearArguments{
                        .instanceCount = 1,
                        .firstInstance = 0,
                        .vertexCount = numVertices,
                        .vertexOffset = 0
                    });
                }
                cmdList->EndRenderPass();

                barrier.resource = swapChain;
                barrier.fromState = ResourceState::ColorOutput;
                barrier.toState = ResourceState::Present;
                cmdList->ResourceBarrier(1, &barrier);
            }
            cmdList->End();

            cmdQueue->Submit(submission);

            frameIndex = (frameIndex + 1) % kNumFrames;
            totalFrames++;
        };

    // Exposed Tick
    auto handle = app->AddTickHandler(renderLoop);
    
    while (!IsEngineRequestingExit())
    {
        app->Tick();
        InputManager::Get().Tick();

		// - Render Loop -

        renderLoop();
    }

    app->RemoveTickHandler(handle);
    
    // - Cleanup -
    
	for (u32 i = 0; i < kNumFrames; i++)
    {
        delete perViewDataBuffers[i];
        delete perObjectDataBuffers[i];
    }

    delete vertShader; delete fragShader;
    delete swapChain;
    delete renderPass;
    delete perObjectSrg; delete perViewSrg;
    gDynamicRHI->FreeCommandLists(kNumFrames, cmdLists);
    delete pipeline;
    delete graphicsFence;
    delete frameBuffer;
    delete vertexBufferView.GetBuffer();
    
    WINDOW_TEST_END;
}
