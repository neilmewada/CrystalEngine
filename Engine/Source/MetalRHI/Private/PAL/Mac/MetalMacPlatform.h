#pragma once


namespace CE::Metal
{
    class MetalMacPlatform : public MetalPlatformBase
    {
        CE_STATIC_CLASS(MetalMacPlatform)
    public:

        static void GetNativeWindowSize(void* nativeWindowHandle, u32* width, u32* height)
        {
            
        }
    };
    
    typedef MetalMacPlatform MetalOSPlatform;

} // namespace CE
