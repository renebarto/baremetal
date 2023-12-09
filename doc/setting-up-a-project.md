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
        set(TOOLCHAIN_ROOT "/home/rene/toolchains/arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-elf")
    else()
        set(TOOLCHAIN_ROOT "D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf")
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
        set(TOOLCHAIN_ROOT "/home/rene/toolchains/arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-elf")
    else()
        set(TOOLCHAIN_ROOT "D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf")
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
The next part checks if there is an existing environment variable `BAREMETAL_TOOLCHAIN_ROOT` set to define the location of the toolchain,
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

The variable `CMAKE_EXE_LINKER_FLAGS` which is a standard CMake variable to hold the linker flags, is extended, if not already done, with the auxiliary libary directory.
This contains a bit of CMake trickery, don't worry about the details.

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

On the command line, first configure the build:

```bat
cd <project_dir>
del /s /f /q cmake-build/*.*
rmdir cmake-build
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/01-building -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/01-building/baremetal.toolchain
popd
```

Output:

```text
-- CMake 3.27.8
-- TOOLCHAIN_ROOT           D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
-- C++ compiler             D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-g++.exe
-- Archiver                 D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ar.exe
-- Linker                   D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ld.exe
-- ObjCopy                  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-objcopy.exe
-- Std include path         D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1/include
-- CMAKE_EXE_LINKER_FLAGS=
-- Adding to CMAKE_EXE_LINKER_FLAGS -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1
-- TOOLCHAIN_ROOT           D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
-- C++ compiler             D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-g++.exe
-- Archiver                 D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ar.exe
-- Linker                   D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ld.exe
-- ObjCopy                  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-objcopy.exe
-- Std include path         D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1/include
-- CMAKE_EXE_LINKER_FLAGS=   -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1
-- The CXX compiler identification is GNU 13.2.1
-- Configuring done (0.5s)
-- Generating done (0.0s)
-- Build files have been written to: D:/Projects/baremetal.github/cmake-build
```

There may be warnings about unused variables, you can ignore these.

Next perform the actual build

```bat
set ROOT=%CD%
pushd tutorial/01-building
cmake --build %ROOT%/cmake-build --target 01-building
popd
```

```output
[2/2] Linking CXX executable 01-building
FAILED: 01-building
cmd.exe /C "cd . && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1 CMakeFiles/01-building.dir/main.cpp.obj -o 01-building   && cd ."
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

As you can see, in both cases, the source file is compiled, but the link stage failed due to some undefined references.
This is due to the fact that the compiler uses the C standard library, but this leaves a number of platform specific functions undefined.
We therefore need to set compiler options the correct way to really build a baremetal `standalone` application.
 

### Linux

On the command line, first configure the build:

```bat
cd <project_dir>
rm -rf cmake-build/
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/01-building -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/01-building/baremetal.toolchain
popd
```

Output:

```text
-- CMake 3.25.1
-- TOOLCHAIN_ROOT           /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
-- C++ compiler             /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-g++
-- Archiver                 /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ar
-- Linker                   /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ld
-- ObjCopy                  /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-objcopy
-- Std include path         /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1/include
-- CMAKE_EXE_LINKER_FLAGS=
-- Adding to CMAKE_EXE_LINKER_FLAGS -L/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1
-- TOOLCHAIN_ROOT           /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
-- C++ compiler             /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-g++
-- Archiver                 /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ar
-- Linker                   /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ld
-- ObjCopy                  /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-objcopy
-- Std include path         /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1/include
-- CMAKE_EXE_LINKER_FLAGS=   -L/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1
-- The CXX compiler identification is GNU 13.2.1
-- The ASM compiler identification is GNU
-- Found assembler: /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
-- Configuring done
-- Generating done
-- Build files have been written to: /home/rene/repo/baremetal.github/cmake-build
```

There may be warnings about unused variables, you can ignore these.

Next perform the actual build

```bat
cd ../tutorial/01-building
cmake --build ../../cmake-build
```

```output
[2/2] Linking CXX executable 01-building
FAILED: 01-building
: && /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-g++ -g -L/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1 CMakeFiles/01-building.dir/main.cpp.obj -o 01-building   && :
/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-exit.o): in function `exit':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/stdlib/exit.c:65:(.text.exit+0x2c): undefined reference to `_exit'
/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-writer.o): in function `_write_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/writer.c:49:(.text._write_r+0x24): undefined reference to `_write'
/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-closer.o): in function `_close_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/closer.c:47:(.text._close_r+0x1c): undefined reference to `_close'
/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-lseekr.o): in function `_lseek_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/lseekr.c:49:(.text._lseek_r+0x24): undefined reference to `_lseek'
/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-readr.o): in function `_read_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/readr.c:49:(.text._read_r+0x24): undefined reference to `_read'
/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-sbrkr.o): in function `_sbrk_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/sbrkr.c:51:(.text._sbrk_r+0x1c): undefined reference to `_sbrk'
collect2: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.
```

As you can see, in both cases, the source file is compiled, but the link stage failed due to some undefined references. This is due to the fact that the compiler uses the C standard library, but this leaves a number of platform specific functions undefined.
We therefore need to set compiler options the correct way to really build a baremetal `standalone` application.
 
## Compiler settings

### Adding project variables

We need to add a few lines to our CMakeLists file, to add definitions, compiler settings, linker options, and link libraries

```cmake
project(01-building
    DESCRIPTION "Application to demonstrate building using CMake"
    LANGUAGES CXX)

