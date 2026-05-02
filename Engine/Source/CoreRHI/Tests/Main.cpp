
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
    gIsEngineRequestingExit = false;

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

static constexpr u32 kNumFrames = RHI::Limits::MaxSwapChainImageCount;

namespace RHI_Triangle
{

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

            perViewSrg = gDynamicRHI->CreateShaderResourceGroup(perViewSrgDesc);
        }

        RHI::ShaderResourceGroup* perObjectSrg = nullptr;
        {
            const RHI::ShaderResourceGroupLayout& perObjectSrgLayout = reflection.srgLayouts[1];

            RHI::ShaderResourceGroupDescriptor perObjectSrgDesc{};
            perObjectSrgDesc.name = "SRG_PerObject";
            perObjectSrgDesc.layout = perObjectSrgLayout;

            perObjectSrg = gDynamicRHI->CreateShaderResourceGroup(perObjectSrgDesc);
        }

        ViewDataConstants viewData{};

        Matrix4x4 projectionMatrix = Matrix4x4::PerspectiveProjection((f32)swapChain->GetWidth() / (f32)swapChain->GetHeight(), 60, 0.1f, 1000.0f);
        projectionMatrix[1][1] *= gDynamicRHI->GetClipSpaceSignY();

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

        f32 deltaTime = 0.0f;
        f32 eulerY = 0;

        auto renderLoop = [&]
            {
                if (frameDoneValue[frameIndex] != 0)
                {
                    graphicsFence->WaitCPU(frameDoneValue[frameIndex]);
                }

                projectionMatrix = Matrix4x4::PerspectiveProjection((f32)swapChain->GetWidth() / (f32)swapChain->GetHeight(), 60, 0.1f, 1000.0f);
                projectionMatrix[1][1] *= gDynamicRHI->GetClipSpaceSignY();

                viewMatrix = Matrix4x4::Translation(Vec3(0, 0, -5));
                viewData.viewProjectionMatrix = projectionMatrix * viewMatrix;

                eulerY += deltaTime * 5;

                objectData.modelMatrix = Matrix4x4::Translation(Vec3(0, 0, 10)) * Quat::EulerRadians(0, eulerY, 0).ToMatrix() * Matrix4x4::Scale(Vec3(1, 1, 1) * 5);

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

                        cmdList->BindVertexBuffers(0, 1, &vertexBufferView);
                        
                        cmdList->CommitShaderResources();

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

        clock_t prevTime = clock();

        while (!IsEngineRequestingExit())
        {
            auto curTime = clock();
            deltaTime = (f32)(curTime - prevTime) / CLOCKS_PER_SEC;

            app->Tick();
            InputManager::Get().Tick();

            // - Render Loop -

            renderLoop();

            prevTime = curTime;
        }

        app->RemoveTickHandler(handle);

        // - Cleanup -

        gDynamicRHI->WaitToShutdown();

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

}

namespace RHI_FrameGraph
{
    struct alignas(16) ViewDataConstants
    {
        Matrix4x4 viewProjectionMatrix;
    };

    struct alignas(16) ObjectDataConstants
    {
        Matrix4x4 modelMatrix;
    };

    constexpr const char RHI_FrameGraph_Shader[] = R"(
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

