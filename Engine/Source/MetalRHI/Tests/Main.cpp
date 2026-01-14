
#include "CoreApplication.h"
#include "MetalRHI.h"
#include "CoreShader.h"

#include "MetalRHITest.h"

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
    ModuleManager::Get().LoadModule("MetalRHI");
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

    RHI::gDynamicRHI = new CE::Metal::MetalRHI();
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
    ModuleManager::Get().UnloadModule("MetalRHI");
    ModuleManager::Get().UnloadModule("CoreRHI");
    ModuleManager::Get().UnloadModule("CoreMedia");
    ModuleManager::Get().UnloadModule("CoreApplication");
    ModuleManager::Get().UnloadModule("Core");
}

constexpr const char HLSL_Test[] = R"(
#include "Core/Macros.hlsli"

cbuffer _SceneData : SRG_PerScene(b0)
{
    float _TimeElapsed;
};

#if FRAGMENT

cbuffer _MaterialData : SRG_PerMaterial(b1)
{
    float4 _Albedo;
    float _Roughness;
    float _Metallic;
};

#endif



struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

#if VERTEX

PSInput VertMain(VSInput input)
{
    PSInput o;
    o.position = float4(input.position, 1.0 * _TimeElapsed);
    return o;
}
#endif

#if FRAGMENT

float4 FragMain(PSInput input) : SV_TARGET
{
    return float4(0, 0, 0, 1);
}

#endif
)";

TEST(RHI, MetalBasic)
{
    WINDOW_TEST_BEGIN;
    
    PlatformApplication* app = PlatformApplication::Get();
    PlatformWindow* mainWindow = app->GetMainWindow();
    
    auto rhi = RHI::gDynamicRHI;
    
    u32 width = 0;
    u32 height = 0;
    
    ShaderCompiler compiler{};

    ShaderBuildConfig buildConfig{};
    buildConfig.entry = "VertMain";
    buildConfig.stage = RHI::ShaderStage::Vertex;
    buildConfig.debugName = "VertMain";
    buildConfig.includeSearchPaths.Add(PlatformDirectories::GetLaunchDir() / "Engine/Shaders");
    
    Array<std::wstring> vertexExtraArgs{};
    vertexExtraArgs.AddRange({
        L"-D", L"COMPILE=1",
        L"-D", L"VERTEX=1",
        L"-fspv-preserve-bindings",
        L"-fspv-preserve-interface"
        });
    
    ShaderReflection vertReflection{};
    
    BinaryBlob vertexMsl;
    ShaderCompiler::ErrorCode result = compiler.BuildMSL(HLSL_Test, COUNTOF(HLSL_Test), buildConfig, vertexMsl, vertexExtraArgs, &vertReflection);
    
    buildConfig.entry = "FragMain";
    buildConfig.stage = RHI::ShaderStage::Fragment;
    buildConfig.debugName = "FragMain";
    
    Array<std::wstring> fragmentExtraArgs{};
    fragmentExtraArgs.AddRange({
        L"-D", L"COMPILE=1",
        L"-D", L"FRAGMENT=1",
        L"-fspv-preserve-bindings",
        L"-fspv-preserve-interface"
        });
    
    ShaderReflection fragReflection{};
    
    BinaryBlob fragmentMsl;
    result = compiler.BuildMSL(HLSL_Test, COUNTOF(HLSL_Test), buildConfig, fragmentMsl, fragmentExtraArgs, &fragReflection);
    
    while (!IsEngineRequestingExit())
    {
        app->Tick();
        InputManager::Get().Tick();
        
        
    }
    
    WINDOW_TEST_END;
}
