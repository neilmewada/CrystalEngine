#pragma once

#if PLATFORM_MAC

#include "CoreTypes.h"
#include "IO/Path.h"

namespace CE
{
    
    class CORE_API MacMisc : public PlatformMiscBase
    {
    public:
        CE_STATIC_CLASS(MacMisc);

        inline static String GetPlatformName()
        {
            return "Mac";
        }

        inline static PlatformName GetCurrentPlatform()
        {
            return PlatformName::Mac;
        }

        static String GetOSVersionString();

        static String GetSystemFileExplorerDisplayName();
    };

    typedef MacMisc PlatformMisc;

} // namespace CE


#endif
