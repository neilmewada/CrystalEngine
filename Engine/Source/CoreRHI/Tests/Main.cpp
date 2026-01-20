
#include "CoreApplication.h"

#if PAL_TRAIT_METAL_SUPPORTED
#include "MetalRHI.h"
#elif PAL_TRAIT_VULKAN_SUPPORTED
#include "VulkanRHI.h"
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
#elif PAL_TRAIT_VULKAN_SUPPORTED
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
        .resizable = false,
        .hidden = false,
        .windowFlags = PlatformWindowFlags::DestroyOnClose
    };
    
    PlatformWindow* mainWindow = app->InitMainWindow(MODULE_NAME, gDefaultWindowWidth, gDefaultWindowHeight, windowInfo);

#if PAL_TRAIT_METAL_SUPPORTED
    RHI::gDynamicRHI = new CE::Metal::MetalRHI();
#elif PAL_TRAIT_VULKAN_SUPPORTED
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
#elif PAL_TRAIT_VULKAN_SUPPORTED
    ModuleManager::Get().UnloadModule("VulkanRHI");
#endif
    ModuleManager::Get().UnloadModule("CoreRHI");
    ModuleManager::Get().UnloadModule("CoreMedia");
    ModuleManager::Get().UnloadModule("CoreApplication");
    ModuleManager::Get().UnloadModule("Core");
}

struct MaterialData
{
    Vec4 albedo;
    float roughness;
    float metallic;
};

struct SceneData
{
    float timeElapsed;
};

constexpr const char HLSL_Test[] = R"(
#include "Core/Macros.hlsli"

cbuffer _SceneData : SRG_PerScene(b0)
{
    float _TimeElapsed;
};

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
    o.position = float4(input.position, 1.0);
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
    BinaryBlob vertMSL;
    BinaryBlob fragMSL;
    
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
    compileInfo.stages.GetLast().outByteCode = &vertMSL;
    
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
    compileInfo.stages.GetLast().outByteCode = &fragMSL;
    
    auto result = compiler.CompileMSL(HLSL_Test, COUNTOF(HLSL_Test), compileInfo);
    String errorMsg = compiler.GetErrorMessage();
    EXPECT_EQ(result, ShaderCompiler::ERR_Success);
    
    RHI::ShaderModuleDescriptor vertShaderDesc{};
    vertShaderDesc.name = "Vertex Shader";
    vertShaderDesc.defaultEntryPoint = "VertMain";
    vertShaderDesc.stage = RHI::ShaderStage::Vertex;
    vertShaderDesc.debugName = vertShaderDesc.name;
    vertShaderDesc.byteCode = vertMSL.GetDataPtr();
    vertShaderDesc.byteSize = vertMSL.GetDataSize();
    
    RHI::ShaderModule* vertShader = gDynamicRHI->CreateShaderModule(vertShaderDesc);
    
    RHI::ShaderModuleDescriptor fragShaderDesc{};
    fragShaderDesc.name = "Fragment Shader";
    fragShaderDesc.defaultEntryPoint = "FragMain";
    fragShaderDesc.stage = RHI::ShaderStage::Fragment;
    fragShaderDesc.debugName = fragShaderDesc.name;
    fragShaderDesc.byteCode = fragMSL.GetDataPtr();
    fragShaderDesc.byteSize = fragMSL.GetDataSize();
    
    RHI::ShaderModule* fragShader = gDynamicRHI->CreateShaderModule(fragShaderDesc);
    
    // - Shader Resources -
    
    RHI::ShaderResourceGroup* perSceneSrg = nullptr;
    {
        const RHI::ShaderResourceGroupLayout& perSceneSrgLayout = reflection.srgLayouts[0];
        
        RHI::ShaderResourceGroupDescriptor perSceneSrgDesc{};
        perSceneSrgDesc.name = "SRG_PerScene";
        perSceneSrgDesc.layout = perSceneSrgLayout;
        perSceneSrgDesc.shader = vertShader;
        
        perSceneSrg = gDynamicRHI->CreateShaderResourceGroup(perSceneSrgDesc);
    }
    
    SceneData sceneData{};
    sceneData.timeElapsed = 1.0f;
    
    RHI::Buffer* sceneDataBuffer = nullptr; // Should be double buffers too
    {
        RHI::BufferDescriptor bufferDesc{};
        bufferDesc.name = "PerScene cbuffer";
        bufferDesc.bufferSize = sizeof(SceneData);
        bufferDesc.bindFlags = RHI::BufferBindFlags::ConstantBuffer;
        bufferDesc.defaultHeapType = RHI::MemoryHeapType::Upload;
        
        sceneDataBuffer = gDynamicRHI->CreateBuffer(bufferDesc);
        
        sceneDataBuffer->UploadData(&sceneData, sizeof(SceneData));
    }
    
    perSceneSrg->Bind("_SceneData", sceneDataBuffer);
    perSceneSrg->FlushBindings();
    
    // - Vertex & Index Buffers -
    
    RHI::Buffer* vertexBuffer = nullptr;
    RHI::VertexBufferView vertexBufferView{};
    u32 numVertices = 0;
    {
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
    RHI::RenderPassFrameBuffer* frameBuffers[kNumFrames] = {};
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
        
        for (int i = 0; i < kNumFrames; i++)
        {
            frameBuffers[i] = gDynamicRHI->CreateRenderPassFrameBuffer(frameBufferDesc);
        }
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
        
        desc.srgLayouts.Add(perSceneSrg->GetLayout());
        
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
    uint64_t frameDoneValue[kNumFrames] = {};
    RHI::Fence* graphicsFence = gDynamicRHI->CreateFence();
    
    while (!IsEngineRequestingExit())
    {
        app->Tick();
        InputManager::Get().Tick();
        
        swapChain->AcquireNextImage();
        
        if (frameDoneValue[frameIndex] != 0)
        {
            graphicsFence->WaitCPU(frameDoneValue[frameIndex]);
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
            
            cmdList->BeginRenderPass(renderPass, frameBuffers[frameIndex], &clearValue);
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
                
                cmdList->SetShaderResourceGroups({ perSceneSrg });
                cmdList->CommitShaderResources();
                
                cmdList->BindVertexBuffers(0, 1, &vertexBufferView);
                
                cmdList->DrawLinear(RHI::DrawLinearArguments{
                    .firstInstance = 0,
                    .instanceCount = 1,
                    .vertexCount = 3,
                    .vertexOffset = 0
                });
            }
            cmdList->EndRenderPass();
        }
        cmdList->End();
        
        cmdQueue->Submit(submission);
        
        frameIndex = (frameIndex + 1) % kNumFrames;
    }
    
    // - Cleanup -
    
    delete vertShader; delete fragShader;
    delete perSceneSrg;
    delete swapChain;
    delete renderPass;
    gDynamicRHI->FreeCommandLists(kNumFrames, cmdLists);
    delete pipeline;
    delete graphicsFence;
    
    for (int i = 0; i < kNumFrames; i++)
    {
        delete frameBuffers[i];
    }
    
    WINDOW_TEST_END;
}
