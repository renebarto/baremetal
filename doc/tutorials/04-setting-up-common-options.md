# Tutorial 04: Setting up common options {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS}

@tableofcontents

In order for CMake projects to work smoothly, it is convenient to do some groundwork for them.

This will again be a length story, as we will set up how we handle setting of compiler definitions and options, linker options, etc.

If you're curious to see how this works, or just want to dive directly into the code,
in `tutorial/04-setting-up-common-options` there is a complete copy of what we work towards in this section.
Its root will clearly be `tutorial/04-setting-up-common-options`.
Please be aware of this when e.g. debugging, the paths in vs.launch.json may not match your specific case.

## CMake function display_list {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_CMAKE_FUNCTION_DISPLAY_LIST}

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

## CMake function show_target_properties {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_CMAKE_FUNCTION_SHOW_TARGET_PROPERTIES}

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
93: 
94:             get_target_property(TARGET_CXX_STANDARD ${target} CXX_STANDARD)
95:             display_list("Target C++ standard               : " ${TARGET_CXX_STANDARD})
96:         endif ()
97:     endif()
98: endfunction()
99: 
...
```

Without going into too much detail, this function prints for a specific target:

- Line 23-24: the type of target (executable, library, interface library)
- Line 26-27: compiler definitions (only for executable and library targets)
- Line 28-29: compiler options (only for executable and library targets)
- Line 32-33: interface include directories
- Line 36-37: include directories (only for executable and library targets)
- Line 39-40: libraries to be linked to (only for executable and library targets)
- Line 42-43: linker options (only for executable and library targets)
- Line 46-47: interface library compiler definitions
- Line 49-50: interface library compiler options
- Line 52-53: interface library include directories
- Line 55-56: interface library libraries to be linked to
- Line 58-59: imported dependencies
- Line 61-62: imported libraries to be linked to
- Line 65-69: dependencies and manually added dependencies
- Line 73-80: output directories for static libraries, shared libraries, and executables
- Line 82-83: linker options
- Line 85-86: the target version
- Line 88-89: the target so version in case of a shared library
- Line 91-92: the target output name
- Line 94-95: the C++ standard used for compilation

## Common compiler directives {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMMON_COMPILER_DIRECTIVES}

As most compiler definitions and compiler options will have some commonality, it is convenient to define these common parts in variables,
and use them in our project configuration.
Very often these will only differ per platform to build for, e.g. between a Windows and a Linux build.
In our case, we will want to differentiate between Raspberry Pi 3, 4 and 5, and between different build configurations.

So we will update the main CMake file to set these variables. We will revisit this part a number of times.

Just before `add_subdirectory(code)` in the main CMake file, insert the following:

### Common definitions and options {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMMON_COMPILER_DIRECTIVES_COMMON_DEFINITIONS_AND_OPTIONS}

```cmake
File: CMakeLists.txt
...
34: if (BAREMETAL_TARGET STREQUAL "RPI3")
35:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
36:     set(BAREMETAL_TARGET_KERNEL kernel8)
37:     set(BAREMETAL_RPI_TARGET 3)
38: elseif (BAREMETAL_TARGET STREQUAL "RPI4")
39:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a72 -mlittle-endian -mcmodel=small)
40:     set(BAREMETAL_TARGET_KERNEL kernel8-rpi4)
41:     set(BAREMETAL_RPI_TARGET 4)
42: elseif (BAREMETAL_TARGET STREQUAL "RPI5")
43:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a76 -mlittle-endian -mcmodel=small)
44:     set(BAREMETAL_TARGET_KERNEL kernel_2712)
45:     set(BAREMETAL_RPI_TARGET 5)
46: endif()
47: set(BAREMETAL_LOAD_ADDRESS 0x80000)
48: 
49: set(DEFINES_C
50:     PLATFORM_BAREMETAL
51:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
52:     )
53: set(DEFINES_C_DEBUG _DEBUG)
54: set(DEFINES_C_RELEASE NDEBUG)
55: set(DEFINES_C_MINSIZEREL NDEBUG)
56: set(DEFINES_C_RELWITHDEBINFO NDEBUG)
57: set(DEFINES_ASM
58:     PLATFORM_BAREMETAL
59:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
60:     )
61: 
62: set(FLAGS_C
63:     ${BAREMETAL_ARCH_CPU_OPTIONS}
64:     -Wall
65:     -Wextra
66:     -Werror
67:     -Wno-parentheses
68:     -ffreestanding
69:     -fsigned-char
70:     -nostartfiles
71:     -std=gnu99
72:     -mno-outline-atomics
73:     -nostdinc
74:     -nostdlib
75: )
76: # -g is added by CMake
77: set(FLAGS_C_DEBUG -O0 -Wno-unused-variable -Wno-unused-parameter)
78: # -O3 is added by CMake
79: set(FLAGS_C_RELEASE -D__USE_STRING_INLINES)
80: # -Os is added by CMake
81: set(FLAGS_C_MINSIZEREL -O3)
82: # -O2 -g is added by CMake
83: set(FLAGS_C_RELWITHDEBINFO )
84: 
85: set(FLAGS_CXX
86:     ${BAREMETAL_ARCH_CPU_OPTIONS}
87:     -Wall
88:     -Wextra
89:     -Werror
90:     -Wno-missing-field-initializers
91:     -Wno-unused-value
92:     -Wno-aligned-new
93:     -ffreestanding
94:     -fsigned-char
95:     -nostartfiles
96:     -mno-outline-atomics
97:     -nostdinc
98:     -nostdlib
99:     -nostdinc++
100:     -fno-exceptions
101:     -fno-rtti
102:     )
103: 
104: # -g is added by CMake
105: set(FLAGS_CXX_DEBUG -O0 -Wno-unused-variable -Wno-unused-parameter)
106: # -O3 is added by CMake
107: set(FLAGS_CXX_RELEASE -D__USE_STRING_INLINES)
108: # -Os is added by CMake
109: set(FLAGS_CXX_MINSIZEREL -O3)
110: # -O2 -g is added by CMake
111: set(FLAGS_CXX_RELWITHDEBINFO )
112: 
113: set(FLAGS_ASM ${BAREMETAL_ARCH_CPU_OPTIONS})
114: set(FLAGS_ASM_DEBUG -O2)
115: set(FLAGS_ASM_RELEASE -O2)
116: set(FLAGS_ASM_MINSIZEREL -O2)
117: set(FLAGS_ASM_RELWITHDEBINFO -O2)
118: 
119: set(LINK_FLAGS
120:     -Wl,--section-start=.init=${BAREMETAL_LOAD_ADDRESS}
121:     -T ${CMAKE_SOURCE_DIR}/baremetal.ld
122:     -nostdlib
123:     -nostartfiles
124:     )
125: set(LINK_FLAGS_DEBUG )
126: set(LINK_FLAGS_RELEASE )
127: set(LINK_FLAGS_MINSIZEREL )
128: set(LINK_FLAGS_RELWITHDEBINFO )
129: 
130: list(APPEND LINK_LIBRARIES )
131: list(APPEND LINK_DIRECTORIES )
132: 
133: set(SUPPORTED_CPP_STANDARD 17)
134: 
135: message(STATUS "C++ compiler version:    ${CMAKE_CXX_COMPILER_VERSION}")
136: message(STATUS "C compiler version:      ${CMAKE_C_COMPILER_VERSION}")
137: message(STATUS "C++ supported standard:  ${SUPPORTED_CPP_STANDARD}")
138: 
...
```

Explanation:
- Line 34-46: Depending on the variable `BAREMETAL_TARGET` which we defined later on, we define the variables `BAREMETAL_ARCH_CPU_OPTIONS` and `BAREMETAL_TARGET_KERNEL`, which define specific compiler options for the platform we're building for (Raspberry Pi 3, 4 or 5), and the base name of the kernel image file.
Also, if `BAREMETAL_TARGET` is set to "RPI3", we set `BAREMETAL_RPI_TARGET` to 3, for `BAREMETAL_TARGET` equal "RPI4", we set `BAREMETAL_RPI_TARGET` to 4, and for `BAREMETAL_TARGET` equal "RPI5", we set `BAREMETAL_RPI_TARGET` to 5
- Line 47: We define the variable `BAREMETAL_LOAD_ADDRESS` to hold the start address for the baremetal application. This will always be 0x80000
- Line 49-52: We define the variable `DEFINES_C` to hold the generic compiler definitions for C and C++, independent of the build configuration.
Here we add the definition `PLATFORM_BAREMETAL` to enable conditional compilation if needed, as well as `BAREMETAL_RPI_TARGET` to select between the target boards
- Line 53-56: We define the variables `DEFINES_C_DEBUG`, `DEFINES_C_RELEASE`, `DEFINES_C_MINSIZEREL` and `DEFINES_C_RELWITHDEBINFO` to hold specific compiler definitions for C and C++ per build configuration
- Line 57-60: We define the variable `DEFINES_ASM` to hold the generic compiler definitions for assembly, independent of the build configuration
- Line 62-75: We define the variable `FLAGS_C` to hold the generic compiler options for C, independent of the build configuration.
Notice that we use the architecture options defined before
- Line 76-83: We define the variables `FLAGS_C_DEBUG`, `FLAGS_C_RELEASE`, `FLAGS_C_MINSIZEREL` and `FLAGS_C_RELWITHDEBINFO` to hold specific compiler options for C per build configuration
- Line 85-102: We define the variable `FLAGS_CXX` to hold the generic compiler options for C++, independent of the build configuration.
Notice that we use the architecture options defined before
- Line 104-111: We define the variables `FLAGS_CXX_DEBUG`, `FLAGS_CXX_RELEASE`, `FLAGS_CXX_MINSIZEREL` and `FLAGS_CXX_RELWITHDEBINFO` to hold specific compiler options for C++ per build configuration
- Line 113: We define the variable `FLAGS_ASM` to hold the generic compiler options for assembly, independent of the build configuration.
Notice that we use the architecture options defined before
- Line 114-117: We define the variables `FLAGS_ASM_DEBUG`, `FLAGS_ASM_RELEASE`, `FLAGS_ASM_MINSIZEREL` and `FLAGS_ASM_RELWITHDEBINFO` to hold specific compiler options for assembly per build configuration
- Line 119-124: We define the variable `LINK_FLAGS` to hold the linker options
- Line 125-128: We define the variables `LINK_FLAGS_DEBUG`, `LINK_FLAGS_RELEASE`, `LINK_FLAGS_MINSIZEREL` and `LINK_FLAGS_RELWITHDEBINFO` to hold specific linker options per build configuration
- Line 130: We add to variable `LINK_LIBRARIES` holding the generic list of libraries to link to (empty for now)
- Line 131: We add to variable `LINK_DIRECTORIES` holding the generic list of directories to search when linking (empty for now)
- Line 133: We define the variable `SUPPORTED_CPP_STANDARD` to holding the C++ standard used
- Line 135-137: We print the current C++ and C compiler versions, as well as the C++ standard version just defined

So the section above adds a lot of variables, to set compiler and linker definitions and options, common and for each different build configuration.

As we now set the variable `BAREMETAL_TARGET` externally, and define the variable `BAREMETAL_TARGET_KERNEL` depending on this, we can remove the two line which set these:

```cmake
File: CMakeLists.txt
...
13: if("${CMAKE_BUILD_TYPE}" STREQUAL "")
14:     set(CMAKE_BUILD_TYPE Debug)
15: endif()
16: set(CONFIG_DIR ${CMAKE_BUILD_TYPE})
17: 
18: set(DEPLOYMENT_DIR "${CMAKE_SOURCE_DIR}/deploy" CACHE STRING "Deployment directory")
19: 
...
```

### Combining common definitions and options {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMMON_COMPILER_DIRECTIVES_COMBINING_COMMON_DEFINITIONS_AND_OPTIONS}

We will now combine the variables defined in the previous section to one for each build configuration.
Then we create a specific version for the currently set build configuration.
For this we add another section just before `add_subdirectory(code)` in the main CMake file:

```cmake
File: CMakeLists.txt
...
136: set(COMPILE_DEFINITIONS_C_DEBUG ${DEFINES_C} ${DEFINES_C_DEBUG})
137: set(COMPILE_DEFINITIONS_C_RELEASE ${DEFINES_C} ${DEFINES_C_RELEASE})
138: set(COMPILE_DEFINITIONS_C_MINSIZEREL ${DEFINES_C} ${DEFINES_C_MINSIZEREL})
139: set(COMPILE_DEFINITIONS_C_RELWITHDEBINFO ${DEFINES_C} ${DEFINES_C_RELWITHDEBINFO})
140: 
141: set(COMPILE_DEFINITIONS_ASM_DEBUG ${DEFINES_ASM} ${DEFINES_ASM_DEBUG})
142: set(COMPILE_DEFINITIONS_ASM_RELEASE ${DEFINES_ASM} ${DEFINES_ASM_RELEASE})
143: set(COMPILE_DEFINITIONS_ASM_MINSIZEREL ${DEFINES_ASM} ${DEFINES_ASM_MINSIZEREL})
144: set(COMPILE_DEFINITIONS_ASM_RELWITHDEBINFO ${DEFINES_ASM} ${DEFINES_ASM_RELWITHDEBINFO})
145: 
146: set(COMPILE_OPTIONS_C_DEBUG ${FLAGS_C} ${FLAGS_C_DEBUG})
147: set(COMPILE_OPTIONS_C_RELEASE ${FLAGS_C} ${FLAGS_C_RELEASE})
148: set(COMPILE_OPTIONS_C_MINSIZEREL ${FLAGS_C} ${FLAGS_C_MINSIZEREL})
149: set(COMPILE_OPTIONS_C_RELWITHDEBINFO ${FLAGS_C} ${FLAGS_C_RELWITHDEBINFO})
150: 
151: set(COMPILE_OPTIONS_CXX_DEBUG ${FLAGS_CXX} ${FLAGS_CXX_DEBUG})
152: set(COMPILE_OPTIONS_CXX_RELEASE ${FLAGS_CXX} ${FLAGS_CXX_RELEASE})
153: set(COMPILE_OPTIONS_CXX_MINSIZEREL ${FLAGS_CXX} ${FLAGS_CXX_MINSIZEREL})
154: set(COMPILE_OPTIONS_CXX_RELWITHDEBINFO ${FLAGS_CXX} ${FLAGS_CXX_RELWITHDEBINFO})
155: 
156: set(COMPILE_OPTIONS_ASM_DEBUG ${FLAGS_ASM} ${FLAGS_ASM_DEBUG})
157: set(COMPILE_OPTIONS_ASM_RELEASE ${FLAGS_ASM} ${FLAGS_ASM_RELEASE})
158: set(COMPILE_OPTIONS_ASM_MINSIZEREL ${FLAGS_ASM} ${FLAGS_ASM_MINSIZEREL})
159: set(COMPILE_OPTIONS_ASM_RELWITHDEBINFO ${FLAGS_ASM} ${FLAGS_ASM_RELWITHDEBINFO})
160: 
161: set(LINKER_OPTIONS_DEBUG ${LINK_FLAGS} ${LINK_FLAGS_DEBUG})
162: set(LINKER_OPTIONS_RELEASE ${LINK_FLAGS} ${LINK_FLAGS_RELEASE})
163: set(LINKER_OPTIONS_MINSIZEREL ${LINK_FLAGS} ${LINK_FLAGS_MINSIZEREL})
164: set(LINKER_OPTIONS_RELWITHDEBINFO ${LINK_FLAGS} ${LINK_FLAGS_RELWITHDEBINFO})
165: 
166: set(LINKER_LIBRARIES ${LINK_LIBRARIES})
167: 
168: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
169:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
170:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
171:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
172:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
173:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
174:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
175: elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
176:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_RELEASE})
177:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_RELEASE})
178:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_RELEASE})
179:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_RELEASE})
180:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_RELEASE})
181:     set(LINKER_OPTIONS ${LINKER_OPTIONS_RELEASE})
182: elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
183:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_MINSIZEREL})
184:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_MINSIZEREL})
185:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_MINSIZEREL})
186:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_MINSIZEREL})
187:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_MINSIZEREL})
188:     set(LINKER_OPTIONS ${LINKER_OPTIONS_MINSIZEREL})
189: elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
190:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_RELWITHDEBINFO})
191:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_RELWITHDEBINFO})
192:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_RELWITHDEBINFO})
193:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_RELWITHDEBINFO})
194:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_RELWITHDEBINFO})
195:     set(LINKER_OPTIONS ${LINKER_OPTIONS_RELWITHDEBINFO})
196: else()
197:     message(FATAL_ERROR "Invalid build type: " ${CMAKE_BUILD_TYPE})
198: endif()
199: 
...
```

Explanation:
- Line 136-139: We combine the generic compiler definitions for C and C++ with the configuration specific ones in variables:
  - `COMPILE_DEFINITIONS_C_DEBUG`
  - `COMPILE_DEFINITIONS_C_RELEASE`
  - `COMPILE_DEFINITIONS_C_MINSIZEREL`
  - `COMPILE_DEFINITIONS_C_RELWITHDEBINFO`
- Line 141-144: We combine the generic compiler definitions for assembly with the configuration specific ones in variables:
  - `COMPILE_DEFINITIONS_ASM_DEBUG`
  - `COMPILE_DEFINITIONS_ASM_RELEASE`
  - `COMPILE_DEFINITIONS_ASM_MINSIZEREL`
  - `COMPILE_DEFINITIONS_ASM_RELWITHDEBINFO`
- Line 146-149: We combine the generic compiler options for C with the configuration specific ones in variables:
  - `COMPILE_OPTIONS_C_DEBUG`
  - `COMPILE_OPTIONS_C_RELEASE`
  - `COMPILE_OPTIONS_C_MINSIZEREL`
  - `COMPILE_OPTIONS_C_RELWITHDEBINFO`
- Line 151-154: We combine the generic compiler options for C++ with the configuration specific ones in variables:
  - `COMPILE_OPTIONS_CXX_DEBUG`
  - `COMPILE_OPTIONS_CXX_RELEASE`
  - `COMPILE_OPTIONS_CXX_MINSIZEREL`
  - `COMPILE_OPTIONS_CXX_RELWITHDEBINFO`
- Line 156-159: We combine the generic compiler options for assembly with the configuration specific ones in variables:
  - `COMPILE_OPTIONS_ASM_DEBUG`
  - `COMPILE_OPTIONS_ASM_RELEASE`
  - `COMPILE_OPTIONS_ASM_MINSIZEREL`
  - `COMPILE_OPTIONS_ASM_RELWITHDEBINFO`
- Line 161-164: We combine the generic linker options with the configuration specific ones in variables:
  - `LINKER_OPTIONS_DEBUG`
  - `LINKER_OPTIONS_RELEASE`
  - `LINKER_OPTIONS_MINSIZEREL`
  - `LINKER_OPTIONS_RELWITHDEBINFO`
- Line 166: We set the variable `LINKER_LIBRARIES` to hold the general libraries to link to
- Line 168-198: Depending on the current build configuration, we define the following variables to the correct ones for the build configuration set:
  - `COMPILE_DEFINITIONS_C`
  - `COMPILE_DEFINITIONS_ASM`
  - `COMPILE_OPTIONS_C`
  - `COMPILE_OPTIONS_CXX`
  - `COMPILE_OPTIONS_ASM`
  - `LINKER_OPTIONS`

### Adding some variables {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMMON_COMPILER_DIRECTIVES_ADDING_SOME_VARIABLES}

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
34:     set(BAREMETAL_TARGET "RPI3") # Board type (RPI3/RPI4/RPI5)
35: endif()
36: 
37: if (BAREMETAL_TARGET STREQUAL "RPI3")
38:     message(STATUS "Building for Raspberry Pi 3")
39: elseif (BAREMETAL_TARGET STREQUAL "RPI4")
40:     message(STATUS "Building for Raspberry Pi 4")
41: elseif (BAREMETAL_TARGET STREQUAL "RPI5")
42:     message(STATUS "Building for Raspberry Pi 5")
43: else()
44:     message(FATAL_ERROR "Incorrect target ${BAREMETAL_TARGET} specified, must be RPI3, RPI4 or RPI5")
45: endif()
46: 
47: message(STATUS "\n** Setting up project **\n--")
48: 
49: message(STATUS "\n##################################################################################")
50: message(STATUS "\n** Setting up toolchain **\n--")
51: 
...
```

