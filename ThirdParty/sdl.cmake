
# this file actually ingests the library and defines targets.
set(TARGET_WITH_NAMESPACE "ThirdParty::sdl")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "sdl")
set(PACKAGE_VERISON "3.4.0")

set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build the SDL3_test library" FORCE)

add_subdirectory(vendor/${PACKAGE_NAME})

add_library(${TARGET_WITH_NAMESPACE} ALIAS SDL3-shared)

set_target_properties(SDL3-shared PROPERTIES FOLDER "ThirdParty")
set_target_properties(SDL_uclibc PROPERTIES FOLDER "ThirdParty")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)


if(${PAL_PLATFORM_IS_MAC})
    target_link_libraries(SDL3-shared
        INTERFACE
            "-framework Cocoa"
            "-framework CoreAudio"
            "-framework Carbon"
            "-framework AudioToolbox"
            "-framework AVFoundation"
            "-framework Metal"
            "-framework MetalKit"
            "-framework IOKit"
            "-framework CoreGraphics"
            "-framework CoreVideo"
            "-framework CoreHaptics"
            "-framework GameController"
            "-framework ForceFeedback"
            "iconv"
    )
endif()