set(PROJECT_TARGET_NAME ${PROJECT_NAME}.elf)

set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE )
set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )

set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE
    -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
    )
set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE )

set(PROJECT_INCLUDE_DIRS_PRIVATE )
set(PROJECT_INCLUDE_DIRS_PUBLIC )

set(PROJECT_LINK_OPTIONS ${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nostartfiles -T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld)

set(PROJECT_DEPENDENCIES )

set(PROJECT_LIBS
    ${PROJECT_DEPENDENCIES}
    )
set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    )

set(PROJECT_INCLUDES_PUBLIC )
set(PROJECT_INCLUDES_PRIVATE )

if (CMAKE_HOST_UNIX)
    set(START_GROUP -Wl,--start-group)
    set(END_GROUP -Wl,--end-group)
endif()
```

So, after the project is defined, we add the following lines:
- We define the variable `PROJECT_TARGET_NAME`, which set the file for our executable to `01-building.elf`
- We define the variables `PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE` and `PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC` which will contain compiler definitions. For now these are empty. There are two, as we can have definitions only for this executable (private) and possibly exported to other targets (public). As an executabel file normally does not export anything, this is a bit superfluous, but keeping this structure will prove helpful later on.
- We define the variables `PROJECT_COMPILE_OPTIONS_CXX_PRIVATE` and `PROJECT_COMPILE_OPTIONS_CXX_PRIVATE` in the same way to set compiler options. Here we set the compiler options to be:
  - -Wall: Set warning level to the highest possible level
  - -O2: Optimize to almost maximum level
  - -ffreestanding: A very important option, meaning we are building a standalone (baremetal) application
  - -nostdinc: Do not use the standard C library includes
  - -nostdlib: Do not use the standard C libraries
  - -nostartfiles: Do not use the standard startup files (`crtbegin.o` and `crtend.o`)
- We defines the variables `PROJECT_INCLUDE_DIRS_PRIVATE` and `PROJECT_INCLUDE_DIRS_PUBLIC` again in the same way to specific include directories
- We define the variable `PROJECT_LINK_OPTIONS` to specify linker options
  - ${CMAKE_EXE_LINKER_FLAGS}: Use the existing linker options (the linker options specified in the [toolchain file](#Toolchain-file))
  - -nostdlib: Do not use the standard C libraries
  - -nostartfiles: Do not use the standard startup files (`crtbegin.o` and `crtend.o`)
  - -T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld: Use the specified linker definition file
- We define a variable `PROJECT_DEPENDENCIES` to hold any libraries we will be depending on. For now this is empty.
- We define the variable `PROJECT_LIBS` to hold all libraries we will be linking to. This means all dependencies, and all specified standard libraries..
- We define two extra variables, only understood by gcc, to group libraries together for correct resolution. These are for the start of the grouping `START_GROUP` and the end of the grouping `END_GROUP`.

### Setting up target

We then need to link these variables to the target we're building:

```cmake
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})