Explanation:
- Line 23: We define a variable `VERBOSE_BUILD` to enable or disable verbose build output. The default is ON
- Line 25-29: Depending on the value of `VERBOSE_BUILD`, we set the standard CMake variable `CMAKE_VERBOSE_MAKEFILE` to ON or OFF, wich is placed in the CMake cache.
This variable enables more verbose build output if set
- Line 30: We set the standard CMake variable `CMAKE_EXPORT_COMPILE_COMMANDS` to ON. This will make CMake generate a JSON file `compile_commands.json` in the CMake build directory, that can be used by Visual Studio and other tools.
This can also come in handy when trying to figure out which source is compiled with which options
- Line 31: We set the standard CMake variable `CMAKE_COLOR_MAKEFILE` to ON. This will generate CMake configuration and build output with ANSI color output. This will not work inside Visual Studio , but it will output in color on the command line
- Line 33-35: Finally, we set the variable `BAREMETAL_TARGET`, if not set already, to RPI3, to make it default to Raspberry Pi 3.
- Line 37-45: We check the variable `BAREMETAL_TARGET`, and print the platform we're building for. Also, if no valid value is given to `BAREMETAL_TARGET`, we display an error and fail the CMake configuration step

### Standard definitions for linker {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMMON_COMPILER_DIRECTIVES_STANDARD_DEFINITIONS_FOR_LINKER}

