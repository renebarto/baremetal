# Baremetal defines and options {#BAREMETAL_DEFINES_AND_OPTIONS}

## CMake setting controlled defines and options {#BAREMETAL_DEFINES_AND_OPTIONS_CMAKE_SETTING_CONTROLLED_DEFINES_AND_OPTIONS}

There are some defines that influence how the Baremetal software is built. These can be influenced by settings at CMake level

### CMAKE_BUILD_TYPE {#BAREMETAL_DEFINES_AND_OPTIONS_CMAKE_SETTING_CONTROLLED_DEFINES_AND_OPTIONS_CMAKE_BUILD_TYPE}

Purpose:
Determines Debug / Release / RelWithDebInfo / MinSizeRel build

Affects defines:
- Debug -> _DEBUG
- Release -> NDEBUG
- RelWithDebInfo-> NDEBUG
- MinSizeRel -> NDEBUG

Affects compiler options:
- Debug -> -g -O0
- Release -> -O3
- RelWithDebInfo-> -Os -O3
- MinSizeRel -> -g -O2

### VERBOSE_BUILD {#BAREMETAL_DEFINES_AND_OPTIONS_CMAKE_SETTING_CONTROLLED_DEFINES_AND_OPTIONS_VERBOSE_BUILD}

Purpose:
Adds verbosity to the build

Affects defines:
- None

Affects compiler options:
- None

### BAREMETAL_TARGET {#BAREMETAL_DEFINES_AND_OPTIONS_CMAKE_SETTING_CONTROLLED_DEFINES_AND_OPTIONS_BAREMETAL_TARGET}

Purpose:
Determines type of platform to build for:

Affects defines:
- BAREMETAL_TARGET=RPI3 -> BAREMETAL_RPI_TARGET=3
- BAREMETAL_TARGET=RPI4 -> BAREMETAL_RPI_TARGET=4

Affects compiler options:
- -mcpu

### BUILD_TUTORIALS {#BAREMETAL_DEFINES_AND_OPTIONS_CMAKE_SETTING_CONTROLLED_DEFINES_AND_OPTIONS_BUILD_TUTORIALS}

Purpose:
If on, also builds all tutorials

Affects defines:
- None

Affects compiler options:
- None

## Important defines {#BAREMETAL_DEFINES_AND_OPTIONS_IMPORTANT_DEFINES}

### PLATFORM_BAREMETAL {#BAREMETAL_DEFINES_AND_OPTIONS_IMPORTANT_DEFINES_PLATFORM_BAREMETAL}

Always defined, indicates we are building for baremetal

### BAREMETAL_RPI_TARGET {#BAREMETAL_DEFINES_AND_OPTIONS_IMPORTANT_DEFINES_BAREMETAL_RPI_TARGET}

Determines type of platform to build for:
- 3: Raspberry Pi 3
- 4: Raspberry Pi 4

### BAREMETAL_CONSOLE_UART0 {#BAREMETAL_DEFINES_AND_OPTIONS_IMPORTANT_DEFINES_BAREMETAL_CONSOLE_UART0}

Makes UART0 the default console device.

### BAREMETAL_CONSOLE_UART1 {#BAREMETAL_DEFINES_AND_OPTIONS_IMPORTANT_DEFINES_BAREMETAL_CONSOLE_UART1}

Makes UART1 the default console device.

### USE_PHYSICAL_COUNTER {#BAREMETAL_DEFINES_AND_OPTIONS_IMPORTANT_DEFINES_USE_PHYSICAL_COUNTER}

Determines whether to use BCM2835 System Timer instead of ARM timer:
- not defined: Use ARM timer
- defined: Use BCM2835 System Timer
