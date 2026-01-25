#pragma once

namespace CE::Editor
{
    
    CLASS()
	class EDITORCORE_API TextureAssetDefinition : public AssetDefinition
	{
		CE_CLASS(TextureAssetDefinition, AssetDefinition)
	public:

		/// Return an array of extensions of source files that produce this asset type.
		virtual const Array<String>& GetSourceExtensions() override;

		virtual SubClassType<AssetImporter> GetAssetImporterClass() override;

		virtual SubClassType<Asset> GetAssetClass() override;

		virtual CE::Name GetIconPath() override;

		SubClass<AssetThumbnailGen> GetThumbnailGeneratorClass() override;

		Color GetColorTag() override;

	};

} // namespace CE::Editor

#include "TextureAssetDefinition.rtti.h"