The demo project in [02-setting-up-a-project](02-setting-up-a-project.md) uses two CMake variables to define the start and end of a linker group. As this will come back for each executable. we will move this definition to the main CMake file:

```
File: CMakeLists.txt
190: if (PLATFORM_BAREMETAL)
191:     set(LINKER_START_GROUP -Wl,--start-group)
192:     set(LINKER_END_GROUP -Wl,--end-group)
193: endif()
194: 
195: set(LINKER_LIBRARIES ${LINK_LIBRARIES})
196: 
```

## Update default variables in CMakeSettings.json {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_UPDATE_DEFAULT_VARIABLES_IN_CMAKESETTINGSJSON}

In order to get more verbose output from CMake, we will need to update `CMakeSettings.json`.
We already set `BAREMETAL_TARGET` in `CMakeSettings.json`, and will now start to use it.

```json
File: CMakeSettings.json
1: {
2:     "environments": [ {} ],
3:     "configurations": [
4:         {
5:             "name": "BareMetal-RPI3-Debug",
6:             "generator": "Ninja",
7:             "configurationType": "Debug",
8:             "buildRoot": "${projectDir}\\cmake-${name}",
9:             "installRoot": "${projectDir}\\output\\install\\${name}",
10:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3",
11:             "buildCommandArgs": "",
12:             "ctestCommandArgs": "",
13:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
14:             "inheritEnvironments": [ "gcc-arm" ]
15:         },
16:         {
17:             "name": "BareMetal-RPI4-Debug",
18:             "generator": "Ninja",
19:             "configurationType": "Debug",
20:             "buildRoot": "${projectDir}\\cmake-${name}",
21:             "installRoot": "${projectDir}\\output\\install\\${name}",
22:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4",
23:             "buildCommandArgs": "",
24:             "ctestCommandArgs": "",
25:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
26:             "inheritEnvironments": [ "gcc-arm" ]
27:         },
28:         {
29:             "name": "BareMetal-RPI5-Debug",
30:             "generator": "Ninja",
31:             "configurationType": "Debug",
32:             "buildRoot": "${projectDir}\\cmake-${name}",
33:             "installRoot": "${projectDir}\\output\\install\\${name}",
34:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5",
35:             "buildCommandArgs": "",
36:             "ctestCommandArgs": "",
37:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
38:             "inheritEnvironments": [ "gcc-arm" ]
39:         },
40:         {
41:             "name": "BareMetal-RPI3-Release",
42:             "generator": "Ninja",
43:             "configurationType": "Release",
44:             "buildRoot": "${projectDir}\\cmake-${name}",
45:             "installRoot": "${projectDir}\\output\\install\\${name}",
46:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3",
47:             "buildCommandArgs": "",
48:             "ctestCommandArgs": "",
49:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
50:             "inheritEnvironments": [ "gcc-arm" ]
51:         },
52:         {
53:             "name": "BareMetal-RPI4-Release",
54:             "generator": "Ninja",
55:             "configurationType": "Release",
56:             "buildRoot": "${projectDir}\\cmake-${name}",
57:             "installRoot": "${projectDir}\\output\\install\\${name}",
58:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4",
59:             "buildCommandArgs": "",
60:             "ctestCommandArgs": "",
61:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
62:             "inheritEnvironments": [ "gcc-arm" ]
63:         },
64:         {
65:             "name": "BareMetal-RPI5-Release",
66:             "generator": "Ninja",
67:             "configurationType": "Release",
68:             "buildRoot": "${projectDir}\\cmake-${name}",
69:             "installRoot": "${projectDir}\\output\\install\\${name}",
70:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5",
71:             "buildCommandArgs": "",
72:             "ctestCommandArgs": "",
73:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
74:             "inheritEnvironments": [ "gcc-arm" ]
75:         },
76:         {
77:             "name": "BareMetal-RPI3-RelWithDebInfo",
78:             "generator": "Ninja",
79:             "configurationType": "RelWithDebInfo",
80:             "buildRoot": "${projectDir}\\cmake-${name}",
81:             "installRoot": "${projectDir}\\output\\install\\${name}",
82:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3",
83:             "buildCommandArgs": "",
84:             "ctestCommandArgs": "",
85:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
86:             "inheritEnvironments": [ "gcc-arm" ]
87:         },
88:         {
89:             "name": "BareMetal-RPI4-RelWithDebInfo",
90:             "generator": "Ninja",
91:             "configurationType": "RelWithDebInfo",
92:             "buildRoot": "${projectDir}\\cmake-${name}",
93:             "installRoot": "${projectDir}\\output\\install\\${name}",
94:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4",
95:             "buildCommandArgs": "",
96:             "ctestCommandArgs": "",
97:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
98:             "inheritEnvironments": [ "gcc-arm" ]
99:         },
100:         {
101:             "name": "BareMetal-RPI5-RelWithDebInfo",
102:             "generator": "Ninja",
103:             "configurationType": "RelWithDebInfo",
104:             "buildRoot": "${projectDir}\\cmake-${name}",
105:             "installRoot": "${projectDir}\\output\\install\\${name}",
106:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5",
107:             "buildCommandArgs": "",
108:             "ctestCommandArgs": "",
109:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
110:             "inheritEnvironments": [ "gcc-arm" ]
111:         },
112:         {
113:             "name": "BareMetal-RPI3-MinSizeRel",
114:             "generator": "Ninja",
115:             "configurationType": "MinSizeRel",
116:             "buildRoot": "${projectDir}\\cmake-${name}",
117:             "installRoot": "${projectDir}\\output\\install\\${name}",
118:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3",
119:             "buildCommandArgs": "",
120:             "ctestCommandArgs": "",
121:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
122:             "inheritEnvironments": [ "gcc-arm" ]
123:         },
124:         {
125:             "name": "BareMetal-RPI4-MinSizeRel",
126:             "generator": "Ninja",
127:             "configurationType": "MinSizeRel",
128:             "buildRoot": "${projectDir}\\cmake-${name}",
129:             "installRoot": "${projectDir}\\output\\install\\${name}",
130:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI4",
131:             "buildCommandArgs": "",
132:             "ctestCommandArgs": "",
133:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
134:             "inheritEnvironments": [ "gcc-arm" ]
135:         },
136:         {
137:             "name": "BareMetal-RPI5-MinSizeRel",
138:             "generator": "Ninja",
139:             "configurationType": "MinSizeRel",
140:             "buildRoot": "${projectDir}\\cmake-${name}",
141:             "installRoot": "${projectDir}\\output\\install\\${name}",
142:             "cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI5",
143:             "buildCommandArgs": "",
144:             "ctestCommandArgs": "",
145:             "cmakeToolchain": "${projectDir}\\baremetal.toolchain",
146:             "inheritEnvironments": [ "gcc-arm" ]
147:         }
148:     ]
149:   }
```

