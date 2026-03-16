#include "FusionCore.h"

namespace CE
{

	FShader::FShader()
	{
		
	}

	Ref<FShader> FShader::Create(Name shaderName, const ShaderReflection& reflection, RawData vertexShader, RawData fragmentShader, Ref<Object> outer)
	{
		if (!outer)
			outer = FApplication::Get();

		RPI::ShaderVariantDescriptor desc{};
		desc.shaderName = shaderName;
		desc.interleaveVertexData = true;

		desc.moduleDesc.Add({});
		auto& vertexDesc = desc.moduleDesc.GetLast();
		vertexDesc.name = "FShader Vertex";
		vertexDesc.defaultEntryPoint = "VertMain";
		vertexDesc.stage = ShaderStage::Vertex;
		vertexDesc.byteCode = vertexShader.data;
		vertexDesc.byteSize = vertexShader.dataSize;

		desc.moduleDesc.Add({});
		auto& fragmentDesc = desc.moduleDesc.GetLast();
		fragmentDesc.name = "FShader Fragment";
		fragmentDesc.stage = ShaderStage::Fragment;
		fragmentDesc.defaultEntryPoint = "FragMain";
		fragmentDesc.byteCode = fragmentShader.data;
		fragmentDesc.byteSize = fragmentShader.dataSize;

		desc.entryPoints.Add("VertMain");
		desc.entryPoints.Add("FragMain");

		desc.reflectionInfo = reflection;

		Ref<FShader> shader = CreateObject<FShader>(outer.Get(), "Shader");
		shader->shader = new RPI::Shader();

		shader->shader->AddVariant(desc);

		return shader;
	}

	RHI::PipelineState* FShader::GetDefaultPipeline()
	{
		if (shader)
		{
			return shader->GetDefaultPipeline();
		}
		return nullptr;
	}

	void FShader::OnBeforeDestroy()
	{
		Super::OnBeforeDestroy();

		delete shader; shader = nullptr;
	}

} // namespace CE
