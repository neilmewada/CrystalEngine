
# this file actually ingests the library and defines targets.
set(TARGET_WITH_NAMESPACE "ThirdParty::spdlog")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "spdlog")
set(LIB_NAME "spdlog")

set(SPDLOG_BUILD_SHARED OFF)

add_subdirectory(vendor/${LIB_NAME})

add_library(${TARGET_WITH_NAMESPACE} ALIAS spdlog)

set_target_properties(spdlog PROPERTIES FOLDER "ThirdParty")


