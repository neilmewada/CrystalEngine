#include "CoreRHI.h"

namespace CE::RHI
{
	/*
	 * Copyright (c) Contributors to the Open 3D Engine Project.
	 * Used under MIT license. https://github.com/o3de/o3de/blob/development/LICENSE_MIT.TXT
	 */

	DrawPacketBuilder::DrawPacketBuilder(IAllocator* allocator)
	{
		if (!allocator)
			allocator = SystemAllocator::Get();
		this->allocator = allocator;
	}

	void DrawPacketBuilder::SetRootConstants(const u8* rootConstants, u8 rootConstantSize)
	{
		if (rootConstantSize > Limits::Pipeline::MaxRootConstantSize)
		{
			CE_LOG(Error, All, "Cannot set root constants of size {}. It needs be <= {}.", rootConstantSize, Limits::Pipeline::MaxRootConstantSize);
			return;
		}

		this->rootConstantSize = rootConstantSize;

		for (int i = 0; i < rootConstantSize; i++)
		{
			this->rootConstants[i] = rootConstants[i];
		}
	}

	void DrawPacketBuilder::SetDrawArguments(const DrawArguments& args)
	{
		this->drawArguments = args;
	}

	void DrawPacketBuilder::SetDebugName(const Name& name)
	{
		this->debugName = name;
	}

	void DrawPacketBuilder::AddScissorState(const ScissorState& scissorState)
	{
		if (scissors.GetSize() >= scissors.GetCapacity())
			return;

		scissors.Add(scissorState);
	}

	void DrawPacketBuilder::AddViewportState(const ViewportState& viewportState)
	{
		if (viewports.GetSize() >= viewports.GetCapacity())
			return;

		viewports.Add(viewportState);
	}

	void DrawPacketBuilder::AddShaderResourceGroup(ShaderResourceGroup* srg)
	{
		//if (srg == nullptr)
		//	return;
		if (shaderResourceGroupCount >= shaderResourceGroups.GetSize())
			return;

		for (int i = 0; i < shaderResourceGroupCount; i++)
		{
			if (shaderResourceGroups[i] == srg)
				return;
		}

		shaderResourceGroups[shaderResourceGroupCount++] = srg;
	}

	void DrawPacketBuilder::AddDrawItem(const DrawItemRequest& request)
	{
		if (drawRequests.GetSize() >= drawRequests.GetCapacity())
			return;
		if (request.vertexBufferViews.IsEmpty() || (request.pipelineState == nullptr && request.pipelineCollection == nullptr))
			return;
		
        drawListMask.Set(request.drawItemTag);
		vertexBufferViewCount += request.vertexBufferViews.GetSize();
		uniqueShaderResourceGroupCount += request.uniqueShaderResourceGroups.GetSize();
		drawRequests.Add(request);
	}

