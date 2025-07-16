#pragma once

namespace CE
{
    CLASS()
    class EDITORCORE_API ThumbnailSystem : public Object, IAssetRegistryListener
    {
        CE_CLASS(ThumbnailSystem, Object)
    protected:

        ThumbnailSystem();

        void OnBeforeDestroy() override;
        
    public:

        virtual ~ThumbnailSystem();

		static Ref<ThumbnailSystem> Get();

        void Initialize();
		void Shutdown();

        void Tick(f32 deltaTime);

    private:

        void OnThumbnailFinished(Ref<AssetThumbnailGen> thumbnailGen);

		//! @brief Path to assets that are dirty (need thumbnail update), mapped to their thumbnail generation classes.
        HashMap<TypeId, Array<Name>> dirtyAssetsByThumbnailGenClass;

		Array<Ref<AssetThumbnailGen>> thumbnailGenerators;
    };
    
} // namespace CE

#include "ThumbnailSystem.rtti.h"
