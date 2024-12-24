message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(04-setting-up-common-options)
add_subdirectory(05-setting-up-tools)
add_subdirectory(06-console-uart1)
add_subdirectory(07-improving-startup-static-initialization)
