#include "FusionCore.h"

namespace CE
{
    using namespace CE::RHI;
    using namespace CE::RPI;

    static WeakRef<FusionApplication> gInstance = nullptr;

    extern RawData GetFusionShaderVert();
    extern RawData GetFusionShaderFrag();
    extern RawData GetFusionShaderVertJson();
    extern RawData GetFusionShaderFragJson();

    extern RawData GetFusionShader2Vert();
    extern RawData GetFusionShader2Frag();
    extern RawData GetFusionShader2VertJson();
    extern RawData GetFusionShader2FragJson();

    FusionApplication::FusionApplication()
    {
        fontManager = CreateDefaultSubobject<FFontManager>("FontManager");
        styleManager = CreateDefaultSubobject<FStyleManager>("StyleManager");
        rootContext = CreateDefaultSubobject<FRootContext>("RootContext");
        imageAtlas = CreateDefaultSubobject<FImageAtlas>("ImageAtlas");

#if PLATFORM_LINUX || PLATFORM_MAC
        defaultScalingFactor = 96.0f / 72.0f;
#endif
    }

    FusionApplication::~FusionApplication()
    {

    }

    FusionApplication* FusionApplication::Get()
    {
        if (gInstance == nullptr)
        {
            gInstance = CreateObject<FusionApplication>(GetTransient(MODULE_NAME), "FusionApplication");
        }
        return gInstance.Get();
    }

    FusionApplication* FusionApplication::TryGet()
    {
        return gInstance.Get();
    }

    void FusionApplication::Initialize(const FusionInitInfo& initInfo)
    {
        assetLoader = initInfo.assetLoader;
        systemDpi = PlatformApplication::Get()->GetSystemDpi();

        PlatformApplication::Get()->AddMessageHandler(this);

        InitializeShader2();

        fontManager->Init();
        imageAtlas->Init();

        IO::Path iconResourcesDir = PlatformDirectories::GetEngineRootDir() / "Engine/Resources/Icons";
        iconResourcesDir.RecursivelyIterateChildren([this](const IO::Path& path)
            {
                if (path.IsDirectory())
                    return;

				if (path.GetExtension() == ".png" || path.GetExtension() == ".jpg" || path.GetExtension() == ".jpeg")
				{
                    LoadImageResource(path, "/Engine/Resources/Icons/" + path.GetFileName().RemoveExtension().GetString());
				}
            });

        IO::Path imagesResourcesDir = PlatformDirectories::GetEngineRootDir() / "Engine/Resources/Images";
        imagesResourcesDir.RecursivelyIterateChildren([this](const IO::Path& path)
            {
                if (path.IsDirectory())
                    return;

                if (path.GetExtension() == ".png" || path.GetExtension() == ".jpg" || path.GetExtension() == ".jpeg")
                {
                    LoadImageResource(path, "/Engine/Resources/Images/" + path.GetFileName().RemoveExtension().GetString());
                }
            });
    }

    void FusionApplication::PreShutdown()
    {
        PlatformApplication::Get()->RemoveMessageHandler(this);

        RHI::FrameScheduler* scheduler = RHI::FrameScheduler::Get();

        // Pre-shutdown and cleanup resources before engine assets are unloaded

        if (scheduler)
        {
            scheduler->WaitUntilIdle();
        }

        fontManager->Shutdown();

        imageAtlas->Shutdown();

        for (int i = destructionQueue.GetSize() - 1; i >= 0; --i)
        {
            if (destructionQueue[i].object.IsValid())
            {
	            destructionQueue[i].object->BeginDestroy();
            }

            destructionQueue.RemoveAt(i);
        }
    }

    void FusionApplication::Shutdown()
    {
        delete fusionShader; fusionShader = nullptr;
        delete fusionShader2; fusionShader2 = nullptr;
    }

    void FusionApplication::RegisterViewport(FViewport* viewport)
    {
        viewports.Add(viewport);
    }

    void FusionApplication::DeregisterViewport(FViewport* viewport)
    {
        viewports.Remove(viewport);
    }

    void FusionApplication::PushCursor(SystemCursor cursor)
    {
        cursorStack.Insert(cursor);
        PlatformApplication::Get()->SetSystemCursor(cursorStack.Last());
    }

    SystemCursor FusionApplication::GetCursor()
    {
        if (cursorStack.IsEmpty())
            return SystemCursor::Default;
        return cursorStack.Last();
    }

    void FusionApplication::PopCursor()
    {
        if (cursorStack.IsEmpty())
            return;

        cursorStack.RemoveLast();

        if (cursorStack.IsEmpty())
        {
            PlatformApplication::Get()->SetSystemCursor(SystemCursor::Default);
        }
        else
        {
            PlatformApplication::Get()->SetSystemCursor(cursorStack.Last());
        }
    }

