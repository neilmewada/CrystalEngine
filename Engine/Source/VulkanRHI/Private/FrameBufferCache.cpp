#include "VulkanRHIPrivate.h"

namespace CE::Vulkan
{
	SIZE_T FrameBufferCache::Key::GetHash() const
	{
		SIZE_T hash = renderPassHash;
		for (SIZE_T attachmentHandle : attachmentHandles)
		{
			CombineHash(hash, attachmentHandle);
		}
		CombineHash(hash, width);
		CombineHash(hash, height);
		CombineHash(hash, layers);
		return hash;
	}

} // namespace CE::Vulkan
