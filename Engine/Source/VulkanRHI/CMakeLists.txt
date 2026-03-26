cmake_minimum_required(VERSION 3.20)

project(VulkanRHI CXX)

ce_add_target(${PROJECT_NAME} SHARED
    NAMESPACE CE
    FOLDER Engine/Source
    PCHHEADER
        PRIVATE
            Public/VulkanRHI.h
    FILES_CMAKE
        PRIVATE
            vulkanrhi_private_files.cmake
        PUBLIC
            vulkanrhi_public_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Private/
            Private/imgui
            ${Vulkan_INCLUDE_DIR}
        PUBLIC
            Private/imgui
            Public/
    COMPILE_DEFINITIONS
        PRIVATE
            ${PROJECT_NAME}_EXPORTS
            IMGUI_API=DLL_EXPORT
        INTERFACE
            IMGUI_API=DLL_IMPORT
    BUILD_DEPENDENCIES
        PRIVATE
            ThirdParty::sdl
            ${Vulkan_LIBRARIES}
            CE::CoreApplication
        PUBLIC
            CE::Core
            CE::CoreRHI
            CE::CoreAssets
        TARGETS Config
        MAC_FRAMEWORKS
            QuartzCore
    RUNTIME_DEPENDENCIES
        Vulkan
        sdl
)

if(${PAL_TRAIT_BUILD_TESTS_SUPPORTED})
    #add_subdirectory(Tests)
endif()
