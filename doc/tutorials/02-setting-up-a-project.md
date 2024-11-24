# Tutorial 02: Setting up a project for building and debugging {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING}

@tableofcontents

Configuration for a project is largely similar for Windows and Linux, however the way we build and debug is slightly different.

For now, let's set up a simple project that simply returns (i.e. does nothing but return 0 in `main()`), and then halts the system.
We'll be adding some scripts to make configuring, building and debugging somewhat easier.

This will be quite an extensive chapter, so bare with me.
This chapter will include lots of explanation on how to work with CMake, how code is started on the platform, etc.

The way we configure the project is very specific, verbose and direct, which we will improve later on in [Setting up project structure](03-setting-up-project-structure.md).

We will assume you checked out the repo to `D:\Projects\baremetal` for Windows and `~/projects/baremetal` for Linux, or have created this directory to start from fresh.
First, we create a folder for the project. Let's say `D:\Projects\baremetal\tutorial\02-setting-up-a-project` on Windows and `~/projects/baremetal/tutorial/02-setting-up-a-project` on Linux.

Depending on whether you checked out the code from the GitHub project, you can either replicate the steps, or read along.

In this directory, we first need to create a CMake file, which is named `CMakeLists.txt`. Be careful about the 's' in _Lists_, 
and also make sure you have the correct casing, especially in Linux.
Whenever we mention a CMake file, we mean a file named `CMakeLists.txt`. This always has the same name by convention of CMake.

As soon as you add this file in Visual Studio, it may detect this is a CMake project and try to configure it.
This will fail as we don't have the correct contents yet. Don't worry about this.
We'll get to Visual Studio in [Setting up project structure](03-setting-up-project-structure.md).

## Create project {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATE_PROJECT}

In this file we will first create a project. This is done in CMake by adding a file named CMakeLists.txt:

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
- Line 1: We require a minimum version of 3.18 for CMake. There should always be a similar line in the beginning of the main CMake file (the CMake file in the root directory of the project)
- Line 3: We print the current version of CMake
- Line 5-7: We define a project named `02-setting-up-a-project`, give it a short description, and specify that it will use C++ and assembly code as programming languages

## Create source file {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATE_SOURCE_FILE}

We'll now add a source file to the project, so let's create a source file first, and simply call it `main.cpp`. The contents will be:

```cpp
File: tutorial/02-setting-up-a-project/main.cpp
1: int main()
2: {
3:     return 0;
4: }
```

For now the application does nothing but return 0.
Notice that we have created a main function that returns an `int` and takes no parameters.
As we are running a baremetal application, there is no way to specify parameters, except through the kernel parameters file, however these will not be converted to parameters to `main()`.

We will also need some assembly code to correctly initialize the CPU.
This will be discussed later, in the [linker definition file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE) and the [startup assembly code](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_STARTUP_ASSEMBLY_CODE) sections.

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
13: set(PROJECT_INCLUDES_PRIVATE)
14: 
15: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
```

Short explanation:
- Line 9-11: We define a variable named `PROJECT_SOURCES` that contains the paths to our source files, for now only containing one source file  (`CMAKE_CURRENT_SOURCE_DIR` is the current source directory, so `main.cpp` will be in the same directory as `CMakeLists.txt`)
- Line 12-13: We define two more variables to contain header files, which are for now empty, `PROJECT_INCLUDES_PUBLIC` and `PROJECT_INCLUDES_PRIVATE`.
- Line 15: We create a so-called target in CMake for an executable, with name `PROJECT_NAME` (this is a standard CMake variable denoting the name of the project we're in, so in this case `02-setting-up-a-project`)
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
5:         set(TOOLCHAIN_ROOT "/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf")
6:     else()
7:         set(TOOLCHAIN_ROOT "D:/toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf")
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
23: set(TOOLCHAIN_AUXILIARY_PATH ${TOOLCHAIN_ROOT}/lib/gcc/${TOOL_DESTINATION_PLATFORM}/13.3.1)
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
64:    set(HAVE_AUX_PATH false)
65: else()
66:    list(FIND ${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH} HAVE_AUX_PATH)
67: endif()
68: message(STATUS "CMAKE_EXE_LINKER_FLAGS=  ${CMAKE_EXE_LINKER_FLAGS}")
69: if (NOT HAVE_AUX_PATH)
70:    message(STATUS "Adding to CMAKE_EXE_LINKER_FLAGS -L${TOOLCHAIN_AUXILIARY_PATH}")
71:    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH}" CACHE INTERNAL "" FORCE)
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
5:         set(TOOLCHAIN_ROOT "/home/rene/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf")
6:     else()
7:         set(TOOLCHAIN_ROOT "D:/toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf")
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

Note that all paths, even on Windows, use forward slashes. This is a convention in CMake.
This makes working with CMake more convenient, so stick to this.
There are some exceptions, such as literal strings.

- Line 1: Here, we include a CMake script to enable forcing the compiler. We need to be able to do this to override the default compiler
- Line 3-11: We check if there is an existing environment variable `BAREMETAL_TOOLCHAIN_ROOT` set to define the location of the toolchain,
and otherwise fall back to a default, different for Windows and Linux of course. The environment variable is a way to override what is in the toolchain file.
Note that the paths here are absolute paths, so if you have you toolchain in a different directory, either use the environment variable `BAREMETAL_TOOLCHAIN_ROOT` or update the path in the toolchain file
- Line 13: We set the variable `PLATFORM_BAREMETAL` for convenience later on, so we know we are using this toolchain file
- Line 14: We set the variable `CMAKE_SYSTEM_NAME` which is a standard variable to denote the system we're going to build.
For baremetal projects this must be set to `Generic`
- Line 15: We set the variable `CMAKE_SYSTEM_PROCESSOR` which is a standard variable to define the processor architecture we're going to build for.
In our case this will be a 64 bit ARM processor, for which the architecture name is `aarch64`
- Line 16: `TOOL_DESTINATION_PLATFORM` is the so called target triplet / quadruplet.
It defines the combination of target architecture, vendor if applicable, the operating system, and the build type.
In our case this is `aarch64-none-elf` meaning a 64 bit ARM architecture, with no OS, and with elf output files
- Line 18: We also print the used toolchain root
- Line 20: We set CMake build output to be more verbose

#### Part 2 {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE_PART_2}

The next part defines the tools to be used, such as the compiler, linker, etc.:

```cmake
File: tutorial/02-setting-up-a-project/baremetal.toolchain
22: set(TOOLCHAIN_PATH ${TOOLCHAIN_ROOT}/bin)
23: set(TOOLCHAIN_AUXILIARY_PATH ${TOOLCHAIN_ROOT}/lib/gcc/${TOOL_DESTINATION_PLATFORM}/13.3.1)
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
- Line 23: We set the variable `TOOLCHAIN_AUXILIARY_PATH` which is the location of auxiliary libraries used for building. Notice that the compiler version is part of this path, so if you use a different version, make sure to use the correct directory

