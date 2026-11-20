cmake_minimum_required(VERSION 3.16)

project(task02 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
endif()

include(CTest)

add_library(${PROJECT_NAME}_tree STATIC
    tree.h
    tree.cpp
)
target_include_directories(${PROJECT_NAME}_tree PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(${PROJECT_NAME}
    demo.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_tree)

if(BUILD_TESTING)
    add_executable(${PROJECT_NAME}_tests
        tests.cpp
    )
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME}_tree)

    add_executable(${PROJECT_NAME}_memory_tests
        memory_tests.cpp
    )
    target_link_libraries(${PROJECT_NAME}_memory_tests PRIVATE ${PROJECT_NAME}_tree)

    foreach(test_target IN ITEMS ${PROJECT_NAME}_tests ${PROJECT_NAME}_memory_tests)
        if(MSVC)
            target_compile_options(${test_target} PRIVATE /UNDEBUG)
        else()
            target_compile_options(${test_target} PRIVATE -UNDEBUG)
        endif()
    endforeach()

    add_test(NAME ${PROJECT_NAME}_basic COMMAND ${PROJECT_NAME}_tests)
    add_test(NAME ${PROJECT_NAME}_memory COMMAND ${PROJECT_NAME}_memory_tests)
endif()