## Update demo CMake file {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_UPDATE_DEMO_CMAKE_FILE}

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
28:     )
29: 
30: set(PROJECT_LIBS
31:     ${LINKER_LIBRARIES}
32:     ${PROJECT_DEPENDENCIES}
33:     )
34: 
35: set(PROJECT_SOURCES
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/start.S
38:     )
39: 
40: set(PROJECT_INCLUDES_PUBLIC )
41: set(PROJECT_INCLUDES_PRIVATE )
42: 
43: if (CMAKE_VERBOSE_MAKEFILE)
44:     display_list("Package                           : " ${PROJECT_NAME} )
45:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
46:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
47:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
48:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
49:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
50:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
51:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
52:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
53:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
54:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
55:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
56:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
57:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
58:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
59:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
60:     display_list("Link libs                         : " ${PROJECT_LIBS} )
61:     display_list("Source files                      : " ${PROJECT_SOURCES} )
62:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
63:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
64: endif()
65: 
66: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
67: 
68: target_link_libraries(${PROJECT_NAME} ${LINKER_START_GROUP} ${PROJECT_LIBS} ${LINKER_END_GROUP})
69: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
70: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
71: target_compile_definitions(${PROJECT_NAME} PRIVATE
72:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
73:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
74:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
75:     )
76: target_compile_definitions(${PROJECT_NAME} PUBLIC
77:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
78:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
79:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
80:     )
81: target_compile_options(${PROJECT_NAME} PRIVATE
82:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
83:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
84:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
85:     )
86: target_compile_options(${PROJECT_NAME} PUBLIC
87:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
88:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
89:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
90:     )
91: 
92: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
93: 
94: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
95: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
96:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
97: endif()
98: 
99: link_directories(${LINK_DIRECTORIES})
100: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
101: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
102: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
103: 
104: show_target_properties(${PROJECT_NAME})
105: 
set(BAREMETAL_EXECUTABLE_TARGET ${PROJECT_NAME})
setup_image(${PROJECT_NAME})
```

- Line 14-15: We can now define the compiler definitions for C++ using the variables set up for C/C++ compiler definitions. The public settings are left empty
- Line 16-17: Similarly for the compiler definitions for assembly using the variables set up for assembly compiler definitions. The public settings are left empty
- Line 18-19: Similarly for the compiler options for C++ using the variables set up for C++ compiler options. The public settings are left empty
- Line 20-21: Similarly for the compiler options for assembly using the variables set up for assembly compiler options. The public settings are left empty
- Line 25: Similarly for linker options using the variables set up for linker options
- Line 30-33: We can now link to all standard libraries by adding `LINKER_LIBRARIES` to `PROJECT_LIBS`.
Note that for now this list is empty
- Line 43-64: Using the custom CMake function `display_list` we can now print all settings for the project.
This is only done if verbose output is requested
- Line 71-90: The compiler definitions and options are set as before
- Line 92: We can now use the `SUPPORTED_CPP_STANDARD` to set the C++ standard property on the target
- Line 99: We set the search path for standard libraries using `LINK_DIRECTORIES`
- Line 104: Using the custom CMake function `show_target_properties` we can now print all target properties.

## Configure and build {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_CONFIGURE_AND_BUILD}

We are now able to configure the project again, and build it.

The output for the configure step should be similar to:

```text
1> CMake generation started for configuration: 'BareMetal-RPI3-Debug'.
1> Command line: "C:\WINDOWS\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="D:\Projects\baremetal\output\install\BareMetal-RPI3-Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH="D:\Projects\baremetal\baremetal.toolchain" -DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DCMAKE_MAKE_PROGRAM="C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" "D:\Projects\baremetal" 2>&1"
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
1> [CMake] ## In directory: D:/Projects/baremetal/code
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal/code/applications
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal/code/applications/demo
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
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      : 
1> [CMake] -- Link libs                         : 
1> [CMake] -- Source files                      :  D:/Projects/baremetal/code/applications/demo/src/main.cpp D:/Projects/baremetal/code/applications/demo/src/start.S
1> [CMake] -- Include files - public            : 
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] -- Properties for demo
1> [CMake] -- Target type                       :  EXECUTABLE
1> [CMake] -- Target defines                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3 _DEBUG> $<$<COMPILE_LANGUAGE:ASM>:PLATFORM_BAREMETAL BAREMETAL_RPI_TARGET=3> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target options                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target include dirs public        :  INCLUDES-NOTFOUND
1> [CMake] -- Target include dirs private       :  INCLUDES-NOTFOUND
1> [CMake] -- Target link libraries             :  -Wl,--start-group -Wl,--end-group
1> [CMake] -- Target link options               :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target exported defines           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported options           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported include dirs      :  INCLUDE_DIRS_EXPORTS-NOTFOUND
1> [CMake] -- Target exported link libraries    :  -Wl,--start-group -Wl,--end-group
1> [CMake] -- Target imported dependencies      : 
1> [CMake] -- Target imported link libraries    : 
1> [CMake] -- Target link dependencies          :  LINK_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target manual dependencies        :  EXPLICIT_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target static library location    :  D:/Projects/baremetal/output/RPI3/Debug/lib
1> [CMake] -- Target dynamic library location   :  LIBRARY_LOCATION-NOTFOUND
1> [CMake] -- Target binary location            :  D:/Projects/baremetal/output/RPI3/Debug/bin
1> [CMake] -- Target link flags                 :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target version                    :  TARGET_VERSION-NOTFOUND
1> [CMake] -- Target so-version                 :  TARGET_SOVERSION-NOTFOUND
1> [CMake] -- Target output name                :  demo.elf
1> [CMake] -- Target C++ standard               :  17
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal/code/applications/demo
1> [CMake] 
1> [CMake] ** Setting up demo-image **
1> [CMake] 
1> [CMake] -- create_image demo-image kernel8.img demo
1> [CMake] -- TARGET_NAME demo.elf
1> [CMake] -- generate D:/Projects/baremetal/deploy/Debug/demo-image/kernel8.img from D:/Projects/baremetal/output/RPI3/Debug/bin/demo
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal/code/libraries
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal/tutorial
1> [CMake] -- Configuring done
1> [CMake] -- Generating done
1> [CMake] -- Build files have been written to: D:/Projects/baremetal/cmake-Baremetal-RPI3-Debug
1> Extracted CMake variables.
1> Extracted source files and headers.
1> Extracted code model.
1> Extracted toolchain configurations.
1> Extracted includes paths.
1> CMake generation finished.
```

As you can see, all the configured demo project is shown, with its settings. This is very helpful in finding configuration problems.

We can then build for Raspberry Pi 3:

```text
>------ Build All started: Project: baremetal, Configuration: BareMetal-RPI3-Debug ------
  [1/4] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [2/4] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -c ../code/applications/demo/src/start.S
  [3/4] cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -o ..\output\RPI3\Debug\bin\demo.elf  -Wl,--start-group  -Wl,--end-group && cd ."
  [4/4] cmd.exe /C "cd /D D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug\code\applications\demo && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E make_directory D:/Projects/baremetal/deploy/Debug/demo-image && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal/output/RPI3/Debug/bin/demo.elf -O binary D:/Projects/baremetal/deploy/Debug/demo-image/kernel8.img"

