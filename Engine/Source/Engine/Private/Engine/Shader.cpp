#include "Engine.h"

namespace CE
{

	ShaderBlob::~ShaderBlob()
	{
		byteCode.Free();
	}

	void CE::ShaderVariant::Release()
	{
		
	}

	ShaderBlob* CE::ShaderVariant::GetShaderBlobForStage(ShaderStage stage)
	{
		for (int i = 0; i < shaderStageBlobs.GetSize(); i++)
		{
			if (shaderStageBlobs[i]->shaderStage == stage)
				return shaderStageBlobs[i];
		}

		return nullptr;
	}

	CE::Shader::Shader()
	{
		
	}

	CE::Shader::~Shader()
	{
		for (const ShaderCollection::Item& item : shaderCollection)
		{
			if (item.shader)
			{
				delete item.shader;
			}
		}

		shaderCollection.Clear();
	}

	void CE::Shader::OnAfterConstruct()
	{
		Super::OnAfterConstruct();
	}

	void CE::Shader::OnAfterDeserialize()
	{
		Super::OnAfterDeserialize();

		SubShader* subShader = GetSubShader();

		if (!subShader)
		{
			String::IsAlphabet('a');
		}
	}

	u32 CE::Shader::GetShaderPassCount()
	{
		SubShader* subShader = GetSubShader();
		if (!subShader)
			return 0;
		return subShader->passes.GetSize();
	}

	RPI::ShaderCollection* CE::Shader::GetShaderCollection()
	{
		LockGuard<SharedMutex> lock{ rpiShaderMutex };

		if (shaderCollection.IsEmpty())
		{
			shaderCollection = RPI::ShaderCollection();

			SubShader* subShader = GetSubShader();

			for (int i = 0; i < GetShaderPassCount(); i++)
			{
				CE::ShaderPass* shaderPass = GetShaderPass(i);
				RHI::DrawListTag drawListTag = {};

				for (const auto& tagEntry : shaderPass->tags)
				{
					if (tagEntry.key == "DrawListTag")
					{
						drawListTag = RPISystem::Get().GetDrawListTagRegistry()->AcquireTag(tagEntry.value);
						break;
					}
				}

				RPI::Shader* rpiShader = new RPI::Shader(drawListTag);

				for (int j = 0; j < shaderPass->variants.GetSize(); j++)
				{
					const CE::ShaderVariant& variant = shaderPass->variants[j];

					RPI::ShaderVariantDescriptor variantDesc{};
					variantDesc.shaderName = shaderPass->passName;
					variantDesc.reflectionInfo = variant.reflectionInfo;
					variantDesc.tags = {};
					variantDesc.tags.AddRange(subShader->tags);
					variantDesc.tags.AddRange(shaderPass->tags);

					for (ShaderBlob* curShaderBlob : variant.shaderStageBlobs)
					{
						variantDesc.moduleDesc.Add({});
						variantDesc.moduleDesc.Top().byteCode = curShaderBlob->byteCode.GetDataPtr();
						variantDesc.moduleDesc.Top().byteSize = curShaderBlob->byteCode.GetDataSize();
						variantDesc.moduleDesc.Top().name = curShaderBlob->GetName().GetString();
						variantDesc.moduleDesc.Top().stage = curShaderBlob->shaderStage;

						Name entryPoint = "Main";

						if (curShaderBlob->shaderStage == RHI::ShaderStage::Vertex)
							entryPoint = shaderPass->vertexEntry;
						else if (curShaderBlob->shaderStage == RHI::ShaderStage::Fragment)
							entryPoint = shaderPass->fragmentEntry;
						else
						{
							EnumType* shaderStageEnum = GetStaticEnum<RHI::ShaderStage>();
							EnumConstant* constant = shaderStageEnum->FindConstantWithValue((s64)curShaderBlob->shaderStage);

							if (constant != nullptr && shaderPass->TagExists(constant->GetName()))
							{
								entryPoint = shaderPass->GetTagValue(constant->GetName());
							}
						}

						variantDesc.entryPoints.Add(entryPoint);
					}

					rpiShader->AddVariant(variantDesc);
				}

				if (drawListTag.IsValid())
				{
					ShaderCollection::Item shaderItem{ .shaderTag = shaderPass->passName, .shader = rpiShader, .enabled = true, .drawListOverride = drawListTag };
					shaderCollection.Add(shaderItem);
				}
				else
				{
					ShaderCollection::Item shaderItem{ .shaderTag = shaderPass->passName, .shader = rpiShader, .enabled = true, .drawListOverride = DrawListTag::NullValue };
					shaderCollection.Add(shaderItem);
				}
			}
		}

		return &shaderCollection;
	}

	SubShader* CE::Shader::GetSubShader()
	{
		String platformName = PlatformMisc::GetPlatformName();
#if PLATFORM_DESKTOP
		String platformGroup = "Desktop";
#elif PLATFORM_MOBILE
		String platformGroup = "Mobile";
#else
		String platformGroup = "";
#endif

		for (int i = 0; i < subShaders.GetSize(); i++)
		{
			const Array<ShaderTagEntry>& subShaderTags = subShaders[i].tags;

			bool valid = true;

			for (const ShaderTagEntry& tag : subShaderTags)
			{
				if (tag.key == "Platform" && (tag.value != platformName && (platformGroup.IsEmpty() || tag.value != platformGroup)))
				{
					valid = false;
				}
			}

			if (valid)
				return &subShaders[i];
		}

		return nullptr;
	}

} // namespace CE
