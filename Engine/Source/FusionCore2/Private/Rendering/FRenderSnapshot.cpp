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

		FUIDrawList* drawList = layer->GetDrawList();
		
		vertexSplits.Insert({ .startIndex = vertexArray.GetCount(), .count = drawList->vertexArray.GetCount() });
		vertexArray.Insert(drawList->vertexArray.GetData(), drawList->vertexArray.GetCount());

		indexSplits.Insert({ .startIndex = indexArray.GetCount(), .count = drawList->indexArray.GetCount() });
		indexArray.Insert(drawList->indexArray.GetData(), drawList->indexArray.GetCount());

		drawItemSplits.Insert({ .startIndex = drawItemArray.GetCount(), .count = drawList->drawItemArray.GetCount() });
		drawItemArray.Insert(drawList->drawItemArray.GetData(), drawList->drawItemArray.GetCount());

		drawCmdSplits.Insert({ .startIndex = drawCmdArray.GetCount(), .count = drawList->drawCmdArray.GetCount() });
		drawCmdArray.Insert(drawList->drawCmdArray.GetData(), drawList->drawCmdArray.GetCount());

		clipRectSplits.Insert({ .startIndex = clipRectArray.GetCount(), .count = drawList->clipRectArray.GetCount() });
		clipRectArray.Insert(drawList->clipRectArray.GetData(), drawList->clipRectArray.GetCount());

		u32 drawCmdSplitCount = layer->GetSplitPointCount();

		SIZE_T cmdBase = drawCmdSplits.Last().startIndex;
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

				prevSplit = sp;
			}

			// Child's index in the split arrays = current count before it inserts
			DoLayer(layer->GetChild(i), (int)vertexSplits.GetCount());
		}

		FRenderPass rp2 = {
			.renderTarget = nullptr,
			.layerIndex = (SIZE_T)layerIndex,
			.drawCmdStartIndex = cmdBase + prevSplit,
			.drawCmdCount = drawCmdSplits[layerIndex].count - prevSplit
		};

		if (rp2.drawCmdCount == 0)
			return;

		// Final segment after the last split (or the whole thing if no splits)
		renderPassArray.Insert(rp2);
	}
}
