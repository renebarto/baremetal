# Tutorial 02: Setting up a project for building and debugging {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING}

@tableofcontents

Configuration for a project is largely similar for Windows and Linux, however the way we build and debug is slightly different.

For now, let's set up a simple project that simply returns (i.e. does nothing but return 0 in `main()`), and then halts the system.

This will be quite an extensive chapter, so bare with me.
This chapter will include lots of explanation on how to work with CMake, how code is started on the platform, etc.

The way we configure the project is very specific, verbose and direct, which we will improve later on in [Setting up project structure](03-setting-up-project-structure.md).

First, we create a folder for the project. Let's say `D:\Projects\tutorial\02-setting-up-a-project` on Windows and `~/tutorial/02-setting-up-a-project` on Linux.
The project and code are already in this location for the GitHub project, so you can either replicate the steps, or read along.

In this directory, we first need to create a CMake file, which is named `CMakeLists.txt`. Be careful about the 's' in _Lists_, 
and also make sure you have the correct casing, especially in Linux.
Whenever we mention a CMake file, we mean a file named `CMakeLists.txt`.

As soon as you add this file in Visual Studio, it may detect this is a CMake project and try to configure it.
This will fail as we don't have the correct contents yet. Don't worry about this.
We'll get to Visual Studio in [Setting up project structure](03-setting-up-project-structure.md).

## Create project {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATE_PROJECT}

In this file we will first create a project:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
1: cmake_minimum_required(VERSION 3.18)
2: 
3: message(STATUS "CMake ${CMAKE_VERSION}")
4: 
5: project(02-setting-up-a-project
6:     DESCRIPTION "Application to demonstrate building using CMake"
7:     LANGUAGES CXX ASM)
```

Explanation:
- Line 1: We require a minimum version of 3.18 for CMake. There should always be a similar line in the beginning of the main CMake file
- Line 3: We print the current version of CMake
- Line 5-7: We define a project named `02-setting-up-a-project`, give it a short description, and specify that it will use C++ and assembly code as language

## Create source file {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATE_SOURCE_FILE}

We'll now add a source file to the project, so let's create a source file first, and simply call it `main.cpp`. The contents will be:

```cpp
File: tutorial/02-setting-up-a-project/main.cpp
1: int main()
2: {
3:     return 0;
4: }
```

For now the application does nothing but return 0. Notice that we have created a main function that returns and `int` and takes no parameters.
As we are running a baremetal application, there is no way to specify parameters, except through the kernel parameters file.

We will also need some assembly code to correctly initialize the CPU.
This will be discussed later, together with the [linker definition file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE) and the [startup assembly code](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_STARTUP_ASSEMBLY_CODE) section.

## Add source to project {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_ADD_SOURCE_TO_PROJECT}

We will add the source file to the project by defining an executable target:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
1: cmake_minimum_required(VERSION 3.18)
2: 
3: message(STATUS "CMake ${CMAKE_VERSION}")
4: 
5: project(02-setting-up-a-project
6:     DESCRIPTION "Application to demonstrate building using CMake"
7:     LANGUAGES CXX ASM)
8: 
9: set(PROJECT_SOURCES
10:     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
11:     )
12: set(PROJECT_INCLUDES_PUBLIC)
13: set(PROJECT_INCLUDES_PRIVATE
14:     )
15: 
16: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
```

