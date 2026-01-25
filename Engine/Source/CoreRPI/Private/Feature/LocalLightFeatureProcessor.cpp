#include "CoreRPI.h"

namespace CE::RPI
{
	void LocalLightInstance::Init(LocalLightFeatureProcessor* fp)
	{
		ZoneScoped;

		if (flags.initialized)
			return;

		flags.initialized = true;
	}

	void LocalLightInstance::Deinit(LocalLightFeatureProcessor* fp)
	{
		ZoneScoped;

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
    }

	LocalLightHandle LocalLightFeatureProcessor::AcquireLight(const LocalLightHandleDescriptor& desc)
	{
		ZoneScoped;

		LocalLightHandle handle = lightInstances.Insert({});
		handle->scene = scene;

		return handle;
	}

	bool LocalLightFeatureProcessor::ReleaseLight(LocalLightHandle& handle)
	{
		ZoneScoped;

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
		ZoneScoped;

		Super::Simulate(packet);

		bool isDirty = false;

		u32 imageIndex = packet.imageIndex;

		if (lightInstances.GetCount() > localLights.GetElementCount())
		{
			localLights.GrowToFit(Math::Max<u32>(localLights.GetElementCount() * BufferGrowRatio, lightInstances.GetCount()));

			isDirty = true;
		}

		std::atomic<int> lightCounter = 0;

		{
			RHI::Buffer* localLightsBuffer = localLights.GetBuffer(imageIndex);

			LocalLightShaderData* data = nullptr;
			localLightsBuffer->Map(0, localLightsBuffer->GetBufferSize(), (void**)&data);

			auto parallelRanges = lightInstances.GetParallelRanges();

			JobCompletion jobCompletion = JobCompletion();

			for (const auto& range : parallelRanges)
			{
				Job* jobFunction = new JobFunction([&range, imageIndex, this, &lightCounter, data](Job*)
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

							int idx = lightCounter.fetch_add(1, std::memory_order_acq_rel);

							data[idx].lightType = it->lightType;
							data[idx].colorAndIntensity = it->colorAndIntensity;
							data[idx].worldPosAndRange = Vec4(it->worldPos, it->range);
						}
					});

				jobFunction->SetDependent(&jobCompletion);
				jobFunction->Start();

				//jobFunction->Process();
				//delete jobFunction;
			}

			jobCompletion.StartAndWaitForCompletion();

			localLightsBuffer->Unmap();
		}

		if (!initialized || isDirty)
		{
			auto sceneSrg = scene->GetShaderResourceGroup();

			for (int i = 0; i < RHI::Limits::MaxSwapChainImageCount; i++)
			{
				sceneSrg->Bind(i, "_LocalLights", localLights.GetBuffer(i));
			}

			sceneSrg->FlushBindings();
			initialized = true;
		}

		auto& lightConstants = scene->GetLightConstants();
		lightConstants.totalLocalLights = lightCounter;
		lightConstants.tileSizeX = lightConstants.tileSizeY = Limits::LocalLightTileSize;
		lightConstants.lightsPerTile = Limits::MaxLightsPerTile;
		lightConstants.lightIndexPoolCapacity = Limits::LightIndexPoolCapacity;
    }

    void LocalLightFeatureProcessor::Render(const RenderPacket& packet)
    {
	    Super::Render(packet);

		u32 imageIndex = packet.imageIndex;

		
    }

} // namespace CE

