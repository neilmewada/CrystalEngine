#include "CoreRPI.h"

namespace CE::RPI
{

	ModelLod::ModelLod()
	{

	}

	ModelLod::~ModelLod()
	{
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

	}

} // namespace CE::RPI