Short explanation:
- Line 9-11: We define a variable named `PROJECT_SOURCES` that contains the path to our source file (`CMAKE_CURRENT_SOURCE_DIR` is the current source directory, so `main.cpp` will be in the same directory as `CMakeLists.txt`)
- Line 12-13: We define two more variables to contain header files, which are for now empty, `PROJECT_INCLUDES_PUBLIC` and `PROJECT_INCLUDES_PRIVATE`.
- Line 16: We create a so-called target in CMake for an executable, with name `PROJECT_NAME` (this is a standard CMake variable denoting the name of the project we're in, so in this case `02-setting-up-a-project`)
  - This target will build from the source files and headers just specified. The term `${X}` means the value of a variable named X.

You will now be able to build the project, however this will be targeting the platform you are currently running on.
So it will build a Windows application `02-setting-up-a-project.exe` for Windows, and a Linux application `02-setting-up-a-project` on Linux.
We'll get to building once we can target the correct platform.

## Build for target {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET}

### Toolchain file {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE}

In order to target the correct platform, we will need to use the toolchain we downloaded. We do this by adding a so-called toolchain file to CMake.
Unlike the CMake file, this file can be named in any way we like, I tend to use the extension `.toolchain` for toolchain files.

This file will be named `baremetal.toolchain` and have the following contents:

```cmake
File: tutorial/02-setting-up-a-project/baremetal.toolchain
1: include(CMakeForceCompiler)
2: 
3: if ("$ENV{BAREMETAL_TOOLCHAIN_ROOT}" STREQUAL "")
4:     if (CMAKE_HOST_UNIX)
5:         set(TOOLCHAIN_ROOT "/opt/toolchains/arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-elf")
6:     else()
7:         set(TOOLCHAIN_ROOT "D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf")
8:     endif()
9: else()
10:     set(TOOLCHAIN_ROOT $ENV{BAREMETAL_TOOLCHAIN_ROOT})
11: endif()
12: 
13: set(PLATFORM_BAREMETAL TRUE)
14: set(CMAKE_SYSTEM_NAME Generic)
15: set(CMAKE_SYSTEM_PROCESSOR aarch64)
16: set(TOOL_DESTINATION_PLATFORM aarch64-none-elf)
17: 
18: message(STATUS "TOOLCHAIN_ROOT           ${TOOLCHAIN_ROOT}")
19: 
20: set(CMAKE_VERBOSE_MAKEFILE ON)
21: 
22: set(TOOLCHAIN_PATH ${TOOLCHAIN_ROOT}/bin)
23: set(TOOLCHAIN_AUXILIARY_PATH ${TOOLCHAIN_ROOT}/lib/gcc/${TOOL_DESTINATION_PLATFORM}/13.2.1)
24: 
25: if (CMAKE_HOST_UNIX)
26:     set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc CACHE FILEPATH "C compiler" FORCE)
27:     set(CMAKE_C_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc CACHE FILEPATH "C compiler" FORCE)
28: 
29:     set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++ CACHE FILEPATH "C++ compiler" FORCE)
30:     set(CMAKE_CXX_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++ CACHE FILEPATH "C++ compiler" FORCE)
31: 
32:     set(CMAKE_AR ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ar CACHE FILEPATH "Library creator" FORCE)
33: 
34:     set(CMAKE_LINKER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ld CACHE FILEPATH "Linker" FORCE)
35: 
36:     set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-objcopy CACHE FILEPATH "ObjCopy tool" FORCE)
37: else()
38:     set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc.exe CACHE FILEPATH "C compiler" FORCE)
39:     set(CMAKE_C_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc.exe CACHE FILEPATH "C compiler" FORCE)
40: 
41:     set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++.exe CACHE FILEPATH "C++ compiler" FORCE)
42:     set(CMAKE_CXX_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++.exe CACHE FILEPATH "C++ compiler" FORCE)
43: 
44:     set(CMAKE_AR ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ar.exe CACHE FILEPATH "Library creator" FORCE)
45: 
46:     set(CMAKE_LINKER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ld.exe CACHE FILEPATH "Linker" FORCE)
47: 
48:     set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-objcopy.exe CACHE FILEPATH "ObjCopy tool" FORCE)
49: endif()
50: 
51: set(STDDEF_INCPATH ${TOOLCHAIN_AUXILIARY_PATH}/include)
52: 
53: message(STATUS "Processor                ${CMAKE_SYSTEM_PROCESSOR}")
54: message(STATUS "Platform tuple           ${TOOL_DESTINATION_PLATFORM}")
55: message(STATUS "Assembler                ${CMAKE_ASM_COMPILER}")
56: message(STATUS "C compiler               ${CMAKE_C_COMPILER}")
57: message(STATUS "C++ compiler             ${CMAKE_CXX_COMPILER}")
58: message(STATUS "Archiver                 ${CMAKE_AR}")
59: message(STATUS "Linker                   ${CMAKE_LINKER}")
60: message(STATUS "ObjCopy                  ${CMAKE_OBJCOPY}")
61: message(STATUS "Std include path         ${STDDEF_INCPATH}")
62: 
63: if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "")
64: 	set(HAVE_AUX_PATH false)
65: else()
66: 	list(FIND ${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH} HAVE_AUX_PATH)
67: endif()
68: message(STATUS "CMAKE_EXE_LINKER_FLAGS=  ${CMAKE_EXE_LINKER_FLAGS}")
69: if (NOT HAVE_AUX_PATH)
70: 	message(STATUS "Adding to CMAKE_EXE_LINKER_FLAGS -L${TOOLCHAIN_AUXILIARY_PATH}")
71: 	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH}" CACHE INTERNAL "" FORCE)
72: endif()
73: 
74: set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
75: set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
76: set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
77: set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

A bit of explanation is in order.

#### Part 1 {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE_PART_1}

```cmake
File: tutorial/02-setting-up-a-project/baremetal.toolchain
1: include(CMakeForceCompiler)
2: 
3: if ("$ENV{BAREMETAL_TOOLCHAIN_ROOT}" STREQUAL "")
4:     if (CMAKE_HOST_UNIX)
5:         set(TOOLCHAIN_ROOT "/home/rene/toolchains/arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-elf")
6:     else()
7:         set(TOOLCHAIN_ROOT "D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf")
8:     endif()
9: else()
10:     set(TOOLCHAIN_ROOT $ENV{BAREMETAL_TOOLCHAIN_ROOT})
11: endif()
12: 
13: set(PLATFORM_BAREMETAL TRUE)
14: set(CMAKE_SYSTEM_NAME Generic)
15: set(CMAKE_SYSTEM_PROCESSOR aarch64)
16: set(TOOL_DESTINATION_PLATFORM aarch64-none-elf)
17: 
18: message(STATUS "TOOLCHAIN_ROOT           ${TOOLCHAIN_ROOT}")
19: 
20: set(CMAKE_VERBOSE_MAKEFILE ON)
```

- Line 1: Here, we include a CMake script to enable forcing the compiler. We need to be able to do this to override the default compiler
- Line 3-11: We check if there is an existing environment variable `BAREMETAL_TOOLCHAIN_ROOT` set to define the location of the toolchain,
and otherwise fall back to a default, different for Windows and Linux of course
- Line 13: We set the variable `PLATFORM_BAREMETAL` for convenience later on
- Line 14: We set the variable `CMAKE_SYSTEM_NAME` which is a standard variable to denote the system we're going to build.
For baremetal projects this must be set to `Generic`
- Line 15: We set the variable `CMAKE_SYSTEM_PROCESSOR` which is a standard variable to define the processor architecture we're going to build for.
In all cases this will be a 64 bit ARM processor, for which the architecture name is `aarch64`
- Line 16: `TOOL_DESTINATION_PLATFORM` is the so called target triplet / quadruplet.
It defines the combination of target architecture, vendor if needed, the operating system, and the build type.
In our case this is `aarch64-none-elf` meaning a 64 bit ARM architecture, with no OS, and with elf output files
-Line 18: We also print the used toolchain root
- Line 20: We set CMAKE build output to be more verbose

#### Part 2 {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE_PART_2}

The next part defines the tools to be used, such as the compiler, linker, etc.:

```cmake
File: tutorial/02-setting-up-a-project/baremetal.toolchain
22: set(TOOLCHAIN_PATH ${TOOLCHAIN_ROOT}/bin)
23: set(TOOLCHAIN_AUXILIARY_PATH ${TOOLCHAIN_ROOT}/lib/gcc/${TOOL_DESTINATION_PLATFORM}/13.2.1)
24: 
25: if (CMAKE_HOST_UNIX)
26:     set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc CACHE FILEPATH "C compiler" FORCE)
27:     set(CMAKE_C_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc CACHE FILEPATH "C compiler" FORCE)
28: 
29:     set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++ CACHE FILEPATH "C++ compiler" FORCE)
30:     set(CMAKE_CXX_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++ CACHE FILEPATH "C++ compiler" FORCE)
31: 
32:     set(CMAKE_AR ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ar CACHE FILEPATH "Library creator" FORCE)
33: 
34:     set(CMAKE_LINKER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ld CACHE FILEPATH "Linker" FORCE)
35: 
36:     set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-objcopy CACHE FILEPATH "ObjCopy tool" FORCE)
37: else()
38:     set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc.exe CACHE FILEPATH "C compiler" FORCE)
39:     set(CMAKE_C_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-gcc.exe CACHE FILEPATH "C compiler" FORCE)
40: 
41:     set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++.exe CACHE FILEPATH "C++ compiler" FORCE)
42:     set(CMAKE_CXX_COMPILER_FORCED ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-g++.exe CACHE FILEPATH "C++ compiler" FORCE)
43: 
44:     set(CMAKE_AR ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ar.exe CACHE FILEPATH "Library creator" FORCE)
45: 
46:     set(CMAKE_LINKER ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-ld.exe CACHE FILEPATH "Linker" FORCE)
47: 
48:     set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/${TOOL_DESTINATION_PLATFORM}-objcopy.exe CACHE FILEPATH "ObjCopy tool" FORCE)
49: endif()
50: 
51: set(STDDEF_INCPATH ${TOOLCHAIN_AUXILIARY_PATH}/include)
52: 
53: message(STATUS "Processor                ${CMAKE_SYSTEM_PROCESSOR}")
54: message(STATUS "Platform tuple           ${TOOL_DESTINATION_PLATFORM}")
55: message(STATUS "Assembler                ${CMAKE_ASM_COMPILER}")
56: message(STATUS "C compiler               ${CMAKE_C_COMPILER}")
57: message(STATUS "C++ compiler             ${CMAKE_CXX_COMPILER}")
58: message(STATUS "Archiver                 ${CMAKE_AR}")
59: message(STATUS "Linker                   ${CMAKE_LINKER}")
60: message(STATUS "ObjCopy                  ${CMAKE_OBJCOPY}")
61: message(STATUS "Std include path         ${STDDEF_INCPATH}")
```

- Line 22: We set the variable `TOOLCHAIN_PATH` which is the actual location of the tools
- Line 23: We set the variable `TOOLCHAIN_AUXILIARY_PATH` which is the location of auxiliary libraries used for building

Then depending on the build platform, we define the tools to be used. 
The part at the end `CACHE FILEPATH "text" FORCE` simply means that the variable is enforced into the CMake cache.

The CMake cache is a file CMake uses to store project variables for later reference.
This file is named `CMakeCache.txt`, and is located in teh CMake build directory (explained later).
- Line 26/38: We set the variable `CMAKE_C_COMPILER` to the path to the C compiler (this is a gcc compiler)
- Line 27/39: We set the variable `CMAKE_C_COMPILER_FORCED` to the path to the C compiler, but forced to be this compiler
- Line 29/41: We set the variable `CMAKE_CXX_COMPILER` to the path to the C++ compiler (this could be gcc as well, but it's common to use g++)
- Line 30/42: We set the variable `CMAKE_CXX_COMPILER_FORCED` to the path to the C++ compiler, but forced to be this compiler
- Line 32/44: We set the variable `CMAKE_AR` to the path to the archiver, in other words the static library linker
- Line 34/46: We set the variable `CMAKE_LINKER` to the path to the linker, which links executables and dynamic libraries
- Line 36/48: We set the variable `CMAKE_OBJCOPY` to the path to the object copier, which we will need to create an image

Notice that we did not set the assembler here, even though it will be used. In our case, gcc is also able to compile assembly code.

- Line 51: We also define the variable `STDDEF_INCPATH` for the standard include path
- Line 53-61: We print all the variables just defined

#### Part 3 {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE_PART_3}

```cmake
File: tutorial/02-setting-up-a-project/baremetal.toolchain
63: if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "")
64: 	set(HAVE_AUX_PATH false)
65: else()
66: 	list(FIND ${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH} HAVE_AUX_PATH)
67: endif()
68: message(STATUS "CMAKE_EXE_LINKER_FLAGS=  ${CMAKE_EXE_LINKER_FLAGS}")
69: if (NOT HAVE_AUX_PATH)
70: 	message(STATUS "Adding to CMAKE_EXE_LINKER_FLAGS -L${TOOLCHAIN_AUXILIARY_PATH}")
71: 	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH}" CACHE INTERNAL "" FORCE)
72: endif()
73: 
74: set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
75: set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
76: set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
77: set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

