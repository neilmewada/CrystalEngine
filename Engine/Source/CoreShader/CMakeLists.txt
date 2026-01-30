cmake_minimum_required(VERSION 3.20)

project(CoreShader CXX)

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)

ce_add_target(${PROJECT_NAME} SHARED
    AUTORTTI
    NAMESPACE CE
    FOLDER Engine/Source
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
            $<${PAL_PLATFORM_IS_DESKTOP}:ThirdParty::dxcompiler>
            $<${PAL_PLATFORM_IS_DESKTOP}:Vulkan::SPIRV-Tools>
            #$<${PAL_PLATFORM_IS_DESKTOP}:Vulkan::SPIRV-Cross>
        PUBLIC
            CE::Core
            CE::CoreRHI
            #$<${PAL_TRAIT_VULKAN_SUPPORTED}:CE::VulkanRHI>
            #$<${PAL_TRAIT_METAL_SUPPORTED}:CE::MetalRHI>
    RUNTIME_DEPENDENCIES
        dxcompiler
        SpirvTools
)


if(${PAL_TRAIT_BUILD_TESTS})
    add_subdirectory(Tests)
endif()

