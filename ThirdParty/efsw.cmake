
# this file actually ingests the library and defines targets.
set(TARGET_WITH_NAMESPACE "ThirdParty::efsw")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "efsw")
set(LIB_NAME "efsw")

add_subdirectory(vendor/${PACKAGE_NAME})

add_library(${TARGET_WITH_NAMESPACE} ALIAS efsw)

set_target_properties(efsw PROPERTIES FOLDER "ThirdParty")
set_target_properties(efsw-static PROPERTIES FOLDER "ThirdParty")



