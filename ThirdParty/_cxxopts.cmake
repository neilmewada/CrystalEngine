
set(TARGET_WITH_NAMESPACE "ThirdParty::cxxopts")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "cxxopts")

add_subdirectory(vendor/${PACKAGE_NAME})

add_library(${TARGET_WITH_NAMESPACE} ALIAS cxxopts)

set_target_properties(cxxopts PROPERTIES FOLDER "ThirdParty")


