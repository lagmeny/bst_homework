cmake_minimum_required(VERSION 3.16)

project(task01 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX)
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
endif()

include(CTest)

add_library(${PROJECT_NAME}_tree STATIC
    tree.h
    tree.c
)
target_include_directories(${PROJECT_NAME}_tree PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(${PROJECT_NAME}
    demo.c
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_tree)

if(BUILD_TESTING)
    add_executable(${PROJECT_NAME}_tests
        tests.c
    )
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME}_tree)

    foreach(test_target IN ITEMS ${PROJECT_NAME}_tests)
        if(MSVC)
            target_compile_options(${test_target} PRIVATE /UNDEBUG)
        else()
            target_compile_options(${test_target} PRIVATE -UNDEBUG)
        endif()
    endforeach()

    add_test(NAME ${PROJECT_NAME}_basic COMMAND ${PROJECT_NAME}_tests)
endif()

option(TASK01_BUILD_STACK_PROBE "Build the separate recursive destruction experiment" OFF)

if(TASK01_BUILD_STACK_PROBE)
    add_executable(${PROJECT_NAME}_stack_probe
        stack_probe.c
    )
endif()