    CMImage FusionApplication::LoadImageAsset(const Name& assetPath)
    {
        if (!assetPath.IsValid() || !assetLoader)
            return {};

        return assetLoader->LoadImageAssetAtPath(assetPath);
    }

    int FusionApplication::LoadImageResource(const IO::Path& resourcePath, const Name& imageName)
    {
        if (resourcePath.IsEmpty())
            return -1;
        
        if (resourcePath.Exists() && !resourcePath.IsDirectory())
        {
            CMImage image = CMImage::LoadFromFile(resourcePath);
            if (image.IsValid())
            {
                auto result = imageAtlas->AddImage(imageName, image);
                return result.IsValid() ? 1 : 0;
            }
        }

        return 0;
    }

    int FusionApplication::FindOrCreateSampler(const RHI::SamplerDescriptor& samplerDesc)
    {
        ZoneScoped;

        RHI::Sampler* sampler = RPISystem::Get().FindOrCreateSampler(samplerDesc);
        if (samplerIndices.KeyExists(sampler))
        {
            return samplerIndices[sampler];
        }

        int index = samplerArray.GetCount();
        if (index > 15)
        {
            CE_LOG(Error, All, "Out of sampler slots!");
        }

        samplerIndices[sampler] = index;
        samplerArray.Insert(sampler);
        samplersUpdated = true;
        return index;
    }

    void FusionApplication::SetExposed()
    {
        isExposed = true;
    }

    void FusionApplication::ResetExposed()
    {
        isExposed = false;
    }

    void FusionApplication::Tick()
    {
        ZoneScoped;

        for (int i = destructionQueue.GetSize() - 1; i >= 0; --i)
        {
            //if (destructionQueue[i].frameCounter >= RHI::Limits::MaxSwapChainImageCount)
            {
                destructionQueue[i].object->BeginDestroy();
                destructionQueue.RemoveAt(i);
                continue;
            }

            destructionQueue[i].frameCounter++;
        }

        for (int i = timers.GetSize() - 1; i >= 0; --i)
        {
            timers[i]->Tick();
        }

        if (rootContext)
        {
            rootContext->Tick();
        }
    }

    void FusionApplication::EmplaceFrameAttachments()
    {
        ZoneScoped;

        rootContext->EmplaceFrameAttachments();
    }

    void FusionApplication::EnqueueScopes()
    {
        ZoneScoped;

        rootContext->EnqueueScopes();
    }

    void FusionApplication::UpdateDrawListMask(DrawListMask& drawListMask)
    {
        rootContext->SetDrawListMask(drawListMask);
    }

    void FusionApplication::EnqueueDrawPackets(DrawListContext& drawList, u32 imageIndex)
    {
        rootContext->EnqueueDrawPackets(drawList, imageIndex);
    }

    void FusionApplication::FlushDrawPackets(DrawListContext& drawList, u32 imageIndex)
    {
        fontManager->Flush(imageIndex);
        imageAtlas->Flush(imageIndex);

        rootContext->SetDrawPackets(drawList);
    }

    Ref<FWindow> FusionApplication::CreateNativeWindow(const Name& windowName, const String& title, u32 width, u32 height, 
        const SubClass<FWindow>& windowClass, 
        const PlatformWindowInfo& info)
    {
        if (windowClass == nullptr || !windowClass->CanBeInstantiated())
            return nullptr;

        ClassType* clazz = windowClass;

        FRootContext* rootContext = FusionApplication::Get()->GetRootContext();
        Ref<FFusionContext> parentContext = rootContext;
        if (rootContext->GetChildContexts().NotEmpty())
        {
            // FRootContext should have only 1 NativeContext which is the primary Native Window
            parentContext = rootContext->GetChildContexts().GetFirst();
        }

        f32 scaling = PlatformApplication::Get()->GetSystemDpi() / 96.0f;
#if PLATFORM_MAC
        scaling = 1;
#elif PLATFORM_LINUX
        scaling *= defaultScalingFactor;
#endif

        PlatformWindow* window = PlatformApplication::Get()->CreatePlatformWindow(title, (u32)(width * scaling), (u32)(height * scaling), info);
        window->SetBorderless(true);

        FNativeContext* context = FNativeContext::Create(window, windowName.GetString(), parentContext.Get());
        parentContext->AddChildContext(context);

        Ref<FWindow> outWindow = nullptr;

        FAssignNewOwnedDynamic(FWindow, outWindow, context, clazz);
        context->SetOwningWidget(outWindow.Get());

        return outWindow;
    }


