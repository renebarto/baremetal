message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

project(baremetal-18
    DESCRIPTION "Bare metal library"
    LANGUAGES CXX ASM)

set(PROJECT_TARGET_NAME ${PROJECT_NAME})

set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE ${COMPILE_DEFINITIONS_C})
set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
set(PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE ${COMPILE_DEFINITIONS_ASM})
set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
set(PROJECT_INCLUDE_DIRS_PRIVATE )
set(PROJECT_INCLUDE_DIRS_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})

set(PROJECT_DEPENDENCIES
    )

set(PROJECT_LIBS
    ${LINKER_LIBRARIES}
    ${PROJECT_DEPENDENCIES}
    )

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
    ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
    )

set(PROJECT_INCLUDES_PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/StdArg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
    )
set(PROJECT_INCLUDES_PRIVATE )

if (CMAKE_VERBOSE_MAKEFILE)
    display_list("Package                           : " ${PROJECT_NAME} )
    display_list("Package description               : " ${PROJECT_DESCRIPTION} )
    display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
    display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
    display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
    display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
    display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
    display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
    display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
    display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
    display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
    display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
    display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
    display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
    display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
    display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
    display_list("Link libs                         : " ${PROJECT_LIBS} )
    display_list("Source files                      : " ${PROJECT_SOURCES} )
    display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
    display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
endif()

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
    )
target_compile_definitions(${PROJECT_NAME} PUBLIC
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
    )
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
    )
target_compile_options(${PROJECT_NAME} PUBLIC
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
    )

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})

list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
endif()

link_directories(${LINK_DIRECTORIES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})

show_target_properties(${PROJECT_NAME})
