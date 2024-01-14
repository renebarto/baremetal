# Tutorial 04: Setting up common options {#TUTORIAL_04}

Contents:
- [CMake function display_list](##CMake-function-display_list)
- [CMake function show_target_properties](##CMake-function-show_target_properties)
- [Common compiler directives](##Common-compiler-directives)
  - [Common definitions and options](###Common-definitions-and-options)
  - [Combining common definitions and options](###Combining-common-definitions-and-options)
  - [Adding some variables](###Adding-some-variables)
- [Update default variables in CMakeSettings.json](##Update-default-variables-in-CMakeSettings.json)
- [Update demo CMake file](##Update-demo-CMake-file)
- [Update demo-image CMake file](##Updatedemo-image-CMake-file)
- [Configure and build](##Configure-and-build)
  - [Compiling startup code](###Compiling-startup-code)
  - [Compiling source code](###Compiling-source-code)
  - [Linking the demo application](###Linking-the-demo-application)
  - [Creating the kernel image](###Creating-the-kernel-image)

In order for CMake projects to work smoothly, it is convenient to do some groundwork for them.

This will again be a length story, as we will set up how we handle setting of compiler definitions and options, linker options, etc.

If you're curious to see how this works, or just want to dive directly into the code,
in `tutorial/04-setting-up-common-options` there is a complete copy of what we work towards in this section.
Its root will clearly be `tutorial/04-setting-up-common-options`.
Please be aware of this when e.g. debugging, the paths in vs.launch.json may not match your specific case.

## CMake function display_list

We will add a function to the `cmake/functions.cmake` file:

```cmake
File: cmake/functions.cmake
...

9: # Converts a CMake list to a CMake string and displays this in a status message along with the text specified.
10: function(display_list text)
11:     set(list_str "")
12:     foreach(item ${ARGN})
13:         string(CONCAT list_str "${list_str} ${item}")
14:     endforeach()
15:     message(STATUS ${text} ${list_str})
16: endfunction()
17: 

...
```

This function prints a text followed by a list of values, separated by spaces.
This can be used for single values, but also for a list of e.g. compiler definitions.

## CMake function show_target_properties

We will add a another function to the `cmake/functions.cmake` file:

```cmake
File: cmake/functions.cmake
...

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
96: 

...
```

Without going into too much detail, this function prints for a specific target:

- Line 23-24: the type of target (executable, library, interface library)
- Line 26-27: compiler definitions
- Line 28-29: compiler options
- Line 32-33: interface include directories
- Line 36-37: include directories
- Line 39-40: standard libraries to be linked to
- Line 42-43: linker options
- Line 46-47: interface library compiler definitions
- Line 49-50: interface library compiler options
- Line 52-53: interface library include directories
- Line 55-56: interface library standard libraries to be linked to
- Line 58-59: imported dependencies
- Line 61-62: imported libraries to be linked to
- Line 65-69: dependencies
- Line 73-80: output directories for static libraries, shared libraries, and executables
- Line 82-83: linker options
- Line 85-86: the target version
- Line 88-89: the target so version in case of a shared library
- Line 91-92: the target output name

## Common compiler directives

As most compiler definitions and compiler options will have some commonality, it is convenient to define these common parts in variables,
and use them in our project configuration.

So we will update the main CMake file to set these variables. We will revisit this part a number of times.

Just before `add_subdirectory(code)` in the main CMake file, insert the following:

### Common definitions and options

```cmake
File: CMakeLists.txt
...

31: if (BAREMETAL_TARGET STREQUAL "RPI3")
32:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
33:     set(BAREMETAL_TARGET_KERNEL kernel8)
34:     set(BAREMETAL_RPI_TARGET 3)
35: elseif (BAREMETAL_TARGET STREQUAL "RPI4")
36:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a72 -mlittle-endian -mcmodel=small)
37:     set(BAREMETAL_TARGET_KERNEL kernel8-rpi4)
38:     set(BAREMETAL_RPI_TARGET 4)
39: endif()
40: set(BAREMETAL_LOAD_ADDRESS 0x80000)
41: 
42: set(DEFINES_C
43:     PLATFORM_BAREMETAL
44:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
45:     )
46: set(DEFINES_C_DEBUG _DEBUG)
47: set(DEFINES_C_RELEASE NDEBUG)
48: set(DEFINES_C_MINSIZEREL NDEBUG)
49: set(DEFINES_C_RELWITHDEBINFO NDEBUG)
50: set(DEFINES_ASM
51:     PLATFORM_BAREMETAL
52:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
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

...
```

Explanation:
- Line 31-39: Depending on the variable `BAREMETAL_TARGET` which we will define later on, we define the variables `BAREMETAL_ARCH_CPU_OPTIONS` and `BAREMETAL_TARGET_KERNEL`, which define specific compiler options for the platform we're building for (for now Raspberry Pi 3 or 4), and the base name of the kernel image file.
Also, if `BAREMETAL_TARGET` is set to "RPI3", we set `BAREMETAL_RPI_TARGET` to 3, and for `BAREMETAL_TARGET` equal "RPI4", we set `BAREMETAL_RPI_TARGET` to 4.
- Line 40: We define the variable `BAREMETAL_LOAD_ADDRESS` to hold the start address for the baremetal application
- Line 42-45: We define the variable `DEFINES_C` to hold the generic compiler definitions for C and C++, independent of the build configuration
- Line 46-49: We define the variables `DEFINES_C_DEBUG`, `DEFINES_C_RELEASE`, `DEFINES_C_MINSIZEREL` and `DEFINES_C_RELWITHDEBINFO` to hold specific compiler definitions for C and C++ per build configuration
- Line 50-53: We define the variable `DEFINES_ASM` to hold the generic compiler definitions for assembly, independent of the build configuration
- Line 55-68: We define the variable `FLAGS_C` to hold the generic compiler options for C, independent of the build configuration
- Line 69-76: We define the variables `FLAGS_C_DEBUG`, `FLAGS_C_RELEASE`, `FLAGS_C_MINSIZEREL` and `FLAGS_C_RELWITHDEBINFO` to hold specific compiler options for C per build configuration
- Line 78-95: We define the variable `FLAGS_CXX` to hold the generic compiler options for C++, independent of the build configuration
- Line 97-104: We define the variables `FLAGS_CXX_DEBUG`, `FLAGS_CXX_RELEASE`, `FLAGS_CXX_MINSIZEREL` and `FLAGS_CXX_RELWITHDEBINFO` to hold specific compiler options for C++ per build configuration
- Line 106: We define the variable `FLAGS_ASM` to hold the generic compiler options for assembly, independent of the build configuration
- Line 107-110: We define the variables `FLAGS_ASM_DEBUG`, `FLAGS_ASM_RELEASE`, `FLAGS_ASM_MINSIZEREL` and `FLAGS_ASM_RELWITHDEBINFO` to hold specific compiler options for assembly per build configuration
- Line 112-117: We define the variable `LINK_FLAGS` to hold the linker options
- Line 118-121: We define the variables `LINK_FLAGS_DEBUG`, `LINK_FLAGS_RELEASE`, `LINK_FLAGS_MINSIZEREL` and `LINK_FLAGS_RELWITHDEBINFO` to hold specific linker options per build configuration
- Line 123: We add to variable `LINK_LIBRARIES` holding the generic list of libraries to link to (empty for now)
- Line 124: We add to variable `LINK_DIRECTORIES` holding the generic list of directories to search for linking (empty for now)
- Line 126: We define the variable `SUPPORTED_CPP_STANDARD` to holding the C++ standard used
- Line 128-130: We print the current C++ and C compiler versions, as well as the C++ standard version just defined

So the section above adds a lot of variables, to set compiler and linker definitions and options, common and for each different build configuration.

### Combining common definitions and options

We will now combine the variables definined in the previous section to one for each build configuration.
Then we create a specific version for the currently set build configuration.
For this we add another section just before `add_subdirectory(code)` in the main CMake file:

```cmake
File: CMakeLists.txt
...

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

...
```

Explanation:
- Line 132-135: We combine the generic compiler definitions for C and C++ with the configuration specific ones in variables:
  - `COMPILE_DEFINITIONS_C_DEBUG`
  - `COMPILE_DEFINITIONS_C_RELEASE`
  - `COMPILE_DEFINITIONS_C_MINSIZEREL`
  - `COMPILE_DEFINITIONS_C_RELWITHDEBINFO`
- Line 137-140: We combine the generic compiler definitions for assembly with the configuration specific ones in variables:
  - `COMPILE_DEFINITIONS_ASM_DEBUG`
  - `COMPILE_DEFINITIONS_ASM_RELEASE`
  - `COMPILE_DEFINITIONS_ASM_MINSIZEREL`
  - `COMPILE_DEFINITIONS_ASM_RELWITHDEBINFO`
- Line 142-145: We combine the generic compiler options for C with the configuration specific ones in variables:
  - `COMPILE_OPTIONS_C_DEBUG`
  - `COMPILE_OPTIONS_C_RELEASE`
  - `COMPILE_OPTIONS_C_MINSIZEREL`
  - `COMPILE_OPTIONS_C_RELWITHDEBINFO`
- Line 147-150: We combine the generic compiler options for C++ with the configuration specific ones in variables:
  - `COMPILE_OPTIONS_CXX_DEBUG`
  - `COMPILE_OPTIONS_CXX_RELEASE`
  - `COMPILE_OPTIONS_CXX_MINSIZEREL`
  - `COMPILE_OPTIONS_CXX_RELWITHDEBINFO`
- Line 152-155: We combine the generic compiler options for assembly with the configuration specific ones in variables:
  - `COMPILE_OPTIONS_ASM_DEBUG`
  - `COMPILE_OPTIONS_ASM_RELEASE`
  - `COMPILE_OPTIONS_ASM_MINSIZEREL`
  - `COMPILE_OPTIONS_ASM_RELWITHDEBINFO`
- Line 157-160: We combine the generic linker options with the configuration specific ones in variables:
  - `LINKER_OPTIONS_DEBUG`
  - `LINKER_OPTIONS_RELEASE`
  - `LINKER_OPTIONS_MINSIZEREL`
  - `LINKER_OPTIONS_RELWITHDEBINFO`
- Line 162: We set the variable `LINKER_LIBRARIES` to hold the general libraries to link to
- Line 164-194: Depending on the current build configuration, we define the following variables to the correct ones for the build configuration set:
  - `COMPILE_DEFINITIONS_C`
  - `COMPILE_DEFINITIONS_ASM`
  - `COMPILE_OPTIONS_C`
  - `COMPILE_OPTIONS_CXX`
  - `COMPILE_OPTIONS_ASM`
  - `LINKER_OPTIONS`

### Adding some variables

We need to set a default value for the platform `BAREMETAL_TARGET`, and we add a variable to be used to set verbose build output.
Just before the main project definition insert the following:

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
33: if("${BAREMETAL_TARGET}" STREQUAL "")
34:     set(BAREMETAL_TARGET "RPI3") # Board type (RPI3/RPI4)
35: endif()
36: 
37: if (BAREMETAL_TARGET STREQUAL "RPI3")
38:     message(STATUS "Building for Raspberry Pi 3")
39: elseif (BAREMETAL_TARGET STREQUAL "RPI4")
40:     message(STATUS "Building for Raspberry Pi 4")
41: else()
42:     message(FATAL_ERROR "Incorrect target ${BAREMETAL_TARGET} specified, must be RPI3 or RPI4")
43: endif()
44: 
45: message(STATUS "\n** Setting up project **\n--")
46: 
47: message(STATUS "\n##################################################################################")
48: message(STATUS "\n** Setting up toolchain **\n--")

...
```

Explanation:
- Line 23: We define a variable `VERBOSE_BUILD` to enable or disable verbose build output. The default is ON
- Line 25-29: Depending on the value of `VERBOSE_BUILD`, we set the standard CMake variable `CMAKE_VERBOSE_MAKEFILE` to ON or OFF. This is placed in the CMake cache
- Line 30: We set the standard CMake variable `CMAKE_EXPORT_COMPILE_COMMANDS` to ON. This will make CMake generate a JSON file `compile_commands.json` in the CMake build directory, that can be used by Visual Studio and other tools.
This can come in handy when trying to figure out which source is compiler with which options
- Line 31: We set the standard CMake variable `CMAKE_COLOR_MAKEFILE` to ON. This will generate CMake configuration and build output with ANSI color output. This will not work inside Visual Studio , but it will output in color on the command line
- Line 33-35: Finally, we set the variable `BAREMETAL_TARGET`, if not set already, to 3, to make it default to Raspberry Pi 3.
- Line 37-43: We check the variable `BAREMETAL_TARGET`, and print the platform we're building for.

## Update default variables in CMakeSettings.json

In order to get more verbose output from CMake, and to set the correct target platform (we will select RaspBerry Pi 3 here), we will need to update `CMakeSettings.json`.
We will add a duplicate set of build configurations, one for Raspberry Pi 3, one for Raspberry Pi 4.
We will set the variable `VERBOSE_BUILD` to ON, and `BAREMETAL_TARGET` to 3 or 4, depending on the configuration.

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
10:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3",
11:       "buildCommandArgs": "",
12:       "ctestCommandArgs": "",
13:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
14:       "inheritEnvironments": [ "gcc-arm" ]
15:     },
16:     {
17:       "name": "BareMetal-RPI4-Debug",
18:       "generator": "Ninja",
19:       "configurationType": "Debug",
20:       "buildRoot": "${projectDir}\\cmake-${name}",
21:       "installRoot": "${projectDir}\\output\\install\\${name}",
22:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4",
23:       "buildCommandArgs": "",
24:       "ctestCommandArgs": "",
25:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
26:       "inheritEnvironments": [ "gcc-arm" ]
27:     },
28:     {
29:       "name": "BareMetal-Release",
30:       "generator": "Ninja",
31:       "configurationType": "Release",
32:       "buildRoot": "${projectDir}\\cmake-${name}",
33:       "installRoot": "${projectDir}\\output\\install\\${name}",
34:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3",
35:       "buildCommandArgs": "",
36:       "ctestCommandArgs": "",
37:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
38:       "inheritEnvironments": [ "gcc-arm" ]
39:     },
40:     {
41:       "name": "BareMetal-RPI4-Release",
42:       "generator": "Ninja",
43:       "configurationType": "Release",
44:       "buildRoot": "${projectDir}\\cmake-${name}",
45:       "installRoot": "${projectDir}\\output\\install\\${name}",
46:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4",
47:       "buildCommandArgs": "",
48:       "ctestCommandArgs": "",
49:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
50:       "inheritEnvironments": [ "gcc-arm" ]
51:     },
52:     {
53:       "name": "BareMetal-RelWithDebInfo",
54:       "generator": "Ninja",
55:       "configurationType": "RelWithDebInfo",
56:       "buildRoot": "${projectDir}\\cmake-${name}",
57:       "installRoot": "${projectDir}\\output\\install\\${name}",
58:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3",
59:       "buildCommandArgs": "",
60:       "ctestCommandArgs": "",
61:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
62:       "inheritEnvironments": [ "gcc-arm" ]
63:     },
64:     {
65:       "name": "BareMetal-RPI4-RelWithDebInfo",
66:       "generator": "Ninja",
67:       "configurationType": "RelWithDebInfo",
68:       "buildRoot": "${projectDir}\\cmake-${name}",
69:       "installRoot": "${projectDir}\\output\\install\\${name}",
70:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4",
71:       "buildCommandArgs": "",
72:       "ctestCommandArgs": "",
73:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
74:       "inheritEnvironments": [ "gcc-arm" ]
75:     },
76:     {
77:       "name": "BareMetal-MinSizeRel",
78:       "generator": "Ninja",
79:       "configurationType": "MinSizeRel",
80:       "buildRoot": "${projectDir}\\cmake-${name}",
81:       "installRoot": "${projectDir}\\output\\install\\${name}",
82:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3",
83:       "buildCommandArgs": "",
84:       "ctestCommandArgs": "",
85:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
86:       "inheritEnvironments": [ "gcc-arm" ]
87:     },
88:     {
89:       "name": "BareMetal-RPI4-MinSizeRel",
90:       "generator": "Ninja",
91:       "configurationType": "MinSizeRel",
92:       "buildRoot": "${projectDir}\\cmake-${name}",
93:       "installRoot": "${projectDir}\\output\\install\\${name}",
94:       "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4",
95:       "buildCommandArgs": "",
96:       "ctestCommandArgs": "",
97:       "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
98:       "inheritEnvironments": [ "gcc-arm" ]
99:     }
100:   ]
101: }
```

## Update demo CMake file

With the addition the new CMake custom functions, and the addition of the set of variables we just defined, we can extend and simplify the demo application CMake file a bit:

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
26:     )
27: 
28: set(PROJECT_LIBS
29:     ${LINKER_LIBRARIES}
30:     ${PROJECT_DEPENDENCIES}
31:     )
32: 
33: set(PROJECT_SOURCES
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/start.S
36:     )
37: 
38: set(PROJECT_INCLUDES_PUBLIC )
39: set(PROJECT_INCLUDES_PRIVATE )
40: 
41: if (CMAKE_VERBOSE_MAKEFILE)
42:     display_list("Package                           : " ${PROJECT_NAME} )
43:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
44:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
45:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
46:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
47:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
48:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
49:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
50:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
51:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
52:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
53:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
54:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
55:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
56:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
57:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
58:     display_list("Link libs                         : " ${PROJECT_LIBS} )
59:     display_list("Source files                      : " ${PROJECT_SOURCES} )
60:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
61:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
62: endif()
63: 
64: if (PLATFORM_BAREMETAL)
65:     set(START_GROUP -Wl,--start-group)
66:     set(END_GROUP -Wl,--end-group)
67: endif()
68: 
69: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
70: 
71: target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
72: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
73: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
74: target_compile_definitions(${PROJECT_NAME} PRIVATE 
75:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
76:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
77:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
78:     )
79: target_compile_definitions(${PROJECT_NAME} PUBLIC 
80:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
81:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
82:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
83:     )
84: target_compile_options(${PROJECT_NAME} PRIVATE 
85:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
86:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
87:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
88:     )
89: target_compile_options(${PROJECT_NAME} PUBLIC 
90:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
91:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
92:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
93:     )
94: 
95: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
96: 
97: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
98: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
99:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
100: endif()
101: 
102: link_directories(${LINK_DIRECTORIES})
103: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
104: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
105: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
106: 
107: show_target_properties(${PROJECT_NAME})
108: 
109: add_subdirectory(create-image)
```

- Line 14: We can now define the compiler definitions for C++ using the variables set up for C/C++ compiler definitions
- Line 16: Similarly for the compiler definitions for assembly using the variables set up for assembly compiler definitions
- Line 17: Similarly for the compiler options for C++ using the variables set up for C++ compiler options
- Line 19: Similarly for the compiler options for assembly using the variables set up for assembly compiler options
- Line 23: Similarly for linker options using the variables set up for linker options
- Line 28-31: We can now link to all standard libraries by adding `LINKER_LIBRARIES` to `PROJECT_LIBS`
- Line 41-62: Using the custom CMake function `display_list` we can now print all settings for the project.
This is only done if verbose output is requested
- Line 74-78: The variables `PROJECT_COMPILE_DEFINITIONS_C_PRIVATE`, `PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE` and `PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE` are used to set private compiler definitions for a specific language, in this case C, C++ or assembly. We use a so-called generator expression for this
- Line 79-83: Similarly for setting public compiler definitions for a specific language
- Line 84-88: Similarly for setting private compiler options for a specific language
- Line 89-93: Similarly for setting public compiler options for a specific language
- Line 95: We can now use the `SUPPORTED_CPP_STANDARD` to set the C++ standard property on the target
- Line 102: We set the search path for standard libraries using `LINK_DIRECTORIES`
- Line 107: Using the custom CMake function `show_target_properties` we can now print all target properties.

## Update demo-image CMake file

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
```

The only difference here is that the `BAREMETAL_TARGET_KERNEL` is defined in the main CMake file, so we can remove the definition here.

## Configure and build

We are now able to configure the project again, and build it.

The output for the configure step should be similar to:

```text
1> CMake generation started for configuration: 'BareMetal-Debug'.
1> Command line: "C:\Windows\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="D:\Projects\baremetal.github\output\install\BareMetal-Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH="D:\Projects\baremetal.github\baremetal.toolchain" -DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DCMAKE_MAKE_PROGRAM="C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" "D:\Projects\baremetal.github" 2>&1"
1> Working directory: D:\Projects\baremetal.github\cmake-BareMetal-Debug
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
1> [CMake] -- Defines C++ - private             :  PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3 _DEBUG
1> [CMake] -- Defines ASM - private             :  PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3
1> [CMake] -- Compiler options C - public       : 
1> [CMake] -- Compiler options C - private      : 
1> [CMake] -- Compiler options C++ - public     : 
1> [CMake] -- Compiler options C++ - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter
1> [CMake] -- Compiler options ASM - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2
1> [CMake] -- Include dirs - public             : 
1> [CMake] -- Include dirs - private            : 
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      : 
1> [CMake] -- Link libs                         : 
1> [CMake] -- Source files                      :  D:/Projects/baremetal.github/code/applications/demo/src/main.cpp D:/Projects/baremetal.github/code/applications/demo/src/start.S
1> [CMake] -- Include files - public            : 
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] -- Properties for demo
1> [CMake] -- Target type                       :  EXECUTABLE
1> [CMake] -- Target defines                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3 _DEBUG> $<$<COMPILE_LANGUAGE:ASM>:PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target options                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target include dirs public        :  INCLUDES-NOTFOUND
1> [CMake] -- Target include dirs private       :  INCLUDES-NOTFOUND
1> [CMake] -- Target link libraries             :  -Wl,--start-group -Wl,--end-group
1> [CMake] -- Target link options               :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target exported defines           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported options           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported include dirs      :  INCLUDE_DIRS_EXPORTS-NOTFOUND
1> [CMake] -- Target exported link libraries    :  -Wl,--start-group -Wl,--end-group
1> [CMake] -- Target imported dependencies      : 
1> [CMake] -- Target imported link libraries    : 
1> [CMake] -- Target link dependencies          :  LINK_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target manual dependencies        :  EXPLICIT_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target static library location    :  D:/Projects/baremetal.github/output/Debug/lib
1> [CMake] -- Target dynamic library location   :  LIBRARY_LOCATION-NOTFOUND
1> [CMake] -- Target binary location            :  D:/Projects/baremetal.github/output/Debug/bin
1> [CMake] -- Target link flags                 :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target version                    :  TARGET_VERSION-NOTFOUND
1> [CMake] -- Target so-version                 :  TARGET_SOVERSION-NOTFOUND
1> [CMake] -- Target output name                :  demo.elf
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

We can then build for Raspberry Pi 3:

```text
>------ Rebuild All started: Project: baremetal, Configuration: BareMetal-Debug ------
  [1/1] "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"  -t clean 
  Cleaning... 2 files.
  [1/4] D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DPLATFORM_BAREMETAL -DBAREMETAL_TARGET=RPI3  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -c ../code/applications/demo/src/start.S
  [2/4] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DPLATFORM_BAREMETAL -DBAREMETAL_TARGET=RPI3 -D_DEBUG  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [3/4] cmd.exe /C "cd . && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -o ..\output\Debug\bin\demo.elf  -Wl,--start-group  -Wl,--end-group && cd ."
  [4/4] cmd.exe /C "cd /D D:\Projects\baremetal.test\cmake-BareMetal-Debug\code\applications\demo\create-image && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal.github/output/Debug/bin/demo.elf -O binary D:/Projects/baremetal.github/deploy/Debug/demo-image/kernel8.img"

Rebuild All succeeded.
```

Or Raspberry Pi 4:

```text
>------ Rebuild All started: Project: 04-setting-up-common-options, Configuration: BareMetal-RPI4-Debug ------
  [1/1] "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"  -t clean 
  Cleaning... 4 files.
  [1/4] D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_TARGET=RPI4 -DPLATFORM_BAREMETAL  -g -mcpu=cortex-a72 -mlittle-endian -mcmodel=small -O2 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -c ../code/applications/demo/src/start.S
  [2/4] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_TARGET=RPI4 -DPLATFORM_BAREMETAL -D_DEBUG  -g -mcpu=cortex-a72 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [3/4] cmd.exe /C "cd . && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -o ..\output\Debug\bin\demo.elf  -Wl,--start-group  -Wl,--end-group && cd ."
  [4/4] cmd.exe /C "cd /D D:\Projects\baremetal.github\cmake-BareMetal-RPI4-Debug\code\applications\demo\create-image && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal.github/output/Debug/bin/demo.elf -O binary D:/Projects/baremetal.github/deploy/Debug/demo-image/kernel8-rpi4.img"

Rebuild All succeeded.
```

You can see that the startup code is compiled (step 1), as well as the demo application's main.cpp (step 2).
The demo application is linked (step 3), and finally the image is created (step 4).
It takes a while to start understanding, even with the description of [Common compiler directives](##Common-compiler-directives),  what the compiler and linker actually do, so we'll dive a bit deeper here.
For this we'll have a look at the build output shown above:

### Compiling startup code

When compiling `code/applications/demo/src/start.S`
```text
D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe
  -DPLATFORM_BAREMETAL -DBAREMETAL_TARGET=RPI3
  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
  -O2 
  -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d
  -o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
  -c ../code/applications/demo/src/start.S
```

#### Compiler

The first part is the compiler call, the rest are command line parameters for the compiler.
For the startup code (which is assembly), this is `aarch64-none-elf-gcc.exe`.

#### Compiler definitions

The -D options specify all compiler definitions. You will recognize the options specified in CMakeSettings.json: `"cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3"`.
- The first option triggers setting the `BAREMETAL_TARGET=RPI3` definition.
- The `PLATFORM_BAREMETAL` definition is triggered by the main CMake file:

```cmake
File: CMakeLists.txt
72: set(DEFINES_ASM
73:     PLATFORM_BAREMETAL
74:     BAREMETAL_TARGET=${BAREMETAL_TARGET}
75:     )
```

- The generic and the build configuration specific definitions are later combined:
```cmake
File: CMakeLists.txt
159: set(COMPILE_DEFINITIONS_ASM_DEBUG ${DEFINES_ASM} ${DEFINES_ASM_DEBUG})
160: set(COMPILE_DEFINITIONS_ASM_RELEASE ${DEFINES_ASM} ${DEFINES_ASM_RELEASE})
161: set(COMPILE_DEFINITIONS_ASM_MINSIZEREL ${DEFINES_ASM} ${DEFINES_ASM_MINSIZEREL})
162: set(COMPILE_DEFINITIONS_ASM_RELWITHDEBINFO ${DEFINES_ASM} ${DEFINES_ASM_RELWITHDEBINFO})
```

- However we don't set any build configuration specific assembly definitions
- And then later variable for the specific build configuration are set:
```cmake
File: CMakeLists.txt
186: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
187:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
188:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
189:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
190:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
191:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
192:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
...
```

#### Compiler options

The complete list below are all compiler options:

```text
-g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
-O2 
-MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d
```

- The first -g is automatically added by CMake as we are building for a debug configuration
- The `-mcpu=cortex-a53 -mlittle-endian -mcmodel=small` options are set due to the fact that we are building for Raspberry Pi 3:
- 
```cmake
File: CMakeLists.txt
53: if (BAREMETAL_TARGET STREQUAL "RPI3")
54:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
55:     set(BAREMETAL_TARGET_KERNEL kernel8)
56:     set(BAREMETAL_RPI_TARGET 3)
...
```
- The options all are defined later on in the main CMake file, where they are combinex with `BAREMETAL_ARCH_CPU_OPTIONS`:

```cmake
File: CMakeLists.txt
128: set(FLAGS_ASM ${BAREMETAL_ARCH_CPU_OPTIONS})
```

- The `-O2` option is set for the specific Debug build configuration:

```cmake
File: CMakeLists.txt
129: set(FLAGS_ASM_DEBUG -O2)
130: set(FLAGS_ASM_RELEASE -O2)
131: set(FLAGS_ASM_MINSIZEREL -O2)
132: set(FLAGS_ASM_RELWITHDEBINFO -O2)
```

- The generic and the build configuration specific definitions are later combined:

```cmake
File: CMakeLists.txt
174: set(COMPILE_OPTIONS_ASM_DEBUG ${FLAGS_ASM} ${FLAGS_ASM_DEBUG})
175: set(COMPILE_OPTIONS_ASM_RELEASE ${FLAGS_ASM} ${FLAGS_ASM_RELEASE})
176: set(COMPILE_OPTIONS_ASM_MINSIZEREL ${FLAGS_ASM} ${FLAGS_ASM_MINSIZEREL})
177: set(COMPILE_OPTIONS_ASM_RELWITHDEBINFO ${FLAGS_ASM} ${FLAGS_ASM_RELWITHDEBINFO})
```

- And then later variable for the specific build configuration are set:

```cmake
File: CMakeLists.txt
186: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
187:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
188:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
189:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
190:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
191:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
192:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
...
```

- Finally, the following options are added automatically by CMake, to generate a dependency file.

```
-MD
-MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
-MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d
```
 
#### Actual compilation

The final parameters determine the source files compiled, and the resulting object file:

```text
-o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
-c ../code/applications/demo/src/start.S
```

The -c option tells the compiler to compile the specified file, the -o option tells it to generate the specified object file. Notice that the path for the object file is relative to the CMake build directory, in this case `cmake-BareMetal-Debug`.

### Compiling source code

```text
D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe
  -DBAREMETAL_TARGET=RPI3 -D_DEBUG
  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
  -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new
  -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics
  -nostdinc -nostdlib -nostdinc++ 
  -fno-exceptions -fno-rtti
  -O0 -Wno-unused-variable -Wno-unused-parameter
  -std=gnu++17
  -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d
  -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
```

#### Compiler

The first part is the compiler call, the rest are command line parameters for the compiler.
For the C++ code this is `aarch64-none-elf-g++.exe`.

#### Compiler definitions

The -D options specify all compiler definitions. You will recognize the options specified in CMakeSettings.json: `"cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3"`.
- The first option triggers setting the `CMAKE_VERBOSE_MAKEFILE` setting, that shows the configuration output.
- The second trigger setting the `BAREMETAL_TARGET=RPI3` definition.
- The `PLATFORM_BAREMETAL` definition is triggered by the main CMake file:

```cmake
File: CMakeLists.txt
64: set(DEFINES_C
65:     PLATFORM_BAREMETAL
66:     BAREMETAL_TARGET=${BAREMETAL_TARGET}
67:     )
```

- The \_DEBUG definition is caused by the `CMAKE_BUILD_TYPE` variable, which is set to `Debug` as we are building a Debug application. This is also done in the main CMake file:
- 
```cmake
File: CMakeLists.txt
68: set(DEFINES_C_DEBUG _DEBUG)
69: set(DEFINES_C_RELEASE NDEBUG)
70: set(DEFINES_C_MINSIZEREL NDEBUG)
71: set(DEFINES_C_RELWITHDEBINFO NDEBUG)
```

- The generic and the build configuration specific definitions are later combined:

```cmake
File: CMakeLists.txt
154: set(COMPILE_DEFINITIONS_C_DEBUG ${DEFINES_C} ${DEFINES_C_DEBUG})
155: set(COMPILE_DEFINITIONS_C_RELEASE ${DEFINES_C} ${DEFINES_C_RELEASE})
156: set(COMPILE_DEFINITIONS_C_MINSIZEREL ${DEFINES_C} ${DEFINES_C_MINSIZEREL})
157: set(COMPILE_DEFINITIONS_C_RELWITHDEBINFO ${DEFINES_C} ${DEFINES_C_RELWITHDEBINFO})
```

- And then later variable for the specific build configuration are set:

```cmake
File: CMakeLists.txt
186: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
187:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
188:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
189:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
190:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
191:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
192:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
...
```

#### Compiler options

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
File: CMakeLists.txt
53: if (BAREMETAL_TARGET STREQUAL "RPI3")
54:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
55:     set(BAREMETAL_TARGET_KERNEL kernel8)
...
```

- The `-Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti` options all are defined later on in the main CMake file, where they are combinex with `BAREMETAL_ARCH_CPU_OPTIONS`:

```cmake
File: CMakeLists.txt
100: set(FLAGS_CXX
101:     ${BAREMETAL_ARCH_CPU_OPTIONS}
102:     -Wall
103:     -Wextra
104:     -Werror
105:     -Wno-missing-field-initializers
106:     -Wno-unused-value
107:     -Wno-aligned-new
108:     -ffreestanding
109:     -fsigned-char
110:     -nostartfiles
111:     -mno-outline-atomics
112:     -nostdinc
113:     -nostdlib
114:     -nostdinc++
115:     -fno-exceptions
116:     -fno-rtti
117:     )
```

- The `-O0 -Wno-unused-variable -Wno-unused-parameter` options are set for the specific Debug build configuration:

```cmake
File: CMakeLists.txt
119: # -g is added by CMake
120: set(FLAGS_CXX_DEBUG -O0 -Wno-unused-variable -Wno-unused-parameter)
121: # -O3 is added by CMake
122: set(FLAGS_CXX_RELEASE -D__USE_STRING_INLINES)
123: # -Os is added by CMake
124: set(FLAGS_CXX_MINSIZEREL -O3)
125: # -O2 -g is added by CMake
126: set(FLAGS_CXX_RELWITHDEBINFO )
```

- The generic and the build configuration specific definitions are later combined:

```cmake
File: CMakeLists.txt
169: set(COMPILE_OPTIONS_CXX_DEBUG ${FLAGS_CXX} ${FLAGS_CXX_DEBUG})
170: set(COMPILE_OPTIONS_CXX_RELEASE ${FLAGS_CXX} ${FLAGS_CXX_RELEASE})
171: set(COMPILE_OPTIONS_CXX_MINSIZEREL ${FLAGS_CXX} ${FLAGS_CXX_MINSIZEREL})
172: set(COMPILE_OPTIONS_CXX_RELWITHDEBINFO ${FLAGS_CXX} ${FLAGS_CXX_RELWITHDEBINFO})
```

- And then later variable for the specific build configuration are set:

```cmake
File: CMakeLists.txt
186: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
187:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
188:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
189:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
190:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
191:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
192:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
...
```

- The option `-std=gnu++17` is automatically added by CMake due to setting the variable `SUPPORTED_CPP_STANDARD` in the main CMake file:

```cmake
File: CMakeLists.txt
148: set(SUPPORTED_CPP_STANDARD 17)
```

- Which is then later used when defining the target properties in the `code/application/demo` CMake files:

```cmake
File: code/applications/demo/CMakeLists.txt
95: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
```

- Finally, the following options are added automatically by CMake, to generate a dependency file.
```
-MD
-MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
-MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d
```

#### Actual compilation

The final parameters determine the source files compiled, and the resulting object file:

```text
-o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
-c ../code/applications/demo/src/main.cpp
```

The -c option tells the compiler to compile the specified file, the -o option tells it to generate the specified object file. Notice that the paths are relative to the CMake build directory, in this case `cmake-BareMetal-Debug`.

### Linking the demo application

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
   code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
   -o ..\output\Debug\bin\demo.elf
   -Wl,--start-group -Wl,--end-group &&
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
   code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
   -o ..\output\Debug\bin\demo.elf
   -Wl,--start-group -Wl,--end-group
```

- The first -g is automatically added by CMake as we are building for a debug configuration
- The `-LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1` option is added by the toolchain:
```cmake
File: baremetal.toolchain
23: set(TOOLCHAIN_AUXILIARY_PATH ${TOOLCHAIN_ROOT}/lib/gcc/${TOOL_DESTINATION_PLATFORM}/13.2.1)

...

63: if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "")
64:     set(HAVE_AUX_PATH false)
65: else()
66:     list(FIND ${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH} HAVE_AUX_PATH)
67: endif()
68: message(STATUS "CMAKE_EXE_LINKER_FLAGS=  ${CMAKE_EXE_LINKER_FLAGS}")
69: if (NOT HAVE_AUX_PATH)
70:     message(STATUS "Adding to CMAKE_EXE_LINKER_FLAGS -L${TOOLCHAIN_AUXILIARY_PATH}")
71:     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_AUXILIARY_PATH}" CACHE INTERNAL "" FORCE)
72: endif()
```

- So the `TOOLCHAIN_AUXILIARY_PATH` is set, and then appended to `CMAKE_EXE_LINKER_FLAGS` if not already there. This option specifies a directory searched for object and library files when linking the target.
The `CMAKE_EXE_LINKER_FLAGS` are automatically added to the linker flags for a target by CMake.
- The options `-Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles` are set in the main CMake file:

```cmake
File: CMakeLists.txt
134: set(LINK_FLAGS
135:     -Wl,--section-start=.init=${BAREMETAL_LOAD_ADDRESS}
136:     -T ${CMAKE_SOURCE_DIR}/baremetal.ld
137:     -nostdlib
138:     -nostartfiles
139:     )
140: set(LINK_FLAGS_DEBUG )
141: set(LINK_FLAGS_RELEASE )
142: set(LINK_FLAGS_MINSIZEREL )
143: set(LINK_FLAGS_RELWITHDEBINFO )
```

- This is then combined later on with the build configuration specific linker flags:

```cmake
File: CMakeLists.txt
File: d:\Projects\baremetal.github\CMakeLists.txt
179: set(LINKER_OPTIONS_DEBUG ${LINK_FLAGS} ${LINK_FLAGS_DEBUG})
180: set(LINKER_OPTIONS_RELEASE ${LINK_FLAGS} ${LINK_FLAGS_RELEASE})
181: set(LINKER_OPTIONS_MINSIZEREL ${LINK_FLAGS} ${LINK_FLAGS_MINSIZEREL})
182: set(LINKER_OPTIONS_RELWITHDEBINFO ${LINK_FLAGS} ${LINK_FLAGS_RELWITHDEBINFO})
```

- Subsequently, the linker flags for the selected build configuration is set:

```cmake
File: CMakeLists.txt
186: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
187:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
188:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
189:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
190:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
191:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
192:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
...
```

- The `LINKER_OPTIONS` variable is then used in the project CMake file (`code/applications/demo/CMakeLists.txt` and `code/libraries/baremetal/CMakeLists.txt`):

```cmake
File: code/applications/demo/CMakeLists.txt
23: set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})
```

- Then the target linker options are set on the target as a property using this veriable:

```cmake
File: code/applications/demo/CMakeLists.txt
97: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
98: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
99:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
100: endif()
```

- What follows is a list of object files to be linked, in this case just one:
- 
```text
code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
```

- This list is determined by the source files specified for the target:
- 
```cmake
File: code/applications/demo/CMakeLists.txt
33: set(PROJECT_SOURCES
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/start.S
36:     )
37: 
38: set(PROJECT_INCLUDES_PUBLIC )
39: set(PROJECT_INCLUDES_PRIVATE )

...

69: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
```

- Next is the option `-o ..\output\Debug\bin\demo.elf`, which specifies the output of the linker, the application.
This is determined by the specification of the name and location of the target file in the project CMake file:

```cmake
File: code/applications/demo/CMakeLists.txt
12: set(PROJECT_TARGET_NAME ${PROJECT_NAME}.elf)

...

103: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
104: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
105: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
```

- As we are building an executable, the location will be the `OUTPUT_BIN_DIR`, the file name will be `PROJECT_TARGET_NAME`.
- Finally, the list of libraries to link to is specified, which is for now empty:

```
File: code/applications/demo/CMakeLists.txt
64: if (PLATFORM_BAREMETAL)
65:     set(START_GROUP -Wl,--start-group)
66:     set(END_GROUP -Wl,--end-group)
67: endif()
```

- This is determined by the list of libraries to link to in the project CMake file:

```cmake
File: code/applications/demo/CMakeLists.txt
25: set(PROJECT_DEPENDENCIES
26:     )
27: 
28: set(PROJECT_LIBS
29:     ${LINKER_LIBRARIES}
30:     ${PROJECT_DEPENDENCIES}
31:     )

...

71: target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
```

- The reason libraries linked to is placed in a group (`-Wl,--start-group <libraries> -Wl,--end-group`) is that the GCC linker tries to resolve symbols in reverse order, i.e. expects the lowest level symbols to be added last, and we cannot always guarantee the correct order of libraires.
By putting them in a group, the linker will iterate as many times as needed to resolve all symbols, if possible of course.

### Creating the kernel image

```text
cmd.exe /C
  "cd /D D:\Projects\baremetal.test\cmake-BareMetal-Debug\code\applications\demo\create-image &&
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

## Running the application

Start QEMU listening to UART1 for the demo project:

```bat
tools\startQEMU-image-uart1.bat demo
```

Start debugging as shown in [Visual Studio CMake integration](##Visual-Studio-CMake-integration). Make sure the demo applications is selected as startup project.

Next: [05-console-uart1](05-console-uart1.md)
