#include "CoreRPI.h"

namespace CE::RPI
{

	ModelLod::ModelLod()
	{

	}

	ModelLod::~ModelLod()
	{
		delete blas; blas = nullptr;

		for (auto buffer : trackedBuffers)
		{
			RPISystem::Get().QueueDestroy(buffer);
		}
		trackedBuffers.Clear();
	}

	void ModelLod::AddMesh(const Mesh& mesh)
	{
		RHI::Buffer* buffer = mesh.indexBufferView.GetBuffer();
		if (buffer == nullptr)
			return;

		for (const VertexBufferInfo& vertexBufferInfo : mesh.vertexBufferInfos)
		{
			totalVertexBuffers = Math::Max(totalVertexBuffers, vertexBufferInfo.bufferIndex + 1);
		}

		meshes.Add(mesh);
	}

	void ModelLod::TrackBuffer(RHI::Buffer* buffer)
	{
		if (!trackedBuffers.Exists(buffer))
		{
			trackedBuffers.Add(buffer);
		}
	}

	void ModelLod::BuildVertexInputSlotDescriptorList(u32 meshIndex, Array<RHI::VertexInputSlotDescriptor>& inputSlots)
	{
		inputSlots.Clear();
		if (meshIndex >= meshes.GetSize())
			return;
		
		const Mesh& mesh = meshes[meshIndex];
		inputSlots.Resize(mesh.vertexBufferInfos.GetSize());
		int slotIndex = 0;

		for (const auto& vertexBufferInfo : mesh.vertexBufferInfos)
		{
			inputSlots[slotIndex].inputRate = RHI::VertexInputRate::PerVertex;
			inputSlots[slotIndex].inputSlot = slotIndex;
			inputSlots[slotIndex].stride = vertexBufferInfo.stride;

			slotIndex++;
		}
	}

	void ModelLod::BuildVertexInputAttributeList(u32 meshIndex, Array<RHI::VertexAttributeDescriptor>& vertexAttribs)
	{
		vertexAttribs.Clear();
		if (meshIndex >= meshes.GetSize())
			return;
		
		const Mesh& mesh = meshes[meshIndex];
		vertexAttribs.Resize(mesh.vertexBufferInfos.GetSize());
		int slotIndex = 0;
		
		for (const auto& vertexBufferInfo : mesh.vertexBufferInfos)
		{
			vertexAttribs[slotIndex].dataType = vertexBufferInfo.attributeType;
			vertexAttribs[slotIndex].inputSlot = slotIndex;
			vertexAttribs[slotIndex].location = slotIndex;
			vertexAttribs[slotIndex].offset = 0;
			
			slotIndex++;
		}
	}

	void ModelLod::OnPostProcess()
	{
		if (RPISystem::Get().IsRayTracingEnabled() && blas == nullptr)
		{
			RHI::RayTracingBlasDescriptor blasDesc{};
			blasDesc.buildFlags = RHI::RayTracingBuildFlags::FastTrace;
			blasDesc.geometries.Reserve(meshes.GetSize());

			for (const RPI::Mesh& mesh : meshes)
			{
				RHI::RayTracingGeometryDescriptor geometryDesc{};
				
				for (const auto& vertexBufferInfo : mesh.vertexBufferInfos)
				{
					if (vertexBufferInfo.semantic.attribute == VertexInputAttribute::Position)
					{
						geometryDesc.vertexBuffer = RHI::VertexBufferView(GetBuffer(vertexBufferInfo.bufferIndex), 
							vertexBufferInfo.byteOffset, 
							vertexBufferInfo.byteCount, 
							vertexBufferInfo.stride);

						geometryDesc.vertexOffset = mesh.drawArguments.indexedArgs.vertexOffset;

						geometryDesc.indexBuffer = mesh.indexBufferView;
						geometryDesc.vertexDataType = vertexBufferInfo.attributeType;

						blasDesc.geometries.Add(geometryDesc);

						break;
					}
				}
			}

			blas = RHI::gDynamicRHI->CreateRayTracingBlas(blasDesc);

			RPISystem::Get().EnqueueBlasBuild(this);
			return;

			RHI::CommandQueue* queue = RHI::gDynamicRHI->GetPrimaryGraphicsQueue();
			RHI::CommandList* cmdList = RHI::gDynamicRHI->AllocateCommandList(queue);
			RHI::Fence* fence = RHI::gDynamicRHI->CreateFence(false);

			cmdList->Begin();
			{
				cmdList->BuildBlas(blas);
			}
			cmdList->End();

			queue->Execute(1, &cmdList, fence);
			fence->WaitForFence();

			RHI::gDynamicRHI->FreeCommandLists(1, &cmdList);
			RHI::gDynamicRHI->DestroyFence(fence);
		}
	}

} // namespace CE::RPI
