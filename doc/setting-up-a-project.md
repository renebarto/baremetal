# Setting up a project for building and debugging

Configuration for a project is largely similar for Windows and Linux, however the way we build and debug is slightly different.

For now, let's set up a project that simply prints something to the console (UART 1 in this case), and then halts the system.

This will be quite an extensive chapter, so bare with me. As soon as we have configured the basic project, adding new code and applications should become easier.
Also, the way we configure the project is very verbose and direct, which we will improve later on.

First, we create a folder for the project. Let's say `D:\Projects\tutorial\01-building` on Windows and `~/tutorial/01-building` on Linux.

In this directory, we first need to create a CMake file, which is named `CMakeLists.txt`. Be careful about the 's' in Lists, and also make sure you have the correct casing, especially in Linux.
As soon as you add this file in Visual Studio, it may detect this is a CMake project and try to configure it. This will fail as we don't have the correct contents yet. Don't worry about this for now.

## Create project

In this file we will first create a project:

```cmake
cmake_minimum_required(VERSION 3.18)

message(STATUS "CMake ${CMAKE_VERSION}")

project(01-building
    DESCRIPTION "Application to demonstrate building using CMake"
    LANGUAGES CXX)

```

Short explanation:
- We require a minimum version of 3.18 for CMake. There should always be a similar line in the main CMake scripts
- We echo the current version of CMake
- We define a project named `01-building`, give it a short description, and specify that it will use C++ code as language

## Create source file

We now wish to add a source file to the project, so let's create a source file first, and simply call it `main.cpp`. The contents will be:

```cpp
int main()
{
    return 0;
}
```

For now the application does nothing but return 0. Notice that we have created a main function that returns and `int` and takes no parameters. As we are running a baremetal application, there is no way to specify parameters, except through the kernel parameters file.

## Add source to project

We will add the source file to the project by defining an executable target:

```cmake
cmake_minimum_required(VERSION 3.18)

message(STATUS "CMake ${CMAKE_VERSION}")

project(01-building
    DESCRIPTION "Application to demonstrate building using CMake"
    LANGUAGES CXX)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    )
set(PROJECT_INCLUDES_PUBLIC)
set(PROJECT_INCLUDES_PRIVATE
    )

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
```

Short explanation:
- We define a variable named `PROJECT_SOURCES` that contains the path to our source file (`CMAKE_CURRENT_SOURCE_DIR` is the current source directory, so `main.cpp` will be in the same directory as `CMakeLists.txt`)
- We define two more variables to contain header files, which are for now empty, `PROJECT_INCLUDES_PUBLIC` and `PROJECT_INCLUDES_PRIVATE`.
- We create a so-called target in CMake for an executable, with name `PROJECT_NAME` (this is a standard CMake variable denoting the name of the project were in, so in this case `01-building`)
  - This target will build from the source files and headers just specified.

You will now be able to build the project, however this will be targeting the platform you are running on.
So it will build a Windows application `01-building.exe` for Windows, and a Linux application `01-building` on Linux.
We'll get to building once we can target the correct platform.

## Build for target

### Toolchain file

In order to target the correct platform, we will need to use the toolchain we downloaded. We do this by adding a so-called toolchain file to CMake.
This file will be named `baremetal.toolchain` and have the following contents:

