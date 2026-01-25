#include "CoreMinimal.h"

namespace CE
{

	Variant::~Variant()
	{
		Free();
	}

#define IF_HASH(Type, Value) \
if (valueTypeId == TYPEID(Type))\
{\
	return CE::GetHash(Value);\
}

	SIZE_T Variant::GetHash() const
	{
		if (!HasValue())
			return 0;

		IF_HASH(bool, BoolValue)
		IF_HASH(f32, Float32Value)
		IF_HASH(f64, Float64Value)
		IF_HASH(s8, Int8Value)
		IF_HASH(s16, Int16Value)
		IF_HASH(s32, Int32Value)
		IF_HASH(s64, Int64Value)
		IF_HASH(u8, Uint8Value)
		IF_HASH(u16, Uint16Value)
		IF_HASH(u32, Uint32Value)
		IF_HASH(u64, Uint64Value)
		IF_HASH(Vec2, Vec2Value)
		IF_HASH(Vec3, Vec3Value)
		IF_HASH(Vec4, Vec4Value)
		IF_HASH(Vec2i, Vec2iValue)
		IF_HASH(Vec3i, Vec3iValue)
		IF_HASH(Vec4i, Vec4iValue)
		IF_HASH(Color, ColorValue)
		IF_HASH(String, StringValue)
		IF_HASH(Name, nameValue)
		IF_HASH(IO::Path, pathValue)
		IF_HASH(Uuid, uuidValue)

		if (valueTypeId == CE::GetTypeId<Matrix4x4>())
		{
			return CalculateHash(MatrixValue.rows, sizeof(MatrixValue.rows));
		}

		if (IsPointer())
		{
			return (SIZE_T)PtrValue;
		}

		if (IsArray())
		{
			return CalculateHash(rawArrayValue.GetData(), rawArrayValue.GetSize());
		}

		if (IsStruct())
		{
			return (SIZE_T)StructValue;
		}

		return 0;
	}

	void Variant::CopyFrom(const Variant& copy)
	{
		valueTypeId = copy.valueTypeId;
		arrayElementTypeId = copy.arrayElementTypeId;
		isPointer = copy.isPointer;
		isArray = copy.isArray;
		isStruct = copy.isStruct;
		if (isStruct)
		{
			structType = copy.structType;
		}

		if (copy.IsOfType<String>())
		{
			StringValue = copy.StringValue;
		}
		else if (copy.isArray && copy.arrayElementTypeId == TYPEID(String))
		{
			stringArrayValue = copy.stringArrayValue;
		}
		else if (copy.isArray && copy.arrayElementTypeId == TYPEID(Name))
		{
			nameArrayValue = copy.nameArrayValue;
		}
		else if (copy.isArray && copy.arrayElementTypeId == TYPEID(IO::Path))
		{
			pathArrayValue = copy.pathArrayValue;
		}
		else if (copy.isArray)
		{
			rawArrayValue = copy.rawArrayValue;
		}
		else if (copy.IsOfType<IO::Path>())
		{
			pathValue = copy.pathValue;
		}
		else if (copy.isStruct && copy.StructValue != nullptr)
		{
			if (structType != nullptr)
			{
				StructValue = malloc(structType->GetSize());
				structType->CopyConstructor(copy.StructValue, StructValue);
			}
			else
			{
				memcpy(this, &copy, sizeof(Variant));
			}
		}
		else
		{
			memcpy(this, &copy, sizeof(Variant));
		}
	}

	void Variant::Free()
	{
		if (IsOfType<String>())
		{
			StringValue.~String();
		}
		else if (isArray && arrayElementTypeId == TYPEID(String))
		{
			stringArrayValue.~Array();
		}
		else if (isArray && arrayElementTypeId == TYPEID(Name))
		{
			nameArrayValue.~Array();
		}
		else if (isArray && arrayElementTypeId == TYPEID(IO::Path))
		{
			pathArrayValue.~Array();
		}
		else if (isArray)
		{
			rawArrayValue.~Array();
		}
		else if (IsOfType<IO::Path>())
		{
			pathValue.~Path();
		}
		else if (isStruct && StructValue != nullptr)
		{
			if (structType != nullptr)
			{
				structType->CallDestructor(StructValue);
			}
			free(StructValue);
			StructValue = nullptr;
			structType = nullptr;
			isStruct = false;
		}

		memset(this, 0, sizeof(Variant));
	}

	bool Variant::IsObject() const
	{
		TypeInfo* type = CE::GetTypeInfo(valueTypeId);
		return type ? type->IsObject() : false;
	}

	bool Variant::CanCastObject(TypeId castTo) const
	{
		TypeInfo* valueType = CE::GetTypeInfo(valueTypeId);
		TypeInfo* castToType = CE::GetTypeInfo(castTo);

		if (valueType == nullptr || castToType == nullptr)
			return false;

		return valueType->IsObject() && castToType->IsObject() && valueType->IsAssignableTo(castTo);
	}


} // namespace CE
