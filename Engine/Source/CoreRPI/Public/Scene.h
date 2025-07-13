#pragma once

#if PAL_TRAIT_BUILD_TESTS
class RPI_Scene_Test;
#endif

namespace CE::RPI
{
	class FeatureProcessor;
	class Texture;

	class CORERPI_API Scene final
	{
	public:

		struct PipelineStateData
		{
			RHI::MultisampleState multisampleState{};
			Array<RHI::Format> colorFormats{};
			RHI::Format depthFormat{};
			
			SceneViewTag viewTag{};
		};

		using PipelineStateList = Array<PipelineStateData>;

		Scene();
		~Scene();

		FeatureProcessor* AddFeatureProcessor(SubClass<FeatureProcessor> classType);

		FeatureProcessor* GetFeatureProcessor(SubClass<FeatureProcessor> classType);

		void AddDefaultFeatureProcessors();

		template<typename TFeatureProcessor> requires TIsBaseClassOf<FeatureProcessor, TFeatureProcessor>::Value
		TFeatureProcessor* AddFeatureProcessor()
		{
			return (TFeatureProcessor*)AddFeatureProcessor(TFeatureProcessor::Type());
		}

		template<typename TFeatureProcessor> requires TIsBaseClassOf<FeatureProcessor, TFeatureProcessor>::Value
		TFeatureProcessor* GetFeatureProcessor()
		{
			return (TFeatureProcessor*)GetFeatureProcessor(TFeatureProcessor::Type());
		}

		void AddView(SceneViewTag viewTag, ViewPtr view);
		void RemoveView(SceneViewTag viewTag, ViewPtr view);

		void AddRenderPipeline(RenderPipeline* renderPipeline);
		void RemoveRenderPipeline(RenderPipeline* renderPipeline);

		u32 GetRenderPipelineCount() const { return renderPipelines.GetSize(); }

		RenderPipeline* GetRenderPipeline(u32 index) const { return renderPipelines[index]; }

		RHI::ShaderResourceGroup* GetShaderResourceGroup() const { return shaderResourceGroup; }

		RHI::DrawListMask& GetDrawListMask(SceneViewTag viewTag);

		LightConstants& GetLightConstants() { return lightConstants; }

		ArrayView<ViewPtr> GetViews(const SceneViewTag& viewTag);

		const SceneViewsByTag& GetViews() const { return viewsByTag; }

		void Simulate(f32 currentTime);

		void PrepareRender(f32 currentTime, u32 imageIndex);

		void SubmitDrawPackets(RHI::DrawListContext& drawList, u32 imageIndex);

		void OnRenderEnd();

		void CollectDrawPackets();

		void GetPipelineMultiSampleState(RHI::DrawListTag drawListTag, RHI::MultisampleState& multisampleState);

		void RebuildPipelineLookupTable();

		void SetSkyboxCubeMap(RPI::Texture* skyboxCubeMap, RPI::Texture* skyboxIrradiance);

		void SetName(const Name& name) { this->name = name; }

		// For internal use only!
		void SetPrimaryViewportSize(Vec2i size) { primaryViewportSize = size; }

		Vec2i GetPrimaryViewportSize() const { return primaryViewportSize; }

	private:

		bool needsLookupTableRebuild = true;

		CE::Name name = "Scene";

		HashMap<RHI::DrawListTag, PipelineStateList> pipelineLookupMap{};

		Array<RenderPipeline*> renderPipelines{};

		Array<FeatureProcessor*> featureProcessors{};

		FeatureProcessor::SimulatePacket simulatePacket{};
		FeatureProcessor::RenderPacket renderPacket{};

		RPI::Texture* skyboxCubeMap = nullptr;
		RPI::Texture* skyboxIrradiance = nullptr;

		// - Temporary Lighting data -

		LightConstants lightConstants{};
		RPI::Buffer* lightConstantsBuffer = nullptr;

		StaticArray<DirectionalLightConstants, RPI::Limits::MaxDirectionalLightsCount> directionalLightConstants = {};
		RPI::Buffer* directionalLightBuffer = nullptr;

		/// @brief A Scene shader resource group (SRG_PerScene).
		RHI::ShaderResourceGroup* shaderResourceGroup = nullptr;

		SceneViewTag mainViewTag = "MainCamera";

		/// @brief A hash map of all views owned by this scene accessed by their respective tags.
		SceneViewsByTag viewsByTag{};

		Vec2i primaryViewportSize;

		friend class MeshDrawPacket;

#if PAL_TRAIT_BUILD_TESTS
		friend class ::RPI_Scene_Test;
#endif
	};
    
} // namespace CE::RPI
