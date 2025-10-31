# Tutorial 06: First application - using the console - UART1 {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1}

@tableofcontents

Yes, I know, we had to come a long way to get to actually write some code.
But I'd like to put down some good fundaments.

Now that we have set up our project structure, integrated with Visual Studio, are able to configure, build, run and debug our projects, and introduced some helpful tools, let's start some actual development.

One of the simplest things to start with is the serial console.
This is also practical, as using the serial console enables us to write output from the application, and also get input.

There are multiple serial consoles on Raspberry Pi.
Raspberry Pi 3 has 2 (UART0 and UART1), Raspberry Pi 4 has 6 (all accessible through GPIO), and Raspberry Pi 5 also has 6 (of which 5 accessible through GPIO).
Raspberry Pi 3 has a so called mini UART, which is on the auxiliary peripheral. Raspberry Pi 4 also has this mini UART, Raspberry Pi 5 does not.

See also [Attaching-a-serial-console](#TUTORIAL_01_SETTING_UP_FOR_DEVELOPMENT_DEPLOYMENT_MECHANISM_ATTACHING_A_SERIAL_CONSOLE).

For this application, we will use UART1, which is the easiest to set up.
It has less functionality, but for a simple serial console both are equally suitable. As said, Raspberry Pi 5 is not equipped with this console, so this example won't work on that device.

## Creating the baremetal library structure - Step 1 {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1}

We will first start to create a library named baremetal, which will contain all our basic functionality.

In `code/libraries`, there is a CMake file, that currently only holds the following contents:

```cmake
File: code/libraries/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3:
```

- We will create a new folder underneath `code/libraries` named baremetal.
- We will add subdirectories underneath named `include` and `src`, which will hold header (.h) files and source files (.cpp) respectively.
- Underneath `code/libraries/baremetal/include` we will create another directory named `baremetal`. The reason for this will be explained shortly.
- We will create a new `CMakeLists.txt` file in `code/libraries/baremetal` with the following contents:

```cmake
File: code/libraries/baremetal/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3:
```

Note that all this can be done from within Visual Studio, or if desired directly on the command line, whatever you prefer.

The file `code/libraries/CMakeLists.txt` will be extended to include our new folder `code/libraries/baremetal`:

```cmake
File: code/libraries/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3:
4: add_subdirectory(baremetal)
```

The resulting structure will look like this:

<img src="images/treeview-baremetal-library.png" alt="Tree view" width="500"/>

The reason for adding an extra directory with the same name as the library underneath `code/libraries/baremetal/include` is the following:

- It is common practice to export the directory of a library containing the header files to applications, so they can include the header files.
- Suppose we have two libraries, let's say `lib-one` and `lib-two`, which both contain a header file `inc.h`.
- We export `lib-one/include` as well as `lib-two/include` as directories to enable the application to use them.
- Now, if the the application wanted to include `inc.h`:

`#include "inc.h"`

- Which one would it include? The answer is, it's undefined.
- By adding an extra directory underneath, so `lib-one/include/lib-one`, and `lib-two/include/lib-two`, and then exporting `lib-one/include` and `lib-two/include`, the application could include either:

`#include "lib-one/inc.h"`

`#include "lib-two/inc.h"`

The name of the subdirectory does not matter much, but clearly it is more readable in code to use the name of the library.
This way it is clear when including a header which library it is part of.

When including headers, we following the rule: If it is a standard compiler header, we use brackets `<>` otherwise we use quotes `""`.

### Creating the library code {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_CREATING_THE_LIBRARY_CODE}

So let's start adding some code for the library, and set up the project for this library.

As we will need to add quite some code, let's do it in small steps.

The first step we'll take is including a header to contain standard ARM instructions, starting with the NOP (No operation) instructions. We will then use this in a loop to wait for a while.

### ARMInstructions.h {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_ARMINSTRUCTIONSH}

We add ARM instructions.

Create the file `code/libraries/baremetal/include/baremetal/ARMInstructions.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/ARMInstructions.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : ARMInstructions.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Common instructions for e.g. synchronization
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: /// @file
43: /// ARM instructions represented as macros for ease of use.
44: ///
45: /// For specific registers, we also define the fields and their possible values.
46: 
47: /// @brief NOP instruction
48: #define NOP()                           asm volatile("nop")
```

This header declares standard ARM instructions.
For now we will only use the NOP (no operation) instruction.
More will be added later.
Notice that we added Doxygen comments as well.

### Dummy.cpp {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_DUMMYCPP}

As the baremetal currently has no source files, only a header file, we will create a dummy C++ file to allow for building the library:

```cpp
File: code/libraries/baremetal/src/Dummy.cpp
1: // This is just a dummy file to allow creating the baremetal project
```

### Project setup for baremetal {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_PROJECT_SETUP_FOR_BAREMETAL}

First let's set up the project for the library:

```cmake
File: code/libraries/baremetal/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3: 
4: project(baremetal
5:     DESCRIPTION "Bare metal library"
6:     LANGUAGES CXX ASM)
7: 
8: set(PROJECT_TARGET_NAME ${PROJECT_NAME})
9: 
10: set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE ${COMPILE_DEFINITIONS_C})
11: set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
12: set(PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE ${COMPILE_DEFINITIONS_ASM})
13: set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
14: set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
15: set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
16: set(PROJECT_INCLUDE_DIRS_PRIVATE )
17: set(PROJECT_INCLUDE_DIRS_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
18: 
19: set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})
20: 
21: set(PROJECT_DEPENDENCIES
22:     )
23: 
24: set(PROJECT_LIBS
25:     ${LINKER_LIBRARIES}
26:     ${PROJECT_DEPENDENCIES}
27:     )
28: 
29: file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp src/*.S)
30: set(GLOB_RECURSE PROJECT_INCLUDES_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/*.h)
31: set(PROJECT_INCLUDES_PRIVATE )
32: 
33: set(PROJECT_INCLUDES_PRIVATE )
34: 
35: if (CMAKE_VERBOSE_MAKEFILE)
36:     display_list("Package                           : " ${PROJECT_NAME} )
37:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
38:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
39:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
40:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
41:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
42:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
43:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
44:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
45:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
46:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
47:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
48:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
49:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
50:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
51:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
52:     display_list("Link libs                         : " ${PROJECT_LIBS} )
53:     display_list("Source files                      : " ${PROJECT_SOURCES} )
54:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
55:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
56: endif()
57: 
58: add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
59: target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
60: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
61: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
62: target_compile_definitions(${PROJECT_NAME} PRIVATE
63:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
64:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
65:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
66:     )
67: target_compile_definitions(${PROJECT_NAME} PUBLIC
68:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
69:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
70:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
71:     )
72: target_compile_options(${PROJECT_NAME} PRIVATE
73:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
74:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
75:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
76:     )
77: target_compile_options(${PROJECT_NAME} PUBLIC
78:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
79:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
80:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
81:     )
82: 
83: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
84: 
85: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
86: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
87:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
88: endif()
89: 
90: link_directories(${LINK_DIRECTORIES})
91: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
92: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
93: 
94: show_target_properties(${PROJECT_NAME})
```

You will recognize the structure used in the previous demo project, with a one addition:

- line 17: We export the `code/libraries/baremetal/include` directory publicly
- line 30: We export the header files in `code/libraries/baremetal/include/baremetal` publicly

### Update application code {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_UPDATE_APPLICATION_CODE}

We will use the NOP instruction (which is part of the baremetal library) in a simple loop:

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: 
3: /// @file
4: /// Demo application main code
5: 
6: /// <summary>
7: /// Demo application main code
8: /// </summary>
9: /// <returns>For now always 0</returns>
10: int main()
11: {
12:     for (int i = 0; i < 1000000; ++i)
13:     {
14:         NOP();
15:     }
16:     return 0;
17: }
```

### Update project setup for demo application {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_UPDATE_PROJECT_SETUP_FOR_DEMO_APPLICATION}

We will update the demo application to add a dependency on the baremetal library:

```cmake
File: code/applications/demo/CMakeLists.txt
1: project(demo
2:     DESCRIPTION "Demo application"
3:     LANGUAGES CXX ASM)
: 
5: message(STATUS "\n**********************************************************************************\n")
6: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
7: 
8: message("\n** Setting up ${PROJECT_NAME} **\n")
9: 
10: include(functions)
11: 
12: set(PROJECT_TARGET_NAME ${PROJECT_NAME}.elf)
13: 
14: set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE ${COMPILE_DEFINITIONS_C})
15: set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
16: set(PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE ${COMPILE_DEFINITIONS_ASM})
17: set(PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC )
18: set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
19: set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
20: set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
21: set(PROJECT_COMPILE_OPTIONS_ASM_PUBLIC )
22: set(PROJECT_INCLUDE_DIRS_PRIVATE )
23: set(PROJECT_INCLUDE_DIRS_PUBLIC )
24: 
25: set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})
26: 
27: set(PROJECT_DEPENDENCIES
28:     baremetal
29:     )
30: 
31: set(PROJECT_LIBS
32:     ${LINKER_LIBRARIES}
33:     ${PROJECT_DEPENDENCIES}
34:     )
...
```

- line 28: We add a dependency to the baremetal library, such that its exported include directories become available, and we link to this library.
Notice we use the name of the library, as in its project name.

### Selectively build tutorial code {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_SELECTIVELY_BUILD_TUTORIAL_CODE}

In order to keep our builds as specific as possible, let's selectively disable building all the code for previous tutorials.

Update the main CMake file:

```cmake
File: CMakeLists.txt
...
229: option(BUILD_TUTORIALS "Build all tutorials" ON)
230: 
231: add_subdirectory(code)
232: 
233: if(BUILD_TUTORIALS)
234:     add_subdirectory(tutorial)
235: endif()
```

This introduces a new variable `BUILD_TUTORIALS`, which is `ON` by default, and only if `ON` includes the `tutorial` directory.

Now let's update CMakeSettings.json and switch off building the tutorial code:

```json
File: CMakeSettings.json
...
5:             "name": "BareMetal-RPI3-Debug",
6:             "generator": "Ninja",
7:             "configurationType": "Debug",
8:             "buildRoot": "${projectDir}\\cmake-${name}",
9:             "installRoot": "${projectDir}\\output\\install\\${name}",
10:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=OFF",
...
17:             "name": "BareMetal-RPI4-Debug",
18:             "generator": "Ninja",
19:             "configurationType": "Debug",
20:             "buildRoot": "${projectDir}\\cmake-${name}",
21:             "installRoot": "${projectDir}\\output\\install\\${name}",
22:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBUILD_TUTORIALS=OFF",
...
29:             "name": "BareMetal-RPI5-Debug",
30:             "generator": "Ninja",
31:             "configurationType": "Debug",
32:             "buildRoot": "${projectDir}\\cmake-${name}",
33:             "installRoot": "${projectDir}\\output\\install\\${name}",
34:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5 -DBUILD_TUTORIALS=OFF",
...
41:             "name": "BareMetal-RPI3-Release",
42:             "generator": "Ninja",
43:             "configurationType": "Release",
44:             "buildRoot": "${projectDir}\\cmake-${name}",
45:             "installRoot": "${projectDir}\\output\\install\\${name}",
46:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=OFF",
...
53:             "name": "BareMetal-RPI4-Release",
54:             "generator": "Ninja",
55:             "configurationType": "Release",
56:             "buildRoot": "${projectDir}\\cmake-${name}",
57:             "installRoot": "${projectDir}\\output\\install\\${name}",
58:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBUILD_TUTORIALS=OFF",
...
65:             "name": "BareMetal-RPI5-Release",
66:             "generator": "Ninja",
67:             "configurationType": "Release",
68:             "buildRoot": "${projectDir}\\cmake-${name}",
69:             "installRoot": "${projectDir}\\output\\install\\${name}",
70:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5 -DBUILD_TUTORIALS=OFF",
...
77:             "name": "BareMetal-RPI3-RelWithDebInfo",
78:             "generator": "Ninja",
79:             "configurationType": "RelWithDebInfo",
80:             "buildRoot": "${projectDir}\\cmake-${name}",
81:             "installRoot": "${projectDir}\\output\\install\\${name}",
82:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=OFF",
...
89:             "name": "BareMetal-RPI4-RelWithDebInfo",
90:             "generator": "Ninja",
91:             "configurationType": "RelWithDebInfo",
92:             "buildRoot": "${projectDir}\\cmake-${name}",
93:             "installRoot": "${projectDir}\\output\\install\\${name}",
94:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBUILD_TUTORIALS=OFF",
...
101:             "name": "BareMetal-RPI5-RelWithDebInfo",
102:             "generator": "Ninja",
103:             "configurationType": "RelWithDebInfo",
104:             "buildRoot": "${projectDir}\\cmake-${name}",
105:             "installRoot": "${projectDir}\\output\\install\\${name}",
106:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5 -DBUILD_TUTORIALS=OFF",
...
113:             "name": "BareMetal-RPI3-MinSizeRel",
114:             "generator": "Ninja",
115:             "configurationType": "MinSizeRel",
116:             "buildRoot": "${projectDir}\\cmake-${name}",
117:             "installRoot": "${projectDir}\\output\\install\\${name}",
118:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=OFF",
...
125:             "name": "BareMetal-RPI4-MinSizeRel",
126:             "generator": "Ninja",
127:             "configurationType": "MinSizeRel",
128:             "buildRoot": "${projectDir}\\cmake-${name}",
129:             "installRoot": "${projectDir}\\output\\install\\${name}",
130:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4 -DBUILD_TUTORIALS=OFF",
...
137:             "name": "BareMetal-RPI5-MinSizeRel",
138:             "generator": "Ninja",
139:             "configurationType": "MinSizeRel",
140:             "buildRoot": "${projectDir}\\cmake-${name}",
141:             "installRoot": "${projectDir}\\output\\install\\${name}",
142:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5 -DBUILD_TUTORIALS=OFF",
...
```

### Configure and build {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_CONFIGURE_AND_BUILD}

We are now able to configure the project again, and build it.

The output for the configure step should be similar to:

```text
1> CMake generation started for configuration: 'BareMetal-RPI3-Debug'.
1> Command line: "C:\WINDOWS\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="D:\Projects\baremetal\output\install\BareMetal-RPI3-Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH="D:\Projects\baremetal\baremetal.toolchain" -DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=OFF -DCMAKE_MAKE_PROGRAM="C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" "D:\Projects\baremetal" 2>&1"
1> Working directory: D:\Projects\baremetal\cmake-BareMetal-RPI3-Debug
1> [CMake] -- CMake 3.20.21032501-MSVC_2
1> [CMake] -- Building for Raspberry Pi 3
1> [CMake] -- 
1> [CMake] ** Setting up project **
1> [CMake] --
1> [CMake] -- 
1> [CMake] ##################################################################################
1> [CMake] -- 
1> [CMake] ** Setting up toolchain **
1> [CMake] --
1> [CMake] -- TOOLCHAIN_ROOT           D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf
1> [CMake] -- Processor                aarch64
1> [CMake] -- Platform tuple           aarch64-none-elf
1> [CMake] -- Assembler                D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
1> [CMake] -- C compiler               D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
1> [CMake] -- C++ compiler             D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-g++.exe
1> [CMake] -- Archiver                 D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ar.exe
1> [CMake] -- Linker                   D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ld.exe
1> [CMake] -- ObjCopy                  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-objcopy.exe
1> [CMake] -- Std include path         D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1/include
1> [CMake] -- CMAKE_EXE_LINKER_FLAGS=   -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
1> [CMake] -- C++ compiler version:    13.3.1
1> [CMake] -- C compiler version:      13.3.1
1> [CMake] -- C++ supported standard:  17
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/applications
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/applications/demo
1> [CMake] 
1> [CMake] ** Setting up demo **
1> [CMake] 
1> [CMake] -- Package                           :  demo
1> [CMake] -- Package description               :  Demo application
1> [CMake] -- Defines C - public                : 
1> [CMake] -- Defines C - private               : 
1> [CMake] -- Defines C++ - public              : 
1> [CMake] -- Defines C++ - private             :  PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3 _DEBUG
1> [CMake] -- Defines ASM - private             :  PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3
1> [CMake] -- Compiler options C - public       : 
1> [CMake] -- Compiler options C - private      : 
1> [CMake] -- Compiler options C++ - public     : 
1> [CMake] -- Compiler options C++ - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter
1> [CMake] -- Compiler options ASM - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2
1> [CMake] -- Include dirs - public             : 
1> [CMake] -- Include dirs - private            : 
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      :  baremetal
1> [CMake] -- Link libs                         :  baremetal
1> [CMake] -- Source files                      :  D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/start.S
1> [CMake] -- Include files - public            : 
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] -- Properties for demo
1> [CMake] -- Target type                       :  EXECUTABLE
1> [CMake] -- Target defines                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3 _DEBUG> $<$<COMPILE_LANGUAGE:ASM>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target options                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target include dirs public        :  INCLUDES-NOTFOUND
1> [CMake] -- Target include dirs private       :  INCLUDES-NOTFOUND
1> [CMake] -- Target link libraries             :  -Wl,--start-group baremetal -Wl,--end-group
1> [CMake] -- Target link options               :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target exported defines           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported options           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported include dirs      :  INCLUDE_DIRS_EXPORTS-NOTFOUND
1> [CMake] -- Target exported link libraries    :  -Wl,--start-group baremetal -Wl,--end-group
1> [CMake] -- Target imported dependencies      : 
1> [CMake] -- Target imported link libraries    : 
1> [CMake] -- Target link dependencies          :  LINK_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target manual dependencies        :  EXPLICIT_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target static library location    :  D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/lib
1> [CMake] -- Target dynamic library location   :  LIBRARY_LOCATION-NOTFOUND
1> [CMake] -- Target binary location            :  D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/bin
1> [CMake] -- Target link flags                 :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target version                    :  TARGET_VERSION-NOTFOUND
1> [CMake] -- Target so-version                 :  TARGET_SOVERSION-NOTFOUND
1> [CMake] -- Target output name                :  demo.elf
1> [CMake] -- Target C++ standard               :  17
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/applications/demo
1> [CMake] 
1> [CMake] ** Setting up demo-image **
1> [CMake] 
1> [CMake] -- create_image demo-image kernel8.img demo
1> [CMake] -- TARGET_NAME demo.elf
1> [CMake] -- generate D:/Projects/RaspberryPi/baremetal.github/deploy/Debug/demo-image/kernel8.img from D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/bin/demo
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/libraries
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal
1> [CMake] -- Package                           :  baremetal
1> [CMake] -- Package description               :  Bare metal library
1> [CMake] -- Defines C - public                : 
1> [CMake] -- Defines C - private               : 
1> [CMake] -- Defines C++ - public              : 
1> [CMake] -- Defines C++ - private             :  PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3 _DEBUG
1> [CMake] -- Defines ASM - private             :  PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3
1> [CMake] -- Compiler options C - public       : 
1> [CMake] -- Compiler options C - private      : 
1> [CMake] -- Compiler options C++ - public     : 
1> [CMake] -- Compiler options C++ - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter
1> [CMake] -- Compiler options ASM - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2
1> [CMake] -- Include dirs - public             :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Include dirs - private            : 
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      : 
1> [CMake] -- Link libs                         : 
1> [CMake] -- Source files                      :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/src/Dummy.cpp
1> [CMake] -- Include files - public            : 
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] -- Properties for baremetal
1> [CMake] -- Target type                       :  STATIC_LIBRARY
1> [CMake] -- Target defines                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3 _DEBUG> $<$<COMPILE_LANGUAGE:ASM>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target options                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target include dirs public        :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target include dirs private       :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target link libraries             :  LIBRARIES-NOTFOUND
1> [CMake] -- Target link options               :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target exported defines           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported options           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported include dirs      :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target exported link libraries    :  LIBRARIES_EXPORTS-NOTFOUND
1> [CMake] -- Target imported dependencies      : 
1> [CMake] -- Target imported link libraries    : 
1> [CMake] -- Target link dependencies          :  LINK_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target manual dependencies        :  EXPLICIT_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target static library location    :  D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/lib
1> [CMake] -- Target dynamic library location   :  LIBRARY_LOCATION-NOTFOUND
1> [CMake] -- Target binary location            :  RUNTIME_LOCATION-NOTFOUND
1> [CMake] -- Target link flags                 :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target version                    :  TARGET_VERSION-NOTFOUND
1> [CMake] -- Target so-version                 :  TARGET_SOVERSION-NOTFOUND
1> [CMake] -- Target output name                :  baremetal
1> [CMake] -- Target C++ standard               :  17
1> [CMake] -- Configuring done (0.1s)
1> [CMake] -- Generating done (0.1s)
1> [CMake] -- Build files have been written to: D:/Projects/RaspberryPi/baremetal.github/cmake-Baremetal-RPI3-Debug
1> Extracted CMake variables.
1> Extracted source files and headers.
1> Extracted code model.
1> Extracted toolchain configurations.
1> Extracted includes paths.
1> CMake generation finished.
```

You will notice that now, also the baremetal library is included, and the application project has a dependency on baremetal.

We can then build:

```text
>------ Build All started: Project: baremetal, Configuration: BareMetal-RPI3-Debug ------
  [1/6] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Dummy.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\Dummy.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Dummy.cpp.obj -c ../code/libraries/baremetal/src/Dummy.cpp
  [2/6] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -fanalyzer -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [3/6] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -c ../code/applications/demo/src/start.S
  [4/6] cmd.exe /C "cd . && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E rm -f ..\output\RPI3\Debug\lib\libbaremetal.a && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ar.exe qc ..\output\RPI3\Debug\lib\libbaremetal.a  code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Dummy.cpp.obj && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ranlib.exe ..\output\RPI3\Debug\lib\libbaremetal.a && cd ."
  [5/6] cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -o ..\output\RPI3\Debug\bin\demo.elf  -Wl,--start-group  ../output/RPI3/Debug/lib/libbaremetal.a  -Wl,--end-group && cd ."
  [6/6] cmd.exe /C "cd /D D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug\code\applications\demo && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E make_directory D:/Projects/baremetal/deploy/Debug/demo-image && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal/output/RPI3/Debug/bin/demo.elf -O binary D:/Projects/baremetal/deploy/Debug/demo-image/kernel8.img"