Then depending on the build platform, we define the tools to be used. 
The part at the end `CACHE FILEPATH "text" FORCE` simply means that the variable is a file path, with a description, which is enforced into the CMake cache.

The CMake cache is a file CMake uses to store project variables for later reference.
This file is named `CMakeCache.txt`, and is located in the CMake build directory (explained later).
- Line 26/38: We set the variable `CMAKE_C_COMPILER` to the path to the C compiler (this is a gcc compiler)
- Line 27/39: We set the variable `CMAKE_C_COMPILER_FORCED` to the path to the C compiler, but forced to be this compiler
- Line 29/41: We set the variable `CMAKE_CXX_COMPILER` to the path to the C++ compiler (this could be gcc as well, but it's common to use g++)
- Line 30/42: We set the variable `CMAKE_CXX_COMPILER_FORCED` to the path to the C++ compiler, but forced to be this compiler
- Line 32/44: We set the variable `CMAKE_AR` to the path to the archiver, in other words the static library linker
- Line 34/46: We set the variable `CMAKE_LINKER` to the path to the linker, which links executables and dynamic libraries
- Line 36/48: We set the variable `CMAKE_OBJCOPY` to the path to the object copier, which we will need to create an image, as explained in [Deployment mechanism](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM)

Notice that we did not set the assembler here, even though it will be used. In our case, gcc is also able to compile assembly code.

- Line 51: We also define the variable `STDDEF_INCPATH` for the standard include path
- Line 53-61: We print all the variables just defined

#### Part 3 {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE_PART_3}

```cmake
File: tutorial/02-setting-up-a-project/baremetal.toolchain
63: if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "")
64:    set(HAVE_AUX_PATH false)
65: else()
66:    list(FIND ${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH} HAVE_AUX_PATH)
67: endif()
68: message(STATUS "CMAKE_EXE_LINKER_FLAGS=  ${CMAKE_EXE_LINKER_FLAGS}")
69: if (NOT HAVE_AUX_PATH)
70:    message(STATUS "Adding to CMAKE_EXE_LINKER_FLAGS -L${TOOLCHAIN_AUXILIARY_PATH}")
71:    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH}" CACHE INTERNAL "" FORCE)
72: endif()
73: 
74: set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
75: set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
76: set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
77: set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

- Line 63-72: The variable `CMAKE_EXE_LINKER_FLAGS` which is a standard CMake variable to hold the linker flags, is extended,
if not already done, with the auxiliary library directory. This contains a bit of CMake trickery, don't worry about the details

Lastly, we need to set some more standard CMake variable:

- Line 74: `CMAKE_FIND_ROOT_PATH_MODE_PROGRAM` specifies not to look for executables in the project path
- Line 75: `CMAKE_FIND_ROOT_PATH_MODE_LIBRARY` specifies to look for libraries only in the project path
- Line 76: `CMAKE_FIND_ROOT_PATH_MODE_INCLUDE` specifies to look for include only in the project path
- Line 77: `CMAKE_FIND_ROOT_PATH_MODE_PACKAGE` specifies to look for packages only in the project path

We now need to use this toolchain file.

### Windows {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_WINDOWS}

On the command line, first configure the build:

```bat
cd <project_root_dir>
del /s /f /q cmake-build\*.*
rmdir /S /Q cmake-build
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/02-setting-up-a-project -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain
popd
```

- `cd D:\Projects\baremetal`: This speaks for itself.
- `del /s /f /q cmake-build\*.*`: This removes all files in the directory `cmake-build`, which we will use to build inside.
This call the CMake build directory, which is defined inside CMake by the variable `CMAKE_BINARY_DIR`.
This directory is where CMake builds all intermediate object files, libraries, etc.
We need to be inside this directory, as this is the location where CMake generates the CMake cache file `CMakeCache.txt`
- `rmdir /S /Q cmake-build`: This removes the build directory itself
- `mkdir cmake-build`: This re-creates the build directory
- `pushd cmake-build`: This steps inside the build directory
- `cmake ../tutorial/02-setting-up-a-project -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain`: This performs the actual project configuration. We call CMake with the following parameters:
  - `../tutorial/02-setting-up-a-project`: This is the directory containing the main CMake file, which in this case is `D:\Projects\baremetal\tutorial\02-setting-up-a-project`, so relative from the build directory this is `../tutorial/02-setting-up-a-project`.
  CMake starts configuring by using the CMake file `CMakeLists.txt` in this directory
  - `-G "Ninja"`: Thisselects the generator to use. As said in [Ninja](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_BUILD_ENVIRONMENT_NINJA) we use Ninja for this
  - `-DCMAKE_BUILD_TYPE:STRING="Debug"`: This defines the variable `CMAKE_BUILD_TYPE` to be a string with value `"Debug"`.
  This is a standard CMake variable, defining the type of build we'll perform. We tell CMake to configure for the Debug build configuration.
Other choices are `Release`, `RelWithDebInfo` (release with debug info) and `RelMinSize` (release with size optimization)
  - `-DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain`: This defines the variable `CMAKE_TOOLCHAIN_FILE` to be a path, with value `../tutorial/02-setting-up-a-project/baremetal.toolchain`, i.e. the path to our toolchain file.
  This again is a standard CMake variable, that forces CMake to use this file as the toolchain definition file.
- `popd`: This steps out of the build directory again

Output:

```text
-- CMake 3.30.1
-- TOOLCHAIN_ROOT           D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
-- C++ compiler             D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-g++.exe
-- Archiver                 D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ar.exe
-- Linker                   D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ld.exe
-- ObjCopy                  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-objcopy.exe
-- Std include path         D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1/include
-- CMAKE_EXE_LINKER_FLAGS=
-- Adding to CMAKE_EXE_LINKER_FLAGS -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
-- TOOLCHAIN_ROOT           D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
-- C++ compiler             D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-g++.exe
-- Archiver                 D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ar.exe
-- Linker                   D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ld.exe
-- ObjCopy                  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-objcopy.exe
-- Std include path         D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1/include
-- CMAKE_EXE_LINKER_FLAGS=   -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
-- The CXX compiler identification is GNU 13.3.1
-- The ASM compiler identification is GNU
-- Found assembler: D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
-- Configuring done (0.9s)
-- Generating done (0.1s)
-- Build files have been written to: D:/Projects/baremetal/cmake-build
```

There may be warnings about unused variables, you can ignore these.

Next perform the actual build

```bat
set ROOT=%CD%
pushd tutorial\02-setting-up-a-project
cmake --build %ROOT%/cmake-build --target 02-setting-up-a-project
popd
```

- `set ROOT=%CD%`: We need to keep the current directory as we will move into a different one
- `pushd tutorial\02-setting-up-a-project`: We move to the directory containing the main CMake file
- `cmake --build %ROOT%/cmake-build --target 02-setting-up-a-project`: This performs the actual build:
  - `--build %ROOT%/cmake-build`: This specifies the build directory to use
  - `--target 02-setting-up-a-project`: This is the target we wish to build. A target is the name of a a project in CMake. As we defined the project `02-setting-up-a-project` in [Create project](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATE_PROJECT), we will build this one.
  We could also not specify the target, is which case all targets will be built
- `popd`: We move back to the original directory

```output
[2/2] Linking CXX executable 02-setting-up-a-project
FAILED: 02-setting-up-a-project
C:\Windows\system32\cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1 CMakeFiles/02-setting-up-a-project.dir/main.cpp.obj -o 02-setting-up-a-project   && cd ."
D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1\libg.a(libc_a-exit.o): in function `exit':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/stdlib/exit.c:65:(.text.exit+0x2c): undefined reference to `_exit'
D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1\libg.a(libc_a-closer.o): in function `_close_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/closer.c:47:(.text._close_r+0x1c): undefined reference to `_close'
D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1\libg.a(libc_a-lseekr.o): in function `_lseek_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/lseekr.c:49:(.text._lseek_r+0x24): undefined reference to `_lseek'
D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1\libg.a(libc_a-readr.o): in function `_read_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/readr.c:49:(.text._read_r+0x24): undefined reference to `_read'
D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1\libg.a(libc_a-writer.o): in function `_write_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/writer.c:49:(.text._write_r+0x24): undefined reference to `_write'
D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1\libg.a(libc_a-sbrkr.o): in function `_sbrk_r':
/data/jenkins/workspace/GNU-toolchain/arm-13-2/src/newlib-cygwin/newlib/libc/reent/sbrkr.c:51:(.text._sbrk_r+0x1c): undefined reference to `_sbrk'
collect2.exe: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.
```

Again it is important to be in the correct directory, the directory where our main CMake file is located.

As you can see, in both cases, the source file is compiled, but the link stage failed due to some undefined references.
This is due to the fact that the compiler uses the C standard library, but this leaves a number of platform specific functions undefined.
We therefore need to set compiler options the correct way to really build a baremetal `standalone` application.

### Linux {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_LINUX}

On the command line, first configure the build:

```bash
cd <project_root_dir>
rm -rf cmake-build/
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/02-setting-up-a-project -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain
popd
```

- `cd ~/projects/baremetal`: This speaks for itself.
- `rm -rf cmake-build/`: This removes all files in the directory `cmake-build`, which we will use to build inside, as well as the directory itself.
This call the CMake build directory, which is defined inside CMake by the variable `CMAKE_BINARY_DIR`.
This directory is where CMake builds all intermediate object files, libraries, etc.
We need to be inside this directory, as this is the location where CMake generates the CMake cache file `CMakeCache.txt`
- `mkdir cmake-build`: This re-creates the build directory
- `pushd cmake-build`: This steps inside the build directory
- `cmake ../tutorial/02-setting-up-a-project -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain`: This performs the actual project configuration. We call CMake with the following parameters:
  - `../tutorial/02-setting-up-a-project`: This is the directory containing the main CMake file, which in this case is `D:\Projects\baremetal\tutorial\02-setting-up-a-project`, so relative from the build directory this is `../tutorial/02-setting-up-a-project`.
  CMake starts configuring by using the CMake file `CMakeLists.txt` in this directory
  - `-G "Ninja"`: Thisselects the generator to use. As said in [Ninja](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_BUILD_ENVIRONMENT_NINJA) we use Ninja for this
  - `-DCMAKE_BUILD_TYPE:STRING="Debug"`: This defines the variable `CMAKE_BUILD_TYPE` to be a string with value `"Debug"`.
  This is a standard CMake variable, defining the type of build we'll perform. We tell CMake to configure for the Debug build configuration.
Other choices are `Release`, `RelWithDebInfo` (release with debug info) and `RelMinSize` (release with size optimization)
  - `-DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain`: This defines the variable `CMAKE_TOOLCHAIN_FILE` to be a path, with value `../tutorial/02-setting-up-a-project/baremetal.toolchain`, i.e. the path to our toolchain file.
  This again is a standard CMake variable, that forces CMake to use this file as the toolchain definition file.
- `popd`: This steps out of the build directory again

Output:

```text
~/repo/baremetal/cmake-build ~/repo/baremetal
~/repo/baremetal/cmake-build ~/repo/baremetal
-- CMake 3.29.6
-- TOOLCHAIN_ROOT           /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
-- C++ compiler             /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-g++
-- Archiver                 /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ar
-- Linker                   /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ld
-- ObjCopy                  /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-objcopy
-- Std include path         /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1/include
-- CMAKE_EXE_LINKER_FLAGS=
-- Adding to CMAKE_EXE_LINKER_FLAGS -L/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
-- TOOLCHAIN_ROOT           /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
-- C++ compiler             /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-g++
-- Archiver                 /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ar
-- Linker                   /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ld
-- ObjCopy                  /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-objcopy
-- Std include path         /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1/include
-- CMAKE_EXE_LINKER_FLAGS=   -L/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
-- The CXX compiler identification is GNU 13.3.1
-- The ASM compiler identification is GNU
-- Found assembler: /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
-- Configuring done (0.1s)
-- Generating done (0.0s)
-- Build files have been written to: /home/rene/repo/baremetal/cmake-build
~/repo/baremetal
```

There may be warnings about unused variables, you can ignore these.

Next perform the actual build

```bat
rootdir=`pwd`
pushd tutorial/02-setting-up-a-project
cmake --build $rootdir/cmake-build --target 02-setting-up-a-project
popd
```

- `rootdir=`pwd``: We need to keep the current directory as we will move into a different one
- `pushd tutorial\02-setting-up-a-project`: We move to the directory containing the main CMake file
- `cmake --build $rootdir/cmake-build --target 02-setting-up-a-project`: This performs the actual build:
  - `--build %ROOT%/cmake-build`: This specifies the build directory to use
  - `--build $rootdir/cmake-build`: This is the target we wish to build. A target is the name of a a project in CMake. As we defined the project `02-setting-up-a-project` in [Create project](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATE_PROJECT), we will build this one.
  We could also not specify the target, is which case all targets will be built
- `popd`: We move back to the original directory

```output
~/repo/baremetal/tutorial/02-setting-up-a-project ~/repo/baremetal
[2/2] Linking CXX executable 02-setting-up-a-project
FAILED: 02-setting-up-a-project
: && /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-g++ -g -L/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1 CMakeFiles/02-setting-up-a-project.dir/main.cpp.obj -o 02-setting-up-a-project   && :
/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-exit.o): in function `exit':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/stdlib/exit.c:65:(.text.exit+0x2c): undefined reference to `_exit'
/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-sbrkr.o): in function `_sbrk_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/sbrkr.c:51:(.text._sbrk_r+0x1c): undefined reference to `_sbrk'
/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-writer.o): in function `_write_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/writer.c:49:(.text._write_r+0x24): undefined reference to `_write'
/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-closer.o): in function `_close_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/closer.c:47:(.text._close_r+0x1c): undefined reference to `_close'
/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-lseekr.o): in function `_lseek_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/lseekr.c:49:(.text._lseek_r+0x24): undefined reference to `_lseek'
/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld: /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/lib/libg.a(libc_a-readr.o): in function `_read_r':
/data/jenkins/workspace/GNU-toolchain/arm-13/src/newlib-cygwin/newlib/libc/reent/readr.c:49:(.text._read_r+0x24): undefined reference to `_read'
collect2: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.
~/repo/baremetal
```

Again it is important to be in the correct directory, the directory where our main CMake file is located.

As you can see, in both cases, the source file is compiled, but the link stage failed due to some undefined references.
This is due to the fact that the compiler uses the C standard library, but this leaves a number of platform specific functions undefined.
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
- Line 5: We define a variable `SCRIPTS_DIR` to hold the path to CMake scripts we will be adding (to contain the custom functions).
This variable is added to the CMake cache to make it persistent
- Line 6: We set the variable `CONFIG_DIR` to denote a build configuration specific directory. For now we'll simply set it to Debug, as we are building for the Debug build configuration
- Line 7: We define a variable `DEPLOYMENT_DIR` to point to the location where our final image will be.
This could be the same path as the output directory, however it makes sense to separate built binaries (executables and libraries) from the final images
- Line 8: We set variable `OUTPUT_BASE_DIR` to denote the root for the built binaries.
This directory will be used in combination with `CONFIG_DIR` and the target executable name to form the full path for the target executable file.
By default, CMake will generate all object files, libraries and executables inside the build directory.
This makes it impractical to find our end results, so we will specify a different directory tree for libraries and executables
- Line 10-12: We set, if not done yet, the `SCRIPTS_DIR` variable to the location of our custom CMake scripts.
It is common practice to create a subfolder `cmake` at the project top level and place the scripts there
- Line 14: Lastly, we add the `SCRIPTS_DIR` to the standard CMake search path for CMake modules, `CMAKE_MODULE_PATH`

### Adding project variables {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_ADDING_PROJECT_VARIABLES}

We need to add a few lines to our CMakeLists file, to add definitions, compiler settings, linker options, and link libraries.
For now, we'll target only Raspberry Pi 3.
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
33: set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE
34:     -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2)
35: set(PROJECT_COMPILE_OPTIONS_ASM_PUBLIC )
36: 
37: set(PROJECT_INCLUDE_DIRS_PRIVATE )
38: set(PROJECT_INCLUDE_DIRS_PUBLIC )
39: 
40: set(PROJECT_LINK_OPTIONS ${CMAKE_EXE_LINKER_FLAGS} -Wl,--section-start=.init=0x80000 -T ${CMAKE_SOURCE_DIR}/link.ld -nostdlib -nostartfiles)
41: 
42: set(PROJECT_DEPENDENCIES )
43: 
44: set(PROJECT_LIBS
45:     ${PROJECT_DEPENDENCIES}
46:     )
47: 
48: set(PROJECT_SOURCES
49:     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
50:     )
51: set(PROJECT_INCLUDES_PUBLIC)
52: set(PROJECT_INCLUDES_PRIVATE)
53: 
54: if (PLATFORM_BAREMETAL)
55:     set(START_GROUP -Wl,--start-group)
56:     set(END_GROUP -Wl,--end-group)
57: endif()
58: 
59: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
```

