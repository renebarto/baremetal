# Baremetal defines and options

## CMake setting controlled defines and options

There are some defines that influence how the Baremetal software is built. These can be influenced by settings at CMake level

| CMake variable   | #define affected     | Compiler options affected | Comments            |
|------------------|----------------------|---------------------------|---------------------|
| CMAKE_BUILD_TYPE<br/>Debug<br/>Release<br/>RelWithDebInfo<br/>MinSizeRel | <br/>_DEBUG</br>NDEBUG<br/>NDEBUG<br/>NDEBUG | <br/>-g -O0<br/>-O3<br/>-Os -O3<br/>-g -O2      | Determines Debug / Release / RelWithDebInfo / MinSizeRel build
| VERBOSE_BUILD    | -                    | -                         | Adds verbosity to the build
| BAREMETAL_TARGET | BAREMETAL_RPI_TARGET | -mcpu                     | Determines type of platform to build for:<br/>BAREMETAL_TARGET=RPI3 -> BAREMETAL_RPI_TARGET=3<br/>BAREMETAL_TARGET=RPI4 -> BAREMETAL_RPI_TARGET=4
| BUILD_TUTORIALS  | -                    | -                         | If on, also builds all tutorials

## Important defines

| #define              | Comments            |
|----------------------|---------------------|
| PLATFORM_BAREMETAL   | Alwyas defined, indicates we are building for baremetal
| BAREMETAL_RPI_TARGET | Determines type of platform to build for:<br/>3: Raspberry Pi 3<br/>4: Raspberry Pi 4
| USE_PHYSICAL_COUNTER | Determines whether to use BCM2835 System Timer instead of ARM timer:<br/>not defined: User ARM timer<br/>defined: USe BCM2835 System Timer
