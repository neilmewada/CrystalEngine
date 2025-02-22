
#include "CoreApplication.h"

#include <CoreGraphics/CoreGraphics.h>
#include <AppKit/AppKit.h>
#include <ApplicationServices/ApplicationServices.h>

namespace CE
{
    MacSDLApplication* MacSDLApplication::Create()
    {
        return new MacSDLApplication();
    }

    void MacSDLApplication::Initialize()
    {
        [[NSUserDefaults standardUserDefaults] setBool: YES
            forKey: @"AppleMomentumScrollSupported"];

        Super::Initialize();

    }

    u32 MacSDLApplication::GetSystemDpi()
    {
        NSScreen *mainScreen = [NSScreen mainScreen];

        // Get the scale factor (Retina scaling)
        CGFloat scale = [mainScreen backingScaleFactor];
        //std::cout << "Scale Factor (Retina): " << scale << std::endl;

        return (u32)(scale * 72.0f);
    }

    MacSDLApplication::MacSDLApplication()
    {

    }

    int MacSDLApplication::GetCurrentDisplayIndex()
    {
        // Get the window info list for the frontmost window
        CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
        if (!windowList) {
            std::cerr << "Failed to get window list!" << std::endl;
            return 0;
        }

        int displayIndex = 0;
        CGRect windowBounds = {};
        CFDictionaryRef windowInfo = nullptr;

        // Iterate through the window list
        for (CFIndex i = 0; i < CFArrayGetCount(windowList); i++) {
            CFDictionaryRef windowDict = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
            CFNumberRef layer = (CFNumberRef)CFDictionaryGetValue(windowDict, kCGWindowLayer);
            
            int layerValue;
            CFNumberGetValue(layer, kCFNumberIntType, &layerValue);
            
            // The active window is usually in layer 0
            if (layerValue == 0) {
                windowInfo = windowDict;
                break;
            }
        }

        if (windowInfo) {
            CFDictionaryRef boundsDict = (CFDictionaryRef)CFDictionaryGetValue(windowInfo, kCGWindowBounds);
            if (boundsDict) {
                CGRectMakeWithDictionaryRepresentation(boundsDict, &windowBounds);
            }
        }

        CFRelease(windowList);

        if (CGRectIsEmpty(windowBounds)) {
            std::cerr << "Failed to get focused window bounds!" << std::endl;
            return 0;
        }

        // Get active display list
        uint32_t displayCount = 0;
        CGGetActiveDisplayList(0, nullptr, &displayCount);
        
        if (displayCount == 0) {
            std::cerr << "No active displays found!" << std::endl;
            return 0;
        }

        CGDirectDisplayID displays[displayCount];
        CGGetActiveDisplayList(displayCount, displays, &displayCount);

        // Find which display contains the window's center
        CGPoint windowCenter = { CGRectGetMidX(windowBounds), CGRectGetMidY(windowBounds) };

        for (uint32_t i = 0; i < displayCount; i++) {
            CGRect displayBounds = CGDisplayBounds(displays[i]);
            if (CGRectContainsPoint(displayBounds, windowCenter)) {
                displayIndex = i;
                break;
            }
        }

        return displayIndex;
    }


    int SDLWindowEventWatch(void* data, SDL_Event* event)
    {
        
        return 0;
    }

}