Build All succeeded.
```

Or Raspberry Pi 4:

```text
>------ Build All started: Project: baremetal, Configuration: BareMetal-RPI4-Debug ------
  [1/4] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_RPI_TARGET=4 -DPLATFORM_BAREMETAL  -g -mcpu=cortex-a72 -mlittle-endian -mcmodel=small -O2 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -c ../code/applications/demo/src/start.S
  [2/4] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=4 -DPLATFORM_BAREMETAL -D_DEBUG  -g -mcpu=cortex-a72 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [3/4] cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -o ..\output\RPI4\Debug\bin\demo.elf  -Wl,--start-group  -Wl,--end-group && cd ."
  [4/4] cmd.exe /C "cd /D D:\Projects\baremetal\cmake-BareMetal-RPI4-Debug\code\applications\demo && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E make_directory D:/Projects/baremetal/deploy/Debug/demo-image && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal/output/RPI4/Debug/bin/demo.elf -O binary D:/Projects/baremetal/deploy/Debug/demo-image/kernel8-rpi4.img"

Build All succeeded.
```

Or Raspberry Pi 5:

```text
>------ Build All started: Project: baremetal, Configuration: BareMetal-RPI5-Debug ------
  [1/4] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=5 -DPLATFORM_BAREMETAL -D_DEBUG  -g -mcpu=cortex-a76 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [2/4] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_RPI_TARGET=5 -DPLATFORM_BAREMETAL  -g -mcpu=cortex-a76 -mlittle-endian -mcmodel=small -O2 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -c ../code/applications/demo/src/start.S
  [3/4] cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -o ..\output\RPI5\Debug\bin\demo.elf  -Wl,--start-group  -Wl,--end-group && cd ."
  [4/4] cmd.exe /C "cd /D D:\Projects\baremetal\cmake-BareMetal-RPI5-Debug\code\applications\demo && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E make_directory D:/Projects/baremetal/deploy/Debug/demo-image && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal/output/RPI5/Debug/bin/demo.elf -O binary D:/Projects/baremetal/deploy/Debug/demo-image/kernel_2712.img"

Build All succeeded.
```

Notice that the compiler settings used are different between the platforms.

You can see that the startup code is compiled (step 1), as well as the demo application's main.cpp (step 2).
The demo application is linked (step 3), and finally the image is created (step 4).
It takes a while to start understanding, even with the description of [Common compiler directives](#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMMON_COMPILER_DIRECTIVES),  what the compiler and linker actually do, so we'll dive a bit deeper here.
For this we'll have a look at the build output shown above:

## Compiling startup code {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_STARTUP_CODE}

When compiling `code/applications/demo/src/start.S` for Raspberry Pi 3 on Windows:
```text
D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe
  -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL
  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
  -O2
  -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d
  -o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
  -c ../code/applications/demo/src/start.S
```

### Compiler {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_STARTUP_CODE_COMPILER}

The first part is the compiler call, the rest are command line parameters for the compiler.
For the startup code (which is assembly), this is `aarch64-none-elf-gcc.exe` (or in Linux `aarch64-none-elf-gcc`).

### Assembler definitions {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_STARTUP_CODE_ASSEMBLER_DEFINITIONS}

The -D options specify all compiler definitions.
You will recognize the options specified in CMakeSettings.json: `"cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3"`.
- The first option triggers setting the `CMAKE_VERBOSE_MAKEFILE` setting, that shows the configuration output:

```
File: CMakeLists.txt
25: if (VERBOSE_BUILD)
26:     set(CMAKE_VERBOSE_MAKEFILE ON CACHE STRING "Verbose build" FORCE)
27: else()
28:     set(CMAKE_VERBOSE_MAKEFILE OFF CACHE STRING "Verbose build" FORCE)
29: endif()
```

- The second option triggers setting the `BAREMETAL_RPI_TARGET=3` definition. Which is converted from `BAREMETAL_TARGET` in the main CMake file:

```
File: CMakeLists.txt
55: if (BAREMETAL_TARGET STREQUAL "RPI3")
56:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
57:     set(BAREMETAL_TARGET_KERNEL kernel8)
58:     set(BAREMETAL_RPI_TARGET 3)
59: elseif (BAREMETAL_TARGET STREQUAL "RPI4")
60:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a72 -mlittle-endian -mcmodel=small)
61:     set(BAREMETAL_TARGET_KERNEL kernel8-rpi4)
62:     set(BAREMETAL_RPI_TARGET 4)
63: elseif (BAREMETAL_TARGET STREQUAL "RPI5")
64:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a76 -mlittle-endian -mcmodel=small)
65:     set(BAREMETAL_TARGET_KERNEL kernel_2712)
66:     set(BAREMETAL_RPI_TARGET 5)
67: endif()
```

- The `PLATFORM_BAREMETAL` definition is triggered by the main CMake file:

```cmake
File: CMakeLists.txt
78: set(DEFINES_ASM
79:     PLATFORM_BAREMETAL
80:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
81:     )
```

- The generic and the build configuration specific definitions are later combined:
```cmake
File: CMakeLists.txt
165: set(COMPILE_DEFINITIONS_ASM_DEBUG ${DEFINES_ASM} ${DEFINES_ASM_DEBUG})
166: set(COMPILE_DEFINITIONS_ASM_RELEASE ${DEFINES_ASM} ${DEFINES_ASM_RELEASE})
167: set(COMPILE_DEFINITIONS_ASM_MINSIZEREL ${DEFINES_ASM} ${DEFINES_ASM_MINSIZEREL})
168: set(COMPILE_DEFINITIONS_ASM_RELWITHDEBINFO ${DEFINES_ASM} ${DEFINES_ASM_RELWITHDEBINFO})
```

- However we don't set any build configuration specific assembly definitions
- And then later variable for the specific build configuration are set:
```cmake
File: CMakeLists.txt
192: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
193:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
194:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
195:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
196:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
197:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
198:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
```

- The `COMPILE_DEFINITIONS_ASM` variable is then used in the project CMake file (`code/applications/demo/CMakeLists.txt`):

```cmake
File: code/applications/demo/CMakeLists.txt
16: set(PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE ${COMPILE_DEFINITIONS_ASM})
17: set(PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC )
```

- Then the target linker options are set on the target as a property using this veriable:

```cmake
File: code/applications/demo/CMakeLists.txt
71: target_compile_definitions(${PROJECT_NAME} PRIVATE
72:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
73:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
74:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
75:     )
76: target_compile_definitions(${PROJECT_NAME} PUBLIC
77:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
78:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
79:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
80:     )
```

Where the `COMPILE_LANGUAGE:ASM` part is used as we are compiling assembly files.

### Assembler options {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_STARTUP_CODE_ASSEMBLER_OPTIONS}

The complete list below are all compiler options:

```text
-g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
-O2 
-MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d
```

- The first -g is automatically added by CMake as we are building for a debug configuration.
This makes the compiler generate debug symbols
- The `-mcpu=cortex-a53 -mlittle-endian -mcmodel=small` options are set due to the fact that we are building for Raspberry Pi 3:

```cmake
File: CMakeLists.txt
55: if (BAREMETAL_TARGET STREQUAL "RPI3")
56:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
57:     set(BAREMETAL_TARGET_KERNEL kernel8)
58:     set(BAREMETAL_RPI_TARGET 3)
```
- The options all are defined later on in the main CMake file, where they are combinex with `BAREMETAL_ARCH_CPU_OPTIONS`:

```cmake
File: CMakeLists.txt
134: set(FLAGS_ASM ${BAREMETAL_ARCH_CPU_OPTIONS})
```

- The `-O2` option is set for the specific Debug build configuration:

```cmake
File: CMakeLists.txt
135: set(FLAGS_ASM_DEBUG -O2)
136: set(FLAGS_ASM_RELEASE -O2)
137: set(FLAGS_ASM_MINSIZEREL -O2)
138: set(FLAGS_ASM_RELWITHDEBINFO -O2)
```

- The generic and the build configuration specific definitions are later combined:

```cmake
File: CMakeLists.txt
180: set(COMPILE_OPTIONS_ASM_DEBUG ${FLAGS_ASM} ${FLAGS_ASM_DEBUG})
181: set(COMPILE_OPTIONS_ASM_RELEASE ${FLAGS_ASM} ${FLAGS_ASM_RELEASE})
182: set(COMPILE_OPTIONS_ASM_MINSIZEREL ${FLAGS_ASM} ${FLAGS_ASM_MINSIZEREL})
183: set(COMPILE_OPTIONS_ASM_RELWITHDEBINFO ${FLAGS_ASM} ${FLAGS_ASM_RELWITHDEBINFO})
```

- And then later variable for the specific build configuration are set:

```cmake
File: CMakeLists.txt
192: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
193:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
194:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
195:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
196:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
197:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
198:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
```

- Finally, the following options are added automatically by CMake, to generate a dependency file.

```
-MD
-MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
-MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d
```

- The `COMPILE_OPTIONS_ASM` variable is then used in the project CMake file (`code/applications/demo/CMakeLists.txt`):

```cmake
File: code/applications/demo/CMakeLists.txt
20: set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
21: set(PROJECT_COMPILE_OPTIONS_ASM_PUBLIC )
```

- Then the target linker options are set on the target as a property using this veriable:

```cmake
File: code/applications/demo/CMakeLists.txt
81: target_compile_options(${PROJECT_NAME} PRIVATE
82:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
83:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
84:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
85:     )
86: target_compile_options(${PROJECT_NAME} PUBLIC
87:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
88:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
89:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
90:     )
```

Where the `COMPILE_LANGUAGE:ASM` part is used as we are compiling assembly files.

### Actual assembly {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_STARTUP_CODE_ACTUAL_ASSEMBLY}

The final parameters determine the source files compiled, and the resulting object file:

```text
-o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
-c ../code/applications/demo/src/start.S
```

The -c option tells the compiler to compile the specified file, the -o option tells it to generate the specified object file.
Notice that the path for the object file is relative to the CMake build directory, in this case `cmake-BareMetal-RPI3-Debug`.

## Compiling source code {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_SOURCE_CODE}

```text
D:\toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe
  -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG
  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
  -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new
  -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics
   -nostdinc -nostdlib -nostdinc++
  -fno-exceptions -fno-rtti
  -O0 -Wno-unused-variable -Wno-unused-parameter
  -std=gnu++17
  -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d
  -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
  -c ../code/applications/demo/src/main.cpp