```cmake
include(CMakeForceCompiler)

if ("$ENV{BAREMETAL_TOOLCHAIN_ROOT}" STREQUAL "")
    if (CMAKE_HOST_UNIX)
        set(TOOLCHAIN_ROOT "/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf")
    else()
        set(TOOLCHAIN_ROOT "F:/toolchains/arm-gnu-toolchain-12.3.rel1-mingw-w64-i686-aarch64-none-elf")
    endif()
else()
    set(TOOLCHAIN_ROOT $ENV{BAREMETAL_TOOLCHAIN_ROOT})
endif()

set(PLATFORM_BAREMETAL TRUE)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_SYSTEM_NAME Generic)
set(TOOL_DESTINATION_PLATFORM aarch64-none-elf)

message(STATUS "TOOLCHAIN_ROOT           ${TOOLCHAIN_ROOT}")

set(CMAKE_VERBOSE_MAKEFILE ON)

set(TOOLCHAIN_PATH ${TOOLCHAIN_ROOT}/bin)
set(TOOLCHAIN_AUXILIARY_PATH ${TOOLCHAIN_ROOT}/lib/gcc/${TOOL_DESTINATION_PLATFORM}/12.3.1)

if (CMAKE_HOST_UNIX)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_C_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc CACHE FILEPATH "C compiler" FORCE)

    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++ CACHE FILEPATH "C++ compiler" FORCE)
    set(CMAKE_CXX_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++ CACHE FILEPATH "C++ compiler" FORCE)

    set(CMAKE_AR ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ar CACHE FILEPATH "Library creator" FORCE)

    set(CMAKE_LINKER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ld CACHE FILEPATH "Linker" FORCE)

    set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-objcopy CACHE FILEPATH "ObjCopy tool" FORCE)
else()
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc.exe CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_C_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc.exe CACHE FILEPATH "C compiler" FORCE)

    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++.exe CACHE FILEPATH "C++ compiler" FORCE)
    set(CMAKE_CXX_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++.exe CACHE FILEPATH "C++ compiler" FORCE)

    set(CMAKE_AR ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ar.exe CACHE FILEPATH "Library creator" FORCE)

    set(CMAKE_LINKER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ld.exe CACHE FILEPATH "Linker" FORCE)

    set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-objcopy.exe CACHE FILEPATH "ObjCopy tool" FORCE)
endif()

set(STDDEF_INCPATH ${TOOLCHAIN_AUXILIARY_PATH}/include)

message(STATUS "Processor                ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Platform tuple           ${TOOL_DESTINATION_PLATFORM}")
message(STATUS "Assembler                ${CMAKE_ASM_COMPILER}")
message(STATUS "C compiler               ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler             ${CMAKE_CXX_COMPILER}")
message(STATUS "Archiver                 ${CMAKE_AR}")
message(STATUS "Linker                   ${CMAKE_LINKER}")
message(STATUS "ObjCopy                  ${CMAKE_OBJCOPY}")
message(STATUS "Std include path         ${STDDEF_INCPATH}")

if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "")
	set(HAVE_AUX_PATH false)
else()
	list(FIND ${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH} HAVE_AUX_PATH)
endif()
message(STATUS "CMAKE_EXE_LINKER_FLAGS=  ${CMAKE_EXE_LINKER_FLAGS}")
if (NOT HAVE_AUX_PATH)
	message(STATUS "Adding to CMAKE_EXE_LINKER_FLAGS -L${TOOLCHAIN_AUXILIARY_PATH}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH}" CACHE INTERNAL "" FORCE)
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

A bit of explanation is in order.

#### Part 1

```cmake
include(CMakeForceCompiler)

if ("$ENV{BAREMETAL_TOOLCHAIN_ROOT}" STREQUAL "")
    if (CMAKE_HOST_UNIX)
        set(TOOLCHAIN_ROOT "/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf")
    else()
        set(TOOLCHAIN_ROOT "F:/toolchains/arm-gnu-toolchain-12.3.rel1-mingw-w64-i686-aarch64-none-elf")
    endif()
else()
    set(TOOLCHAIN_ROOT $ENV{BAREMETAL_TOOLCHAIN_ROOT})
endif()

set(PLATFORM_BAREMETAL TRUE)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(TOOL_DESTINATION_PLATFORM aarch64-none-elf)

message(STATUS "TOOLCHAIN_ROOT           ${TOOLCHAIN_ROOT}")

