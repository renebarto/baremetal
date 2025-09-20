function(list_to_string in out)
    set(tmp "")
    foreach(VAL ${${in}})
        string(APPEND tmp "${VAL} ")
    endforeach()
    set(${out} "${tmp}" PARENT_SCOPE)
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