```

### Compiler {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_SOURCE_CODE_COMPILER}

The first part is the compiler call, the rest are command line parameters for the compiler.
For the C++ code this is `aarch64-none-elf-g++.exe` (or in Linux `aarch64-none-elf-g++`).

### Compiler definitions {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_SOURCE_CODE_COMPILER_DEFINITIONS}

The -D options specify all compiler definitions.
You will recognize the options specified in CMakeSettings.json: `"cmakeCommandArgs": "-DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3"`.
- The first option triggers setting the `CMAKE_VERBOSE_MAKEFILE` setting, that shows the configuration output:

```
File: CMakeLists.txt
25: if (VERBOSE_BUILD)
26:     set(CMAKE_VERBOSE_MAKEFILE ON CACHE STRING "Verbose build" FORCE)
27: else()
28:     set(CMAKE_VERBOSE_MAKEFILE OFF CACHE STRING "Verbose build" FORCE)
29: endif()
```

- The second triggers setting the `BAREMETAL_RPI_TARGET=3` definition. Which is converted from `BAREMETAL_TARGET` in the main CMake file:

```
File: CMakeLists.txt
55: if (BAREMETAL_TARGET STREQUAL "RPI3")
56:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
57:     set(BAREMETAL_TARGET_KERNEL kernel8)
58:     set(BAREMETAL_RPI_TARGET 3)
59: elseif (BAREMETAL_TARGET STREQUAL "RPI4")
60:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a72 -mlittle-endian -mcmodel=small)
61:     set(BAREMETAL_TARGET_KERNEL kernel8-rpi4)
62:     set(BAREMETAL_RPI_TARGET 4)
63: elseif (BAREMETAL_TARGET STREQUAL "RPI5")
64:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a76 -mlittle-endian -mcmodel=small)
65:     set(BAREMETAL_TARGET_KERNEL kernel_2712)
66:     set(BAREMETAL_RPI_TARGET 5)
67: endif()
```

- The `PLATFORM_BAREMETAL` definition is triggered by the main CMake file:

```cmake
File: CMakeLists.txt
70: set(DEFINES_C
71:     PLATFORM_BAREMETAL
72:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
73:     )
```

- The \_DEBUG definition is caused by the `CMAKE_BUILD_TYPE` variable, which is set to `Debug` as we are building a Debug application.
- This is also done in the main CMake file:

```cmake
File: CMakeLists.txt
74: set(DEFINES_C_DEBUG _DEBUG)
75: set(DEFINES_C_RELEASE NDEBUG)
76: set(DEFINES_C_MINSIZEREL NDEBUG)
77: set(DEFINES_C_RELWITHDEBINFO NDEBUG)
```

- The generic and the build configuration specific definitions are later combined:

```cmake
File: CMakeLists.txt
160: set(COMPILE_DEFINITIONS_C_DEBUG ${DEFINES_C} ${DEFINES_C_DEBUG})
161: set(COMPILE_DEFINITIONS_C_RELEASE ${DEFINES_C} ${DEFINES_C_RELEASE})
162: set(COMPILE_DEFINITIONS_C_MINSIZEREL ${DEFINES_C} ${DEFINES_C_MINSIZEREL})
163: set(COMPILE_DEFINITIONS_C_RELWITHDEBINFO ${DEFINES_C} ${DEFINES_C_RELWITHDEBINFO})
```

- And then later variable for the specific build configuration are set:

```cmake
File: CMakeLists.txt
192: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
193:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
194:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
195:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
196:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
197:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
198:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
```

- The `COMPILE_DEFINITIONS_C` variable is then used in the project CMake file (`code/applications/demo/CMakeLists.txt`):

```cmake
File: code/applications/demo/CMakeLists.txt
14: set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE ${COMPILE_DEFINITIONS_C})
15: set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
```

- Then the target linker options are set on the target as a property using this veriable:

```cmake
File: code/applications/demo/CMakeLists.txt
71: target_compile_definitions(${PROJECT_NAME} PRIVATE
72:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
73:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
74:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
75:     )
76: target_compile_definitions(${PROJECT_NAME} PUBLIC
77:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
78:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
79:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
80:     )
```

Where the `COMPILE_LANGUAGE:CXX` part is used as we are compiling C++ source files.

### Compiler options {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_SOURCE_CODE_COMPILER_OPTIONS}

The complete list below are all compiler options:

```text
-g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small
-Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char
-nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -g -Wno-unused-variable -Wno-unused-parameter
-std=gnu++17
-MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d
```

- The first -g is automatically added by CMake as we are building for a debug configuration.
This makes the compiler generate debug symbols
- The `-mcpu=cortex-a53 -mlittle-endian -mcmodel=small` options are set due to the fact that we are building for Raspberry Pi 3:

```cmake
File: CMakeLists.txt
55: if (BAREMETAL_TARGET STREQUAL "RPI3")
56:     set(BAREMETAL_ARCH_CPU_OPTIONS -mcpu=cortex-a53 -mlittle-endian -mcmodel=small)
57:     set(BAREMETAL_TARGET_KERNEL kernel8)
58:     set(BAREMETAL_RPI_TARGET 3)
```

- The `-Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti` options all are defined later on in the main CMake file, where they are combined with `BAREMETAL_ARCH_CPU_OPTIONS`:

```cmake
File: CMakeLists.txt
106: set(FLAGS_CXX
107:     ${BAREMETAL_ARCH_CPU_OPTIONS}
108:     -Wall
109:     -Wextra
110:     -Werror
111:     -Wno-missing-field-initializers
112:     -Wno-unused-value
113:     -Wno-aligned-new
114:     -ffreestanding
115:     -fsigned-char
116:     -nostartfiles
117:     -mno-outline-atomics
118:     -nostdinc
119:     -nostdlib
120:     -nostdinc++
121:     -fno-exceptions
122:     -fno-rtti
123:     )
```

- The `-O0 -Wno-unused-variable -Wno-unused-parameter` options are set for the specific Debug build configuration:

```cmake
File: CMakeLists.txt
125: # -g is added by CMake
126: set(FLAGS_CXX_DEBUG -O0 -Wno-unused-variable -Wno-unused-parameter)
127: # -O3 is added by CMake
128: set(FLAGS_CXX_RELEASE -D__USE_STRING_INLINES)
129: # -Os is added by CMake
130: set(FLAGS_CXX_MINSIZEREL -O3)
131: # -O2 -g is added by CMake
132: set(FLAGS_CXX_RELWITHDEBINFO )
```

- The generic and the build configuration specific definitions are later combined:

```cmake
File: CMakeLists.txt
175: set(COMPILE_OPTIONS_CXX_DEBUG ${FLAGS_CXX} ${FLAGS_CXX_DEBUG})
176: set(COMPILE_OPTIONS_CXX_RELEASE ${FLAGS_CXX} ${FLAGS_CXX_RELEASE})
177: set(COMPILE_OPTIONS_CXX_MINSIZEREL ${FLAGS_CXX} ${FLAGS_CXX_MINSIZEREL})
178: set(COMPILE_OPTIONS_CXX_RELWITHDEBINFO ${FLAGS_CXX} ${FLAGS_CXX_RELWITHDEBINFO})
```

- And then later variable for the specific build configuration are set:

```cmake
File: CMakeLists.txt
192: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
193:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
194:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
195:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
196:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
197:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
198:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
```

- The option `-std=gnu++17` is automatically added by CMake due to setting the variable `SUPPORTED_CPP_STANDARD` in the main CMake file:

```cmake
File: CMakeLists.txt
154: set(SUPPORTED_CPP_STANDARD 17)
```

- Which is then later used when defining the target properties in the `code/application/demo` CMake file:

```cmake
File: code/applications/demo/CMakeLists.txt
97: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
```

- Finally, the following options are added automatically by CMake, to generate a dependency file.
```
-MD
-MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
-MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d
```

- The `COMPILE_OPTIONS_CXX` variable is then used in the project CMake file (`code/applications/demo/CMakeLists.txt`):

```cmake
File: code/applications/demo/CMakeLists.txt
18: set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
19: set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
```

- Then the target linker options are set on the target as a property using this veriable:

```cmake
File: code/applications/demo/CMakeLists.txt
81: target_compile_options(${PROJECT_NAME} PRIVATE
82:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
83:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
84:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
85:     )
86: target_compile_options(${PROJECT_NAME} PUBLIC
87:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
88:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
89:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
90:     )
```

Where the `COMPILE_LANGUAGE:CXX` part is used as we are compiling C++ source files.

### Actual compilation {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_COMPILING_SOURCE_CODE_ACTUAL_COMPILATION}

The final parameters determine the source files compiled, and the resulting object file:

```text
-o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
-c ../code/applications/demo/src/main.cpp
```

The -c option tells the compiler to compile the specified file, the -o option tells it to generate the specified object file.
Notice that the paths are relative to the CMake build directory, in this case `cmake-BareMetal-RPI3-Debug`.

## Linking the demo application {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_LINKING_THE_DEMO_APPLICATION}

```text
cmd.exe /C
 "cd . &&
  D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe
   -g
   -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
   -Wl,--section-start=.init=0x80000
   -T D:/Projects/baremetal/baremetal.ld
   -nostdlib -nostartfiles
   code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
   code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
   -o ..\output\RPI3\Debug\bin\demo.elf
   -Wl,--start-group  -Wl,--end-group &&
  cd ."