So, after the project is defined, we add the following lines:
- Line 20: We define the variable `PROJECT_TARGET_NAME`, which sets the file name used for our executable to `02-setting-up-a-project.elf`
- Line 22-23: We define the variables `PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE` and `PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC` which will contain compiler definitions.
For now these are empty. There are two, as we can have definitions only for this executable (private) and possibly exported to other targets (public).
As an executable file does not export anything, this is a bit superfluous, but keeping this structure will prove helpful later on when creating libraries.
- Line 25-31: We define the variable `PROJECT_COMPILE_OPTIONS_CXX_PRIVATE` in the same way to set compiler options.
Here we set the private compiler options to be:
  - `-mcpu=cortex-a53 -mlittle-endian -mcmodel=small`: Set CPU architecture options for Raspberry Pi 3 (CPU is AMD Cortex-A53, we use small endian architecture)
So we always fall back to Cortex-A53 behaviour
  - `-Wall`: Set warning level to the highest possible level
  - `-Wextra`: Set warning level to even higher level
  - `-Werror`: Treat warnings as errors
  - `-Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -Wno-unused-variable -Wno-unused-parameter`: Switch off some specific warnings for Debug configuration
  - `-ffreestanding`: A very important option, meaning we are building a standalone (baremetal) application
  - `-fsigned-char`: Use signed characters
  - `-nostartfiles`: Do not use the standard startup / shutdown files (`crtbegin.o` and `crtend.o`).
  Another important option that basically tells the compiler not to add initialization or uninitialization code, as we will do this ourselves
  - `-mno-outline-atomics`: Needed to make sure we can build with ARMv8.0 and atomics
  - `-nostdinc`: Do not use the standard C library includes
  - `-nostdlib`: Do not use the standard C libraries
  - `-nostdinc++`: Do not use the standard C++ library includes
  - `-fno-exceptions`: Do not enable exceptions
  - `-fno-rtti`: Do not use Run Time Type Information
  - `-O0`: Do not `optimize
  - `-std=gnu++17: Support C\++17 language
