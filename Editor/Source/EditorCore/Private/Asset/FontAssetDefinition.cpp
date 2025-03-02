#include "EditorCore.h"

namespace CE::Editor
{
	const Array<String>& FontAssetDefinition::GetSourceExtensions()
	{
		static const Array<String> extensions{
			".ttf"
		};

		return extensions;
	}

	SubClass<AssetImporter> FontAssetDefinition::GetAssetImporterClass()
	{
		return GetStaticClass<FontAssetImporter>();
	}

	SubClassType<Asset> FontAssetDefinition::GetAssetClass()
	{
		return CE::Font::StaticClass();
	}

} // namespace CE::Editor
