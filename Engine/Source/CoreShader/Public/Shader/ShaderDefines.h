#pragma once

namespace CE
{

	ENUM()
	enum class ShaderBlobFormat
	{
		Spirv = 0
	};

	typedef HashMap<Name, int> VariableBindingMap;
    
	STRUCT()
	struct CORESHADER_API ShaderReflection
	{
		CE_STRUCT(ShaderReflection)
	public:

		inline bool IsValid() const
		{
			return srgLayouts.NotEmpty();
		}

		RHI::ShaderResourceGroupLayout& FindOrAdd(RHI::SRGType srgType);

		FIELD(ReadOnly)
		Array<RHI::ShaderResourceGroupLayout> srgLayouts{};

		FIELD(ReadOnly)
		Array<Name> vertexInputs{};

		FIELD(ReadOnly)
		Array<RHI::VertexAttributeDataType> vertexInputTypes{};

		FIELD(ReadOnly)
		Array<RHI::ShaderStructMemberType> rootConstantLayout{};

		FIELD(ReadOnly)
		ShaderStage rootConstantStages = ShaderStage::None;

		FIELD(ReadOnly)
		Vec3i invocationSize;

		const VariableBindingMap& GetVariableNameMap() const;

	private:

		void OnAfterDeserialize();

		mutable VariableBindingMap variableNameToBindingSlot{};
		
		friend class ShaderReflector;
		friend class Shader;
	};

} // namespace CE

#include "ShaderDefines.rtti.h"