- Line 63-72: The variable `CMAKE_EXE_LINKER_FLAGS` which is a standard CMake variable to hold the linker flags, is extended,
if not already done, with the auxiliary libary directory. This contains a bit of CMake trickery, don't worry about the details

Lastly, we need to set some more standard CMake variable:

- Line 74: `CMAKE_FIND_ROOT_PATH_MODE_PROGRAM` to signal not to look for executables in the path just specfied
- Line 75: `CMAKE_FIND_ROOT_PATH_MODE_LIBRARY` to signal to look for libraries in the path just specfied
- Line 76: `CMAKE_FIND_ROOT_PATH_MODE_INCLUDE` to signal to look for include in the path just specfied
- Line 77: `CMAKE_FIND_ROOT_PATH_MODE_PACKAGE` to signal to look for packages in the path just specfied

We now need to use this toolchain file.

### Windows {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_WINDOWS}

On the command line, first configure the build:

```bat
cd <project_root_dir>
del /s /f /q cmake-build/*.*
rmdir cmake-build
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/02-setting-up-a-project -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain
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

Here, we call CMake to configure the project (we do this from within the `cmake-build` directory):
- ../tutorial/02-setting-up-a-project: This specifies the location of the main CMake file, relative to the current location.
Note that we are in `cmake-build`, which is the CMake build directory.
This directory is where CMake builds all intermediate object files, libraries, etc.
We need to be inside this directory, as this is the location where CMake generates the CMake cache file `CMakeCache.txt`
- -G "Ninja": We tell CMake to use Ninja as the build generator
- -DCMAKE_BUILD_TYPE:STRING="Debug": We tell CMake to configure for the Debug build configuration.
Other choices are Release, RelWithDebInfo (release with debug info) and RelMinSize (release with size optimization)
- -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain: We specify the toolchain file to be used by CMake, again relative to the current location

Next perform the actual build

```bat
set ROOT=%CD%
pushd tutorial/02-setting-up-a-project
cmake --build %ROOT%/cmake-build --target 02-setting-up-a-project
popd
```

```output
[2/2] Linking CXX executable 02-setting-up-a-project
FAILED: 02-setting-up-a-project
cmd.exe /C "cd . && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1 CMakeFiles/02-setting-up-a-project.dir/main.cpp.obj -o 02-setting-up-a-project   && cd ."
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

