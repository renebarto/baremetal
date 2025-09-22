message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(04-setting-up-common-options)
add_subdirectory(05-setting-up-tools)
if (NOT ${BAREMETAL_RPI_TARGET} EQUAL 5)
    add_subdirectory(06-console-uart1)
endif()