- Line 32: We set the variable `PROJECT_COMPILE_OPTIONS_CXX_PUBLIC` to set the public compiler options to be empty, meaning we don't export anything
- Line 33-34: We define the variables `PROJECT_COMPILE_OPTIONS_ASM_PRIVATE` in the same way to set assembler options.
Here we set the private assembler options to be:
  - `-mcpu=cortex-a53 -mlittle-endian -mcmodel=small`: Set CPU architecture options for Raspberry Pi 3 (CPU is AMD Cortex-A53, we use small endian architecture)
  - `-O2`: Optimize code
- Line 35: We set the variable `PROJECT_COMPILE_OPTIONS_ASM_PUBLIC` to set the public assembler options to be empty, meaning we don't export anything
- Line 37-38: We defines the variables `PROJECT_INCLUDE_DIRS_PRIVATE` and `PROJECT_INCLUDE_DIRS_PUBLIC`, again in the same way to specify include directories.
For now, everything is in the same directory, so we leave this empty
- Line 40: We define the variable `PROJECT_LINK_OPTIONS` to specify linker options
  - `${CMAKE_EXE_LINKER_FLAGS}`: Use the existing linker options (the linker options specified in the [toolchain file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILD_FOR_TARGET_TOOLCHAIN_FILE))
  - `-nostdlib`: Do not use the standard C libraries
  - `-nostartfiles`: Do not use the standard startup / shutdown files (`crtbegin.o` and `crtend.o`)
  - `-Wl,--section-start=.init=0x80000`: Define the start address of the executable to be 0x80000 (this is always the same for 64 bit architectures).
  Note that we specify .init here, meaning that we expect the startup code to be in this section.
  This is important and should match with the definition in the startup assembly code
  - `-T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld`: Use the specified linker definition file (see [Adding linker definition file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_ADDING_THE_LINKER_DEFINITION_FILE))
