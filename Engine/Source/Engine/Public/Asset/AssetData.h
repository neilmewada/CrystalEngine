#pragma once

namespace CE
{

	struct ENGINE_API AssetData
	{
		// - Constructors -

		AssetData() {}

		// - Fields -

		// Bundle::name field value.
		Name bundleName{};

		Name bundlePath{};

		// Uuid of the bundle that contains this asset.
		Uuid bundleUuid{};

		// Path of the primary asset within the bundle. Ex: TextureMap
		Name assetName{};

		// Uuid of the primary asset within the bundle.
		Uuid assetUuid{};

		// Primary asset's ClassTypeName. Ex: /Code/System.CE::Texture2D
		Name assetClassTypeName{};

		// Path to the source asset. For Editor only!
		Name sourceAssetPath{};
	};

	template<>
	inline SIZE_T GetHash<AssetData>(const AssetData& assetData)
	{
		return GetCombinedHashes({ 
			assetData.bundleName.GetHashValue(),
			assetData.assetName.GetHashValue(), 
			assetData.assetClassTypeName.GetHashValue(),
			assetData.bundleUuid.GetHash(),
			assetData.assetUuid.GetHash(),
			assetData.sourceAssetPath.GetHashValue()
		});
	}
    
} // namespace CE
