#pragma once

#include "Types/CoreTypeDefs.h"

#include "RTTI/RTTI.h"

#include <vector>

#include "Event.h"

namespace CE
{
    class Object;
    class IScriptEvent;

    enum FieldFlags
    {
        FIELD_NoFlags = 0,
        FIELD_Hidden = BIT(0),
        FIELD_ReadOnly = BIT(1),
        FIELD_Serializable = BIT(2),
        FIELD_Config = BIT(3),
        FIELD_ImportSetting = BIT(4),
		FIELD_Internal = BIT(5),
        FIELD_EditAnywhere = BIT(6),
        FIELD_VisibleAnywhere = BIT(7),
        FIELD_FusionProperty = BIT(8),
        FIELD_FusionPropertyLayout = BIT(9),
        FIELD_FusionPropertyData = BIT(10),
        FIELD_FusionPropertyDataLayout = BIT(11)
    };
    ENUM_CLASS_FLAGS(FieldFlags);
    
    class CORE_API FieldType : public TypeInfo, public IntrusiveBase
    {
    private:
        FieldType(const String& name, TypeId fieldTypeId, TypeId underlyingTypeId, SIZE_T size, SIZE_T offset, String attributes,
            const TypeInfo* owner = nullptr, 
            RefType refType = RefType::None,
            const String& relativePathToParent = "");

        static Ptr<FieldType> Clone(const Ptr<FieldType>& copy);

        void ConstructInternal();

    public:

        virtual ~FieldType();

		virtual void InitializeDefaults(void* instance) override;
		virtual void CallDestructor(void* instance) override;

		virtual const CE::Name& GetTypeName() override;

        String GetDisplayName() override;
        
        bool IsField() const override { return true; }

        INLINE FieldType* GetNext() const { return next; }

        TypeId GetTypeId() const override
        {
            return fieldTypeId;
        }

        INLINE SIZE_T GetOffset() const { return offset; }

        virtual u32 GetSize() const override { return (u32)size; }
        
        virtual bool IsAssignableTo(CE::TypeId typeId) override;
        virtual bool IsObject() override { return IsAssignableTo(TYPEID(Object)); }

        bool HasCustomPODSerialization() override;
        
		bool IsPODField();
        bool IsArrayField() const;
        bool IsStringField() const;
		bool IsTypeInfoField();

        bool IsIntegerField() const;
        bool IsDecimalField() const;
		bool IsEnumField();
        bool IsEnumFlagsField();
        bool IsEnumArrayField();

        bool IsEventField() const;
        bool IsDelegateField() const;
        bool IsPropertyBindingField() const;

        bool IsNumericField() const { return IsIntegerField() || IsDecimalField(); }

        bool IsObjectField() const;
		bool IsStructField();

        bool IsRefCounted() const;
        bool IsStrongRefCounted() const;
        bool IsWeakRefCounted() const;

        bool HasAnyFieldFlags(FieldFlags flags) const
        {
            return (fieldFlags & flags) != 0;
        }

        bool HasAllFieldFlags(FieldFlags flags) const
        {
            return (fieldFlags & flags) == flags;
        }

        bool IsSerialized() const;
        bool IsHidden() const;
        bool IsReadOnly() const;
		bool IsInternal() const;

        bool IsFusionProperty();
        bool IsFusionBasicProperty();
        bool IsFusionDataProperty();
        bool IsFusionDataLayoutProperty();
        bool IsFusionLayoutProperty();

        bool IsEditAnywhere() const;
        bool IsVisibleAnywhere() const;

        /// The strict owner of this field, which remains same for all derived classes.
		TypeInfo* GetOwnerType();
        
        /// The struct/class's instance that this field belongs to, even if it is inherited from a base class.
		TypeInfo* GetInstanceOwnerType();
        
        virtual CE::TypeId GetUnderlyingTypeId() const override { return underlyingTypeId; }

		TypeInfo* GetUnderlyingType() override;

        TypeInfo* GetDeclarationType() const;

        TypeId GetDeclarationTypeId() const { return fieldTypeId; }

		String GetFieldValueAsString(void* instance);

        f64 GetNumericFieldValue(void* instance);

		// Returns pointer to the field's location in memory itself
		void* GetFieldInstance(void* instance)
		{
			return (void*)((SIZE_T)instance + offset);
		}

		//template<typename T>
		//T& GetFieldValue(void* instance)
		//{
		//	return *(T*)((SIZE_T)instance + offset);
		//}

        template<typename T>
        const T& GetFieldValue(void* instance) const
        {
            return *(T*)((SIZE_T)instance + offset);
        }

        IScriptEvent* GetFieldEventValue(void* instance) const
		{
			return (IScriptEvent*)((SIZE_T)instance + offset);
		}

        IScriptDelegate* GetFieldDelegateValue(void* instance) const
		{
			return (IScriptDelegate*)((SIZE_T)instance + offset);
		}
        
        template<typename T>
        void SetFieldValue(void* instance, const T& value)
        {
			if (IsReadOnly())
				return;
			ForceSetFieldValue(instance, value);

            if (instanceOwner && instanceOwner->IsObject())
            {
                NotifyObjectFieldUpdate((Object*)instance);
            }
        }

    	Ref<Object> GetFieldObjectValue(void* instance);

        void SetFieldObjectValue(void* instance, const Ref<Object>& object);

		template<typename T>
		INLINE void ForceSetFieldValue(void* instance, const T& value)
		{
            if (instance == nullptr)
                return;

			*(T*)((SIZE_T)instance + offset) = value;
		}

		bool CopyTo(void* srcInstance, const Ptr<FieldType>& destField, void* destInstance);

        s64 GetFieldEnumValue(void* instance);
        void SetFieldEnumValue(void* instance, s64 value);
		void ForceSetFieldEnumValue(void* instance, s64 value);

		// - Array API -

		u32 GetArraySize(void* instance);

		void ResizeArray(void* instance, u32 numElements);

		void InsertArrayElement(void* instance);
		void InsertArrayElement(void* instance, u32 insertPosition);

		void DeleteArrayElement(void* instance, u32 deletePosition);

		template<typename T>
		const T& GetArrayElementValueAt(u32 index, void* instance) const
		{
			if (!IsArrayField())
				return {};

			const Array<T>& array = GetFieldValue<Array<T>>(instance);
            return array[index];
		}

        template<typename T>
        void SetArrayElementValueAt(u32 index, void* instance, const T& value)
        {
            if (!IsArrayField())
                return;

            Array<T>& array = const_cast<Array<T>&>(GetFieldValue<Array<T>>(instance));
            array[index] = value;
        }

        Array<Ptr<FieldType>> GetArrayFieldListPtr(void* instance);

        Ptr<FieldType> GetArrayFieldElementPtr(void* instance, u32 position);

        void* GetArrayInstance(void* instance);

    private:

        void NotifyObjectFieldUpdate(Object* instance);

        FieldFlags fieldFlags = FIELD_NoFlags;
        
		Name typeName{};
        TypeId fieldTypeId;
        TypeId underlyingTypeId;

		mutable TypeInfo* declarationType = nullptr;
		TypeInfo* underlyingTypeInfo = nullptr;

        SIZE_T offset;
        SIZE_T size;

        FieldType* next = nullptr;
        TypeInfo* owner = nullptr;
		TypeInfo* instanceOwner = nullptr;
        RefType refType = RefType::None;
    	String relativePathToParent;

        friend class StructType;
        friend class ClassType;

    };

	typedef FieldType Field;
    
} // namespace CE

CE_RTTI_TYPEINFO(CORE_API, CE, FieldType, TYPEID(CE::TypeInfo))
