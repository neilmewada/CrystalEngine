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
} // namespace CE::Editor