The build command uses CMake again, in a different way (we do this from with the main project directory):
- --build %ROOT%/cmake-build: Specifies the build directory to be used (the directory `cmake-build`)
- --target 02-setting-up-a-project: Specifies the target to be built. If we specify no target, everything is built

Again it is important to be in the correct directory, the directory where our main CMake file is located.

As you can see, in both cases, the source file is compiled, but the link stage failed due to some undefined references.
This is due to the fact that the compiler uses the C standard library, but this leaves a number of platform specific functions undefined.
We therefore need to set compiler options the correct way to really build a baremetal `standalone` application.

### Linux {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_LINUX}

On the command line, first configure the build:

```bat
cd <project_root_dir>
rm -rf cmake-build/
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/02-setting-up-a-project -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain
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

Here, we call CMake to configure the project (we do this from within the `cmake-build` directory):
- ../tutorial/02-setting-up-a-project: This specifies the location of the main CMake file, relative to the current location.
Note that we are in `cmake-build`, which is the CMake build directory.
This directory is where CMake builds all intermediate object files, libraries, etc.
We need to be inside this directory, as this is the location where CMake generates the CMake cache file `CMakeCache.txt`
- -G "Ninja": We tell CMake to use Ninja as the build generator
- -DCMAKE_BUILD_TYPE:STRING="Debug": We tell CMake to configure for the Debug build configuration.
Other choices are Release, RelWithDebInfo (release with debug info) and RelMinSize (release with size optimization)
- -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain: We specify the toolchain file to be used by CMake, again relative to the current location

Next perform the actual build

```bat
rootdir=`pwd`
pushd tutorial/02-setting-up-a-project
cmake --build $rootdir/cmake-build --target 02-setting-up-a-project
popd
```

```output
[2/2] Linking CXX executable 02-setting-up-a-project
FAILED: 02-setting-up-a-project
: && /opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-g++ -g -L/opt/toolchains/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1 CMakeFiles/02-setting-up-a-project.dir/main.cpp.obj -o 02-setting-up-a-project   && :
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

The build command uses CMake again, in a different way (we do this from with the main project directory):
- --build $rootdir/cmake-build: Specifies the build directory to be used (the directory `cmake-build`)
- --target 02-setting-up-a-project: Specifies the target to be built. If we specify no target, everything is built

Again it is important to be in the correct directory, the directory where our main CMake file is located.

As you can see, in both cases, the source file is compiled, but the link stage failed due to some undefined references. This is due to the fact that the compiler uses the C standard library, but this leaves a number of platform specific functions undefined.
We therefore need to set compiler options the correct way to really build a baremetal `standalone` application.
 
## Compiler settings {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS}

### Setting up for custom CMake modules and binary tree {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_SETTING_UP_FOR_CUSTOM_CMAKE_MODULES_AND_BINARY_TREE}

