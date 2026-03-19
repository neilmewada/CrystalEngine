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

		renderPassArray.RemoveAll();

		matricesPerLayer.RemoveAll();
	}

	void FRenderSnapshot::BuildSnapshot(Ref<FLayer> layer)
	{
		ZoneScoped;

		Clear();

		DoLayer(layer, 0);
	}

	void FRenderSnapshot::DoLayer(Ref<FLayer> layer, int layerIndex)
	{
		ZoneScoped;

		FUIDrawList* drawList = layer->GetDrawList();
		
		vertexSplits.Insert({ .startIndex = vertexArray.GetCount(), .count = drawList->vertexArray.GetCount() });
		vertexArray.Insert(drawList->vertexArray.GetData(), drawList->vertexArray.GetCount());

		indexSplits.Insert({ .startIndex = indexArray.GetCount(), .count = drawList->indexArray.GetCount() });
		indexArray.Insert(drawList->indexArray.GetData(), drawList->indexArray.GetCount());

		drawItemSplits.Insert({ .startIndex = drawItemArray.GetCount(), .count = drawList->drawItemArray.GetCount() });
		drawItemArray.Insert(drawList->drawItemArray.GetData(), drawList->drawItemArray.GetCount());

		drawCmdSplits.Insert({ .startIndex = drawCmdArray.GetCount(), .count = drawList->drawCmdArray.GetCount() });
		drawCmdArray.Insert(drawList->drawCmdArray.GetData(), drawList->drawCmdArray.GetCount());

		u32 drawCmdSplitCount = layer->GetSplitPointCount();

		SIZE_T cmdBase = drawCmdSplits.Last().startIndex;
		u32 prevSplit = 0;

		Matrix4x4 layerGlobalMatrix = layer->GetGlobalTransform().ToMatrix4x4();
		matricesPerLayer.Insert(layerGlobalMatrix);

		for (u32 i = 0; i < drawCmdSplitCount; i++)
		{
			u32 sp = layer->GetSplitPoint(i);

			// Emit render pass for this layer's cmds before the split point
			renderPassArray.Insert({
				.renderTarget = nullptr,
				.layerIndex = (SIZE_T)layerIndex,
				.drawCmdStartIndex = cmdBase + prevSplit,
				.drawCmdCount = sp - prevSplit   // excludes the placeholder at sp
			});

			prevSplit = sp + 1; // +1 skips the placeholder cmd

			// Child's index in the split arrays = current count before it inserts
			DoLayer(layer->GetChild(i), (int)vertexSplits.GetCount());
		}

		// Final segment after the last split (or the whole thing if no splits)
		renderPassArray.Insert({
			.renderTarget = nullptr,
			.layerIndex = (SIZE_T)layerIndex,
			.drawCmdStartIndex = cmdBase + prevSplit,
			.drawCmdCount = drawCmdSplits[layerIndex].count - prevSplit
		});
	}
}
