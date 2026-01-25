#include "CoreRHI.h"

namespace CE::RHI
{
	/*
	 * Copyright (c) Contributors to the Open 3D Engine Project.
	 * Used under Apache 2.0 license. https://github.com/o3de/o3de/blob/development/LICENSE.TXT
	 */

    void DrawListContext::Init(const DrawListMask& drawListMask)
    {
		// Set mask
		if (drawListMask.Any())
		{
			this->drawListMask = drawListMask;
		}
    }

    void DrawListContext::Shutdown()
    {
		// Clear all lists
		
		threadDrawListsByTag.ForEach([](DrawListsByTag& list)
			{
				for (int i = 0; i < list.GetSize(); i++)
				{
					list[i].Clear();
				}
			});

		for (auto& mergedList : mergedDrawListsByTag)
		{
			mergedList.Clear();
		}

		// Reset mask
		drawListMask.Reset();
    }

    DrawList& DrawListContext::GetDrawListForTag(DrawListTag tag)
	{
		static DrawList empty{};
		if (tag >= mergedDrawListsByTag.GetSize())
			return empty;

		return mergedDrawListsByTag[tag];
	}

	void DrawListContext::AddDrawPacket(DrawPacket* drawPacket, f32 depth)
	{
		if (!drawPacket)
			return;
		
		DrawListsByTag& threadDrawListsByTag = this->threadDrawListsByTag.GetStorage();

		for (int i = 0; i < drawPacket->GetDrawItemCount(); i++)
		{
			const DrawListTag drawListTag = drawPacket->GetDrawListTag(i);
			
			if (this->drawListMask.Test(drawListTag.Get()))
			{
				DrawItemProperties drawItemProperties = drawPacket->GetDrawItemProperties(i);
				drawItemProperties.depth = depth;
				
				auto& drawList = threadDrawListsByTag[drawListTag.Get()];
				drawList.listTag = drawListTag;
				drawList.AddDrawItem(drawItemProperties);
				
			}
		}
	}

	void DrawListContext::AddDrawItem(DrawItemProperties drawItem, DrawListTag drawListTag)
	{
		ZoneScoped;

		DrawListsByTag& threadDrawListsByTag = this->threadDrawListsByTag.GetStorage();
		threadDrawListsByTag[drawListTag].AddDrawItem(drawItem);
		threadDrawListsByTag[drawListTag].listTag = drawListTag;
	}

	void DrawListContext::Finalize()
	{
		ZoneScoped;

		for (int i = 0; i < mergedDrawListsByTag.GetSize(); i++)
		{
			mergedDrawListsByTag[i].Clear();
			mergedDrawListsByTag[i].listTag = (DrawListTag)i;
		}

		threadDrawListsByTag.ForEach([&](DrawListsByTag& drawLists)
			{
				for (int i = 0; i < drawLists.GetSize(); i++)
				{
					mergedDrawListsByTag[i].Merge(drawLists[i]);
					drawLists[i].Clear();
				}
			});
	}

	void DrawListContext::ClearAll()
	{
		for (int i = 0; i < mergedDrawListsByTag.GetSize(); i++)
		{
			mergedDrawListsByTag[i].Clear();
			mergedDrawListsByTag[i].listTag = (DrawListTag)i;
		}

		threadDrawListsByTag.ForEach([&](DrawListsByTag& drawLists)
			{
				for (int i = 0; i < drawLists.GetSize(); i++)
				{
					drawLists[i].Clear();
				}
			});
	}

} // namespace CE::RHI
