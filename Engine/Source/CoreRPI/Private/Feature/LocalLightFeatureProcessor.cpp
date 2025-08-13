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

		views.Clear();
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
				sceneSrg->Bind(i, "_LocalLights", localLights.GetBuffer(i));
				sceneSrg->Bind(i, "_LightIndexPool", lightIndexPool.GetBuffer(i));
				sceneSrg->Bind(i, "_TileHeaders", tileHeaders.GetBuffer(i));
			}

			sceneSrg->FlushBindings();
			initialized = true;
		}

		scene->GetLightConstants().totalLocalLights = lightInstances.GetCount();
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

						it->cameraViews.Clear();

						for (View* view : packet.views)
						{
							if (EnumHasFlag(view->GetUsageFlags(), View::UsageCamera))
							{
								it->cameraViews.Add(view);
							}
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

