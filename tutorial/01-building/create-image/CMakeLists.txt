project(demo-image
    DESCRIPTION "Kernel image for 01-building RPI 64 bit bare metal")

message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

message("\n** Setting up ${PROJECT_NAME} **\n")

set(BAREMETAL_TARGET_KERNEL kernel8)
set(DEPENDENCY demo)
set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)

create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