- Line 42: We define a variable `PROJECT_DEPENDENCIES` to hold any libraries we will be depending on. For now this is empty
- Line 44-45: We define the variable `PROJECT_LIBS` to hold all libraries we will be linking to. This means all dependencies, and all specified standard libraries
- Line 47-49: We define the variable `PROJECT_SOURCE` to hold the source files to be used for building, as before
- Line 51-52: We define the variables `PROJECT_INCLUDES_PUBLIC` and `PROJECT_INCLUDES_PRIVATE` to hold the public and private header files to be used for building, as before
- Line 54-57: We define two extra variables, only understood by gcc, to group libraries together for correct resolution when linking. These are for the start of the grouping `START_GROUP` and the end of the grouping `END_GROUP`

### Setting up the target {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_SETTING_UP_THE_TARGET}

We then need to link these variables to the target we're building:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
59: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
60: 
61: target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
62: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
63: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
64: target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE})
65: target_compile_definitions(${PROJECT_NAME} PUBLIC  ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC})
66: target_compile_options(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE})
67: target_compile_options(${PROJECT_NAME} PUBLIC  ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC})
68:
```

Explanation:
- Line 61: We link to the specified libraries (empty list for now) in a group. Hence `${START_GROUP} ${PROJECT_LIBS} ${END_GROUP}`
- Line 62-63: We specify include directories for both private and public use
- Line 64-65: We specify compiler definitions for both private and public use
- Line 66-67: We specify compiler options for both private and public use. Notice that we don't use the assembler options yet, we'll get to that later

Next we specify the linker options.
As the options are specified as a list separated by semicolons, and we need to create a string of values separated by spaces, we use a custom function:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
69: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
70: message(STATUS "PROJECT_LINK_OPTIONS=${PROJECT_LINK_OPTIONS_STRING}")
71: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
72:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
73: endif()
```

Explanation:
- Line 69: Converts the list of linker options to a string with spaces as delimiter using the custom functions `list_to_string`.
We will define this custom function later on. 
Normal behaviour when converting a list to a string would be to insert semicolons as separators, so we define a special function to fix this
- Line 70: Prints the string
- Line 71-73: Sets the linker flags, only if the string is not empty

