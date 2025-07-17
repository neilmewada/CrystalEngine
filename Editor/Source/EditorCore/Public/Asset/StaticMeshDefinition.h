#pragma once

namespace CE::Editor
{

	CLASS()
	class EDITORCORE_API StaticMeshAssetDefinition : public AssetDefinition
	{
		CE_CLASS(StaticMeshAssetDefinition, AssetDefinition)
	public:

		/// Return an array of extensions of source files that produce this asset type.
		virtual const Array<String>& GetSourceExtensions() override;

		virtual SubClass<AssetImporter> GetAssetImporterClass() override;

		virtual SubClassType<Asset> GetAssetClass() override;

		Color GetColorTag() override;
	};
    
} // namespace CE::Editor

#include "StaticMeshDefinition.rtti.h"
