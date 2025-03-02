#pragma once

namespace CE::Editor
{
    
    CLASS()
	class EDITORCORE_API FontAssetDefinition : public AssetDefinition
	{
		CE_CLASS(FontAssetDefinition, AssetDefinition)
	public:

		/// Return an array of extensions of source files that produce this asset type.
		virtual const Array<String>& GetSourceExtensions() override;

		virtual SubClass<AssetImporter> GetAssetImporterClass() override;

		virtual SubClassType<Asset> GetAssetClass() override;

	};

} // namespace CE::Editor

#include "FontAssetDefinition.rtti.h"