set(CMAKE_VERBOSE_MAKEFILE ON)
```

Here, we include a CMake script to enable forcing the compiler. We need to be able to do this to override the default compiler.
The next part checks if there is an environment variable `BAREMETAL_TOOLCHAIN_ROOT` set to define the location of the toolchain,
and otherwise fall back to a default, different for Windows and Linux of course.

We also set a number of variables:
- `PLATFORM_BAREMETAL` for convenience later on
- `CMAKE_SYSTEM_NAME` which is a standard variable to denote the system we're going to build. For baremetal projects this must be set to `Generic`
- `CMAKE_SYSTEM_PROCESSOR` which is a standard variable to define the processor architecture we're going to build for. In all cases this will be a 64 bit ARM processor, for which the architecture name is `aarch64`
- `TOOL_DESTINATION_PLATFORM` is the so called target triplet / quadruplet. It defines the combination of target architecture, vendor if needed, the operating system, and the build type.
In our case this is `aarch64-none-elf` meaning a 64 bit ARM architecture, with no OS, and with elf output files

We also print the used toolchain root, and set CMAKE build output to be more verbose.

#### Part 2

The next part defines the tools to be used, such as the compiler, linker, etc.:

```cmake
set(TOOLCHAIN_PATH ${TOOLCHAIN_ROOT}/bin)
set(TOOLCHAIN_AUXILIARY_PATH ${TOOLCHAIN_ROOT}/lib/gcc/${TOOL_DESTINATION_PLATFORM}/13.2.1)

if (CMAKE_HOST_UNIX)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_C_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc CACHE FILEPATH "C compiler" FORCE)

    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++ CACHE FILEPATH "C++ compiler" FORCE)
    set(CMAKE_CXX_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++ CACHE FILEPATH "C++ compiler" FORCE)

    set(CMAKE_AR ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ar CACHE FILEPATH "Library creator" FORCE)

    set(CMAKE_LINKER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ld CACHE FILEPATH "Linker" FORCE)

    set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-objcopy CACHE FILEPATH "ObjCopy tool" FORCE)
else()
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc.exe CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_C_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc.exe CACHE FILEPATH "C compiler" FORCE)

    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++.exe CACHE FILEPATH "C++ compiler" FORCE)
    set(CMAKE_CXX_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++.exe CACHE FILEPATH "C++ compiler" FORCE)

    set(CMAKE_AR ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ar.exe CACHE FILEPATH "Library creator" FORCE)

    set(CMAKE_LINKER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ld.exe CACHE FILEPATH "Linker" FORCE)

    set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-objcopy.exe CACHE FILEPATH "ObjCopy tool" FORCE)
endif()

set(STDDEF_INCPATH ${TOOLCHAIN_AUXILIARY_PATH}/include)

message(STATUS "Processor                ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Platform tuple           ${TOOL_DESTINATION_PLATFORM}")
message(STATUS "Assembler                ${CMAKE_ASM_COMPILER}")
message(STATUS "C compiler               ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler             ${CMAKE_CXX_COMPILER}")
message(STATUS "Archiver                 ${CMAKE_AR}")
message(STATUS "Linker                   ${CMAKE_LINKER}")
message(STATUS "ObjCopy                  ${CMAKE_OBJCOPY}")
message(STATUS "Std include path         ${STDDEF_INCPATH}")
```

First, two more variables are defined

- `TOOLCHAIN_PATH` which is the actual location of the tools
- `TOOLCHAIN_AUXILIARY_PATH` which is the location of auxiliary libraries used for building

Then depending on the build platform, we define the tools to be used. The part at the end `CACHE FILEPATH "text" FORCE` simply means that the variable is enforced into the CMake cache

- `CMAKE_C_COMPILER` the path to the C compiler (this is a gcc compiler)
- `CMAKE_C_COMPILER_FORCED` the path to the C compiler, but forced to be this compiler
- `CMAKE_CXX_COMPILER` the path to the C++ compiler (this could be gcc as well, but it's common to use g++)
- `CMAKE_CXX_COMPILER_FORCED` the path to the C++ compiler, but forced to be this compiler
- `CMAKE_AR` the path to the archiver, in other words the static library linker
- `CMAKE_LINKER` the path to the linker, which links executables and dynamic libraries
- `CMAKE_OBJCOPY` the path to the object copier, which we will need to create an image

Be aware that we did not set the assembler here, even though it will be used. Normally, gcc is also able to compile assembly code.

We also defined the variable `STDDEF_INCPATH` for the standard include path

Next we print all the variables just defined.

#### Part 3

```cmake
if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "")
	set(HAVE_AUX_PATH false)