```

Here we see a total of 3 commands being performed inside a command shell:

- The first is just a cd command (actually moving to the same directory)
- The second links the exeutable file
- The third is again a cd command to the same directory

### Linker {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_LINKING_THE_DEMO_APPLICATION_LINKER}

The first part is the linker call, the rest are command line parameters for the linker.
As you can see this is `aarch64-none-elf-g++.exe` (or in Linux `aarch64-none-elf-g++`).
So the compiler is also used for linking.

### Linker options {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_LINKING_THE_DEMO_APPLICATION_LINKER_OPTIONS}

Options used when linking are:
```text
   -g
   -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1
   -Wl,--section-start=.init=0x80000
   -T D:/Projects/baremetal/baremetal.ld
   -nostdlib -nostartfiles
   code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
   code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
   -o ..\output\RPI3\Debug\bin\demo.elf
   -Wl,--start-group  -Wl,--end-group
```

- The first -g is automatically added by CMake as we are building for a debug configuration.
This makes the linker generate debug symbols
- The `-LD:\toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1` option is added by the toolchain:
```cmake
File: baremetal.toolchain
23: set(TOOLCHAIN_AUXILIARY_PATH ${TOOLCHAIN_ROOT}/lib/gcc/${TOOL_DESTINATION_PLATFORM}/13.3.1)
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
- The options `-Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles` are set in the main CMake file:

```cmake
File: CMakeLists.txt
140: set(LINK_FLAGS
141:     -Wl,--section-start=.init=${BAREMETAL_LOAD_ADDRESS}
142:     -T ${CMAKE_SOURCE_DIR}/baremetal.ld
143:     -nostdlib
144:     -nostartfiles
145:     )
146: set(LINK_FLAGS_DEBUG )
147: set(LINK_FLAGS_RELEASE )
148: set(LINK_FLAGS_MINSIZEREL )
149: set(LINK_FLAGS_RELWITHDEBINFO )
```

- This is then combined later on with the build configuration specific linker flags:

```cmake
File: CMakeLists.txt
185: set(LINKER_OPTIONS_DEBUG ${LINK_FLAGS} ${LINK_FLAGS_DEBUG})
186: set(LINKER_OPTIONS_RELEASE ${LINK_FLAGS} ${LINK_FLAGS_RELEASE})
187: set(LINKER_OPTIONS_MINSIZEREL ${LINK_FLAGS} ${LINK_FLAGS_MINSIZEREL})
188: set(LINKER_OPTIONS_RELWITHDEBINFO ${LINK_FLAGS} ${LINK_FLAGS_RELWITHDEBINFO})
```

- Subsequently, the linker flags for the selected build configuration are set:

```cmake
File: CMakeLists.txt
192: if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
193:     set(COMPILE_DEFINITIONS_C ${COMPILE_DEFINITIONS_C_DEBUG})
194:     set(COMPILE_DEFINITIONS_ASM ${COMPILE_DEFINITIONS_ASM_DEBUG})
195:     set(COMPILE_OPTIONS_C ${COMPILE_OPTIONS_C_DEBUG})
196:     set(COMPILE_OPTIONS_CXX ${COMPILE_OPTIONS_CXX_DEBUG})
197:     set(COMPILE_OPTIONS_ASM ${COMPILE_OPTIONS_ASM_DEBUG})
198:     set(LINKER_OPTIONS ${LINKER_OPTIONS_DEBUG})
```

- The `LINKER_OPTIONS` variable is then used in the project CMake file (`code/applications/demo/CMakeLists.txt`):

```cmake
File: code/applications/demo/CMakeLists.txt
25: set(PROJECT_LINK_OPTIONS ${LINKER_OPTIONS})
```

- Then the target linker options are set on the target as a property using this veriable:

```cmake
File: code/applications/demo/CMakeLists.txt
99: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
100: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
101:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
102: endif()
```

- What follows is a list of object files to be linked, in this case just two:

```text
code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj
code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj
```

- This list is determined by the source files specified for the target:

```cmake
File: code/applications/demo/CMakeLists.txt
35: set(PROJECT_SOURCES
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/start.S
38:     )
39: 
40: set(PROJECT_INCLUDES_PUBLIC )
41: set(PROJECT_INCLUDES_PRIVATE )
...
71: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
```

