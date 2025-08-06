
include_guard(GLOBAL)

set(SUPPORTED_ASSET_FILE_EXTENSIONS ".shader")
set(SUPPORTED_ASSET_FILE_EXTENSIONS ".compute")
list(APPEND SUPPORTED_ASSET_FILE_EXTENSIONS ".png")
list(APPEND SUPPORTED_ASSET_FILE_EXTENSIONS ".jpg")
list(APPEND SUPPORTED_ASSET_FILE_EXTENSIONS ".jpeg")
list(APPEND SUPPORTED_ASSET_FILE_EXTENSIONS ".hdr")
list(APPEND SUPPORTED_ASSET_FILE_EXTENSIONS ".fbx" )
list(APPEND SUPPORTED_ASSET_FILE_EXTENSIONS ".FBX")
#list(APPEND SUPPORTED_ASSET_FILE_EXTENSIONS ".ttf")

# Add assets
function(ce_add_assets NAME)
    set(options GENERATED)
    set(oneValueArgs OUTPUT_DIRECTORY TYPE)
    set(multiValueArgs ASSET_FILES INCLUDE_DIRECTORIES ASSET_EXTENSIONS)

    set(ce_add_assets_TYPE_Engine "Engine")
    set(ce_add_assets_TYPE_Editor "Editor")
    set(ce_add_assets_TYPE_Game   "Game")
    set(ce_add_assets_TYPE_Plugin "Plugin")
    
    cmake_parse_arguments(ce_add_assets "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ce_add_assets_TYPE_${ce_add_assets_TYPE})
        message(FATAL_ERROR "Invalid asset type: ${ce_add_assets_TYPE}")
        return()
    endif()

    foreach(asset_ext ${ce_add_assets_ASSET_EXTENSIONS})
        list(APPEND SUPPORTED_ASSET_FILE_EXTENSIONS "${asset_ext}")
    endforeach()

    # INCLUDE_DIRECTORIES
    set(INCLUDE_DIRECTORIES_COMMAND "")
    foreach(include_dir ${ce_add_assets_INCLUDE_DIRECTORIES})
        if(IS_ABSOLUTE ${include_dir})
            
        else()
            set(include_dir "${CMAKE_CURRENT_SOURCE_DIR}/${include_dir}")
        endif()
        
        list(APPEND INCLUDE_DIRECTORIES_COMMAND "-I ${include_dir}")
    endforeach()

    # OUTPUT_DIRECTORY
    set(OUTPUT_DIRECTORY "${CE_OUTPUT_DIR}/${ce_add_assets_OUTPUT_DIRECTORY}")

    # ASSET_FILES

    set(ASSET_FILES "")
    foreach(asset_file_cmake ${ce_add_assets_ASSET_FILES})
        include(${asset_file_cmake})
        list(APPEND ASSET_FILES ${FILES})
    endforeach()

    set(DEST_ASSET_FILES "")
    set(COPY_COMMANDS "")
    set(INPUT_COMMANDS "")
    set(TEMP_DIR "${CE_OUTPUT_DIR}/Temp")
    
    foreach(asset_file ${ASSET_FILES})
        cmake_path(GET asset_file EXTENSION LAST_ONLY asset_file_ext)
        set(is_file_supported FALSE)

        list(FIND SUPPORTED_ASSET_FILE_EXTENSIONS ${asset_file_ext} index)

        if(${index} GREATER_EQUAL 0)
            set(is_file_supported TRUE)
            set(output_file_ext ".casset")
        else() # Unsupported file! Just copy it over as it is.
            set(output_file_ext ${asset_file_ext})
        endif()

        cmake_path(RELATIVE_PATH asset_file OUTPUT_VARIABLE dest_asset_file_rel)
        set(dest_asset_file "${OUTPUT_DIRECTORY}/${dest_asset_file_rel}")
        cmake_path(REPLACE_EXTENSION dest_asset_file LAST_ONLY "${output_file_ext}")

        cmake_path(GET dest_asset_file PARENT_PATH dest_asset_file_parent)
        list(APPEND DEST_ASSET_FILES "${dest_asset_file}")

        if(${is_file_supported})
            # add_custom_command(OUTPUT ${dest_asset_file}
            #     COMMAND "AssetProcessor" --mode "${ce_add_assets_TYPE}" -I "${CMAKE_SOURCE_DIR}/Engine/Shaders" --target "${PAL_PLATFORM_NAME}" --input-root "${CMAKE_CURRENT_SOURCE_DIR}" --output-root "${OUTPUT_DIRECTORY}" -i "${asset_file}" --temp "${TEMP_DIR}"
            #     DEPENDS ${asset_file}
            #     VERBATIM
            # )
        else()
            add_custom_command(OUTPUT ${dest_asset_file}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${asset_file}" "${dest_asset_file_parent}"
            )
        endif()
    endforeach()

    if (PAL_TRAIT_PREBUILD_SUPPORTED)
        add_custom_target(${NAME}
            DEPENDS ${DEST_ASSET_FILES}
            VERBATIM
            SOURCES ${ASSET_FILES}
        )

        add_custom_command(TARGET ${NAME}
            PRE_BUILD
            COMMAND "AssetProcessor" --mode "${ce_add_assets_TYPE}" -I "${CMAKE_SOURCE_DIR}/Engine/Shaders" --target "${PAL_PLATFORM_NAME}" --input-root "${CMAKE_CURRENT_SOURCE_DIR}" --output-root "${OUTPUT_DIRECTORY}" --temp "${TEMP_DIR}" --project "${CE_OUTPUT_DIR}"
        )
    else()
        add_custom_target(${NAME}
            COMMAND "AssetProcessor" --mode "${ce_add_assets_TYPE}" -I "${CMAKE_SOURCE_DIR}/Engine/Shaders" --target "${PAL_PLATFORM_NAME}" --input-root "${CMAKE_CURRENT_SOURCE_DIR}" --output-root "${OUTPUT_DIRECTORY}" --temp "${TEMP_DIR}" --project "${CE_OUTPUT_DIR}"
            #DEPENDS ${DEST_ASSET_FILES}
            VERBATIM
            SOURCES ${ASSET_FILES}
        )
    endif ()

    set_target_properties(${NAME} 
        PROPERTIES
            FOLDER "Assets"
    )

    add_dependencies(${NAME} Config)

    ce_group_sources_by_folder(${NAME})

endfunction()

