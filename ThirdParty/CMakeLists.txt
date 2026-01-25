cmake_minimum_required(VERSION 3.20)

project(ThirdParty)

include(xxHash.cmake)
include(spdlog.cmake)
include(_cxxopts.cmake)
include(efsw.cmake)
include(zip.cmake)
include(crcpp.cmake)
include(stb.cmake)
include(miniz.cmake)
include(freetype.cmake)
include(assimp.cmake)
include(sdl.cmake)
include(cpptrace.cmake)
include(Findispctexturecompressor.cmake)
include(tracy.cmake)
include(JoltPhysics.cmake)

# Linux packages
# libxxhash-dev 


if(${PAL_PLATFORM_IS_WINDOWS})
    find_library(Vulkan_dxc_DEBUG_LIBRARY
        NAMES dxcompilerd
        HINTS
        ${_Vulkan_hint_library_search_paths})

    mark_as_advanced(Vulkan_dxc_DEBUG_LIBRARY)

    cmake_path(GET Vulkan_dxc_LIBRARY PARENT_PATH Vulkan_dxc_DEBUG_LIBRARY)
    set(Vulkan_dxc_DEBUG_LIBRARY "${Vulkan_dxc_DEBUG_LIBRARY}/dxcompilerd.lib")

    set_property(TARGET Vulkan::dxc_lib APPEND
        PROPERTY
        IMPORTED_CONFIGURATIONS Debug)
    set_property(TARGET Vulkan::dxc_lib
        PROPERTY
        IMPORTED_LOCATION_DEBUG "${Vulkan_dxc_DEBUG_LIBRARY}")

    message(STATUS "Vulkan DXC Debug: ${Vulkan_dxc_DEBUG_LIBRARY}")
endif()