else()
	list(FIND ${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH} HAVE_AUX_PATH)
endif()
message(STATUS "CMAKE_EXE_LINKER_FLAGS=  ${CMAKE_EXE_LINKER_FLAGS}")
if (NOT HAVE_AUX_PATH)
	message(STATUS "Adding to CMAKE_EXE_LINKER_FLAGS -L${TOOLCHAIN_AUXILIARY_PATH}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH}" CACHE INTERNAL "" FORCE)
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

First, the variable `CMAKE_EXE_LINKER_FLAGS` which is a standard CMake variable to hold the linker flags, is extended, if not already done, with the auxiliary libary directory.

Lastly, we need to set some more standard CMake variable:

- `CMAKE_FIND_ROOT_PATH_MODE_PROGRAM` to signal not to look for executables in the path just specfied
- `CMAKE_FIND_ROOT_PATH_MODE_LIBRARY` to signal to look for libraries in the path just specfied
- `CMAKE_FIND_ROOT_PATH_MODE_INCLUDE` to signal to look for include in the path just specfied
- `CMAKE_FIND_ROOT_PATH_MODE_PACKAGE` to signal to look for packages in the path just specfied

### CMake functions

### CMake script extension

### Linker description file


We now need to use this toolchain file.

### Windows

### Linux

On the command line, first configure the build:

```bat
cd <project_dir>
mkdir cmake-build
cd cmake-build
cmake ../tutorial/01-building -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH="../tutorial/01-building/baremetal.toolchain"
```

Output:

```text
-- CMake 3.27.8
-- TOOLCHAIN_ROOT           D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler                D:/Toolchains/arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
-- C compiler               D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
-- C++ compiler             D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-g++.exe
-- Archiver                 D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ar.exe
-- Linker                   D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ld.exe
-- ObjCopy                  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-objcopy.exe
-- Std include path         D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1/include
-- CMAKE_EXE_LINKER_FLAGS=   -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: D:/Projects/baremetal.github/cmake-build
```

There may be warnings about unused variables, you can ignore these.

Next perform the actual build

```bat
cd ../tutorial/01-building
cmake --build ../../cmake-build
```

```output
[2/2] Linking CXX executable demo
FAILED: demo
cmd.exe /C "cd . && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1 CMakeFiles/demo.dir/main.cpp.obj -o demo   && cd ."
D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1\libg.a(libc_a-exit.o): in function `exit':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/stdlib/exit.c:65:(.text.exit+0x2c): undefined reference to `_exit'
D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1\libg.a(libc_a-closer.o): in function `_close_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/closer.c:47:(.text._close_r+0x1c): undefined reference to `_close'
D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1\libg.a(libc_a-lseekr.o): in function `_lseek_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/lseekr.c:49:(.text._lseek_r+0x24): undefined reference to `_lseek'
D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1\libg.a(libc_a-readr.o): in function `_read_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/readr.c:49:(.text._read_r+0x24): undefined reference to `_read'
D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1\libg.a(libc_a-writer.o): in function `_write_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/writer.c:49:(.text._write_r+0x24): undefined reference to `_write'
D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1\libg.a(libc_a-sbrkr.o): in function `_sbrk_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/sbrkr.c:51:(.text._sbrk_r+0x1c): undefined reference to `_sbrk'
collect2.exe: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.
```

As you can see, in both cases, the source file is compiled, but the link stage failed due to some undefined references. This is due to the fact that the compiler uses the C standard library, but this leaves a number of platform specific functions undefined.
We therefore need to set compiler options the correct way to really build a baremetal `standalone` application.
 
