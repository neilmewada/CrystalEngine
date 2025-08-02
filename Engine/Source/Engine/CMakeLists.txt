cmake_minimum_required(VERSION 3.20)

project(Engine CXX)

ce_add_target(Engine SHARED
    AUTORTTI
    RESOURCES
    NAMESPACE CE
    FOLDER Engine/Source
    PCHHEADER 
        PRIVATE 
            Public/Engine.h
    FILES_CMAKE
        PRIVATE
            engine_private_files.cmake
        PUBLIC
            engine_public_files.cmake
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
            ThirdParty::sdl
            CE::CoreApplication
        PUBLIC
            CE::Core
            CE::CoreSettings
            CE::CoreMedia
            CE::CoreShader
            CE::CorePhysics
            CE::CoreRHI
            CE::CoreRPI
            CE::FusionCore
            CE::Fusion
        TARGETS 
            Config
    RUNTIME_DEPENDENCIES
        
)


if(${PAL_TRAIT_BUILD_ENGINE_TESTS_SUPPORTED})
    add_subdirectory(Tests)
endif()