Lastly, we set the executable file name, and the location of executable files and libraries:

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
74: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
75: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/lib)
76: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin)
```

Explanation:
- Line 74: sets the target executable name `OUTPUT_NAME`, a standard target property
- Line 75: defines the target location for static libraries `ARCHIVE_OUTPUT_DIRECTORY`, a standard target property.
This is not strictly needed here, as we don't generate any statis libraries, but again this structure helps later on when creating libraries, and helps recognize things
- Line 76: defines the target location for executables `RUNTIME_OUTPUT_DIRECTORY`, a standard target property

The last two lines use the variables `OUTPUT_BASE_DIR` and `CONFIG_DIR` defined before in [Setting up for custom CMake modules and binary tree](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_SETTING_UP_FOR_CUSTOM_CMAKE_MODULES_AND_BINARY_TREE).
It is common practice to collect output files together in a separate binaries tree.
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
This function is needed as the linker options are placed in a list, which when converted to a string will be a semicolon delimited list, not space delimited.

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

The we need to set up the linker definitions file as pointed to by the [linker options](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_COMPILER_SETTINGS_ADDING_PROJECT_VARIABLES).

We'll name this file `link.ld` as used by the linker options and add the following contents:

```text
File: tutorial/02-setting-up-a-project/link.ld
1: /*------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
55:     . = SIZEOF_HEADERS;
56:     /* Executable initialization section */
57:     .init : {
58:         *(.init)
59:     } : init
60: 
61:     /* Executable cleanup section */
62:     .fini : {
63:         *(.fini)
64:     } : fini
65: 
66:     /* Code section */
67:     .text : {
68:         *(.text* .text.* .gnu.linkonce.t*)
69: 
70:         _etext = .;
71:     } : text
72: 
73:     /* Executable read only data section */
74:     .rodata : {
75:         *(.rodata*)
76:     } : rodata
77: 
78:     /* Executable static initialization section */
79:     .init_array : {
80:         __init_start = .;
81: 
82:         KEEP(*(.init_array*))
83: 
84:         __init_end = .;
85:     }
86: 
87:     /* Executable read/write data section */
88:     .data : {
89:         *(.data*)
90:     } : data
91: 
92:     /* Executable uninitialized data section */
93:     .bss : {
94:         __bss_start = .;
95: 
96:         *(.bss*)
97:         *(COMMON)
98: 
99:         __bss_end = .;
100:     } : data
101: }
102: /* bss size is actual size rounded down to blocks of 8 bytes */
103: __bss_size = (__bss_end - __bss_start) >> 3;
```

The linker definition file defines the different sections in the executable file.

- Line 41: The `ENTRY(_start)` statement sets the starting point of the executable to the location denoted by label `_start`. This label need to be defined an will contain the startup code. We will cover this in a minute see [Startup assembly code](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_STARTUP_ASSEMBLY_CODE).
- Line 44-51: The `PHDRS` part defines the Program Header Table (refer to [ELF header format](pdf/elf-format.pdf) for more information)
- Line 57-59: .init is the initialization section, which contains code with is normally in the `crtbegin.o` file. This is also where we will place the startup code (hence the reference to .init in the linker options)
- Line 62-64: .fini is the cleanup section, similarly containing code in the `crtend.o` file. For now this is empty
- Line 67-71: .text is the code section
  - This contains subsections (`.text* .text.* .gnu.linkonce.t*`)
  - We save the end of the text section in a variable \_etext for later reference
- Line 74-76: .rodata is the constants section, i.e. it contains data that is read-only
- Line 79-85: .init_array is the static initializer section.
It contains a table of functions used to initialize static data, such as constructors of static class objects.
This is always stored.
We will get to this in [Improving startup and static initialization](06-improving-startup-static-initialization.md)
- Line 89-90: .data contains read/write data for the executable
- Line 93-100: .bss contains unitialized data, such as simple global (extern) or local (static) variables. They are normally zeroed out before the program starts.
We save the start and the end of this section in variables \__bss_start and \__bss_end respectively
- Line 103: data in the .bss section is initialized in chunks of 8 bytes (rounded down to the nearest multiple of 8)
We save the number of 8 byte chunks in a variable \__bss_size (hence the >> 3 operation)

## Startup assembly code {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_STARTUP_ASSEMBLY_CODE}

The final step is adding startup code.

In order for the CPU to be correctly initialized, and the cores handled correctly, we need to add some assembly code.
This code will roughly do the following:
- It will check which core our code is running.
If it is core 0, we will continue, otherwise we will start a waiting loop, simply waiting for events (one of the events that may happen is shutdown, we need to wait for that).
Effectively, we simply halt all cores except core 0
- Next, we set the stack pointer to a pointer after our code, so that we have a stack to work with.
We expect the code to be no larger than 2 Mb, and reserved 128 Kb for the stack (the stack grows down)
- Then we initialize the data in the .bss section (see above in [Adding the linker definition file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_ADDING_THE_LINKER_DEFINITION_FILE))
- Lastly, we call the main() function defined in [Create source file](#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATE_SOURCE_FILE).

The startup code will be stored in the `start.S` assembly file:

```assembly
File: tutorial/02-setting-up-a-project/start.S
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
17: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
41: #define MEM_KERNEL_START        _start
42: #define KERNEL_MAX_SIZE         0x200000 // Max 2 Mb
43: #define MEM_KERNEL_END          (MEM_KERNEL_START + KERNEL_MAX_SIZE)
44: 
45: #define KERNEL_STACK_SIZE       0x20000 // 128 Kb
46: #define MEM_KERNEL_STACK        (MEM_KERNEL_END + KERNEL_STACK_SIZE)
47: 
48: .section ".init"
49: 
50: .global _start
51: 
52: _start:
53:     // Read MPIDR_EL1 register, low 7 bits contain core id (as we have 4 cores, we mask only lowest two bits)
54:     mrs     x1, mpidr_el1
55:     // Mask away everything but the core id
56:     and     x1, x1, #3
57:     // If core id is 0, continue
58:     cbz     x1, core0
59:     // If core id > 0, start wait loop
60: waitevent:
61:     wfe
62:     b       waitevent
63: 
64: core0:
65:     // core 0
66: 
67:     // set top of stack just before our code (stack grows to a lower address per AAPCS64)
68:     ldr     x1, =MEM_KERNEL_STACK
69:     mov     sp, x1
70: 
71:     // clear bss
72:     // Load bss start
73:     ldr     x1, =__bss_start
74:     // Load bss size (size is number of 8 byte blocks in bss section)
75:     ldr     w2, =__bss_size // In 8 byte chunks, so actual size is __bss_size * 8
76:     // If bss is empty
77:     cbz     w2, empty_bss
78: 
79: clear_bss_loop:
80:     // Store 0 in x1 location for 8 bytes, increment x1 by 8
81:     str     xzr, [x1], #8
82:     // Count down number of blocks
83:     sub     w2, w2, #1
84:     // Loop as loop as the end is not reached
85:     cbnz    w2, clear_bss_loop
86: 
87:     // jump to C code, should not return
88: empty_bss:
89:     bl      main
90:     // for fail safety, halt this core too
91:     b       waitevent
```

Without going into too much detail, the code performs the following steps:

- Line 41: We define `MEM_KERNEL_START` to be the start of our code
- Line 42: We define the maximum kernel size `KERNEL_MAX_SIZE` to be 2 Mb
- Line 43: We define the end of the kernel to be `MEM_KERNEL_END` = (`MEM_KERNEL_START` + `KERNEL_MAX_SIZE`)
- Line 45: We define the stack size `KERNEL_STACK_SIZE` to be 128 Kb
- Line 46: We define the top os stack `MEM_KERNEL_STACK` = (`MEM_KERNEL_END` + `KERNEL_STACK_SIZE`)
- Line 48: the startup code is part of the `.init` subsection defined in the linker definition file and the linker options
- Line 52: this is the entry point for the \_start function, essentially the entry point of our code, as also mentioned in the linker definition file
- Line 54-58: determine which core the code is running on, and jump to `core0` if the core id is 0. We'll explain the MPIDR_EL1 register, among others, a bit later
- Line 60-62: in other cases we loop waiting for an event (effectively halting the core)
- Line 68-69: we set the stack pointer
- Line 71-77: information on the `.bss` section is retrieved. If the .bss section is empty, we jump to `empty_bss`
- Line 81-85: write 0 to the next 8 bytes of the .bss section, and while not at the end, repeat
- Line 88-89: we're done clearing the `.bss` section.
Call to the main() function in `tutorial/02-setting-up-a-project/main.cpp`
- Line 91: when main() returns, also halt core 0

For more information on ARM assembly code code also [getting-started-with-ARM-assembly-language](pdf/getting-started-with-arm-assembly-language.pdf).
For more information on the `MPIDR_EL1` register see also [Arm Architecture Registers](pdf/ARM-architecture-registers.pdf) page 1390, or [ARM® Architecture Reference Manual](pdf/arm-aarch64-reference-manual.pdf), section D8.2.64.

## Update project CMake file {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_UPDATE_PROJECT_CMAKE_FILE}

We need to add the startup code to the project.

Update the file `tutorial/02-setting-up-a-project/CMakeLists.txt`

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
File: d:\Projects\Private\RaspberryPi\baremetal\tutorial\02-setting-up-a-project\CMakeLists.txt
...
50: set(PROJECT_SOURCES
51:     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
52:     ${CMAKE_CURRENT_SOURCE_DIR}/start.S
53:     )
...
```

