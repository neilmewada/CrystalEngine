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
        thumbnailTexture->SetColorSpace(TextureColorSpace::None);

        BundleSaveResult result = Bundle::SaveToDisk(thumbnailAsset, nullptr, thumbnailPath);

        return result == BundleSaveResult::Success;
    }

    void AssetThumbnailGen::SaveThumbnailToDisk(void* rgbaImageData, u32 width, u32 height, const Name& assetPath,
	    bool compress)
    {
        CMImage image = CMImage::LoadRawImageFromMemory((unsigned char*)rgbaImageData, width, height,
            CMImageFormat::RGBA8, CMImageSourceFormat::None,
            8, 8 * 4);

        CMImageEncoder encoder{};

        u64 size = encoder.GetCompressedSizeRequirement(image, CMImageSourceFormat::BC7);
        if (!compress || size == 0)
        {
            SaveThumbnailToDisk(image, assetPath);
        }
        else
        {
            u8* compressedData = new u8[size];
            if (encoder.EncodeToBCn(image, compressedData, CMImageSourceFormat::BC7))
            {
                CMImage compressedImage = CMImage::LoadRawImageFromMemory(compressedData, width, height,
                    CMImageFormat::BC7, CMImageSourceFormat::None, 8 / 4, 8);
                SaveThumbnailToDisk(compressedImage, assetPath);
            }
            else
            {
                SaveThumbnailToDisk(image, assetPath);
            }
            delete[] compressedData;
        }
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

