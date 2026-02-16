
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

#if PLATFORM_MAC
    gDefaultWindowWidth = 1000;
    gDefaultWindowHeight = 600;
#else
	gDefaultWindowWidth = 1280;
	gDefaultWindowHeight = 900;
#endif

    
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

static constexpr f32 quadVertexData[] = {
    // Positions
    -1.0f, -1.0f, 0, 0,
    1.0f, -1.0f, 0, 0,
    -1.0f, 1.0f, 0, 0,
    -1.0f, 1.0f, 0, 0,
    1.0f, -1.0f, 0, 0,
    1.0f, 1.0f, 0, 0,
    // UVs
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
};

static std::tuple<RHI::ShaderModule*, RHI::ShaderModule*> CompileGraphicsShader(BinaryBlob& vertBlob, BinaryBlob& fragBlob, ShaderReflection& reflection, const void* data, u32 dataSize)
{
    ShaderCompiler compiler{};

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

    auto result = compiler.CompileAuto(data, dataSize, compileInfo);
    String errorMsg = compiler.GetErrorMessage();
    EXPECT_EQ(result, ShaderCompiler::ERR_Success);

	if (result != ShaderCompiler::ERR_Success)
    {
        std::cout << "Shader compilation failed: " << errorMsg << std::endl;
    }

    RHI::ShaderModuleDescriptor vertShaderDesc{};
    vertShaderDesc.name = "Vertex Shader";
    vertShaderDesc.defaultEntryPoint = "VertMain";
    vertShaderDesc.stage = RHI::ShaderStage::Vertex;
    vertShaderDesc.debugName = vertShaderDesc.name;
    vertShaderDesc.byteCode = vertBlob.GetDataPtr();
    vertShaderDesc.byteSize = vertBlob.GetDataSize();

    RHI::ShaderModuleDescriptor fragShaderDesc{};
    fragShaderDesc.name = "Fragment Shader";
    fragShaderDesc.defaultEntryPoint = "FragMain";
    fragShaderDesc.stage = RHI::ShaderStage::Fragment;
    fragShaderDesc.debugName = fragShaderDesc.name;
    fragShaderDesc.byteCode = fragBlob.GetDataPtr();
    fragShaderDesc.byteSize = fragBlob.GetDataSize();

    return std::make_tuple(
        RHI::gDynamicRHI->CreateShaderModule(vertShaderDesc),
        RHI::gDynamicRHI->CreateShaderModule(fragShaderDesc)
    );
}

static auto BuildShaderModules()
{
	
}

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

    namespace Shaders
    {

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

        constexpr char RHI_FrameGraph_SinglePassBlur[] = R"(
#include "Core/Macros.hlsli"

struct VSInput
{
    float3 position : POSITION;
	float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput o;
    o.position = float4(input.position, 1.0);
	o.uv = input.uv;
    return o;
}
#endif

#if FRAGMENT

Texture2D<float4> _InputTexture : SRG_PerPass(t1);
SamplerState _InputSampler : SRG_PerScene(s0);

float4 SinglePassBlur(float2 uv, float2 texelSize)
{
	float4 sum = 0;
	const int radius = 12;

	for (int i = -radius; i <= radius; i++)
    {
        for (int j = -radius; j <= radius; j++)
        {
            float2 offset = float2(i, j) * texelSize;
            sum += _InputTexture.Sample(_InputSampler, uv + offset);
        }
    }

	return sum / (radius * 2 + 1) / (radius * 2 + 1);
}

float4 FragMain(PSInput input) : SV_TARGET
{
	uint width, height;
    _InputTexture.GetDimensions(width, height);

	const float2 uv = input.uv;

    float4 c = _InputTexture.Sample(_InputSampler, uv);
	c = SinglePassBlur(uv, float2(1.0 / width, 1.0 / height));
    return float4(c.rgb, c.a);
}

#endif
)";

        constexpr char RHI_FrameGraph_HPassBlur[] = R"(
#include "Core/Macros.hlsli"

struct VSInput
{
    float3 position : POSITION;
	float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput o;
    o.position = float4(input.position, 1.0);
	o.uv = input.uv;
    return o;
}
#endif

#if FRAGMENT

Texture2D<float4> _InputTexture : SRG_PerPass(t1);
SamplerState _InputSampler : SRG_PerScene(s0);

float4 HPassBlur(float2 uv, float2 texelSize)
{
	float4 sum = 0;
	const int radius = 12;

	float sigma = (float)radius / 2.0;
    float sigma2 = sigma * sigma;
	float totalWeight = 0;

	for (int i = -radius; i <= radius; i++)
    {
        float2 offset = float2(i, 0) * texelSize;
		float weight = exp(-(float(i * i)) / (2.0 * sigma2));

        sum += _InputTexture.Sample(_InputSampler, uv + offset);

		totalWeight += weight;
    }

	return sum / totalWeight;
}