## Creating an image {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATING_AN_IMAGE}

Now we can build the code to generate `output/Debug/bin/02-setting-up-a-project.elf`, however that application cannot simply be run in QEMU or on a physical device.
We need to create an image for that. This is a fairly simple step, adding a new target for the image.

We will first create some more custom CMake functions for this.

### Add custom CMake functions {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATING_AN_IMAGE_ADD_CUSTOM_CMAKE_FUNCTIONS}

Update the file `tutorial/02-setting-up-a-project/cmake/functions.cmake`.

```cmake
File: tutorial/02-setting-up-a-project/cmake/functions.cmake
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
23:         COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}
24:         COMMAND ${CMAKE_OBJCOPY} ${OUTPUT_BASE_DIR}/${CONFIG_DIR}/bin/${TARGET_NAME} -O binary ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
25:         DEPENDS ${project}
26:         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
27:     )
28: 
29:     add_custom_target(${target} ALL DEPENDS
30:         ${DEPLOYMENT_DIR}/${CONFIG_DIR}/${target}/${image}
31:         )
32: endfunction()
33: 
34: function(setup_image target)
35:     project(${target}-image
36:         DESCRIPTION "Kernel image for ${target}")
37: 
38:     message(STATUS "\n**********************************************************************************\n")
39:     message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
40: 
41:     message("\n** Setting up ${PROJECT_NAME} **\n")
42: 
43:     set(DEPENDENCY ${target})
44:     set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)
45: 
46:     create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
47: endfunction()
```

- Line 9-32: We create a new function `create_image` which takes three parameters:
  - target is name of the CMake target we are creating for the image file
  - image is the name of the image file
  - project is the CMake (executable) target we are creating this image file for
  - Line 10: We simply print the command and its parameters
  - Line 12-15: We check whether the executable target we are creating the image file for is an actual CMake target, i.e. that this is defined as a target for an executable.
If not, we print a message and fail
  - Line 17-18: We get the name of the executable file into variable `OUTPUT_NAME`, and print it
  - Line 20: We print a message that we are going to generate an image
  - Line 21-27: We create a CMake custom command (a kind or object needed to run external executables in CMake)
    - `OUTPUT` specifies the file it will create (in order to enable other targets to depend on it)
    - The first `COMMAND` makes sure the directory for the image file exists
    - The second `COMMAND` executes the objcopy command to copy the executable file to the image file
    - `DEPENDS` specifies any CMake targets this command depends on.
This is our executable target, as everytime we generate a new executable file, we need to build the image file again
    -  `WORKING_DIRECTORY` is the directory from which this command is invoked
  - Line 29-31: We create a new custom CMake target for the image file
    - `DEPENDS` specifies the file we depend on, which is the image file.
    - So this target will build itself by running the custom command we just created.
    - And due to its dependencies, it will only do so if the executable target is rebuilt.
- Line 34-47: We create a new function `setup_image` which takes one parameter:
  - dependency is name of the CMake executable target we are depending on
  - Line 35-36: We define a new project for the image file, using the name of the executable target, and adding `-image`
  - Line 38-41: We print information on the directory we're in and which project we're setting up
  - Line 43: We define the variable `DEPENDENCY` to hold the name of the CMake target we depend on
  - Line 44: We define the variable `IMAGE_NAME` to hold the filename of the image file.
This will be kernel8.img for Raspberry Pi, but we use the variable `BAREMETAL_TARGET_KERNEL` to make the function more generic, to also cater for other Raspberyr Pi versions
  - Line 46: We call the function `create_image` to create the custom command and custom target we discussed above

This may all seem complex, but this functionality can be used again later on by simply changing the parameters.

### Update CMake file {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CREATING_AN_IMAGE_UPDATE_CMAKE_FILE}

We need to use the functions added in the previous section.
Update the file `tutorial/02-setting-up-a-project/CMakeLists.txt`

```cmake
File: tutorial/02-setting-up-a-project/CMakeLists.txt
...
...
16: include(functions)
17: 
18: set(BAREMETAL_TARGET_KERNEL kernel8)
19: 
20: project(02-setting-up-a-project
21:     DESCRIPTION "Application to demonstrate building using CMake"
22:     LANGUAGES CXX ASM)
23: 
24: message(STATUS "\n**********************************************************************************\n")
25: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
26: 
27: message("\n** Setting up ${PROJECT_NAME} **\n")
28: 
...
87: 
88: set(BAREMETAL_EXECUTABLE_TARGET ${PROJECT_NAME})
89: setup_image(${PROJECT_NAME})
```

- Line 18: We create the variable `BAREMETAL_TARGET_KERNEL` to specify the base name of the kernel image to create.
This depends on the target platform we're building for. Here we set it to `kernel8` for Raspberry Pi 3 64 bit (see [Startup kernel for ARM CPU](#TUTORIAL_00_RASPBERRY_PI_STARTUP_KERNEL_FOR_ARM_CPU))
- Line 24-27: We print information on the current directory and name of the executable project
- Line 88: We create the variable `BAREMETAL_EXECUTABLE_TARGET` to save the executable target name.
Although not strictly needed, we may want to use it after the next line, which replaces the current project, and therefore changes the project name
- Line 89: We call the custom function `setup_image` to create a project, custom target and custom command for the image

We can now start to build the application and image.

## Configuring {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CONFIGURING}

We first need to configure the build for CMake:

### Windows {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CONFIGURING_WINDOWS}

As described before:

```bat
del /S /f /q cmake-build\*.*
rmdir cmake-build
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/02-setting-up-a-project -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain
popd
```

### Linux {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_CONFIGURING_LINUX}

As described before:

```bash
rm -rf cmake-build/
mkdir cmake-build
pushd cmake-build
cmake ../tutorial/02-setting-up-a-project -G Ninja -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../tutorial/02-setting-up-a-project/baremetal.toolchain
popd
```

Output of the configuration step will be similar to the following:

