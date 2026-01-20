
if(NOT ${PAL_TRAIT_BUILD_TESTS_SUPPORTED})
    return()
endif()

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

add_library(GoogleTest INTERFACE IMPORTED)
target_link_libraries(GoogleTest INTERFACE gtest_main)

set_target_properties(gmock PROPERTIES FOLDER "Tests")
set_target_properties(gmock_main PROPERTIES FOLDER "Tests")
set_target_properties(gtest PROPERTIES FOLDER "Tests")
set_target_properties(gtest_main PROPERTIES FOLDER "Tests")

function(ce_add_test NAME)
    if(NOT ${CE_BUILD_TESTS})
        return()
    endif()

    set(options AUTORTTI RESOURCES)
    set(oneValueArgs TARGET FOLDER)
    set(multiValueArgs SOURCES BUILD_DEPENDENCIES ASSETS)
    set(include_dirs "${CMAKE_CURRENT_SOURCE_DIR}")

    cmake_parse_arguments(ce_add_test "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(SOURCES ${ce_add_test_SOURCES})
    
    if(${ce_add_test_RESOURCES})
        file(GLOB_RECURSE RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*")
        list(APPEND SOURCES ${RESOURCE_FILES})
    endif()

    add_executable(${NAME} ${SOURCES})

    if(${PAL_PLATFORM_IS_WINDOWS})
        set_property(TARGET ${NAME} PROPERTY
            MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,Development>:ProgramDatabase>"
        )
    endif()

    set(output_asset_files "")

    foreach(asset_file ${ce_add_test_ASSETS})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${asset_file}")
            set(dest_asset_file "${CE_OUTPUT_DIR}/Tests/${ce_add_test_TARGET}/${asset_file}")
            list(APPEND output_asset_files "${dest_asset_file}")
            cmake_path(GET dest_asset_file PARENT_PATH dest_asset_file_parent)
            add_custom_command(OUTPUT ${dest_asset_file}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${asset_file}" "${dest_asset_file_parent}"
            )
        endif()
    endforeach()
    
    if(output_asset_files)
        add_custom_target(${NAME}_Assets
            DEPENDS ${output_asset_files}
            VERBATIM
        )

        set_target_properties(${NAME}_Assets
            PROPERTIES
                FOLDER "${ce_add_test_FOLDER}"
        )
        
        add_dependencies(${NAME} ${NAME}_Assets)

    endif()

    set_target_properties(${NAME} 
        PROPERTIES
            FOLDER "${ce_add_test_FOLDER}"
    )
    
    target_link_libraries(${NAME}
        PRIVATE
            #GTest::GTest
            GoogleTest
            ${ce_add_test_TARGET}
    )

    get_target_property(target_include_paths ${ce_add_test_TARGET} INCLUDE_DIRECTORIES)
    foreach(dir ${target_include_paths})
        list(APPEND include_dirs "${dir}")
    endforeach()
    
    get_target_property(target_links ${ce_add_test_TARGET} LINK_LIBRARIES)
    target_link_libraries(${NAME}
        PRIVATE
            ${target_links}
    )

    string(TOLOWER ${ce_add_test_TARGET} ce_add_test_TARGET_LOWERCASE)

    target_compile_definitions(${NAME} 
        PRIVATE
            BUNDLE_NAME="/Code/${NAME}"
            MODULE_NAME="${NAME}"
    )

    if(${PAL_PLATFORM_IS_MAC})
        target_link_libraries(${NAME} PRIVATE "c" "c++")
    endif()

    # AUTORTTI

    if(${ce_add_test_AUTORTTI})
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Generated")
        list(APPEND include_dirs "${CMAKE_CURRENT_BINARY_DIR}/Generated")

        add_custom_target(${NAME}_AutoRtti
            COMMAND "AutoRTTI" -m ${NAME} --noapi -d "${CMAKE_CURRENT_SOURCE_DIR}/" -o "${CMAKE_CURRENT_BINARY_DIR}/Generated"
            VERBATIM
        )

        set_target_properties(${NAME}_AutoRtti PROPERTIES FOLDER "AutoRTTI")

        add_dependencies(${NAME} ${NAME}_AutoRtti)

    endif()

    # RESOURCES

    if(${ce_add_test_RESOURCES})
        target_compile_definitions(${NAME} PRIVATE _RESOURCES=1)
        
        set(resource_output_files "")
        foreach(rsrc_path ${RESOURCE_FILES})
            cmake_path(RELATIVE_PATH rsrc_path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE rsrc_rel_path)
            list(APPEND resource_output_files "${CMAKE_CURRENT_BINARY_DIR}/${rsrc_rel_path}.h")
        endforeach()

        add_custom_command(OUTPUT ${resource_output_files}
            COMMAND "ResourceCompiler" -m ${NAME} -d "${CMAKE_CURRENT_SOURCE_DIR}/Resources" -o "${CMAKE_CURRENT_BINARY_DIR}/Resources"
            DEPENDS ${RESOURCE_FILES}
            VERBATIM
        )

        add_custom_target(${NAME}_Resources
            DEPENDS ${resource_output_files}
            SOURCES ${RESOURCE_FILES}
            VERBATIM
        )

        set_target_properties(${NAME}_Resources
            PROPERTIES
                FOLDER "${ce_add_test_FOLDER}"
        )

        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources")
        list(APPEND include_dirs "${CMAKE_CURRENT_BINARY_DIR}/Resources")

        add_dependencies(${NAME} ${NAME}_Resources)

    else()
        target_compile_definitions(${NAME} PRIVATE _RESOURCES=0)
    endif()

    target_include_directories(${NAME} 
        PRIVATE ${include_dirs}
    )

    # BUILD_DEPENDENCIES

    set(multiValueArgs PRIVATE PUBLIC INTERFACE TARGETS MAC_FRAMEWORKS)
    cmake_parse_arguments(ce_add_test_BUILD_DEPENDENCIES "" "" "${multiValueArgs}" ${ce_add_test_BUILD_DEPENDENCIES})

    if(${PAL_PLATFORM_IS_MAC})
        list(APPEND ce_add_test_BUILD_DEPENDENCIES_PRIVATE "c")
        list(APPEND ce_add_test_BUILD_DEPENDENCIES_PRIVATE "c++")
        list(APPEND ce_add_test_BUILD_DEPENDENCIES_PRIVATE "-framework CoreServices")
        if(ce_add_test_BUILD_DEPENDENCIES_MAC_FRAMEWORKS)
            foreach(framework ${ce_add_test_BUILD_DEPENDENCIES_MAC_FRAMEWORKS})
                list(APPEND ce_add_test_BUILD_DEPENDENCIES_PRIVATE "-framework ${framework}")
            endforeach()
        endif()
    endif()

    if(ce_add_test_BUILD_DEPENDENCIES_PRIVATE OR ce_add_test_BUILD_DEPENDENCIES_PUBLIC OR ce_add_test_BUILD_DEPENDENCIES_INTERFACE)
        target_link_libraries(${NAME}
            PRIVATE   ${ce_add_test_BUILD_DEPENDENCIES_PRIVATE}
            PUBLIC    ${ce_add_test_BUILD_DEPENDENCIES_PUBLIC}
            INTERFACE ${ce_add_test_BUILD_DEPENDENCIES_INTERFACE}
        )
    endif()

    if(ce_add_test_BUILD_DEPENDENCIES_TARGETS)
        add_dependencies(${NAME} ${ce_add_test_BUILD_DEPENDENCIES_TARGETS})
    endif()

    add_test(${NAME} ${NAME})
    
endfunction()