    TEST(RHI, FrameGraph)
    {
        WINDOW_TEST_BEGIN;

        PlatformApplication* app = PlatformApplication::Get();
        PlatformWindow* mainWindow = app->GetMainWindow();

        // - SwapChain -

        const RHI::Format depthFormat = gDynamicRHI->GetAvailableDepthOnlyFormats()[0];

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
            L"-fspv-preserve-interface",
            L"-fspv-debug=vulkan-with-source",
            L"-Zi"
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
            L"-fspv-preserve-interface",
            L"-fspv-debug=vulkan-with-source",
            L"-Zi"
            });
        compileInfo.stages.GetLast().outByteCode = &fragBlob;

        auto result = compiler.CompileAuto(RHI_FrameGraph_Shader, COUNTOF(RHI_FrameGraph_Shader), compileInfo);
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

            perViewSrg = gDynamicRHI->CreateShaderResourceGroup(perViewSrgDesc);
        }

        RHI::ShaderResourceGroup* perObjectSrg = nullptr;
        {
            const RHI::ShaderResourceGroupLayout& perObjectSrgLayout = reflection.srgLayouts[1];

            RHI::ShaderResourceGroupDescriptor perObjectSrgDesc{};
            perObjectSrgDesc.name = "SRG_PerObject";
            perObjectSrgDesc.layout = perObjectSrgLayout;

            perObjectSrg = gDynamicRHI->CreateShaderResourceGroup(perObjectSrgDesc);
        }

        ViewDataConstants viewData{};

        Matrix4x4 projectionMatrix = Matrix4x4::PerspectiveProjection((f32)swapChain->GetWidth() / (f32)swapChain->GetHeight(), 60, 0.1f, 1000.0f);
		projectionMatrix[1][1] *= gDynamicRHI->GetClipSpaceSignY(); // Metal API needs Y flip

        Matrix4x4 viewMatrix = Matrix4x4::Translation(Vec3(0, 0, -5));
        viewData.viewProjectionMatrix = projectionMatrix * viewMatrix;

        ObjectDataConstants objectData{};
        objectData.modelMatrix = Matrix4x4::Translation(Vec3(0, 0, 20)) * Matrix4x4::Scale(Vec3(1, 1, 1) * 5);

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

        // - Geometry -
		
    	Array<RHI::VertexBufferView> vertexBufferViews{};
        {
            RHI::BufferDescriptor bufferDesc{};
			bufferDesc.name = "Vertex Buffer";
            bufferDesc.bindFlags = BufferBindFlags::VertexBuffer;
            bufferDesc.defaultHeapType = MemoryHeapType::Default;

            bufferDesc.bufferSize = sizeof(CubeVertices);
            bufferDesc.structureByteStride = sizeof(Vec4);
            vertexBufferViews.Add(RHI::VertexBufferView(gDynamicRHI->CreateBuffer(bufferDesc), 0, bufferDesc.bufferSize, bufferDesc.structureByteStride));
            vertexBufferViews.GetLast().GetBuffer()->UploadData(CubeVertices, sizeof(CubeVertices));

            bufferDesc.bufferSize = sizeof(CubeVertexColors);
			bufferDesc.structureByteStride = sizeof(Vec4);
            vertexBufferViews.Add(RHI::VertexBufferView(gDynamicRHI->CreateBuffer(bufferDesc), 0, bufferDesc.bufferSize, bufferDesc.structureByteStride));
            vertexBufferViews.GetLast().GetBuffer()->UploadData(CubeVertexColors, sizeof(CubeVertexColors));
        }

        RHI::IndexBufferView indexBufferView{};
        u32 numIndices = 0;
        {
	        RHI::BufferDescriptor bufferDesc{};
            bufferDesc.name = "Index Buffer";
            bufferDesc.bindFlags = RHI::BufferBindFlags::IndexBuffer;
            bufferDesc.defaultHeapType = RHI::MemoryHeapType::Default;
            
        	bufferDesc.bufferSize = sizeof(CubeIndices);
            bufferDesc.structureByteStride = sizeof(u16);

            RHI::Buffer* indexBuffer = gDynamicRHI->CreateBuffer(bufferDesc);
            indexBuffer->UploadData(CubeIndices, sizeof(CubeIndices));
			indexBufferView = RHI::IndexBufferView(indexBuffer, 0, indexBuffer->GetBufferSize(), RHI::IndexFormat::Uint16);

			numIndices = COUNTOF(CubeIndices);
        }

        // - Graphics Pipeline -

        RHI::PipelineState* depthPipeline = nullptr;
        RHI::PipelineState* colorPipeline = nullptr;
        {
            RHI::GraphicsPipelineDescriptor desc{};

            desc.blendState.colorBlends.Add({});

            desc.depthStencilState.depthState.enable = true;
            desc.depthStencilState.depthState.writeEnable = true;
            desc.depthStencilState.depthState.testEnable = true;
            desc.depthStencilState.depthState.compareOp = RHI::CompareOp::LEqual;

            desc.depthStencilState.stencilState.enable = false;

            desc.rasterState.cullMode = RHI::CullMode::Off;
            desc.rasterState.fillMode = RHI::FillMode::Solid;

            desc.multisampleState.sampleCount = 1;

            desc.renderPassLayout.attachmentLayouts.Add(RHI::RenderPassAttachmentLayout{
                .attachmentUsage = ScopeAttachmentUsage::DepthStencil,
                .format = depthFormat,
                .loadAction = AttachmentLoadAction::Clear,
                .storeAction = AttachmentStoreAction::Store,
            });
            desc.subpass = 0;

            desc.vertexInputSlots.Add({});
            desc.vertexInputSlots[0].inputRate = RHI::VertexInputRate::PerVertex;
            desc.vertexInputSlots[0].inputSlot = 0;
            desc.vertexInputSlots[0].stride = sizeof(Vec4);

            desc.vertexInputSlots.Add({});
            desc.vertexInputSlots[1].inputRate = RHI::VertexInputRate::PerVertex;
            desc.vertexInputSlots[1].inputSlot = 1;
            desc.vertexInputSlots[1].stride = sizeof(Vec4);

            desc.vertexAttributes.Add({});
            desc.vertexAttributes[0].location = 0;
            desc.vertexAttributes[0].offset = 0;
            desc.vertexAttributes[0].dataType = RHI::VertexAttributeDataType::Float3;
            desc.vertexAttributes[0].inputSlot = 0;

            desc.vertexAttributes.Add({});
            desc.vertexAttributes[1].location = 1;
            desc.vertexAttributes[1].offset = 0;
            desc.vertexAttributes[1].dataType = RHI::VertexAttributeDataType::Float3;
            desc.vertexAttributes[1].inputSlot = 1;

            desc.srgLayouts.Add(perViewSrg->GetLayout());
            desc.srgLayouts.Add(perObjectSrg->GetLayout());

            desc.shaderStages.Add({});
            desc.shaderStages[0].entryPoint = "VertMain";
            desc.shaderStages[0].shaderModule = vertShader;

            desc.shaderStages.Add({});
            desc.shaderStages[1].entryPoint = "FragMain";
            desc.shaderStages[1].shaderModule = fragShader;

            desc.name = "DepthPipeline";
            depthPipeline = gDynamicRHI->CreateGraphicsPipeline(desc);

			// Color Pipeline

            desc.depthStencilState.depthState.enable = true;
            desc.depthStencilState.depthState.writeEnable = false;
            desc.depthStencilState.depthState.testEnable = true;

            desc.renderPassLayout.attachmentLayouts.Clear();
            desc.renderPassLayout.attachmentLayouts.Add(RHI::RenderPassAttachmentLayout{
                .attachmentUsage = ScopeAttachmentUsage::Color,
                .format = swapChain->GetSwapChainFormat(),
                .loadAction = AttachmentLoadAction::Clear,
                .storeAction = AttachmentStoreAction::Store,
            });
            desc.renderPassLayout.attachmentLayouts.Add(RHI::RenderPassAttachmentLayout{
                .attachmentUsage = ScopeAttachmentUsage::DepthStencil,
                .format = depthFormat,
                .loadAction = AttachmentLoadAction::Load,
                .storeAction = AttachmentStoreAction::Store,
            });
            desc.subpass = 0;

            desc.name = "ColorPipeline";
			colorPipeline = gDynamicRHI->CreateGraphicsPipeline(desc);
        }

		// - Draw List -

        RHI::DrawListContext drawList{};
        DrawListTagRegistry tagRegistry{};
        RHI::DrawListTag depthDrawTag = tagRegistry.AcquireTag("depth");
        RHI::DrawListTag colorDrawTag = tagRegistry.AcquireTag("color");

        RHI::DrawListMask drawListMask{};
        drawListMask.Set(depthDrawTag);
        drawListMask.Set(colorDrawTag);

        Ptr<RHI::DrawPacket> cubeDrawPacket;
        {
            RHI::DrawPacketBuilder drawPacketBuilder{};

            drawPacketBuilder.SetDebugName("Cube");

            drawPacketBuilder.AddShaderResourceGroup(perObjectSrg);

            drawPacketBuilder.AddDrawItem({
                .stencilRef = 0,
                .drawItemTag = depthDrawTag,
                .indexBufferView = indexBufferView,
                .vertexBufferViews = vertexBufferViews,
                .pipelineState = depthPipeline,
                .drawFilterMask = DrawFilterMask::ALL
            });

            drawPacketBuilder.AddDrawItem({
                .stencilRef = 0,
                .drawItemTag = colorDrawTag,
                .indexBufferView = indexBufferView,
                .vertexBufferViews = vertexBufferViews,
                .pipelineState = colorPipeline,
                .drawFilterMask = DrawFilterMask::ALL
            });

            drawPacketBuilder.SetDrawArguments(RHI::DrawIndexedArguments{
                .instanceCount = 1,
                .firstInstance = 0,
                .vertexOffset = 0,
                .indexCount = numIndices,
                .firstIndex = 0
            });

            cubeDrawPacket = drawPacketBuilder.Build();
        }

        // - Frame Scheduler -

        RHI::FrameSchedulerDescriptor desc{};
        desc.numFramesInFlight = 2;
        RHI::FrameScheduler* scheduler = RHI::FrameScheduler::Create(desc);

        constexpr const char* kColorAttachmentId = "SwapChain";
        constexpr const char* kDepthAttachmentId = "DepthBuffer";

		constexpr const char* kDepthPrePassId = "DepthPrePass";
		constexpr const char* kColorPassId = "ColorPass";

        bool rebuildFrameGraph = true;

        auto buildFrameGraph = [&]
            {
        		scheduler->BeginFrameGraph();
                {
                    RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();

					// 1. Emplace Attachments

                    attachmentDatabase.EmplaceFrameAttachment(kColorAttachmentId, swapChain);

                    attachmentDatabase.EmplaceFrameAttachment(kDepthAttachmentId, RHI::ImageDescriptor{
                        .name = "DepthTexture",
                        .width = swapChain->GetWidth(),
                        .height = swapChain->GetHeight(),
                        .depth = 1,
                        .dimension = Dimension::Dim2D,
                        .format = gDynamicRHI->GetAvailableDepthOnlyFormats()[0],
                        .mipLevels = 1,
                        .sampleCount = 1,
                        .arrayLayers = 1,
                        .bindFlags = TextureBindFlags::Depth,
                        .defaultHeapType = MemoryHeapType::Default
                    });

                    // 2. Create Scopes

                    scheduler->BeginScope(kDepthPrePassId);
                    {
	                    RHI::ImageScopeAttachmentDescriptor depthAttachment{};
                        depthAttachment.attachmentId = kDepthAttachmentId;
                        depthAttachment.loadStoreAction.clearValueDepth = 1.0f;
                        depthAttachment.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Clear;
                        depthAttachment.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
                        depthAttachment.multisampleState.sampleCount = 1;
                        scheduler->UseAttachment(depthAttachment, RHI::ScopeAttachmentUsage::DepthStencil, RHI::ScopeAttachmentAccess::ReadWrite);
                        
                    	scheduler->UseShaderResourceGroup(perViewSrg);
						
                        scheduler->UsePipeline(depthPipeline);
                    }
                    scheduler->EndScope();

                    scheduler->BeginScope(kColorPassId);
                    {
                        RHI::ImageScopeAttachmentDescriptor swapChainAttachment{};
                        swapChainAttachment.attachmentId = kColorAttachmentId;
                        swapChainAttachment.loadStoreAction.clearValue = Vec4(0, 0, 0.2f, 1);
                        swapChainAttachment.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Clear;
                        swapChainAttachment.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
                        scheduler->UseAttachment(swapChainAttachment, RHI::ScopeAttachmentUsage::Color, RHI::ScopeAttachmentAccess::ReadWrite);

						RHI::ImageScopeAttachmentDescriptor depthAttachment{};
						depthAttachment.attachmentId = kDepthAttachmentId;
						depthAttachment.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
						depthAttachment.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
                        scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);

                        scheduler->UseShaderResourceGroup(perViewSrg);

                        scheduler->UsePipeline(colorPipeline);

                        scheduler->PresentSwapChain(swapChain);
                    }
					scheduler->EndScope();
                }
                EXPECT_TRUE(scheduler->EndFrameGraph());

				scheduler->Compile();

                rebuildFrameGraph = false;
            };

        // - Main Loop -

        clock_t prevTime = clock();
        f32 deltaTime = 0;
        f32 eulerY = 0;

        auto renderLoop = [&]
            {
                if (rebuildFrameGraph)
                {
                    buildFrameGraph();
                }

                u32 frameIndex = scheduler->BeginExecution();
                {
                    if ((frameIndex >= RHI::Limits::MaxSwapChainImageCount) || rebuildFrameGraph)
                    {
                        rebuildFrameGraph = true;
                        return;
                    }
                    
                    projectionMatrix = Matrix4x4::PerspectiveProjection((f32)swapChain->GetWidth() / (f32)swapChain->GetHeight(), 60, 0.1f, 1000.0f);
                    projectionMatrix[1][1] *= gDynamicRHI->GetClipSpaceSignY();

                    viewMatrix = Matrix4x4::Translation(Vec3(0, 0, -5));
                    viewData.viewProjectionMatrix = projectionMatrix * viewMatrix;

					eulerY += deltaTime;
					objectData.modelMatrix = Matrix4x4::Translation(Vec3(0, 0, 20)) * Quat::EulerRadians(0, eulerY, 0).ToMatrix() * Matrix4x4::Scale(Vec3(1, 1, 1) * 5);

					perViewDataBuffers[frameIndex]->UploadData(&viewData, sizeof(ViewDataConstants));
					perObjectDataBuffers[frameIndex]->UploadData(&objectData, sizeof(ObjectDataConstants));

                    drawList.Shutdown();

                    drawList.Init(drawListMask);

                    // Add Draw Packets to drawList
                    drawList.AddDrawPacket(cubeDrawPacket);

                    drawList.Finalize();

                    auto& depthDrawList = drawList.GetDrawListForTag(depthDrawTag);
                    scheduler->SetScopeDrawList(kDepthPrePassId, &depthDrawList);

					auto& colorDrawList = drawList.GetDrawListForTag(colorDrawTag);
					scheduler->SetScopeDrawList(kColorPassId, &colorDrawList);
                }
                scheduler->EndExecution();
            };

		// - Message Handler -

        class MessageHandler : public ApplicationMessageHandler
        {
        public:
			MessageHandler(bool& rebuildFrameGraph) : rebuildFrameGraph(rebuildFrameGraph)
			{}

            void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override
            {
                rebuildFrameGraph = true;
			}

            void OnWindowExposed(PlatformWindow* window) override
            {
                rebuildFrameGraph = true;
            }

        private:

            bool& rebuildFrameGraph;
        };

		MessageHandler msgHandler{rebuildFrameGraph};

        app->AddMessageHandler(&msgHandler);
        auto handle = app->AddTickHandler(renderLoop);

        while (!IsEngineRequestingExit())
        {
            auto curTime = clock();
            deltaTime = (f32)(curTime - prevTime) / CLOCKS_PER_SEC;

            app->Tick();
            InputManager::Get().Tick();

            // - Render Loop -

            renderLoop();

            prevTime = curTime;
        }

        app->RemoveTickHandler(handle);
        app->RemoveMessageHandler(&msgHandler);

		// - Cleanup -

        tagRegistry.ReleaseTag(depthDrawTag);
        tagRegistry.ReleaseTag(colorDrawTag);

		gDynamicRHI->WaitToShutdown();
        
        delete scheduler;

		for (int i = 0; i < kNumFrames; i++)
        {
            delete perViewDataBuffers[i];
            delete perObjectDataBuffers[i];
        }

		for (int i = 0; i < vertexBufferViews.GetSize(); i++)
        {
            delete vertexBufferViews[i].GetBuffer();
        }
        delete indexBufferView.GetBuffer();
        cubeDrawPacket = nullptr;

        delete swapChain;
		delete vertShader; delete fragShader;
        delete perViewSrg; delete perObjectSrg;
        delete colorPipeline; delete depthPipeline;

        WINDOW_TEST_END;
    }

