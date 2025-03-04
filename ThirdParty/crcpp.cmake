
set(TARGET_WITH_NAMESPACE "ThirdParty::crcpp")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "CRCpp")
set(LIB_NAME "crcpp")

set(BUILD_TEST OFF  CACHE BOOL "" FORCE)

add_subdirectory(vendor/${PACKAGE_NAME})

add_library(${TARGET_WITH_NAMESPACE} ALIAS CRCpp)

set_target_properties(CRCpp PROPERTIES FOLDER "ThirdParty")



