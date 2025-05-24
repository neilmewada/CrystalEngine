#pragma once

#define BUNDLE_MAGIC_NUMBER CE::FromBigEndian((u64)0x0042554e444c4500) // .BUNDLE.

#define BUNDLE_VERSION_MAJOR (u32)3
#define BUNDLE_VERSION_MINOR (u32)1
#define BUNDLE_VERSION_PATCH (u32)0

#define BUNDLE_OBJECT_MAGIC_NUMBER CE::FromBigEndian((u64)0x004f424a45435400) // .OBJECT.

namespace CE
{

	struct BundleVersions
	{
		static constexpr u32 SourceAssetPathMajor = 3;
		static constexpr u32 SourceAssetPathMinor = 1;
	};
	
}
