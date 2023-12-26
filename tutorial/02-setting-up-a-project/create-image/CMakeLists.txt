project(02-setting-up-a-project-image
    DESCRIPTION "Kernel image for 02-setting-up-a-project RPI 64 bit bare metal")

message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

message("\n** Setting up ${PROJECT_NAME} **\n")

set(BAREMETAL_TARGET_KERNEL kernel8)
set(DEPENDENCY 02-setting-up-a-project)
set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)

create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