static void TestBegin(bool gui = false)
{
	ModuleManager::Get().LoadModule("Core");
	ModuleManager::Get().LoadModule("CoreApplication");
	ModuleManager::Get().LoadModule("CoreMedia");
	ModuleManager::Get().LoadModule("CoreRHI");

#if PAL_TRAIT_VULKAN_SUPPORTED
	ModuleManager::Get().LoadModule("VulkanRHI");
	RHI::gDynamicRHI = new Vulkan::VulkanRHI;
#endif

	if (gui)
	{
		auto app = PlatformApplication::Get();
		app->Initialize();

		InputManager::Get().Initialize(app);

		app->InitMainWindow("Test", 1280, 720, PlatformWindowInfo{
			false, false, true, false, 0, true, {}, PlatformWindowFlags::DestroyOnClose
		});
	}

	RHI::gDynamicRHI->Initialize();
	RHI::gDynamicRHI->PostInitialize();

	RHI::FrameSchedulerDescriptor desc{ 2 };

	[[maybe_unused]] RHI::FrameScheduler* scheduler = RHI::FrameScheduler::Create(desc);
}

static void TestEnd()
{
	auto scheduler = RHI::FrameScheduler::Get();
	delete scheduler;

	RHI::gDynamicRHI->PreShutdown();
	RHI::gDynamicRHI->Shutdown();

	if (PlatformApplication* app = PlatformApplication::TryGet())
	{
		InputManager::Get().Shutdown(app);

		app->PreShutdown();
		app->Shutdown();

		delete app;
	}

	delete RHI::gDynamicRHI; RHI::gDynamicRHI = nullptr;

#if PAL_TRAIT_VULKAN_SUPPORTED
	ModuleManager::Get().UnloadModule("VulkanRHI");
#endif
	ModuleManager::Get().UnloadModule("CoreRHI");
	ModuleManager::Get().UnloadModule("CoreApplication");
	ModuleManager::Get().UnloadModule("CoreMedia");
	ModuleManager::Get().UnloadModule("Core");
}

