#pragma once

#include "Misc/CoreDefines.h"
#include "Misc/CoreMacros.h"
#include "Containers/String.h"
#include "Containers/Array.h"
#include "Containers/HashMap.h"
#include "CoreTypes.h"

#include "RTTIDefines.h"

#include "Math/Math.h"
#include "Math/Vector.h"

namespace CE
{

	// Forward Declarations
	class TypeInfo;
	class ClassType;
	class StructType;
	class EnumType;
	class Object;

	template<typename T>
	class Ref;

	template<typename T>
	class WeakRef;

	// **********************************************************
	// Variant

	class CORE_API VariantCastException : public std::runtime_error
	{
	public:
		VariantCastException(String message) : std::runtime_error(message.ToStdString())
		{}
	};

	class CORE_API Variant final
	{
	public:
		Variant() : valueTypeId(0)
		{
			memset(this, 0, sizeof(Variant));
		}

		Variant(const Variant& copy)
		{
			Free();
			CopyFrom(copy);
		}

		CE::Variant& operator=(const CE::Variant& copy)
		{
			Free();
			CopyFrom(copy);
			return *this;
		}

		~Variant();

		// Internal use only!
		void CopyFrom(const Variant& copy);

		// Internal use only! Frees up memory used by subvalues
		void Free();

		Variant(f32 value) { SetInternalValue(value); }
		Variant(f64 value) { SetInternalValue(value); }

		Variant(u8  value) { SetInternalValue(value); }
		Variant(u16 value) { SetInternalValue(value); }
		Variant(u32 value) { SetInternalValue(value); }
		Variant(u64 value) { SetInternalValue(value); }

		Variant(s8  value) { SetInternalValue(value); }
		Variant(s16 value) { SetInternalValue(value); }
		Variant(s32 value) { SetInternalValue(value); }
		Variant(s64 value) { SetInternalValue(value); }
		Variant(const Matrix4x4& value) { SetInternalValue(value); }

		Variant(bool value) { SetInternalValue(value); }
        
        Variant(Vec2  value) { SetInternalValue(value); }
        Variant(Vec3  value) { SetInternalValue(value); }
        Variant(Vec4  value) { SetInternalValue(value); }
		Variant(Quat  value) { SetInternalValue(value); }
		Variant(Color  value) { SetInternalValue(value); }

		Variant(Vec2i value) { SetInternalValue(value); }
		Variant(Vec3i value) { SetInternalValue(value); }
		Variant(Vec4i value) { SetInternalValue(value); }

        Variant(String value) { Clear(); StringValue = value; valueTypeId = TYPEID(String); }

		Variant(IO::Path value) 
		{
			SetInternalValue(value);
		}
		
		Variant(Name value) 
		{
			SetInternalValue(value);
		}

		template<typename TStruct> requires (not std::is_same_v<TStruct, Variant>) and TStruct::IsStruct and TIsCopyable<TStruct>::Value
		Variant(const TStruct& value)
		{
			SetStructValue(value);
		}

		template<typename ElementType>
		Variant(const CE::Array<ElementType>& value)
		{
			memset(this, 0, sizeof(Variant));
			rawArrayValue = Array<u8>(value.GetSize() * sizeof(ElementType));

			for (int i = 0; i < value.GetSize(); i++)
			{
				ElementType* dest = (ElementType*)(&*rawArrayValue.Begin() + i * sizeof(ElementType));
				*dest = value[i];
			}

			rawArrayValue.ElementTypeId = TYPEID(ElementType);
			valueTypeId = TYPEID(CE::Array<ElementType>);
			arrayElementTypeId = TYPEID(ElementType);
			isArray = true;
		}

		template<>
		Variant(const CE::Array<String>& value)
		{
			memset(this, 0, sizeof(Variant));

			stringArrayValue = value;
			valueTypeId = TYPEID(CE::Array<String>);
			arrayElementTypeId = TYPEID(String);
			isArray = true;
		}

		template<>
		Variant(const CE::Array<Name>& value)
		{
			memset(this, 0, sizeof(Variant));

			nameArrayValue = value;
			valueTypeId = TYPEID(CE::Array<Name>);
			arrayElementTypeId = TYPEID(Name);
			isArray = true;
		}

