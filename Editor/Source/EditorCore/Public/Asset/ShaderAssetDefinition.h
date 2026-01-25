#pragma once

namespace CE::Editor
{

	CLASS()
	class EDITORCORE_API ShaderAssetDefinition : public AssetDefinition
	{
		CE_CLASS(ShaderAssetDefinition, AssetDefinition)
	public:

		/// Return an array of extensions of source files that produce this asset type.
		virtual const Array<String>& GetSourceExtensions() override;

		virtual SubClassType<AssetImporter> GetAssetImporterClass() override;

		virtual SubClassType<Asset> GetAssetClass() override;

		Color GetColorTag() override;
	};
    
} // namespace CE::Editor

#include "ShaderAssetDefinition.rtti.h"
