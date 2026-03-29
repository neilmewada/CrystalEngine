#pragma once

namespace CE
{
    struct FResolvedImage
    {
        int textureSlot = -1;
        Vec2 uvMin = Vec2(0, 0);
    	Vec2 uvMax = Vec2(1, 1);

        bool IsValid() const
        {
			return textureSlot >= 0;
        }
    };

    CLASS(Config = Engine)
    class FUSIONCORE_API FImageService : public FService
    {
        CE_CLASS(FImageService, FService)
    protected:

        FImageService();

        void OnStart() override;

        void OnShutdown() override;

        void TickService(FServiceTickPhase tickPhase) override;
        
    public:

        // - Public API -

        FResolvedImage ResolveImage(const Name& imageName);

    private:

        // - Internal Methods -

        void LoadImageResources();

    protected:

        // - Config -

        FIELD(Config)
        u32 atlasMaxIconSize = 512;

        // - Atlas -

        FIELD()
        Ref<FImageAtlas> imageAtlas;

    };
    
} // namespace CE

#include "FImageService.rtti.h"
