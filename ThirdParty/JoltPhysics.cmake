
# this file actually ingests the library and defines targets.
set(TARGET_WITH_NAMESPACE "ThirdParty::JoltPhysics")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "JoltPhysics")
set(LIB_NAME "Jolt")


set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "Use static MSVC runtime library for JoltPhysics" FORCE)
add_subdirectory(vendor/${PACKAGE_NAME}/Build)

add_library(${TARGET_WITH_NAMESPACE} ALIAS ${LIB_NAME})

set_target_properties(${LIB_NAME} PROPERTIES FOLDER "ThirdParty")