We will define two variables `OUTPUT_BASE_DIR` and `CONFIG_DIR`, which are later used to form the path to the target executable file.
Next to that, we will need some CMake custom functions, for which we need to prepare.

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
1: cmake_minimum_required(VERSION 3.18)
2: 
3: message(STATUS "CMake ${CMAKE_VERSION}")
4: 
5: set(SCRIPTS_DIR "" CACHE STRING "CMake scripts path")
6: set(CONFIG_DIR Debug)
7: set(DEPLOYMENT_DIR ${CMAKE_SOURCE_DIR}/../../deploy)
8: set(OUTPUT_BASE_DIR "${CMAKE_SOURCE_DIR}/../../output" CACHE STRING "Output directory")
9: 
10: if ("${SCRIPTS_DIR}" STREQUAL "")
11:     set(SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/cmake" CACHE STRING "CMake scripts path" FORCE)
12: endif()
13: 
14: list(APPEND CMAKE_MODULE_PATH ${SCRIPTS_DIR})
15: 
16: project(02-setting-up-a-project
17:     DESCRIPTION "Application to demonstrate building using CMake"
18:     LANGUAGES CXX ASM)
19: 
```

Explanation:
- Line 5: We define a variable `SCRIPTS_DIR` to hold the path to CMake scripts we will be adding (to contain the custom functions)
- Line 6: We set the variable `CONFIG_DIR` to denote a build configuration specific directory. For now we'll simply set it to Debug, as we are building for the Debug build configuration
- Line 7: We define a variable `DEPLOYMENT_DIR` to point to the location where our final image will be.
This could be the same path as the output directory, however it makes sense to separate intermediate binaries from the final images
- Line 8: We set variable `OUTPUT_BASE_DIR` to denote the root for the intermediate binaries.
This directory will be used in combination with `CONFIG_DIR` and the target executable name to form the full path for the target executable file.
By default, CMake will generate all object files, libraries and executables inside the build directory.
This makes it impractical to find our end results, so we will specify a different directory tree for libraries and executables
- Line 10-12: We set, if not done yet, the `SCRIPTS_DIR` variable to the location of our custom CMake scripts.
It is custom practice to create a subfolder `cmake` at the project top level and place the scripts there
- Line 14: Lastly, we add the `SCRIPTS_DIR` to the standard CMake search path for CMake modules, `CMAKE_MODULE_PATH`

### Adding project variables {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_ADDING_PROJECT_VARIABLES}

We need to add a few lines to our CMakeLists file, to add definitions, compiler settings, linker options, and link libraries

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
16: project(02-setting-up-a-project
17:     DESCRIPTION "Application to demonstrate building using CMake"
18:     LANGUAGES CXX ASM)
19: 
20: set(PROJECT_TARGET_NAME ${PROJECT_NAME}.elf)
21: 
22: set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE )
23: set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
24: 
25: set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE 
26:     -mcpu=cortex-a53 -mlittle-endian -mcmodel=small 
27:     -Wall -Wextra -Werror 
28:     -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -Wno-unused-variable -Wno-unused-parameter
29:     -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++
30:     -fno-exceptions -fno-rtti -O0 -std=gnu++17
31:     )
32: set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
33: 
34: set(PROJECT_INCLUDE_DIRS_PRIVATE )
35: set(PROJECT_INCLUDE_DIRS_PUBLIC )
36: 
37: set(PROJECT_LINK_OPTIONS ${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nostartfiles -Wl,--section-start=.init=0x80000 -T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld)
38: 
39: set(PROJECT_DEPENDENCIES )
40: 
41: set(PROJECT_LIBS
42:     ${PROJECT_DEPENDENCIES}
43:     )
44: set(PROJECT_SOURCES
45:     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
46:     )
47: 
48: set(PROJECT_INCLUDES_PUBLIC )
49: set(PROJECT_INCLUDES_PRIVATE )
50: 
51: if (PLATFORM_BAREMETAL)
52:     set(START_GROUP -Wl,--start-group)
53:     set(END_GROUP -Wl,--end-group)
54: endif()
55: 
56: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
```

So, after the project is defined, we add the following lines:
- Line 20: We define the variable `PROJECT_TARGET_NAME`, which sets the file name used for our executable to `02-setting-up-a-project.elf`
- Line 22-23: We define the variables `PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE` and `PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC` which will contain compiler definitions.
For now these are empty. There are two, as we can have definitions only for this executable (private) and possibly exported to other targets (public).
As an executable file does not export anything, this is a bit superfluous, but keeping this structure will prove helpful later on.
- Line 25-31: We define the variables `PROJECT_COMPILE_OPTIONS_CXX_PRIVATE` and `PROJECT_COMPILE_OPTIONS_CXX_PRIVATE` in the same way to set compiler options.
Here we set the private compiler options to be:
  - -mcpu=cortex-a53 -mlittle-endian -mcmodel=small: Set CPU architecture options for Raspberry Pi 3 (CPU is AMD Cortex-A53, we use small endian architecture)
  - -Wall: Set warning level to the highest possible level
  - -Wextra: Set warning level to even higher level
  - -Werror: Treat warnings as errors
  - -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -Wno-unused-variable -Wno-unused-parameter: Switch off some specific warnings for Debug configuration
  - -ffreestanding: A very important option, meaning we are building a standalone (baremetal) application
  - -fsigned-char: Use signed characters
  - -nostartfiles: Do not use the standard startup files (`crtbegin.o` and `crtend.o`)
  - -mno-outline-atomics: Needed to make sure we can build with ARMv8.0 and atomics
  - -nostdinc: Do not use the standard C library includes
  - -nostdlib: Do not use the standard C libraries
  - -nostdinc++: Do not use the standard C++ library includes
  - -fno-exceptions: Do not enable exceptions
  - -fno-rtti: Do not use Run Time Type Information
  - -O0: Do not optimize
  - -std=gnu++17: Support C++17 language
- Line 32: We set the public compiler options to be empty, meaning we don't export anything
- Line 34-35: We defines the variables `PROJECT_INCLUDE_DIRS_PRIVATE` and `PROJECT_INCLUDE_DIRS_PUBLIC` again in the same way to specific include directories.
For now, everything is in the same directory, so we leave this empty
- Line 37: We define the variable `PROJECT_LINK_OPTIONS` to specify linker options
  - ${CMAKE_EXE_LINKER_FLAGS}: Use the existing linker options (the linker options specified in the [toolchain file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE))
  - -nostdlib: Do not use the standard C libraries
  - -nostartfiles: Do not use the standard startup files (`crtbegin.o` and `crtend.o`)
  - -Wl,--section-start=.init=0x80000: Define the start address of the executable to be 0x80000
  - -T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld: Use the specified linker definition file (see [Adding linker definition file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE))
- Line 39: We define a variable `PROJECT_DEPENDENCIES` to hold any libraries we will be depending on. For now this is empty
- Line 41-43: We define the variable `PROJECT_LIBS` to hold all libraries we will be linking to. This means all dependencies, and all specified standard libraries
- Line 44-46: We define the variable `PROJECT_SOURCE` to hold the source files to be used for building
- Line 48-49: We define the variables `PROJECT_INCLUDES_PUBLIC` and `PROJECT_INCLUDES_PRIVATE` to hold the public and private header files to be used for building
- Line 51-54: We define two extra variables, only understood by gcc, to group libraries together for correct resolution. These are for the start of the grouping `START_GROUP` and the end of the grouping `END_GROUP`

### Setting up the target {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_SETTING_UP_THE_TARGET}

We then need to link these variables to the target we're building:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
56: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
57: 
58: target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
59: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
60: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
61: target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE})
62: target_compile_definitions(${PROJECT_NAME} PUBLIC  ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC})
63: target_compile_options(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE})
64: target_compile_options(${PROJECT_NAME} PUBLIC  ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC})
65: 
```

Explanation:
- Line 58: We link to the specified libraries (empty list for now) in a group. Hence `${START_GROUP} ${PROJECT_LIBS} ${END_GROUP}`
- Line 59-60: We specify include directories for both private and public use
- Line 61-62: We specify compiler definitions for both private and public use
- Line 63-64: We specify compiler options for both private and public use

Next we specify the linker options.
As the options are specified as a list separated by semicolons, and we need to create a string of values separated by spaces, we use a custom function:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
66: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
67: message(STATUS "PROJECT_LINK_OPTIONS=${PROJECT_LINK_OPTIONS_STRING}")
68: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
69:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
70: endif()
71: 
```

Explanation:
- Line 66: Converts the list to a string with spaces are delimiter using the custom functions `list_to_string`. We will define this custom function later on
- Line 67: prints the string
- Line 68-70: sets the linker flags, only if the string is not empty