target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE})
target_compile_definitions(${PROJECT_NAME} PUBLIC  ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC})
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE})
target_compile_options(${PROJECT_NAME} PUBLIC  ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC})
```

Explanation:
- We link to the specified libraries (empty list for now) in a group. Hence `${START_GROUP} ${PROJECT_LIBS} ${END_GROUP}`
- We specify include directories for both private and public use
- We specify compiler definitions for both private and public use
- We specify compiler options for both private and public use

Next we specify the linker options. As the options are specified as a list separated by semicolons, and we need to create a string of values separated by spaces, we need to use a custom function:

```cmake
list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
message(STATUS "PROJECT_LINK_OPTIONS=${PROJECT_LINK_OPTIONS_STRING}")
if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
endif()
```

Explanation:
- The first line converts the list to a string with spaces are delimiter using the custom functions `list_to_string`
- The second line prints the string
- The fourth line sets the linker flags, only if the string is not empty

We will need to define this custom function later on.

Lastly, we set the executable file name, and the location of executable files and libraries:

```cmake
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/lib)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin)
```

Explanation:
- The first line sets the executable name `OUTPUT_NAME`, a standard property
- The second line defines the location for static libraries `ARCHIVE_OUTPUT_DIRECTORY`, a standard property
- The third line defines the location for executables `RUNTIME_OUTPUT_DIRECTORY`, a standard property

The last two lines use the variables `OUTPUT_BASE_DIR` and `CONFIG_DIR`. It is common practice to collect output files together in a binaries tree.
`OUTPUT_BASE_DIR` is the root of this tree. `CONFIG_DIR` is the configuration we're building for. CMake supports 4 configurations by default:
- Debug: Debug build (with debug symbols)
- Release: Release build (no symbols)
- RelWithDebInfo: Release build with debug symbols)
- MinSizeRel: Release with minimum size optimization

When building, we can set the configuration using the standard CMake variable `CMAKE_BUILD_TYPE`, as we did earlier setting it to Debug.

### Setting up for custom CMake modules and binary tree

So, we still need to define these two variables, and make the custom function available.

```cmake
cmake_minimum_required(VERSION 3.18)

message(STATUS "CMake ${CMAKE_VERSION}")

set(SCRIPTS_DIR "" CACHE STRING "CMake scripts path")
set(CONFIG_DIR Debug)
set(DEPLOYMENT_DIR ${CMAKE_SOURCE_DIR}/../../deploy)
set(OUTPUT_BASE_DIR "${CMAKE_SOURCE_DIR}/../../output" CACHE STRING "Output directory")

