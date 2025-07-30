#include "CoreRPI.h"

/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

namespace CE::RPI
{

	StaticMeshFeatureProcessor::StaticMeshFeatureProcessor()
	{
		
	}

	StaticMeshFeatureProcessor::~StaticMeshFeatureProcessor()
	{
		auto parallelRanges = modelInstances.GetParallelRanges();

		for (const auto& range : parallelRanges)
		{
			for (auto it = range.begin; it != range.end; ++it)
			{
				it->Deinit(this);
			}
		}
	}

	void ModelDataInstance::Init(StaticMeshFeatureProcessor* fp)
	{
		if (flags.initialized)
			return;
		
		flags.initialized = true;

		for (int i = 0; i < objectBuffers.GetSize(); ++i)
		{
			RHI::BufferDescriptor bufferDescriptor{};
			bufferDescriptor.name = "ObjectBuffer " + debugName.GetString();
			bufferDescriptor.bindFlags = RHI::BufferBindFlags::ConstantBuffer;
			bufferDescriptor.bufferSize = sizeof(Matrix4x4);
			bufferDescriptor.defaultHeapType = RHI::MemoryHeapType::Upload;
			bufferDescriptor.structureByteStride = sizeof(Matrix4x4);

			objectBuffers[i] = RHI::gDynamicRHI->CreateBuffer(bufferDescriptor);

			Matrix4x4 defaultValue = Matrix4x4::Identity();

			objectBuffers[i]->UploadData(&defaultValue, sizeof(defaultValue));
		}

		for (int i = 0; i < model->GetModelLodCount(); ++i)
		{
			BuildDrawPacketList(fp, i);
		}
	}

	void ModelDataInstance::Deinit(StaticMeshFeatureProcessor* fp)
	{
		for (auto& objectBuffer : objectBuffers)
		{
			RPISystem::Get().QueueDestroy(objectBuffer);
			objectBuffer = nullptr;
		}

		for (auto& srg : objectSrgList)
		{
			delete srg; srg = nullptr;
		}

		objectSrgList.Clear();

		flags.initialized = false;
	}

	void ModelDataInstance::BuildDrawPacketList(StaticMeshFeatureProcessor* fp, u32 modelLodIndex)
	{
		ModelLod* lod = model->GetModelLod(modelLodIndex);
		SIZE_T meshCount = lod->GetMeshCount();

		if (drawPacketsListByLod.GetSize() != model->GetModelLodCount())
		{
			drawPacketsListByLod.Resize(model->GetModelLodCount());
		}

		MeshDrawPacketList& drawPacketList = drawPacketsListByLod[modelLodIndex];
		drawPacketList.Clear();

		for (RHI::ShaderResourceGroup* srg : objectSrgList)
		{
			delete srg;
		}

		objectSrgList.Clear();

		for (int i = 0; i < meshCount; i++)
		{
			CustomMaterialId materialId = CustomMaterialId(modelLodIndex, i);
			RPI::Material* material = materialMap[materialId];
			if (material == nullptr)
			{
				material = materialMap[DefaultCustomMaterialId];
				if (material == nullptr)
				{
					continue;
				}
			}

			const auto& objectSrgLayout = material->GetCurrentShader()->GetDefaultVariant()->GetSrgLayout(RHI::SRGType::PerObject);

			RHI::ShaderResourceGroup* objectSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(objectSrgLayout);

			for (int j = 0; j < objectBuffers.GetSize(); ++j)
			{
				objectSrg->Bind(j, "_ObjectData", objectBuffers[j]);
			}

			objectSrg->FlushBindings();

			objectSrgList.Add(objectSrg);

			RPI::MeshDrawPacket& packet = drawPacketList.EmplaceBack(lod, i, objectSrg, material);
			packet.SetDebugName(debugName);
			
			packet.SetStencilRef(0);
			packet.Update(scene, false);
		}
	}

	void ModelDataInstance::UpdateDrawPackets(StaticMeshFeatureProcessor* fp, bool forceUpdate)
	{
		for (MaterialLodIndex i = 0; i < drawPacketsListByLod.GetSize(); ++i)
		{
			for (MaterialMeshIndex j = 0; j < drawPacketsListByLod[i].GetSize(); ++j)
			{
				CustomMaterialId materialId = CustomMaterialId(i, j);
				
				if (drawPacketsListByLod[i][j].GetMaterial() != materialMap[materialId])
				{
					drawPacketsListByLod[i][j].SetMaterial(materialMap[materialId]);
				}

				drawPacketsListByLod[i][j].Update(scene, forceUpdate);
			}
		}
	}