		template<>
		Variant(const CE::Array<IO::Path>& value)
		{
			memset(this, 0, sizeof(Variant));

			pathArrayValue = value;
			valueTypeId = TYPEID(CE::Array<IO::Path>);
			arrayElementTypeId = TYPEID(IO::Path);
			isArray = true;
		}

		template<typename TObject> requires TIsBaseClassOf<Object, TObject>::Value
		Variant(Ref<TObject> value) : valueTypeId(TYPEID(TObject)), PtrValue(value.Get())
		{
			isPointer = true;
		}

		template<typename RefType> requires TIsBaseClassOf<Object, std::remove_cvref_t<RefType>>::Value
		Variant(RefType& ref) : Variant((RefType*)&ref)
		{
			
		}

		template<typename PtrType>
		Variant(PtrType* ptr) : valueTypeId(TYPEID(PtrType)), PtrValue(ptr)
		{
			isPointer = true;
		}

		template<typename PtrType>
		Variant(const PtrType* ptr) : valueTypeId(TYPEID(PtrType)), PtrValue((void*)ptr)
		{
			isPointer = true;
		}

		Variant(std::nullptr_t nullValue) : valueTypeId(TYPEID(void)), PtrValue(nullptr)
		{
			isPointer = true;
		}

		template<>
		Variant(char* value)
		{
			Clear();
			StringValue = value;
			valueTypeId = TYPEID(String);
		}

		template<>
		Variant(const char* value)
		{
			Clear();
			StringValue = value; 
			valueTypeId = TYPEID(String);
		}

		template<typename T> requires std::is_enum<T>::value
		Variant(T enumValue)
		{
			SetInternalValue((std::underlying_type_t<T>)enumValue);
			valueTypeId = TYPEID(T);
		}

		INLINE bool HasValue() const
		{
			return valueTypeId != 0;
		}

		template<typename T>
		INLINE bool IsOfType() const
		{
			return valueTypeId == TYPEID(T);
		}

		bool IsTextType() const
		{
			return IsOfType<String>() || IsOfType<Name>();
		}

		String GetTextValue() const
		{
			if (IsOfType<String>())
			{
				return StringValue;
			}
			else if (IsOfType<Name>())
			{
				return nameValue.GetString();
			}
			else
			{
				throw VariantCastException(String::Format("Failed to cast Variant to a Text type"));
			}
		}

		Name GetNameValue() const
		{
			if (IsOfType<String>())
			{
				return StringValue;
			}
			else if (IsOfType<Name>())
			{
				return nameValue;
			}
			else
			{
				throw VariantCastException(String::Format("Failed to cast Variant to a Text type"));
			}
		}

		INLINE TypeId GetValueTypeId() const
		{
			return valueTypeId;
		}

		INLINE bool IsPointer() const
		{
			return isPointer;
		}

		INLINE bool IsArray() const
		{
			return isArray;
		}

		bool IsStruct() const
		{
			return isStruct;
		}

		bool IsObject() const;
		bool CanCastObject(TypeId castTo) const;

		SIZE_T GetHash() const;

		template<typename T>
		INLINE bool IsArrayElementOfType() const
		{
			return arrayElementTypeId == TYPEID(T);
		}

		template<typename TType>
		TType GetValue() const
		{
			typedef std::remove_cvref_t<TType> T;
			constexpr bool IsRefType = std::is_reference_v<TType>;

			if constexpr (TIsRef<T>::Value)
			{
				if (!CanCastObject(TYPEID(T)))
				{
					throw VariantCastException(String::Format("Failed to cast Variant to the return type"));
				}

				return (typename TIsRef<T>::Type*)PtrValue;
			}

			if (CanCastObject(TYPEID(T)))
			{

			}
			else if (valueTypeId != GetTypeId<T>())
			{
				throw VariantCastException(String::Format("Failed to cast Variant to the return type"));
			}

			if constexpr (TIsStruct<T>::Value)
			{
				if (StructValue == nullptr)
				{
					throw VariantCastException(String::Format("Failed to cast struct type. Struct value is NULL!"));
				}
				const T& val = *(T*)StructValue;
				return val;
			}
			else if constexpr (TIsSameType<T, Array<String>>::Value)
			{
				return GetArrayStringValue();
			}
			else if constexpr (TIsSameType<T, Array<Name>>::Value)
			{
				return GetArrayNameValue();
			}
			else if constexpr (TIsSameType<T, Array<IO::Path>>::Value)
			{
				return GetArrayPathValue();
			}
			else if constexpr (IsRefType)
			{
				if (isPointer)
				{
					return (T&)*(T*)PtrValue;
				}
				return *(T*)this;
			}
			else
			{
				return *(T*)this;
			}
		}

