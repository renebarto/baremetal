function(list_to_string in out)
    set(tmp "")
    foreach(VAL ${${in}})
        string(APPEND tmp "${VAL} ")
    endforeach()
    set(${out} "${tmp}" PARENT_SCOPE)
endfunction()

# Converts a CMake list to a CMake string and displays this in a status message along with the text specified.
function(display_list text)
    set(list_str "")
    foreach(item ${ARGN})
        string(CONCAT list_str "${list_str} ${item}")
    endforeach()
    message(STATUS ${text} ${list_str})
endfunction()

function(get_git_tag out)
    if (UNIX)
        find_package(Git)
    else()
        find_program(GIT_EXECUTABLE git PATHS "C:/Program Files/Git/bin")
    endif()
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --match "[0-9]*.[0-9]*.[0-9]*" --tags --abbrev=5 HEAD
        RESULT_VARIABLE RESULT
        OUTPUT_VARIABLE OUTPUT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (${RESULT} EQUAL 0)
        set(${out} "${OUTPUT}" PARENT_SCOPE)
    else()
        set(${out} "" PARENT_SCOPE)
    endif()
endfunction()

function(parse_version version_number version_major version_minor version_level version_build)
    if ("${${version_number}}" STREQUAL "")
        set(${version_number} "0.0.0.0" PARENT_SCOPE)
    endif()
    string(REPLACE "." ";" VERSION_PARTS "${${version_number}}")
    LIST(LENGTH VERSION_PARTS VERSION_NUM_PARTS)

    set(VERSION_MAJOR 0)
    if (VERSION_NUM_PARTS GREATER_EQUAL 1)
        list(GET VERSION_PARTS 0 VERSION_MAJOR)
    endif()
    set(${version_major} "${VERSION_MAJOR}" PARENT_SCOPE)

    set(VERSION_MINOR 0)
    if (VERSION_NUM_PARTS GREATER_EQUAL 2)
        list(GET VERSION_PARTS 1 VERSION_MINOR)
    endif()
    set(${version_minor} "${VERSION_MINOR}" PARENT_SCOPE)

    set(VERSION_LEVEL 0)
    if (VERSION_NUM_PARTS GREATER_EQUAL 3)
        list(GET VERSION_PARTS 2 VERSION_LEVEL)
    endif()
    set(${version_level} "${VERSION_LEVEL}" PARENT_SCOPE)

    set(VERSION_BUILD 0)
    if (VERSION_NUM_PARTS GREATER_EQUAL 4)
        list(GET VERSION_PARTS 3 VERSION_BUILD)
    endif()
    set(${version_build} "${VERSION_BUILD}" PARENT_SCOPE)

    if ("${VERSION_MAJOR}" STREQUAL "" OR
        "${VERSION_MINOR}" STREQUAL "" OR
        "${VERSION_LEVEL}" STREQUAL "" OR
        "${VERSION_BUILD}" STREQUAL "")
        message(SEND_ERROR "Incorrectly specified version number: ${version_number}")
    endif()
endfunction()