	void ModelDataInstance::UpdateSrgs(int imageIndex)
	{
		objectBuffers[imageIndex]->UploadData(&localToWorldTransform, sizeof(Matrix4x4));
	}

	ModelHandle StaticMeshFeatureProcessor::AcquireMesh(const ModelHandleDescriptor& modelHandleDescriptor, const CustomMaterialMap& materialMap)
	{
		ModelHandle handle = modelInstances.Insert({});
		handle->debugName = modelHandleDescriptor.debugName;
		handle->model = modelHandleDescriptor.model;
		handle->originalModel = modelHandleDescriptor.originalModel;
		handle->scene = this->scene;
		handle->materialMap = materialMap;
		
		return handle;
	}

	ModelHandle StaticMeshFeatureProcessor::AcquireMesh(const ModelHandleDescriptor& modelHandleDescriptor,
		RPI::Material* defaultMaterial)
	{
		if (defaultMaterial == nullptr)
		{
			return ModelHandle();
		}

		CustomMaterialMap materialMap{};
		materialMap[DefaultCustomMaterialId] = defaultMaterial;

		return AcquireMesh(modelHandleDescriptor, materialMap);
	}

	bool StaticMeshFeatureProcessor::ReleaseMesh(ModelHandle& handle)
	{
		if (handle.IsValid())
		{
			handle->Deinit(this);
			modelInstances.Remove(handle);
			return true;
		}

		return false;
	}

	void StaticMeshFeatureProcessor::Simulate(const SimulatePacket& packet)
	{
		ZoneScoped;

		Super::Simulate(packet);

		JobCompletion jobCompletion = JobCompletion();

		Array<Job*> initJobs = CreateInitJobs();

		if (initJobs.GetSize() == 1)
		{
			initJobs[0]->Process();
			initJobs[0]->Finish();
			delete initJobs[0];
			initJobs.Clear();
		}
		else if (initJobs.GetSize() > 1)
		{
			for (Job* initJob : initJobs)
			{
				initJob->SetDependent(&jobCompletion);
				initJob->Start();
			}
		}

		jobCompletion.StartAndWaitForCompletion();

		forceRebuildDrawPackets = false;
	}

	void StaticMeshFeatureProcessor::Render(const RenderPacket& packet)
	{
		ZoneScoped;

		Super::Render(packet);

		auto parallelRanges = modelInstances.GetParallelRanges();

		JobCompletion jobCompletion{};
		int imageIndex = packet.imageIndex;

		// - Enqueue Draw Packets to relevant views -

		for (const auto& range : parallelRanges)
		{
			Job* jobFunction = new JobFunction([&range, imageIndex, &packet](Job*)
				{
					for (View* view : packet.views)
					{
						for (auto it = range.begin; it != range.end; ++it)
						{
							if (it->drawPacketsListByLod.IsEmpty())
								continue;
							if (!it->flags.visible)
								continue;

							it->UpdateSrgs(imageIndex);

							for (RHI::ShaderResourceGroup* objectSrg : it->objectSrgList)
							{
								objectSrg->FlushBindings();
							}

							const auto& meshDrawPacketList = it->drawPacketsListByLod[0];

							for (int i = 0; i < meshDrawPacketList.GetSize(); ++i)
							{
								RHI::DrawPacket* drawPacket = meshDrawPacketList[i].GetDrawPacket();
								view->AddDrawPacket(drawPacket, 0);
							}
						}
					}
				});

			jobFunction->SetDependent(&jobCompletion);
			jobFunction->Start();
		}

		jobCompletion.StartAndWaitForCompletion();
	}

	void StaticMeshFeatureProcessor::OnRenderEnd()
	{
		ZoneScoped;

		Super::OnRenderEnd();

		
	}

	Array<Job*> StaticMeshFeatureProcessor::CreateInitJobs()
	{
		ZoneScoped;

		Array<Job*> jobs{};

		auto ranges = modelInstances.GetParallelRanges();

		for (const auto& range : ranges)
		{
			const auto initJobLambda = [this, range](Job* job)
				{
					for (auto it = range.begin; it != range.end; ++it)
					{
						if (!it->flags.visible)
						{
							continue;
						}

						if (!it->flags.initialized)
						{
							it->Init(this);
						}
						
						it->UpdateDrawPackets(this, forceRebuildDrawPackets);
					}
				};

			jobs.Add(new JobFunction(initJobLambda, true));
		}

		return jobs;
	}

} // namespace CE::RPI
