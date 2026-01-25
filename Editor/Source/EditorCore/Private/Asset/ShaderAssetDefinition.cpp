#include "EditorCore.h"

namespace CE::Editor
{

	const Array<String>& ShaderAssetDefinition::GetSourceExtensions()
	{
		static const Array<String> extensions = {
			".shader"
		};
		return extensions;
	}

	SubClassType<AssetImporter> ShaderAssetDefinition::GetAssetImporterClass()
	{
		return GetStaticClass<ShaderAssetImporter>();
	}

	SubClassType<Asset> ShaderAssetDefinition::GetAssetClass()
	{
		return CE::Shader::StaticClass();
	}

	Color ShaderAssetDefinition::GetColorTag()
	{
		return Color(0.7f, 0.4f, 1.0f, 1.0f);
	}
} // namespace CE::Editor
