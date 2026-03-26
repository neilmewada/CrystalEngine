#pragma once

#include "Core.h"

// Media
#include "CoreAssets/Image.h"
#include "CoreAssets/CubeMap.h"

#include "CoreAssets/ImageEncoder.h"

#include "CoreAssets/Font.h"

namespace CE
{
    class COREASSETS_API CoreAssetsModule : public CE::Module
    {
    public:
        virtual void StartupModule() override;

        virtual void ShutdownModule() override;

        virtual void RegisterTypes() override;
    };
}