if ("${SCRIPTS_DIR}" STREQUAL "")
    set(SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/cmake" CACHE STRING "CMake scripts path" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${SCRIPTS_DIR})
```

Explanation:
- We define a variable `SCRIPTS_DIR` to hold the path to CMake scripts we will be adding (to contain the custom functions)
- We set the `CONFIG_DIR` variable mentioned above. For now we'll simply set it to Debug
- We define a variable `DEPLOYMENT_DIR` to point to the location where our final image will be. This could be the sample path as the output directory, however it makes sense to separate intermediate binaries from the final images.
- We set the root for the intermediate binaries `OUTPUT_BASE_DIR`
- We set, if not done yet, the `SCRIPTS_DIR` variable to the location of our custom CMake scripts. It is custom practice to create a subfolder `cmake` and place the scripts there.
- Lastly, we add the `SCRIPTS_DIR` to the standard CMake path for CMake modules, `CMAKE_MODULE_PATH`

### Adding custom CMake module

Now, we still need to add the custom CMake script. So we create a folder `cmake` and underneath create a file `functions.cmake`. It is standard practice to name CMake scripts with extension `.cmake`.

The contents of the file are:

```cmake
function(list_to_string in out)
    set(tmp "")
    foreach(VAL ${${in}})
        string(APPEND tmp "${VAL} ")
    endforeach()
    set(${out} "${tmp}" PARENT_SCOPE)
endfunction()
```

Without going into too much detail. This function is called `list_to_string`, and has one input and one output parameter.
It cycles through the parts of the input variable, and creates a string by adding each part followed by a space.
Finally it sets the output variable. The `PARENT_SCOPE` is needed to convert the value to outside the function.

Finally we need to be able to use the function. For this, we need to include the module:

```cmake
list(APPEND CMAKE_MODULE_PATH ${SCRIPTS_DIR})

include(functions)
```

As you can see, we just name the script, without path or extension. This is possible, because we already added the path to the standard CMake module paths `CMAKE_MODULE_PATH`, and because we're using the stanrd CMake module extension `.cmake`

### Adding linker definition file

The last thing we need to do is specify the linker definitions file as pointed to by the [linker options](#Setting-up-target).

This file is named `link.ld` and contains the following:

```text
SECTIONS
{
    . = 0x80000;
    .text : { *(.text.boot) }

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
```

Without going into detail, this file specifies what sections to include in the executable, and what its start address is (0x80000).
So in this case, we only use the sections .text, and and .text.boot sections, the rest is left out.

### Creating an image

Now we can build the code to generate output/Debug/bin/01-building.elf, however that application cannot simply be run in e.g. QEMU. We need to create an image for that. This is a fairly simple step, adding a new target for the image.

We will create a subdirectory `create-image` underneath our current project, and add a `CMakeLists.txt` file there:

```cmake
project(01-building-image
    DESCRIPTION "Kernel image for 01-building RPI 64 bit bare metal")

message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

message("\n** Setting up ${PROJECT_NAME} **\n")

set(BAREMETAL_TARGET_KERNEL kernel8)
set(DEPENDENCY 01-building)
set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)

create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
```

This defines a new project `01-building-image` with creates some variables, and the calls another custom function:

- We create the variable `BAREMETAL_TARGET_KERNEL` to specify the kernel image to create. This depends on the target platform. Here we create kernel8.img for Raspberry Pi 3 (see [System startup](system-startup#config.txt)).
- We create the variable `DEPENDENCY` to specify the project we are going to create the image for.
- We create the variable `IMAGE_NAME` to denote the complete filename of the image.

We then call the custom function `create_image` to create a target for the image.

The customer function is added to the `functions.cmake` module:

```cmake
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
        COMMAND ${CMAKE_OBJCOPY} ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin/${TARGET_NAME} -O binary ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
        DEPENDS ${project}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(${target} ALL DEPENDS 
        ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
        )
endfunction()
```

The function `create_image` takes three parameters:
- The target to be created, in this case `01-building-image`
- The image filename, in this case `kernel8.img`
- The target that creates the application to be added to the image, in this case `01-building.elf`

Explanation:
- The function first shows how it was called, and then checks whether the application target exists, and prints an error if not
- The `OUTPUT_NAME` property from the application target is retrieved, so in this case `01-building.elf` and stored in variable `TARGET_NAME`
- The property value is printed
- The action to be taken is printed
- A custom CMake command is created
  - Its output is `${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}`. This uses the variables `DEPLOYMENT_DIR` and `CONFIG_DIR` defined before, and then adds the name of the application project as a directory, and then the image name.
So the final path will be `deploy/Debug/01-building/kernel8.img`
  - The command to be performed used the `CMAKE_OBJCOPY` tool specified in the toolchain file.
The actual command run will be `aarch64-none-elf-objcopy output/Debg/bin/01-building.elf -O binary deploy/Debug/01-building/kernel8.img`
- A custom CMake target `01-building-image` is created, that depends on the output of the command just created.

This may all seem complex, but this functionality can be used again later on by simply changing the parameters.

Now we still need to use the new target. We simply do this by referring to the subdirectory containing the new `CMakeLists.txt`.

Remembed that our current project folder, `tutorial/01-building` contains a `CMakeLists.txt` file, we add the following line to this file to include its `create-image` subdirectory. This instructs CMake to also use the `CMakeLists.txt` file in this directory:

```cmake
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/lib)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin)

