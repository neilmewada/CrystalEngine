
include_guard(GLOBAL)

# file(GLOB THIRD_PARTY_CMAKE_FILES ${CE_ROOT_DIR}/ThirdParty/*${PAL_PLATFORM_NAME_LOWERCASE}*/Find*.cmake)

# foreach(THIRD_PARTY_CMAKE_FILE ${THIRD_PARTY_CMAKE_FILES})
#     get_filename_component(THIRD_PARTY_CMAKE_FILE_DIRECTORY ${THIRD_PARTY_CMAKE_FILE} DIRECTORY)
#     list(APPEND CMAKE_MODULE_PATH ${THIRD_PARTY_CMAKE_FILE_DIRECTORY})
# endforeach()

list(APPEND CMAKE_MODULE_PATH ${CE_ROOT_DIR}/ThirdParty)

set(FETCHCONTENT_QUIET FALSE)

# \arg:TARGET_TYPE: SHARED; STATIC; MODULE; 
function(ce_validate_package PACKAGE_FULL_NAME PACKAGE_SHORT_NAME)
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${PACKAGE_FULL_NAME}")
        return()
    endif()
    
    set(package_url "https://gitlab.com/neelrmewada/crystalengine-thirdparty/-/raw/main/ThirdParty/${PAL_PLATFORM_NAME}/${PACKAGE_FULL_NAME}.zip?inline=false")
    # set(package_url "https://github.com/neelmewada/CrystalEngine-ThirdParty/blob/main/ThirdParty/${PAL_PLATFORM_NAME}/${PACKAGE_FULL_NAME}.zip?raw=true")
    set(package_local_zip_path "${CMAKE_CURRENT_LIST_DIR}/${PACKAGE_FULL_NAME}.zip")

    if(NOT EXISTS "${package_local_zip_path}")
        message("***********************************************************************************")

        message(STATUS "Downloading Package: ${PACKAGE_SHORT_NAME}")

        file(DOWNLOAD ${package_url} "${package_local_zip_path}" STATUS download_status SHOW_PROGRESS)
        
        list(GET download_status 0 status_code)
        list(GET download_status 1 status_code_msg)
        
        if(NOT ${status_code} EQUAL 0)
            message(SEND_ERROR "Download Failed! Error code ${status_code}. Error Message: ${status_code_msg}\nDownload URL: ${package_url}")
            file(REMOVE ${package_local_zip_path})
            return()
        endif()
        
        file(ARCHIVE_EXTRACT INPUT ${package_local_zip_path} DESTINATION "${CMAKE_CURRENT_LIST_DIR}" VERBOSE)
        file(REMOVE ${package_local_zip_path})

        message("***********************************************************************************")
    endif()
endfunction()


# function to setup a runtime dependency target
function(ce_add_rt_deps NAME)
    set(target "${NAME}_RT")

    set(options AUTORTTI)
    set(oneValueArgs OUTPUT_SUBDIRECTORY FOLDER ROOT_PATH MAC_ROOT_PATH LINUX_ROOT_PATH)
    set(multiValueArgs COPY_DIRS COPY_FILES COPY_LIBS INCLUDE_DIRECTORIES BIN_DIRS)

    cmake_parse_arguments(ce_add_rt_deps "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_target(${target})

    if(NOT ce_add_rt_deps_ROOT_PATH)
        message(FATAL_ERROR "ROOT_PATH not supplied to ce_add_rt_deps ${NAME}")
    endif()

    if(ce_add_rt_deps_MAC_ROOT_PATH AND PAL_PLATFORM_IS_MAC)
        set(ce_add_rt_deps_ROOT_PATH "${ce_add_rt_deps_MAC_ROOT_PATH}")
    endif()

    if (PAL_PLATFORM_IS_LINUX AND ce_add_rt_deps_LINUX_ROOT_PATH)
        set(ce_add_rt_deps_ROOT_PATH "${ce_add_rt_deps_LINUX_ROOT_PATH}")
    endif()

    set_target_properties(${target} PROPERTIES ROOT_PATH "${ce_add_rt_deps_ROOT_PATH}")

    if(ce_add_rt_deps_COPY_DIRS)
        set_target_properties(${target} PROPERTIES COPY_DIRS "${ce_add_rt_deps_COPY_DIRS}")
    endif()
    
    if(ce_add_rt_deps_BIN_DIRS)
        set_target_properties(${target} PROPERTIES BIN_DIRS "${ce_add_rt_deps_BIN_DIRS}")
    endif()
    
    if(ce_add_rt_deps_COPY_LIBS)
        set_target_properties(${target} PROPERTIES COPY_LIBS "${ce_add_rt_deps_COPY_LIBS}")
    endif()

    if(ce_add_rt_deps_COPY_FILES)
        set_target_properties(${target} PROPERTIES COPY_FILES "${ce_add_rt_deps_COPY_FILES}")
    endif()
    
    set_target_properties(${target} 
        PROPERTIES
            FOLDER "Dependencies"
    )
    
endfunction()


