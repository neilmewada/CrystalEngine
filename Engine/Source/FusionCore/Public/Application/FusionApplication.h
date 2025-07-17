#pragma once


namespace CE
{
	class FViewport;
    class FImageAtlas;
	class FNativeContext;
    class FFusionContext;
    class FLayoutManager;
    class FNativeContext;
    class FRootContext;
    class FStyleManager;
    class FGameWindow;
    class FWindow;

    struct FusionRawImageData
    {
		RHI::TextureView* textureView = nullptr;
		RHI::ShaderResourceGroup* textureSrg = nullptr;

        bool IsValid() const
        {
            return textureView != nullptr && textureSrg != nullptr;
		}
    };

    struct IFusionAssetLoader
    {
        virtual ~IFusionAssetLoader() = default;

        virtual RHI::Texture* LoadTextureAtPath(const Name& path) = 0;

        virtual FusionRawImageData LoadRawTextureAtPath(const Name& path) = 0;

        virtual CMImage LoadImageAssetAtPath(const Name& path) = 0;

    };

    struct FusionInitInfo
    {
        IFusionAssetLoader* assetLoader = nullptr;
    };

    CLASS(Config = Engine)
    class FUSIONCORE_API FusionApplication : public Object, protected ApplicationMessageHandler
    {
        CE_CLASS(FusionApplication, Object)
    public:

        FusionApplication();
        virtual ~FusionApplication();

        static FusionApplication* Get();
        static FusionApplication* TryGet();

        FFontManager* GetFontManager() const { return fontManager.Get(); }

        FStyleManager* GetStyleManager() const { return styleManager.Get(); }

        FRootContext* GetRootContext() const { return rootContext.Get(); }

        FImageAtlas* GetImageAtlas() const { return imageAtlas.Get(); }

        f32 GetDefaultScalingFactor() const { return defaultScalingFactor; }

        bool IsMailboxPresentationPreferred() const { return preferMailboxPresentMode; }

        void Initialize(const FusionInitInfo& initInfo);
        void PreShutdown();
        void Shutdown();

        void RegisterViewport(FViewport* viewport);
        void DeregisterViewport(FViewport* viewport);

        void PushCursor(SystemCursor cursor);
        SystemCursor GetCursor();
        void PopCursor();

        void DispatchOnMainThread(const Delegate<void(void)>& execute);

        CMImage LoadImageAsset(const Name& assetPath);
        int LoadImageResource(const IO::Path& resourcePath, const Name& imageName);
        FusionRawImageData LoadRawTextureAtPath(const Name& path);

        int FindOrCreateSampler(const RHI::SamplerDescriptor& samplerDesc);

        bool IsExposed() const { return isExposed; }
        void SetExposed();
        void ResetExposed();

        void Tick();

        void RequestFrameGraphUpdate();

        RPI::Shader* GetFusionShader() const { return fusionShader; }
        RPI::Shader* GetFusionShader2() const { return fusionShader2; }

        RHI::ShaderResourceGroupLayout GetPerViewSrgLayout() const { return perViewSrgLayout; }
        RHI::ShaderResourceGroupLayout GetPerObjectSrgLayout() const { return perObjectSrgLayout; }

        void QueueDestroy(Object* object);

        u64 ComputeMemoryFootprint() override;

        void EmplaceFrameAttachments();
        void EnqueueScopes();

        void UpdateDrawListMask(RHI::DrawListMask& drawListMask);

        void EnqueueDrawPackets(RHI::DrawListContext& drawList, u32 imageIndex);

        void FlushDrawPackets(RHI::DrawListContext& drawList, u32 imageIndex);

        Ref<FWindow> CreateNativeWindow(const Name& windowName, const String& title, u32 width, u32 height,
            const SubClass<FWindow>& windowClass, 
            const PlatformWindowInfo& info = {});

        template<typename TWindow> requires TIsBaseClassOf<FWindow, TWindow>::Value and (!TIsSameType<FWindow, TWindow>::Value)
        Ref<TWindow> CreateNativeWindow(const Name& windowName, const String& title, u32 width, u32 height,
            SubClass<TWindow> windowClass,
            const PlatformWindowInfo& info = {})
        {
            if (windowClass == nullptr)
            {
                windowClass = TWindow::StaticClass();
			}
            return (Ref<TWindow>)CreateNativeWindow(windowName, title, width, height, static_cast<SubClass<FWindow>>(windowClass.GetClassType()), info);
        }

        template<typename TWindow> requires TIsBaseClassOf<FWindow, TWindow>::Value and (!TIsSameType<FWindow, TWindow>::Value)
        Ref<TWindow> CreateNativeWindow(const Name& windowName, const String& title, u32 width, u32 height,
                                        const PlatformWindowInfo& info = {})
        {
            return (Ref<TWindow>)CreateNativeWindow(windowName, title, width, height, TWindow::StaticClass(), info);
        }

        ScriptEvent<void(FGameWindow*)> onRenderViewportDestroyed;

        ScriptEvent<void(void)> onFrameGraphUpdateRequested;

    protected:

        void OnWindowRestored(PlatformWindow* window) override;
        void OnWindowDestroyed(PlatformWindow* window) override;
        void OnWindowClosed(PlatformWindow* window) override;
        void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override;
        void OnWindowMinimized(PlatformWindow* window) override;
        void OnWindowCreated(PlatformWindow* window) override;
        void OnWindowExposed(PlatformWindow* window) override;

        void OnRenderViewportDestroyed(FGameWindow* renderViewport);

    protected:

        void InitializeShader2();

        void PrepareDrawList();

        int curImageIndex = 0;
        bool isExposed = false;

        SharedMutex mainThreadDispatcherLock;
        Array<Delegate<void(void)>> mainThreadDispatcher;

        FIELD()
        Ref<FRootContext> rootContext = nullptr;

        FIELD()
        Ref<FFontManager> fontManager = nullptr;

        FIELD()
        Ref<FStyleManager> styleManager = nullptr;

        FIELD()
        Ref<FImageAtlas> imageAtlas = nullptr;

        FIELD(Config)
        f32 defaultScalingFactor = 1.0f;

        FIELD(Config)
        bool preferMailboxPresentMode = false;

        IFusionAssetLoader* assetLoader = nullptr;

        StableDynamicArray<SystemCursor, 32, false> cursorStack;

        using FImageArray = StableDynamicArray<RHI::Texture*, 64, false>;
        using FTextureArray = StableDynamicArray<RPI::Texture*, 64, false>;
        using FSamplerArray = StableDynamicArray<RHI::Sampler*, 32, false>;

        FSamplerArray samplerArray;
        HashMap<RHI::Sampler*, int> samplerIndices;
        bool samplersUpdated = true;
        u32 systemDpi = 0;

        Array<FTimer*> timers;

        RPI::Shader* fusionShader = nullptr;
        RPI::Shader* fusionShader2 = nullptr;
        RHI::ShaderResourceGroupLayout perViewSrgLayout{};
        RHI::ShaderResourceGroupLayout perDrawSrgLayout{};
        RHI::ShaderResourceGroupLayout perObjectSrgLayout{};

        RHI::DrawListContext drawList{};

        Array<FNativeContext*> nativeWindows{};

        struct DestroyItem
        {
            Ref<Object> object = nullptr;
            int frameCounter = 0;
        };
        Array<DestroyItem> destructionQueue{};

        HashSet<FViewport*> viewports{};

        FUSION_FRIENDS;
        friend class FTimer;
        friend class Engine;
        friend class FGameWindow;
    };
    
} // namespace CE

#include "FusionApplication.rtti.h"
