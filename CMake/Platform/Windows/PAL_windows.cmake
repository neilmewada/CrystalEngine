
ce_set(PAL_EXECUTABLE_APPLICATION_FLAG WIN32_EXECUTABLE)

ce_set(CMAKE_DEBUG_POSTFIX "")

enable_language(RC)

ce_set(EDITOR_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_LIST_DIR}/EditorIcon.rc")

if(${CE_STANDALONE})
    ce_set(PAL_TRAIT_BUILD_EDITOR 0)
    ce_set(PAL_TRAIT_BUILD_HOST_TOOLS 0)
    ce_set(PAL_TRAIT_BUILD_HOST_LIBRARIES 0)
    ce_set(PAL_TRAIT_BUILD_STANDALONE 1)
    ce_set(PAL_TRAIT_QT_SUPPORTED 0)

    add_compile_definitions(PAL_TRAIT_BUILD_STANDALONE=1)
else()
    ce_set(PAL_TRAIT_BUILD_EDITOR 1)
    ce_set(PAL_TRAIT_BUILD_HOST_TOOLS 1)
    ce_set(PAL_TRAIT_BUILD_HOST_LIBRARIES 1)
    ce_set(PAL_TRAIT_BUILD_STANDALONE 0)
    ce_set(PAL_TRAIT_QT_SUPPORTED 0)

    add_compile_definitions(PAL_TRAIT_BUILD_EDITOR=1)
    #add_compile_definitions(PAL_TRAIT_QT_SUPPORTED=1)
    add_compile_definitions(PAL_TRAIT_WITH_EDITOR_DATA=1)
endif()

# Graphics APIs
ce_set(PAL_TRAIT_VULKAN_SUPPORTED 1)
ce_set(PAL_TRAIT_METAL_SUPPORTED 0)
ce_set(PAL_TRAIT_SDL_SUPPORTED 1)

add_compile_definitions(PAL_TRAIT_VULKAN_SUPPORTED=1)
add_compile_definitions(PAL_TRAIT_SDL_SUPPORTED=1)

# For testing only
#add_compile_definitions(PAL_TRAIT_VULKAN_LIMITED_DESCRIPTOR_SETS=1)
#add_compile_definitions(_CRTDBG_MAP_ALLOC=1)

ce_set(PAL_TRAIT_BUILD_TESTS_SUPPORTED 1)
ce_set(PAL_TRAIT_BUILD_EDITOR_TESTS_SUPPORTED 1)
ce_set(PAL_TRAIT_BUILD_ENGINE_TESTS_SUPPORTED 1)

if(${CE_BUILD_TESTS} AND NOT ${PAL_TRAIT_BUILD_TESTS_SUPPORTED})
    set(CE_BUILD_TESTS FALSE)
endif()

if(${CE_BUILD_TESTS})
    add_compile_definitions(PAL_TRAIT_BUILD_TESTS=1)
endif()


if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    ce_set(PAL_TRAIT_COMPILER_ID MSVC)
    ce_set(PAL_TRAIT_COMPILER_ID_LOWERCASE msvc)
    add_compile_definitions(COMPILER_MSVC=1)

    add_compile_options("/MP")
    add_compile_options("$<$<CONFIG:Development>:/Zi>")
    add_compile_options("/wd4996")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    ce_set(PAL_TRAIT_COMPILER_ID Clang)
    ce_set(PAL_TRAIT_COMPILER_ID_LOWERCASE clang)
    add_compile_definitions(COMPILER_CLANG=1)
else()
    message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} not supported on ${PAL_PLATFORM_NAME}")
endif()

ce_set(PAL_PLATFORM_IS_64_BIT 1)
ce_set(PAL_PLATFORM_IS_WINDOWS 1)
ce_set(PAL_PLATFORM_IS_DESKTOP 1)

add_compile_definitions(PLATFORM_WINDOWS=1)
add_compile_definitions(PLATFORM_DESKTOP=1)
add_compile_definitions(IS_64BIT=1)

#########################################
# Platform Utils

macro(ce_decorated_lib_name LIB_NAME OUT_NAME)
    if((${LIB_NAME} MATCHES ".dll$") OR (${LIB_NAME} MATCHES ">$"))
        set(${OUT_NAME} "${LIB_NAME}")
    else()
        set(${OUT_NAME} "${LIB_NAME}.dll")
    endif()
endmacro()


