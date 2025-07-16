#include "EditorCore.h"

namespace CE::Editor
{

    AssetThumbnailGen::AssetThumbnailGen()
    {

    }

    AssetThumbnailGen::~AssetThumbnailGen()
    {
        
    }

    Name AssetThumbnailGen::GetThumbnailPath(Name assetPath)
    {
        return "/Temp/ThumbnailCache" + assetPath.GetString();
    }

    bool AssetThumbnailGen::SaveThumbnailToDisk(const CMImage& rawImage, const Name& assetPath)
    {
		Name thumbnailPath = GetThumbnailPath(assetPath);
        String objectName = FixObjectName(assetPath.GetLastComponent());

		Ref<Bundle> thumbnailAsset = CreateObject<Bundle>(nullptr, objectName, OF_NoFlags);
        Ref<CE::Texture2D> thumbnailTexture = Texture2D::Create(thumbnailAsset, "Thumbnail", rawImage);

        BundleSaveResult result = Bundle::SaveToDisk(thumbnailAsset, nullptr, thumbnailPath);

        return result == BundleSaveResult::Success;
    }

    void AssetThumbnailGen::OnFinish()
    {
		WeakRef<Self> self = this;

        gEngine->DispatchOnMainThread([self]
            {
                if (auto selfLock = self.Lock())
                {
                    self->onFinishEvent.Broadcast(selfLock);
                }
            });
    }
} // namespace CE

