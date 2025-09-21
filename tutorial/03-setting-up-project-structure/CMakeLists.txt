cmake_minimum_required(VERSION 3.18)

message(STATUS "CMake ${CMAKE_VERSION}")

set(SCRIPTS_DIR "" CACHE STRING "CMake scripts path")

if ("${SCRIPTS_DIR}" STREQUAL "")
    set(SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/cmake" CACHE STRING "CMake scripts path" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${SCRIPTS_DIR})

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CONFIG_DIR ${CMAKE_BUILD_TYPE})

set(BAREMETAL_TARGET_KERNEL kernel8)
set(BAREMETAL_TARGET RPI3)

set(DEPLOYMENT_DIR "${CMAKE_SOURCE_DIR}/deploy" CACHE STRING "Deployment directory")
set(OUTPUT_BASE_DIR "${CMAKE_SOURCE_DIR}/output/${BAREMETAL_TARGET}" CACHE STRING "Output directory")
set(OUTPUT_BIN_DIR "${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin")
set(OUTPUT_LIB_DIR "${OUTPUT_BASE_DIR}/${CONFIG_DIR}/lib")

message(STATUS "\n** Setting up project **\n--")

message(STATUS "\n##################################################################################")
message(STATUS "\n** Setting up toolchain **\n--")

project(baremetal-main
    DESCRIPTION "Baremetal overall project")

add_subdirectory(code)
add_subdirectory(tutorial)
