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

		vertexWritePtr = nullptr;
		indexWritePtr = nullptr;
		vertexCurrentIdx = 0;
	}

	void FUIDrawList::Finalize()
	{
		// indexCount is maintained incrementally by prim functions (e.g. PrimRect).
		// Nothing to do here.
	}

	u32 FUIDrawList::AddDrawItem(const FUIDrawItem& item)
	{
		u32 index = (u32)drawItemArray.GetCount();
		drawItemArray.Insert(item);
		return index;
	}

	void FUIDrawList::PushTransform(const FAffineTransform& transform)
	{
		transformStack.Insert(GetCurrentTransform() * transform);
	}

	void FUIDrawList::PopTransform()
	{
		transformStack.RemoveLast();
	}

	FAffineTransform FUIDrawList::GetCurrentTransform()
	{
		return transformStack.IsEmpty() ? FAffineTransform::Identity() : transformStack.Last();
	}

	FUIDrawCmd& FUIDrawList::ForceNewDrawCmd()
	{
		FUIDrawCmd cmd{};
		cmd.indexOffset = (u32)indexArray.GetCount();
		cmd.vertexOffset = 0;
		cmd.blendMode = drawCmdArray.IsEmpty() ? FUIBlendMode::Normal : drawCmdArray.Last().blendMode;
		cmd.scissorRect = drawCmdArray.IsEmpty() ? Rect() : drawCmdArray.Last().scissorRect;
		cmd.customShaderId = drawCmdArray.IsEmpty() ? 0 : drawCmdArray.Last().customShaderId;
		drawCmdArray.Insert(cmd);

		return drawCmdArray.Last();
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

			if (last.indexCount == 0)
			{
				// Last command had nothing written — reuse the slot with the new state
				last.blendMode      = blendMode;
				last.scissorRect    = scissorRect;
				last.customShaderId = customShaderId;
				return last;
			}
			// indexCount is already current (maintained by prim functions), no sealing needed.
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

	void FUIDrawList::PrimReserve(int vertexCount, int indexCount)
	{
		ZoneScoped;

		SIZE_T curVertexCount = vertexArray.GetCount();

		vertexArray.InsertRange(vertexCount);
		vertexWritePtr = vertexArray.GetData() + curVertexCount;

		SIZE_T curIndexCount = indexArray.GetCount();

		indexArray.InsertRange(indexCount);
		indexWritePtr = indexArray.GetData() + curIndexCount;
	}

	void FUIDrawList::PrimRect(const Rect& quad, u32 color, Vec2* uvs, u32 drawItemIndex)
	{
		Vec2 topLeft = quad.min;
		Vec2 topRight = Vec2(quad.max.x, quad.min.y);
		Vec2 bottomRight = Vec2(quad.max.x, quad.max.y);
		Vec2 bottomLeft = Vec2(quad.min.x, quad.max.y);

		Vec2 topLeftUV = uvs != nullptr ? uvs[0] : Vec2(0, 0);
		Vec2 topRightUV = uvs != nullptr ? uvs[1] : Vec2(1, 0);
		Vec2 bottomRightUV = uvs != nullptr ? uvs[2] : Vec2(1, 1);
		Vec2 bottomLeftUV = uvs != nullptr ? uvs[3] : Vec2(0, 1);

		FUIIndex idx = vertexCurrentIdx;
		indexWritePtr[0] = idx; indexWritePtr[1] = (idx + 1); indexWritePtr[2] = (idx + 2);
		indexWritePtr[3] = idx; indexWritePtr[4] = (idx + 2); indexWritePtr[5] = (idx + 3);

		vertexWritePtr[0].pos = topLeft; vertexWritePtr[0].color = color; vertexWritePtr[0].uv = topLeftUV;
		vertexWritePtr[0].drawItemIndex = drawItemIndex;

		vertexWritePtr[1].pos = topRight; vertexWritePtr[1].color = color; vertexWritePtr[1].uv = topRightUV;
		vertexWritePtr[1].drawItemIndex = drawItemIndex;

		vertexWritePtr[2].pos = bottomRight; vertexWritePtr[2].color = color; vertexWritePtr[2].uv = bottomRightUV;
		vertexWritePtr[2].drawItemIndex = drawItemIndex;

		vertexWritePtr[3].pos = bottomLeft; vertexWritePtr[3].color = color; vertexWritePtr[3].uv = bottomLeftUV;
		vertexWritePtr[3].drawItemIndex = drawItemIndex;

		vertexWritePtr += 4;
		vertexCurrentIdx += 4;
		indexWritePtr += 6;

		drawCmdArray.Last().indexCount += 6;
	}
}
