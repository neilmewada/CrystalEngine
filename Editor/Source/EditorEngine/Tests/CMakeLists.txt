cmake_minimum_required(VERSION 3.20)

set(TEST_TARGET EditorEngine)

set(TEST_NAME ${TEST_TARGET}_Test)
project(${TEST_NAME})

enable_testing()

file(GLOB_RECURSE SRCS "*.cpp" "*.h")

ce_add_test(${PROJECT_NAME}
    TARGET ${TEST_TARGET}
    FOLDER "Tests/Editor"
    SOURCES
        ${SRCS}
    BUILD_DEPENDENCIES
        TARGETS Config
)
