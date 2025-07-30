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

	static HashMap<TypeId, u8> FieldTypeBytes{
		{ 0, 0 },
		{ TYPEID(u8), 1 },
		{ TYPEID(u16), 2 },
		{ TYPEID(u32), 3 },
		{ TYPEID(u64), 4 },
		{ TYPEID(s8), 5 },
		{ TYPEID(s16), 6 },
		{ TYPEID(s32), 7 },
		{ TYPEID(s64), 8 },
		{ TYPEID(f32), 9 },
		{ TYPEID(f64), 10 },
		{ TYPEID(b8), 11 },
		{ TYPEID(String), 12 }, { TYPEID(Name), 12 }, { TYPEID(IO::Path), 12 },
		{ TYPEID(Vec2), 13 },
		{ TYPEID(Vec3), 14 },
		{ TYPEID(Vec4), 15 },
		{ TYPEID(Color), 15 },
		{ TYPEID(Vec2i), 16 },
		{ TYPEID(Vec3i), 17 },
		{ TYPEID(Vec4i), 18 },
		{ TYPEID(BinaryBlob), 0x16 },
		{ TYPEID(Object), 0x17 },
		{ TYPEID(ScriptDelegate<>), 0x18 },
		{ TYPEID(ScriptEvent<>), 0x19 },
		{ TYPEID(Uuid), 0x1B },
		{ TYPEID(ObjectMap), 0x1C }
	};
	
}
