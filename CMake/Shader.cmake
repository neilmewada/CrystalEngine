include_guard(GLOBAL)

function(ce_add_shaders NAME)
    set(options COPY_CONFIGS)
    set(oneValueArgs VERSION OUTPUT_DIRECTORY FOLDER)
    set(multiValueArgs SOURCES COMPILE_DEFINITIONS INCLUDE_DIRECTORIES)

    set(DXC "$ENV{VULKAN_SDK}/bin/dxc")
    cmake_parse_arguments(ce_add_shaders "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(DEST_SHADERBIN_FILES "")

    foreach(hlsl_file ${ce_add_shaders_SOURCES})
        cmake_path(GET hlsl_file EXTENSION LAST_ONLY hlsl_file_ext)
        
        if((${hlsl_file_ext} STREQUAL ".hlsl") OR (${hlsl_file_ext} STREQUAL ".hlsli")) # Check if the file is a HLSL or HLSLI file

            cmake_path(REMOVE_EXTENSION hlsl_file)
            cmake_path(GET hlsl_file FILENAME hlsl_file_name)
            cmake_path(RELATIVE_PATH hlsl_file OUTPUT_VARIABLE dest_shaderbin_file_rel)
            set(dest_shaderbin_file "${CE_OUTPUT_DIR}/${ce_add_shaders_OUTPUT_DIRECTORY}/${dest_shaderbin_file_rel}${hlsl_file_ext}")
            list(APPEND DEST_SHADERBIN_FILES ${dest_shaderbin_file})

            cmake_path(GET dest_shaderbin_file PARENT_PATH dest_shaderbin_file_parent)

            add_custom_command(OUTPUT ${dest_shaderbin_file}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${hlsl_file}${hlsl_file_ext}" "${dest_shaderbin_file_parent}"
            )

        endif()
    endforeach()

    add_custom_target(${NAME}
        DEPENDS ${DEST_SHADERBIN_FILES}
        SOURCES ${ce_add_shaders_SOURCES}
        VERBATIM
    )

    ce_group_sources_by_folder(${NAME})

    if(ce_add_shaders_FOLDER)
        set_target_properties(${NAME} 
            PROPERTIES
                FOLDER "${ce_add_shaders_FOLDER}"
        )
    endif()
    
endfunction()