Build All succeeded. 
```

- You can see that the demo application's main.cpp file and baremetal library's Dummy.cpp file are compiled (step 1 and 2)
- Then the demo application's start.S is compiled
- Then the baremetal library is removed and re-created (step 4)
- The demo application is linked, using the baremetal library (step 5)
- And finally the image is created (step 6)

The only intrinsically different steps are step 4 and 5.

#### Removing and re-creating the baremetal library (step 4) {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_CONFIGURE_AND_BUILD_REMOVING_AND_RE_CREATING_THE_BAREMETAL_LIBRARY_STEP_4}

The baremetal library is removed and re-created using the following command:

```text
cmd.exe /C
  "cd . &&
  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    -E 
      rm -f ..\output\RPI3\Debug\lib\libbaremetal.a &&
      D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ar.exe
        qc ..\output\RPI3\Debug\lib\libbaremetal.a
        code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Dummy.cpp.obj &&
      D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ranlib.exe
        ..\output\RPI3\Debug\lib\libbaremetal.a &&
    cd ."
```

Here we see a total of 3 commands being performed inside a command shell:

1. This is just a cd command (actually moving to the same directory)
2. This is a cmake call to remove and re-create the baremetal library, which runs another command shell containing 3 commands:
   1. This command removes the baremetal library `output\RPI3\Debug\lib\libbaremetal.a` (relative to the CMake build directory). We use a CMake internal command to copy with platform differences
   2. This creates the baremetal library. The ar tool is the archiver, which is used to create static libraries (the options qc mean _quick append_ and _create_)
   3. This adds a symbol table to the baremetal library
3. This is again just a cd command (actually moving to the same directory)

#### Linking the demo application (step 5) {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_CONFIGURE_AND_BUILD_LINKING_THE_DEMO_APPLICATION_STEP_5}

```text
cmd.exe /C
  "cd . &&
  D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe
    -g
    -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
    -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld
    -nostdlib -nostartfiles
    code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
    code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
    -o ..\output\RPI3\Debug\bin\demo.elf
    -Wl,--start-group  ../output/RPI3/Debug/lib/libbaremetal.a  -Wl,--end-group &&
  cd ."
