#include "FusionCore.h"

namespace CE
{
	void FUIDrawList::Clear()
	{
		ZoneScoped;

		vertexArray.RemoveAll();
		indexArray.RemoveAll();
		drawItemArray.RemoveAll();
		clipRectArray.RemoveAll();
		gradientStopArray.RemoveAll();
		drawCmdArray.RemoveAll();
	}

	void FUIDrawList::Finalize()
	{
		if (!drawCmdArray.IsEmpty())
		{
			auto& last = drawCmdArray.Last();
			last.indexCount = (u32)indexArray.GetCount() - last.indexOffset;
		}
	}

	u32 FUIDrawList::AddDrawItem(const FUIDrawItem& item)
	{
		u32 index = (u32)drawItemArray.GetCount();
		drawItemArray.Insert(item);
		return index;
	}

	void FUIDrawList::PrimWriteVtx(Vec2 pos, Vec2 uv, u32 color, u32 drawItemIndex)
	{
		FUIVertex vtx{};
		vtx.pos = pos;
		vtx.uv = uv;
		vtx.color = color;
		vtx.drawItemIndex = drawItemIndex;
		vertexArray.Insert(vtx);
	}

	void FUIDrawList::PrimWriteIdx(FUIIndex idx)
	{
		indexArray.Insert(idx);
	}

	FUIDrawCmd& FUIDrawList::AcquireDrawCmd(FUIBlendMode blendMode, Rect scissorRect, u32 customShaderId)
	{
		if (!drawCmdArray.IsEmpty())
		{
			auto& last = drawCmdArray.Last();
			if (last.blendMode == blendMode &&
				last.scissorRect == scissorRect &&
				last.customShaderId == customShaderId)
			{
				return last;
			}

			u32 currentIndexCount = (u32)indexArray.GetCount() - last.indexOffset;
			if (currentIndexCount == 0)
			{
				// Last command had nothing written — reuse the slot with the new state
				last.blendMode      = blendMode;
				last.scissorRect    = scissorRect;
				last.customShaderId = customShaderId;
				return last;
			}

			// Seal the previous command before opening a new one
			last.indexCount = currentIndexCount;
		}

		FUIDrawCmd cmd{};
		cmd.indexOffset    = (u32)indexArray.GetCount();
		cmd.vertexOffset   = 0;
		cmd.blendMode      = blendMode;
		cmd.scissorRect    = scissorRect;
		cmd.customShaderId = customShaderId;
		drawCmdArray.Insert(cmd);

		return drawCmdArray.Last();
	}

}
