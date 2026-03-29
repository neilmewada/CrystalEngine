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

        imageAtlas->Shutdown();
    }

    void FImageService::TickService(FServiceTickPhase tickPhase)
    {
        auto renderService = FApplication::Get()->GetService<FRenderService>();

        if (tickPhase == FServiceTickPhase::Render)
        {
			u32 frameIndex = renderService->GetCurrentFrameIndex();

            imageAtlas->Flush(frameIndex);
        }
    }

    FResolvedImage FImageService::ResolveImage(const Name& imageName)
    {
		String imagePath = imageName.GetString();

        auto imageItem = imageAtlas->FindImage(imageName);

        if (imageItem.IsValid())
        {
            return FResolvedImage{ imageItem.textureSlot, imageItem.uvMin, imageItem.uvMax };
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

} // namespace CE

