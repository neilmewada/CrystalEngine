
# this file actually ingests the library and defines targets.
set(TARGET_WITH_NAMESPACE "ThirdParty::xxHash")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "xxhash")
set(LIB_NAME "xxHash")

add_subdirectory(vendor/${LIB_NAME}/cmake_unofficial)

add_library(${TARGET_WITH_NAMESPACE} ALIAS xxhash)

set_target_properties(xxhash PROPERTIES FOLDER "ThirdParty")
set_target_properties(xxhsum PROPERTIES FOLDER "ThirdParty")

