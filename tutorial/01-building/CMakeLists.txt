cmake_minimum_required(VERSION 3.18)

message(STATUS "CMake ${CMAKE_VERSION}")

project(01-building
    DESCRIPTION "Application to demonstrate building using CMake"
    LANGUAGES CXX ASM)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    )

set(PROJECT_INCLUDES_PUBLIC )
set(PROJECT_INCLUDES_PRIVATE )

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
