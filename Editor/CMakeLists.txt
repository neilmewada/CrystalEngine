cmake_minimum_required(VERSION 3.20)

project(Editor)
set(CURRENT_BUNDLE_CATEGORY "Editor")

# Assets
add_subdirectory(Assets)
add_subdirectory(Styles)

# Editor Modules
add_subdirectory(Source/EditorCore)
add_subdirectory(Source/EditorEngine)
add_subdirectory(Source/CrystalEditor)

# Editor Executables
add_subdirectory(Source/EditorLauncher)