Lastly, we set the executable file name, and the location of executable files and libraries:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
71: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
72: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/lib)
73: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin)
74: 
```

Explanation:
- Line 71: sets the executable name `OUTPUT_NAME`, a standard property
- Line 72: defines the location for static libraries `ARCHIVE_OUTPUT_DIRECTORY`, a standard property
- Line 73: defines the location for executables `RUNTIME_OUTPUT_DIRECTORY`, a standard property

The last two lines use the variables `OUTPUT_BASE_DIR` and `CONFIG_DIR` defined before in [Setting up for custom CMake modules and binary tree](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_SETTING_UP_FOR_CUSTOM_CMAKE_MODULES_AND_BINARY_TREE).
It is common practice to collect output files together in a binaries tree.
`OUTPUT_BASE_DIR` is the root of this tree. `CONFIG_DIR` is the configuration we're building for. CMake supports 4 configurations by default:
- Debug: Debug build (with debug symbols)
- Release: Release build (no symbols)
- RelWithDebInfo: Release build with debug symbols
- MinSizeRel: Release with minimum size optimization

When building, we can set the configuration using the standard CMake variable `CMAKE_BUILD_TYPE`, as we did earlier setting it to Debug.

### Adding custom CMake module {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_ADDING_CUSTOM_CMAKE_MODULE}

Now, we still need to add the custom CMake script for the function `list_to_string`. 
So we create a folder `cmake` and underneath create a file `functions.cmake`.
It is standard practice to name CMake scripts with extension `.cmake`.

The contents of the file are:

```cmake
File: tutorial/02-setting-up-a-project/cmake/functions.cmake
1: function(list_to_string in out)
2:     set(tmp "")
3:     foreach(VAL ${${in}})
4:         string(APPEND tmp "${VAL} ")
5:     endforeach()
6:     set(${out} "${tmp}" PARENT_SCOPE)
7: endfunction()
```

Without going into too much detail. This function is called `list_to_string`, and has one input (`in`) and one output (`out`) parameter.
It cycles through the parts of the input variable, and creates a string by adding each part followed by a space.
Finally it sets the output variable. The `PARENT_SCOPE` is needed to convert the value to outside the function (for the output variable).

Finally we need to be able to use the function. For this, we need to include the module:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
14: list(APPEND CMAKE_MODULE_PATH ${SCRIPTS_DIR})
15: 
16: include(functions)
17: 
18: project(02-setting-up-a-project
19:     DESCRIPTION "Application to demonstrate building using CMake"
20:     LANGUAGES CXX ASM)
21: 
```

As you can see, we just name the script (line 16), without path or extension.
This is possible, because we already added the path to the standard CMake module paths `CMAKE_MODULE_PATH`, and because we're using the standard CMake module extension `.cmake`

## Adding the linker definition file {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_ADDING_THE_LINKER_DEFINITION_FILE}

The we need to set up the linker definitions file as pointed to by the [linker options](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_SETTING_UP_THE_TARGET).

This file is named `link.ld` and contains the following:

```text
File: tutorial/02-setting-up-a-project/link.ld
1: /*------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : link.ld
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Linker definition file
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
17: //
18: // Permission is hereby granted, free of charge, to any person
19: // obtaining a copy of this software and associated documentation
20: // files(the "Software"), to deal in the Software without
21: // restriction, including without limitation the rights to use, copy,
22: // modify, merge, publish, distribute, sublicense, and /or sell copies
23: // of the Software, and to permit persons to whom the Software is
24: // furnished to do so, subject to the following conditions :
25: //
26: // The above copyright notice and this permission notice shall be
27: // included in all copies or substantial portions of the Software.
28: //
29: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
30: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
31: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
32: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
33: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
34: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
35: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
36: // DEALINGS IN THE SOFTWARE.
37: //
38: //------------------------------------------------------------------------------*/
39: 
40: /* Executable entry point (defined in start.S) */
41: ENTRY(_start)
42: 
43: /* Executable headers */
44: PHDRS
45: {
46:     init PT_LOAD FILEHDR PHDRS FLAGS(RE);
47:     fini PT_LOAD FILEHDR PHDRS FLAGS(RE);
48:     text PT_LOAD FILEHDR PHDRS FLAGS(RE);
49:     rodata PT_LOAD FLAGS(RE);
50:     data PT_LOAD FLAGS(RWE);
51: }
52: 
53: SECTIONS
54: {
55:     /* Code section */
56:     .text : {
57:         KEEP(*(.text.boot))
58:         *(.text* .text.* .gnu.linkonce.t*)
59: 
60:         _etext = .;
61:     } : text
62: 
63:     . = SIZEOF_HEADERS;
64:     /* Executable initialization section */
65:     .init : {
66:         *(.init)
67:     } : init
68: 
69:     /* Executable cleanup section */
70:     .fini : {
71:         *(.fini)
72:     } : fini
73: 
74:     /* Executable read only data section */
75:     .rodata : {
76:         *(.rodata*)
77:     } : rodata
78: 
79:     /* Executable static initialization section */
80:     .init_array : {
81:         __init_start = .;
82: 
83:         KEEP(*(.init_array*))
84: 
85:         __init_end = .;
86:     }
87: 
88:     /* Executable read/write data section */
89:     .data : {
90:         *(.data*)
91:     } : data
92: 
93:     /* Executable uninitialized data section */
94:     .bss : {
95:         __bss_start = .;
96: 
97:         *(.bss*)
98:         *(COMMON)
99: 
100:         __bss_end = .;
101:     } : data
102: }
103: /* bss size is actual size rounded down to blocks of 8 bytes */
104: __bss_size = (__bss_end - __bss_start) >> 3;
```

The linker definition file defines the different sections in the executable file.

- Line 41: The `ENTRY(_start)` statement sets the starting point of the executable to the location denoted by label `_start`. We will cover this in a minute.
- Line 44-51: The `PHDRS` part defines the Program Header Table (refer to [ELF header format](../cpu/arm/elf-format.pdf) for more information)
- Line 56-61: .text is the code section
  - This section starts with the .text.boot subsection, which is always stored in the executable (`KEEP`).
  - The other subsections (`.text* .text.* .gnu.linkonce.t*`) are only kept as needed.
- Line 65-67: .init is the initialization section, which contains code with is normally in the crtbegin.o file. We will run into this later on, for now it is empty
- Line 70-72: .fini is the cleanup section, similarly containing code in crtend.o.
- Line 75-77: .rodata is the constants section, i.e. it contains data that is read-only
- Line 80-86: .init_array is the static initializer section.
It contains a table of functions used to initialize static data, such as constructors of static class objects. This is always stored. We will get to this in [Improving startup and static initialization](06-improving-startup-static-initialization.md)
- Line 89-91: .data contains read/write data for the executable
- Line 94-101: .bss contains unitialized data, such as simple global (extern) or local (static) variables. They are normally zeroed out before the program starts.
- Line 104: data in the .bss section is initialized in chunks of 8 bytes (rounded down to the nearest multiple of 8)

## Startup assembly code {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_STARTUP_ASSEMBLY_CODE}

The final step is adding startup code.

