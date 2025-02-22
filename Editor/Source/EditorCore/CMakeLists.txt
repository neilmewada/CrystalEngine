cmake_minimum_required(VERSION 3.20)

if(PAL_PLATFORM_IS_MAC)
    project(EditorCore CXX OBJC OBJCXX)
else()
    project(EditorCore CXX)
endif()

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)

ce_add_target(${PROJECT_NAME} SHARED
    AUTORTTI
    NAMESPACE CE
    FOLDER Editor/Source
    PCHHEADER 
        PRIVATE 
            Public/${PROJECT_NAME}.h
    FILES_CMAKE
        PRIVATE
            ${PROJECT_NAME_LOWERCASE}_private_files.cmake
        PUBLIC
            ${PROJECT_NAME_LOWERCASE}_public_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Private/
        PUBLIC
            Public/
    COMPILE_DEFINITIONS
        PRIVATE
            ${PROJECT_NAME}_EXPORTS
    BUILD_DEPENDENCIES
        PRIVATE
        PUBLIC
            CE::Core
            CE::CoreApplication
            CE::CoreSettings
            CE::CoreShader
            CE::CoreMedia
            CE::CoreRHI
            $<${PAL_TRAIT_VULKAN_SUPPORTED}:CE::VulkanRHI>
            CE::FusionCore
            CE::Engine
        MAC_FRAMEWORKS Cocoa
    RUNTIME_DEPENDENCIES
        
)


if(${PAL_TRAIT_BUILD_EDITOR_TESTS_SUPPORTED})
    add_subdirectory(Tests)
endif()

