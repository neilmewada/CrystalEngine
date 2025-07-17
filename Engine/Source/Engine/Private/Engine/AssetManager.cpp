#include "Engine.h"

namespace CE
{
    
	AssetManager::AssetManager()
	{
		assetRegistry = CreateDefaultSubobject<AssetRegistry>("AssetRegistry", OF_Transient);
	}

	AssetManager::~AssetManager()
	{
		
	}

	AssetManager* AssetManager::Get()
	{
		if (gEngine == nullptr)
			return nullptr;
		return gEngine->GetAssetManager();
	}

	AssetRegistry* AssetManager::GetRegistry()
	{
		if (Self::Get() == nullptr)
			return nullptr;
		return Self::Get()->assetRegistry;
	}

	void AssetManager::Initialize()
	{
		// Cache asset paths & load saved cache
		assetRegistry->InitializeCache();
	}

	void AssetManager::Shutdown()
	{
		{
			LockGuard lock{ loadedAssetsMutex };

			for (auto [uuid, bundle] : loadedAssetsByUuid)
			{
				if (bundle != nullptr)
				{
					bundle->BeginDestroy();
				}
			}

			loadedAssetsByUuid.Clear();
			loadedAssetsByPath.Clear();
		}

		{
			for (auto [path, srg] : loadedImageSrgsByPath)
			{
				RHI::gDynamicRHI->DestroyShaderResourceGroup(srg);
			}
			loadedImageSrgsByPath.Clear();

			for (auto [path, textureView] : loadedImageViewsByPath)
			{
				RHI::gDynamicRHI->DestroyTextureView(textureView);
			}
			loadedImageViewsByPath.Clear();

			for (auto [path, bundle] : loadedTempBundlesByPath)
			{
				if (bundle.IsValid())
				{
					bundle->BeginDestroy();
				}
			}
			loadedTempBundlesByPath.Clear();
		}

		assetRegistry->Shutdown();
	}

	void AssetManager::Tick(f32 deltaTime)
	{
		
	}

	AssetData* AssetManager::GetPrimaryAssetDataAtPath(const Name& path)
	{
		if (!assetRegistry)
			return nullptr;
		return assetRegistry->GetPrimaryAssetByPath(path);
	}

	Array<AssetData*> AssetManager::GetAssetsDataAtPath(const Name& path)
	{
		if (!assetRegistry)
			return {};
		return assetRegistry->GetAssetsByPath(path);
	}

	Ref<Bundle> AssetManager::LoadAssetBundleAtPath(const Name& path)
	{
		ZoneScoped;

		Ref<Bundle> bundle = nullptr;

		loadedAssetsMutex.Lock();
		if (loadedAssetsByPath.KeyExists(path) && loadedAssetsByPath[path] != nullptr)
		{
			bundle = loadedAssetsByPath[path];
			loadedAssetsMutex.Unlock();
		}
		else
		{
			loadedAssetsMutex.Unlock();

			LoadBundleArgs args{
				.loadFully = true,
				.forceReload = false
			};

			bundle = Bundle::LoadBundle(this, path, args);

			if (bundle == nullptr)
			{
				return {};
			}

			loadedAssetsMutex.Lock();
			loadedAssetsByPath[path] = bundle;
			loadedAssetsByUuid[bundle->GetUuid()] = bundle;
			loadedAssetsMutex.Unlock();
		}

		return bundle;
	}

	Array<Ref<Asset>> AssetManager::LoadAssetsAtPath(const Name& path, SubClass<Asset> classType)
	{
		ZoneScoped;
		ZoneTextF(path.GetCString());

		Array<AssetData*> assetDatas = GetAssetsDataAtPath(path);
		if (assetDatas.IsEmpty())
			return {};

		Ref<Bundle> bundle = nullptr;

		loadedAssetsMutex.Lock();
		if (loadedAssetsByPath.KeyExists(path) && loadedAssetsByPath[path] != nullptr)
		{
			bundle = loadedAssetsByPath[path];
			loadedAssetsMutex.Unlock();
		}
		else
		{
			LoadBundleArgs args{
				.loadFully = true,
				.forceReload = false
			};

			bundle = Bundle::LoadBundle(this, path, args);
			if (bundle == nullptr)
			{
				loadedAssetsMutex.Unlock();
				return {};
			}
			loadedAssetsByPath[path] = bundle;
			loadedAssetsByUuid[bundle->GetUuid()] = bundle;
			loadedAssetsMutex.Unlock();
		}
		
		if (!bundle)
			return {};

		Array<Ref<Asset>> assets{};

		if (classType == nullptr || !classType->IsSubclassOf<Asset>())
			classType = Asset::StaticType();

		for (auto assetData : assetDatas)
		{
			Ref<Object> object = bundle->LoadObject(assetData->assetUuid);
			if (object.IsValid() && object->IsOfType(classType))
			{
				assets.Add((Ref<Asset>)object);
			}
		}

		return assets;
	}

	void AssetManager::UnloadAsset(Asset* asset)
	{
		ZoneScoped;

		if (asset == nullptr)
			return;

		Ref<Bundle> bundle = asset->GetBundle();
		if (bundle == nullptr)
			return;

		LockGuard lock{ loadedAssetsMutex };

		loadedAssetsByPath.Remove(bundle->GetBundlePath());
		loadedAssetsByUuid.Remove(bundle->GetUuid());
		bundle->BeginDestroy();
	}