In order for the CPU to be correctly initialized, and the cores handled correctly, we need to add some assembly code. This code will roughly do the following:
- It will check which core our code is running.
If it is core 0, we will continue, otherwise we will start a waiting loop, simply waiting for events (one of the events that may happen is shutdown, we need to wait for that).
Effectively, we simply halt all cores except core 0
- Next, we set the stack point just below our code (the stack grows down), so that we have a stack to work with
- Then we initialize the data in the .bss section (see above in [Adding the linker definition file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE))
- Lastly, we call the main() function defined in [Create source file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATE_SOURCE_FILE).

The startup code will be stored in the `start.S` assembly file:

```assembly
File: tutorial/02-setting-up-a-project/start.S
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : start.S
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Startup code. This is the entry point to any executable. It puts all cores except core 0 in sleep mode.
11: //               For core 0, it sets the stack pointer to just below the code (as the stack grows down), and then calls main().
12: //
13: //------------------------------------------------------------------------------
14: //
15: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
16: //
17: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
18: //
19: // Permission is hereby granted, free of charge, to any person
20: // obtaining a copy of this software and associated documentation
21: // files(the "Software"), to deal in the Software without
22: // restriction, including without limitation the rights to use, copy,
23: // modify, merge, publish, distribute, sublicense, and /or sell copies
24: // of the Software, and to permit persons to whom the Software is
25: // furnished to do so, subject to the following conditions :
26: //
27: // The above copyright notice and this permission notice shall be
28: // included in all copies or substantial portions of the Software.
29: //
30: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
31: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
32: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
33: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
34: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
35: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
36: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
37: // DEALINGS IN THE SOFTWARE.
38: //
39: //------------------------------------------------------------------------------
40: 
41: .section ".text.boot"
42: 
43: .global _start
44: 
45: _start:
46:     // Read MPIDR_EL1 register, low 7 bits contain core id (as we have 4 cores, we mask only lowest two bits)
47:     mrs     x1, mpidr_el1
48:     // Mask away everything but the core id
49:     and     x1, x1, #3
50:     // If core id is 0, continue
51:     cbz     x1, core0
52:     // If core id > 0, start wait loop
53: waitevent:
54:     wfe
55:     b       waitevent
56: 
57: core0:
58:     // core 0
59: 
60:     // set top of stack just before our code (stack grows to a lower address per AAPCS64)
61:     ldr     x1, =_start
62:     mov     sp, x1
63: 
64:     // clear bss
65:     // Load bss start
66:     ldr     x1, =__bss_start
67:     // Load bss size
68:     ldr     w2, =__bss_size
69:     // If bss is empty
70:     cbz     w2, empty_bss
71: 
72: clear_bss_loop:
73:     // Store 0 in x1 location for 8 bytes, increment x1 by 8
74:     str     xzr, [x1], #8
75:     // Cound down size
76:     sub     w2, w2, #1
77:     // Loop as loop as the end is not reached
78:     cbnz    w2, clear_bss_loop
79: 
80:     // jump to C code, should not return
81: empty_bss:
82:     bl      main
83:     // for failsafe, halt this core too
84:     b       waitevent
```

Without going into too much detail, the code performs the following steps:

- Line 41: the startup code is part of the `.text.boot` subsection defined in the linker definition file
- Line 45: this is the entry point for the \_start function, essentially the entry point of our code
- Line 47-51: determine which core the code is running on, and jump to `core0` if the core id is 0. We'll explain the MPIDR_EL1 register, among others, a bit later
- Line 53-55: in other cases we loop waiting for an event (effectively halting the core)
- Line 61-62: the stack pointer is set just below the code
- Line 64-70: information on the `.bss` section is retrieved. If the .bss section is empty, we jump to `empty_bss`
- Line 74-78: write 0 to the next 8 bytes of the .bss section, and while not at the end, repeat
- Line 82: call to the main() function in `tutorial/02-setting-up-a-project/main.cpp`
- Line 84: when main() returns, also halt core 0

For more information on ARM assembly code code also [getting-started-with-ARM-assembly-language](../cpu/arm/getting-started-with-ARM-assembly-language.pdf).
For more information on the `MPIDR_EL1` register see also [ARM Cortex-A53 System Registers](../cpu/arm/arm-cortex-a53-system-registers.md) and [Arm® Architecture Registers](../cpu/arm/ARM-architecture-registers.pdf) page 1390.

