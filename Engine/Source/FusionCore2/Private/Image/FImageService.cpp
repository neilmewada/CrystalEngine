#include "FusionCore.h"

namespace CE
{

    FImageService::FImageService()
    {

    }

    void FImageService::OnStart()
    {
	    Super::OnStart();

        imageAtlas = CreateObject<FImageAtlas>(this, "ImageAtlas");

        imageAtlas->Init();

        LoadImageResources();
    }

    void FImageService::OnShutdown()
    {
	    Super::OnShutdown();

        for (RPI::Texture* loadedImage : loadedImages)
        {
            delete loadedImage;
        }
        loadedImages.Clear();

        imageAtlas->Shutdown();
    }

    void FImageService::TickService(FServiceTickPhase tickPhase)
    {
        ZoneScoped;

        auto renderService = FApplication::Get()->GetService<FRenderService>();

        if (tickPhase == FServiceTickPhase::Render)
        {
			u32 frameIndex = renderService->GetCurrentFrameIndex();

            imageAtlas->Flush(frameIndex);
        }
    }

    FResolvedImage FImageService::ResolveImage(const Name& imageName, bool tiled)
    {
        ZoneScoped;

		String imagePath = imageName.GetString();

		if (imagesByName.KeyExists(imageName))
        {
            return imagesByName[imageName];
        }

        auto imageItem = imageAtlas->FindImage(imageName);

        if (imageItem.IsValid())
        {
            if (!tiled)
            {
	            return FResolvedImage{ imageItem.textureSlot, imageItem.uvMin, imageItem.uvMax, imageItem.width, imageItem.height };
            }
            else
            {
				// Do not use atlas for tiled images, as they will be rendered with UVs outside the 0-1 range, which can cause issues with texture filtering and wrapping.
                return LoadImageResourceInSlot(imageName);
            }
        }

        return {};
    }

    void FImageService::LoadImageResources()
    {
        IO::Path engineResourcesDir = PlatformDirectories::GetLaunchDir() / "Engine/Resources";

        if (engineResourcesDir.Exists())
        {
            engineResourcesDir.RecursivelyIterateChildren([&](const IO::Path& path)
            {
                if (path.IsDirectory())
                    return;

                if (path.GetExtension() == ".png" || path.GetExtension() == ".jpg" || path.GetExtension() == ".jpeg")
                {
                    CAImage image = CAImage::LoadFromFile(path);

                    String pathStr = IO::Path::GetRelative(path, engineResourcesDir).GetString().Replace({ '\\' }, '/');

                    if (image.IsValid())
                    {
                        imageAtlas->AddImage(String::Format("res:/{}", pathStr), image);

                        image.Free();
                    }
                }
            });
        }
    }

    FResolvedImage FImageService::LoadImageResourceInSlot(const Name& imageName)
    {
        ZoneScoped;

		auto renderService = FApplication::Get()->GetService<FRenderService>();

		String imagePath = imageName.GetString();

        if (!imagePath.StartsWith("res:/"))
            return {};

		String relativePath = imagePath.GetSubstring(5);

		IO::Path path = PlatformDirectories::GetLaunchDir() / "Engine/Resources" / relativePath;

		if (!path.Exists())
            return {};

        if (path.GetExtension() == ".png" || path.GetExtension() == ".jpg" || path.GetExtension() == ".jpeg")
        {
            CAImage image = CAImage::LoadFromFile(path);

			String pathStr = IO::Path::GetRelative(path, PlatformDirectories::GetLaunchDir()).GetString().Replace({ '\\' }, '/');

            RPI::Texture* texture = new RPI::Texture(image);

            int textureSlot = renderService->RegisterTexture(texture->GetRhiTexture());

			imagesByName[imageName] = FResolvedImage{ textureSlot, Vec2(0, 0), Vec2(1, 1), image.GetWidth(), image.GetHeight() };

			loadedImages.Add(texture);

			image.Free();

			return imagesByName[imageName];
        }

        return {};
    }
} // namespace CE

