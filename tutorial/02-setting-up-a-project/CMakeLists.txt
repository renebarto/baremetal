cmake_minimum_required(VERSION 3.18)

message(STATUS "CMake ${CMAKE_VERSION}")

set(SCRIPTS_DIR "" CACHE STRING "CMake scripts path")
set(CONFIG_DIR Debug)
set(DEPLOYMENT_DIR ${CMAKE_SOURCE_DIR}/../../deploy)
set(OUTPUT_BASE_DIR "${CMAKE_SOURCE_DIR}/../../output" CACHE STRING "Output directory")

if ("${SCRIPTS_DIR}" STREQUAL "")
    set(SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/cmake" CACHE STRING "CMake scripts path" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${SCRIPTS_DIR})

include(functions)

set(BAREMETAL_TARGET_KERNEL kernel8)

project(02-setting-up-a-project
    DESCRIPTION "Application to demonstrate building using CMake"
    LANGUAGES CXX ASM)

message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

message("\n** Setting up ${PROJECT_NAME} **\n")

set(PROJECT_TARGET_NAME ${PROJECT_NAME}.elf)

set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE )
set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )

set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE
    -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
    -Wall -Wextra -Werror
    -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -Wno-unused-variable -Wno-unused-parameter
    -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++
    -fno-exceptions -fno-rtti -O0 -std=gnu++17
    )
set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE
    -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2)
set(PROJECT_COMPILE_OPTIONS_ASM_PUBLIC )

set(PROJECT_INCLUDE_DIRS_PRIVATE )
set(PROJECT_INCLUDE_DIRS_PUBLIC )

set(PROJECT_LINK_OPTIONS ${CMAKE_EXE_LINKER_FLAGS} -Wl,--section-start=.init=0x80000 -T ${CMAKE_SOURCE_DIR}/link.ld -nostdlib -nostartfiles)

set(PROJECT_DEPENDENCIES )

set(PROJECT_LIBS
    ${PROJECT_DEPENDENCIES}
    )

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/start.S
    )
set(PROJECT_INCLUDES_PUBLIC)
set(PROJECT_INCLUDES_PRIVATE)

if (PLATFORM_BAREMETAL)
    set(START_GROUP -Wl,--start-group)
    set(END_GROUP -Wl,--end-group)
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})

target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE})
target_compile_definitions(${PROJECT_NAME} PUBLIC  ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC})
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE})
target_compile_options(${PROJECT_NAME} PUBLIC  ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC})

list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
message(STATUS "PROJECT_LINK_OPTIONS=${PROJECT_LINK_OPTIONS_STRING}")
if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/lib)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin)

set(BAREMETAL_EXECUTABLE_TARGET ${PROJECT_NAME})
setup_image(${PROJECT_NAME})