    void FusionApplication::PrepareDrawList()
    {
        drawList.Shutdown();

        if (!rootContext)
            return;

        RHI::DrawListMask drawListMask{};
        HashSet<RHI::DrawListTag> drawListTags{};

        UpdateDrawListMask(drawListMask);

        for (int i = 0; i < drawListMask.GetSize(); ++i)
        {
            if (drawListMask.Test(i))
            {
                drawListTags.Add((u8)i);
            }
        }

        drawList.Init(drawListMask);
        {
            EnqueueDrawPackets(drawList, curImageIndex);
        }
        drawList.Finalize();

        FlushDrawPackets(drawList, curImageIndex);
    }

    void FusionApplication::RequestFrameGraphUpdate()
    {
        onFrameGraphUpdateRequested.Broadcast();
    }

    // - Application Callbacks -

    void FusionApplication::QueueDestroy(Object* object)
    {
        destructionQueue.Add({ .object = object, .frameCounter = 0 });
    }

    u64 FusionApplication::ComputeMemoryFootprint()
    {
        u64 memory = Super::ComputeMemoryFootprint();

        memory += sizeof(RPI::Shader);

        return memory;
    }

    void FusionApplication::OnWindowRestored(PlatformWindow* window)
    {
        
    }

    void FusionApplication::OnWindowDestroyed(PlatformWindow* window)
    {
        rootContext->MarkLayoutDirty();

        for (int i = rootContext->childContexts.GetSize() - 1; i >= 0; i--)
        {
            Ref<FFusionContext> context = rootContext->childContexts[i];

            if (!context->IsOfType<FNativeContext>())
                continue;

            FNativeContext* nativeContext = static_cast<FNativeContext*>(context.Get());
            
            if (window->IsMainWindow() || nativeContext->GetPlatformWindow() == window)
            {
                if (nativeContext->owningWidget)
                    nativeContext->owningWidget->BeginDestroy();
                nativeContext->owningWidget = nullptr;

                if (!nativeContext->isDestroyed)
                {
                    nativeContext->platformWindow = nullptr;
                    nativeContext->QueueDestroy();
                }
            }
        }
    }

    void FusionApplication::OnWindowClosed(PlatformWindow* window)
    {

    }