add_subdirectory(create-image)
```

We can now start to build the application and image.

### Building

We first need to configure the build for CMake:

#### Windows

```bat
del /S /f /q cmake-build\*.*
rmdir cmake-build
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/01-building -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/01-building/baremetal.toolchain
popd
```

#### Linux

```bash
rm -rf cmake-build/
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/01-building -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/01-building/baremetal.toolchain
popd
```

In other words, we clean up the CMake build directory, and recreate it, then we step into this directory, and configure CMake to use tutorial/01-building as the root CMake directory, and use the toolchain file. We are building for Debug.

Then we can build the targets:

#### Windows

```bat
set ROOT=%CD%
pushd tutorial/01-building
cmake --build %ROOT%/cmake-build --target 01-building-image
popd
```

#### Linux

```bash
rootdir=`pwd`
pushd tutorial/01-building
cmake --build $rootdir/cmake-build --target 01-building-image
popd
```

We save the root directory to be able to reference it, and step into the project directory. There we run cmake with the --build parameter to specify the build directory.
The target we're going to build is the image, so `01-building-image`. This will automaticall build all its dependencies, so `01-building.elf` will also be built

After this step, we will have built the application in output/Debug/bin/01-building.elf, and the image in deploy/Debug/01-building-image/kernel8.img

The image is very small, as the application basically does nothing, but you have built your first baremetal application!

### Debugging

To show that the application actually works, let's run it in QEMU and debug it.

#### Windows

To run QEMU:

```bat
"c:\Program Files\qemu\qemu-system-aarch64.exe" -M raspi3b -kernel D:\Projects\baremetal.github\deploy\Debug\01-building-image\kernel8.img -serial stdio -s -S
```

To run GDB:
```bat
cd output\Debug\bin
D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gdb.exe
```

```text
GNU gdb (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 13.2.90.20231008-git
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=i686-w64-mingw32 --target=aarch64-none-elf".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://bugs.linaro.org/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word".
```

#### Linux

To run QEMU:

```bash
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio -s -S
```

To run GDB:
```bash
cd output/Debug/bin
gdb-multiarch
```

```text
GNU gdb (Debian 13.1-3) 13.1
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word".
```

In GDB:
```gdb
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000000000000000 in ?? ()
(gdb) symbol-file 01-building.elf
Reading symbols from 01-building.elf...
(gdb) b main.cpp:1
Breakpoint 1 at 0x80000: file D:/Projects/baremetal.github/tutorial/01-building/main.cpp, line 2.
(gdb) c
Continuing.

Thread 1 hit Breakpoint 1, main () at D:/Projects/baremetal.github/tutorial/01-building/main.cpp:3
3           return 0;
```

So we ended up in line 3 of the main() function:

```cpp
int main()
{
    return 0;
}
```

Next, we close down debugging again, as our application would otherwise keep looping:

```gdb
(gdb) kill
Kill the program being debugged? (y or n) y
[Inferior 1 (process 1) killed]
(gdb) quit
```
