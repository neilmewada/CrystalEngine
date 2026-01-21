#pragma once

namespace CE::RHI
{
	ENUM()
	enum class SRGType
	{
		PerScene = 0,
		PerView,
		PerPass,
		PerSubPass,
		PerMaterial,
		PerObject,
		PerDraw,
        COUNT
	};
	ENUM_CLASS(SRGType);

    ENUM()
    enum class ShaderStructMemberType
    {
        None = 0,
        UInt, Int,
        Float,
        Float2,
        Float3,
        Float4,
        Float4x4,
		Struct
    };
    ENUM_CLASS_FLAGS(ShaderStructMemberType);

    STRUCT()
    struct CORERHI_API ShaderStructMember final
    {
        CE_STRUCT(ShaderStructMember)
    public:

        FIELD()
        Name name{};

        FIELD()
        ShaderStructMemberType dataType{};

		FIELD()
		u32 arrayCount = 1; // Static arrays
        
		FIELD()
		Array<ShaderStructMember> nestedMembers{};
    };

    ENUM()
    enum class SRGVariableFlags
    {
        None = 0,
        RarelyUpdated = BIT(0)
    };
    ENUM_CLASS_FLAGS(SRGVariableFlags);

	STRUCT()
	struct CORERHI_API SRGVariableDescriptor final
	{
		CE_STRUCT(SRGVariableDescriptor)
	public:

		SRGVariableDescriptor() {}

		SRGVariableDescriptor(const Name& name, u32 bindingSlot, ShaderResourceType type, ShaderStage shaderStages, u32 arrayCount = 1)
			: name(name), bindingSlot(bindingSlot), type(type), shaderStages(shaderStages), arrayCount(arrayCount)
		{}

		/// @brief Name of the variable.
		FIELD()
		Name name{};

		/// @brief Each variable has a unique binding slot, usually retrieved via reflection.
		FIELD()
		u32 bindingSlot = 0;

		FIELD()
		u32 arrayCount = 1;

		FIELD()
		ShaderResourceType type = ShaderResourceType::None;

		FIELD()
		b8 usesDynamicOffset = false;
        
        FIELD()
        SRGVariableFlags flags = SRGVariableFlags::None;
        
		FIELD()
		RHI::Format format{};

		FIELD()
		ShaderStage shaderStages = ShaderStage::Default;
        
        FIELD()
        Array<ShaderStructMember> structMembers{};
	};

	STRUCT()
	struct CORERHI_API ShaderResourceGroupLayout final
	{
		CE_STRUCT(ShaderResourceGroupLayout)
	public:

		FIELD()
		SRGType srgType = SRGType::PerScene;

		FIELD()
		Array<SRGVariableDescriptor> variables{};

		bool IsEmpty() const
		{
			return variables.IsEmpty();
		}

		const SRGVariableDescriptor& FindVariable(const Name& name) const
		{
			static SRGVariableDescriptor empty{};

			for (const auto& variable : variables)
			{
				if (variable.name == name)
				{
					return variable;
				}
			}

			return empty;
		}
        
        const SRGVariableDescriptor& FindVariable(u32 bindingSlot) const
        {
            static SRGVariableDescriptor empty{};

            for (const auto& variable : variables)
            {
                if (variable.bindingSlot == bindingSlot)
                {
                    return variable;
                }
            }

            return empty;
        }

		Self& TryAdd(const SRGVariableDescriptor& variable)
		{
			for (int i = 0; i < variables.GetSize(); i++)
			{
				if (variables[i].name == variable.name)
				{
					variables[i].shaderStages |= variable.shaderStages;
					return *this;
				}
			}

			variables.Add(variable);

			return *this;
		}

		/// @brief Merge 'other' SRG layout into 'this' SRG layout.
		void Merge(const ShaderResourceGroupLayout& other)
		{
			if ((int)other.srgType > (int)srgType)
			{
				srgType = other.srgType;
			}

			for (const auto& otherVariable : other.variables)
			{
				bool exists = false;
				
				for (const auto& thisVariable : variables)
				{
					if (thisVariable.bindingSlot == otherVariable.bindingSlot || thisVariable.name == otherVariable.name)
					{
						exists = true;
						break;
					}
				}

				if (!exists)
				{
					variables.Add(otherVariable);
				}
			}
		}

	};

    struct CORERHI_API ShaderResourceGroupDescriptor
    {
        Name name = "ShaderResourceGroup";
        RHI::ShaderResourceGroupLayout layout{};
        
        /// You need to pass a shader module to create a shader resource group, just for the setup.
        /// The shader module does not need to be active after the initialization.
        RHI::ShaderModule* shaderHint = nullptr;
    };
    
} // namespace CE::RHI

#include "ShaderResourceGroupLayout.rtti.h"
