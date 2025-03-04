
# this file actually ingests the library and defines targets.
set(TARGET_WITH_NAMESPACE "ThirdParty::zip")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "zip")
set(LIB_NAME "zip")

add_subdirectory(vendor/${PACKAGE_NAME})

add_library(${TARGET_WITH_NAMESPACE} ALIAS zip)

set_target_properties(zip PROPERTIES FOLDER "ThirdParty")
set_target_properties(uninstall PROPERTIES FOLDER "ThirdParty")


