#include "FusionCore.h"

namespace CE
{

	FShader::FShader()
	{
		
	}

	Ref<FShader> FShader::Create(RawData vertexShader, RawData fragmentShader, Ref<Object> outer)
	{
		if (!outer)
			outer = FApplication::Get();

		RPI::ShaderVariantDescriptor desc{};
		desc.interleaveVertexData = true;

		desc.moduleDesc.Add({});
		auto& vertexDesc = desc.moduleDesc.GetLast();
		vertexDesc.name = "FShader Vertex";
		vertexDesc.defaultEntryPoint = "VertMain";
		vertexDesc.stage = ShaderStage::Vertex;
		
	}

	void FShader::OnBeforeDestroy()
	{
		Super::OnBeforeDestroy();

		delete shader; shader = nullptr;
	}

} // namespace CE
