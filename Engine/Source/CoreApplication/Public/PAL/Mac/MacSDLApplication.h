#pragma once

#include "PAL/SDL/SDLApplication.h"

namespace CE
{

    class COREAPPLICATION_API MacSDLApplication final : public SDLApplication
    {
    public:
        typedef SDLApplication Super;

        static MacSDLApplication* Create();

        void Initialize() override;

        u32 GetSystemDpi() override;
        
        int GetCurrentDisplayIndex() override;

    private:

        MacSDLApplication();

    };

    typedef MacSDLApplication PlatformApplicationImpl;
    
}
