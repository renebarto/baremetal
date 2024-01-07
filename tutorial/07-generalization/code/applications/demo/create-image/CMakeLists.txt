project(07-generalization-image
    DESCRIPTION "Kernel image for demo RPI 64 bit bare metal")

message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

message("\n** Setting up ${PROJECT_NAME} **\n")

set(DEPENDENCY 07-generalization)
set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)

create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