	void AssetManager::UnloadAsset(Ref<Bundle> bundle)
	{
		ZoneScoped;

		if (bundle == nullptr)
			return;

		for (IAssetRegistryListener* listener : assetRegistry->listeners)
		{
			if (listener != nullptr)
			{
				listener->OnAssetUnloaded(bundle->GetUuid());
			}
		}

		LockGuard lock{ loadedAssetsMutex };

		loadedAssetsByPath.Remove(bundle->GetBundlePath());
		loadedAssetsByUuid.Remove(bundle->GetUuid());
		bundle->BeginDestroy();
	}

	Ref<Asset> AssetManager::LoadAssetAtPath(const Name& path)
	{
		ZoneScoped;
		ZoneTextF(path.GetCString());

		AssetData* assetData = GetPrimaryAssetDataAtPath(path);
		if (!assetData)
			return nullptr;
		
		Ref<Bundle> bundle = nullptr;
		
		{
			LockGuard lock{ loadedAssetsMutex };
			
			if (loadedAssetsByPath.KeyExists(path) && loadedAssetsByPath[path] != nullptr)
			{
				bundle = loadedAssetsByPath[path];
			}
			else
			{
				LoadBundleArgs args{
					.loadFully = true,
					.forceReload = true,
					.destroyOutdatedObjects = true
				};

				bundle = Bundle::LoadBundle(this, path, args);
				if (bundle == nullptr)
				{
					return nullptr;
				}
				loadedAssetsByPath[path] = bundle;
				loadedAssetsByUuid[bundle->GetUuid()] = bundle;
			}
		}
		
		if (!bundle)
			return nullptr;

		auto primaryObject = bundle->GetPrimaryObjectData();

		Ref<Object> object = bundle->LoadObject(primaryObject.uuid);
		if (!object || !object->IsOfType<Asset>())
			return nullptr;
		
		return (Ref<Asset>)object;
	}

	RHI::Texture* AssetManager::LoadTextureAtPath(const Name& path)
	{
		Ref<CE::Texture2D> texture = LoadAssetAtPath<CE::Texture2D>(path);
		if (!texture)
			return nullptr;

		RPI::Texture* rpiTexture = texture->GetRpiTexture();
		if (!rpiTexture)
			return nullptr;

		return rpiTexture->GetRhiTexture();
	}

	FusionRawImageData AssetManager::LoadRawTextureAtPath(const Name& path)
	{
		if (FusionApplication::TryGet() == nullptr)
			return {};

		if (!path.GetString().StartsWith("/Temp"))
			return {};

		if (loadedImageViewsByPath.KeyExists(path))
		{
			if (RHI::TextureView* view = loadedImageViewsByPath[path])
			{
				return { view, loadedImageSrgsByPath[path] };
			}
		}
		
		Ref<Bundle> textureBundle = nullptr;

		if (loadedTempBundlesByPath.KeyExists(path) && loadedTempBundlesByPath[path] != nullptr)
		{
			textureBundle = loadedTempBundlesByPath[path];
		}
		else
		{
			LoadBundleArgs args{
				.loadTemporary = true,
				.loadFully = true,
				.forceReload = false,
				.destroyOutdatedObjects = false
			};

			textureBundle = Bundle::LoadBundle(this, path, args);
			if (textureBundle == nullptr)
				return {};
			loadedTempBundlesByPath[path] = textureBundle;
		}

		if (!textureBundle)
			return {};

		Ref<CE::Texture2D> texture = textureBundle->LoadObject<CE::Texture2D>();
		if (!texture)
			return {};

		RPI::Texture* rpiTexture = texture->GetRpiTexture();
		if (!rpiTexture)
			return {};

		RHI::Texture* rhiTexture = rpiTexture->GetRhiTexture();
		if (!rhiTexture)
			return {};

		RHI::TextureViewDescriptor viewDesc{};
		viewDesc.texture = rhiTexture;
		viewDesc.format = rhiTexture->GetFormat();
		viewDesc.dimension = Dimension::Dim2DArray;
		viewDesc.baseArrayLayer = 0;
		viewDesc.arrayLayerCount = 1;
		viewDesc.baseMipLevel = 0;
		viewDesc.mipLevelCount = 1;

		RHI::TextureView* textureView = RHI::gDynamicRHI->CreateTextureView(viewDesc);
		if (!textureView)
			return {};

		RHI::ShaderResourceGroupLayout textureSrgLayout = FusionApplication::Get()->GetFusionShader2()->GetDefaultVariant()->GetSrgLayout(SRGType::PerDraw);
		RHI::ShaderResourceGroup* textureSrg = RHI::gDynamicRHI->CreateShaderResourceGroup(textureSrgLayout);
		if (!textureSrg)
		{
			RHI::gDynamicRHI->DestroyTextureView(textureView);
			return {};
		}

		RHI::SamplerDescriptor samplerDesc{};
		samplerDesc.addressModeU = SamplerAddressMode::ClampToBorder;
		samplerDesc.addressModeV = SamplerAddressMode::ClampToBorder;
		samplerDesc.addressModeW = SamplerAddressMode::ClampToBorder;
		samplerDesc.borderColor = SamplerBorderColor::FloatTransparentBlack;
		samplerDesc.enableAnisotropy = false;
		samplerDesc.samplerFilterMode = FilterMode::Linear;

		textureSrg->Bind("_Texture", textureView);
		textureSrg->Bind("_TextureSampler", RPISystem::Get().FindOrCreateSampler(samplerDesc));
		textureSrg->FlushBindings();

		loadedImageViewsByPath[path] = textureView;
		loadedImageSrgsByPath[path] = textureSrg;

		return { textureView, textureSrg };
	}

	CMImage AssetManager::LoadImageAssetAtPath(const Name& path)
	{
		Ref<CE::Texture2D> texture = LoadAssetAtPath<CE::Texture2D>(path);
		if (!texture)
			return {};

		return texture->GetCMImage();
	}
} // namespace CE
