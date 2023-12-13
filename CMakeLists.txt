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
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CONFIG_DIR Debug)
elseif(
    ${CMAKE_BUILD_TYPE} STREQUAL "Release" OR
    ${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel" OR 
    ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
        set(CONFIG_DIR Release)
else()
    message(FATAL_ERROR "Invalid build type: " ${CMAKE_BUILD_TYPE})
endif()

set(DEPLOYMENT_DIR "${CMAKE_SOURCE_DIR}/deploy" CACHE STRING "Deployment directory")
set(OUTPUT_BASE_DIR "${CMAKE_SOURCE_DIR}/output" CACHE STRING "Output directory")
set(OUTPUT_BIN_DIR "${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin")
set(OUTPUT_LIB_DIR "${OUTPUT_BASE_DIR}/${CONFIG_DIR}/lib")

project(baremetal
    DESCRIPTION "Baremetal overall project")

add_subdirectory(code)
