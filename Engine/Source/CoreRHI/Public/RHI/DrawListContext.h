#pragma once

namespace CE::RHI
{
	/*
	 * Copyright (c) Contributors to the Open 3D Engine Project.
	 * Used under Apache 2.0 license. https://github.com/o3de/o3de/blob/development/LICENSE.TXT
	 */

	class CORERHI_API DrawListContext final
	{
	public:

		void Init(const DrawListMask& drawListMask);
		void Shutdown();

		DrawList& GetDrawListForTag(DrawListTag tag);

		/// @brief Thread safe function to add a draw bundle to the DrawList.
		/// @param drawPacket to add.
		/// @param depth of object from view
		void AddDrawPacket(DrawPacket* drawPacket, f32 depth = 0.0f);

		void AddDrawItem(DrawItemProperties drawItem, DrawListTag drawListTag);

		void Finalize();

		void ClearAll();

		/// @brief Sorts the merged draw list for the given tag by depth, descending (back-to-front).
		void Sort(DrawListTag tag);

	private:
		
		ThreadLocalContext<DrawListsByTag> threadDrawListsByTag{};
		DrawListsByTag mergedDrawListsByTag{};
		DrawListMask drawListMask{};
	};
    
} // namespace CE::RHI
