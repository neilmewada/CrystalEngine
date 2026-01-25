#include "CoreRPI.h"

namespace CE::RPI
{

	Shader::Shader(RHI::DrawListTag drawListTag) : drawListTag(drawListTag)
	{

	}

	Shader::~Shader()
	{
		if (drawListTag.IsValid())
		{
			RPISystem::Get().GetDrawListTagRegistry()->ReleaseTag(drawListTag);
			drawListTag = {};
		}

		for (RPI::ShaderVariant* variant : variants)
		{
			delete variant;
		}
		variants.Clear();
	}

	Name Shader::GetName()
	{
		if (variants.NotEmpty())
		{
			return variants[0]->pipelineDesc.name;
		}

		return Name();
	}

	RPI::ShaderVariant* Shader::AddVariant(const ShaderVariantDescriptor& variantDesc)
	{
		ShaderVariant* variant = new ShaderVariant(variantDesc);
		variants.Add(variant);
		return variant;
	}

} // namespace CE::RPI
