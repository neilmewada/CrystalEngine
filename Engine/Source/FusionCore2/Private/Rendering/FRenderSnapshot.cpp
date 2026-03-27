#include "FusionCore.h"

namespace CE
{
	FRenderSnapshot::FRenderSnapshot(RHI::DrawListTag drawListTag) : drawListTag(drawListTag)
	{
		
	}

	void FRenderSnapshot::Clear()
	{
		ZoneScoped;

		vertexArray.RemoveAll();
		vertexSplits.RemoveAll();

		indexArray.RemoveAll();
		indexSplits.RemoveAll();

		drawItemArray.RemoveAll();
		drawItemSplits.RemoveAll();

		drawCmdArray.RemoveAll();
		drawCmdSplits.RemoveAll();

		clipRectArray.RemoveAll();
		clipRectSplits.RemoveAll();

		gradientStopArray.RemoveAll();
		gradientStopSplits.RemoveAll();

		renderPassArray.RemoveAll();

		matricesPerLayer.RemoveAll();
	}

	void FRenderSnapshot::BuildSnapshot(Ref<FLayerTree> layerTree)
	{
		ZoneScoped;

		Clear();

		DoLayer(layerTree->GetRootLayer(), 0);
	}

	void FRenderSnapshot::DoLayer(Ref<FLayer> layer, int layerIndex)
	{
		ZoneScoped;

		layer->needsCompositing = false;

		RHI::DeviceLimits* limits = gDynamicRHI->GetDeviceLimits();
		SIZE_T structuredBufferAlignment = limits->GetMinStructuredBufferOffsetAlignment();

		FUIDrawList* drawList = layer->GetDrawList();

		vertexSplits.Insert({ .startOffset = vertexArray.GetByteSize(), .byteSize = drawList->vertexArray.GetByteSize() });
		vertexArray.Insert(drawList->vertexArray.GetData(), drawList->vertexArray.GetCount());

		indexSplits.Insert({ .startOffset = indexArray.GetByteSize(), .byteSize = drawList->indexArray.GetByteSize() });
		indexArray.Insert(drawList->indexArray.GetData(), drawList->indexArray.GetCount());

		{
			SIZE_T alignedOffset = Memory::AlignUp(drawItemArray.GetByteSize(), structuredBufferAlignment);
			drawItemArray.InsertRange((int)((alignedOffset - drawItemArray.GetByteSize()) / sizeof(FUIDrawItem)));
			drawItemSplits.Insert({ .startOffset = alignedOffset, .byteSize = drawList->drawItemArray.GetByteSize() });
			drawItemArray.Insert(drawList->drawItemArray.GetData(), drawList->drawItemArray.GetCount());
		}

		drawCmdSplits.Insert({ .startOffset = drawCmdArray.GetByteSize(), .byteSize = drawList->drawCmdArray.GetByteSize() });
		drawCmdArray.Insert(drawList->drawCmdArray.GetData(), drawList->drawCmdArray.GetCount());

		{
			SIZE_T alignedOffset = Memory::AlignUp(clipRectArray.GetByteSize(), structuredBufferAlignment);
			clipRectArray.InsertRange((int)((alignedOffset - clipRectArray.GetByteSize()) / sizeof(FUIClipRect)));
			clipRectSplits.Insert({ .startOffset = alignedOffset, .byteSize = drawList->clipRectArray.GetByteSize() });
			clipRectArray.Insert(drawList->clipRectArray.GetData(), drawList->clipRectArray.GetCount());
		}

		{
			SIZE_T alignedOffset = Memory::AlignUp(gradientStopArray.GetByteSize(), structuredBufferAlignment);
			gradientStopArray.InsertRange((int)((alignedOffset - gradientStopArray.GetByteSize()) / sizeof(FUIGradientStop)));
			gradientStopSplits.Insert({ .startOffset = alignedOffset, .byteSize = drawList->gradientStopArray.GetByteSize() });
			gradientStopArray.Insert(drawList->gradientStopArray.GetData(), drawList->gradientStopArray.GetCount());
		}

		u32 drawCmdSplitCount = layer->GetSplitPointCount();

		SIZE_T cmdBase = drawCmdSplits.Last().startOffset / sizeof(FUIDrawCmd);
		u32 prevSplit = 0;

		Matrix4x4 layerGlobalMatrix = layer->GetGlobalTransform().ToMatrix4x4();
		matricesPerLayer.Insert(layerGlobalMatrix);

		for (u32 i = 0; i < drawCmdSplitCount; i++)
		{
			u32 sp = layer->GetSplitPoint(i);

			FRenderPass rp1 = {
				.renderTarget = nullptr,
				.layerIndex = (SIZE_T)layerIndex,
				.drawCmdStartIndex = cmdBase + prevSplit,
				.drawCmdCount = sp - prevSplit   // excludes the placeholder at sp
			};

			if (rp1.drawCmdCount > 0)
			{
				// Emit render pass for this layer's cmds before the split point
				renderPassArray.Insert(rp1);

				prevSplit = sp; // There is no "dummy" draw command for split points.
			}

			// Child's index in the split arrays = current count before it inserts
			DoLayer(layer->GetChild(i), (int)vertexSplits.GetCount());
		}

		FRenderPass rp2 = {
			.renderTarget = nullptr,
			.layerIndex = (SIZE_T)layerIndex,
			.drawCmdStartIndex = cmdBase + prevSplit,
			.drawCmdCount = drawCmdSplits[layerIndex].byteSize / sizeof(FUIDrawCmd) - prevSplit
		};

		if (rp2.drawCmdCount == 0)
			return;

		// Final segment after the last split (or the whole thing if no splits)
		renderPassArray.Insert(rp2);
	}
}
