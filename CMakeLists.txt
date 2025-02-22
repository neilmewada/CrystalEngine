cmake_minimum_required(VERSION 3.20)

set (CMAKE_CXX_STANDARD 20)

project(CrystalEngine LANGUAGES C CXX)

include(CMake/CESet.cmake)
include(CMake/Macros.cmake)
include(CMake/Version.cmake)

ce_set(CE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

################################################################################
# CMake headers
################################################################################
include(FetchContent)
include(CMake/GeneralSettings.cmake)
include(CMake/Options.cmake)
include(CMake/PAL.cmake)
include(CMake/Configurations.cmake)
include(CMake/OutputDirectory.cmake)
include(CMake/CompilerOptions.cmake)
include(CMake/Targets.cmake)
include(CMake/ThirdParty.cmake)
include(CMake/Assets.cmake)
include(CMake/ConfigIni.cmake)
include(CMake/Shader.cmake)
include(CMake/Styles.cmake)
include(CMake/Resources.cmake)

# Import 3rd Party libraries
include(CMake/ThirdPartyPackages.cmake)
add_subdirectory(ThirdParty)

# Test
include(CMake/Tests.cmake)

# Setup Config Targets
add_subdirectory(Config)

# Build Engine
add_subdirectory(Engine)

# Build Editor
if(${PAL_TRAIT_BUILD_EDITOR})
    add_subdirectory(Editor)
endif()

# Build Host Tools
#if(${PAL_TRAIT_BUILD_HOST_TOOLS})
    add_subdirectory(Tools)
#endif()