float4 FragMain(PSInput input) : SV_TARGET
{
	uint width, height;
    _InputTexture.GetDimensions(width, height);

	const float2 uv = input.uv;

    float4 c = _InputTexture.Sample(_InputSampler, uv);
	c = HPassBlur(uv, float2(1.0 / width, 1.0 / height));
    return float4(c.rgb, c.a);
}

#endif
)";

        constexpr char RHI_FrameGraph_VPassBlur[] = R"(
#include "Core/Macros.hlsli"

struct VSInput
{
    float3 position : POSITION;
	float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput o;
    o.position = float4(input.position, 1.0);
	o.uv = input.uv;
    return o;
}
#endif

#if FRAGMENT

Texture2D<float4> _InputTexture : SRG_PerPass(t1);
SamplerState _InputSampler : SRG_PerScene(s0);

float4 VPassBlur(float2 uv, float2 texelSize)
{
	float4 sum = 0;
	const int radius = 12;

	float sigma = (float)radius / 2.0;
    float sigma2 = sigma * sigma;
	float totalWeight = 0;

	for (int i = -radius; i <= radius; i++)
    {
        float2 offset = float2(0, i) * texelSize;
		float weight = exp(-(float(i * i)) / (2.0 * sigma2));

        sum += _InputTexture.Sample(_InputSampler, uv + offset);

		totalWeight += weight;
    }

	return sum / totalWeight;
}

float4 FragMain(PSInput input) : SV_TARGET
{
	uint width, height;
    _InputTexture.GetDimensions(width, height);

	const float2 uv = input.uv;

    float4 c = _InputTexture.Sample(_InputSampler, uv);
	c = VPassBlur(uv, float2(1.0 / width, 1.0 / height));
    return float4(c.rgb, c.a);
}

