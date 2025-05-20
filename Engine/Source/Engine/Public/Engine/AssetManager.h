#pragma once

namespace CE
{
	// TODO: Implement CWidgetResourceLoader in AssetManager

	CLASS()
	class ENGINE_API AssetManager : public Object, public IFusionAssetLoader
	{
		CE_CLASS(AssetManager, Object)
	public:

		AssetManager();
		virtual ~AssetManager();

		static AssetManager* Get();

		static AssetRegistry* GetRegistry();

		virtual void Initialize();
		virtual void Shutdown();

		virtual void Tick(f32 deltaTime);

		AssetData* GetPrimaryAssetDataAtPath(const Name& path);
		Array<AssetData*> GetAssetsDataAtPath(const Name& path);
		
		Ref<Asset> LoadAssetAtPath(const Name& path);

		RHI::Texture* LoadTextureAtPath(const Name& path) override;

		CMImage LoadImageAssetAtPath(const Name& path) override;

		Array<Ref<Asset>> LoadAssetsAtPath(const Name& path, SubClass<Asset> classType = Asset::StaticType());

		template<typename TAsset> requires TIsBaseClassOf<Asset, TAsset>::Value
		inline Array<Ref<TAsset>> LoadAssetsAtPath(const Name& path)
		{
			Array<Asset*> assets = LoadAssetsAtPath(path, TAsset::StaticType());
			Array<TAsset*> outAssets{};
			for (auto asset : assets)
			{
				TAsset* cast = Object::CastTo<TAsset>(asset);
				if (cast)
				{
					outAssets.Add(cast);
				}
			}
			return outAssets;
		}

		template<typename TAsset> requires TIsBaseClassOf<Asset, TAsset>::Value
		inline Ref<TAsset> LoadAssetAtPath(const Name& path)
		{
			Ref<Asset> asset = LoadAssetAtPath(path);
			if (asset.IsNull())
				return nullptr;
			return Object::CastTo<TAsset>(asset.Get());
		}

		void UnloadAsset(Asset* asset);
		void UnloadAsset(Ref<Bundle> bundle);

	protected:

		FIELD()
		AssetRegistry* assetRegistry = nullptr;

		SharedMutex loadedAssetsMutex{};
		HashMap<Name, Ref<Bundle>> loadedAssetsByPath{};
		HashMap<Uuid, WeakRef<Bundle>> loadedAssetsByUuid{};

		friend class Engine;
		friend class AssetRegistry;
	};
    
} // namespace CE

#include "AssetManager.rtti.h"
