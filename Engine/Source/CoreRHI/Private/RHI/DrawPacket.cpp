#include "CoreRHI.h"

namespace CE::RHI
{
	/*
	 * Copyright (c) Contributors to the Open 3D Engine Project.
	 * Used under Apache 2.0 license. https://github.com/o3de/o3de/blob/development/LICENSE.TXT
	 */

	void DrawPacket::operator delete(void* p, size_t size)
	{
		reinterpret_cast<const DrawPacket*>(p)->allocator->deallocate(p);
	}

	DrawItemProperties DrawPacket::GetDrawItemProperties(SIZE_T index)
	{
		CE_ASSERT(index < GetDrawItemCount(), "GetDrawItemProperties(): index {} out of bounds {}", index, GetDrawItemCount());

		return DrawItemProperties(&drawItems[index], 0.0f, drawFilterMasks[index]);
	}

} // namespace CE::RHI