#endif
)";
    }

    TEST(RHI, FrameGraph)
    {
        WINDOW_TEST_BEGIN;
        using namespace RHI_FrameGraph::Shaders;

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

        ShaderReflection reflection{};
        BinaryBlob vertBlob;
        BinaryBlob fragBlob;

        auto [vertShader, fragShader] = CompileGraphicsShader(vertBlob, fragBlob, reflection, RHI_FrameGraph_Shader, COUNTOF(RHI_FrameGraph_Shader));

        ShaderReflection blurReflection{};
        BinaryBlob blurHVertBlob;
        BinaryBlob blurHFragBlob;
        
        auto [blurHVertShader, blurHFragShader] = CompileGraphicsShader(blurHVertBlob, blurHFragBlob, blurReflection, RHI_FrameGraph_HPassBlur, COUNTOF(RHI_FrameGraph_HPassBlur));

        ShaderReflection blurVReflection{};
		BinaryBlob blurVVertBlob;
		BinaryBlob blurVFragBlob;

		auto [blurVVertShader, blurVFragShader] = CompileGraphicsShader(blurVVertBlob, blurVFragBlob, blurVReflection, RHI_FrameGraph_VPassBlur, COUNTOF(RHI_FrameGraph_VPassBlur));

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

        RHI::ShaderResourceGroup* blurPerSceneSrg = nullptr;
        {
            const RHI::ShaderResourceGroupLayout& blurPerSceneSrgLayout = blurReflection.srgLayouts[1];
            RHI::ShaderResourceGroupDescriptor blurPerSceneSrgDesc{};
            blurPerSceneSrgDesc.name = "Blur SRG_PerScene";
            blurPerSceneSrgDesc.layout = blurPerSceneSrgLayout;
            blurPerSceneSrg = gDynamicRHI->CreateShaderResourceGroup(blurPerSceneSrgDesc);
        }

        RHI::ShaderResourceGroupLayout blurPassSrgLayout = blurReflection.srgLayouts[0];

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

        RHI::Sampler* blurSampler = nullptr;

        {
			RHI::SamplerDescriptor samplerDesc{};
			samplerDesc.addressModeU = RHI::SamplerAddressMode::ClampToEdge;
			samplerDesc.addressModeV = RHI::SamplerAddressMode::ClampToEdge;
			samplerDesc.addressModeW = RHI::SamplerAddressMode::ClampToEdge;
			samplerDesc.samplerFilterMode = RHI::FilterMode::Linear;

			blurSampler = gDynamicRHI->CreateSampler(samplerDesc);

            blurPerSceneSrg->Bind("_InputSampler", blurSampler);
        }

        perViewSrg->FlushBindings();
        perObjectSrg->FlushBindings();
        blurPerSceneSrg->FlushBindings();

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

		Array<RHI::VertexBufferView> fullScreenQuadVertexBufferView{};
        RHI::DrawPacket* fullScreenQuadDrawPacket = nullptr;
        RHI::DrawList fullScreenQuadDrawList;
        {
            RHI::BufferDescriptor bufferDesc{};
            bufferDesc.name = "FullScreenQuad Vertex Buffer";
            bufferDesc.bindFlags = BufferBindFlags::VertexBuffer;
            bufferDesc.defaultHeapType = MemoryHeapType::Default;
            bufferDesc.bufferSize = sizeof(quadVertexData);
            bufferDesc.structureByteStride = sizeof(quadVertexData);

            RHI::Buffer* vertexBuffer = gDynamicRHI->CreateBuffer(bufferDesc);
            vertexBuffer->UploadData(quadVertexData, sizeof(quadVertexData));

            fullScreenQuadVertexBufferView.Add(RHI::VertexBufferView(vertexBuffer, 0, sizeof(Vec4) * 6, sizeof(Vec4)));
            fullScreenQuadVertexBufferView.Add(RHI::VertexBufferView(vertexBuffer, sizeof(Vec4) * 6, sizeof(Vec2) * 6, sizeof(Vec2)));

			RHI::DrawPacketBuilder drawPacketBuilder{};

            drawPacketBuilder.SetDebugName("Full Screen Quad");

            drawPacketBuilder.AddShaderResourceGroup(blurPerSceneSrg);

            drawPacketBuilder.AddDrawItem({
                .stencilRef = 0,
                .drawItemTag = 0,
                .indexBufferView = {},
                .vertexBufferViews = fullScreenQuadVertexBufferView
            });

            drawPacketBuilder.SetDrawArguments(RHI::DrawLinearArguments{
				.instanceCount = 1,
				.firstInstance = 0,
				.vertexCount = 6,
				.vertexOffset = 0
            });

            fullScreenQuadDrawPacket = drawPacketBuilder.Build();

            fullScreenQuadDrawList.AddDrawItem(&fullScreenQuadDrawPacket->drawItems[0]);
		}

        // - Graphics Pipeline -

        RHI::PipelineState* depthPipeline = nullptr;
        RHI::PipelineState* colorPipeline = nullptr;
		RHI::PipelineState* blurHPipeline = nullptr;
		RHI::PipelineState* blurVPipeline = nullptr;
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

            // Blur Pipeline

            desc.srgLayouts.Clear();
            desc.srgLayouts.Add(blurPerSceneSrg->GetLayout());
            desc.srgLayouts.Add(blurPassSrgLayout);

            desc.depthStencilState.depthState.enable = false;
            desc.depthStencilState.depthState.writeEnable = false;
            desc.depthStencilState.depthState.testEnable = false;

            desc.vertexInputSlots.Resize(2);
            desc.vertexInputSlots[0].stride = sizeof(Vec4);
            desc.vertexInputSlots[1].stride = sizeof(Vec2);

            desc.vertexAttributes.Resize(2);
            desc.vertexAttributes[0].dataType = RHI::VertexAttributeDataType::Float3;
            desc.vertexAttributes[1].dataType = RHI::VertexAttributeDataType::Float2;

			desc.shaderStages[0].shaderModule = blurHVertShader;
			desc.shaderStages[1].shaderModule = blurHFragShader;

            desc.renderPassLayout.attachmentLayouts.Clear();
            desc.renderPassLayout.attachmentLayouts.Add(RHI::RenderPassAttachmentLayout{
                .attachmentUsage = ScopeAttachmentUsage::Color,
                .format = swapChain->GetSwapChainFormat(),
                .loadAction = AttachmentLoadAction::Clear,
                .storeAction = AttachmentStoreAction::Store,
            });
            desc.subpass = 0;

			desc.name = "BlurPipeline";
			blurHPipeline = gDynamicRHI->CreateGraphicsPipeline(desc);

            desc.shaderStages[0].shaderModule = blurVVertShader;
            desc.shaderStages[1].shaderModule = blurVFragShader;

            blurVPipeline = gDynamicRHI->CreateGraphicsPipeline(desc);
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

        constexpr const char* kColorAttachmentId = "ColorOutput";
        constexpr const char* kSwapChainAttachmentId = "SwapChain";
        constexpr const char* kDepthAttachmentId = "DepthBuffer";

        constexpr const char* kDepthPrePassId = "DepthPrePass";
        constexpr const char* kColorPassId = "ColorPass";
        constexpr const char* kBlurSinglePassId = "BlurSinglePass";
        constexpr const char* kBlurHPassId = "BlurHPass";
        constexpr const char* kBlurVPassId = "BlurVPass";

        bool rebuildFrameGraph = true;

        auto buildFrameGraph = [&]
            {
                scheduler->BeginFrameGraph();
                {
                    RHI::FrameAttachmentDatabase& attachmentDatabase = scheduler->GetAttachmentDatabase();

                    // 1. Emplace Attachments

                    attachmentDatabase.EmplaceFrameAttachment(kSwapChainAttachmentId, swapChain);
                    attachmentDatabase.EmplaceFrameAttachment(kColorAttachmentId, RHI::ImageDescriptor{
                        .name = "ColorAttachment",
                        .width = swapChain->GetWidth(),
                        .height = swapChain->GetHeight(),
                        .depth = 1,
                        .dimension = Dimension::Dim2D,
                        .format = swapChain->GetSwapChainFormat(),
                        .mipLevels = 1,
                        .sampleCount = 1,
                        .arrayLayers = 1,
                        .bindFlags = TextureBindFlags::Color | TextureBindFlags::ShaderRead,
                        .defaultHeapType = MemoryHeapType::Default
                    });
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
                        RHI::ImageScopeAttachmentDescriptor colorAttachment{};
                        colorAttachment.attachmentId = kColorAttachmentId;
                        colorAttachment.loadStoreAction.clearValue = Vec4(0, 0, 0.2f, 1);
                        colorAttachment.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Clear;
                        colorAttachment.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
                        scheduler->UseAttachment(colorAttachment, RHI::ScopeAttachmentUsage::Color, RHI::ScopeAttachmentAccess::ReadWrite);

                        RHI::ImageScopeAttachmentDescriptor depthAttachment{};
                        depthAttachment.attachmentId = kDepthAttachmentId;
                        depthAttachment.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
                        depthAttachment.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
                        scheduler->UseAttachment(depthAttachment, ScopeAttachmentUsage::DepthStencil, ScopeAttachmentAccess::Read);

                        scheduler->UseShaderResourceGroup(perViewSrg);

                        scheduler->UsePipeline(colorPipeline);
                    }
                    scheduler->EndScope();

                    scheduler->BeginScope(kBlurSinglePassId);
                    {
                        RHI::ImageScopeAttachmentDescriptor colorAttachment{};
                        colorAttachment.shaderInputName = "_InputTexture";
                        colorAttachment.attachmentId = kColorAttachmentId;
                        colorAttachment.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Load;
                        colorAttachment.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
                        scheduler->UseAttachment(colorAttachment, RHI::ScopeAttachmentUsage::Shader, RHI::ScopeAttachmentAccess::Read);

						RHI::ImageScopeAttachmentDescriptor swapChainAttachment{};
						swapChainAttachment.attachmentId = kSwapChainAttachmentId;
						swapChainAttachment.loadStoreAction.loadAction = RHI::AttachmentLoadAction::Clear;
						swapChainAttachment.loadStoreAction.storeAction = RHI::AttachmentStoreAction::Store;
                        scheduler->UseAttachment(swapChainAttachment, RHI::ScopeAttachmentUsage::Color, RHI::ScopeAttachmentAccess::ReadWrite);

                        scheduler->UseShaderResourceGroup(blurPerSceneSrg);

                        scheduler->UsePassSrgLayout(blurPassSrgLayout);

						scheduler->UsePipeline(blurHPipeline);

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
                    projectionMatrix[1][1] *= (f32)gDynamicRHI->GetClipSpaceSignY();

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

                    scheduler->SetScopeDrawList(kBlurSinglePassId, &fullScreenQuadDrawList);
                }
                scheduler->EndExecution();
            };

        // - Message Handler -

        class MessageHandler : public ApplicationMessageHandler
        {
        public:
            MessageHandler(bool& rebuildFrameGraph) : rebuildFrameGraph(rebuildFrameGraph)
            {
            }

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

        MessageHandler msgHandler{ rebuildFrameGraph };

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

        delete fullScreenQuadVertexBufferView[0].GetBuffer();
        fullScreenQuadVertexBufferView.Clear();

        for (int i = 0; i < vertexBufferViews.GetSize(); i++)
        {
            delete vertexBufferViews[i].GetBuffer();
        }
        vertexBufferViews.Clear();
        delete indexBufferView.GetBuffer();
        cubeDrawPacket = nullptr;

        delete swapChain;
        delete vertShader; delete fragShader;
        delete blurHVertShader; delete blurHFragShader;
        delete blurVVertShader; delete blurVFragShader;
        delete perViewSrg; delete perObjectSrg; delete blurPerSceneSrg;
        delete colorPipeline; delete depthPipeline; delete blurHPipeline; delete blurVPipeline;
        delete blurSampler;
        delete fullScreenQuadDrawPacket;

        WINDOW_TEST_END;
    }
}
