#pragma once

namespace CE
{
	class SceneSubsystem;
	class ActorComponent;

	CLASS()
	class ENGINE_API RendererSubsystem : public EngineSubsystem, ApplicationMessageHandler
	{
		CE_CLASS(RendererSubsystem, EngineSubsystem)
	public:

		RendererSubsystem();

		RHI::FrameScheduler* GetScheduler() const { return scheduler; }
    
		FUNCTION()
		void RebuildFrameGraph();

		f32 GetTickPriority() const override;

		void RegisterFeatureProcessor(SubClass<ActorComponent> componentClass, SubClass<RPI::FeatureProcessor> fpClass);

		template<typename TActorComponent, typename TFeatureProcessor>
			requires TIsBaseClassOf<ActorComponent, TActorComponent>::Value and TIsBaseClassOf<RPI::FeatureProcessor, TFeatureProcessor>::Value
		void RegisterFeatureProcessor()
		{
			RegisterFeatureProcessor(TActorComponent::StaticType(), TFeatureProcessor::StaticType());
		}

		SubClass<RPI::FeatureProcessor> GetFeatureProcessClass(SubClass<ActorComponent> componentClass);

		FUNCTION()
		void AddViewport(FGameWindow* viewport);

		FUNCTION()
		void RemoveViewport(FGameWindow* viewport);

		const Array<FGameWindow*>& GetAllViewports() const { return renderViewports; }

	protected:

		void OnSceneDestroyed(CE::Scene* scene);

		void OnWindowCreated(PlatformWindow* window) override;
		void OnWindowDestroyed(PlatformWindow* window) override;
		void OnWindowClosed(PlatformWindow* window) override;
		void OnWindowMinimized(PlatformWindow* window) override;
		void OnWindowResized(PlatformWindow* window, u32 newWidth, u32 newHeight) override;
		void OnWindowRestored(PlatformWindow* window) override;
		void OnWindowExposed(PlatformWindow* window) override;

		void Initialize() override;

		void PostInitialize() override;

		void PreShutdown() override;

		void Shutdown() override;

		void Tick(f32 delta) override;

		void BuildFrameGraph();
		void CompileFrameGraph();

		void SubmitDrawPackets(int imageIndex);

	private:

		// - Fields -
		
		FIELD()
		SceneSubsystem* sceneSubsystem = nullptr;

		HashMap<ClassType*, SubClass<RPI::FeatureProcessor>> componentClassToFeatureProcessorClass{};
		HashMap<u64, Vec2i> windowSizesById;
		
		u32 curImageIndex = 0;

		Array<FGameWindow*> renderViewports;

		// - Frame Graph -

		RHI::FrameScheduler* scheduler = nullptr;

		RHI::DrawListContext drawList{};

		bool rebuildFrameGraph = true;
		bool recompileFrameGraph = true;

		friend class SceneSubsystem;
	};
    
} // namespace CE

#include "RendererSubsystem.rtti.h"