We need to add the startup code to the project:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
46: set(PROJECT_SOURCES
47:     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/start.S
49:     )
50: 
```

## Creating an image {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATING_AN_IMAGE}

Now we can build the code to generate `output/Debug/bin/02-setting-up-a-project.elf`, however that application cannot simply be run in e.g. QEMU.
We need to create an image for that. This is a fairly simple step, adding a new target for the image.

We will create a subdirectory `create-image` underneath our current project, and add a CMake file `CMakeLists.txt` there:

```cmake
File: tutorial/02-setting-up-a-project/create-image/CMakeLists.txt
1: project(02-setting-up-a-project-image
2:     DESCRIPTION "Kernel image for 02-setting-up-a-project RPI 64 bit bare metal")
3: 
4: message(STATUS "\n**********************************************************************************\n")
5: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
6: 
7: message("\n** Setting up ${PROJECT_NAME} **\n")
8: 
9: set(BAREMETAL_TARGET_KERNEL kernel8)
10: set(DEPENDENCY 02-setting-up-a-project)
11: set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)
12: 
13: create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
```

- Line 1-2: We define a new project `02-setting-up-a-project-image`
- Line 9: We create the variable `BAREMETAL_TARGET_KERNEL` to specify the kernel image to create
This depends on the target platform we're building for. Here we create kernel8.img for Raspberry Pi 3 64 bit (see [System startup](system-startup#config.txt))
- Line 10: We create the variable `DEPENDENCY` to specify the project we are going to create the image for (and which we depend on)
- Line 11: We create the variable `IMAGE_NAME` to specify the complete filename of the image
- Line 13: We call the custom function `create_image` to create a target for the image

The customer function is added to the `functions.cmake` module:

```cmake
File: tutorial/02-setting-up-a-project/cmake/functions.cmake
1: function(list_to_string in out)
2:     set(tmp "")
3:     foreach(VAL ${${in}})
4:         string(APPEND tmp "${VAL} ")
5:     endforeach()
6:     set(${out} "${tmp}" PARENT_SCOPE)
7: endfunction()
8: 
9: function(create_image target image project)
10:     message(STATUS "create_image ${target} ${image} ${project}")
11: 
12:     if(NOT TARGET ${project})
13:       message(STATUS "There is no target named '${project}'")
14:       return()
15:     endif()
16: 
17:     get_target_property(TARGET_NAME ${project} OUTPUT_NAME)
18:     message(STATUS "TARGET_NAME ${TARGET_NAME}")
19: 
20:     message(STATUS "generate ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image} from ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin/${project}")
21:     add_custom_command(
22:         OUTPUT ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
23:         COMMAND ${CMAKE_OBJCOPY} ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin/${TARGET_NAME} -O binary ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
24:         DEPENDS ${project}
25:         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
26:     )
27: 
28:     add_custom_target(${target} ALL DEPENDS 
29:         ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
30:         )
31: endfunction()
```

The function `create_image` takes three parameters:
- The target to be created (`target`), in this case `02-setting-up-a-project-image`
- The image filename (`image`), in this case `kernel8.img`
- The target (`project`) that creates the application to be added to the image, in this case `02-setting-up-a-project.elf`

Explanation:
- Line 10: The function shows how it was called
- Line 12-15: The function checks whether the application target exists (is a target), and prints an error if not
- Line 17: The `OUTPUT_NAME` property of the application target (the project we depend on) is retrieved, so in this case `02-setting-up-a-project.elf`,
and stored in variable `TARGET_NAME`
- Line 18: The property value is printed
- Line 20: The action to be taken is printed
- Line 21-26: A custom CMake command is created
  - Its output is `${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}`.
This uses the variables `DEPLOYMENT_DIR` and `CONFIG_DIR` defined before, and then adds the name of the application project as a directory, and then the image name
So the final path will be `deploy/Debug/02-setting-up-a-project/kernel8.img`
  - The command to be performed uses the `CMAKE_OBJCOPY` tool specified in the toolchain file
  - The command depends on our application project
The actual command run will be `aarch64-none-elf-objcopy output/Debg/bin/02-setting-up-a-project.elf -O binary deploy/Debug/02-setting-up-a-project/kernel8.img`
- Line 28-30: A custom CMake target `02-setting-up-a-project-image` is created, that depends on the output of the command just created.

This may all seem complex, but this functionality can be used again later on by simply changing the parameters.

Now we still need to use the new target. We simply do this by referring to the subdirectory containing the new CMake file.

We add the following line to this file to include its `create-image` subdirectory. This instructs CMake to also use the `CMakeLists.txt` file in that project directory:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
69: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
70: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/lib)
71: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin)
72: 
73: add_subdirectory(create-image)
```

We can now start to build the application and image.

## Configuring {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CONFIGURING}

We first need to configure the build for CMake:

### Windows {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CONFIGURING_WINDOWS}

```bat
del /S /f /q cmake-build\*.*
rmdir cmake-build
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/02-setting-up-a-project -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain
popd
```

### Linux {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CONFIGURING_LINUX}

```bash
rm -rf cmake-build/
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/02-setting-up-a-project -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain
popd
```

In other words, we clean up the CMake build directory, and recreate it, then we step into this directory, and configure CMake to use `tutorial/02-setting-up-a-project` as the root CMake directory, and use the toolchain file.
We are building for Debug.

## Building {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILDING}

Then we can build the targets:

### Windows {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILDING_WINDOWS}

```bat
set ROOT=%CD%
pushd tutorial\02-setting-up-a-project
cmake --build %ROOT%/cmake-build --target 02-setting-up-a-project-image
popd
```

### Linux {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILDING_LINUX}

```bash
rootdir=`pwd`
pushd tutorial/02-setting-up-a-project
cmake --build $rootdir/cmake-build --target 02-setting-up-a-project-image
popd
```

We save the root directory to be able to reference it, and step into the project directory. There we run cmake with the `--build` parameter to specify the build directory.
The target we're going to build is the image, so `02-setting-up-a-project-image`.
This will automatically build all its dependencies, so `02-setting-up-a-project.elf` will also be built

After this step, we will have built the application in `output/Debug/bin/02-setting-up-a-project.elf`, and the image in `deploy/Debug/02-setting-up-a-project-image/kernel8.img`.

The image is very small, as the application basically does nothing, but you have built your __first baremetal application!__

## Debugging {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_DEBUGGING}

To show that the application actually works, let's run it in QEMU and debug it.

### Windows {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_DEBUGGING_WINDOWS}

To run QEMU:

```bat
cd <project_root_dir>
"c:\Program Files\qemu\qemu-system-aarch64.exe" -M raspi3b -kernel deploy\Debug\02-setting-up-a-project-image\kernel8.img -serial stdio -s -S
```

To run GDB:
```bat
cd output\Debug\bin
D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gdb.exe --args 02-setting-up-a-project.elf
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
Type "apropos word" to search for commands related to "word"...
Reading symbols from 02-setting-up-a-project.elf...
```

### Linux {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_DEBUGGING_LINUX}

To run QEMU:

```bash
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio -s -S
```

To run GDB:
```bash
cd output/Debug/bin
gdb-multiarch --args 02-setting-up-a-project.elf
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
Type "apropos word" to search for commands related to "word"...
Reading symbols from 02-setting-up-a-project.elf...
```

In GDB:
```gdb
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
0x0000000000000000 in ?? ()
(gdb) load
Loading section .eh_frame, size 0x28 lma 0x158
Loading section .text, size 0x58 lma 0x80000
Start address 0x0000000000080000, load size 128
Transfer rate: 1024 bits in <1 sec, 64 bytes/write.
(gdb) b main.cpp:3
Breakpoint 1 at 0x80050: file /home/rene/repo/baremetal.github/main.cpp, line 3.
(gdb) c
Continuing.

Thread 1 hit Breakpoint 1, main () at /home/rene/repo/baremetal.github/main.cpp:3
3	    return 0;
```

So we ended up in line 3 of the main() function:

```cpp
File: main.cpp
1: int main()
2: {
3:     return 0;
4: }
```

Next, we step one further, ending up in start.S, and then close down debugging again:

```gdb
(gdb) n
empty_bss () at /home/rene/repo/baremetal.github/start.S:84
84	    b       waitevent
(gdb) kill
Kill the program being debugged? (y or n) y
[Inferior 1 (process 1) killed]
(gdb) quit
```

Next: [03-setting-up-project-structure](03-setting-up-project-structure.md)