```text
~/repo/baremetal/cmake-build ~/repo/baremetal
-- CMake 3.30.2
-- TOOLCHAIN_ROOT           /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
-- C++ compiler             /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-g++
-- Archiver                 /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ar
-- Linker                   /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ld
-- ObjCopy                  /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-objcopy
-- Std include path         /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1/include
-- CMAKE_EXE_LINKER_FLAGS=
-- Adding to CMAKE_EXE_LINKER_FLAGS -L/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
-- TOOLCHAIN_ROOT           /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf
-- Processor                aarch64
-- Platform tuple           aarch64-none-elf
-- Assembler
-- C compiler               /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
-- C++ compiler             /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-g++
-- Archiver                 /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ar
-- Linker                   /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ld
-- ObjCopy                  /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-objcopy
-- Std include path         /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1/include
-- CMAKE_EXE_LINKER_FLAGS=   -L/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
-- The CXX compiler identification is GNU 13.3.1
-- The ASM compiler identification is GNU
-- Found assembler: /opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
--
**********************************************************************************

--
## In directory: /home/rene/repo/baremetal/tutorial/02-setting-up-a-project

** Setting up 02-setting-up-a-project **

-- PROJECT_LINK_OPTIONS= -L/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1 -Wl,--section-start=.init=0x80000 -T /home/rene/repo/baremetal/tutorial/02-setting-up-a-project/link.ld -nostdlib -nostartfiles
-- The C compiler identification is GNU 13.3.1
--
**********************************************************************************

--
## In directory: /home/rene/repo/baremetal/tutorial/02-setting-up-a-project

** Setting up 02-setting-up-a-project-image **

-- create_image 02-setting-up-a-project-image kernel8.img 02-setting-up-a-project
-- TARGET_NAME 02-setting-up-a-project.elf
-- generate /home/rene/repo/baremetal/tutorial/02-setting-up-a-project/../../deploy/Debug/02-setting-up-a-project-image/kernel8.img from /home/rene/repo/baremetal/tutorial/02-setting-up-a-project/../../output/Debug/bin/02-setting-up-a-project
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /home/rene/repo/baremetal/cmake-build
~/repo/baremetal
```

This will show information first on the executable target project `02-setting-up-a-project` and then the image creation project `02-setting-up-a-project-image`.

## Building {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILDING}

Then we can build the targets.

The target we're going to build is the image, so `02-setting-up-a-project-image`.
This will automatically build all its dependencies, so `02-setting-up-a-project.elf` will also be built

After this step, we will have built the application in `output/Debug/bin/02-setting-up-a-project.elf`, and the image in `deploy/Debug/02-setting-up-a-project-image/kernel8.img`.

The image is very small, as the application basically does nothing, but you have built your __first baremetal application!__

### Windows {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILDING_WINDOWS}

Similar to what was described before, however, we use the image as the target to be built.

```bat
set ROOT=%CD%
pushd tutorial\02-setting-up-a-project
cmake --build %ROOT%/cmake-build --target 02-setting-up-a-project-image
popd
```

### Linux {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_BUILDING_LINUX}

Similar to what was described before, however, we use the image as the target to be built.

```bash
rootdir=`pwd`
pushd tutorial/02-setting-up-a-project
cmake --build $rootdir/cmake-build --target 02-setting-up-a-project-image
popd
```

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
D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gdb.exe --args 02-setting-up-a-project.elf
```

```text
GNU gdb (Arm GNU Toolchain 13.3.Rel1 (Build arm-13.24)) 14.2.90.20240526-git
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
(gdb)
```

### Linux {#TUTORIAL_02_SETTING_UP_A_PROJECT_FOR_BUILDING_AND_DEBUGGING_DEBUGGING_LINUX}

To run QEMU:

```bash
cd <project_root_dir>
qemu-system-aarch64 -M raspi3b -kernel deploy/Debug/02-setting-up-a-project-image/kernel8.img -serial stdio -s -S
```

To run GDB:
```bash
cd output/Debug/bin/
gdb-multiarch --args 02-setting-up-a-project.elf
```

```text
GNU gdb (Ubuntu 15.0.50.20240403-0ubuntu1) 15.0.50.20240403-git
Copyright (C) 2024 Free Software Foundation, Inc.
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
(gdb)
```

In GDB (example is in Linux, Windows is similar):

```gdb
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
0x0000000000000000 in ?? ()
(gdb) load
Loading section .init, size 0x50 lma 0x80000
Loading section .text, size 0x8 lma 0x80050
Start address 0x0000000000080000, load size 88
Transfer rate: 85 KB/sec, 44 bytes/write.
(gdb) b start.S:54
Breakpoint 1 at 0x80000: file /home/rene/repo/baremetal/tutorial/02-setting-up-a-project/start.S, line 54.
(gdb) c
Continuing.

Thread 1 hit Breakpoint 1, _start () at /home/rene/repo/baremetal/tutorial/02-setting-up-a-project/start.S:54
54          mrs     x1, mpidr_el1
(gdb) n
56          and     x1, x1, #3
(gdb) info registers x1
x1             0x80000000          2147483648
(gdb) n
58          cbz     x1, core0
(gdb) n
68          ldr     x1, =MEM_KERNEL_STACK
(gdb) n
69          mov     sp, x1
(gdb) b start.S:89
Breakpoint 2 at 0x80034: file /home/rene/repo/baremetal/tutorial/02-setting-up-a-project/start.S, line 89.
(gdb) c
Continuing.

Thread 1 hit Breakpoint 2, empty_bss () at /home/rene/repo/baremetal/tutorial/02-setting-up-a-project/start.S:89
89          bl      main
(gdb) s
main () at /home/rene/repo/baremetal/tutorial/02-setting-up-a-project/main.cpp:3
3           return 0;
```

Notice that we step over the instruction `mov sp, x1` using a breakpoint again. This is because the debugger tends to hang on this instruction as the stack pointer changes.
Notice also that we can inspect register values. As you can see, the value is `0x80000000`, so the lower bit are all zero. This means we're running on core 0.

So we ended up in line 3 of the main() function:

```cpp
File: main.cpp
1: int main()
2: {
3:     return 0;
4: }
```

Next, we step one further, to go to line 4, and then one more, ending up in start.S, and then close down debugging again (Windows, similar for Linux):

```gdb
(gdb) n
4       }
(gdb) n
empty_bss () at D:/Projects/baremetal/tutorial/02-setting-up-a-project\start.S:84
84	    b       waitevent
(gdb) kill
Kill the program being debugged? (y or n) y
[Inferior 1 (process 1) killed]
(gdb) quit
```

Next: [03-setting-up-project-structure](03-setting-up-project-structure.md)