    void FusionApplication::OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight)
    {

        rootContext->MarkLayoutDirty();
    }

    void FusionApplication::OnWindowMinimized(PlatformWindow* window)
    {

    }

    void FusionApplication::OnWindowCreated(PlatformWindow* window)
    {

    }

    void FusionApplication::OnWindowExposed(PlatformWindow* window)
    {

    }

    void FusionApplication::OnRenderViewportDestroyed(FGameWindow* renderViewport)
    {
        onRenderViewportDestroyed(renderViewport);
    }

    // - Shader Resources -

    void FusionApplication::InitializeShader2()
    {
        RawData vertexShader = GetFusionShader2Vert();
        RawData fragmentShader = GetFusionShader2Frag();

        String vertexShaderJson = (char*)GetFusionShader2VertJson().data;
        String fragmentShaderJson = (char*)GetFusionShader2FragJson().data;

        JValue vertexReflection{};
        JValue fragmentReflection{};

        JsonSerializer::Deserialize2(vertexShaderJson, vertexReflection);
        JsonSerializer::Deserialize2(fragmentShaderJson, fragmentReflection);

        RPI::ShaderVariantDescriptor2 variantDesc{};
        variantDesc.interleaveVertexData = true;
        variantDesc.shaderName = "FusionShader";
        variantDesc.entryPoints.Resize(2);
        variantDesc.entryPoints[0] = "VertMain";
        variantDesc.entryPoints[1] = "FragMain";

        variantDesc.moduleDesc.Resize(2);
        variantDesc.moduleDesc[0].byteCode = vertexShader.data;
        variantDesc.moduleDesc[0].byteSize = vertexShader.dataSize;
        variantDesc.moduleDesc[0].stage = ShaderStage::Vertex;
        variantDesc.moduleDesc[0].name = "VertMain";

        variantDesc.moduleDesc[1].byteCode = fragmentShader.data;
        variantDesc.moduleDesc[1].byteSize = fragmentShader.dataSize;
        variantDesc.moduleDesc[1].stage = ShaderStage::Fragment;
        variantDesc.moduleDesc[1].name = "FragMain";

        RHI::SRGVariableDescriptor perViewData{};
        perViewData.name = "_PerViewData";
        perViewData.bindingSlot = (u32)vertexReflection["ubos"][0]["binding"].GetNumberValue();
        perViewData.shaderStages = ShaderStage::Vertex | ShaderStage::Fragment;
        perViewData.type = ShaderResourceType::ConstantBuffer;

        variantDesc.reflectionInfo.FindOrAdd(SRGType::PerView)
            .TryAdd(perViewData);

        perViewSrgLayout = variantDesc.reflectionInfo.FindOrAdd(SRGType::PerView);

        {
            variantDesc.reflectionInfo.rootConstantStages = ShaderStage::Vertex | ShaderStage::Fragment;
            variantDesc.reflectionInfo.rootConstantLayout = { ShaderStructMemberType::Float2, ShaderStructMemberType::UInt };
            for (int i = 0; i < FusionRenderer2::MaxClipRectStack; ++i)
            {
                variantDesc.reflectionInfo.rootConstantLayout.Add(ShaderStructMemberType::UInt);
            }

            variantDesc.reflectionInfo.FindOrAdd(SRGType::PerObject)
                .TryAdd(SRGVariableDescriptor(
                    "_Objects",
                    (u32)vertexReflection["ssbos"][0]["binding"].GetNumberValue(),
                    ShaderResourceType::StructuredBuffer,
                    ShaderStage::Vertex | ShaderStage::Fragment
                ))
                .TryAdd(SRGVariableDescriptor(
                    "_ClipRects",
                    (u32)vertexReflection["ssbos"][1]["binding"].GetNumberValue(),
                    ShaderResourceType::StructuredBuffer,
                    ShaderStage::Vertex | ShaderStage::Fragment
                ))
                .TryAdd(SRGVariableDescriptor(
                    "_DrawData",
                    (u32)vertexReflection["ssbos"][2]["binding"].GetNumberValue(),
                    ShaderResourceType::StructuredBuffer,
                    ShaderStage::Vertex | ShaderStage::Fragment
                ))
                .TryAdd(SRGVariableDescriptor(
                    "_GradientKeys",
                    (u32)vertexReflection["ssbos"][3]["binding"].GetNumberValue(),
                    ShaderResourceType::StructuredBuffer,
                    ShaderStage::Vertex | ShaderStage::Fragment
                ))
            ;

            variantDesc.reflectionInfo.FindOrAdd(SRGType::PerMaterial)
				.TryAdd(SRGVariableDescriptor(
					"_FontAtlas",
                    (u32)fragmentReflection["separate_images"][0]["binding"].GetNumberValue(),
                    ShaderResourceType::Texture2D,
                    ShaderStage::Fragment
                ))
				.TryAdd(SRGVariableDescriptor(
					"_FontAtlasSampler",
                    (u32)fragmentReflection["separate_samplers"][0]["binding"].GetNumberValue(),
                    ShaderResourceType::SamplerState,
                    ShaderStage::Fragment
                ))
            ;

            variantDesc.reflectionInfo.FindOrAdd(SRGType::PerDraw)
                .TryAdd(SRGVariableDescriptor(
                    "_Texture",
                    (u32)fragmentReflection["separate_images"][1]["binding"].GetNumberValue(),
                    ShaderResourceType::Texture2DArray,
                    ShaderStage::Fragment
                ))
                .TryAdd(SRGVariableDescriptor(
                    "_TextureSampler",
                    (u32)fragmentReflection["separate_samplers"][1]["binding"].GetNumberValue(),
                    ShaderResourceType::SamplerState,
                    ShaderStage::Fragment
                ))
            ;
        }

        variantDesc.reflectionInfo.vertexInputs.Add("POSITION");
        variantDesc.reflectionInfo.vertexInputTypes.Add(VertexAttributeDataType::Float2);

        variantDesc.reflectionInfo.vertexInputs.Add("TEXCOORD0");
        variantDesc.reflectionInfo.vertexInputTypes.Add(VertexAttributeDataType::Float2);

        variantDesc.reflectionInfo.vertexInputs.Add("COLOR0");
        variantDesc.reflectionInfo.vertexInputTypes.Add(VertexAttributeDataType::UChar4);

        variantDesc.reflectionInfo.vertexInputs.Add("TEXCOORD1");
        variantDesc.reflectionInfo.vertexInputTypes.Add(VertexAttributeDataType::Int);

        variantDesc.reflectionInfo.vertexInputs.Add("TEXCOORD2");
        variantDesc.reflectionInfo.vertexInputTypes.Add(VertexAttributeDataType::Int);

        ShaderTagEntry cullMode{};
        cullMode.key = "Cull";
        cullMode.value = "Off";
        variantDesc.tags.Add(cullMode);

        fusionShader2 = new RPI::Shader();
        fusionShader2->AddVariant(variantDesc);
    }

} // namespace CE

