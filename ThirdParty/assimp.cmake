
# this file actually ingests the library and defines targets.
set(TARGET_WITH_NAMESPACE "ThirdParty::assimp")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "assimp")

set(ASSIMP_BUILD_TESTS OFF  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)

if(PAL_PLATFORM_IS_MAC)
    add_compile_options(-Wno-conversion)
endif()


add_subdirectory(vendor/${PACKAGE_NAME})

add_library(${TARGET_WITH_NAMESPACE} ALIAS assimp)

set_target_properties(assimp PROPERTIES FOLDER "ThirdParty")

if(TARGET zlibstatic)
    set_target_properties(zlibstatic PROPERTIES FOLDER "ThirdParty")
endif()

if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "ThirdParty")
endif()


