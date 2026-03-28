#pragma once

namespace CE
{
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
