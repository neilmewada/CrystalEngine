#include "FusionCore.h"

// Credit: Dear Imgui
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = ImRsqrt(d2); VX *= inv_len; VY *= inv_len; } } (void)0
#define IM_FIXNORMAL2F_MAX_INVLEN2          100.0f // 500.0f (see #4053, #3366)
#define IM_FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 > 0.000001f) { float inv_len2 = 1.0f / d2; if (inv_len2 > IM_FIXNORMAL2F_MAX_INVLEN2) inv_len2 = IM_FIXNORMAL2F_MAX_INVLEN2; VX *= inv_len2; VY *= inv_len2; } } (void)0
static constexpr float ImRsqrt(float x) { return 1.0f / sqrtf(x); }

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

	FUIDrawCmd& FUIDrawList::NewDrawCmd()
	{
		if (!drawCmdArray.IsEmpty() && drawCmdArray.Last().indexCount == 0)
			return drawCmdArray.Last(); // already a clean boundary, reuse it

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

	void FUIDrawList::PrimRect(const Rect& rect, u32 color, Vec2* uvs, u32 drawItemIndex)
	{
		const Vec2 topLeft     = Vec2(rect.min.x, rect.min.y);
		const Vec2 topRight    = Vec2(rect.max.x, rect.min.y);
		const Vec2 bottomRight = Vec2(rect.max.x, rect.max.y);
		const Vec2 bottomLeft  = Vec2(rect.min.x, rect.max.y);

		const Vec2 topLeftUV = uvs != nullptr ? uvs[0] : Vec2(0, 0);
		const Vec2 topRightUV = uvs != nullptr ? uvs[1] : Vec2(1, 0);
		const Vec2 bottomRightUV = uvs != nullptr ? uvs[2] : Vec2(1, 1);
		const Vec2 bottomLeftUV = uvs != nullptr ? uvs[3] : Vec2(0, 1);

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

	void FUIDrawList::AddPolyLine(const Vec2* points, int numPoints, u32 color, f32 thickness, bool closed, bool antiAliased)
	{
		ZoneScoped;

		if (points == nullptr || numPoints <= 0)
			return;

        constexpr Vec2 uv = Vec2();

		const int count = closed ? numPoints : numPoints - 1; // The number of line segments we need to draw
		const bool thickLine = (thickness > fringeScale);

        if (antiAliased)
        {
            // [PATH 1]
            // Anti-aliased stroke
            const float AA_SIZE = fringeScale;
            const u32 transparentColor = color & ~ColorAlphaMask;

            // Thicknesses <1.0 should behave like thickness 1.0
            thickness = Math::Max(thickness, 1.0f);
            const int integerThickness = (int)thickness;
            const float fractionalThickness = thickness - integerThickness;

            const int indexCount = (thickLine ? count * 18 : count * 12);
            const int vertexCount = (thickLine ? numPoints * 4 : numPoints * 3);
            PrimReserve(vertexCount, indexCount);

            // Temporary buffer
            // The first <numPoints> items are normals at each line point, then after that there are either 2 or 4 temp points for each line point
            temporaryPoints.RemoveAll();
            temporaryPoints.InsertRange(numPoints * (thickLine ? 5 : 3));
            Vec2* tempNormals = temporaryPoints.GetData();
            Vec2* tempPoints = tempNormals + numPoints;

            // Calculate normals (tangents) for each line segment
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == numPoints ? 0 : i1 + 1;
                float dx = points[i2].x - points[i1].x;
                float dy = points[i2].y - points[i1].y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                tempNormals[i1].x = dy;
                tempNormals[i1].y = -dx;
            }

            if (!closed)
                tempNormals[numPoints - 1] = tempNormals[numPoints - 2];

            if (!thickLine)
            {
                const float half_draw_size = AA_SIZE;

                // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
                if (!closed)
                {
                    tempPoints[0] = points[0] + tempNormals[0] * half_draw_size;
                    tempPoints[1] = points[0] - tempNormals[0] * half_draw_size;
                    tempPoints[(numPoints - 1) * 2 + 0] = points[numPoints - 1] + tempNormals[numPoints - 1] * half_draw_size;
                    tempPoints[(numPoints - 1) * 2 + 1] = points[numPoints - 1] - tempNormals[numPoints - 1] * half_draw_size;
                }

                // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
                // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
                // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
                unsigned int idx1 = vertexCurrentIdx; // Vertex index for start of line segment
                for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
                {
                    const int i2 = (i1 + 1) == numPoints ? 0 : i1 + 1; // i2 is the second point of the line segment
                    const unsigned int idx2 = ((i1 + 1) == numPoints) ? vertexCurrentIdx : (idx1 + 3); // Vertex index for end of segment

                    // Average normals
                    float dm_x = (tempNormals[i1].x + tempNormals[i2].x) * 0.5f;
                    float dm_y = (tempNormals[i1].y + tempNormals[i2].y) * 0.5f;
                    IM_FIXNORMAL2F(dm_x, dm_y);
                    dm_x *= half_draw_size; // dm_x, dm_y are offset to the outer edge of the AA area
                    dm_y *= half_draw_size;

                    // Add temporary vertexes for the outer edges
                    Vec2* out_vtx = &tempPoints[i2 * 2];
                    out_vtx[0].x = points[i2].x + dm_x;
                    out_vtx[0].y = points[i2].y + dm_y;
                    out_vtx[1].x = points[i2].x - dm_x;
                    out_vtx[1].y = points[i2].y - dm_y;

                    {
                        // Add indexes for four triangles
                        indexWritePtr[0] = (FUIIndex)(idx2 + 0); indexWritePtr[1] = (FUIIndex)(idx1 + 0); indexWritePtr[2] = (FUIIndex)(idx1 + 2);  // Right tri 1
                        indexWritePtr[3] = (FUIIndex)(idx1 + 2); indexWritePtr[4] = (FUIIndex)(idx2 + 2); indexWritePtr[5] = (FUIIndex)(idx2 + 0);  // Right tri 2
                        indexWritePtr[6] = (FUIIndex)(idx2 + 1); indexWritePtr[7] = (FUIIndex)(idx1 + 1); indexWritePtr[8] = (FUIIndex)(idx1 + 0);  // Left tri 1
                        indexWritePtr[9] = (FUIIndex)(idx1 + 0); indexWritePtr[10] = (FUIIndex)(idx2 + 0); indexWritePtr[11] = (FUIIndex)(idx2 + 1);// Left tri 2
                        indexWritePtr += 12;
                    }

                    idx1 = idx2;
                }

                {
                    // If we're not using a texture, we need the center vertex as well
                    for (int i = 0; i < numPoints; i++)
                    {
                        vertexWritePtr[0].pos = points[i];             vertexWritePtr[0].uv = uv; vertexWritePtr[0].color = color;            // Center of line
                        vertexWritePtr[1].pos = tempPoints[i * 2 + 0]; vertexWritePtr[1].uv = uv; vertexWritePtr[1].color = transparentColor; // Left-side outer edge
                        vertexWritePtr[2].pos = tempPoints[i * 2 + 1]; vertexWritePtr[2].uv = uv; vertexWritePtr[2].color = transparentColor; // Right-side outer edge
                        vertexWritePtr += 3;
                    }
                }
            }
            else
            {
                // [PATH 2] Non texture-based lines (thick): we need to draw the solid line core and thus require four vertices per point
                const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;

                // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
                if (!closed)
                {
                    const int points_last = numPoints - 1;
                    tempPoints[0] = points[0] + tempNormals[0] * (half_inner_thickness + AA_SIZE);
                    tempPoints[1] = points[0] + tempNormals[0] * (half_inner_thickness);
                    tempPoints[2] = points[0] - tempNormals[0] * (half_inner_thickness);
                    tempPoints[3] = points[0] - tempNormals[0] * (half_inner_thickness + AA_SIZE);
                    tempPoints[points_last * 4 + 0] = points[points_last] + tempNormals[points_last] * (half_inner_thickness + AA_SIZE);
                    tempPoints[points_last * 4 + 1] = points[points_last] + tempNormals[points_last] * (half_inner_thickness);
                    tempPoints[points_last * 4 + 2] = points[points_last] - tempNormals[points_last] * (half_inner_thickness);
                    tempPoints[points_last * 4 + 3] = points[points_last] - tempNormals[points_last] * (half_inner_thickness + AA_SIZE);
                }

                // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
                // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
                // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
                unsigned int idx1 = vertexCurrentIdx; // Vertex index for start of line segment
                for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
                {
                    const int i2 = (i1 + 1) == numPoints ? 0 : (i1 + 1); // i2 is the second point of the line segment
                    const unsigned int idx2 = (i1 + 1) == numPoints ? vertexCurrentIdx : (idx1 + 4); // Vertex index for end of segment

                    // Average normals
                    float dm_x = (tempNormals[i1].x + tempNormals[i2].x) * 0.5f;
                    float dm_y = (tempNormals[i1].y + tempNormals[i2].y) * 0.5f;
                    IM_FIXNORMAL2F(dm_x, dm_y);
                    float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                    float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                    float dm_in_x = dm_x * half_inner_thickness;
                    float dm_in_y = dm_y * half_inner_thickness;

                    // Add temporary vertices
                    Vec2* out_vtx = &tempPoints[i2 * 4];
                    out_vtx[0].x = points[i2].x + dm_out_x;
                    out_vtx[0].y = points[i2].y + dm_out_y;
                    out_vtx[1].x = points[i2].x + dm_in_x;
                    out_vtx[1].y = points[i2].y + dm_in_y;
                    out_vtx[2].x = points[i2].x - dm_in_x;
                    out_vtx[2].y = points[i2].y - dm_in_y;
                    out_vtx[3].x = points[i2].x - dm_out_x;
                    out_vtx[3].y = points[i2].y - dm_out_y;

                    // Add indexes
                    indexWritePtr[0] = (FUIIndex)(idx2 + 1); indexWritePtr[1] = (FUIIndex)(idx1 + 1); indexWritePtr[2] = (FUIIndex)(idx1 + 2);
                    indexWritePtr[3] = (FUIIndex)(idx1 + 2); indexWritePtr[4] = (FUIIndex)(idx2 + 2); indexWritePtr[5] = (FUIIndex)(idx2 + 1);
                    indexWritePtr[6] = (FUIIndex)(idx2 + 1); indexWritePtr[7] = (FUIIndex)(idx1 + 1); indexWritePtr[8] = (FUIIndex)(idx1 + 0);
                    indexWritePtr[9] = (FUIIndex)(idx1 + 0); indexWritePtr[10] = (FUIIndex)(idx2 + 0); indexWritePtr[11] = (FUIIndex)(idx2 + 1);
                    indexWritePtr[12] = (FUIIndex)(idx2 + 2); indexWritePtr[13] = (FUIIndex)(idx1 + 2); indexWritePtr[14] = (FUIIndex)(idx1 + 3);
                    indexWritePtr[15] = (FUIIndex)(idx1 + 3); indexWritePtr[16] = (FUIIndex)(idx2 + 3); indexWritePtr[17] = (FUIIndex)(idx2 + 2);
                    indexWritePtr += 18;

                    idx1 = idx2;
                }

                // Add vertices
                for (int i = 0; i < numPoints; i++)
                {
                    vertexWritePtr[0].pos = tempPoints[i * 4 + 0]; vertexWritePtr[0].uv = uv; vertexWritePtr[0].color = transparentColor;
                    vertexWritePtr[1].pos = tempPoints[i * 4 + 1]; vertexWritePtr[1].uv = uv; vertexWritePtr[1].color = color;
                    vertexWritePtr[2].pos = tempPoints[i * 4 + 2]; vertexWritePtr[2].uv = uv; vertexWritePtr[2].color = color;
                    vertexWritePtr[3].pos = tempPoints[i * 4 + 3]; vertexWritePtr[3].uv = uv; vertexWritePtr[3].color = transparentColor;
                    vertexWritePtr += 4;
                }
            }

            vertexCurrentIdx += vertexCount;
            drawCmdArray.Last().indexCount += indexCount;
        }
        else
        {
            // [PATH 4] Non texture-based, Non anti-aliased lines
            const int indexCount = count * 6;
            const int vertexCount = count * 4;    // FIXME-OPT: Not sharing edges
            PrimReserve(vertexCount, indexCount);

            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == numPoints ? 0 : i1 + 1;
                const Vec2& p1 = points[i1];
                const Vec2& p2 = points[i2];

                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                dx *= (thickness * 0.5f);
                dy *= (thickness * 0.5f);

                vertexWritePtr[0].pos.x = p1.x + dy; vertexWritePtr[0].pos.y = p1.y - dx; vertexWritePtr[0].uv = uv; vertexWritePtr[0].color = color;
                vertexWritePtr[1].pos.x = p2.x + dy; vertexWritePtr[1].pos.y = p2.y - dx; vertexWritePtr[1].uv = uv; vertexWritePtr[1].color = color;
                vertexWritePtr[2].pos.x = p2.x - dy; vertexWritePtr[2].pos.y = p2.y + dx; vertexWritePtr[2].uv = uv; vertexWritePtr[2].color = color;
                vertexWritePtr[3].pos.x = p1.x - dy; vertexWritePtr[3].pos.y = p1.y + dx; vertexWritePtr[3].uv = uv; vertexWritePtr[3].color = color;
                vertexWritePtr += 4;

                indexWritePtr[0] = (FUIIndex)(vertexCurrentIdx); indexWritePtr[1] = (FUIIndex)(vertexCurrentIdx + 1); indexWritePtr[2] = (FUIIndex)(vertexCurrentIdx + 2);
                indexWritePtr[3] = (FUIIndex)(vertexCurrentIdx); indexWritePtr[4] = (FUIIndex)(vertexCurrentIdx + 2); indexWritePtr[5] = (FUIIndex)(vertexCurrentIdx + 3);
                indexWritePtr += 6;
                vertexCurrentIdx += 4;
            }

            vertexCurrentIdx += vertexCount;
            drawCmdArray.Last().indexCount += indexCount;
        }
	}
}