function(show_target_properties target)
    if (CMAKE_VERBOSE_MAKEFILE)
        message(STATUS "")
        message(STATUS "Properties for ${target}")

        get_target_property(TARGET_TYPE ${target} TYPE)
        display_list("Target type                       : " ${TARGET_TYPE})
        if (NOT TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
            get_target_property(DEFINES ${target} COMPILE_DEFINITIONS)
            display_list("Target defines                    : " ${DEFINES})
            get_target_property(OPTIONS ${target} COMPILE_OPTIONS)
            display_list("Target options                    : " ${OPTIONS})
        endif ()

        get_target_property(INCLUDES ${target} INTERFACE_INCLUDE_DIRECTORIES)
        display_list("Target include dirs public        : " ${INCLUDES})

        if (NOT TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
            get_target_property(INCLUDES ${target} INCLUDE_DIRECTORIES)
            display_list("Target include dirs private       : " ${INCLUDES})

            get_target_property(LIBRARIES ${target} LINK_LIBRARIES)
            display_list("Target link libraries             : " ${LIBRARIES})

            get_target_property(LINK_OPTIONS ${target} LINK_FLAGS)
            display_list("Target link options               : " ${LINK_OPTIONS})
        endif ()

        get_target_property(DEFINES_EXPORTS ${target} INTERFACE_COMPILE_DEFINITIONS)
        display_list("Target exported defines           : " ${DEFINES_EXPORTS})

        get_target_property(OPTIONS_EXPORTS ${target} INTERFACE_COMPILE_OPTIONS)
        display_list("Target exported options           : " ${OPTIONS_EXPORTS})

        get_target_property(INCLUDE_DIRS_EXPORTS ${target} INTERFACE_INCLUDE_DIRECTORIES)
        display_list("Target exported include dirs      : " ${INCLUDE_DIRS_EXPORTS})

        get_target_property(LIBRARIES_EXPORTS ${target} INTERFACE_LINK_LIBRARIES)
        display_list("Target exported link libraries    : " ${LIBRARIES_EXPORTS})

        get_test_property(IMPORT_DEPENDENCIES ${target} IMPORTED_LINK_DEPENDENT_LIBRARIES)
        display_list("Target imported dependencies      : " ${IMPORT_DEPENDENCIES})

        get_test_property(IMPORT_LIBRARIES ${target} IMPORTED_LINK_INTERFACE_LIBRARIES)
        display_list("Target imported link libraries    : " ${IMPORT_LIBRARIES})

        if (NOT TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
            get_target_property(LINK_DEPENDENCIES ${target} LINK_DEPENDS)
            display_list("Target link dependencies          : " ${LINK_DEPENDENCIES})

            get_target_property(EXPLICIT_DEPENDENCIES ${target} MANUALLY_ADDED_DEPENDENCIES)
            display_list("Target manual dependencies        : " ${EXPLICIT_DEPENDENCIES})
        endif ()

        if (NOT TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
            get_target_property(ARCHIVE_LOCATION ${target} ARCHIVE_OUTPUT_DIRECTORY)
            display_list("Target static library location    : " ${ARCHIVE_LOCATION})

            get_target_property(LIBRARY_LOCATION ${target} LIBRARY_OUTPUT_DIRECTORY)
            display_list("Target dynamic library location   : " ${LIBRARY_LOCATION})

            get_target_property(RUNTIME_LOCATION ${target} RUNTIME_OUTPUT_DIRECTORY)
            display_list("Target binary location            : " ${RUNTIME_LOCATION})

            get_target_property(TARGET_LINK_FLAGS ${target} LINK_FLAGS)
            display_list("Target link flags                 : " ${TARGET_LINK_FLAGS})

            get_target_property(TARGET_VERSION ${target} VERSION)
            display_list("Target version                    : " ${TARGET_VERSION})

            get_target_property(TARGET_SOVERSION ${target} SOVERSION)
            display_list("Target so-version                 : " ${TARGET_SOVERSION})

            get_target_property(TARGET_OUTPUT_NAME ${target} OUTPUT_NAME)
            display_list("Target output name                : " ${TARGET_OUTPUT_NAME})

            get_target_property(TARGET_CXX_STANDARD ${target} CXX_STANDARD)
            display_list("Target C++ standard               : " ${TARGET_CXX_STANDARD})
        endif ()
    endif()
endfunction()

function(create_image target image project)
    message(STATUS "create_image ${target} ${image} ${project}")

    if(NOT TARGET ${project})
      message(STATUS "There is no target named '${project}'")
      return()
    endif()

    get_target_property(TARGET_NAME ${project} OUTPUT_NAME)
    message(STATUS "TARGET_NAME ${TARGET_NAME}")

    message(STATUS "generate ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image} from ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin/${project}")
    add_custom_command(
        OUTPUT ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}
        COMMAND ${CMAKE_OBJCOPY} ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin/${TARGET_NAME} -O binary ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
        DEPENDS ${project}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(${target} ALL DEPENDS
        ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
        )
endfunction()

function(setup_image target)
    project(${target}-image
        DESCRIPTION "Kernel image for ${target}")

    message(STATUS "\n**********************************************************************************\n")
    message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

    message("\n** Setting up ${PROJECT_NAME} **\n")

    set(DEPENDENCY ${target})
    set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)

    create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
endfunction()