```

This is almost the same as shown before, the only difference is that now, the libary group contains the baremetal library:
```text
-Wl,--start-group  ../output/RPI3/Debug/lib/libbaremetal.a  -Wl,--end-group
```

This is due to the added dependency on baremetal in the application's CMake file.

### Running the application {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1_RUNNING_THE_APPLICATION}

Start QEMU, and start debugging as before, only this time, set a breakpoint on line 61 of start.S.

<img src="images/visualstudio-debug-assembly.png" alt="Debugging assembly code" width="800"/>

As you can see, we can also debug inside the assembly code. You can step through and see the code inside main() be called.
One thing you will see after continuing to the next line, is that register x1 is filled with value 0x80000000, which after masking results in 0x00000000. In other words, we're running on core 0.
The other cores will not run, as we did not allow them to yet.

One important remark however:
- The code that sets the stack pointer tends to throw the debugger off balance. So in this case it is better to also set a breakpoint on line 96 of start.S, and simply continue once you get to line 76.

<img src="images/visualstudio-debug-assembly-2.png" alt="Debugging assembly code before jumping into main()" width="480"/>

## Creating the library code - step 2 {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2}

Let's try and write something more useful. We'll write code to set up UART1 and the GPIO pins, and write a string to the console.

In order to set up the console, we will need access to two devices:
- GPIO to set up the connections for UART1 to GPIO pins 14 and 15
- UART1 to configure the console and write to it

### Some information {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_SOME_INFORMATION}

As you can see in [Alternative functions for GPIO](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO), using UART1 means connecting TXD1 (UART1 transmit) to GPIO14 and RXD1 (UART1 receive) to GPIO15.
For this we need to set the function of GPIO 14 and 15 to alternate function 5.
As can be seen in [Raspberry PI auxiliary peripheral](#RASPBERRY_PI_AUXILIARY_PERIPHERAL), [Raspberry Pi UART1](#RASPBERRY_PI_UART1) and [Raspberry Pi GPIO](#RASPBERRY_PI_GPIO), we need to set a number of registers:

- First we need to switch off UART1 so we can program the GPIO pins. So we must write 0 to bit 0 of `AUX_ENABLES`
- Then we need to program GPIO 14 and 15
  - For GPIO 14:
    - For Raspberry Pi 3:
      - We need to write 0x00000000 to `RPI_GPIO_GPPUD` to set the pull up / down mode to off
      - Then wait a bit
      - We need to write 1 to bit 14 of `RPI_GPIO_GPPUDCLK0` to take over the pull up / down mode for GPIO 14
      - Then wait a bit
      - We need to write 0 to bit 14 of `RPI_GPIO_GPPUDCLK0` to take over the pull up / down mode for GPIO 14
    - For Raspberry Pi 4:
      - We need to set bits 28 and 29 of `RPI_GPIO_GPPUPPDN0` to 00 to set the pull up / down mode to off
      - Then wait a bit
      - We need to write 0 to bit 14 of `RPI_GPIO_GPPUDCLK0` to set the pull up / down mode to off
    - We need to write 010 to bits 12-14 of `RPI_GPIO_GPFSEL1`
  - For GPIO 15:
    - For Raspberry Pi 3:
      - We need to write 0x00000000 to `RPI_GPIO_GPPUD` to set the pull up / down mode to off
      - Then wait a bit
      - We need to write 1 to bit 15 of `RPI_GPIO_GPPUDCLK0` to take over the pull up / down mode for GPIO 14
      - Then wait a bit
      - We need to write 0 to bit 15 of `RPI_GPIO_GPPUDCLK0` to take over the pull up / down mode for GPIO 14
    - For Raspberry Pi 4:
      - We need to set bits 30 and 31 of `RPI_GPIO_GPPUPPDN0` to 00 to set the pull up / down mode to off
      - Then wait a bit
      - We need to write 0 to bit 14 of `RPI_GPIO_GPPUDCLK0` to set the pull up / down mode to off
    - We need to write 010 to bits 15-17 of `RPI_GPIO_GPFSEL1`
- After this is done we can program UART1, so we must write 1 to bit 0 of `AUX_ENABLES` (this will enable access to the UART1 registers)
- Then we must disable Tx and Rx by writing 00 to bits 0-1 of `RPI_AUX_MU_CNTL`
- We want to set 8 bit mode, so we write 00000011 to `RPI_AUX_MU_LCR`
- We want set RTS high, so we write 00000000 to `RPI_AUX_MU_MCR`
- For now we disable interrupts so we write 00000000 to `RPI_AUX_MU_IER`
- We wish to enable and clear both receive and transmit FIFO, so we write 11000110 to `RPI_AUX_MU_IIR`
- We need to set the baudrate.
There is a difference between Raspberry Pi 3 and 4, in the clock that is used to form the bit clock for UART1.
- See also [BCM2837 ARM Peripherals](pdf/bcm2837-peripherals.pdf) or [BCM2711 ARM Peripherals](pdf/bcm2711-peripherals.pdf), section `2.2.1 Mini UART implementation details.`
  - For RPI3 the base clock is 250 MHz. For example for 115200 baud, if we calculate 250000000 / 115200 we get 2170.
  We need to divide this by 8, so that is 271.
  Then we decrement by 1 leaving a delay count of 270
  - For RPI4 the base clock is 500 MHz. For example for 115200 baud, if we calculate 500000000 / 115200 we get 4340.
  We need to divide this by 8, so that is 542.
  Then we decrement by 1 leaving a delay count of 541
  - We'll make the baud rate programmable using the formula in the documentation:
```
baudrate = system_clock_freq / (8 * (baudrate_reg + 1)
or
baudrate_reg = system_clock_freq / baudrate / 8 - 1

For Raspberry Pi the system_clock_freq is 250 MHz.
For Raspberry Pi 4, it is 267.3 Mhz, although it is also said to be 500 MHz.
Actually requesting the clock frequency (which will be covered later), reveals the number given here however.
```
  - We need to write the resulting value for `baudrate_reg` to `RPI_AUX_MU_BAUD`
- Then finally we can enable the Tx and Rx by writing 11 to bits 0-1 of `RPI_AUX_MU_CNTL`

In order to access these devices, we'll add a header file with addresses for the different registers.

For this header, we also need some standard definitions and types, so we'll add these first.

### Macros.h {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_MACROSH}

We'll add some basic definitions first.

Create the file `code/libraries/baremetal/include/baremetal/Macros.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/Macros.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Macros.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Common defines
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: /// @file
43: /// Generic macros
44: 
45: /// @brief Convert bit index into integer with zero bit
46: /// @param n Bit index
47: #define BIT0(n)              (0)
48: /// @brief Convert bit index into integer with one bit
49: /// @param n Bit index
50: #define BIT1(n)              (1UL << (n))
```

Some explanation:

- Line 47: We define the macro `BIT0` as a 0 bit at index n. This is not strictly needed, however it enhances readability
- Line 50: We define the macro `BIT1` as a 1 bit at index n, which is used to identify values of field in registers

### Types.h {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_TYPESH}

Then we define basic standard types.

Create the file `code/libraries/baremetal/include/baremetal/Types.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/Types.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Types.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Common types, platform dependent
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: /// @file
43: /// Standard types
44: 
45: /// @brief Unsigned 8 bit integer
46: typedef unsigned char uint8;
47: /// @brief Unsigned 16 bit integer
48: typedef unsigned short uint16;
49: /// @brief Unsigned 32 bit integer
50: typedef unsigned int uint32;
51: /// @brief Unsigned 64 bit integer
52: typedef unsigned long uint64;
53: 
54: /// @brief Signed 8 bit integer
55: typedef signed char int8;
56: /// @brief Signed 16 bit integer
57: typedef signed short int16;
58: /// @brief Signed 32 bit integer
59: typedef signed int int32;
60: /// @brief Signed 64 bit integer
61: typedef signed long int64;
62: 
63: /// @brief Pointer as signed 64 bit integer
64: typedef int64 intptr;
65: /// @brief Pointer as unsigned 64 bit integer
66: typedef uint64 uintptr;
67: /// @brief Unsigned size type
68: typedef uint64 size_type;
69: /// @brief Signed size type
70: typedef int64 ssize_type;
71: 
72: /// @brief Unsigned size
73: typedef size_type size_t;
74: /// @brief Signed size
75: typedef ssize_type ssize_t;
76: 
77: /// @brief Pointer to unsigned volatile byte (for registers)
78: typedef volatile uint8 *regaddr;
```

This header defines the following types:
- 8/16/32/64 bit integer types, both signed an unsigned
- pointer like types (these are 64 bit as we are on a 64 bit system), again both signed and unsigned
- size types, again both signed and unsigned
- a volatile byte pointer. The latter is used to address registers.
Notice that we use a pointer to a byte. This is for easy calculation of addresses.
However, when reading from and writing to registers, we almost always need to write 32 bits.
We will add some utility functions to safely read and write 32 bit values from and to memory.

### MemoryAccess.h {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_MEMORYACCESSH}

We will add a simple class with two static methods to read from and write to memory.

Create the file `code/libraries/baremetal/include/baremetal/MemoryAccess.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryAccess.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccess.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccess
9: //
10: // Description : Memory read/write
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "baremetal/Types.h"
43: 
44: /// @file
45: /// Memory access class
46: 
47: namespace baremetal {
48: 
49: /// <summary>
50: /// Memory access interface
51: /// </summary>
52: class MemoryAccess
53: {
54: public:
55:     static uint32 Read32(regaddr address);
56:     static void Write32(regaddr address, uint32 data);
57: };
58: 
59: } // namespace baremetal
```

This declaration should speak for itself.

### MemoryAccess.cpp {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_MEMORYACCESSCPP}

Let's implement the two methods for MemoryAccess.

Create the file `code/libraries/baremetal/src/MemoryAccess.cpp`:

```cpp
File: code/libraries/baremetal/src/MemoryAccess.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccess.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccess
9: //
10: // Description : Memory read/write
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
38: //------------------------------------------------------------------------------
39: 
40: #include "baremetal/MemoryAccess.h"
41: 
42: /// @file
43: /// Memory access class implementation
44: 
45: using namespace baremetal;
46: 
47: /// <summary>
48: /// Read a 32 bit value from register at address
49: /// </summary>
50: /// <param name="address">Address of register</param>
51: /// <returns>32 bit register value</returns>
52: uint32 MemoryAccess::Read32(regaddr address)
53: {
54:     return *reinterpret_cast<uint32 volatile *>(address);
55: }
56: 
57: /// <summary>
58: /// Write a 32 bit value to register at address
59: /// </summary>
60: /// <param name="address">Address of register</param>
61: /// <param name="data">Data to write</param>
62: void MemoryAccess::Write32(regaddr address, uint32 data)
63: {
64:     *reinterpret_cast<uint32 volatile *>(address) = data;
65: }
```

This could should also speak for itself. Later on, we'll create an abstraction of this code for testing purposes.

<u>As we now have the first functional source file in the project, we can remove the previous `Dummy.cpp` file.</u>

### BCMRegisters.h {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_BCMREGISTERSH}

Now we add some registers of the Broadcom SoC in the Raspberry Pi (specifically those for GPIO and UART1 (mini UART).
This file will include the two headers file defined before.
We will use the prefix `RPI_` for registers that are specific for the Raspberry Pi SoC, and `ARM_` for generic ARM registers.

Create the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : BCMRegisters.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Locations and definitions for Raspberry Pi registers in the Broadcomm SoC
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
38: //------------------------------------------------------------------------------
39: 
40: /// @file
41: /// Register addresses of Raspberry Pi peripheral registers.
42: ///
43: /// For specific registers, we also define the fields and their possible values.
44: 
45: #pragma once
46: 
47: #include "baremetal/Macros.h"
48: #include "baremetal/Types.h"
49: 
50: #if BAREMETAL_RPI_TARGET == 3
51: /// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 3
52: #define RPI_BCM_IO_BASE                 0x3F000000
53: #elif BAREMETAL_RPI_TARGET == 4
54: /// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 4
55: #define RPI_BCM_IO_BASE                 0xFE000000
56: #define ARM_IO_BASE                     0xFF840000
57: #else
58: /// @brief Base address for Raspberry PI BCM I/O for Raspberry Pi 5
59: #define RPI_BCM_IO_BASE                 0x107C000000UL
60: #define ARM_IO_BASE                     0x107C000000UL
61: #endif
62: #if BAREMETAL_RPI_TARGET <= 4
63: /// @brief End address for Raspberry PI 3 / 4 BCM I/O
64: #define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0xFFFFFF)
65: #else
66: /// @brief End address for Raspberry PI 5 BCM I/O
67: #define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0x3FFFFFF)
68: #endif
69: 
70: //---------------------------------------------
71: // Raspberry Pi GPIO
72: //---------------------------------------------
73: 
74: /// @brief Raspberry Pi GPIO registers base address. See @ref RASPBERRY_PI_GPIO
75: #define RPI_GPIO_BASE    RPI_BCM_IO_BASE + 0x00200000
76: /// @brief Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
77: #define RPI_GPIO_GPFSEL0 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000000)
78: /// @brief Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
79: #define RPI_GPIO_GPFSEL1 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000004)
80: /// @brief Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
81: #define RPI_GPIO_GPFSEL2 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000008)
82: /// @brief Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
83: #define RPI_GPIO_GPFSEL3 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000000C)
84: /// @brief Raspberry Pi GPIO function select register 4 (GPIO 40..49) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
85: #define RPI_GPIO_GPFSEL4 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000010)
86: /// @brief Raspberry Pi GPIO function select register 5 (GPIO 50..53) (3 bits / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
87: #define RPI_GPIO_GPFSEL5 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000014)
88: /// @brief Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
89: #define RPI_GPIO_GPSET0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000001C)
90: /// @brief Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
91: #define RPI_GPIO_GPSET1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000020)
92: /// @brief Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
93: #define RPI_GPIO_GPCLR0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000028)
94: /// @brief Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
95: #define RPI_GPIO_GPCLR1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000002C)
96: /// @brief Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
97: #define RPI_GPIO_GPLEV0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000034)
98: /// @brief Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
99: #define RPI_GPIO_GPLEV1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000038)
100: /// @brief Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
101: #define RPI_GPIO_GPEDS0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000040)
102: /// @brief Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R). See @ref RASPBERRY_PI_GPIO
103: #define RPI_GPIO_GPEDS1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000044)
104: /// @brief Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
105: #define RPI_GPIO_GPREN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000004C)
106: /// @brief Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
107: #define RPI_GPIO_GPREN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000050)
108: /// @brief Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
109: #define RPI_GPIO_GPFEN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000058)
110: /// @brief Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
111: #define RPI_GPIO_GPFEN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000005C)
112: /// @brief Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
113: #define RPI_GPIO_GPHEN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000064)
114: /// @brief Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
115: #define RPI_GPIO_GPHEN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000068)
116: /// @brief Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
117: #define RPI_GPIO_GPLEN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000070)
118: /// @brief Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
119: #define RPI_GPIO_GPLEN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000074)
120: /// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
121: #define RPI_GPIO_GPAREN0 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000007C)
122: /// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
123: #define RPI_GPIO_GPAREN1 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000080)
124: /// @brief Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
125: #define RPI_GPIO_GPAFEN0 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000088)
126: /// @brief Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). See @ref RASPBERRY_PI_GPIO
127: #define RPI_GPIO_GPAFEN1 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000008C)
128: #if BAREMETAL_RPI_TARGET == 3
129: /// @brief Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
130: #define RPI_GPIO_GPPUD     reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000094)
131: /// @brief Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
132: #define RPI_GPIO_GPPUDCLK0 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x00000098)
133: /// @brief Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W). Raspberry Pi 3 only. See @ref RASPBERRY_PI_GPIO
134: #define RPI_GPIO_GPPUDCLK1 reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x0000009C)
135: #elif BAREMETAL_RPI_TARGET == 4
136: /// @brief Raspberry Pi GPIO pull up/down pin multiplexer register. Undocumented
137: #define RPI_GPIO_GPPINMUXSD reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000D0)
138: /// @brief Raspberry Pi GPIO pull up/down mode register 0 (GPIO 0..15) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
139: #define RPI_GPIO_GPPUPPDN0  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000E4)
140: /// @brief Raspberry Pi GPIO pull up/down mode register 1 (GPIO 16..31) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
141: #define RPI_GPIO_GPPUPPDN1  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000E8)
142: /// @brief Raspberry Pi GPIO pull up/down mode register 2 (GPIO 32..47) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
143: #define RPI_GPIO_GPPUPPDN2  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000EC)
144: /// @brief Raspberry Pi GPIO pull up/down mode register 3 (GPIO 48..53) (2 bits / GPIO) (R/W). Raspberry Pi 4/5 only. See @ref RASPBERRY_PI_GPIO
145: #define RPI_GPIO_GPPUPPDN3  reinterpret_cast<regaddr>(RPI_GPIO_BASE + 0x000000F0)
146: #else // RPI target 5
147: // Not supported yet
148: #endif
149: 
150: //---------------------------------------------
151: // Raspberry Pi auxiliary (SPI1 / SPI2 / UART1)
152: //---------------------------------------------
153: 
154: /// @brief Raspberry Pi Auxilary registers base address. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
155: #define RPI_AUX_BASE          RPI_BCM_IO_BASE + 0x00215000
156: /// @brief Raspberry Pi Auxiliary IRQ register. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
157: #define RPI_AUX_IRQ           reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000000) // AUXIRQ
158: /// @brief Raspberry Pi Auxiliary Enable register. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
159: #define RPI_AUX_ENABLES       reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000004) // AUXENB
160: 
161: /// @brief Raspberry Pi Auxiliary Enable register values
162: /// @brief Raspberry Pi Auxiliary Enable register Enable SPI2. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
163: #define RPI_AUX_ENABLES_SPI2  BIT1(2)
164: /// @brief Raspberry Pi Auxiliary Enable register Enable SPI1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
165: #define RPI_AUX_ENABLES_SPI1  BIT1(1)
166: /// @brief Raspberry Pi Auxiliary Enable register Enable UART1. See @ref RASPBERRY_PI_AUXILIARY_PERIPHERAL
167: #define RPI_AUX_ENABLES_UART1 BIT1(0)
168: 
169: //---------------------------------------------
170: // Raspberry Pi auxiliary mini UART (UART1)
171: //---------------------------------------------
172: 
173: /// @brief Raspberry Pi Mini UART (UART1) I/O register. See @ref RASPBERRY_PI_UART1
174: #define RPI_AUX_MU_IO         reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000040)
175: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register. See @ref RASPBERRY_PI_UART1
176: #define RPI_AUX_MU_IER        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000044)
177: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register. See @ref RASPBERRY_PI_UART1
178: #define RPI_AUX_MU_IIR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000048)
179: /// @brief Raspberry Pi Mini UART (UART1) Line Control register. See @ref RASPBERRY_PI_UART1
180: #define RPI_AUX_MU_LCR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x0000004C)
181: /// @brief Raspberry Pi Mini UART (UART1) Modem Control register. See @ref RASPBERRY_PI_UART1
182: #define RPI_AUX_MU_MCR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000050)
183: /// @brief Raspberry Pi Mini UART (UART1) Line Status register. See @ref RASPBERRY_PI_UART1
184: #define RPI_AUX_MU_LSR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000054)
185: /// @brief Raspberry Pi Mini UART (UART1) Modem Status register. See @ref RASPBERRY_PI_UART1
186: #define RPI_AUX_MU_MSR        reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000058)
187: /// @brief Raspberry Pi Mini UART (UART1) Scratch register. See @ref RASPBERRY_PI_UART1
188: #define RPI_AUX_MU_SCRATCH    reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x0000005C)
189: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register. See @ref RASPBERRY_PI_UART1
190: #define RPI_AUX_MU_CNTL       reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000060)
191: /// @brief Raspberry Pi Mini UART (UART1) Extra Status register. See @ref RASPBERRY_PI_UART1
192: #define RPI_AUX_MU_STAT       reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000064)
193: /// @brief Raspberry Pi Mini UART (UART1) Baudrate register. See @ref RASPBERRY_PI_UART1
194: #define RPI_AUX_MU_BAUD       reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000068)
195: #if BAREMETAL_RPI_TARGET == 3
196: /// @brief Raspberry Pi Mini UART (UART1) clock frequency on Raspberry PI 3
197: #define AUX_UART_CLOCK 250000000
198: #elif BAREMETAL_RPI_TARGET == 4
199: /// @brief Raspberry Pi Mini UART (UART1) clock frequency on Raspberry PI 4
200: #define AUX_UART_CLOCK 267300000
201: #else
202: // Not supported yet
203: #endif
204: /// @brief Calculate Raspberry Pi Mini UART (UART1) baud rate value from frequency
205: #define RPI_AUX_MU_BAUD_VALUE(baud)   static_cast<uint32>((AUX_UART_CLOCK / (baud * 8)) - 1)
206: 
207: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register values
208: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable transmit interrupts. See @ref RASPBERRY_PI_UART1
209: #define RPI_AUX_MU_IER_TX_IRQ_ENABLE  BIT1(1)
210: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable receive interrupts. See @ref RASPBERRY_PI_UART1
211: #define RPI_AUX_MU_IER_RX_IRQ_ENABLE  BIT1(0)
212: 
213: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register values
214: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO enabled (R). See @ref RASPBERRY_PI_UART1
215: #define RPI_AUX_MU_IIR_TX_FIFO_ENABLE BIT1(7)
216: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO enabled (R). See @ref RASPBERRY_PI_UART1
217: #define RPI_AUX_MU_IIR_RX_FIFO_ENABLE BIT1(6)
218: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO clear (W). See @ref RASPBERRY_PI_UART1
219: #define RPI_AUX_MU_IIR_TX_FIFO_CLEAR  BIT1(2)
220: /// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO clear (W). See @ref RASPBERRY_PI_UART1
221: #define RPI_AUX_MU_IIR_RX_FIFO_CLEAR  BIT1(1)
222: 
223: /// @brief Raspberry Pi Mini UART (UART1) Line Control register values
224: /// @brief Raspberry Pi Mini UART (UART1) Line Control register 7 bit characters. See @ref RASPBERRY_PI_UART1
225: #define RPI_AUX_MU_LCR_DATA_SIZE_7    0
226: /// @brief Raspberry Pi Mini UART (UART1) Line Control register 8 bit characters. See @ref RASPBERRY_PI_UART1
227: #define RPI_AUX_MU_LCR_DATA_SIZE_8    BIT1(0) | BIT1(1)
228: 
229: /// @brief Raspberry Pi Mini UART (UART1) Modem Control register values
230: /// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS low. See @ref RASPBERRY_PI_UART1
231: #define RPI_AUX_MU_MCR_RTS_LOW        BIT1(1)
232: /// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS high. See @ref RASPBERRY_PI_UART1
233: #define RPI_AUX_MU_MCR_RTS_HIGH       BIT0(1)
234: 
235: /// @brief Raspberry Pi Mini UART (UART1) Line Status register values
236: /// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit idle. See @ref RASPBERRY_PI_UART1
237: #define RPI_AUX_MU_LSR_TX_IDLE        BIT1(6)
238: /// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit empty. See @ref RASPBERRY_PI_UART1
239: #define RPI_AUX_MU_LSR_TX_EMPTY       BIT1(5)
240: /// @brief Raspberry Pi Mini UART (UART1) Line Status register receive overrun. See @ref RASPBERRY_PI_UART1
241: #define RPI_AUX_MU_LSR_RX_OVERRUN     BIT1(1)
242: /// @brief Raspberry Pi Mini UART (UART1) Line Status register receive ready. See @ref RASPBERRY_PI_UART1
243: #define RPI_AUX_MU_LSR_RX_READY       BIT1(0)
244: 
245: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register values
246: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable CTS. See @ref RASPBERRY_PI_UART1
247: #define RPI_AUX_MU_CNTL_ENABLE_CTS    BIT1(3)
248: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable RTS. See @ref RASPBERRY_PI_UART1
249: #define RPI_AUX_MU_CNTL_ENABLE_RTS    BIT1(2)
250: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable transmit. See @ref RASPBERRY_PI_UART1
251: #define RPI_AUX_MU_CNTL_ENABLE_TX     BIT1(1)
252: /// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable receive. See @ref RASPBERRY_PI_UART1
253: #define RPI_AUX_MU_CNTL_ENABLE_RX     BIT1(0)
```

We will not go into details, as it will be quite clear from what was discussed before what the registers are.
More information on the GPIO registers can be found in [Raspberry Pi GPIO](#RASPBERRY_PI_GPIO),
as well as in the official [BCM2837 ARM Peripherals](pdf/bcm2837-peripherals.pdf) (page 89),
[BCM2711 ARM Peripherals](pdf/bcm2711-peripherals.pdf) (page 65) and
[RP1 Peripherals](pdf/rp1-peripherals.pdf) (page 14).

As you can see the GPIO register addresses are all prefixed with `RPI_GPIO_`.

You'll also notice there are some registers that are different between Raspberry Pi 3 and Raspberry Pi 4 for GPIO.
The process for setting pull up / down status is different between these boards, as described above.

For Raspberry Pi 3:

- Write the pull up / down setting to the `RPI_GPIO_GPPUD` register (0 for off, 1 for pull down, 2 for pull up, 3 is invalid / unknown)
- Wait 150 cycles
- Write a 1 bit to either `RPI_GPIO_GPPUDCLK0` (for GPIO 0 through 31) or `RPI_GPIO_GPPUDCLK1` (for GPIO 32 through 53)
- Wait 150 cycles
- Write a 0 bit to either `RPI_GPIO_GPPUDCLK0` (for GPIO 0 through 31) or `RPI_GPIO_GPPUDCLK1` (for GPIO 32 through 53)

For Raspberry Pi 4 or later:

- Write the pull up / down setting (0 for off, 1 for pull up, 2 for pull down, note the difference) for bits 0 and 1 of RPI_GPIO_GPPUPPDN0 for GPIO 0, bits 2 and 3 for GPIO 1, etc. and then bits 0 and 1 of RPI_GPIO_GPPUPPDN1 for GPIO 16, etc.

So for Raspberry Pi 3 we have one value, that we 'clock in' by pulsing the respective bit in RPI_GPIO_GPPUDCLK0 or RPI_GPIO_GPPUDCLK1,
for Raspberry Pi 4, we simply write the pull up / down status directly to a register.

Some tutorials will show the same approach for Raspberry Pi 3 and 4, as apparently the 'old' way still works on Raspberry Pi 4.

More information on the Mini UART (UART1) registers can be found in [Raspberry PI auxiliary peripheral](#RASPBERRY_PI_AUXILIARY_PERIPHERAL) and [Raspberry Pi UART1](#RASPBERRY_PI_UART1),
as well as in the official [BCM2837 ARM Peripherals](pdf/bcm2837-peripherals.pdf) (page 8) and
[BCM2711 ARM Peripherals](pdf/bcm2711-peripherals.pdf) (page 10).
Raspberry Pi 5 no longer has a mini UART, so this code will not work there. We will be using another UART, UART0, later on.

The Mini UART or UART1 register addresses are all prefixed with `RPI_AUX_MU_`, the auxiliary registers have the prefix `RPI_AUX_`.

One thing to note:

- Line 193-198: We define a macro `RPI_AUX_MU_BAUD_VALUE` which can be used to determine the value to write to the `RPI_AUX_MU_BAUD` register.
This takes into account the clock frequencies for Raspberry Pi 3 and 4.

### UART1.h {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UART1H}

We need to declare the UART1 functions.

Create the file `code/libraries/baremetal/include/baremetal/UART1.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : UART1.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : UART1
9: //
10: // Description : RPI UART1 class
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "baremetal/Types.h"
43: 
44: /// @file
45: /// Raspberry Pi UART1 serial device declaration
46: 
47: /// @brief baremetal namespace
48: namespace baremetal {
49: 
50: /// @brief GPIO mode
51: enum class GPIOMode
52: {
53:     /// @brief GPIO used as input
54:     Input,
55:     /// @brief GPIO used as output
56:     Output,
57:     /// @brief GPIO used as input, using pull-up
58:     InputPullUp,
59:     /// @brief GPIO used as input, using pull-down
60:     InputPullDown,
61:     /// @brief GPIO used as Alternate Function 0. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
62:     AlternateFunction0,
63:     /// @brief GPIO used as Alternate Function 1. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
64:     AlternateFunction1,
65:     /// @brief GPIO used as Alternate Function 2. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
66:     AlternateFunction2,
67:     /// @brief GPIO used as Alternate Function 3. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
68:     AlternateFunction3,
69:     /// @brief GPIO used as Alternate Function 4. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
70:     AlternateFunction4,
71:     /// @brief GPIO used as Alternate Function 5. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
72:     AlternateFunction5,
73:     /// @brief GPIO mode unknown / not set / invalid
74:     Unknown,
75: };
76: 
77: /// @brief GPIO function
78: enum class GPIOFunction
79: {
80:     /// @brief GPIO used as input
81:     Input,
82:     /// @brief GPIO used as output
83:     Output,
84:     /// @brief GPIO used as Alternate Function 0. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
85:     AlternateFunction0,
86:     /// @brief GPIO used as Alternate Function 1. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
87:     AlternateFunction1,
88:     /// @brief GPIO used as Alternate Function 2. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
89:     AlternateFunction2,
90:     /// @brief GPIO used as Alternate Function 3. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
91:     AlternateFunction3,
92:     /// @brief GPIO used as Alternate Function 4. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
93:     AlternateFunction4,
94:     /// @brief GPIO used as Alternate Function 5. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
95:     AlternateFunction5,
96:     /// @brief GPIO function unknown / not set / invalid
97:     Unknown,
98: };
99: 
100: /// @brief GPIO pull mode
101: enum class GPIOPullMode
102: {
103:     /// @brief GPIO pull mode off (no pull-up or pull-down)
104:     Off,
105:     /// @brief GPIO pull mode pull-down
106:     PullDown,
107:     /// @brief GPIO pull mode pull-up
108:     PullUp,
109:     /// @brief GPIO pull mode unknown / not set / invalid
110:     Unknown,
111: };
112: 
113: /// <summary>
114: /// Encapsulation for the UART1 device.
115: /// </summary>
116: class UART1
117: {
118: private:
119:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
120:     bool m_isInitialized;
121:     /// @brief Baudrate set for device
122:     unsigned m_baudrate;
123: 
124: public:
125:     // Constructs a default UART1 instance.
126:     UART1();
127:     // Initialize the UART1 device. Only performed once, guarded by m_isInitialized.
128:     //
129:     //  Set baud rate and characteristics (8N1) and map to GPIO
130:     void Initialize(unsigned baudrate);
131:     // Return set baudrate
132:     unsigned GetBaudrate() const;
133:     // Read a character
134:     char Read();
135:     // Write a character
136:     void Write(char c);
137:     // Write a string
138:     void WriteString(const char* str);
139: 
140: private:
141:     // Set GPIO pin mode
142:     bool SetMode(MCP23017Pin pinNumber, GPIOMode mode);
143:     // Set GPIO pin function
144:     bool SetFunction(MCP23017Pin pinNumber, GPIOFunction function);
145:     // Set GPIO pin pull mode
146:     bool SetPullMode(MCP23017Pin pinNumber, GPIOPullMode pullMode);
147:     // Switch GPIO off
148:     bool Off(MCP23017Pin pinNumber, GPIOMode mode);
149: };
150: 
151: } // namespace baremetal
```

This header declares the class `UART1` inside the namespace `baremetal`. All types and functions inside the baremetal library will use this namespace.

The class has a default constructor, and a method to initialize it. It also declares a method to read and write a character, as well as to write a string.
The other methods are used to set up the GPIO pins correctly, as part of the Initialize() method. These will move somewhere else later on.

- Line 50-74: We define the enum class type GPIOMode, which combines the input / output / alternate function mode with the pull up / down mode.
- Line 77-97: We define the enum class type GPIOFunctrion which sets either input, output, or one of the alternate functions. These are encoded in three bits, to be written to `RPI_GPIO_GPFSEL0..5`
- Line 100-110: We define the enum class type GPIOPullMode which sets either none, pull up or pull down mode. These are encoded in two bits, to be written to `RPI_GPIO_GPPUD`, or (after conversion) to `RPI_GPIO_GPPUPPDN0..3`
- Line 115-144: We declare the `UART1` class
  - Line 120: We declare the private class member variable `m_isInitialized`
  - Line 122: We declare the private class member variable `m_baudrate`
  - Line 126: We declare the (default) constructor
  - Line 130: We declare the method `Initialize()` which sets up the UART if needed
  - Line 132: We declare the method `GetBaudrate()` which returns the baudrate set for the device
  - Line 134: We declare the method `Read()` which reads a character from the UART
  - Line 136: We declare the method `Write()` which writes a character to the UART
  - Line 138: We declare the method `WriteString()` which writes a string to the UART
  - Line 142: We declare the private method `SetMode()` which sets the mode for a GPIO pin.
This will call the methods `SetFunction()` and `SetPullMode()`
  - Line 144: We declare the private method `SetFunction()` which sets the function for a GPIO pin
  - Line 146: We declare the private method `SetPullMode()` which sets the pull mode for a GPIO pin
  - Line 148: We declare the private method `Off()` which sets sets the output status of a GPIO pin to off (if this is an output pin)

### UART1.cpp {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UART1CPP}

Finally we need to implement the UART1 functions and methods.
Not all functions / methods have been documented here, as we will be moving some elsewhere later on.

Create the file `code/libraries/baremetal/src/UART1.cpp`:

```cpp
File: code/libraries/baremetal/src/UART1.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : UART1.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : UART1
9: //
10: // Description : RPI UART1 class
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
38: //------------------------------------------------------------------------------
39: 
40: #include "baremetal/UART1.h"
41: 
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/MemoryAccess.h"
45: 
46: /// @file
47: /// Raspberry Pi UART1 serial device implementation
48: 
49: /// @brief Total count of GPIO pins, numbered from 0 through 53
50: #define NUM_GPIO 54
51: 
52: namespace baremetal {
53: 
54: #if BAREMETAL_RPI_TARGET == 3
55: /// @brief Count of NOP instructions to pause when programming GPIO for Raspberry Pi 3
56: static const int NumWaitCycles = 150;
57: 
58: /// <summary>
59: /// ait the specified number of cycles, by executing a NOP instruction for each cycle
60: /// </summary>
61: /// <param name="numCycles">NUmber of cycles to wait</param>
62: static void WaitCycles(uint32 numCycles)
63: {
64:     if (numCycles)
65:     {
66:         while (numCycles--)
67:         {
68:             NOP();
69:         }
70:     }
71: }
72: #endif // BAREMETAL_RPI_TARGET == 3
73: 
74: /// <summary>
75: /// Constructs a UART1 instance.
76: /// </summary>
77: UART1::UART1()
78:     : m_isInitialized{}
79:     , m_baudrate{}
80: {
81: }
82: 
83: /// <summary>
84: /// Initialize the UART1 device. Only performed once, guarded by m_isInitialized.
85: ///
86: ///  Set baud rate and characteristics (8N1) and map to GPIO
87: /// </summary>
88: /// <param name="baudrate">Baud rate to set, maximum is 115200</param>
89: void UART1::Initialize(unsigned baudrate)
90: {
91:     if (m_isInitialized)
92:         return;
93: 
94:     // initialize UART
95:     auto value = MemoryAccess::Read32(RPI_AUX_ENABLES);
96:     MemoryAccess::Write32(RPI_AUX_ENABLES, value & ~RPI_AUX_ENABLES_UART1); // Disable UART1, AUX mini uart
97: 
98:     SetMode(14, GPIOMode::AlternateFunction5);
99: 
100:     SetMode(15, GPIOMode::AlternateFunction5);
101: 
102:     MemoryAccess::Write32(RPI_AUX_ENABLES, value | RPI_AUX_ENABLES_UART1); // enable UART1, AUX mini uart
103:     MemoryAccess::Write32(RPI_AUX_MU_CNTL, 0);                             // Disable Tx, Rx
104:     MemoryAccess::Write32(RPI_AUX_MU_LCR, RPI_AUX_MU_LCR_DATA_SIZE_8);     // 8 bit mode
105:     MemoryAccess::Write32(RPI_AUX_MU_MCR, RPI_AUX_MU_MCR_RTS_HIGH);        // RTS high
106:     MemoryAccess::Write32(RPI_AUX_MU_IER, 0);                              // Disable interrupts
107:     MemoryAccess::Write32(RPI_AUX_MU_IIR, RPI_AUX_MU_IIR_TX_FIFO_ENABLE | RPI_AUX_MU_IIR_RX_FIFO_ENABLE |
108:                                               RPI_AUX_MU_IIR_TX_FIFO_CLEAR |
109:                                               RPI_AUX_MU_IIR_RX_FIFO_CLEAR);                       // Clear FIFO
110:     MemoryAccess::Write32(RPI_AUX_MU_BAUD, RPI_AUX_MU_BAUD_VALUE(baudrate));                       // Set baudrate
111:     MemoryAccess::Write32(RPI_AUX_MU_CNTL, RPI_AUX_MU_CNTL_ENABLE_RX | RPI_AUX_MU_CNTL_ENABLE_TX); // Enable Tx, Rx
112: 
113:     m_baudrate = baudrate;
114:     m_isInitialized = true;
115: }
116: 
117: /// <summary>
118: /// Return set baudrate
119: /// </summary>
120: /// <returns>Baudrate set for device</returns>
121: unsigned UART1::GetBaudrate() const
122: {
123:     return m_baudrate;
124: }
125: 
126: /// <summary>
127: /// Receive a character
128: /// </summary>
129: /// <returns>Character received</returns>
130: char UART1::Read()
131: {
132:     // wait until something is in the buffer
133:     // Check Rx FIFO holds data
134:     while (!(MemoryAccess::Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LSR_RX_READY))
135:     {
136:         NOP();
137:     }
138:     // Read it and return
139:     return static_cast<char>(MemoryAccess::Read32(RPI_AUX_MU_IO));
140: }
141: 
142: /// <summary>
143: /// Send a character
144: /// </summary>
145: /// <param name="c">Character to be sent</param>
146: void UART1::Write(char c)
147: {
148:     // wait until we can send
149:     // Check Tx FIFO empty
150:     while (!(MemoryAccess::Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LSR_TX_EMPTY))
151:     {
152:         NOP();
153:     }
154:     // Write the character to the buffer
155:     MemoryAccess::Write32(RPI_AUX_MU_IO, static_cast<uint32>(c));
156: }
157: 
158: /// <summary>
159: /// Write a string
160: /// </summary>
161: /// <param name="str">String to be written</param>
162: void UART1::WriteString(const char* str)
163: {
164:     while (*str)
165:     {
166:         // convert newline to carriage return + newline
167:         if (*str == '\n')
168:             Write('\r');
169:         Write(*str++);
170:     }
171: }
172: 
173: bool UART1::SetMode(MCP23017Pin pinNumber, GPIOMode mode)
174: {
175:     if (pinNumber >= NUM_GPIO)
176:         return false;
177:     if (mode >= GPIOMode::Unknown)
178:         return false;
179:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
180:     {
181:         if (!SetPullMode(pinNumber, GPIOPullMode::Off))
182:             return false;
183: 
184:         if (!SetFunction(pinNumber, static_cast<GPIOFunction>(static_cast<unsigned>(mode) -
185:                                                               static_cast<unsigned>(GPIOMode::AlternateFunction0) +
186:                                                               static_cast<unsigned>(GPIOFunction::AlternateFunction0))))
187:             return false;
188:     }
189:     else if (GPIOMode::Output == mode)
190:     {
191:         if (!SetPullMode(pinNumber, GPIOPullMode::Off))
192:             return false;
193: 
194:         if (!SetFunction(pinNumber, GPIOFunction::Output))
195:             return false;
196:     }
197:     else
198:     {
199:         if (!SetPullMode(pinNumber, (mode == GPIOMode::InputPullUp)     ? GPIOPullMode::PullUp
200:                                     : (mode == GPIOMode::InputPullDown) ? GPIOPullMode::PullDown
201:                                                                         : GPIOPullMode::Off))
202:             return false;
203:         if (!SetFunction(pinNumber, GPIOFunction::Input))
204:             return false;
205:     }
206:     if (mode == GPIOMode::Output)
207:         Off(pinNumber, mode);
208:     return true;
209: }
210: 
211: bool UART1::SetFunction(MCP23017Pin pinNumber, GPIOFunction function)
212: {
213:     if (pinNumber >= NUM_GPIO)
214:         return false;
215:     if (function >= GPIOFunction::Unknown)
216:         return false;
217: 
218:     regaddr selectRegister = RPI_GPIO_GPFSEL0 + (pinNumber / 10) * 4;
219:     uint32 shift = (pinNumber % 10) * 3;
220: 
221:     static const unsigned FunctionMap[] = {0, 1, 4, 5, 6, 7, 3, 2};
222: 
223:     uint32 value = MemoryAccess::Read32(selectRegister);
224:     value &= ~(7 << shift);
225:     value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
226:     MemoryAccess::Write32(selectRegister, value);
227:     return true;
228: }
229: 
230: bool UART1::SetPullMode(MCP23017Pin pinNumber, GPIOPullMode pullMode)
231: {
232:     if (pullMode >= GPIOPullMode::Unknown)
233:         return false;
234:     if (pinNumber >= NUM_GPIO)
235:         return false;
236: 
237: #if BAREMETAL_RPI_TARGET == 3
238:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (pinNumber / 32) * 4;
239:     uint32 shift = pinNumber % 32;
240: 
241:     MemoryAccess::Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
242:     WaitCycles(NumWaitCycles);
243:     MemoryAccess::Write32(clkRegister, static_cast<uint32>(1 << shift));
244:     WaitCycles(NumWaitCycles);
245:     MemoryAccess::Write32(clkRegister, 0);
246: #else
247:     regaddr modeReg = RPI_GPIO_GPPUPPDN0 + (pinNumber / 16) * 4;
248:     unsigned shift = (pinNumber % 16) * 2;
249: 
250:     static const unsigned ModeMap[3] = {0, 2, 1};
251: 
252:     uint32 value = MemoryAccess::Read32(modeReg);
253:     value &= ~(3 << shift);
254:     value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
255:     MemoryAccess::Write32(modeReg, value);
256: #endif
257: 
258:     return true;
259: }
260: 
261: bool UART1::Off(MCP23017Pin pinNumber, GPIOMode mode)
262: {
263:     if (pinNumber >= NUM_GPIO)
264:         return false;
265: 
266:     // Output level can be set in input mode for subsequent switch to output
267:     if (mode >= GPIOMode::Unknown)
268:         return false;
269: 
270:     unsigned regOffset = (pinNumber / 32) * 4;
271:     uint32 regMask = 1 << (pinNumber % 32);
272: 
273:     bool value = false;
274: 
275:     regaddr setClrReg = (value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset;
276: 
277:     MemoryAccess::Write32(setClrReg, regMask);
278: 
279:     return true;
280: }
281: 
282: } // namespace baremetal
```

For AUX and UART1 register documentation, refer to [BCM2837 ARM Peripherals](pdf/bcm2837-peripherals.pdf) (page 8) and [BCM2711 ARM Peripherals](pdf/bcm2711-peripherals.pdf) (page 8).
Be aware that some of the documentation for BCM2835 contains errors, which have been updated for BCM2837. The Line Control Register (see below) contains such errors.

- Line 50: The source file starts by defining the total number of GPIO pins that can be used on Raspberry Pi (it has GPIO pins 0 through 53)
- Line 52: We define the baremetal namespace again, everything else will be inside this namespace
- Line 54-72: Note that these lines are only compiled for Raspberry Pi 3, we don't need them for Raspberry Pi 4
  - Line 56: We define a constant to set the number of NOP cycles we wait between GPIO operations `NumWaitCycles` (defined as 150)
  - Line 62-71: We define a static function `WaitCycles()`, that waits the specified number of NOP instructions.
  As discussed in [BCMRegister.h](#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_BCMREGISTERSH) some operations on the GPIO we need to pause a bit with Raspberry Pi 3, that is what this function is for
- Line 74-81: We implement the UART1 constructor, which only initializes the member variables
- Line 83-115: We implement the `Initialize()` method. This invokes the most part of the code
  - Line 91-92: We check whether the class was already initialized (`m_isInitialized` is true), if so we simply return
  - Line 95: We read the Auxiliary Enables Register, which contains enable bits for SPI1, SPI2 and UART1
  - Line 96: We clear the enable bit for UART1 by writing to the Auxiliary Enables Register. This will disable UART1, as advised in the documentation
  - Line 98: We set GPIO pin 14 to GPIO Alternate function 5, which makes it the UART1 TxD signal (transmit). See also [GPIO functions](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO)
  - Line 100: We set GPIO pin 15 to GPIO Alternate function 5, which makes it the UART1 RxD signal (receive). See also [GPIO functions](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO)
  - Line 102: We set the enable bit again for UART1 by writing to the Auxiliary Enables Register. This will enable UART1, and makes its registers available
  - Line 103: We write to the UART1 Control Register to disable Rx and Tx (receive and transmit) signals
  - Line 104: We set UART1 to 8 bit mode by writing to the Line Control Register
  - Line 105: We set the RTS signal high (see documentation, we don't use RTS in this case) by writing to the Modem Control Register
  - Line 106: We disable UART1 interrupts by writing to the Interrupt Control Register
  - Line 107: We enable and clear the receive and transmit FIFO buffers by writing to the Interrupt Identity Register
  - Line 109: We set the baud rate (speed), using the macro `RPI_AUX_MU_BAUD_VALUE`
  Due to the resulting numbers, setting a bitrate higher than 115200 will lead to problems as the accuracy will not be sufficient
  - Line 111: We enable Rx and Tx signals again by writing to the UART1 Control Register
  - Line 113: We save the baudrate set in `m_baudrate`
  - Line 114: We set `m_isInitialized` to true
- Line 117-124: We implement the `GetBaudrate()` method, which returns the baudrate set
- Line 126-140: We implement the `Read()` method, which reads a character from UART1
  - The function waits for the device to become available for reading by executing NOP instructions while the Line Status Register does not contain a 1 for the Rx ready bit (which signals there is at least one character ready to be read)
  - It then reads a character to the IO register and returns the character
- Line 142-156: We implement the `Write()` method, which writes a character to UART1
  - The function waits for the device to become available for writing by executing NOP instructions while the Line Status Register does not contain a 1 for the Tx empty bit (which signals there is room for a character)
  - It then writes the character to the IO register
- Line 158-171: We implement the `WriteString()` method, which writes a string to UART1
  - This simply iterates through the string and writes the character using the `Write()` method.
  - The only special case is that a line feed (`\n`) in the string is written as a line feed plus carriage return character (`\r` followed by `\n`)
- The rest will be described below

#### SetMode {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UART1CPP_SETMODE}

The `SetMode()` method used in line 98 and 100 is implemented as:
```cpp
File: code/libraries/baremetal/src/UART1.cpp
173: bool UART1::SetMode(MCP23017Pin pinNumber, GPIOMode mode)
174: {
175:     if (pinNumber >= NUM_GPIO)
176:         return false;
177:     if (mode >= GPIOMode::Unknown)
178:         return false;
179:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
180:     {
181:         if (!SetPullMode(pinNumber, GPIOPullMode::Off))
182:             return false;
183: 
184:         if (!SetFunction(pinNumber, static_cast<GPIOFunction>(static_cast<unsigned>(mode) -
185:                                                               static_cast<unsigned>(GPIOMode::AlternateFunction0) +
186:                                                               static_cast<unsigned>(GPIOFunction::AlternateFunction0))))
187:             return false;
188:     }
189:     else if (GPIOMode::Output == mode)
190:     {
191:         if (!SetPullMode(pinNumber, GPIOPullMode::Off))
192:             return false;
193: 
194:         if (!SetFunction(pinNumber, GPIOFunction::Output))
195:             return false;
196:     }
197:     else
198:     {
199:         if (!SetPullMode(pinNumber, (mode == GPIOMode::InputPullUp)     ? GPIOPullMode::PullUp
200:                                     : (mode == GPIOMode::InputPullDown) ? GPIOPullMode::PullDown
201:                                                                         : GPIOPullMode::Off))
202:             return false;
203:         if (!SetFunction(pinNumber, GPIOFunction::Input))
204:             return false;
205:     }
206:     if (mode == GPIOMode::Output)
207:         Off(pinNumber, mode);
208:     return true;
209: }
```

- Line 175-178: Some sanity checks are performed. If these fail, false is returned
- Line 179-188: If the GPIOMode passed signifies an Alternate Function, the pull mode on the pin is switched off using `SetPullMode()`, and the function is set using `SetFunction()`
- Line 189-196: If the GPIOMode is Output, the pull mode on the pin is switched off using `SetPullMode()`, and the function is set to Output using `SetFunction()`
- Line 198-205: If the GPIOMode is Input, InputPullDown or InputPullUp, the pull mode on the pin is set accordingly using `SetPullMode()`, and the function is set to Input using `SetFunction()`
- Line 206-207: If the mode is output, the output is set to off (false, low) using `Off()`

#### SetFunction {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UART1CPP_SETFUNCTION}

The `SetFunction()` method is implemented as:
```cpp
File: code/libraries/baremetal/src/UART1.cpp
211: bool UART1::SetFunction(MCP23017Pin pinNumber, GPIOFunction function)
212: {
213:     if (pinNumber >= NUM_GPIO)
214:         return false;
215:     if (function >= GPIOFunction::Unknown)
216:         return false;
217: 
218:     regaddr selectRegister = RPI_GPIO_GPFSEL0 + (pinNumber / 10) * 4;
219:     uint32 shift = (pinNumber % 10) * 3;
220: 
221:     static const unsigned FunctionMap[] = {0, 1, 4, 5, 6, 7, 3, 2};
222: 
223:     uint32 value = MemoryAccess::Read32(selectRegister);
224:     value &= ~(7 << shift);
225:     value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
226:     MemoryAccess::Write32(selectRegister, value);
227:     return true;
228: }
```

- Line 213-216: Some sanity checks are performed. If these fail, false is returned
- Line 218: We calculate the register for the GPIO function select register.
This register uses 3 bits for every GPIO pin, so a 32 bit register can deal with 10 GPIO pins (the upper 2 bits are not used).
This means there are 6 registers:
  - `RPI_GPIO_GPFSEL0` for GPIO pins 0 to 9
  - `RPI_GPIO_GPFSEL1` for GPIO pins 10 to 19
  - `RPI_GPIO_GPFSEL2` for GPIO pins 20 to 29
  - `RPI_GPIO_GPFSEL3` for GPIO pins 30 to 39
  - `RPI_GPIO_GPFSEL4` for GPIO pins 40 to 49
  - `RPI_GPIO_GPFSEL5` for the rest of the GPIO pins
- Line 219: We calculate the corresponding bit shift to select the correct bits in the GPIO function select register
- Line 222: We read the current value of the GPIO function select register.
Notice that we use the `MemoryAccess` class for this
- Line 224: We mask the bits for the selected GPIO pin
- Line 225: We add the bits for the pull mode of the selected GPIO pin, using the conversion map defined in Line 182.
For input of the map we use the integer conversion of the `GPIOFunction` enum.
This map is not strictly necessary, however it is convenient for readability to define the `GPIOFunction` enum this way
- Line 226: We write the value to the GPIO function select register.
Notice again that we use the `MemoryAccess` class for this

#### SetPullMode {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UART1CPP_SETPULLMODE}

The `SetPullMode()` method is implemented as:
```cpp
File: code/libraries/baremetal/src/UART1.cpp
230: bool UART1::SetPullMode(MCP23017Pin pinNumber, GPIOPullMode pullMode)
231: {
232:     if (pullMode >= GPIOPullMode::Unknown)
233:         return false;
234:     if (pinNumber >= NUM_GPIO)
235:         return false;
236: 
237: #if BAREMETAL_RPI_TARGET == 3
238:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (pinNumber / 32) * 4;
239:     uint32 shift = pinNumber % 32;
240: 
241:     MemoryAccess::Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
242:     WaitCycles(NumWaitCycles);
243:     MemoryAccess::Write32(clkRegister, static_cast<uint32>(1 << shift));
244:     WaitCycles(NumWaitCycles);
245:     MemoryAccess::Write32(clkRegister, 0);
246: #else
247:     regaddr modeReg = RPI_GPIO_GPPUPPDN0 + (pinNumber / 16) * 4;
248:     unsigned shift = (pinNumber % 16) * 2;
249: 
250:     static const unsigned ModeMap[3] = {0, 2, 1};
251: 
252:     uint32 value = MemoryAccess::Read32(modeReg);
253:     value &= ~(3 << shift);
254:     value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
255:     MemoryAccess::Write32(modeReg, value);
256: #endif
257: 
258:     return true;
259: }
```

- Line 232-235: Some sanity checks are performed. If these fail, false is returned
- Line 238-245: For Raspberry Pi 3 we set the pull mode
  - Line 238: We calculate the register for the GPIO pull up/down clock.
This register uses 1 bit for every GPIO pin, so a 32 bit register can deal with 32 GPIO pins.
This means there are 2 registers:
    - `RPI_GPIO_GPPUDCLK0` for GPIO pins 0 to 31
    - `RPI_GPIO_GPPUDCLK1` for the rest of the GPIO pins
  - Line 239: We calculate the corresponding bit shift to select the correct bit in the GPIO pull up/down clock register.
The GPIO pull up/down clock register uses one bit for every GPIO pin
  - Line 241: We write the pull mode to the GPIO pull up/down register.
For the value we use the integer conversion of the `GPIOPullMode` enum.
  - Line 242: We wait a bit (`NumWaitCycles` NOP operations)
  - Line 243: We set the GPIO pull up/down clock bit high. This acts as a strobe to set the pull mode for the GPIO pin
  - Line 244: We wait a bit again
  - Line 245: We set the GPIO pull up/down clock bit low again
- Line 247-255: For Raspberry Pi 4 we set the pull mode
  - Line 247: We calculate the register for the GPIO pull up/down register.
Raspberry Pi 4 use two bits for every pin, so a 32 bit register can deal with 16 GPIO pins.
This means there are 4 registers:
    - `RPI_GPIO_GPPUPPDN0` for GPIO pins 0 to 15
    - `RPI_GPIO_GPPUPPDN1` for GPIO pins 16 to 31
    - `RPI_GPIO_GPPUPPDN2` for GPIO pins 32 to 47
    - `RPI_GPIO_GPPUPPDN3` for the rest of the GPIO pins
  - Line 248: We calculate the corresponding bit shift to select the correct bits in the GPIO pull up/down register
  - Line 252: We read the current value of the GPIO pull up/down register
  - Line 253: We mask the bits for the selected GPIO pin
  - Line 254: We add the bits for the pull modeof the selected GPIO pin, using the conversion map defined in Line 211.
For input of the map we use the integer conversion of the `GPIOPullMode` enum.
Note that these values are different than for Raspberry Pi 3, hence the mapping
  - Line 255: We write the value to the GPIO pull up/down register

#### Off {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UART1CPP_OFF}

The `Off()` method is implemented as:
```cpp
File: code/libraries/baremetal/src/UART1.cpp
261: bool UART1::Off(MCP23017Pin pinNumber, GPIOMode mode)
262: {
263:     if (pinNumber >= NUM_GPIO)
264:         return false;
265: 
266:     // Output level can be set in input mode for subsequent switch to output
267:     if (mode >= GPIOMode::Unknown)
268:         return false;
269: 
270:     unsigned regOffset = (pinNumber / 32) * 4;
271:     uint32 regMask = 1 << (pinNumber % 32);
272: 
273:     bool value = false;
274: 
275:     regaddr setClrReg = (value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset;
276: 
277:     MemoryAccess::Write32(setClrReg, regMask);
278: 
279:     return true;
280: }
```

- Line 263-268: Some sanity checks are performed. If these fail, false is returned
- Line 270: We calculate the index for the GPIO set or clear register.
A GPIO pin can be set by writing a 1 to the correct GPIO pin set register at the correct bit offset, and can be reset by writing a 1 to the correct GPIO pin clear register at the correct bit offset.
This register uses 1 bit for every GPIO pin, so a 32 bit register can deal with 32 GPIO pins.
This means there are 2 registers:
  - `RPI_GPIO_GPSET0` or `RPI_GPIO_GPCLR0` for GPIO pins 0 to 31
  - `RPI_GPIO_GPSET1` or `RPI_GPIO_GPCLR1` for the rest of the GPIO pins
- Line 271: We calculate the corresponding bit shift to select the correct bit in the GPIO set of clear register
- Line 273: We set the value. This is a bit superfluous, but we will be reusing this code later
- Line 275: We decide depending on the value whether to use the GPIO set register or GPIO clear register
- Line 277: We write the corresponding bit in the correct register

### Update startup code {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE}

#### Startup.S {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_STARTUPS}

As we are going to write to registers, we first need to set up the system such that this is allowed.
For more information on ARM exception levels, see [AArch64 Exception Model](pdf/aarch64-exception-model.pdf).
By default, all access to registers on Exception Level 1 (EL1) and below will be trapped, leading to an exception at EL2.
By default (depending on the boot firmware, which we will not handle here), we start in EL2.

So we will need to do some programming in assembly to make sure the SoC is set up correctly, so when we move to EL1, we can write to registers.
Also, in order to prepare for running code which uses stack and heap, we will change the startup code.

This code will use a memory map, which will be loaded through two additional headers, that will be covered soon.
Also, as all applications will be using the same startup code, it is more logical to move this code to the baremetal library.
We therefore remove `code/applications/demo/src/start.S`, and add a new file `code/libraries/baremetal/src/Startup.S`

```asm
File: code/libraries/baremetal/src/Startup.S
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Startup.S
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Startup code. This is the entry point to any executable. It puts all cores except core 0 in sleep mode.
11: //               For core 0, it sets the stack pointer to just below the code (as the stack grows down), and then calls main().
12: //               Note: this file is based on the Circle startup assembly file by Rene Stange.
13: //
14: //------------------------------------------------------------------------------
15: //
16: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
17: //
18: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
19: //
20: // Permission is hereby granted, free of charge, to any person
21: // obtaining a copy of this software and associated documentation
22: // files(the "Software"), to deal in the Software without
23: // restriction, including without limitation the rights to use, copy,
24: // modify, merge, publish, distribute, sublicense, and /or sell copies
25: // of the Software, and to permit persons to whom the Software is
26: // furnished to do so, subject to the following conditions :
27: //
28: // The above copyright notice and this permission notice shall be
29: // included in all copies or substantial portions of the Software.
30: //
31: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
32: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
33: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
34: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
35: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
36: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
37: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
38: // DEALINGS IN THE SOFTWARE.
39: //
40: //------------------------------------------------------------------------------
41: 
42: #include "baremetal/SysConfig.h"
43: 
44: .macro armv8_switch_to_el1_m, xreg1, xreg2
45: 
46:     // Initialize Generic Timers
47:     mrs     \xreg1, cnthctl_el2
48:     orr     \xreg1, \xreg1, #0x3        // Enable EL1 access to timers
49:     msr     cnthctl_el2, \xreg1
50:     msr     cntvoff_el2, xzr
51: 
52:     // Initilize MPID/MPIDR registers
53:     mrs     \xreg1, midr_el1
54:     mrs     \xreg2, mpidr_el1
55:     msr     vpidr_el2, \xreg1
56:     msr     vmpidr_el2, \xreg2
57: 
58:     // Disable coprocessor traps
59:     mov     \xreg1, #0x33ff
60:     msr     cptr_el2, \xreg1            // Disable coprocessor traps to EL2
61:     msr     hstr_el2, xzr               // Disable coprocessor traps to EL2
62:     mov     \xreg1, #3 << 20
63:     msr     cpacr_el1, \xreg1           // Enable FP/SIMD at EL1
64: 
65:     // Initialize HCR_EL2
66:     mov     \xreg1, #(1 << 31)          // 64bit EL1
67:     msr     hcr_el2, \xreg1
68: 
69:     // SCTLR_EL1 initialization
70:     //
71:     // setting RES1 bits (29,28,23,22,20,11) to 1
72:     // and RES0 bits (31,30,27,21,17,13,10,6) +
73:     // UCI,EE,EOE,WXN,nTWE,nTWI,UCT,DZE,I,UMA,SED,ITD,
74:     // CP15BEN,SA0,SA,C,A,M to 0
75:     mov     \xreg1, #0x0800
76:     movk    \xreg1, #0x30d0, lsl #16
77:     msr     sctlr_el1, \xreg1
78: 
79:     // Return to the EL1_SP1 mode from EL2
80:     mov     \xreg1, #0x3c4
81:     msr     spsr_el2, \xreg1            // EL1_SP0 | D | A | I | F
82:     adr     \xreg1, label1
83:     msr     elr_el2, \xreg1
84:     eret
85: label1:
86: .endm
87: 
88: .section .init
89: 
90:     .globl _start
91: _start:                                 // normally entered from armstub8 in EL2 after boot
92:     // Read MPIDR_EL1 register, low 7 bits contain core id (as we have 4 cores, we mask only lowest two bits)
93:     mrs     x1, mpidr_el1
94:     // Mask away everything but the core id
95:     and     x1, x1, #3
96:     // If core id is 0, continue
97:     cbz     x1, core0
98:     // If core id > 0, start wait loop
99: waitevent:
100:     wfe
101:     b       waitevent
102: 
103: core0:
104:     // core 0
105:     mrs     x0, CurrentEL               // check if already in EL1t mode?
106:     cmp     x0, #4
107:     beq     EL1
108: 
109:     ldr     x0, =MEM_EXCEPTION_STACK    // IRQ, FIQ and exception handler run in EL1h
110:     msr     sp_el1, x0                  // init their stack
111: 
112:     armv8_switch_to_el1_m x0, x1
113: 
114: EL1:
115:     ldr     x0, =MEM_KERNEL_STACK       // main thread runs in EL1t and uses sp_el0
116:     mov     sp, x0                      // init its stack
117: 
118:     // clear bss
119:     // Load bss start
120:     ldr     x1, =__bss_start
121:     // Load bss size (size is number of 8 byte blocks in bss section)
122:     ldr     w2, =__bss_size // In 8 byte chunks, so actual size is __bss_size * 8
123:     // If bss is empty
124:     cbz     w2, empty_bss
125: 
126: clear_bss_loop:
127:     // Store 0 in x1 location for 8 bytes, increment x1 by 8
128:     str     xzr, [x1], #8
129:     // Count down number of blocks
130:     sub     w2, w2, #1
131:     // Loop as long as the end is not reached
132:     cbnz    w2, clear_bss_loop
133: 
134:     // jump to C code, should not return
135: empty_bss:
136:     bl      main                        // Jump to main()
137:     // For fail safety, halt this core too
138:     b       waitevent
139: 
140: // End
```

- Line 42: We include `SysConfig.h`. This header will define some defaults, and then include `MemoryMap.h`. Both will be handled in the next sections.
- Line 44-86: A macro to set the ARM registers correctly, this will be explained in [Macro armv8_switch_to_el1_m](#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_MACRO_ARMV8_SWITCH_TO_EL1_M) below
- Line 88: Start of the .init section
- Line 90: Declaration of the _start function, such that it can be linked elsewhere
- Line 91: Label of the _start function, which is where the function actually starts
- Line 93-97: As before, we determine which core the code is running on, and jump to `core0` if the core id is 0. We'll explain the MPIDR_EL1 register, among others, a bit later
- Line 100-101: In other cases we loop waiting for an event (effectively halting the core)
- Line 105: We read the current exception level, see [ARM architecture registers](pdf/arm-architecture-registers.pdf), section `CurrentEL, Current Exception Level`.
This will contain the current exception level in bit 2 and 3, the other bits will be 0.
- Line 106: We check whether the value read is equal to 4 (i.e. bits 3 and 2 are `01`), which means EL1, otherwise we continue
- Line 107: If the values are equal, we jump to label EL1, meaning that initialization was already done.
- Line 109: We load the exception stack address for core 0.
This is also used for FIQ (fast interrupt) and IRQ (normal interrupt).
We will get to interrupts later on.
The variable referenced here is defined in `MemoryMap.h` (see [MemoryMap.h](#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_MEMORYMAPH))
- Line 110: We set the stack pointer for EL1 exceptions to this address
- Line 112: We switch to EL1 using the macro armv8_switch_to_el1_m. See below in [Macro armv8_switch_to_el1_m](#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_MACRO_ARMV8_SWITCH_TO_EL1_M)
- Line 115: We load the kernel stack address. The variable referenced here is defined in `MemoryMap.h` (see [MemoryMap.h](#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_MEMORYMAPH))
- Line 116: We set the code stack pointer to this address
- Line 120-124: As before, information on the `.bss` section is retrieved. If the .bss section is empty, we jump to `empty_bss`
- Line 128-132: As before, we write 0 to the next 8 bytes of the .bss section, and while not at the end, repeat
- Line 135-136: We're done clearing the `.bss` section, and call to the main() function
- Line 138: when main() returns, also halt core 0

Unless core 0 fires up the other cores, the other cores are never started.
So we don't actually need to check which core we're on now, but this will prepare us for multiprocessing later on.

#### Macro armv8_switch_to_el1_m {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_MACRO_ARMV8_SWITCH_TO_EL1_M}

The macro armv8_switch_to_el1_m sets up a number of registers for our code to run smoothly.
This contains quite some intricate details, so bare with me.

```asm
File: code/libraries/baremetal/src/Startup.S
44: .macro armv8_switch_to_el1_m, xreg1, xreg2
45: 
46:     // Initialize Generic Timers
47:     mrs     \xreg1, cnthctl_el2
48:     orr     \xreg1, \xreg1, #0x3        // Enable EL1 access to timers
49:     msr     cnthctl_el2, \xreg1
50:     msr     cntvoff_el2, xzr
51: 
52:     // Initilize MPID/MPIDR registers
53:     mrs     \xreg1, midr_el1
54:     mrs     \xreg2, mpidr_el1
55:     msr     vpidr_el2, \xreg1
56:     msr     vmpidr_el2, \xreg2
57: 
58:     // Disable coprocessor traps
59:     mov     \xreg1, #0x33ff
60:     msr     cptr_el2, \xreg1            // Disable coprocessor traps to EL2
61:     msr     hstr_el2, xzr               // Disable coprocessor traps to EL2
62:     mov     \xreg1, #3 << 20
63:     msr     cpacr_el1, \xreg1           // Enable FP/SIMD at EL1
64: 
65:     // Initialize HCR_EL2
66:     mov     \xreg1, #(1 << 31)          // 64bit EL1
67:     msr     hcr_el2, \xreg1
68: 
69:     // SCTLR_EL1 initialization
70:     //
71:     // setting RES1 bits (29,28,23,22,20,11) to 1
72:     // and RES0 bits (31,30,27,21,17,13,10,6) +
73:     // UCI,EE,EOE,WXN,nTWE,nTWI,UCT,DZE,I,UMA,SED,ITD,
74:     // CP15BEN,SA0,SA,C,A,M to 0
75:     mov     \xreg1, #0x0800
76:     movk    \xreg1, #0x30d0, lsl #16
77:     msr     sctlr_el1, \xreg1
78: 
79:     // Return to the EL1_SP1 mode from EL2
80:     mov     \xreg1, #0x3c4
81:     msr     spsr_el2, \xreg1            // EL1_SP0 | D | A | I | F
82:     adr     \xreg1, label1
83:     msr     elr_el2, \xreg1
84:     eret
85: label1:
86: .endm
```

- Line 47: The register `cnthctl_el2` (Counter-timer Hypervisor Control register EL2, see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `CNTHCTL_EL2, Counter-timer Hypervisor Control
register`) is read.
- Line 48: The two lower bits (bit 0 and 1) are set on this value.
In short this means the bits `EL0VCTEN` and `EL0CTEN` are set, which allows code in EL0 and EL1 to access the timer frequency and physical and virtual timer counter registers without access being trapped.
- Line 49: The new value is written to the `cnthctl_el2` register
- Line 50: The register `cntvoff_el2` (Counter-timer Virtual Offset register, see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `CNTVOFF_EL2, Counter-timer Virtual Offset register`) is set to 0.
The term `xzr` points to the virtual zero register, which can be wzr (32 bits) or xzr (64 bits).
This sets the offset of the virtual timer count relative to the physical timer count to 0.
- Line 53: The register `midr_el1` (Main ID Register, see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `MIDR_EL1, Main ID Register`) is read and stored in the first parameter.
This register is read-only and holds information on the chip, such as the manfacturer, the variant, architecture, part number and revision.
- Line 54: The register `mpidr_el1` (Multiprocessor Affinity Register, see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `MPIDR_EL1, Multiprocessor Affinity Register`) is read and stored in the second parameter.
This register contains the affinity levels
- Line 55: We write to register `vpidr_el2` (Virtualization Processor ID Register, see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `VPIDR_EL2, Virtualization Processor ID Register`) the value read from the `midr_el1` register.
This register is used for virtualization, and has a 64 bit value.
- Line 56: We write to register `vmpidr_el2` (Virtualization Processor ID Register, see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `VMPIDR_EL2, Virtualization Multiprocessor ID Register`) the value read from the `mpidr_el1` register.
This register is used for virtualization, and has a 64 bit value.
- Line 59-60: We set the value 0x33FF into the first parameter, and write this to the register `cptr_el2` (Architectural Feature Trap Register (EL2), see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `CPTR_EL2, Architectural Feature Trap Register (EL2)`).
This register defines which traps are set for exception level 2. The value written will set the following bits:
  - TCPAC bit to 0: this will not trap any instructions
  - TAM bit to 0: this will not trap any access to activity monitor registers
  - TTA bit to 0: this will not trap any access to trace registers
  - TSM bit to 1: this will trap SME and SVE instructions
  - TFP to 0: this will not trap SIMD and floating point operations
  - TZ bit to 1: this will trap SVE instructions
- Line 61: The register `hstr_el2` (Hypervisor System Trap Register, see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `HSTR_EL2, Hypervisor System Trap Register`) is set to 0.
This register defines which traps are set for the hypervisor mode for exception level 2. The value written will set all trap bits to 0, disabling the traps for all kinds of instructions.
- Line 62-63: We set the value 0x03 << 20 => 0x00300000 into the first parameter, and write this to the register `cpacr_el1` (Architectural Feature Access Control Register, see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `CPACR_EL1, Architectural Feature Access Control
Register`).
This register controls access to certain functions in EL0 and EL1 mode. The value written will set the following bits:
  - TTA bit to 0: this will not trap any access to system registers
  - SMEN bits to 0: this will trap SME instructions
  - FPEN bits to 1: this will not trap SIMD and floating point instructions
  - ZEN bits to 0: this will trap SVE instructions
- Line 66-67: We set the value 0x01 << 31 => 0x80000000 into the first parameter, and write this to the register `hcr_el2` (Hypervisor Configuration Register, see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `HCR_EL2, Hypervisor Configuration Register`).
This register controls virtualization and trapping for EL2 in this mode. The value written will set the following bits:
  - RW bit to 1: this will set EL1 mode to 64 bit
- Line 75-77: We set the value 0x0800 | 0x30d0 << 16 => 0x30D00800 into the first parameter, and write this to the register `sctlr_el1` (System Control Register (EL1), see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `SCTLR_EL1, System Control Register (EL1)`).
This register controls the system, from a EL1 perspective.
The value is constructed in a strange way, but the first part `mov \xreg1, #0x0800` writes a 16 bit value in the first parameter, and the second `movk \xreg1, #0x30d0, lsl #16` copies a 16 bit value, while shiftting it 16 bits to the left, into the first parameter, while keeping the other bits untouched.
The final result is that 0x0800 is written in the lower 16 bits, and 0x30D0 is written in the higher 16 bits.
The value written will set the following bits:
  - EnIA bit to 0: this feature is not implemented so we set RES0
  - EnIB bit to 0: this feature is not implemented so we set RES0
  - LSMAOE bit to 1: this feature is not implemented so we set RES1
  - nTLSMD bit to 1: this feature is not implemented so we set RES1
  - EnDA bit to 0: this feature is not implemented so we set RES0
  - UCI bit to 0: this will trap a set of instructions in EL0
  - EE bit to 0: this sets data access in EL1 to be little endian
  - EOE bit to 0: this sets data access in EL0 to be little endian
  - SPAN bit to 1: this feature is not implemented so we set RES1
  - EIS bit to 1: this feature is not implemented so we set RES1
  - EISB bit to 0: this feature is not implemented so we set RES0
  - TSCXT bit to 1: this feature is not implemented so we set RES1
  - WXN bit to 0: this will enable writing to memory for EL1 and EL0
  - nTWE bit to 0: this will trap wfe (Wait for event) instructions from EL0 to EL1
  - nTWI bit to 0: this will trap wfi (Wait for interrupt) instructions from EL0 to EL1
  - UCT bit to 0: this will trap access to the CTR_EL0 register from EL0 to EL1
  - DZE bit to 0: this will trap certain instructions from EL0 to EL1
  - EnDB bit to 0: this feature is not implemented so we set RES0
  - I bit to 0: this will flag access to L1 cache as non cacheable
  - EOS bit to 1: this feature is not implemented so we set RES1
  - EnRCTX bit to 0: this feature is not implemented so we set RES0
  - UMA bit to 0: this will trap access to the DAIF register from EL0 to EL1
  - SED bit to 0: this will enable use of the SETEND instruction in EL0
  - ITD bit to 0: this will enable all 32 bit functionality in EL0 mode
  - nAA bit to 0: this feature is not implemented so we set RES0
  - CP15BEN bit to 0: this will trap DMB, DSB and ISB instructions in EL0 to EL1
  - SA0 bit to 0: this will not check for stack pointer alignment in EL0
  - SA bit to 0: this will not check for stack pointer alignment in EL1
  - C bit to 0: this will flag access to L1 cache as non cacheable
  - A bit to 0: this will not check for memory alignment in EL1 / EL0
  - M bit to 0: this will disable MMU translation for EL1 / EL0
  - In short, most access from EL0 level is trapped, and moved up to EL1. EL1 however can perform these operations.
- Line 80-81: We set the value 0x03c4 into the first parameter, and write this to the register `spsr_el2` (Saved Program Status Register (EL2), see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `SPSR_EL2, Saved Program Status Register (EL2)`).
This register will hold the programs state for when an exception moves to EL2 state.
The value written will set the following bits:
  - ALLINT bit to 0: this feature is not implemented so we set RES0
  - SSBS bit to 0: this feature is not implemented so we set RES0
  - BTYPE bits to 0: this feature is not implemented so we set RES0
  - D bit to 1: Debug exception mask is copied
  - A bit to 1: SError exception mask is copied
  - I bit to 1: IRQ interrupt mask is copied
  - F bit to 1: FIQ interrupt mask is copied
  - M[4] bit to 0: execute in 64 bit mode
  - M[3:0] bits to 0100: set EL1t mode -> move to EL1t
- Line 82-83: We get the address of after the last instruction in the macro, which is the address of the next instruction to be executed after the macro is invoked.
This is stored in the first parameter, and then written to the register `elr_el2` (Exception Link Register (EL2), see [ARM architecture registers](pdf/ARM-architecture-registers.pdf), section `ELR_EL2, Exception Link Register (EL2)`).
This register sets the return address for when a EL2 exception was executed.
- Line 84: We return to the exception level set in register `spsr_el2`, which means we move to EL1t.

As said this is all very intricate and detailed, forcing one to dive into all the details of quite some specific ARM registers. You could also simply decide to accept what was explained here, and use the code.

#### SysConfig.h {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_SYSCONFIGH}

We add the system configuration header.
This header can be used to set certain default system parameters, if they are not overriden.

Create the file `code/libraries/baremetal/include/baremetal/SysConfig.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/SysConfig.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : SysConfig.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : System configuration defines
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: /// @file
43: /// System configuration parameters. This file will include MemoryMap.h to set the defaults if not overridden.
44: 
45: /// @brief Number of cores to use (if ARM_ALLOW_MULTI_CORE is defined)
46: #define CORES    4
47: 
48: /// @brief Size of 1 Megabyte
49: #define MEGABYTE 0x100000
50: /// @brief Size of 1 Gigabyte
51: #define GIGABYTE 0x40000000ULL
52: 
53: /// @brief KERNEL_MAX_SIZE is the maximum allowed size of a built kernel image.
54: ///
55: /// If your kernel image contains big data areas it may be required to
56: /// increase this value. The value must be a multiple of 16 KByte.
57: #ifndef KERNEL_MAX_SIZE
58: #define KERNEL_MAX_SIZE (2 * MEGABYTE)
59: #endif
60: 
61: /// @brief Set part to be used by GPU (normally set in config.txt)
62: #ifndef GPU_MEM_SIZE
63: #define GPU_MEM_SIZE (64 * MEGABYTE)
64: #endif
65: 
66: #include "baremetal/MemoryMap.h"
```

For now, this header only defines some parameters:

- Line 46: The number of cores used in this system. For Raspberry Pi 3 and higher, this is always 4.
- Line 49: We define the value for one megabyte (Mb), or 1 << 20
- Line 51: We define the value for one gigabyte (Gb), or 1 << 30
- Line 57-59: Unless overridden by the build, we set the maximum size for the kernel code `KERNEL_MAX_SIZE` to be 2 Mb
- Line 62-64: Unless overridden by the build, we set the GPU memory size `GPU_MEM_SIZE` to be 64 Mb.
This splits up the physical memory between the CPU and the GPU.
Unless we write graphics heavy applications, 64 Mb for the GPU should be fine
- Line 66: We then include `MemoryMap.h`. See the next section

Just for clarity, you will see the GPU (Graphics Processing Unit) also named VPU (Vector Processing Unit) and VC (VideoCore, which is the name Broadcom uses for this part of the SoC).
This is all the same thing.

#### MemoryMap.h {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_MEMORYMAPH}

We add the definition of the memory layout for the system. As we are building a baremetal system, we have much more freedom in this, but we also need to take care that the mapping is well defined and complete.

Create the file `code/libraries/baremetal/include/baremetal/MemoryMap.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryMap.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryMap.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : Memory layout definitions
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: /// @file
43: /// Memory map definitions
44: ///
45: /// Some of the definitions could be pre-defined in either SysConfig.h or by the build, so include SysConfig.h instead of MemoryMap.h directly
46: 
47: /// @brief Default RAM memory size (minimum size as of RPI 3)
48: #define MEM_SIZE (1024 * MEGABYTE)
49: /// @brief Part of RAM mapped to GPU (minimum size is 64Mb)
50: #if !defined(GPU_MEM_SIZE)
51: /// @brief set in config.txt
52: #define GPU_MEM_SIZE (64 * MEGABYTE)
53: #endif
54: /// @brief Part of RAM mapped to ARM (this is the total amount - amount allocated to GPU)
55: #define ARM_MEM_SIZE (MEM_SIZE - GPU_MEM_SIZE)
56: 
57: /// @brief Memory reserved for paging. Paging memory is placed just below the low heap
58: #define PAGE_RESERVE (16 * MEGABYTE)
59: 
60: /// @brief Size of every page
61: #define PAGE_SIZE    0x10000 // 64Kb
62: 
63: /// @brief Maximum size of the kernel space (if not already specified in SysConfig.h)
64: #if !defined(KERNEL_MAX_SIZE)
65: #define KERNEL_MAX_SIZE (2 * MEGABYTE)
66: #endif
67: /// @brief Memory reserved for the stack (this memory is reserved for every core)
68: #define KERNEL_STACK_SIZE       0x20000 // 128 Kb
69: /// @brief Memory reserved for the exception stack (this memory is reserved for every core)
70: #define EXCEPTION_STACK_SIZE    0x8000 // 32 Kb
71: /// @brief Location where the kernel starts. This is also the location where the code starts
72: #define MEM_KERNEL_START        0x80000
73: /// @brief End of kernel space (start + size)
74: #define MEM_KERNEL_END          (MEM_KERNEL_START + KERNEL_MAX_SIZE)
75: /// @brief Top of stack for core 0 (stack grows down)
76: #define MEM_KERNEL_STACK        (MEM_KERNEL_END + KERNEL_STACK_SIZE)
77: /// @brief Top of exception stack for core 0 (stack grows down). Also includes the stacks for cores 1..CORES-1
78: #define MEM_EXCEPTION_STACK     (MEM_KERNEL_STACK + KERNEL_STACK_SIZE * (CORES - 1) + EXCEPTION_STACK_SIZE)
79: /// @brief Top of exception stack for all cores (stack grows down). Also includes the exception stacks for cores 1..CORES-1
80: #define MEM_EXCEPTION_STACK_END (MEM_EXCEPTION_STACK + EXCEPTION_STACK_SIZE * (CORES - 1))
```

This deserves some explanation. The memory map layout is as defined in the next sections

#### Raspberry Pi 3 {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_RASPBERRY_PI_3}

| Base       | Size       | Contents               | Remarks      |
|------------|------------|------------------------|--------------|
| 0x00000000 | 256 bytes  | ARM stub               | Contains spinlock table for cores 1-3 (start address for each core). Discussed later
| 0x00000100 | variable   | ATAGS                  | unused. This used to be the space where the board tags we stored
| 0x00080000 | 0x00200000 | Kernel image           | 2Mb, can be different if KERNEL_MAX_SIZE redefined. Start address is defined by `MEM_KERNEL_START`
|            |            | .init                  | Startup code
|            |            | .fini                  | Shutdown code
|            |            | .text                  | Code
|            |            | .rodata                | Read only data
|            |            | .init_array            | Static construction functions
|            |            | .data                  | Read/write data
|            |            | .bss                   | Uninitialized data
| 0x00280000 |            | End of kernel image    | Defined by `MEM_KERNEL_END`
| 0x00280000 | 0x00020000 | Kernel stack core 0    | Stack for core 0, growing down, size is defined by `KERNEL_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_STACK = MEM_KERNEL_END + 1 * KERNEL_STACK_SIZE`
| 0x002A0000 | 0x00020000 | Kernel stack core 1    | Stack for core 1, growing down, size is defined by `KERNEL_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 2 * KERNEL_STACK_SIZE`
| 0x002C0000 | 0x00020000 | Kernel stack core 2    | Stack for core 2, growing down, size is defined by `KERNEL_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 3 * KERNEL_STACK_SIZE`
| 0x002E0000 | 0x00020000 | Kernel stack core 3    | Stack for core 3, growing down, size is defined by `KERNEL_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE`
| 0x00308000 | 0x00008000 | Exception stack core 0 | Stack for handling exceptions for core 0, growing down, size if defined by `EXCEPTION_STACK_SIZE`. Top of stack address is defined by `MEM_EXCEPTION_STACK = MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE + 1 * EXCEPTION_STACK_SIZE`
| 0x00310000 | 0x00008000 | Exception stack core 1 | Stack for handling exceptions for core 1, growing down, size if defined by `EXCEPTION_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE + 2 * EXCEPTION_STACK_SIZE`
| 0x00318000 | 0x00008000 | Exception stack core 2 | Stack for handling exceptions for core 2, growing down, size if defined by `EXCEPTION_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE + 3 * EXCEPTION_STACK_SIZE`
| 0x00320000 | 0x00008000 | Exception stack core 3 | Stack for handling exceptions for core 3, growing down, size if defined by `EXCEPTION_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE + 4 * EXCEPTION_STACK_SIZE`
| 0x00328000 |            | End of exception stack | Defined by `MEM_EXCEPTION_STACK_END`
| ...        |            |                        |
| 0x00500000 | 0x00100000 | Coherent region        | For property mailbox, VCHIQ. Discussed later
| 0x00600000 | variable   | Heap allocator         | Range used for heap allocation (using malloc()). Discussed later
| ????????   | 0x01000000 | Page allocator         | Range for page allocation (using palloc()). Size is defined as `PAGE_RESERVE`. Discussed later
| 0x3C000000 | 0x04000000 | GPU memory             | Depending on split location (64 Mb GPU memory in this example) (mapped to different range by memory management unit). This address is defined as `ARM_MEM_SIZE`
| 0x3F000000 | 0x01000000 | Peripherals            | Address range mapped to I/O for peripherals (GPIO, UART, etc.)
| 0x40000000 |            | End of physical RAM    | Raspberry Pi 3 always has 1Gb memory
| 0x40000000 |            | Local peripherals      | ARM local peripherals are addressed here
| ...        |            |                        |

#### Raspberry Pi 4 {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_STARTUP_CODE_RASPBERRY_PI_4}

| Base        | Size       | Contents                   | Remarks      |
|-------------|------------|----------------------------|--------------|
| 0x000000000 | 0x00000100 | ARM stub                   | Contains spinlock table for cores 1-3 (start address for each core). Discussed later
| 0x000000100 | variable   | ATAGS                      | unused
| 0x00006F000 | 0x00001000 | EL3 stack                  | Stack for exception level 3. Discussed later
| 0x000070000 | 0x00000800 | Exception vector table EL3 | Contains lookup table for exception level 3 handling. Discussed later
| 0x000080000 | 0x00200000 | Kernel image               | 2Mb, can be larger if KERNEL_MAX_SIZE redefined. Start address is defined by `MEM_KERNEL_START`
|             |            | .init                      | Startup code
|             |            | .fini                      | Shutdown code
|             |            | .text                      | Code
|             |            | .rodata                    | Read only data
|             |            | .init_array                | Static construction functions
|             |            | .data                      | Read/write data
|             |            | .bss                       | Uninitialized data
| 0x000280000 |            | End of kernel image        | Defined by `MEM_KERNEL_END`
| 0x000280000 | 0x00020000 | Kernel stack core 0        | Stack for core 0, growing down, size is defined by `KERNEL_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_STACK = MEM_KERNEL_END + 1 * KERNEL_STACK_SIZE`
| 0x0002A0000 | 0x00020000 | Kernel stack core 1        | Stack for core 1, growing down, size is defined by `KERNEL_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 2 * KERNEL_STACK_SIZE`
| 0x0002C0000 | 0x00020000 | Kernel stack core 2        | Stack for core 2, growing down, size is defined by `KERNEL_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 3 * KERNEL_STACK_SIZE`
| 0x0002E0000 | 0x00020000 | Kernel stack core 3        | Stack for core 3, growing down, size is defined by `KERNEL_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE`
| 0x000308000 | 0x00008000 | Exception stack core 0     | Stack for handling exceptions for core 0, growing down, size if defined by `EXCEPTION_STACK_SIZE`. Top of stack address is defined by `MEM_EXCEPTION_STACK = MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE + 1 * EXCEPTION_STACK_SIZE`
| 0x000310000 | 0x00008000 | Exception stack core 1     | Stack for handling exceptions for core 1, growing down, size if defined by `EXCEPTION_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE + 2 * EXCEPTION_STACK_SIZE`
| 0x000318000 | 0x00008000 | Exception stack core 2     | Stack for handling exceptions for core 2, growing down, size if defined by `EXCEPTION_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE + 3 * EXCEPTION_STACK_SIZE`
| 0x000320000 | 0x00008000 | Exception stack core 3     | Stack for handling exceptions for core 3, growing down, size if defined by `EXCEPTION_STACK_SIZE`. Top of stack address is defined by `MEM_KERNEL_END + 4 * KERNEL_STACK_SIZE + 4 * EXCEPTION_STACK_SIZE`
| 0x000328000 |            | End of exception stack     | Defined by `MEM_EXCEPTION_STACK_END`
| ...         |            |                            |
| 0x000500000 | 0x00400000 | Coherent region            | For property mailbox, VCHIQ, xHCI. Discussed later
| 0x000900000 | variable   | Heap allocator             | Range used for heap allocation (using malloc()). Discussed later
| ????????    | 0x01000000 | Page allocator             | Range for page allocation (using palloc()). Size if defined as `PAGE_RESERVE`. Discussed later
| 0x03C000000 | 0x04000000 | GPU memory                 | Depending on split location (64 Mb GPU memory in this example) (mapped to different range by memory management unit). This address is defined as `ARM_MEM_SIZE`
| 0x040000000 | variable   | high heap allocator        | Memory above 1Gb can be used for high heap, but is unused above 0xC0000000 (3 Gb). Discussed later
| 0x080000000 |            | End of physical RAM        | Raspberry Pi 4 can have 2, 4 or 8 Gb of RAM. This is for 2 Gb version
| 0x0FC000000 | 0x04000000 | Peripherals                | Notice the different peripheral mapping address from Raspberry Pi 3
| 0x100000000 |            | End of physical RAM        | For 4 Gb version
| 0x200000000 |            | End of physical RAM        | For 8 Gb version
| ...         |            |                            |
| 0x600000000 | 64 MByte   | xHCI controller            | Mapped from 0x600000000

In other words, the code space is between `MEM_KERNEL_START` and `MEM_KERNEL_END`

Then follows the stack space for cores 0, 1, 2, and 3 (separate for every core)
Then end (top) of the stack for core 0 is `MEM_KERNEL_STACK`.
This is the value used in line 102 of `Startup.S`

After this the stack space for exception and interrupt handling is allocated for cores 0, 1, 2, and 3 (separate for every core).
The end (top) of the exception stack for core 0 is `MEM_EXCEPTION_STACK`.
This is the value used in line 96 of `Startup.S`

Finally the end of the exception stacks is `MEM_EXCEPTION_STACK_END`.
As you can see in the tables, there is also heap space, page space, etc.
We'll get around to that later.

Notice that the peripheral device registers are partly mapped over the actual memory, decreasing usable memory slightly.

### CMake file refresh {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_CMAKE_FILE_REFRESH}

As we've added some files to the baremetal library, and removed Dummy.cpp, we will need to reconfigure the project.
As we are using `GLOB`, we can simply reconfigure without the need for changes to the CMake file.
We need to do this every time we add or remove files, but will not be repeating this.

### Update application {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_APPLICATION}

The last thing we need to do is update the application code to actually make use of the functionality we just created.
So we change the code in main.cpp:

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/UART1.h"
3: 
4: /// @file
5: /// Demo application main code
6: 
7: /// <summary>
8: /// Demo application main code
9: /// </summary>
10: /// <returns>For now always 0</returns>
11: int main()
12: {
13:     baremetal::UART1 uart;
14:     uart.Initialize(115200);
15: 
16:     uart.WriteString("Hello World!\n");
17:     for (int i = 0; i < 1000000; ++i)
18:         NOP();
19:     return 0;
20: }
```

In the main() function, we first create an instance of the UART, then initialize it with a call to `Initialize()` using a baud rate of 115200, and finally we write the string "Hello World!\n" to the console.
Notice the `\n` character, and remember that we will write the sequency `\r\n` instead of the simple line feed.

After we write the text to UART1, we wait a while before returning, to give the UART1 some time to actually send the data. Otherwise you may end up with half the characters on a real device.

### Update CMake file for application {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_UPDATE_CMAKE_FILE_FOR_APPLICATION}

As we have now added `Startup.S` to the baremetal library, we can remove `start.S` from the application.
We also need to update the CMake file as we now depend on the baremetal library.

```cmake
File: code/applications/demo/CMakeLists.txt
...
27: set(PROJECT_DEPENDENCIES
28:     baremetal
29:     )
...
```

### Configure and build {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_CONFIGURE_AND_BUILD}

We are now able to configure the project again, and build it.

The output for the configure step should be similar to:

```text
1> CMake generation started for configuration: 'BareMetal-RPI3-Debug'.
1> Command line: "C:\WINDOWS\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="D:\Projects\baremetal\output\install\BareMetal-RPI3-Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH="D:\Projects\baremetal\baremetal.toolchain" -DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DBUILD_TUTORIALS=OFF -DCMAKE_MAKE_PROGRAM="C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" "D:\Projects\baremetal" 2>&1"
1> Working directory: D:\Projects\baremetal\cmake-BareMetal-RPI3-Debug
1> [CMake] -- CMake 3.20.21032501-MSVC_2
1> [CMake] -- Building for Raspberry Pi 3
1> [CMake] -- 
1> [CMake] ** Setting up project **
1> [CMake] --
1> [CMake] -- 
1> [CMake] ##################################################################################
1> [CMake] -- 
1> [CMake] ** Setting up toolchain **
1> [CMake] --
1> [CMake] -- TOOLCHAIN_ROOT           D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf
1> [CMake] -- Processor                aarch64
1> [CMake] -- Platform tuple           aarch64-none-elf
1> [CMake] -- Assembler                D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
1> [CMake] -- C compiler               D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
1> [CMake] -- C++ compiler             D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-g++.exe
1> [CMake] -- Archiver                 D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ar.exe
1> [CMake] -- Linker                   D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ld.exe
1> [CMake] -- ObjCopy                  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-objcopy.exe
1> [CMake] -- Std include path         D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1/include
1> [CMake] -- CMAKE_EXE_LINKER_FLAGS=   -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
1> [CMake] -- C++ compiler version:    13.3.1
1> [CMake] -- C compiler version:      13.3.1
1> [CMake] -- C++ supported standard:  17
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/applications
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/applications/demo
1> [CMake] 
1> [CMake] ** Setting up demo **
1> [CMake] 
1> [CMake] -- Package                           :  demo
1> [CMake] -- Package description               :  Demo application
1> [CMake] -- Defines C - public                : 
1> [CMake] -- Defines C - private               : 
1> [CMake] -- Defines C++ - public              : 
1> [CMake] -- Defines C++ - private             :  PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3 _DEBUG
1> [CMake] -- Defines ASM - private             :  PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3
1> [CMake] -- Compiler options C - public       : 
1> [CMake] -- Compiler options C - private      : 
1> [CMake] -- Compiler options C++ - public     : 
1> [CMake] -- Compiler options C++ - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter
1> [CMake] -- Compiler options ASM - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2
1> [CMake] -- Include dirs - public             : 
1> [CMake] -- Include dirs - private            : 
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      :  baremetal
1> [CMake] -- Link libs                         :  baremetal
1> [CMake] -- Source files                      :  D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp
1> [CMake] -- Include files - public            : 
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] -- Properties for demo
1> [CMake] -- Target type                       :  EXECUTABLE
1> [CMake] -- Target defines                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3 _DEBUG> $<$<COMPILE_LANGUAGE:ASM>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target options                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target include dirs public        :  INCLUDES-NOTFOUND
1> [CMake] -- Target include dirs private       :  INCLUDES-NOTFOUND
1> [CMake] -- Target link libraries             :  -Wl,--start-group baremetal -Wl,--end-group
1> [CMake] -- Target link options               :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target exported defines           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported options           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported include dirs      :  INCLUDE_DIRS_EXPORTS-NOTFOUND
1> [CMake] -- Target exported link libraries    :  -Wl,--start-group baremetal -Wl,--end-group
1> [CMake] -- Target imported dependencies      : 
1> [CMake] -- Target imported link libraries    : 
1> [CMake] -- Target link dependencies          :  LINK_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target manual dependencies        :  EXPLICIT_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target static library location    :  D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/lib
1> [CMake] -- Target dynamic library location   :  LIBRARY_LOCATION-NOTFOUND
1> [CMake] -- Target binary location            :  D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/bin
1> [CMake] -- Target link flags                 :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target version                    :  TARGET_VERSION-NOTFOUND
1> [CMake] -- Target so-version                 :  TARGET_SOVERSION-NOTFOUND
1> [CMake] -- Target output name                :  demo.elf
1> [CMake] -- Target C++ standard               :  17
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/applications/demo
1> [CMake] 
1> [CMake] ** Setting up demo-image **
1> [CMake] 
1> [CMake] -- create_image demo-image kernel8.img demo
1> [CMake] -- TARGET_NAME demo.elf
1> [CMake] -- generate D:/Projects/RaspberryPi/baremetal.github/deploy/Debug/demo-image/kernel8.img from D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/bin/demo
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/libraries
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal
1> [CMake] -- Package                           :  baremetal
1> [CMake] -- Package description               :  Bare metal library
1> [CMake] -- Defines C - public                : 
1> [CMake] -- Defines C - private               : 
1> [CMake] -- Defines C++ - public              : 
1> [CMake] -- Defines C++ - private             :  PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3 _DEBUG
1> [CMake] -- Defines ASM - private             :  PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3
1> [CMake] -- Compiler options C - public       : 
1> [CMake] -- Compiler options C - private      : 
1> [CMake] -- Compiler options C++ - public     : 
1> [CMake] -- Compiler options C++ - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter
1> [CMake] -- Compiler options ASM - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2
1> [CMake] -- Include dirs - public             :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Include dirs - private            : 
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      : 
1> [CMake] -- Link libs                         : 
1> [CMake] -- Source files                      :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/src/MemoryAccess.cpp D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/src/Startup.S D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/src/UART1.cpp
1> [CMake] -- Include files - public            : 
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] -- Properties for baremetal
1> [CMake] -- Target type                       :  STATIC_LIBRARY
1> [CMake] -- Target defines                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3 _DEBUG> $<$<COMPILE_LANGUAGE:ASM>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target options                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target include dirs public        :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target include dirs private       :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target link libraries             :  LIBRARIES-NOTFOUND
1> [CMake] -- Target link options               :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target exported defines           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported options           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported include dirs      :  D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target exported link libraries    :  LIBRARIES_EXPORTS-NOTFOUND
1> [CMake] -- Target imported dependencies      : 
1> [CMake] -- Target imported link libraries    : 
1> [CMake] -- Target link dependencies          :  LINK_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target manual dependencies        :  EXPLICIT_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target static library location    :  D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/lib
1> [CMake] -- Target dynamic library location   :  LIBRARY_LOCATION-NOTFOUND
1> [CMake] -- Target binary location            :  RUNTIME_LOCATION-NOTFOUND
1> [CMake] -- Target link flags                 :  -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target version                    :  TARGET_VERSION-NOTFOUND
1> [CMake] -- Target so-version                 :  TARGET_SOVERSION-NOTFOUND
1> [CMake] -- Target output name                :  baremetal
1> [CMake] -- Target C++ standard               :  17
1> [CMake] -- Configuring done (0.1s)
1> [CMake] -- Generating done (0.1s)
1> [CMake] -- Build files have been written to: D:/Projects/RaspberryPi/baremetal.github/cmake-Baremetal-RPI3-Debug
1> Extracted CMake variables.
1> Extracted source files and headers.
1> Extracted code model.
1> Extracted toolchain configurations.
1> Extracted includes paths.
1> CMake generation finished.
```

We can then build:

```text
>------ Build All started: Project: baremetal, Configuration: BareMetal-RPI3-Debug ------
  [1/7] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -ID:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\Startup.S.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj -c D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/src/Startup.S
  [2/7] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -ID:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include -g -std=gnu++17 -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\MemoryAccess.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj -c D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/src/MemoryAccess.cpp
  [3/7] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -ID:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include -g -std=gnu++17 -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp
  [4/7] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -ID:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/include -g -std=gnu++17 -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\UART1.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj -c D:/Projects/RaspberryPi/baremetal.github/code/libraries/baremetal/src/UART1.cpp
  [5/7] C:\WINDOWS\system32\cmd.exe /C "cd . && "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E rm -f D:\Projects\baremetal\output\RPI3\Debug\lib\libbaremetal.a && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ar.exe qc D:\Projects\baremetal\output\RPI3\Debug\lib\libbaremetal.a  code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ranlib.exe D:\Projects\baremetal\output\RPI3\Debug\lib\libbaremetal.a && cd ."
  [6/7] C:\WINDOWS\system32\cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/RaspberryPi/baremetal.github/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -o D:\Projects\baremetal\output\RPI3\Debug\bin\demo.elf  -Wl,--start-group  D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/lib/libbaremetal.a  -Wl,--end-group && cd ."
  [7/7] C:\WINDOWS\system32\cmd.exe /C "cd /D D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug\code\applications\demo && "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E make_directory D:/Projects/RaspberryPi/baremetal.github/deploy/Debug/demo-image && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/RaspberryPi/baremetal.github/output/RPI3/Debug/bin/demo.elf -O binary D:/Projects/RaspberryPi/baremetal.github/deploy/Debug/demo-image/kernel8.img"

Build All succeeded.
```

- The baremetal library's Startup.S file is compiled (step 1)
- The demo application's main.cpp file is compiled (step 2).
- The baremetal library's MemoryAccess.cpp file is compiled (step 3).
- The baremetal library's UART1.cpp file is compiled (step 4).
- Then the baremetal library is removed and re-created (step 5).
- The demo application is linked, using the baremetal library (step 6)
- And finally the image is created (step 7).

The only difference is that now `Startup.S` is built in baremetal, instead of `start.S` in the application, and the `Dummy.cpp` file is now replaced with `MemoryAccess.cpp` and `UART1.cpp`.

### Running the application - Step 2 {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_LIBRARY_CODE___STEP_2_RUNNING_THE_APPLICATION___STEP_2}

Start QEMU, and start debugging as before.

After the WriteString() method is called, you will see output in the command window.

```text
Hello World!
c:\Program Files\qemu\qemu-system-aarch64.exe: QEMU: Terminated via GDBstub
```

You will also notice that when you end the application, it simply hangs.
We will get to restarting and halting the system later.

Unless something worthwhile can be mentioned, we will not describe building and running / debugging in detail any longer.

## Tutorial code {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_TUTORIAL_CODE}

As was already discussed in [Tutorial code setup](#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_TUTORIAL_CODE_SETUP), we will use the infrastructure that is already there.
So, for this chapter, we will create a copy of the `code` tree, and make some small changes to prevent conflicts.

### Main application CMake file {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_TUTORIAL_CODE_MAIN_APPLICATION_CMAKE_FILE}

The main application's project name will be named after the tutorial, to not conflict with the one in the main code tree.
Also the description is changed:

```cmake
File: code/applications/demo/CMakeLists.txt
1: project(demo
2:     DESCRIPTION "Demo application"
3:     LANGUAGES CXX ASM)
...
```

```cmake
File: tutorial/05-console-uart1/code/applications/demo/CMakeLists.txt
1: project(05-console-uart1
2:     DESCRIPTION "Tutorial 06 Console UART1 application"
3:     LANGUAGES CXX ASM)
...
```

### Baremetal library CMake file {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_TUTORIAL_CODE_BAREMETAL_LIBRARY_CMAKE_FILE}

In the same way, we need to make sure the baremetal library project name is not conflicting, so we add `-06` to the name.

```cmake
File: code/libraries/baremetal/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3:
4: project(baremetal
5: 	DESCRIPTION "Bare metal library"
6: 	LANGUAGES CXX ASM)
...
```

```cmake
File: tutorial/06-console-uart1/code/libraries/baremetal/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3:
4: project(baremetal-06
5:     DESCRIPTION "Bare metal library"
6:     LANGUAGES CXX ASM)
...
```

This also means that the main application project needs to depend on the new version of `baremetal`:

```cmake
File: code/applications/demo/CMakeLists.txt
...
25: set(PROJECT_DEPENDENCIES
26:     baremetal
27:     )
...
```

```cmake
File: tutorial/05-console-uart1/code/applications/demo/CMakeLists.txt
...
25: set(PROJECT_DEPENDENCIES
26:     baremetal-06
27:     )
...
```

### Tutorial results {#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_TUTORIAL_CODE_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-06.a`
- an application `output/Debug/bin/06-console-uart1.elf`
- an image in `deploy/Debug/06-console-uart1-image`

In every following tutorial, the changes described here will be similar.

Next: [07-improving-startup-static-initialization](07-improving-startup-static-initialization.md)