#define TEST_BEGIN(...) using namespace RHI; TestBegin(__VA_ARGS__)
#define TEST_END() TestEnd()

static void BuildSampleRenderPipeline(RHI::FrameScheduler* scheduler, RHI::SwapChain* swapChain)
{
	using namespace RHI;

	scheduler->BeginFrameGraph();
	{
		auto& attachmentDatabase = scheduler->GetAttachmentDatabase();

		const u32 kWidth = swapChain->GetWidth();
		const u32 kHeight = swapChain->GetHeight();

		const Name kDepthAttachment = "_Depth";
		attachmentDatabase.EmplaceFrameAttachment(kDepthAttachment, RHI::ImageDescriptor{
			.name = kDepthAttachment,
			.width = kWidth,
			.height = kHeight,
			.depth = 1,
			.dimension = Dimension::Dim2D,
			.format = Format::D32_SFLOAT,
			.mipLevels = 1,
			.sampleCount = 1,
			.arrayLayers = 1,
			.bindFlags = TextureBindFlags::Depth,
			.defaultHeapType = MemoryHeapType::Default
			});

		const Name kShadowMapAttachment = "_ShadowMap";
		attachmentDatabase.EmplaceFrameAttachment(kShadowMapAttachment, RHI::ImageDescriptor{
			.name = kShadowMapAttachment,
			.width = 1024,
			.height = 1024,
			.depth = 1,
			.dimension = Dimension::Dim2D,
			.format = Format::D32_SFLOAT,
			.mipLevels = 1,
			.sampleCount = 1,
			.arrayLayers = 1,
			.bindFlags = TextureBindFlags::Depth,
			.defaultHeapType = MemoryHeapType::Default
			});

		const Name kColorOutputAttachment = "_ColorOutput";
		attachmentDatabase.EmplaceFrameAttachment(kColorOutputAttachment, swapChain);

		const Name kTiledLightList = "_TiledLightList";
		attachmentDatabase.EmplaceFrameAttachment(kTiledLightList, RHI::BufferDescriptor{
			.name = kTiledLightList,
			.bufferSize = 1_MB,
			.structureByteStride = 1_MB,
			.alignment = 0,
			.bindFlags = BufferBindFlags::StructuredBuffer,
			.defaultHeapType = MemoryHeapType::Default
		});

		scheduler->BeginScope("Depth");
		{
			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;

			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Shadow");
		{
			ImageScopeAttachmentDescriptor shadowAttachment{};
			shadowAttachment.attachmentId = kShadowMapAttachment;

			scheduler->UseAttachment(shadowAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Skybox");
		{
			ImageScopeAttachmentDescriptor colorAttachment{};
			colorAttachment.attachmentId = kColorOutputAttachment;
			
			scheduler->UseAttachment(colorAttachment, ScopeAttachmentUsage::Color, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("TileCulling", ScopeOperation::Compute, HardwareQueueClass::Compute);
		{
			BufferScopeAttachmentDescriptor lightListAttachment{};
			lightListAttachment.attachmentId = kTiledLightList;
			lightListAttachment.shaderInputName = "_LightList";

			scheduler->UseAttachment(lightListAttachment, ScopeAttachmentUsage::Shader, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Opaque");
		{
			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;
			
			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);

			ImageScopeAttachmentDescriptor colorAttachment{};
			colorAttachment.attachmentId = kColorOutputAttachment;

			scheduler->UseAttachment(colorAttachment, ScopeAttachmentUsage::Color, ScopeAttachmentAccess::ReadWrite);

			BufferScopeAttachmentDescriptor lightListAttachment{};
			lightListAttachment.attachmentId = kTiledLightList;
			lightListAttachment.shaderInputName = "_LightList";

			scheduler->UseAttachment(lightListAttachment, ScopeAttachmentUsage::Shader, ScopeAttachmentAccess::Read);

		}
		scheduler->EndScope();

		scheduler->BeginScope("Transparent");
		{
			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;

			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);

			ImageScopeAttachmentDescriptor colorAttachment{};
			colorAttachment.attachmentId = kColorOutputAttachment;

			scheduler->UseAttachment(colorAttachment, ScopeAttachmentUsage::Color, ScopeAttachmentAccess::ReadWrite);
		}
		scheduler->EndScope();
	}
	scheduler->EndFrameGraph();
}

static void BuildRealRenderPipeline(RHI::FrameScheduler* scheduler, RHI::SwapChain* swapChain)
{
	using namespace RHI;

	scheduler->BeginFrameGraph();
	{
		auto& attachmentDatabase = scheduler->GetAttachmentDatabase();

		const u32 kWidth = swapChain->GetWidth();
		const u32 kHeight = swapChain->GetHeight();

		const Name kDepthAttachment = "_Depth";
		attachmentDatabase.EmplaceFrameAttachment(kDepthAttachment, RHI::ImageDescriptor{
			.name = kDepthAttachment,
			.width = kWidth,
			.height = kHeight,
			.depth = 1,
			.dimension = Dimension::Dim2D,
			.format = Format::D32_SFLOAT,
			.mipLevels = 1,
			.sampleCount = 1,
			.arrayLayers = 1,
			.bindFlags = TextureBindFlags::Depth,
			.defaultHeapType = MemoryHeapType::Default
		});

		const Name kShadowMapAttachment = "_ShadowMap";
		attachmentDatabase.EmplaceFrameAttachment(kShadowMapAttachment, RHI::ImageDescriptor{
			.name = kShadowMapAttachment,
			.width = 1024,
			.height = 1024,
			.depth = 1,
			.dimension = Dimension::Dim2D,
			.format = Format::D32_SFLOAT,
			.mipLevels = 1,
			.sampleCount = 1,
			.arrayLayers = 1,
			.bindFlags = TextureBindFlags::Depth,
			.defaultHeapType = MemoryHeapType::Default
			});

		const Name kColorOutputAttachment = "_ColorOutput";
		attachmentDatabase.EmplaceFrameAttachment(kColorOutputAttachment, swapChain);

		scheduler->BeginScope("Depth");
		{
			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;

			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Shadow");
		{
			ImageScopeAttachmentDescriptor shadowAttachment{};
			shadowAttachment.attachmentId = kShadowMapAttachment;

			scheduler->UseAttachment(shadowAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Skybox");
		{
			ImageScopeAttachmentDescriptor colorAttachment{};
			colorAttachment.attachmentId = kColorOutputAttachment;

			scheduler->UseAttachment(colorAttachment, ScopeAttachmentUsage::Color, ScopeAttachmentAccess::Write);
		}
		scheduler->EndScope();

		scheduler->BeginScope("Opaque");
		{
			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;

			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);

			ImageScopeAttachmentDescriptor shadowAttachment{};
			shadowAttachment.attachmentId = kShadowMapAttachment;
			shadowAttachment.shaderInputName = "_ShadowMap";

			scheduler->UseAttachment(shadowAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);

			ImageScopeAttachmentDescriptor colorAttachment{};
			colorAttachment.attachmentId = kColorOutputAttachment;

			scheduler->UseAttachment(colorAttachment, ScopeAttachmentUsage::Color, ScopeAttachmentAccess::ReadWrite);

		}
		scheduler->EndScope();

		scheduler->BeginScope("Transparent");
		{
			ImageScopeAttachmentDescriptor depthAttachment{};
			depthAttachment.attachmentId = kDepthAttachment;

			scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);

			ImageScopeAttachmentDescriptor colorAttachment{};
			colorAttachment.attachmentId = kColorOutputAttachment;

			scheduler->UseAttachment(colorAttachment, ScopeAttachmentUsage::Color, ScopeAttachmentAccess::ReadWrite);
		}
		scheduler->EndScope();
	}
	scheduler->EndFrameGraph();
}

TEST(FrameGraphTest, Basic)
{
	TEST_BEGIN(true);

	auto app = PlatformApplication::Get();
	auto scheduler = RHI::FrameScheduler::Get();

	auto swapChain = RHI::gDynamicRHI->CreateSwapChain(app->GetMainWindow(), RHI::SwapChainDescriptor{
		2, { Format::R8G8B8A8_UNORM, Format::B8G8R8A8_UNORM }, 0, 0, false
	});

	BuildSampleRenderPipeline(scheduler, swapChain);

	for (int t = 0; t <= scheduler->GetMaxTimelineLevel(); t++)
	{
		Array scopes = scheduler->GetScopesAtTimelineLevel(t);

		if (t == 0)
		{
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Depth"; }));
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Shadow"; }));
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Skybox"; }));
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "TileCulling" && s->GetOperation() == ScopeOperation::Compute; }));
		}
		else if (t == 1)
		{
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Opaque"; }));
		}
		else if (t == 2)
		{
			EXPECT_TRUE(scopes.Exists([](Scope* s) { return s->GetId() == "Transparent"; }));
		}
	}

	while (!IsEngineRequestingExit())
	{
		app->Tick();
		InputManager::Get().Tick();

		
	}

	delete swapChain;
	
	TEST_END();
}

