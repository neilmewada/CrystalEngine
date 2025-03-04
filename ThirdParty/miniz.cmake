
# this file actually ingests the library and defines targets.
set(TARGET_WITH_NAMESPACE "ThirdParty::miniz")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "miniz")
set(LIB_NAME "miniz")

add_subdirectory(vendor/${LIB_NAME})

add_library(${TARGET_WITH_NAMESPACE} ALIAS miniz)

set_target_properties(miniz PROPERTIES FOLDER "ThirdParty")


#[[

# this file actually ingests the library and defines targets.
set(TARGET_WITH_NAMESPACE "ThirdParty::minizip")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(PACKAGE_NAME "minizip")
set(PACKAGE_VERISON "3.0.9")
set(PACKAGE_REVISION 1)

set(PACKAGE_FULL_NAME "${PACKAGE_NAME}-${PACKAGE_VERISON}-rev${PACKAGE_REVISION}-${PAL_PLATFORM_NAME_LOWERCASE}")

ce_validate_package(${PACKAGE_FULL_NAME} ${PACKAGE_NAME})

set(LIB_NAME "libminizip")

set(${LIB_NAME}_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/${PACKAGE_FULL_NAME})

set(${LIB_NAME}_INCLUDE_DIR ${${LIB_NAME}_BASE_DIR}/minizip)
set(${LIB_NAME}_LIBS_DIR ${${LIB_NAME}_BASE_DIR}/minizip)

if (${PAL_PLATFORM_NAME} STREQUAL "Mac")
    set(${LIB_NAME}_STATIC_LIBRARY_DEBUG   ${${LIB_NAME}_LIBS_DIR}/Debug/${LIB_NAME}.a)
    set(${LIB_NAME}_STATIC_LIBRARY_DEV     ${${LIB_NAME}_LIBS_DIR}/Development/${LIB_NAME}.a)
    set(${LIB_NAME}_STATIC_LIBRARY_RELEASE ${${LIB_NAME}_LIBS_DIR}/Release/${LIB_NAME}.a)

    # libz
    set(${LIB_NAME}_STATIC_LIBRARY_DEBUG   ${${LIB_NAME}_LIBS_DIR}/Debug/libz.lib)
    set(${LIB_NAME}_STATIC_LIBRARY_DEV     ${${LIB_NAME}_LIBS_DIR}/Development/libz.lib)
    set(${LIB_NAME}_STATIC_LIBRARY_RELEASE ${${LIB_NAME}_LIBS_DIR}/Release/libz.lib)

elseif (${PAL_PLATFORM_NAME} STREQUAL "Windows")
    # minizip
    set(${LIB_NAME}_STATIC_LIBRARY_DEBUG   ${${LIB_NAME}_LIBS_DIR}/Debug/${LIB_NAME}.lib)
    set(${LIB_NAME}_STATIC_LIBRARY_DEV     ${${LIB_NAME}_LIBS_DIR}/Development/${LIB_NAME}.lib)
    set(${LIB_NAME}_STATIC_LIBRARY_RELEASE ${${LIB_NAME}_LIBS_DIR}/Release/${LIB_NAME}.lib)

    # zlibstatic
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_DEBUG   ${${LIB_NAME}_LIBS_DIR}/Debug/zlibstaticd.lib)
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_DEV     ${${LIB_NAME}_LIBS_DIR}/Development/zlibstatic.lib)
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_RELEASE ${${LIB_NAME}_LIBS_DIR}/Release/zlibstatic.lib)

    # zstd
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_DEBUG   ${${LIB_NAME}_LIBS_DIR}/Debug/zstd_static.lib)
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_DEV     ${${LIB_NAME}_LIBS_DIR}/Development/zstd_static.lib)
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_RELEASE ${${LIB_NAME}_LIBS_DIR}/Release/zstd_static.lib)

    # lzma
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_DEBUG   ${${LIB_NAME}_LIBS_DIR}/Debug/liblzma.lib)
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_DEV     ${${LIB_NAME}_LIBS_DIR}/Development/liblzma.lib)
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_RELEASE ${${LIB_NAME}_LIBS_DIR}/Release/liblzma.lib)

    # bzip2
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_DEBUG   ${${LIB_NAME}_LIBS_DIR}/Debug/bzip2.lib)
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_DEV     ${${LIB_NAME}_LIBS_DIR}/Development/bzip2.lib)
    list(APPEND ${LIB_NAME}_STATIC_LIBRARY_RELEASE ${${LIB_NAME}_LIBS_DIR}/Release/bzip2.lib)

else()
    error("${PACKAGE_NAME} build not found for platform: ${PAL_PLATFORM_NAME}")
endif()

add_library(${TARGET_WITH_NAMESPACE} STATIC IMPORTED GLOBAL)

set_target_properties(${TARGET_WITH_NAMESPACE}
    PROPERTIES
        IMPORTED_LOCATION_DEBUG       ${${LIB_NAME}_STATIC_LIBRARY_DEBUG}
        IMPORTED_LOCATION_DEVELOPMENT ${${LIB_NAME}_STATIC_LIBRARY_DEV}
        IMPORTED_LOCATION_PROFILE     ${${LIB_NAME}_STATIC_LIBRARY_DEV}
        IMPORTED_LOCATION_RELEASE     ${${LIB_NAME}_STATIC_LIBRARY_RELEASE}
)

if(${PAL_PLATFORM_NAME} STREQUAL "Mac")
    set_target_properties(${TARGET_WITH_NAMESPACE}
        PROPERTIES
            IMPORTED_LOCATION 
                "${${LIB_NAME}_LIBS_DIR}/$<IF:$<CONFIG:Development,Profile>,Development,$<CONFIG>>/lib${LIB_NAME}.a"
                "${${LIB_NAME}_LIBS_DIR}/$<IF:$<CONFIG:Development,Profile>,Development,$<CONFIG>>/libz.a"
                "${${LIB_NAME}_LIBS_DIR}/$<IF:$<CONFIG:Development,Profile>,Development,$<CONFIG>>/liblzma.a"
                "${${LIB_NAME}_LIBS_DIR}/$<IF:$<CONFIG:Development,Profile>,Development,$<CONFIG>>/libbzip2.a"
                "${${LIB_NAME}_LIBS_DIR}/$<IF:$<CONFIG:Development,Profile>,Development,$<CONFIG>>/libzstd.a"
    )
endif()


target_include_directories(${TARGET_WITH_NAMESPACE}
    INTERFACE
        ${${LIB_NAME}_INCLUDE_DIR}
)

set(${LIB_NAME}_FOUND True)

]]