		//template<>
		const Array<String>& GetArrayStringValue() const
		{
			if (valueTypeId != TYPEID(Array<String>))
			{
				throw VariantCastException("Failed to cast Variant to the return type of Array<String> because it is not an array");
			}
			if (arrayElementTypeId != TYPEID(String))
			{
				throw VariantCastException("Failed to cast Variant to the return type of Array<String> because of Element Type");
			}

			return stringArrayValue;
		}

		//template<>
		const Array<Name>& GetArrayNameValue() const
		{
			if (valueTypeId != TYPEID(Array<Name>))
			{
				throw VariantCastException("Failed to cast Variant to the return type of Array<Name> because it is not an array");
			}
			if (arrayElementTypeId != TYPEID(Name))
			{
				throw VariantCastException("Failed to cast Variant to the return type of Array<Name> because of Element Type");
			}

			return nameArrayValue;
		}

		//template<>
		const Array<IO::Path>& GetArrayPathValue() const
		{
			if (valueTypeId != TYPEID(Array<IO::Path>))
			{
				throw VariantCastException("Failed to cast Variant to the return type of Array<IO::Path> because it is not an array");
			}
			if (arrayElementTypeId != TYPEID(IO::Path))
			{
				throw VariantCastException("Failed to cast Variant to the return type of Array<IO::Path> because of Element Type");
			}

			return pathArrayValue;
		}

		INLINE void* GetRawPtrValue() const
		{
			return PtrValue;
		}

		template<typename PtrType>
		INLINE PtrType* TryGetPointerValue() const
		{
			if (TYPEID(PtrType) != valueTypeId)
				return nullptr;
			return (PtrType*)PtrValue;
		}

		template<typename PtrType>
		INLINE PtrType* GetPointerValue() const
		{
			return (PtrType*)PtrValue;
		}

	private:

		template<typename T>
		INLINE void SetInternalValue(const T& value)
		{
			memset(this, 0, sizeof(Variant)); // Initialize memory to 0
			valueTypeId = TYPEID(T);
			*(T*)this = value;
		}

		template<typename TStruct>
		void SetStructValue(const TStruct& value)
		{
			Clear();
			valueTypeId = TYPEID(TStruct);
			structType = TStruct::StaticType();
			isStruct = true;
			StructValue = malloc(sizeof(TStruct));
			new(StructValue) TStruct(value);
		}

		/// Initializes/sets the memory block to zero
		void Clear()
		{
			memset(this, 0, sizeof(Variant)); // Initialize memory to 0
		}

		union
		{
			// Simple types
			bool   BoolValue;
			String StringValue;
			s8	   Int8Value;
			s16    Int16Value;
			s32    Int32Value;
			s64    Int64Value;
			u8     Uint8Value;
			u16    Uint16Value;
			u32    Uint32Value;
			u64    Uint64Value;
			f32    Float32Value;
			f64    Float64Value;
			void*  PtrValue;
			struct
			{
				void* StructValue;
				StructType* structType;
			};
            
            Vec2   Vec2Value;
            Vec3   Vec3Value;
            Vec4   Vec4Value;
			Quat   QuatValue;
			Matrix4x4 MatrixValue;

			Color  ColorValue;

			Vec2i  Vec2iValue;
			Vec3i  Vec3iValue;
			Vec4i  Vec4iValue;

			IO::Path pathValue;
			CE::Name nameValue;

			// Arrays
			CE::Array<u8> rawArrayValue;
			CE::Array<String> stringArrayValue;
			CE::Array<Name> nameArrayValue;
			CE::Array<IO::Path> pathArrayValue;
		};

		TypeId valueTypeId = 0;
		TypeId arrayElementTypeId = 0;
		bool isPointer = false;
		bool isArray = false;
		bool isStruct = false;
	};

} // namespace CE