- Next is the option `-o ..\output\RPI3\Debug\bin\demo.elf`, which specifies the output of the linker, the application.
This is determined by the specification of the name and location of the target file in the project CMake file:

```cmake
File: code/applications/demo/CMakeLists.txt
12: set(PROJECT_TARGET_NAME ${PROJECT_NAME}.elf)
...
105: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
106: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
107: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
```

- As we are building an executable, the location will be the `OUTPUT_BIN_DIR`, the file name will be `PROJECT_TARGET_NAME`.
- Finally, the list of libraries to link to is specified, which is for now empty, but surrounded by the group begin and end:

```
File: CMakeLists.txt
190: if (PLATFORM_BAREMETAL)
191:     set(LINKER_START_GROUP -Wl,--start-group)
192:     set(LINKER_END_GROUP -Wl,--end-group)
193: endif()
```

- This is determined by the list of libraries to link to in the project CMake file:

```cmake
File: code/applications/demo/CMakeLists.txt
27: set(PROJECT_DEPENDENCIES
28:     )
29: 
30: set(PROJECT_LIBS
31:     ${LINKER_LIBRARIES}
32:     ${PROJECT_DEPENDENCIES}
33:     )
...
68: target_link_libraries(${PROJECT_NAME} ${LINKER_START_GROUP} ${PROJECT_LIBS} ${LINKER_END_GROUP})
```

- The reason libraries linked to is placed in a group (`-Wl,--start-group <libraries> -Wl,--end-group`) is that the GCC linker tries to resolve symbols in reverse order, i.e. expects the lowest level symbols to be added last, and we cannot always guarantee the correct order of libraries.
By putting them in a group, the linker will iterate as many times as needed to resolve all symbols, if possible of course.

## Creating the kernel image {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_CREATING_THE_KERNEL_IMAGE}

```text
cmd.exe /C 
    cd /D D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug\code\applications\demo &&
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        -E make_directory D:/Projects/baremetal/deploy/Debug/demo-image &&
    D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe
        D:/Projects/baremetal/output/RPI3/Debug/bin/demo.elf
        -O binary
        D:/Projects/baremetal/deploy/Debug/demo-image/kernel8.img
```

Here we see a 3 commands being performed inside a command shell:

- The first is just a cd command (moving into the demo application directory)
- The second runs CMake to create the directory for the image file (the -E option runs CMake specific commands, in this case `make_directory` to create a directory if it does not exist)
- The third creates the image file
  - This runs the objcopy tool `aarch64-none-elf-objcopy.exe` (or on Linux `aarch64-none-elf-objcopy`)
  - Its first parameter is the executable file to be placed in the image
  - The second parameter (specified with option `-O`) specifies the image file to be created. The option `binary` makes sure the executable is copied into the image file verbatim, i.e. without conversions

## Running the application {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_RUNNING_THE_APPLICATION}

### Windows {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_RUNNING_THE_APPLICATION_WINDOWS}

Start QEMU listening to UART0 for the demo project:

```bat
tools\startQEMU-image-uart0-rpi3.bat demo
```

or

```bat
tools\startQEMU-image-uart0-rpi4.bat demo
```

### Linux {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_RUNNING_THE_APPLICATION_LINUX}

Start QEMU listening to UART0 for the demo project:

```bat
tools/startQEMU-image-uart0-rpi3.sh demo
```

Start debugging as shown in [Visual Studio CMake integration](#TUTORIAL_03_SETTING_UP_PROJECT_STRUCTURE_VISUAL_STUDIO_CMAKE_INTEGRATION). Make sure the demo applications is selected as startup project.

## Tutorial code setup {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_TUTORIAL_CODE_SETUP}

Starting from here, we'll be adding the code we create in a tutorial into the tutorial folder, and add it as a project to CMake.
We already added a CMake file to the `tutorial` directory, and for every chapter, we'll add a new folder here.
As we cannot have two executables with the same name in CMake, we'll be renaming the one in the tutorial folder.

### Adding the code {#TUTORIAL_04_SETTING_UP_COMMON_OPTIONS_TUTORIAL_CODE_SETUP_ADDING_THE_CODE}

We'll add a CMake file under the subdirectory named after this tutorial file, and copy the code there, as well as a new CMake file:

<img src="images/tutorial-code-for-chapter-04.png" alt="Tutorial folder structure" width="600"/>

The code will simply be copied, but the demo CMake file will be changed slightly:

```cmake
File: tutorial/04-setting-up-common-options/code/applications/demo/CMakeLists.txt
1: project(04-setting-up-common-options
2:     DESCRIPTION "Tutorial 04 Setting up common options"
3:     LANGUAGES CXX ASM)
4: 
5: message(STATUS "\n**********************************************************************************\n")
6: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
```

The extra CMake file for this chapter will simply include the code folder:
```cmake
File: tutorial/04-setting-up-common-options/CMakeLists.txt
1: add_subdirectory(code)
```

Also, we'll add the folder for the code to the CMake file under `tutorial`:

```cmake
File: tutorial/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3: 
4: add_subdirectory(04-setting-up-common-options)
```

A similar structure will be created for each chapter from now on, which will configure and build the executables for each tutorial chapter next to the main code:

```text
1> [CMake] ## In directory: D:/Projects/baremetal/tutorial/04-setting-up-common-options/code/applications/demo
1> [CMake] 
1> [CMake] ** Setting up 04-setting-up-common-options-image **
1> [CMake] 
1> [CMake] -- create_image 04-setting-up-common-options-image kernel8.img 04-setting-up-common-options
1> [CMake] -- TARGET_NAME 04-setting-up-common-options.elf
1> [CMake] -- generate D:/Projects/baremetal/deploy/Debug/04-setting-up-common-options-image/kernel8.img from D:/Projects/baremetal/output/RPI3/Debug/bin/04-setting-up-common-options
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal/tutorial/04-setting-up-common-options/code/libraries
```

```text
>------ Build All started: Project: baremetal, Configuration: BareMetal-RPI3-Debug ------
  [1/8] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\start.S.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -c ../code/applications/demo/src/start.S
  [2/8] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2 -MD -MT tutorial/04-setting-up-common-options/code/applications/demo/CMakeFiles/04-setting-up-common-options.dir/src/start.S.obj -MF tutorial\04-setting-up-common-options\code\applications\demo\CMakeFiles\04-setting-up-common-options.dir\src\start.S.obj.d -o tutorial/04-setting-up-common-options/code/applications/demo/CMakeFiles/04-setting-up-common-options.dir/src/start.S.obj -c ../tutorial/04-setting-up-common-options/code/applications/demo/src/start.S
  [3/8] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [4/8] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG  -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT tutorial/04-setting-up-common-options/code/applications/demo/CMakeFiles/04-setting-up-common-options.dir/src/main.cpp.obj -MF tutorial\04-setting-up-common-options\code\applications\demo\CMakeFiles\04-setting-up-common-options.dir\src\main.cpp.obj.d -o tutorial/04-setting-up-common-options/code/applications/demo/CMakeFiles/04-setting-up-common-options.dir/src/main.cpp.obj -c ../tutorial/04-setting-up-common-options/code/applications/demo/src/main.cpp
  [5/8] cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj code/applications/demo/CMakeFiles/demo.dir/src/start.S.obj -o ..\output\RPI3\Debug\bin\demo.elf  -Wl,--start-group  -Wl,--end-group && cd ."
  [6/8] cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles tutorial/04-setting-up-common-options/code/applications/demo/CMakeFiles/04-setting-up-common-options.dir/src/main.cpp.obj tutorial/04-setting-up-common-options/code/applications/demo/CMakeFiles/04-setting-up-common-options.dir/src/start.S.obj -o ..\output\RPI3\Debug\bin\04-setting-up-common-options.elf  -Wl,--start-group  -Wl,--end-group && cd ."
  [7/8] cmd.exe /C "cd /D D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug\code\applications\demo && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E make_directory D:/Projects/baremetal/deploy/Debug/demo-image && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal/output/RPI3/Debug/bin/demo.elf -O binary D:/Projects/baremetal/deploy/Debug/demo-image/kernel8.img"
  [8/8] cmd.exe /C "cd /D D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug\tutorial\04-setting-up-common-options\code\applications\demo && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E make_directory D:/Projects/baremetal/deploy/Debug/04-setting-up-common-options-image && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal/output/RPI3/Debug/bin/04-setting-up-common-options.elf -O binary D:/Projects/baremetal/deploy/Debug/04-setting-up-common-options-image/kernel8.img"

Build All succeeded.
```

As you can see, now two executables and two image files are created.

Next: [05-setting-up-tools](05-setting-up-tools.md)
