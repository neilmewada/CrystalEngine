#pragma once

namespace CE
{
	struct IThumbnailSystemListener
	{
		virtual ~IThumbnailSystemListener() = default;

		virtual void OnThumbnailsUpdated(const Array<Name>& assetPaths) {}
	};

    CLASS()
    class EDITORCORE_API ThumbnailSystem : public Object, IAssetRegistryListener
    {
        CE_CLASS(ThumbnailSystem, Object)
    protected:

        ThumbnailSystem();

        void OnBeforeDestroy() override;

        void OnAssetImported(const Name& bundlePath, const Name& sourcePath) override;

        void OnAssetRenamed(Uuid bundleUuid, const Name& oldName, const Name& newName, const Name& newPath) override;

        void MarkAssetDirty(const Name& bundlePath);
        
    public:

        virtual ~ThumbnailSystem();

		static Ref<ThumbnailSystem> Get();

        void Initialize();
		void Shutdown();

        void Tick(f32 deltaTime);

        void AddThumbnailListener(IThumbnailSystemListener* listener);
		void RemoveThumbnailListener(IThumbnailSystemListener* listener);

		static Name GetThumbnailPath(Name assetPath);

    private:

        void OnThumbnailFinished(Ref<AssetThumbnailGen> thumbnailGen);

		//! @brief Path to assets that are dirty (need thumbnail update), mapped to their thumbnail generation classes.
        HashMap<TypeId, Array<Name>> dirtyAssetsByThumbnailGenClass;

		Array<Ref<AssetThumbnailGen>> thumbnailGenerators;

        Array<IThumbnailSystemListener*> thumbnailListeners;
    };
    
} // namespace CE

#include "ThumbnailSystem.rtti.h"
