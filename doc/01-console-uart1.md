# First application - using the console - UART1

Contents:
- [Creating the baremetal library structure](##Creating-the-baremetal-library-structure)
- [Some groundwork](##Some-groundwork)
  - [CMake function display_list](###CMake-function-display_list)
  - [CMake function show_target_properties](###CMake-function-show_target_properties)
  - [Common compiler directives](###Common-compiler-directives)
  - [Update default variables in CMakeSettings.json](###Update-default-variables-in-CMakeSettings.json)
  - [Update demo CMake file](###Update-demo-CMake-file)
  - [Update demo-image CMake file](###Updatedemo-image-CMake-file) 

Now that we have set up our project structure, integrated with Visual Studio, and are able to configure, build, run and debug our projects, let's start some actual development.

This will again be a length story, as we will have to lay quite some groundwork.

One of the simplest things to start with is the serial console.
This is also practical, as using the serial console enables us to write output from the application, and also get input.

There are two serial consoles possible, UART0 and UART1, which can be used in parallel, however it is most common to use one of the two, as they normally use the same GPIO pins (14 and 15).
See also [here](setting-up-for-development.md###Attaching-a-serial-console). For this application, we will use UART1, which is the easiest to set up. It has less functionality, but for a simple serial console both are equally suitable.

## Creating the baremetal library structure

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
- Underneath code/libraries/baremetal/include we will create another directory named baremetal again. The reason for this will be explained shortly.
- We will create a new `CMakeLists.txt` file in `code/libraries/baremetal` with the following contents:

```cmake
File: code/libraries/baremetal/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3: 
```

Note that all this can be done from within Visual Studio, of if desired directly on the command line, whatever you prefer.

The file `code/libraries/CMakeLists.txt` will be extended to include our new folder `code/libraries/baremetal`:

```cmake
File: code/libraries/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3: 
4: add_subdirectory(baremetal)
```

The resulting structure will look like this:

<img src="images/TreeViewBaremetalLibrary.png" alt="Tree view" width="600"/>

The reason for adding an extra directory with the same name as the library underneath `code/libraries/baremetal/include` is the following:

- It is common practice to export the directory containing the header files to applications, so they can include the header files.
- If we would have two libraries, let's say `lib-one` and `lib-two`, which both contain a header file `inc.h`, and we export `lib-one/include` as well as `lib-two/include` as directories, if the the application wanted to include `inc.h`:

```cpp
#include "inc.h"
```

- Which one would it include? The answer is, it's undefined.
- By adding an extra directory underneath, so `lib-one/include/lib-one`, and `lib-two/include/lib-two`, the application could include either:

```cpp
#include "lib-one/inc.h"
#include "lib-two/inc.h"
```

The name of the subdirectory does not matter much, but clearly it is more readable in code to use the name of the library.

## Some groundwork

In order for CMake projects to work smoothly, it is convenient to do some groundwork for them.

### CMake function display_list

We will add a function to the `cmake/functions.cmake` file:

```cmake
File: cmake/functions.cmake
9: # Converts a CMake list to a CMake string and displays this in a status message along with the text specified.
10: function(display_list text)
11:     set(list_str "")
12:     foreach(item ${ARGN})
13:         string(CONCAT list_str "${list_str} ${item}")
14:     endforeach()
15:     message(STATUS ${text} ${list_str})
16: endfunction()
```

This function prints a text followed by a list of values, separated by spaces.
This can be used for single values, but also for a list of compiler definitions.

### CMake function show_target_properties

We will add a another function to the `cmake/functions.cmake` file:

```cmake
File: cmake/functions.cmake
18: function(show_target_properties target)
19:     if (CMAKE_VERBOSE_MAKEFILE)
20:         message(STATUS "")
21:         message(STATUS "Properties for ${target}")
22: 
23:         get_target_property(TARGET_TYPE ${target} TYPE)
24:         display_list("Target type                       : " ${TARGET_TYPE})
25:         if (NOT TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
26:             get_target_property(DEFINES ${target} COMPILE_DEFINITIONS)
27:             display_list("Target defines                    : " ${DEFINES})
28:             get_target_property(OPTIONS ${target} COMPILE_OPTIONS)
29:             display_list("Target options                    : " ${OPTIONS})
30:         endif ()
31: 
32:         get_target_property(INCLUDES ${target} INTERFACE_INCLUDE_DIRECTORIES)
33:         display_list("Target include dirs public        : " ${INCLUDES})
34: 
35:         if (NOT TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
36:             get_target_property(INCLUDES ${target} INCLUDE_DIRECTORIES)
37:             display_list("Target include dirs private       : " ${INCLUDES})
38: 
39:             get_target_property(LIBRARIES ${target} LINK_LIBRARIES)
40:             display_list("Target link libraries             : " ${LIBRARIES})
41: 
42:             get_target_property(LINK_OPTIONS ${target} LINK_FLAGS)
43:             display_list("Target link options               : " ${LINK_OPTIONS})
44:         endif ()
45: 
46:         get_target_property(DEFINES_EXPORTS ${target} INTERFACE_COMPILE_DEFINITIONS)
47:         display_list("Target exported defines           : " ${DEFINES_EXPORTS})
48: 
49:         get_target_property(OPTIONS_EXPORTS ${target} INTERFACE_COMPILE_OPTIONS)
50:         display_list("Target exported options           : " ${OPTIONS_EXPORTS})
51: 
52:         get_target_property(INCLUDE_DIRS_EXPORTS ${target} INTERFACE_INCLUDE_DIRECTORIES)
53:         display_list("Target exported include dirs      : " ${INCLUDE_DIRS_EXPORTS})
54: 
55:         get_target_property(LIBRARIES_EXPORTS ${target} INTERFACE_LINK_LIBRARIES)
56:         display_list("Target exported link libraries    : " ${LIBRARIES_EXPORTS})
57: 
58:         get_test_property(IMPORT_DEPENDENCIES ${target} IMPORTED_LINK_DEPENDENT_LIBRARIES)
59:         display_list("Target imported dependencies      : " ${IMPORT_DEPENDENCIES})
60: 
61:         get_test_property(IMPORT_LIBRARIES ${target} IMPORTED_LINK_INTERFACE_LIBRARIES)
62:         display_list("Target imported link libraries    : " ${IMPORT_LIBRARIES})
63: 
64:         if (NOT TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
65:             get_target_property(LINK_DEPENDENCIES ${target} LINK_DEPENDS)
66:             display_list("Target link dependencies          : " ${LINK_DEPENDENCIES})
67: 
68:             get_target_property(EXPLICIT_DEPENDENCIES ${target} MANUALLY_ADDED_DEPENDENCIES)
69:             display_list("Target manual dependencies        : " ${EXPLICIT_DEPENDENCIES})
70:         endif ()
71: 
72:         if (NOT TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
73:             get_target_property(ARCHIVE_LOCATION ${target} ARCHIVE_OUTPUT_DIRECTORY)
74:             display_list("Target static library location    : " ${ARCHIVE_LOCATION})
75: 
76:             get_target_property(LIBRARY_LOCATION ${target} LIBRARY_OUTPUT_DIRECTORY)
77:             display_list("Target dynamic library location   : " ${LIBRARY_LOCATION})
78: 
79:             get_target_property(RUNTIME_LOCATION ${target} RUNTIME_OUTPUT_DIRECTORY)
80:             display_list("Target binary location            : " ${RUNTIME_LOCATION})
81: 
82:             get_target_property(TARGET_LINK_FLAGS ${target} LINK_FLAGS)
83:             display_list("Target link flags                 : " ${TARGET_LINK_FLAGS})
84: 
85:             get_target_property(TARGET_VERSION ${target} VERSION)
86:             display_list("Target version                    : " ${TARGET_VERSION})
87: 
88:             get_target_property(TARGET_SOVERSION ${target} SOVERSION)
89:             display_list("Target so-version                 : " ${TARGET_SOVERSION})
90: 
91:             get_target_property(TARGET_OUTPUT_NAME ${target} OUTPUT_NAME)
92:             display_list("Target output name                : " ${TARGET_OUTPUT_NAME})
93:         endif ()
94:     endif()
95: endfunction()
```

Without going into too much detail, this function prints for a specific target:

- line 23-24: the type of target (executable, library, interface library)
- line 26-27: compiler definitions
- line 28-29: compiler options
- line 32-33: interface include directories
- line 36-37: include directories
- line 39-40: standard libraries to be linked to
- line 42-43: linker options
- line 46-47: interface library compiler definitions
- line 49-50: interface library compiler options
- line 52-53: interface library include directories
- line 55-56: interface library standard libraries to be linked to
- line 58-59: imported dependencies
- line 61-62: imported libraries to be linked to
- line 65-69: dependencies
- line 73-80: output directories for static libraries, shared libraries, and executables
- line 82-83: linker options
- line 85-86: the target version
- line 88-89: the target so version in case of a shared library
- line 91-92: the target output name

### Common compiler directives

As most compiler definitions and compiler options will have some commonality, it is convenient to define these common parts in variables, and use them in our project configuration.

So we will update the main CMake file to set these variables. We will revisit this part a number of times.

Just before `add_subdirectory(code)` in the main CMake file, insert the following:

#### Common definitions and options

```cmake
File: CMakeLists.txt
31: if (BAREMETAL_RPI_TARGET EQUAL 3)
32:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
33:     set(BAREMETAL_TARGET_KERNEL kernel8)
34: elseif (BAREMETAL_RPI_TARGET EQUAL 4)
35:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a72 -mlittle-endian -mcmodel=small)
36:     set(BAREMETAL_TARGET_KERNEL kernel8-rpi4)
37: else()
38:     message(FATAL_ERROR "Incorrect RPI target ${BAREMETAL_RPI_TARGET} specified, must be 3 or 4")
39: endif()
40: set(BAREMETAL_LOAD_ADDRESS 0x80000)
41: 
42: set(DEFINES_C 
43:     PLATFORM_BAREMETAL
44:     RPI_TARGET=${BAREMETAL_RPI_TARGET}
45:     )
46: set(DEFINES_C_DEBUG _DEBUG)
47: set(DEFINES_C_RELEASE NDEBUG)
48: set(DEFINES_C_MINSIZEREL NDEBUG)
49: set(DEFINES_C_RELWITHDEBINFO NDEBUG)
50: set(DEFINES_ASM
51:     PLATFORM_BAREMETAL
52:     RPI_TARGET=${BAREMETAL_RPI_TARGET}
53:     )
54: 
55: set(FLAGS_C
56:     ${BAREMETAL_ARCH_CPU_OPTIONS}
57:     -Wall 
58:     -Wextra
59:     -Werror
60:     -Wno-parentheses
61:     -ffreestanding
62:     -fsigned-char
63:     -nostartfiles 
64:     -std=gnu99 
65:     -mno-outline-atomics
66:     -nostdinc 
67:     -nostdlib 
68: )
69: # -g is added by CMake
70: set(FLAGS_C_DEBUG -O0 -Wno-unused-variable -Wno-unused-parameter)
71: # -O3 is added by CMake
72: set(FLAGS_C_RELEASE -D__USE_STRING_INLINES)
73: # -Os is added by CMake
74: set(FLAGS_C_MINSIZEREL -O3)
75: # -O2 -g is added by CMake
76: set(FLAGS_C_RELWITHDEBINFO )
77: 
78: set(FLAGS_CXX
79:     ${BAREMETAL_ARCH_CPU_OPTIONS}
80:     -Wall
81:     -Wextra
82:     -Werror
83:     -Wno-missing-field-initializers
84:     -Wno-unused-value
85:     -Wno-aligned-new
86:     -ffreestanding 
87:     -fsigned-char 
88:     -nostartfiles
89:     -mno-outline-atomics
90:     -nostdinc
91:     -nostdlib
92:     -nostdinc++
93:     -fno-exceptions
94:     -fno-rtti
95:     )
96: 
97: # -g is added by CMake
98: set(FLAGS_CXX_DEBUG -O0 -Wno-unused-variable -Wno-unused-parameter)
99: # -O3 is added by CMake
100: set(FLAGS_CXX_RELEASE -D__USE_STRING_INLINES)
101: # -Os is added by CMake
102: set(FLAGS_CXX_MINSIZEREL -O3)
103: # -O2 -g is added by CMake
104: set(FLAGS_CXX_RELWITHDEBINFO )
105: 
106: set(FLAGS_ASM ${BAREMETAL_ARCH_CPU_OPTIONS})
107: set(FLAGS_ASM_DEBUG -O2)
108: set(FLAGS_ASM_RELEASE -O2)
109: set(FLAGS_ASM_MINSIZEREL -O2)
110: set(FLAGS_ASM_RELWITHDEBINFO -O2)
111: 
112: set(LINK_FLAGS
113:     -Wl,--section-start=.init=${BAREMETAL_LOAD_ADDRESS}
114:     -T ${CMAKE_SOURCE_DIR}/baremetal.ld
115:     -nostdlib
116:     -nostartfiles 
117:     )
118: set(LINK_FLAGS_DEBUG )
119: set(LINK_FLAGS_RELEASE )
120: set(LINK_FLAGS_MINSIZEREL )
121: set(LINK_FLAGS_RELWITHDEBINFO )
122: 
123: list(APPEND LINK_LIBRARIES )
124: list(APPEND LINK_DIRECTORIES )
125: 
126: set(SUPPORTED_CPP_STANDARD 17)
127: 
128: message(STATUS "C++ compiler version:    ${CMAKE_CXX_COMPILER_VERSION}")
129: message(STATUS "C compiler version:      ${CMAKE_C_COMPILER_VERSION}")
130: message(STATUS "C++ supported standard:  ${SUPPORTED_CPP_STANDARD}")
131: 
132: set(COMPILE_DEFINITIONS_C_DEBUG ${DEFINES_C} ${DEFINES_C_DEBUG})
133: set(COMPILE_DEFINITIONS_C_RELEASE ${DEFINES_C} ${DEFINES_C_RELEASE})
134: set(COMPILE_DEFINITIONS_C_MINSIZEREL ${DEFINES_C} ${DEFINES_C_MINSIZEREL})
135: set(COMPILE_DEFINITIONS_C_RELWITHDEBINFO ${DEFINES_C} ${DEFINES_C_RELWITHDEBINFO})
136: 
137: set(COMPILE_DEFINITIONS_ASM_DEBUG ${DEFINES_ASM} ${DEFINES_ASM_DEBUG})
138: set(COMPILE_DEFINITIONS_ASM_RELEASE ${DEFINES_ASM} ${DEFINES_ASM_RELEASE})
139: set(COMPILE_DEFINITIONS_ASM_MINSIZEREL ${DEFINES_ASM} ${DEFINES_ASM_MINSIZEREL})
140: set(COMPILE_DEFINITIONS_ASM_RELWITHDEBINFO ${DEFINES_ASM} ${DEFINES_ASM_RELWITHDEBINFO})
141: 
142: set(COMPILE_OPTIONS_C_DEBUG ${FLAGS_C} ${FLAGS_C_DEBUG})
143: set(COMPILE_OPTIONS_C_RELEASE ${FLAGS_C} ${FLAGS_C_RELEASE})
144: set(COMPILE_OPTIONS_C_MINSIZEREL ${FLAGS_C} ${FLAGS_C_MINSIZEREL})
145: set(COMPILE_OPTIONS_C_RELWITHDEBINFO ${FLAGS_C} ${FLAGS_C_RELWITHDEBINFO})
146: 
147: set(COMPILE_OPTIONS_CXX_DEBUG ${FLAGS_CXX} ${FLAGS_CXX_DEBUG})
148: set(COMPILE_OPTIONS_CXX_RELEASE ${FLAGS_CXX} ${FLAGS_CXX_RELEASE})
149: set(COMPILE_OPTIONS_CXX_MINSIZEREL ${FLAGS_CXX} ${FLAGS_CXX_MINSIZEREL})
150: set(COMPILE_OPTIONS_CXX_RELWITHDEBINFO ${FLAGS_CXX} ${FLAGS_CXX_RELWITHDEBINFO})
151: 
152: set(COMPILE_OPTIONS_ASM_DEBUG ${FLAGS_ASM} ${FLAGS_ASM_DEBUG})
153: set(COMPILE_OPTIONS_ASM_RELEASE ${FLAGS_ASM} ${FLAGS_ASM_RELEASE})
154: set(COMPILE_OPTIONS_ASM_MINSIZEREL ${FLAGS_ASM} ${FLAGS_ASM_MINSIZEREL})
155: set(COMPILE_OPTIONS_ASM_RELWITHDEBINFO ${FLAGS_ASM} ${FLAGS_ASM_RELWITHDEBINFO})
156: 
157: set(LINKER_OPTIONS_DEBUG ${LINK_FLAGS} ${LINK_FLAGS_DEBUG})
158: set(LINKER_OPTIONS_RELEASE ${LINK_FLAGS} ${LINK_FLAGS_RELEASE})
159: set(LINKER_OPTIONS_MINSIZEREL ${LINK_FLAGS} ${LINK_FLAGS_MINSIZEREL})
160: set(LINKER_OPTIONS_RELWITHDEBINFO ${LINK_FLAGS} ${LINK_FLAGS_RELWITHDEBINFO})
161: 
162: set(LINKER_LIBRARIES ${LINK_LIBRARIES})
163: 
164: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
165:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
166:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
167:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
168:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
169:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
170:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
171: elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
172:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_RELEASE})
173:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_RELEASE})
174:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_RELEASE})
175:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_RELEASE})
176:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_RELEASE})
177:     set(LINKER_OPTIONS ${LINKER_OPTIONS_RELEASE})
178: elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
179:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_MINSIZEREL})
180:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_MINSIZEREL})
181:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_MINSIZEREL})
182:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_MINSIZEREL})
183:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_MINSIZEREL})
184:     set(LINKER_OPTIONS ${LINKER_OPTIONS_MINSIZEREL})
185: elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
186:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_RELWITHDEBINFO})
187:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_RELWITHDEBINFO})
188:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_RELWITHDEBINFO})
189:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_RELWITHDEBINFO})
190:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_RELWITHDEBINFO})
191:     set(LINKER_OPTIONS ${LINKER_OPTIONS_RELWITHDEBINFO})
192: else()
193:     message(FATAL_ERROR "Invalid build type: " ${CMAKE_BUILD_TYPE})
194: endif()
195: 
```

Explanation:
- line 31-39: Depending on the variable `BAREMETAL_RPI_TARGET` which we will define later on, we define the variables `BAREMETAL_ARCH_CPU_OPTIONS` and `BAREMETAL_TARGET_KERNEL`, which define specific compiler options for the platform we're building fir (for now Raspberry Pi 3 or 4), and the base name of the kernel image file
- line 40: We define the variable `BAREMETAL_LOAD_ADDRESS` to hold the start address for the baremetal application
- line 42-45: We define the variable `DEFINES_C` to hold the generic compiler definitions for C and C++, independent of the build configuration
- line 46-49: We define the variables `DEFINES_C_DEBUG`, `DEFINES_C_RELEASE`, `DEFINES_C_MINSIZEREL` and `DEFINES_C_RELWITHDEBINFO` to hold specific compiler definitions for C and C++ per build configuration
- line 50-53: We define the variable `DEFINES_ASM` to hold the generic compiler definitions for assembly, independent of the build configuration
- line 55-68: We define the variable `FLAGS_C` to hold the generic compiler options for C, independent of the build configuration
- line 70-76: We define the variables `FLAGS_C_DEBUG`, `FLAGS_C_RELEASE`, `FLAGS_C_MINSIZEREL` and `FLAGS_C_RELWITHDEBINFO` to hold specific compiler options for C per build configuration
- line 78-95: We define the variable `FLAGS_CXX` to hold the generic compiler options for C++, independent of the build configuration
- line 98-104: We define the variables `FLAGS_CXX_DEBUG`, `FLAGS_CXX_RELEASE`, `FLAGS_CXX_MINSIZEREL` and `FLAGS_CXX_RELWITHDEBINFO` to hold specific compiler options for C++ per build configuration
- line 106: We define the variable `FLAGS_ASM` to hold the generic compiler options for assembly, independent of the build configuration
- line 107-110: We define the variables `FLAGS_ASM_DEBUG`, `FLAGS_ASM_RELEASE`, `FLAGS_ASM_MINSIZEREL` and `FLAGS_ASM_RELWITHDEBINFO` to hold specific compiler options for assembly per build configuration
- line 112-117: We define the variable `LINK_FLAGS` to hold the linker options
- line 118-121: We define the variables `LINK_FLAGS_DEBUG`, `LINK_FLAGS_RELEASE`, `LINK_FLAGS_MINSIZEREL` and `LINK_FLAGS_RELWITHDEBINFO` to hold specific linker options per build configuration
- line 123: We add to variable `LINK_LIBRARIES` holding the generic list of libraries to link to (empty for now)
- line 124: We add to variable `LINK_DIRECTORIES` holding the generic list of directories to search for linking (empty for now)
- line 126: We define the variable `SUPPORTED_CPP_STANDARD` to holding the C++ standard used
- line 128-130: We print the current C++ and C compiler versions, as well as the C++ standard version just defined
- line 132-135: We combine the generic compiler definitions for C and C++ with the configuration specific ones in variables:
  - `COMPILE_DEFINITIONS_C_DEBUG`
  - `COMPILE_DEFINITIONS_C_RELEASE`
  - `COMPILE_DEFINITIONS_C_MINSIZEREL`
  - `COMPILE_DEFINITIONS_C_RELWITHDEBINFO`
- line 137-140: We combine the generic compiler definitions for assembly with the configuration specific ones in variables:
  - `COMPILE_DEFINITIONS_ASM_DEBUG`
  - `COMPILE_DEFINITIONS_ASM_RELEASE`
  - `COMPILE_DEFINITIONS_ASM_MINSIZEREL`
  - `COMPILE_DEFINITIONS_ASM_RELWITHDEBINFO`
- line 142-145: We combine the generic compiler options for C with the configuration specific ones in variables:
  - `COMPILE_OPTIONS_C_DEBUG`
  - `COMPILE_OPTIONS_C_RELEASE`
  - `COMPILE_OPTIONS_C_MINSIZEREL`
  - `COMPILE_OPTIONS_C_RELWITHDEBINFO`
- line 147-150: We combine the generic compiler options for C++ with the configuration specific ones in variables:
  - `COMPILE_OPTIONS_CXX_DEBUG`
  - `COMPILE_OPTIONS_CXX_RELEASE`
  - `COMPILE_OPTIONS_CXX_MINSIZEREL`
  - `COMPILE_OPTIONS_CXX_RELWITHDEBINFO`
- line 152-155: We combine the generic compiler options for assembly with the configuration specific ones in variables:
  - `COMPILE_OPTIONS_ASM_DEBUG`
  - `COMPILE_OPTIONS_ASM_RELEASE`
  - `COMPILE_OPTIONS_ASM_MINSIZEREL`
  - `COMPILE_OPTIONS_ASM_RELWITHDEBINFO`
- line 157-160: We combine the generic linker options with the configuration specific ones in variables:
  - `LINKER_OPTIONS_DEBUG`
  - `LINKER_OPTIONS_RELEASE`
  - `LINKER_OPTIONS_MINSIZEREL`
  - `LINKER_OPTIONS_RELWITHDEBINFO`
- line 162: We set the variable `LINKER_LIBRARIES` to hold the general libraries to link to
- line 164-194: Depending on the current build configuration, we define the following variables to the correct ones for the build configuration set:
  - `COMPILE_DEFINITIONS_C`
  - `COMPILE_DEFINITIONS_ASM`
  - `COMPILE_OPTIONS_C`
  - `COMPILE_OPTIONS_CXX`
  - `COMPILE_OPTIONS_ASM`
  - `LINKER_OPTIONS`

We need to set a default value for the platform, and we add a variable to be used to set verbose build output.
Just before the main project definition insert the following:

#### Adding some variables

```cmake
File: CMakeLists.txt
23: option(VERBOSE_BUILD "Verbose build" ON)
24: 
25: if (VERBOSE_BUILD)
26:     set(CMAKE_VERBOSE_MAKEFILE ON CACHE STRING "Verbose build" FORCE)
27: else()
28:     set(CMAKE_VERBOSE_MAKEFILE OFF CACHE STRING "Verbose build" FORCE)
29: endif()
30: set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
31: set(CMAKE_COLOR_MAKEFILE   ON)
32: 
33: if("${BAREMETAL_RPI_TARGET}" STREQUAL "")
34:     set(BAREMETAL_RPI_TARGET 3) # RPI board type (3/4)
35: endif()
36: 
37: message(STATUS "\n** Setting up project **\n--")
38: 
39: message(STATUS "\n##################################################################################")
40: message(STATUS "\n** Setting up toolchain **\n--")
41: 
42: project(baremetal-main
43:     DESCRIPTION "Baremetal overall project")
44: 
```

Explanation:
- line 23: We define a variable `VERBOSE_BUILD` to enable or disable verbose build output. The default is ON
- line 25-29: Depending on the value of `VERBOSE_BUILD`, we set the standard CMake variable `CMAKE_VERBOSE_MAKEFILE` to ON or OFF. This is placed in the CMake cache
- line 30: We set the standard CMake variable `CMAKE_EXPORT_COMPILE_COMMANDS` to ON. This will make CMake generate a JSON file `compile_commands.json` in the CMake build directory, that can be used by Visual Studio and other tools.
This can come in handy when trying to figure out which source is compiler with which options
- line 31: We set the standard CMake variable `CMAKE_COLOR_MAKEFILE` to ON. This will generate CMake configuration and build output with ANSI color output. This will not work inside Visual Studio , but it will output in color on the command line
- line 33-35: Finally, we se the variable `BAREMETAL_RPI_TARGET`, if not set already, to 3, to make it default to Raspberry Pi 3.

### Update default variables in CMakeSettings.json

In order to get more verbose output from CMake, and to set the correct target platform (we will select RaspBerry Pi 3 here), we will need to update `CMakeSettings.json`.
We will set the variable `VERBOSE_BUILD` to ON, and `BAREMETAL_RPI_TARGET` to 3.

```json
File: CMakeSettings.json
1: {
2:   "environments": [ {} ],
3:   "configurations": [
4:     {
5:       "name": "BareMetal-Debug",
6:       "generator": "Ninja",
7:       "configurationType": "Debug",
8:       "buildRoot": "${projectDir}\\cmake-${name}",
9:       "installRoot": "${projectDir}\\output\\install\\${name}",
10:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_RPI_TARGET=3",
11:       "buildCommandArgs": "",
12:       "ctestCommandArgs": "",
13:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
14:       "inheritEnvironments": [ "gcc-arm" ]
15:     },
16:     {
17:       "name": "BareMetal-Release",
18:       "generator": "Ninja",
19:       "configurationType": "Release",
20:       "buildRoot": "${projectDir}\\cmake-${name}",
21:       "installRoot": "${projectDir}\\output\\install\\${name}",
22:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_RPI_TARGET=3",
23:       "buildCommandArgs": "",
24:       "ctestCommandArgs": "",
25:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
26:       "inheritEnvironments": [ "gcc-arm" ]
27:     },
28:     {
29:       "name": "BareMetal-RelWithDebInfo",
30:       "generator": "Ninja",
31:       "configurationType": "RelWithDebInfo",
32:       "buildRoot": "${projectDir}\\cmake-${name}",
33:       "installRoot": "${projectDir}\\output\\install\\${name}",
34:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_RPI_TARGET=3",
35:       "buildCommandArgs": "",
36:       "ctestCommandArgs": "",
37:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
38:       "inheritEnvironments": [ "gcc-arm" ]
39:     },
40:     {
41:       "name": "BareMetal-MinSizeRel",
42:       "generator": "Ninja",
43:       "configurationType": "MinSizeRel",
44:       "buildRoot": "${projectDir}\\cmake-${name}",
45:       "installRoot": "${projectDir}\\output\\install\\${name}",
46:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_RPI_TARGET=3",
47:       "buildCommandArgs": "",
48:       "ctestCommandArgs": "",
49:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
50:       "inheritEnvironments": [ "gcc-arm" ]
51:     }
52:   ]
53: }
```

### Update demo CMake file

With the addition of the set of variables we just defined, we can simplify the demo application CMake file a bit:

```cmake
File: code/applications/demo/CMakeLists.txt
1: project(demo
2:     DESCRIPTION "Demo application"
3:     LANGUAGES CXX ASM)
4: 
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
17: set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
18: set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
19: set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
20: set(PROJECT_INCLUDE_DIRS_PRIVATE )
21: set(PROJECT_INCLUDE_DIRS_PUBLIC )
22: 
23: set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})
24: 
25: set(PROJECT_DEPENDENCIES
26:     baremetal
27:     )
28: 
29: set(PROJECT_LIBS
30:     ${LINKER_LIBRARIES}
31:     ${PROJECT_DEPENDENCIES}
32:     )
33: 
34: set(PROJECT_SOURCES
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/start.S
37:     )
38: 
39: set(PROJECT_INCLUDES_PUBLIC )
40: set(PROJECT_INCLUDES_PRIVATE )
41: 
42: if (CMAKE_VERBOSE_MAKEFILE)
43:     display_list("Package                           : " ${PROJECT_NAME} )
44:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
45:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
46:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
47:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
48:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
49:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
50:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
51:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
52:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
53:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
54:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
55:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
56:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
57:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
58:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
59:     display_list("Link libs                         : " ${PROJECT_LIBS} )
60:     display_list("Source files                      : " ${PROJECT_SOURCES} )
61:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
62:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
63: endif()
64: 
65: if (PLATFORM_BAREMETAL)
66:     set(START_GROUP -Wl,--start-group)
67:     set(END_GROUP -Wl,--end-group)
68: endif()
69: 
70: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
71: 
72: target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
73: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
74: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
75: target_compile_definitions(${PROJECT_NAME} PRIVATE 
76:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
77:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
78:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
79:     )
80: target_compile_definitions(${PROJECT_NAME} PUBLIC 
81:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
82:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
83:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
84:     )
85: target_compile_options(${PROJECT_NAME} PRIVATE 
86:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
87:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
88:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
89:     )
90: target_compile_options(${PROJECT_NAME} PUBLIC 
91:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
92:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
93:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
94:     )
95: 
96: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
97: 
98: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
99: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
100:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
101: endif()
102: 
103: link_directories(${LINK_DIRECTORIES})
104: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
105: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
106: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
107: 
108: add_subdirectory(create-image)
109: 
```

- line 14: We can now define the compiler definitions for C++ using the variables set up for C/C++ compiler definitions
- line 16: Similarly for the compiler definitions for assembly using the variables set up for assembly compiler definitions
- line 17: Similarly for the compiler options for C++ using the variables set up for C/C++ compiler options
- line 19: Similarly for the compiler options for assembly using the variables set up for assembly compiler options
- line 23: Similar for linker options using the variables set up for linker options
- line 25-27: We now add the baremetal library to our dependencies. This means its public include directories are now used, and the library is linked to
- line 29-32: We can now link to all standard libraries by adding `LINKER_LIBRARIES` to `PROJECT_LIBS`
- line 42-63: Using the custom CMake function `display_list` we can now print all settings for the project. This is only done if verbose output is requested
- line 96: We can now use the `SUPPORTED_CPP_STANDARD` to set the C++ standard property on the target
- line 103: We set the serach path for standard libraries using `LINK_DIRECTORIES`

### Update demo-image CMake file

The same way, we can update the demo-image CMake file:

```cmake
File: code/applications/demo/create-image/CMakeLists.txt
1: project(demo-image
2:     DESCRIPTION "Kernel image for demo RPI 64 bit bare metal")
3: 
4: message(STATUS "\n**********************************************************************************\n")
5: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
6: 
7: message("\n** Setting up ${PROJECT_NAME} **\n")
8: 
9: set(DEPENDENCY demo)
10: set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)
11: 
12: create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
13: 
```

The only difference here is that the `BAREMETAL_TARGET_KERNEL` is defined in the main CMake file, so we can remove the definition here.

# ======================================

## Creating the library code - step 1

So let's start adding some code for the library, and set up the project for this library.

As we will need to add quite some code, let's do it in small steps.

The first step we'll take is including a header to contain standard ARM instructions, starting with the NOP (No operation) instructions. We will then use this in a loop to wait for a while.

### ARMInstructions.h

We add ARM instructions.

`code/libraries/baremetal/include/baremetal/ARMInstructions.h`
```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : ArmInstructions.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Common instructions for ARM for e.g. synchronization
//
//------------------------------------------------------------------------------

#pragma once

#include <baremetal/Macros.h>

/// @file
/// ARM instructions represented as macros for ease of use.
///
/// For specific registers, we also define the fields and their possible values.

/// @brief NOP instruction
#define NOP()                           asm volatile("nop")
```

This header declares a number of standard ARM instructions. More will be added later.

### Dummy.cpp

As the baremetal currently has no source files, only a header file, we will create a dummy C++ file to allow for building the library:

`code/libraries/baremetal/src/Dummy.cpp`
```cpp
// This is just a dummy file to allow creating the baremetal project

```

### Project setup for baremetal

First let's set up the project for the library:

`code/libraries/baremetal/CMakeLists.txt`
```cmake
message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

project(baremetal
	DESCRIPTION "Bare metal library"
	LANGUAGES CXX ASM)

set(PROJECT_TARGET_NAME ${PROJECT_NAME})

set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE ${COMPILE_DEFINITIONS_C})
set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
set(PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE ${COMPILE_DEFINITIONS_ASM})
set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
set(PROJECT_INCLUDE_DIRS_PRIVATE )
set(PROJECT_INCLUDE_DIRS_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})

set(PROJECT_DEPENDENCIES
    )

set(PROJECT_LIBS
    ${LINKER_LIBRARIES}
    ${PROJECT_DEPENDENCIES}
    )

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Dummy.cpp
    )

set(PROJECT_INCLUDES_PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
    )
set(PROJECT_INCLUDES_PRIVATE )

if (CMAKE_VERBOSE_MAKEFILE)
    display_list("Package                           : " ${PROJECT_NAME} )
    display_list("Package description               : " ${PROJECT_DESCRIPTION} )
    display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
    display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
    display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
    display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
    display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
    display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
    display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
    display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
    display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
    display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
    display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
    display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
    display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
    display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
    display_list("Link libs                         : " ${PROJECT_LIBS} )
    display_list("Source files                      : " ${PROJECT_SOURCES} )
    display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
    display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
endif()

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
    )
target_compile_definitions(${PROJECT_NAME} PUBLIC 
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
    )
target_compile_options(${PROJECT_NAME} PRIVATE 
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
    )
target_compile_options(${PROJECT_NAME} PUBLIC 
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
    )

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})

list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
endif()

link_directories(${LINK_DIRECTORIES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})

show_target_properties(${PROJECT_NAME})
```

You will recognize the structure used in the previous demo project, with a few additions:

- There is a section `if (CMAKE_VERBOSE_MAKEFILE)` ... `endif()` that uses the custom CMake function `display_list` which was defined [before](###CMake-function-display_list) a number of times.
This simply prints out information on the project, in case we request verbose output.
- The last line uses the custom CMake function `show_target_properties`, which was also defined [before](###CMake-function-show_target_properties)
This prints out various properties attached to the target (in this case the library), in case we request verbose output.
- The `target_compile_definitions` and `target_compile_options` (both PRIVATE and PUBLIC) are set in a slightly different way, using variable such as:
  - `PROJECT_COMPILE_DEFINITIONS_C_PRIVATE`
  - `PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE`
  - `PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE`
  
  These variables are used to e.g. define compiler definitions for a specific language, in this case C, C++ or assembly.
  These variables are set in the beginning of the file, and are based on the common variables defined in the main CMake file [here](###Common-compiler-directives).

### Update application code

We will use the NOP instruction (which is part of the baremetal library) in a simple loop:

code/applications/demo/src/main.cpp
```cpp
#include "baremetal/ARMInstructions.h"

int main()
{
    for (int i = 0; i < 1000000; ++i)
    {
        NOP();
    }
    return 0;
}
```

### Update project setup for demo application

We will update the demo application CMakeLists.txt in a similar way:

`code/applications/demo/CMakeLists.txt`
```cmake
project(demo
    DESCRIPTION "Demo application"
    LANGUAGES CXX ASM)

message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

message("\n** Setting up ${PROJECT_NAME} **\n")

include(functions)

set(PROJECT_TARGET_NAME ${PROJECT_NAME}.elf)

set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE ${COMPILE_DEFINITIONS_C})
set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
set(PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE ${COMPILE_DEFINITIONS_ASM})
set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
set(PROJECT_INCLUDE_DIRS_PRIVATE )
set(PROJECT_INCLUDE_DIRS_PUBLIC )

set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})

set(PROJECT_DEPENDENCIES
    baremetal
    )

set(PROJECT_LIBS
    ${LINKER_LIBRARIES}
    ${PROJECT_DEPENDENCIES}
    )

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    )

set(PROJECT_INCLUDES_PUBLIC )
set(PROJECT_INCLUDES_PRIVATE )

if (CMAKE_VERBOSE_MAKEFILE)
    display_list("Package                           : " ${PROJECT_NAME} )
    display_list("Package description               : " ${PROJECT_DESCRIPTION} )
    display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
    display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
    display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
    display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
    display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
    display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
    display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
    display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
    display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
    display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
    display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
    display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
    display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
    display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
    display_list("Link libs                         : " ${PROJECT_LIBS} )
    display_list("Source files                      : " ${PROJECT_SOURCES} )
    display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
    display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
endif()

if (PLATFORM_BAREMETAL)
    set(START_GROUP -Wl,--start-group)
    set(END_GROUP -Wl,--end-group)
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})

target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
    )
target_compile_definitions(${PROJECT_NAME} PUBLIC 
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
    )
target_compile_options(${PROJECT_NAME} PRIVATE 
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
    )
target_compile_options(${PROJECT_NAME} PUBLIC 
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
    $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
    )

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})

list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
endif()

link_directories(${LINK_DIRECTORIES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})

add_subdirectory(create-image)
```

So, we re-use the compiler definitions and compiler and linker options from the main `CMakeLists.txt` file.
We also add a dependency to the baremetal library, such that its exported include directories become available, and we link to this library.

### Update project setup for demo-image project

We will also update the project for the demo image creation, as the `BAREMETAL_TARGET_KERNEL` variable is also defined in the top level CMake file (and is now dependent on the target system we build for).
In the We will update the demo application CMakeLists.txt in a similar way:

`code/applications/demo/create-image/CMakeLists.txt`
```cmake
project(demo-image
    DESCRIPTION "Kernel image for demo RPI 64 bit bare metal")

message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

message("\n** Setting up ${PROJECT_NAME} **\n")

set(DEPENDENCY demo)
set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)

create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
```

So we simply remove the setting of the `BAREMETAL_TARGET_KERNEL` variable.

### Configure and build

We are now able to configure the project again, and build it.

The output for the configure step should be similar to:

```text
1> CMake generation started for configuration: 'BareMetal-Debug'.
1> Command line: "C:\Windows\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="D:\Projects\baremetal.github\output\install\BareMetal-Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH="D:\Projects\baremetal.github\baremetal.toolchain" -DVERBOSE_BUILD=ON -DBAREMETAL_RPI_TARGET=3 -DCMAKE_MAKE_PROGRAM="C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" "D:\Projects\baremetal.github" 2>&1"
1> Working directory: D:\Projects\baremetal.github\cmake-BareMetal-Debug
1> [CMake] -- CMake 3.20.21032501-MSVC_2
1> [CMake] -- 
1> [CMake] ** Setting up project **
1> [CMake] --
1> [CMake] -- 
1> [CMake] ##################################################################################
1> [CMake] -- 
1> [CMake] ** Setting up toolchain **
1> [CMake] --
1> [CMake] -- TOOLCHAIN_ROOT           D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf
1> [CMake] -- Processor                aarch64
1> [CMake] -- Platform tuple           aarch64-none-elf
1> [CMake] -- Assembler                D:/Toolchains/arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
1> [CMake] -- C compiler               D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
1> [CMake] -- C++ compiler             D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-g++.exe
1> [CMake] -- Archiver                 D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ar.exe
1> [CMake] -- Linker                   D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ld.exe
1> [CMake] -- ObjCopy                  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-objcopy.exe
1> [CMake] -- Std include path         D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1/include
1> [CMake] -- CMAKE_EXE_LINKER_FLAGS=   -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1
1> [CMake] -- C++ compiler version:    13.2.1
1> [CMake] -- C compiler version:      13.2.1
1> [CMake] -- C++ supported standard:  17
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/applications
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/applications/demo
1> [CMake] 
1> [CMake] ** Setting up demo **
1> [CMake] 
1> [CMake] -- Package                           :  demo
1> [CMake] -- Package description               :  Demo application
1> [CMake] -- Defines C - public                : 
1> [CMake] -- Defines C - private               : 
1> [CMake] -- Defines C++ - public              : 
1> [CMake] -- Defines C++ - private             :  PLATFORM_BAREMETAL RPI_TARGET=3 _DEBUG
1> [CMake] -- Defines ASM - private             :  PLATFORM_BAREMETAL RPI_TARGET=3
1> [CMake] -- Compiler options C - public       : 
1> [CMake] -- Compiler options C - private      : 
1> [CMake] -- Compiler options C++ - public     : 
1> [CMake] -- Compiler options C++ - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter
1> [CMake] -- Compiler options ASM - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2
1> [CMake] -- Include dirs - public             : 
1> [CMake] -- Include dirs - private            : 
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      :  baremetal
1> [CMake] -- Link libs                         :  baremetal
1> [CMake] -- Source files                      :  D:/Projects/baremetal.github/code/applications/demo/src/main.cpp
1> [CMake] -- Include files - public            : 
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/applications/demo/create-image
1> [CMake] 
1> [CMake] ** Setting up demo-image **
1> [CMake] 
1> [CMake] -- create_image demo-image kernel8.img demo
1> [CMake] -- TARGET_NAME demo.elf
1> [CMake] -- generate D:/Projects/baremetal.github/deploy/Debug/demo-image/kernel8.img from D:/Projects/baremetal.github/output/Debug/bin/demo
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/libraries
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/libraries/baremetal
1> [CMake] -- Package                           :  baremetal
1> [CMake] -- Package description               :  Bare metal library
1> [CMake] -- Defines C - public                : 
1> [CMake] -- Defines C - private               : 
1> [CMake] -- Defines C++ - public              : 
1> [CMake] -- Defines C++ - private             :  PLATFORM_BAREMETAL RPI_TARGET=3 _DEBUG
1> [CMake] -- Defines ASM - private             :  PLATFORM_BAREMETAL RPI_TARGET=3
1> [CMake] -- Compiler options C - public       : 
1> [CMake] -- Compiler options C - private      : 
1> [CMake] -- Compiler options C++ - public     : 
1> [CMake] -- Compiler options C++ - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter
1> [CMake] -- Compiler options ASM - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2
1> [CMake] -- Include dirs - public             :  D:/Projects/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Include dirs - private            : 
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      : 
1> [CMake] -- Link libs                         : 
1> [CMake] -- Source files                      :  D:/Projects/baremetal.github/code/libraries/baremetal/src/Dummy.cpp
1> [CMake] -- Include files - public            :  D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/ARMInstructions.h
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] -- Properties for baremetal
1> [CMake] -- Target type                       :  STATIC_LIBRARY
1> [CMake] -- Target defines                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:PLATFORM_BAREMETAL RPI_TARGET=3 _DEBUG> $<$<COMPILE_LANGUAGE:ASM>:PLATFORM_BAREMETAL RPI_TARGET=3> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target options                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target include dirs public        :  D:/Projects/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target include dirs private       :  D:/Projects/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target link libraries             :  LIBRARIES-NOTFOUND
1> [CMake] -- Target link options               :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target exported defines           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported options           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported include dirs      :  D:/Projects/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target exported link libraries    :  LIBRARIES_EXPORTS-NOTFOUND
1> [CMake] -- Target imported dependencies      : 
1> [CMake] -- Target imported link libraries    : 
1> [CMake] -- Target link dependencies          :  LINK_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target manual dependencies        :  EXPLICIT_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target static library location    :  D:/Projects/baremetal.github/output/Debug/lib
1> [CMake] -- Target dynamic library location   :  LIBRARY_LOCATION-NOTFOUND
1> [CMake] -- Target binary location            :  RUNTIME_LOCATION-NOTFOUND
1> [CMake] -- Target link flags                 :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target version                    :  TARGET_VERSION-NOTFOUND
1> [CMake] -- Target so-version                 :  TARGET_SOVERSION-NOTFOUND
1> [CMake] -- Target output name                :  baremetal
1> [CMake] -- Configuring done
1> [CMake] -- Generating done
1> [CMake] -- Build files have been written to: D:/Projects/baremetal.github/cmake-BareMetal-Debug
1> Extracted CMake variables.
1> Extracted source files and headers.
1> Extracted code model.
1> Extracted toolchain configurations.
1> Extracted includes paths.
1> CMake generation finished.
```

As you can see, all configured projects are shown, with their settings. This is very helpful in finding configuration problems.

We can then build:

```text
>------ Rebuild All started: Project: baremetal, Configuration: BareMetal-Debug ------
  [1/1] "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"  -t clean 
  Cleaning... 5 files.
  [1/5] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DPLATFORM_BAREMETAL -DRPI_TARGET=3 -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Dummy.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\Dummy.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Dummy.cpp.obj -c ../code/libraries/baremetal/src/Dummy.cpp
  [2/5] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DPLATFORM_BAREMETAL -DRPI_TARGET=3 -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [3/5] cmd.exe /C "cd . && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E rm -f ..\output\Debug\lib\libbaremetal.a && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ar.exe qc ..\output\Debug\lib\libbaremetal.a  code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Dummy.cpp.obj && D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ranlib.exe ..\output\Debug\lib\libbaremetal.a && cd ."
  [4/5] cmd.exe /C "cd . && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -o ..\output\Debug\bin\demo.elf  ../output/Debug/lib/libbaremetal.a && cd ."
  [5/5] cmd.exe /C "cd /D D:\Projects\baremetal.github\cmake-BareMetal-Debug\code\applications\demo\create-image && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal.github/output/Debug/bin/demo.elf -O binary D:/Projects/baremetal.github/deploy/Debug/demo-image/kernel8.img"

Rebuild All succeeded.
```

You can see that the demo application's main.cpp is compiled (step 2), as well as the baremetal library's Dummy.cpp (step 1).
Then the baremetal library is removed and re-created (step 3).
The demo application is linked, using the baremetal library (step 4), and finally the image is created (step 5).
It takes a while to start understanding what the compiler and linker actually do, so we'll dive a bit deeper here:

#### Compiling sources (step 1 and 2):

```text
D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe
  -DPLATFORM_BAREMETAL -DRPI_TARGET=3 -D_DEBUG
  -I../code/libraries/baremetal/include
  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
  -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new
  -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib
  -nostdinc++ -fno-exceptions -fno-rtti
  -O0 -Wno-unused-variable -Wno-unused-parameter
  -std=gnu++17
  -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d
  -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
  -c ../code/applications/demo/src/main.cpp
```

##### Compiler

The first part is the compiler call, the rest are command line parameters for the compiler.

##### Compiler definitions

The -D options specify all compiler definitions. You will recognize the options specified in CMakeSettings.json: `"cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_RPI_TARGET=3"`.
- The first option triggers setting the `CMAKE_VERBOSE_MAKEFILE` setting, that shows the configuration output.
- The second trigger setting the `RPI_TARGET=3` definition.
- The `PLATFORM_BAREMETAL` definition is triggered by the main CMake file:
```cmake
set(DEFINES_C 
    PLATFORM_BAREMETAL
    RPI_TARGET=${BAREMETAL_RPI_TARGET}
    )
```
- The \_DEBUG definition is caused by the `CMAKE_BUILD_TYPE` variable, which is set to `Debug` as we are building a Debug application. This is also done in the main CMake file:
```cmake
set(DEFINES_C_DEBUG _DEBUG)
set(DEFINES_C_RELEASE NDEBUG)
set(DEFINES_C_MINSIZEREL NDEBUG)
set(DEFINES_C_RELWITHDEBINFO NDEBUG)
```
- The generic and the build configuration specific definitions are later combined:
```cmake
set(COMPILE_DEFINITIONS_C_DEBUG ${DEFINES_C} ${DEFINES_C_DEBUG})
set(COMPILE_DEFINITIONS_C_RELEASE ${DEFINES_C} ${DEFINES_C_RELEASE})
set(COMPILE_DEFINITIONS_C_MINSIZEREL ${DEFINES_C} ${DEFINES_C_MINSIZEREL})
set(COMPILE_DEFINITIONS_C_RELWITHDEBINFO ${DEFINES_C} ${DEFINES_C_RELWITHDEBINFO})
```
- And then later variable for the specific build configuration are set:
```cmake
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
    set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
    set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
    set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
    set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
    set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
...
```

##### Compiler include paths

The parameter `-I../code/libraries/baremetal/include` specifies the directory to use for include files when building main.cpp. This is a results of the added dependency in the `code/applicationd/demo` CMake file:

```cmake
set(PROJECT_DEPENDENCIES
    baremetal
    )
```

While is added to the libraries to be linked:

```cmake
set(PROJECT_LIBS
    ${LINKER_LIBRARIES}
    ${PROJECT_DEPENDENCIES}
    )
```

Which is then used to set the libraries to be linked:

```cmake
target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
```

##### Compiler options

The complete list below are all compiler options:

```text
-g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
-Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char
-nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -g -Wno-unused-variable -Wno-unused-parameter
-std=gnu++17
-MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d
```

- The first -g is automatically added by CMake as we are building for a debug configuration
- The `-mcpu=cortex-a53 -mlittle-endian -mcmodel=small` options are set due to the fact that we are building for Raspberry Pi 3:
```cmake
if (BAREMETAL_RPI_TARGET EQUAL 3)
    set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
    set(BAREMETAL_TARGET_KERNEL kernel8)
...
```
- The `-Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char
-nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti` options all are defined later on in the main CMake file, where they are combinex with `BAREMETAL_ARCH_CPU_OPTIONS`:
```cmake
set(FLAGS_CXX
    ${BAREMETAL_ARCH_CPU_OPTIONS}
    -Wall
    -Wextra
    -Werror
    -Wno-missing-field-initializers
    -Wno-unused-value
    -Wno-aligned-new
    -ffreestanding 
    -fsigned-char 
    -nostartfiles
    -mno-outline-atomics
    -nostdinc
    -nostdlib
    -nostdinc++
    -fno-exceptions
    -fno-rtti
    )
```
- The `-O0 -g -Wno-unused-variable -Wno-unused-parameter` options are set for the specific Debug build configuration:
```cmake
set(FLAGS_CXX_DEBUG -O0 -g -Wno-unused-variable -Wno-unused-parameter)
```
- The generic and the build configuration specific definitions are later combined:
```cmake
set(COMPILE_OPTIONS_CXX_DEBUG ${FLAGS_CXX} ${FLAGS_CXX_DEBUG})
set(COMPILE_OPTIONS_CXX_RELEASE ${FLAGS_CXX} ${FLAGS_CXX_RELEASE})
set(COMPILE_OPTIONS_CXX_MINSIZEREL ${FLAGS_CXX} ${FLAGS_CXX_MINSIZEREL})
set(COMPILE_OPTIONS_CXX_RELWITHDEBINFO ${FLAGS_CXX} ${FLAGS_CXX_RELWITHDEBINFO})
```
- And then later variable for the specific build configuration are set:
```cmake
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
    set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
    set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
    set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
    set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
    set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
...
```
- The option `-std=gnu++17` is automatically added by CMake due to setting the variable `SUPPORTED_CPP_STANDARD` in the main CMake file:
```cmake
set(SUPPORTED_CPP_STANDARD 17)
```
- Which is then later used when defining the target properties in the `code/application/demo` and `code/libraries/baremetal` CMake files:
```cmake
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
```
- Finally, the options `-MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d` are added automatically by CMake, to generate a dependency file.

##### Actual compilation

The final parameters determine the source files compiled, and the resulting object file:

```text
-o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
-c ../code/applications/demo/src/main.cpp
```

The -c option tells the compiler to compile the specified file, the -o option tells it to generate the specified object file. Notice that the paths are relative to the CMake build directory, in this case `cmake-BareMetal-Debug`.

#### Creating the baremetal library (step 3)

```text
cmd.exe /C 
 "cd . && 
  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E rm -f ..\output\Debug\lib\libbaremetal.a &&
  D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ar.exe
    qc
    ..\output\Debug\lib\libbaremetal.a
    code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Dummy.cpp.obj && 
  D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ranlib.exe
    ..\output\Debug\lib\libbaremetal.a && 
  cd ."
```

Here we see a total of 5 commands being performed inside a command shell:

- The first is just a cd command (actually moving to the same directory)
- The second is a cmake call to remove the baremetal library
- The third creates the baremetal library (the options qc mean _quick append_ and _create_)
- The fourth adds a symbol to the baremetal library
- The fifth is again a cd command to the same directory

#### Linking the demo application (step 4)

```text
cmd.exe /C
  "cd . &&
  D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe
    -g
    -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1
    -Wl,--section-start=.init=0x80000
    -T D:/Projects/baremetal.github/baremetal.ld
    -nostdlib -nostartfiles
    code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
    -o ..\output\Debug\bin\demo.elf
    -Wl,--start-group  ../output/Debug/lib/libbaremetal.a  -Wl,--end-group &&
  cd ."
```

Here we see a total of 3 commands being performed inside a command shell:

- The first is just a cd command (actually moving to the same directory)
- The second links the exeutable file
- The third is again a cd command to the same directory

Options used when linking are:
```text
-g
-LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1
-Wl,--section-start=.init=0x80000
-T D:/Projects/baremetal.github/baremetal.ld
-nostdlib -nostartfiles
code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
-o ..\output\Debug\bin\demo.elf
-Wl,--start-group  ../output/Debug/lib/libbaremetal.a  -Wl,--end-group
```

- The first -g is automatically added by CMake as we are building for a debug configuration
- The `-LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1` option is added by the toolchain:
```cmake
set(TOOLCHAIN_AUXILIARY_PATH ${TOOLCHAIN_ROOT}/lib/gcc/${TOOL_DESTINATION_PLATFORM}/13.2.1)

...

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
```
- So the `TOOLCHAIN_AUXILIARY_PATH` is set, and then appended to `CMAKE_EXE_LINKER_FLAGS` if not already there. This option specifies a directory searched for object and library files when linking the target.
The `CMAKE_EXE_LINKER_FLAGS` are automatically added to the linker flags for a target by CMake.
- The options `-Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles` are set in the main CMake file:
```cmake
set(LINK_FLAGS
    -Wl,--section-start=.init=${BAREMETAL_LOAD_ADDRESS}
    -T ${CMAKE_SOURCE_DIR}/baremetal.ld
    -nostdlib
    -nostartfiles 
    )
```
- This is then combined later on with the build configuration specific linker flags:
```cmake
set(LINKER_OPTIONS_DEBUG ${LINK_FLAGS} ${LINK_FLAGS_DEBUG})
set(LINKER_OPTIONS_RELEASE ${LINK_FLAGS} ${LINK_FLAGS_RELEASE})
set(LINKER_OPTIONS_MINSIZEREL ${LINK_FLAGS} ${LINK_FLAGS_MINSIZEREL})
set(LINKER_OPTIONS_RELWITHDEBINFO ${LINK_FLAGS} ${LINK_FLAGS_RELWITHDEBINFO})
```
- Subsequently, the linker flags for the selected build configuration is set:
```cmake
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
    set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
    set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
    set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
    set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
    set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
...
```
- The `LINKER_OPTIONS` variable is then used in the project CMake file (`code/applications/demo/CMakeLists.txt` and `code/libraries/baremetal/CMakeLists.txt`):
```cmake
set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})
```
- Then the target linker options are set using this veriable:
```cmake
list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
endif()
```
- What follows is a list of object files to be linked, in this case just one:
```text
code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
```
- This list is determined by the source files specified for the target:
```cmake
set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    )

set(PROJECT_INCLUDES_PUBLIC )
set(PROJECT_INCLUDES_PRIVATE )

...

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
```
- Next is the option `-o ..\output\Debug\bin\demo.elf`, which specifies the output of the linker, the application.
This is determined by the specification of the name and location of the target file in the project CMake file:
```cmake
set(PROJECT_TARGET_NAME ${PROJECT_NAME}.elf)

...

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
```
- As we are building an executable, the location will be the `OUTPUT_BIN_DIR`, the file name will be `PROJECT_TARGET_NAME`.
- Finally, the list of libraries to link to is specified: `-Wl,--start-group  ../output/Debug/lib/libbaremetal.a  -Wl,--end-group`. This is determined by the list of libraries to link to in the project CMake file:
```cmake
set(PROJECT_DEPENDENCIES
    baremetal
    )

set(PROJECT_LIBS
    ${LINKER_LIBRARIES}
    ${PROJECT_DEPENDENCIES}
    )

...

target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
```
- The reason libraries linked to is placed in a group (`-Wl,--start-group <libraries> -Wl,--end-group`) is that the GCC linker expects the lowest level symbols to be added last, and we cannot always guarantee the correct order of libraires.
By putting them in a group, the linker will iterate as many times as needed to resolve all symbols.

#### Creating the kernel image (step 5)

```text
cmd.exe /C 
 "cd /D D:\Projects\baremetal.github\cmake-BareMetal-Debug\code\applications\demo\create-image && 
  D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe 
    D:/Projects/baremetal.github/output/Debug/bin/demo.elf 
    -O binary D:/Projects/baremetal.github/deploy/Debug/demo-image/kernel8.img"
```

Here we see a 2 commands being performed inside a command shell:

- The first is just a cd command (moving into the create-image directory)
- The second creates the image file
  - This runs the objcopy tool
  - Its first parameter is the executable file to be placed in the image
  - The second parameter (specified with option `-O`) specifies the image file to be created.

### Running the application

Start QEMU listening to UART1 for the demo project:

```bat
tools\startQEMU-image-uart1.bat demo
```

Start debugging as shown in [Building](setting-up-project-structure.md###Building) and [Debugging](setting-up-project-structure.md###Debugging). Make sure the demo applications is selected as startup project.


# ==========================

The code beneath is clearly also in this repository, and will look slightly different, for copyright / licensing reasons. Please refer to [readme](../README.md##License).
This code contains documentation in Doxygen format. Later on we'll add scripting to generate Doxygen documentation from this code. The examples below leave this out for reasons of brevity.

### Types.h

First we define some standard types. Add the file `code/libraries/baremetal/include/baremetal/Types.h`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : Types.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Common types, platform dependent
//
//------------------------------------------------------------------------------

#pragma once

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

typedef unsigned long uint64;
typedef signed long int64;
typedef int64 intptr;
typedef uint64 uintptr;
typedef uint64 size_type;
typedef int64 ssize_type;

typedef size_type size_t;
typedef ssize_type ssize_t;
```

This header defines types for 8/16/32/64 bit integer types, both signed an unsigned, pointer like types, again both signed and unsigned, as well as size types, again both signed and unsigned.

### CharDevice.h

We then add an abstract class for character devices. Add the file `code/libraries/baremetal/include/baremetal/CharDevice.h`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : CharDevice.h
//
// Namespace   : baremetal
//
// Class       : CharDevice
//
// Description : Generic character read / write device interface
//
//------------------------------------------------------------------------------

#pragma once

namespace baremetal {

class CharDevice
{
public:
    virtual ~CharDevice() = default;

    virtual char Read() = 0;
    virtual void Write(char c) = 0;
};

} // namespace baremetal
```

This header declares an abstract class `CharDevice`, in the `baremetal` namespace, that provides the means to read and write characters. Our serial console will derive from this, effectively implementing the interface `CharDevice`.

### IMemoryAccess.h

Next we'll add an interface for memory read/write access. Add the file `code/libraries/baremetal/include/baremetal/IMemoryAccess.h`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : IMemoryAccess.h
//
// Namespace   : baremetal
//
// Class       : IMemoryAccess
//
// Description : Memory read/write abstract interface
//
//------------------------------------------------------------------------------

#pragma once

#include <baremetal/Types.h>

namespace baremetal {

class IMemoryAccess
{
public:
    virtual ~IMemoryAccess() = default;

    virtual uint8  Read8(uintptr address)                                                    = 0;
    virtual void   Write8(uintptr address, uint8 data)                                       = 0;
    virtual void   ReadModifyWrite8(uintptr address, uint8 mask, uint8 data, uint8 shift)    = 0;
    virtual uint16 Read16(uintptr address)                                                   = 0;
    virtual void   Write16(uintptr address, uint16 data)                                     = 0;
    virtual void   ReadModifyWrite16(uintptr address, uint16 mask, uint16 data, uint8 shift) = 0;
    virtual uint32 Read32(uintptr address)                                                   = 0;
    virtual void   Write32(uintptr address, uint32 data)                                     = 0;
    virtual void   ReadModifyWrite32(uintptr address, uint32 mask, uint32 data, uint8 shift) = 0;
};

} // namespace baremetal
```

This header declares an abstract class `IMemoryAccess`, in the `baremetal` namespace, that provides read / write access to memory, in size of 1, 2 and 4 bytes.

### MemoryAccess.h

We will then add a class that implements the `IMemoryAccess` interface. Add the file `code/libraries/baremetal/include/baremetal/MemoryAccess.h`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : MemoryAccess.h
//
// Namespace   : baremetal
//
// Class       : MemoryAccess
//
// Description : Memory read/write
//
//------------------------------------------------------------------------------

#pragma once

#include <baremetal/IMemoryAccess.h>

namespace baremetal {

class MemoryAccess : public IMemoryAccess
{
public:
    uint8  Read8(uintptr address) override;
    void   Write8(uintptr address, uint8 data) override;
    void   ReadModifyWrite8(uintptr address, uint8 mask, uint8 data, uint8 shift) override;
    uint16 Read16(uintptr address) override;
    void   Write16(uintptr address, uint16 data) override;
    void   ReadModifyWrite16(uintptr address, uint16 mask, uint16 data, uint8 shift) override;
    uint32 Read32(uintptr address) override;
    void   Write32(uintptr address, uint32 data) override;
    void   ReadModifyWrite32(uintptr address, uint32 mask, uint32 data, uint8 shift) override;
};

} // namespace baremetal
```

This header declares a class `MemoryAccess`, in the `baremetal` namespace, which implements `IMemoryAccess`.

### UART1.h

We add the class for the serial console. Add the file `code/libraries/baremetal/include/baremetal/UART1.h`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : UART1.h
//
// Namespace   : baremetal
//
// Class       : UART1
//
// Description : UART1 class
//
//------------------------------------------------------------------------------

#pragma once

#include <baremetal/CharDevice.h>

namespace baremetal {

class IMemoryAccess;

class UART1 : public CharDevice
{
private:
    bool           m_initialized;
    IMemoryAccess &m_memoryAccess;

public:
    UART1(IMemoryAccess &memoryAccess);
    void Initialize();
    char Read() override;
    void Write(char c) override;
    void WriteString(const char *str);
};

} // namespace baremetal
```

This header declares a class `UART1`, in the `baremetal` namespace, which implements `CharDevice`.

- The constructor used a `IMemoryAccess` interface to read and write to memory
- The class has an `Initialize` method to initialize the class. Once initialized, `m_initialized` will be set to `true`
- The `Read` and `Write` method implement the `CharDevice` interface methods, which allow reading and writing a single character
- The `WriteString` method extends the functionality by supporting writing a null-terminated string

Then we add the implementation of the classes just declared.
The `Types.h` header does not need implementation as it just declares some types.
The classes `CharDevice` and `IMemoryAccess` are abstract interfaces, so do not need implementation.

### BCMRegisters.h

We add registers of the Broadcom SoC in the Raspberry Pi. Add the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : BCMRegisters.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Locations and definitions for Raspberry Pi registers in the Broadcomm SoC
//
//------------------------------------------------------------------------------

#pragma once

#include <baremetal/Macros.h>

#if RPI_TARGET == 3
/// @brief Base address for Raspberry PI BCM I/O
#define RPI_BCM_IO_BASE 0x3F000000
#else
/// @brief Base address for Raspberry PI BCM I/O
#define RPI_BCM_IO_BASE 0xFE000000
#endif
/// @brief End address for Raspberry PI BCM I/O
#define RPI_BCM_IO_END                  (RPI_BCM_IO_BASE + 0xFFFFFF)

/// @brief Base address for Raspberry PI GPU I/O
#define RPI_GPU_IO_BASE                 0x7E000000

/// @brief Base address for Raspberry PI GPU memory (cached)
#define RPI_GPU_CACHED_BASE             0x40000000
/// @brief Base address for Raspberry PI GPU memory (uncached)
#define RPI_GPU_UNCACHED_BASE           0xC0000000

/// @brief Base address for Raspberry PI GPU memory
#define RPI_GPU_MEM_BASE                RPI_GPU_UNCACHED_BASE

/// Convert ARM I/O address to GPU bus address (also works for aliases)
#define RPI_ARM_TO_GPU(addr)            (((addr) & ~0xC0000000) | GPU_MEM_BASE)
/// Convert GPU bus address to ARM I/O address (also works for aliases)
#define RPI_GPU_TO_ARM(addr)            ((addr) & ~0xC0000000)

// Raspberry Pi GPIO

/// @brief Raspberry Pi GPIO registers base address
#define RPI_GPIO_BASE                   RPI_BCM_IO_BASE + 0x00200000
/// @brief Raspberry Pi GPIO function select register 0 (GPIO 0..9) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL0                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000000)
/// @brief Raspberry Pi GPIO function select register 1 (GPIO 10..19) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL1                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000004)
/// @brief Raspberry Pi GPIO function select register 2 (GPIO 20..29) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL2                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000008)
/// @brief Raspberry Pi GPIO function select register 3 (GPIO 30..39) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL3                static_cast<uintptr>(RPI_GPIO_BASE + 0x0000000C)
/// @brief Raspberry Pi GPIO function select register 4 (GPIO 40..49) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL4                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000010)
/// @brief Raspberry Pi GPIO function select register 5 (GPIO 50..53) (3 bits / GPIO) (R/W)
#define RPI_GPIO_GPFSEL5                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000014)
/// @brief Raspberry Pi GPIO set register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPSET0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000001C)
/// @brief Raspberry Pi GPIO set register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPSET1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000020)
/// @brief Raspberry Pi GPIO clear register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPCLR0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000028)
/// @brief Raspberry Pi GPIO clear register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPCLR1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000002C)
/// @brief Raspberry Pi GPIO level register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEV0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000034)
/// @brief Raspberry Pi GPIO level register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEV1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000038)
/// @brief Raspberry Pi GPIO event detected register 0 (GPIO 0..31) (1 bit / GPIO) (R)
#define RPI_GPIO_GPEDS0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000040)
/// @brief Raspberry Pi GPIO event detected register 1 (GPIO 32..53) (1 bit / GPIO) (R)
#define RPI_GPIO_GPEDS1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000044)
/// @brief Raspberry Pi GPIO rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPREN0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000004C)
/// @brief Raspberry Pi GPIO rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPREN1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000050)
/// @brief Raspberry Pi GPIO falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPFEN0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000058)
/// @brief Raspberry Pi GPIO falling edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPFEN1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000005C)
/// @brief Raspberry Pi GPIO high level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPHEN0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000064)
/// @brief Raspberry Pi GPIO high level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPHEN1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000068)
/// @brief Raspberry Pi GPIO low level detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEN0                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000070)
/// @brief Raspberry Pi GPIO low level detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPLEN1                 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000074)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAREN0                static_cast<uintptr>(RPI_GPIO_BASE + 0x0000007C)
/// @brief Raspberry Pi GPIO asynchronous rising edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAREN1                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000080)
/// @brief Raspberry Pi GPIO asynchronous falling edge detect enable register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAFEN0                static_cast<uintptr>(RPI_GPIO_BASE + 0x00000088)
/// @brief Raspberry Pi GPIO asynchronous fallign edge detect enable register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPAFEN1                static_cast<uintptr>(RPI_GPIO_BASE + 0x0000008C)
#if RPI_TARGET <= 3
/// @brief Raspberry Pi GPIO pull up/down mode register (2 bits) (R/W)
#define RPI_GPIO_GPPUD     static_cast<uintptr>(RPI_GPIO_BASE + 0x00000094)
/// @brief Raspberry Pi GPIO pull up/down clock register 0 (GPIO 0..31) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPPUDCLK0 static_cast<uintptr>(RPI_GPIO_BASE + 0x00000098)
/// @brief Raspberry Pi GPIO pull up/down clock register 1 (GPIO 32..53) (1 bit / GPIO) (R/W)
#define RPI_GPIO_GPPUDCLK1 static_cast<uintptr>(RPI_GPIO_BASE + 0x0000009C)
#else // RPI target 4 or 5
#define RPI_GPIO_GPPINMUXSD static_cast<uintptr>(RPI_GPIO_BASE + 0x000000D0)
#define RPI_GPIO_GPPUPPDN0  static_cast<uintptr>(RPI_GPIO_BASE + 0x000000E4)
#define RPI_GPIO_GPPUPPDN1  static_cast<uintptr>(RPI_GPIO_BASE + 0x000000E8)
#define RPI_GPIO_GPPUPPDN2  static_cast<uintptr>(RPI_GPIO_BASE + 0x000000EC)
#define RPI_GPIO_GPPUPPDN3  static_cast<uintptr>(RPI_GPIO_BASE + 0x000000F0)
#endif

// Raspberry Pi auxilary registers (SPI1 / SPI2 / UART1)

/// @brief Raspberry Pi Auxilary registers base address
#define RPI_AUX_BASE                  RPI_BCM_IO_BASE + 0x00215000
/// @brief Raspberry Pi Auxiliary IRQ register
#define RPI_AUX_IRQ                   static_cast<uintptr>(RPI_AUX_BASE + 0x00000000) // AUXIRQ
/// @brief Raspberry Pi Auxiliary Enable register
#define RPI_AUX_ENABLE                static_cast<uintptr>(RPI_AUX_BASE + 0x00000004) // AUXENB

// Raspberry Pi auxilary mini UART registers (UART1)

/// @brief Raspberry Pi Mini UART (UART1) I/O register
#define RPI_AUX_MU_IO                 static_cast<uintptr>(RPI_AUX_BASE + 0x00000040)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register
#define RPI_AUX_MU_IER                static_cast<uintptr>(RPI_AUX_BASE + 0x00000044)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register
#define RPI_AUX_MU_IIR                static_cast<uintptr>(RPI_AUX_BASE + 0x00000048)
/// @brief Raspberry Pi Mini UART (UART1) Line Control register
#define RPI_AUX_MU_LCR                static_cast<uintptr>(RPI_AUX_BASE + 0x0000004C)
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register
#define RPI_AUX_MU_MCR                static_cast<uintptr>(RPI_AUX_BASE + 0x00000050)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register
#define RPI_AUX_MU_LSR                static_cast<uintptr>(RPI_AUX_BASE + 0x00000054)
/// @brief Raspberry Pi Mini UART (UART1) Modem Status register
#define RPI_AUX_MU_MSR                static_cast<uintptr>(RPI_AUX_BASE + 0x00000058)
/// @brief Raspberry Pi Mini UART (UART1) Scratch register
#define RPI_AUX_MU_SCRATCH            static_cast<uintptr>(RPI_AUX_BASE + 0x0000005C)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register
#define RPI_AUX_MU_CNTL               static_cast<uintptr>(RPI_AUX_BASE + 0x00000060)
/// @brief Raspberry Pi Mini UART (UART1) Extra Status register
#define RPI_AUX_MU_STAT               static_cast<uintptr>(RPI_AUX_BASE + 0x00000064)
/// @brief Raspberry Pi Mini UART (UART1) Baudrate register
#define RPI_AUX_MU_BAUD               static_cast<uintptr>(RPI_AUX_BASE + 0x00000068)

/// @brief Raspberry Pi Auxiliary Enable register values
/// @brief Raspberry Pi Auxiliary Enable register Enable SPI2
#define RPI_AUX_ENABLE_SPI2           BIT(2)
/// @brief Raspberry Pi Auxiliary Enable register Enable SPI1
#define RPI_AUX_ENABLE_SPI1           BIT(1)
/// @brief Raspberry Pi Auxiliary Enable register Enable UART1
#define RPI_AUX_ENABLE_UART1          BIT(0)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable transmit interrupts
#define RPI_AUX_MU_IER_TX_IRQ_ENABLE  BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Enable register enable receive interrupts
#define RPI_AUX_MU_IER_RX_IRQ_ENABLE  BIT(0)

/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register values
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO enabled (R)
#define RPI_AUX_MU_IIR_TX_FIFO_ENABLE BIT(7)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO enabled (R)
#define RPI_AUX_MU_IIR_RX_FIFO_ENABLE BIT(6)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register transmit FIFO clear (W)
#define RPI_AUX_MU_IIR_TX_FIFO_CLEAR  BIT(2)
/// @brief Raspberry Pi Mini UART (UART1) Interrupt Identify register receive FIFO clear (W)
#define RPI_AUX_MU_IIR_RX_FIFO_CLEAR  BIT(1)

/// @brief Raspberry Pi Mini UART (UART1) Line Control register values
/// @brief Raspberry Pi Mini UART (UART1) Line Control register 7 bit characters
#define RPI_AUX_MU_LCR_DATA_SIZE_7    0
/// @brief Raspberry Pi Mini UART (UART1) Line Control register 8 bit characters
#define RPI_AUX_MU_LCR_DATA_SIZE_8    BIT(0) | BIT(1)

/// @brief Raspberry Pi Mini UART (UART1) Modem Control register values
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS low
#define RPI_AUX_MU_MCR_RTS_LOW        BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Modem Control register set RTS high
#define RPI_AUX_MU_MCR_RTS_HIGH       0

/// @brief Raspberry Pi Mini UART (UART1) Line Status register values
/// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit idle
#define RPI_AUX_MU_LST_TX_IDLE        BIT(6)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register transmit empty
#define RPI_AUX_MU_LST_TX_EMPTY       BIT(5)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register receive overrun
#define RPI_AUX_MU_LST_RX_OVERRUN     BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Line Status register receive ready
#define RPI_AUX_MU_LST_RX_READY       BIT(0)

/// @brief Raspberry Pi Mini UART (UART1) Extra Control register values
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable CTS
#define RPI_AUX_MU_CNTL_ENABLE_CTS    BIT(3)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable RTS
#define RPI_AUX_MU_CNTL_ENABLE_RTS    BIT(2)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable transmit
#define RPI_AUX_MU_CNTL_ENABLE_TX     BIT(1)
/// @brief Raspberry Pi Mini UART (UART1) Extra Control register enable receive
#define RPI_AUX_MU_CNTL_ENABLE_RX     BIT(0)

```

This header declares a number of memory addresses, more specifically for the GPIO pins control and for UART1. More will be added later.

### Macros.h

We add some standard definitions. Add the file `code/libraries/baremetal/include/baremetal/Macros.h`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : Macros.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Common defines
//
//------------------------------------------------------------------------------

#pragma once

/// @file
/// Generic macros

/// @defgroup Macros
/// @{

/// @brief Convert bit index into integer
/// @param n Bit index
#define BIT(n)              (1U << (n))

/// @}
```

This header defines a number of standard macros. More will be added later.

### IGPIOPin.h

We add an abstract GPIO pin interface class `IGPIOPin`. Add the file `code/libraries/baremetal/include/baremetal/IGPIOPin.h`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : IGPIOPin.h
//
// Namespace   : baremetal
//
// Class       : IGPIOPin
//
// Description : GPIO pin abstraction
//
//------------------------------------------------------------------------------

#pragma once

#include <baremetal/Types.h>

/// @file
/// Abstract GPIO pin. Could be either a virtual or physical pin

namespace baremetal {

/// @brief GPIO mode
enum class GPIOMode
{
    /// @brief GPIO used as input
    Input,
    /// @brief GPIO used as output
    Output,
    /// @brief GPIO used as input, using pull-up
    InputPullUp,
    /// @brief GPIO used as input, using pull-down
    InputPullDown,
    /// @brief GPIO used as Alternate Function 0. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction0,
    /// @brief GPIO used as Alternate Function 1. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction1,
    /// @brief GPIO used as Alternate Function 2. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction2,
    /// @brief GPIO used as Alternate Function 3. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction3,
    /// @brief GPIO used as Alternate Function 4. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction4,
    /// @brief GPIO used as Alternate Function 5. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction5,
    Unknown = 10,
};

/// @brief GPIO function
enum class GPIOFunction
{
    /// @brief GPIO used as input
    Input,
    /// @brief GPIO used as output
    Output,
    /// @brief GPIO used as Alternate Function 0. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction0,
    /// @brief GPIO used as Alternate Function 1. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction1,
    /// @brief GPIO used as Alternate Function 2. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction2,
    /// @brief GPIO used as Alternate Function 3. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction3,
    /// @brief GPIO used as Alternate Function 4. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction4,
    /// @brief GPIO used as Alternate Function 5. See \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    AlternateFunction5,
    Unknown = 8,
};

/// @brief GPIO pull mode
enum class GPIOPullMode
{
    /// @brief GPIO pull mode off (no pull-up or pull-down). See \ref BCM_GPIO_PULL_UP_DOWN
    Off,
    /// @brief GPIO pull mode pull-down. See \ref BCM_GPIO_PULL_UP_DOWN
    PullDown,
    /// @brief GPIO pull mode pull-up. See \ref BCM_GPIO_PULL_UP_DOWN
    PullUp,
    /// @brief GPIO pull mode reserved. See \ref BCM_GPIO_PULL_UP_DOWN
    Unknown,
};

/// @brief Abstraction of a GPIO pin
class IGPIOPin
{
public:
    virtual ~IGPIOPin() = default;

    /// @brief Return pin number (high bit = 0 for a phsical pin, 1 for a virtual pin)
    /// @return Pin number
    virtual uint8 GetPinNumber() const = 0;
    /// @brief Assign a GPIO pin
    /// @param pin      Pin number
    /// @return true if successful, false otherwise
    virtual bool AssignPin(uint8 pinNumber) = 0;

    virtual bool AutoAckInterrupt() const = 0;

    virtual void AutoAcknowledgeInterrupt() = 0;

    virtual void InterruptHandler() = 0;

    /// @brief Switch GPIO on
    virtual void On() = 0;
    /// @brief Switch GPIO off
    virtual void Off() = 0;
    /// @brief Get GPIO value
    virtual bool Get() = 0;
    /// @brief Set GPIO on (true) or off (false)
    virtual void Set(bool on) = 0;
    /// @brief Invert GPIO value on->off off->on
    virtual void Invert() = 0;
};

class String;

} // namespace baremetal

namespace serialization {

baremetal::String Serialize(const baremetal::GPIOMode &value);
baremetal::String Serialize(const baremetal::GPIOFunction &value);
baremetal::String Serialize(const baremetal::GPIOPullMode &value);

} // namespace serialization
```

This header defines a number of standard macros. More will be added later.


### PhysicalGPIOPin.h

We add a class to handle GPIO pins. Add the file `code/libraries/baremetal/include/baremetal/Macros.h`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : PhysicalGPIOPin.h
//
// Namespace   : baremetal
//
// Class       : PhysicalGPIOPin
//
// Description : Physical GPIO pin
//
//------------------------------------------------------------------------------

#pragma once

//#include <baremetal/GPIOControl.h>
#include <baremetal/IGPIOPin.h>
#include <baremetal/MemoryAccess.h>

/// @file
/// Physical GPIO pin

namespace baremetal {

enum class GPIOInterrupt
{
    RisingEdge,
    FallingEdge,
    HighLevel,
    LowLevel,
    AsyncRisingEdge,
    OnAsyncFallingEdge,
    Unknown
};

using GPIOInterruptHandler = void(void *param);

/// @brief Physical GPIO pin (i.e. available on GPIO header)
class PhysicalGPIOPin : public IGPIOPin
{
private:
    uint8                 m_pinNumber;
    unsigned              m_regOffset;
    uint32                m_regMask;
    GPIOMode              m_mode;
    GPIOFunction          m_function;
    GPIOPullMode          m_pullMode;
    bool                  m_value;

    GPIOInterruptHandler *m_handler;
    void                 *m_param;
    bool                  m_autoAck;
    GPIOInterrupt         m_interrupt;
    GPIOInterrupt         m_interrupt2;
    IMemoryAccess        &m_memoryAccess;

public:
    /// @brief Creates a virtual GPIO pin
    PhysicalGPIOPin(IMemoryAccess &memoryAccess = GetMemoryAccess());
    // PhysicalGPIOPin(const PhysicalGPIOPin &other);

    /// @brief Creates a virtual GPIO pin
    PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess &memoryAccess = GetMemoryAccess());

    uint8 GetPinNumber() const override;
    /// @brief Assign a GPIO pin
    /// @param pin      Pin number
    /// @return true if successful, false otherwise
    bool AssignPin(uint8 pinNumber) override;
    /// @brief Get the mode for the GPIO pin
    /// @return mode GPIO mode. See \ref GPIOMode
    GPIOMode GetMode();
    /// @brief Set the mode for the GPIO pin
    /// @param mode GPIO mode to be selected. See \ref GPIOMode
    /// @return true if successful, false otherwise
    bool SetMode(GPIOMode mode);
    /// @brief Get GPIO pin function
    /// @return GPIO pin function used. See \ref GPIOFunction
    GPIOFunction GetFunction();
    /// @brief Get GPIO pin pull mode
    /// @return GPIO pull mode used. See \ref GPIOPullMode
    GPIOPullMode GetPullMode();
    /// @brief Set GPIO pin pull mode
    /// @param pullMode GPIO pull mode to be used. See \ref GPIOPullMode
    void SetPullMode(GPIOPullMode pullMode);

    void ConnectInterrupt(GPIOInterruptHandler *handler, void *param, bool autoAck = true);
    void DisconnectInterrupt();
    bool AutoAckInterrupt() const override;

    void EnableInterrupt(GPIOInterrupt interrupt);
    void DisableInterrupt();

    void EnableInterrupt2(GPIOInterrupt interrupt);
    void DisableInterrupt2();

    void AcknowledgeInterrupt();
    void AutoAcknowledgeInterrupt() override;

    void InterruptHandler() override;
    void DisableAllInterrupts();

    /// @brief Switch GPIO on
    void On() override;
    /// @brief Switch GPIO off
    void Off() override;
    /// @brief Get GPIO value
    bool Get() override;
    /// @brief Set GPIO on (true) or off (false)
    void Set(bool on) override;
    /// @brief Invert GPIO value on->off off->on
    void Invert() override;

private:
    /// @brief Set GPIO pin function
    /// @param function GPIO function to be selected. See \ref GPIOFunction, \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
    void SetFunction(GPIOFunction function);
};

} // namespace baremetal
```

This header defines a number of standard macros. More will be added later.

### MemoryAccess.cpp

We need to implement `MemoryAccess`. Add the file `code/libraries/baremetal/src/MemoryAccess.cpp`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : MemoryAccess.cpp
//
// Namespace   : baremetal
//
// Class       : MemoryAccess
//
// Description : Memory read/write
//
//------------------------------------------------------------------------------

#include <baremetal/MemoryAccess.h>

using namespace baremetal;

uint8 MemoryAccess::Read8(uintptr address)
{
    return *reinterpret_cast<uint8 volatile *>(address);
}

void MemoryAccess::Write8(uintptr address, uint8 data)
{
    *reinterpret_cast<uint8 volatile *>(address) = data;
}

void MemoryAccess::ReadModifyWrite8(uintptr address, uint8 mask, uint8 data, uint8 shift)
{
    auto value = Read8(address);
    value &= mask;
    value |= ((data << shift) & ~mask);
    Write8(address, value);
}

uint16 MemoryAccess::Read16(uintptr address)
{
    return *reinterpret_cast<uint16 volatile *>(address);
}

void MemoryAccess::Write16(uintptr address, uint16 data)
{
    *reinterpret_cast<uint16 volatile *>(address) = data;
}

void MemoryAccess::ReadModifyWrite16(uintptr address, uint16 mask, uint16 data, uint8 shift)
{
    auto value = Read16(address);
    value &= mask;
    value |= ((data << shift) & ~mask);
    Write16(address, value);
}

uint32 MemoryAccess::Read32(uintptr address)
{
    return *reinterpret_cast<uint32 volatile *>(address);
}

void MemoryAccess::Write32(uintptr address, uint32 data)
{
    *reinterpret_cast<uint32 volatile *>(address) = data;
}

void MemoryAccess::ReadModifyWrite32(uintptr address, uint32 mask, uint32 data, uint8 shift)
{
    auto value = Read32(address);
    value &= mask;
    value |= ((data << shift) & ~mask);
    Write32(address, value);
}
```

### UART1.cpp

Finally we need to implement `UART1`. Add the file `code/libraries/baremetal/src/UART1.cpp`:

```cpp
//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : UART1.cpp
//
// Namespace   : baremetal
//
// Class       : UART1
//
// Description : RPI UART1 class
//
//------------------------------------------------------------------------------

#include <baremetal/UART1.h>

namespace baremetal {

UART1::UART1(IMemoryAccess &memoryAccess)
    : m_initialized{}
    , m_memoryAccess{memoryAccess}
{
}

// Set baud rate and characteristics (115200 8N1) and map to GPIO
void UART1::Initialize()
{
    if (m_initialized)
        return;

    // initialize UART

    // TODO
    m_initialized = true;
}

// Write a character

void UART1::Write(char c)
{
    // TODO
}

// Receive a character

char UART1::Read()
{
    // TODO
    return {};
}

void UART1::WriteString(const char *str)
{
    while (*str)
    {
        // convert newline to carriage return + newline
        if (*str == '\n')
            Write('\r');
        Write(*str++);
    }
}

} // namespace baremetal
```

You will notice that we did not actually implement the UART1 functionality yet. For that we will need some more code, we will get to that shortly.
