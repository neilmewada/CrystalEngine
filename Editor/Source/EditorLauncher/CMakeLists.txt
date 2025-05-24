cmake_minimum_required(VERSION 3.20)

project(EditorLauncher CXX)

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)

ce_add_target(${PROJECT_NAME} CONSOLEAPP
    VS_STARTUP_PROJECT
    NAMESPACE CE
    FOLDER Editor/Source
    PCHHEADER
        PRIVATE
            Sources/${PROJECT_NAME}.h
    FILES_CMAKE
        PRIVATE
            ${PROJECT_NAME_LOWERCASE}_source_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Sources/
    BUILD_DEPENDENCIES
        PRIVATE
            ThirdParty::cxxopts
            CE::Core
            CE::CoreApplication
            CE::EditorCore
            CE::CrystalEditor
        TARGETS
            Config 
            EngineAssets 
            EditorAssets
            EngineShaders
    RUNTIME_DEPENDENCIES

)


if(${PAL_TRAIT_BUILD_TESTS})
    add_subdirectory(Tests)
endif()

