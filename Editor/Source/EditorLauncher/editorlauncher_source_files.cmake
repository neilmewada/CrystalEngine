
file(GLOB_RECURSE FILES "Sources/*.h" "Sources/*.cpp")

ce_filter_platform_files(FILES)

list(APPEND FILES "${EDITOR_ICON_RESOURCE_WINDOWS}")