	DrawPacket* DrawPacketBuilder::Build()
	{
		ZoneScoped;

		if (drawRequests.GetSize() == 0)
			return nullptr;

		SIZE_T drawPacketOffset = AllocateOffset(sizeof(DrawPacket), alignof(DrawPacket));

		SIZE_T drawItemsOffset = AllocateOffset(sizeof(DrawItem) * drawRequests.GetSize(), alignof(DrawItem));

		SIZE_T drawListTagsOffset = AllocateOffset(sizeof(DrawListTag) * drawRequests.GetSize(), alignof(DrawListTag));

		SIZE_T drawFilterMasksOffset = AllocateOffset(sizeof(DrawFilterMask) * drawRequests.GetSize(), alignof(DrawFilterMask));

		SIZE_T rootConstantsOffset = AllocateOffset(sizeof(u8) * rootConstantSize, alignof(u8));

		SIZE_T shaderResourceGroupsOffset = AllocateOffset(sizeof(ShaderResourceGroup*) * shaderResourceGroupCount, alignof(ShaderResourceGroup*));

		SIZE_T uniqueShaderResourceGroupOffset = AllocateOffset(sizeof(ShaderResourceGroup*) * uniqueShaderResourceGroupCount, alignof(ShaderResourceGroup*));

		SIZE_T vertexBufferViewsOffset = AllocateOffset(sizeof(VertexBufferView) * vertexBufferViewCount, alignof(VertexBufferView));

		SIZE_T indexBufferViewsOffset = AllocateOffset(sizeof(VertexBufferView) * drawRequests.GetSize(), alignof(IndexBufferView));

		SIZE_T scissorsOffset = AllocateOffset(sizeof(ScissorState) * scissors.GetSize(), alignof(ScissorState));

		SIZE_T viewportsOffset = AllocateOffset(sizeof(ViewportState) * viewports.GetSize(), alignof(ViewportState));

		SIZE_T totalByteCount = byteOffsetCurrent;

		u8* allocationData = (u8*)allocator->AlignedAlloc(totalByteCount, alignof(DrawPacket));

		auto drawPacket = new(allocationData) DrawPacket();
		drawPacket->allocator = allocator;
        drawPacket->drawListMask = drawListMask;
        
        if (shaderResourceGroupCount > 0)
        {
            ShaderResourceGroup** shaderResourceGroups = reinterpret_cast<ShaderResourceGroup**>(allocationData + shaderResourceGroupsOffset);
            
            for (int i = 0; i < shaderResourceGroupCount; i++) 
            {
                shaderResourceGroups[i] = this->shaderResourceGroups[i];
            }
            
            drawPacket->shaderResourceGroups = shaderResourceGroups;
            drawPacket->shaderResourceGroupCount = shaderResourceGroupCount;
        }
        
        if (scissors.GetSize() > 0)
        {
            ScissorState* scissorsStates = reinterpret_cast<ScissorState*>(allocationData + scissorsOffset);
            
            for (int i = 0; i < scissors.GetSize(); i++)
            {
                scissors[i] = this->scissors[i];
            }
            
            drawPacket->scissors = scissorsStates;
            drawPacket->scissorCount = scissors.GetSize();
        }
        
        if (viewports.GetSize() > 0)
        {
            ViewportState* viewportStates = reinterpret_cast<ViewportState*>(allocationData + viewportsOffset);
            
            for (int i = 0; i < viewports.GetSize(); i++)
            {
                viewports[i] = this->viewports[i];
            }
            
            drawPacket->viewports = viewportStates;
            drawPacket->viewportCount = viewports.GetSize();
        }

		if (rootConstantSize > 0)
		{
			auto rootConstants = reinterpret_cast<u8*>(allocationData + rootConstantsOffset);
			memcpy(rootConstants, this->rootConstants.GetData(), rootConstantSize);
			
			drawPacket->rootConstants = rootConstants;
			drawPacket->rootConstantSize = rootConstantSize;
		}
        
        auto drawItems = reinterpret_cast<DrawItem*>(allocationData + drawItemsOffset);
        auto drawListTags = reinterpret_cast<DrawListTag*>(allocationData + drawListTagsOffset);
        auto drawFilterMasks = reinterpret_cast<DrawFilterMask*>(allocationData + drawFilterMasksOffset);

		drawPacket->debugName = debugName;
        drawPacket->drawItems = drawItems;
        drawPacket->drawListTags = drawListTags;
        drawPacket->drawFilterMasks = drawFilterMasks;
		drawPacket->drawItemCount = drawRequests.GetSize();

		auto indexBufferViews = reinterpret_cast<IndexBufferView*>(allocationData + indexBufferViewsOffset);
        
        for (int i = 0; i < drawRequests.GetSize(); i++)
        {
            const DrawItemRequest& drawRequest = this->drawRequests[i];
            
            drawListTags[i] = drawRequest.drawItemTag;
            drawFilterMasks[i] = drawRequest.drawFilterMask;
            
            DrawItem& drawItem = drawItems[i];
        	drawItem.debugName = debugName;
            drawItem.enabled = true; // TODO: Add enabled flag
            drawItem.indexBufferView = indexBufferViews;
            drawItem.vertexBufferViewCount = 0;
			drawItem.vertexBufferViews = nullptr;
            drawItem.scissorCount = drawPacket->scissorCount;
			drawItem.scissors = drawPacket->scissors;
            drawItem.viewportCount = drawPacket->viewportCount;
            drawItem.viewports = drawPacket->viewports;
            drawItem.shaderResourceGroups = drawPacket->shaderResourceGroups;
            drawItem.shaderResourceGroupCount = drawPacket->shaderResourceGroupCount;
			drawItem.uniqueShaderResourceGroups = nullptr;
			drawItem.uniqueShaderResourceGroupCount = 0;
            drawItem.stencilRef = drawRequest.stencilRef;
            drawItem.pipelineState = drawRequest.pipelineState;
			drawItem.pipelineCollection = drawRequest.pipelineCollection;
            drawItem.arguments = drawArguments;
			drawItem.rootConstants = drawPacket->rootConstants;
			drawItem.rootConstantSize = drawPacket->rootConstantSize;
            
			*indexBufferViews++ = drawRequest.indexBufferView;
        }

		if (vertexBufferViewCount > 0)
		{
			auto vertexBufferViews = reinterpret_cast<VertexBufferView*>(allocationData + vertexBufferViewsOffset);

			drawPacket->vertexBufferViews = vertexBufferViews;
			drawPacket->vertexBufferViewCount = vertexBufferViewCount;

			for (int i = 0; i < drawRequests.GetSize(); i++)
			{
				const DrawItemRequest& drawRequest = this->drawRequests[i];

				if (!drawRequest.vertexBufferViews.IsEmpty())
				{
					DrawItem& drawItem = drawItems[i];
					drawItem.vertexBufferViews = vertexBufferViews;
					drawItem.vertexBufferViewCount = drawRequest.vertexBufferViews.GetSize();

					for (const VertexBufferView& vertexBufferView : drawRequest.vertexBufferViews)
					{
						*vertexBufferViews++ = vertexBufferView;
					}
				}
			}
		}

		if (uniqueShaderResourceGroupCount > 0)
		{
			ShaderResourceGroup** uniqueShaderResourceGroups = reinterpret_cast<ShaderResourceGroup**>(allocationData + uniqueShaderResourceGroupOffset);

			drawPacket->uniqueShaderResourceGroups = uniqueShaderResourceGroups;
			drawPacket->uniqueShaderResourceGroupCount = uniqueShaderResourceGroupCount;

			for (int i = 0; i < drawRequests.GetSize(); i++)
			{
				const DrawItemRequest& drawRequest = this->drawRequests[i];

				if (!drawRequest.uniqueShaderResourceGroups.IsEmpty())
				{
					DrawItem& drawItem = drawItems[i];
					drawItem.uniqueShaderResourceGroups = uniqueShaderResourceGroups;
					drawItem.uniqueShaderResourceGroupCount = drawRequest.uniqueShaderResourceGroups.GetSize();

					for (RHI::ShaderResourceGroup* uniqueSrg : drawRequest.uniqueShaderResourceGroups)
					{
						*uniqueShaderResourceGroups++ = uniqueSrg;
					}
				}
			}
		}
        
		return drawPacket;
	}

	SIZE_T DrawPacketBuilder::AllocateOffset(SIZE_T byteCount, SIZE_T byteAlignment)
	{
        if (byteCount == 0)
            return byteOffsetCurrent;
        
		SIZE_T addressCurrentAligned = Memory::GetAlignedSize(byteOffsetCurrent, byteAlignment);
		SIZE_T byteCountAligned = Memory::GetAlignedSize(byteCount, byteAlignment);
		SIZE_T nextByteAddress = addressCurrentAligned + byteCountAligned;

		byteOffsetCurrent = nextByteAddress;
		return addressCurrentAligned;
	}

} // namespace CE::RHI
