#include "CoreRPI.h"

namespace CE::RPI
{
	void LocalLightInstance::Init(LocalLightFeatureProcessor* fp)
	{
		if (flags.initialized)
			return;

		flags.initialized = true;
	}

	void LocalLightInstance::Deinit(LocalLightFeatureProcessor* fp)
	{
		if (!flags.initialized)
			return;

		shadowViews.Clear();
		flags.initialized = false;
	}

	void LocalLightInstance::UpdateSrgs(int imageIndex)
	{

	}

	LocalLightFeatureProcessor::LocalLightFeatureProcessor()
    {

    }

    void LocalLightFeatureProcessor::OnAfterConstruct()
    {
	    Super::OnAfterConstruct();

		if (IsDefaultInstance())
			return;

		localLights.Init("LocalLightDataBuffer", LightDataBufferInitialSize);
		lightIndexPool.Init("LightIndexPool", LightIndexPoolBufferInitialSize);
		tileHeaders.Init("TileHeaders", TileHeaderBufferInitialSize);
    }

    void LocalLightFeatureProcessor::OnBeforeDestroy()
    {
	    Super::OnBeforeDestroy();

		auto parallelRanges = lightInstances.GetParallelRanges();

		for (const auto& range : parallelRanges)
		{
			for (auto it = range.begin; it != range.end; ++it)
			{
				it->Deinit(this);
			}
		}

		localLights.Shutdown();
		lightIndexPool.Shutdown();
		tileHeaders.Shutdown();
    }

	LocalLightHandle LocalLightFeatureProcessor::AcquireLight(const LocalLightHandleDescriptor& desc)
	{
		LocalLightHandle handle = lightInstances.Insert({});
		handle->scene = scene;

		return handle;
	}

	bool LocalLightFeatureProcessor::ReleaseLight(LocalLightHandle& handle)
	{
		if (handle.IsValid())
		{
			handle->Deinit(this);
			lightInstances.Remove(handle);
			return true;
		}

		return false;
	}

    void LocalLightFeatureProcessor::Simulate(const SimulatePacket& packet)
    {
		Super::Simulate(packet);

		bool isDirty = false;

		if (lightInstances.GetCount() > localLights.GetElementCount())
		{
			localLights.GrowToFit(Math::Max<u32>(localLights.GetElementCount() * BufferGrowRatio, lightInstances.GetCount()));

			isDirty = true;
		}

		if (!initialized || isDirty)
		{
			auto sceneSrg = scene->GetShaderResourceGroup();

			for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
			{
				//sceneSrg->Bind(i, "_LocalLights", localLights.GetBuffer(i));
				//sceneSrg->Bind(i, "_LightIndexPool", lightIndexPool.GetBuffer(i));
				//sceneSrg->Bind(i, "_TileHeaders", tileHeaders.GetBuffer(i));
			}

			sceneSrg->FlushBindings();
			initialized = true;
		}

		constexpr u32 MaxResolution = 4096;
		constexpr u32 LightIndexPoolCapacity = (MaxResolution * MaxResolution) / (Limits::LocalLightTileSize * Limits::LocalLightTileSize) * Limits::MaxLightsPerTile;

		auto& lightConstants = scene->GetLightConstants();
		lightConstants.totalLocalLights = lightInstances.GetCount();
		lightConstants.tileSizeX = lightConstants.tileSizeY = Limits::LocalLightTileSize;
		lightConstants.lightsPerTile = Limits::MaxLightsPerTile;
		lightConstants.lightIndexPoolCapacity = LightIndexPoolCapacity;
    }

    void LocalLightFeatureProcessor::Render(const RenderPacket& packet)
    {
	    Super::Render(packet);

		u32 imageIndex = packet.imageIndex;

		JobCompletion jobCompletion{};

		auto parallelRanges = lightInstances.GetParallelRanges();

		for (const auto& range : parallelRanges)
		{
			Job* jobFunction = new JobFunction([&range, imageIndex, &packet, this](Job*)
				{
					for (auto it = range.begin; it != range.end; ++it)
					{
						if (!it->flags.initialized)
						{
							it->Init(this);
						}

						if (!it->flags.visible)
						{
							continue;
						}

						it->UpdateSrgs(imageIndex);
					}
				});

			jobFunction->SetDependent(&jobCompletion);
			jobFunction->Start();
		}

		jobCompletion.StartAndWaitForCompletion();
    }

} // namespace CE

