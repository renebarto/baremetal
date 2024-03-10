# Tutorial 17: Unit test infrastructure {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE}

@tableofcontents

## New tutorial setup {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/17-unit-tests`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-17.a`
- an application `output/Debug/bin/17-unit-tests.elf`
- an image in `deploy/Debug/17-unit-tests-image`

## Creating a framework for unit testing {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_CREATING_A_FRAMEWORK_FOR_UNIT_TESTING}

In tutorials `15-string` and `16-serializing-and-formatting` we have been adding quite some code, and added assertion to verify correctness.

Even though we can test our code by using `assert()` macros, it is more convenient to write a framework to execute unit tests, in which they are automatically registered, and organized in different groups.
We'll follow the following concept here:

- Tests are classes that execute a single test (with possibly multiple test cases), and are self-contained and isolated.
I.e. they do not need any context, and don't influence the context.
- Test fixtures are collections of tests that belong together, for example because they test a specific class or group of functionality. A test fixture has a class, which can be used to hold common variables, and allows for a common test setup / teardown.
- Test suites are collections of test fixtures that belong together, for example because they test a complete library.

We'll use a result class to gather test results, which will then be collected in a results class.
A test reporter is a visitor class that is called to print the status of the test run and the results of every test.
This concept is similar although not exactly the same as the one used by Google Test.

For an overview, see the image below.

<img src="images/unittest-class-structure.png" alt="Tree view" width="800"/>

Let's start off by implementing a simple test class, and using a macro to define the test.

## Creating the unittest library - Step 1 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_CREATING_THE_UNITTEST_LIBRARY__STEP_1}

We'll place the unit testing functionality in a new library, named `unittest`.
The structure is similar to what we set up for `baremetal` (see [Creating the baremetal library structure - Step 1](#TUTORIAL_05_FIRST_APPLICATION__USING_THE_CONSOLE__UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE__STEP_1))

<img src="images/treeview-unittest-library.png" alt="Tree view" width="300"/>

### CMake file for unittest {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_CREATING_THE_UNITTEST_LIBRARY__STEP_1_CMAKE_FILE_FOR_UNITTEST}

We need to create the unittest project CMake file

Create the file `code/libraries/unittest/CMakeLists.txt`.

```cmake
File: d:\Projects\baremetal.github\code\libraries\unittest\CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3:
4: project(unittest
5:     DESCRIPTION "Unit test library"
6:     LANGUAGES CXX)
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
22:     baremetal
23:     )
24:
25: set(PROJECT_LIBS
26:     ${LINKER_LIBRARIES}
27:     ${PROJECT_DEPENDENCIES}
28:     )
29:
30: set(PROJECT_SOURCES
31:     )
32:
33: set(PROJECT_INCLUDES_PUBLIC
34:     )
35: set(PROJECT_INCLUDES_PRIVATE )
36:
37: if (CMAKE_VERBOSE_MAKEFILE)
38:     display_list("Package                           : " ${PROJECT_NAME} )
39:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
40:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
41:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
42:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
43:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
44:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
45:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
46:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
47:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
48:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
49:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
50:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
51:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
52:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
53:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
54:     display_list("Link libs                         : " ${PROJECT_LIBS} )
55:     display_list("Source files                      : " ${PROJECT_SOURCES} )
56:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
57:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
58: endif()
59:
60: add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
61: target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
62: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
63: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
64: target_compile_definitions(${PROJECT_NAME} PRIVATE
65:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
66:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
67:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
68:     )
69: target_compile_definitions(${PROJECT_NAME} PUBLIC
70:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
71:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
72:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
73:     )
74: target_compile_options(${PROJECT_NAME} PRIVATE
75:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
76:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
77:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
78:     )
79: target_compile_options(${PROJECT_NAME} PUBLIC
80:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
81:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
82:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
83:     )
84:
85: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
86:
87: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
88: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
89:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
90: endif()
91:
92: link_directories(${LINK_DIRECTORIES})
93: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
94: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
95:
96: show_target_properties(${PROJECT_NAME})
```

As no source files are added yet, configuring will fail, but we'll get to that in a while.

### Update libraries CMake file {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_CREATING_THE_UNITTEST_LIBRARY__STEP_1_UPDATE_LIBRARIES_CMAKE_FILE}

First we need to include the unittest project in the libraries CMake file.

Update the file `code/libraries/CMakeLists.txt`

```cmake
message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(baremetal)
add_subdirectory(unittest)
```

## Adding a test - Step 2 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST__STEP_2}

### Test.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST__STEP_2_TESTH}

We will start by adding a test class. All tests (and later test fixtures) will derive from this class, and implement its `RunImpl()` method to run the actual test.

Create the file `code/libraries/unittest/include/unittest/Test.h`

```cpp
File: code/libraries/unittest/include/unittest/Test.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Test.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : Test
9: //
10: // Description : Testcase
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include <unittest/TestDetails.h>
43: 
44: /// @file
45: /// Test
46: 
47: namespace unittest
48: {
49: 
50: /// <summary>
51: /// Test class
52: /// </summary>
53: class Test
54: {
55: public:
56:     Test() = default;
57:     Test(const Test&) = delete;
58:     Test(Test&&) = delete;
59:     virtual ~Test() = default;
60: 
61:     Test& operator = (const Test&) = delete;
62:     Test& operator = (Test&&) = delete;
63: 
64:     virtual void RunImpl() const;
65: };
66: 
67: } // namespace unittest
```

The `Test` class is added to the `unittest` namespace.

- Line 53-65: We declare the class `TestBase`
  - Line 56: We declare a default constructor with default implementation
  - Line 57-58: We remove the copy constructor and move constructor
  - Line 59: We declare the destructor with default implementation. This may be important as we will be inheriting from this class
  - Line 61-62: We remove the assignment operators
  - Line 64: We declare the overridable `RunImpl()` method

### Test.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST__STEP_2_TESTCPP}

We'll implement the `Test` class.

Create the file `code/libraries/unittest/src/Test.cpp`

```cpp
File: code/libraries/unittest/src/Test.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Test.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : Test
9: //
10: // Description : Test class
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
38: //------------------------------------------------------------------------------
39: 
40: #include <unittest/Test.h>
41: 
42: /// @file
43: /// Test implementation
44: 
45: using namespace baremetal;
46: 
47: namespace unittest {
48: 
49: /// <summary>
50: /// Actual test implementation
51: /// </summary>
52: void Test::RunImpl() const
53: {
54: }
55: 
56: } // namespace unittest
```

As you can see this is a very simple implementation, we simply implement the `RunImpl()` method with an default implementation.

### Update CMake file {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST__STEP_2_UPDATE_CMAKE_FILE}

As we have now added some source files to the `unittest` library, we need to update its CMake file.

Update the file `code/libraries/unitttest/CMakeLists.txt`

```cmake
File: code/libraries/unitttest/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Test.cpp
32:     )
33: 
34: set(PROJECT_INCLUDES_PUBLIC
35:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Test.h
36:     )
37: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST__STEP_2_UPDATE_APPLICATION_CODE}

Let's start using the class we just created. We'll add a simple test case by declaring and implementing a class derived from `TestBase`.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Assert.h>
2: #include <baremetal/Console.h>
3: #include <baremetal/Logger.h>
4: #include <baremetal/System.h>
5: #include <baremetal/Timer.h>
6: 
7: #include <unittest/Test.h>
8: 
9: LOG_MODULE("main");
10: 
11: using namespace baremetal;
12: 
13: class MyTest
14:     : public unittest::Test
15: {
16: public:
17:     void RunImpl() const override;
18: };
19: 
20: void MyTest::RunImpl() const
21: {
22:     LOG_INFO("Running test");
23: }
24: 
25: int main()
26: {
27:     auto& console = GetConsole();
28: 
29:     MyTest myTest;
30:     myTest.RunImpl();
31: 
32:     LOG_INFO("Wait 5 seconds");
33:     Timer::WaitMilliSeconds(5000);
34: 
35:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
36:     char ch{};
37:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
38:     {
39:         ch = console.ReadChar();
40:         console.WriteChar(ch);
41:     }
42:     if (ch == 'p')
43:         assert(false);
44: 
45:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
46: }
```

- Line 7: We include the header for `Test`
- Line 13-18: We declare the class `MyTest` based on `Test`
  - Line 17: We declare an override for the `RunImpl()` method
- Line 20-22: We implement the `RunImpl()` method for `MyTest`. It simply logs a string
- Line 29-30: We define an instance of MyTest, and then run the test.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging. We'll first switch off the memory debug output to get a cleaner console.

Update the file `CMakeLists.txt`

```cmake
...
61: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
62: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
63: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
64: option(BAREMETAL_TRACE_DEBUG "Enable debug tracing output" OFF)
65: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" OFF)
66: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" OFF)
...
```

The application will run the test, and therefore show the log output.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:201)
Info   Running test (main:22)
Info   Wait 5 seconds (main:32)
Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

## Test administration - Step 3 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3}

In order to register tests, we need to add an extra class that holds information on a test.
For this we introduce the `TestInfo` class.
This class will depend on another information class, `TestDetails` which holds the name of the test, the source file information, etc.

### TestDetails.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_TESTDETAILSH}

First we'll add the `TestDetails` class to describe a test.
This will hold its test suite name, test fixture name, test name, source file, and line number.

Create the file `code/libraries/unittest/include/unittest/TestDetails.h`

```cpp
File: code/libraries/unittest/include/unittest/TestDetails.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestDetails.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestDetails
9: //
10: // Description : Test detail
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include <baremetal/String.h>
43: 
44: /// @file
45: /// Test details
46: 
47: namespace unittest
48: {
49: 
50: /// <summary>
51: /// Details of a test
52: /// </summary>
53: class TestDetails
54: {
55: private:
56:     /// @brief Name of test suite test is part of
57:     const baremetal::string m_suiteName;
58:     /// @brief Name of test fixture test is part of
59:     const baremetal::string m_fixtureName;
60:     /// @brief Test name
61:     const baremetal::string m_testName;
62:     /// @brief Source file name of test
63:     const baremetal::string m_fileName;
64:     /// @brief Source line number of test
65:     const int m_lineNumber;
66: 
67: public:
68:     TestDetails();
69:     TestDetails(const baremetal::string& testName, const baremetal::string& fixtureName, const baremetal::string& suiteName, const baremetal::string& fileName, int lineNumber);
70:     TestDetails(const TestDetails& other, int lineNumber);
71: 
72:     /// <summary>
73:     /// Returns test suite name
74:     /// </summary>
75:     /// <returns>Test suite name</returns>
76:     const baremetal::string& SuiteName() const { return m_suiteName; }
77:     /// <summary>
78:     /// Returns test fixture name
79:     /// </summary>
80:     /// <returns>Test fixture name</returns>
81:     const baremetal::string& FixtureName() const { return m_fixtureName; }
82:     /// <summary>
83:     /// Returns test name
84:     /// </summary>
85:     /// <returns>Test name</returns>
86:     const baremetal::string& TestName() const { return m_testName; }
87:     /// <summary>
88:     /// Returns test source file name
89:     /// </summary>
90:     /// <returns>Test source file name</returns>
91:     const baremetal::string& SourceFileName() const { return m_fileName; }
92:     /// <summary>
93:     /// Returns test source line number
94:     /// </summary>
95:     /// <returns>Test source line number</returns>
96:     int SourceFileLineNumber() const { return m_lineNumber; }
97: };
98: 
99: } // namespace unittest
```

The `TestDetails` class is added to the `unittest` namespace.

- Line 42: We use strings, so we need to include the header for the `string` class
- Line 53-97: We declare the class `TestDetails` which will hold information on a test
  - Line 57: The class member variable `m_suiteName` is the test suite name
  - Line 59: The class member variable `m_fixtureName` is the test fixture name
  - Line 61: The class member variable `m_testName` is the test name
  - Line 63: The class member variable `m_fileName` is the source file in which the actual test is defined
  - Line 65: The class member variable `m_lineNumber` is the source line in which the actual test is defined
  - Line 68: We declare the default constructor
  - Line 69: We declare the normal constructor which specifies all the needed information
  - Line 70: We declare the kind of copy constructor, but with an override for the source line number
  - Line 76: We declare an accessor `SuiteName()` for the test suite name
  - Line 81: We declare an accessor `FixtureName()` for the test fixture name
  - Line 86: We declare an accessor `TestName()` for the test name
  - Line 91: We declare an accessor `SourceFileName()` for the source file name
  - Line 96: We declare an accessor `SourceFileLineNumber()` for the source line number

### TestDetails.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_TESTDETAILSCPP}

Let's implement the `TestDetails` class.

Create the file `code/libraries/unittest/src/TestDetails.cpp`

```cpp
File: code/libraries/unittest/src/TestDetails.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestDetails.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestDetails
9: //
10: // Description : Test details
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
38: //------------------------------------------------------------------------------
39: 
40: #include <unittest/TestDetails.h>
41: 
42: /// @file
43: /// Test details implementation
44: 
45: using namespace baremetal;
46: 
47: namespace unittest {
48: 
49: /// <summary>
50: /// Default constructor
51: /// </summary>
52: TestDetails::TestDetails()
53:     : m_suiteName{}
54:     , m_fixtureName{}
55:     , m_testName{}
56:     , m_fileName{}
57:     , m_lineNumber{}
58: {
59: }
60: 
61: /// <summary>
62: /// Explicit constructor
63: /// </summary>
64: /// <param name="testName">Test name</param>
65: /// <param name="fixtureName">Name of test fixture test is part of</param>
66: /// <param name="suiteName">Name of test suite test is part of</param>
67: /// <param name="fileName">Source file name of test</param>
68: /// <param name="lineNumber">Source line number of test</param>
69: TestDetails::TestDetails(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
70:     : m_suiteName{ suiteName }
71:     , m_fixtureName{ fixtureName }
72:     , m_testName{ testName }
73:     , m_fileName{ fileName }
74:     , m_lineNumber{ lineNumber }
75: {
76: }
77: 
78: /// <summary>
79: /// Construct from other test details, override source line number
80: /// </summary>
81: /// <param name="other">Test details to copy from</param>
82: /// <param name="lineNumber">Source line number to set</param>
83: TestDetails::TestDetails(const TestDetails& other, int lineNumber)
84:     : m_suiteName{ other.m_suiteName }
85:     , m_fixtureName{ other.m_fixtureName }
86:     , m_testName{ other.m_testName }
87:     , m_fileName{ other.m_fileName }
88:     , m_lineNumber{ lineNumber }
89: {
90: }
91: 
92: } // namespace unittest
```

- Line 52-59: We implement the default constructor
- Line 69-76: We implement the non default constructor
- Line 83-90: We implement the "copy" constructor

### TestResults.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_TESTRESULTSH}

In order to use the `TestInfo` class, we need to declare `TestResults`.
For now we'll simply declare a simple class with a default constructor.
We'll fill in the details later.

Create the file `code/libraries/unittest/include/unittest/TestResults.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResults.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestResults.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestResults
9: //
10: // Description : Test run results
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: /// @file
43: /// Test run results
44: 
45: namespace unittest
46: {
47: 
48: /// <summary>
49: /// Test results class
50: /// </summary>
51: class TestResults
52: {
53: public:
54:     TestResults() = default;
55: };
56: 
57: } // namespace unittest
```

### CurrentTest.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_TESTRESULTSH}

When running tests, we need a way to keep track of the details for the current test, as well as the test run results.
We therefore introduce a simple class `CurrentTest` that holds pointers to both.

Create the file `code/libraries/unittest/include/unittest/CurrentTest.h`

```cpp
File: code/libraries/unittest/include/unittest/CurrentTest.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : CurrentTest.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : CurrentTest
9: //
10: // Description : Current test info
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: /// @file
43: /// Current test information
44: 
45: namespace unittest
46: {
47: 
48: class TestResults;
49: class TestDetails;
50: 
51: class CurrentTest
52: {
53: public:
54:     static TestResults*& Results();
55:     static const TestDetails*& Details();
56: };
57: 
58: } // namespace unittest
```

### CurrentTest.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_TESTINFOCPP}

We'll implement the `CurrentTest` class.

Create the file `code/libraries/unittest/src/CurrentTest.cpp`

```cpp
File: code/libraries/unittest/src/CurrentTest.cpp
File: d:\Projects\baremetal.github\code\libraries\unittest\src\CurrentTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : CurrentTest.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : CurrentTest
9: //
10: // Description : Current test info
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
38: //------------------------------------------------------------------------------
39: 
40: #include <unittest/CurrentTest.h>
41: 
42: /// @file
43: /// Current test information implementation
44: 
45: namespace unittest
46: {
47: 
48: /// <summary>
49: /// Returns reference to static test results pointer
50: ///
51: /// As a reference is returned, the pointer can also be set. This pointer is used to keep track of test results during the test run.
52: /// </summary>
53: /// <returns>A reference to the current test results pointer</returns>
54: TestResults *& CurrentTest::Results()
55: {
56:     static TestResults* testResults = nullptr;
57:     return testResults;
58: }
59: 
60: /// <summary>
61: /// Returns reference to static test details pointer
62: ///
63: /// As a reference is returned, the pointer can also be set. This pointer is used to keep track of test details during the test run.
64: /// </summary>
65: /// <returns>A reference to the current test details pointer</returns>
66: const TestDetails *& CurrentTest::Details()
67: {
68:     static const TestDetails* testDetails = nullptr;
69:     return testDetails;
70: }
71: 
72: } // namespace unittest
```

- Line 54-58: We implement the `Results()` method.
This simply returns a pointer to the static variable inside the function, which can the be set and used
- Line 66-70: We implement the `Details()` method.
This simply returns a pointer to the static variable inside the function, which can the be set and used

### TestInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_TESTINFOH}

Next we will add the test administration class `TestInfo`.
This class will be used to register information for a test.

Create the file `code/libraries/unittest/include/unittest/TestInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestInfo.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestInfo.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestInfo
9: //
10: // Description : Testcase
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include <unittest/TestDetails.h>
43: 
44: /// @file
45: /// Test administration
46: 
47: namespace unittest
48: {
49: 
50: class Test;
51: class TestResults;
52: 
53: /// <summary>
54: /// Test administration class
55: /// </summary>
56: class TestInfo
57: {
58: private:
59:     /// @brief Test details
60:     const TestDetails m_details;
61:     /// @brief Pointer to actual test
62:     Test* m_testInstance;
63:     /// @brief Pointer to next test case in list
64:     TestInfo* m_next;
65: 
66: public:
67:     TestInfo();
68:     TestInfo(const TestInfo&) = delete;
69:     TestInfo(TestInfo&&) = delete;
70:     explicit TestInfo(
71:         const baremetal::string& testName,
72:         const baremetal::string& fixtureName = {},
73:         const baremetal::string& suiteName = {},
74:         const baremetal::string& fileName = {},
75:         int lineNumber = {});
76: 
77:     TestInfo& operator = (const TestInfo&) = delete;
78:     TestInfo& operator = (TestInfo&&) = delete;
79: 
80:     /// <summary>
81:     /// Returns the test details
82:     /// </summary>
83:     /// <returns>Test details</returns>
84:     const TestDetails& Details() const { return m_details; }
85: 
86:     void SetTest(Test* test);
87: 
88:     void Run(TestResults& testResults);
89: };
90: 
91: } // namespace unittest
```

The `TestInfo` class is again added to the `unittest` namespace.

- Line 50: We forward declare the `Test` class.
- Line 51: We forward declare a class `TestResults. This is the container for all test results during a test run.
We'll get to that in a minute.
- Line 56-90: We declare the class `TestInfo`
  - Line 60: We declare the details for the test
  - Line 62: We declare a pointer to the actual test to run
  - Line 64: We declare a pointer to the next test. Tests administration classes will be stored in a linked list
  - Line 67: We declare a default constructor
  - Line 68-69: We remove the copy constructor and move constructor
  - Line 70-75: We declare an explicit constructor
  - Line 77-78: We remove the assignment operators
  - Line 84: We declare and implement a method `Details()` to retrieve details
  - Line 86: We declare a method `SetTest()` to set the test instance pointer
  - Line 88: We declare a method `Run()` to run the test and update the test results

### TestInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_TESTINFOCPP}

We'll implement the `TestInfo` class.

Create the file `code/libraries/unittest/src/TestInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestInfo.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestInfo.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestInfo
9: //
10: // Description : Testcase base class
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
38: //------------------------------------------------------------------------------
39: 
40: #include <unittest/TestInfo.h>
41: 
42: #include <unittest/CurrentTest.h>
43: #include <unittest/Test.h>
44: 
45: /// @file
46: /// Test case administration implementation
47: 
48: using namespace baremetal;
49: 
50: namespace unittest {
51: 
52: /// <summary>
53: /// Default constructor
54: /// </summary>
55: TestInfo::TestInfo()
56:     : m_details{}
57:     , m_testInstance{}
58:     , m_next{}
59: {
60: }
61: 
62: /// <summary>
63: /// Explicit constructor
64: /// </summary>
65: /// <param name="testName">Name of test</param>
66: /// <param name="fixtureName">Name of test fixture test is part of</param>
67: /// <param name="suiteName">Name of test suite test is part of</param>
68: /// <param name="fileName">Source file name of test</param>
69: /// <param name="lineNumber">Source line number of test</param>
70: TestInfo::TestInfo(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
71:     : m_details{ testName, fixtureName, suiteName, fileName, lineNumber }
72:     , m_testInstance{}
73:     , m_next{}
74: {
75: }
76: 
77: /// <summary>
78: /// Set test instance to link to test administration
79: /// </summary>
80: /// <param name="test">Test instance to link</param>
81: void TestInfo::SetTest(Test* test)
82: {
83:     m_testInstance = test;
84: }
85: 
86: /// <summary>
87: /// Run the test instance, and update the test results
88: /// </summary>
89: /// <param name="testResults"></param>
90: void TestInfo::Run(TestResults& testResults)
91: {
92:     CurrentTest::Details() = &Details();
93:     CurrentTest::Results() = &testResults;
94:     if (m_testInstance != nullptr)
95:         m_testInstance->RunImpl();
96: }
97: 
98: } // namespace unittest
```

- Line 55-60: We implement the default constructor
- Line 70-75: We implement the non default constructor
- Line 81-84: We implement the `SetTest()` method
- Line 90-96: We implement the `Run()` method

### Update CMake file {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_UPDATE_CMAKE_FILE}

As we have now added some source files to the `unittest` library, we need to update its CMake file.

Update the file `code/libraries/unitttest/CMakeLists.txt`

```cmake
File: code/libraries/unitttest/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Test.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestInfo.cpp
35:     )
36: 
37: set(PROJECT_INCLUDES_PUBLIC
38:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
39:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Test.h
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestInfo.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
43:     )
44: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_UPDATE_APPLICATION_CODE}

Let's start using the class we just created. We'll add a simple test case by declaring and implementing a class derived from `TestBase`.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Assert.h>
2: #include <baremetal/Console.h>
3: #include <baremetal/Logger.h>
4: #include <baremetal/System.h>
5: #include <baremetal/Timer.h>
6: 
7: #include <unittest/Test.h>
8: #include <unittest/TestInfo.h>
9: #include <unittest/TestResults.h>
10: 
11: LOG_MODULE("main");
12: 
13: using namespace baremetal;
14: 
15: class MyTest
16:     : public unittest::Test
17: {
18: public:
19:     void RunImpl() const override;
20: };
21: 
22: void MyTest::RunImpl() const
23: {
24:     LOG_INFO("Running test");
25: }
26: 
27: int main()
28: {
29:     auto& console = GetConsole();
30: 
31:     MyTest myTest;
32:     unittest::TestInfo myTestInfo("MyTest", "DummyFixture", "DummySuite", __FILE__, __LINE__);
33:     myTestInfo.SetTest(&myTest);
34:     unittest::TestResults results;
35: 
36:     myTestInfo.Run(results);
37: 
38:     LOG_INFO("Wait 5 seconds");
39:     Timer::WaitMilliSeconds(5000);
40: 
41:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
42:     char ch{};
43:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
44:     {
45:         ch = console.ReadChar();
46:         console.WriteChar(ch);
47:     }
48:     if (ch == 'p')
49:         assert(false);
50: 
51:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
52: }
```

- Line 8: We include the header for `TestInfo`
- Line 9: We include the header for `TestResults`
- Line 32: We define an instance of `TestInfo`, specifying the test name, test fixture name, test suite name, and the source location
- Line 33: We link the test instance `myTest` to the `TestInfo` instance
- Line 34: We define an instance of `TestResults`
- Line 36: We run the test through the `Run()` method of `TestInfo`

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the test again, and the output is almost identical to the previous step.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:201)
Info   Running test (main:24)
Info   Wait 5 seconds (main:38)
Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

## Adding test fixtures - Step 4 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4}

Now that we have a test we can run, let's add the test fixture, to hold multiple tests, and provide for a setup / teardown method call.
Test fixtures are slightly different from tests in their structure.
- A `Test` only has a `RunImpl()` method which can be overridden.
A `Test` class is overridden by a test implementation
- A `TestFixture` class has a `RunImpl()` method which can be overridden, as well as a `SetUp()` and `TearDown()` method.
These methods can be overridden to implement setting up and tearing down context for all tests that are part of the test fixture.
The `TestFixture` class is inherited by a fixture helper class for each test in the fixture, that acts a test, but is constructed before running the test, and destructed afterwards.
The inheriting class will have a `RunImpl()` method that will run the actual test. Its constructor will run the `SetUp()` method, the destructor will run the `TearDown()` method
A `TestFixture` will have an accompanying class inheriting from `Test`. It's `RunImpl()` method will instantiate the inheriting `TextFixture` class, run its `RunImpl()` method and destruct it again

- A `TestInfo` class holds the test details, a pointer to the actual `Test` instantiation and a pointer to the next test in the list
The `TestDetails` holds only information on a test, such as the test name, test fixture name, etc.
- A `TestFixtureInfo` class holds the a pointer to the next fixture, and the pointers to the first and last test in the fixture.

See also the image below.

<img src="images/unittest-class-structure-test-fixture.png" alt="Tree view" width="800"/>

Things will become more clear when we start using the test fixtures.

### TestFixture.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_TESTFIXTUREH}

The `TestFixture` class is as said simply a base class for helper derivatives.

Create the file `code/libraries/unittest/include/unittest/TestFixture.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixture.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestFixture.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestFixture
9: //
10: // Description : Test fixture functionality
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: /// @file
43: /// Test fixture
44: 
45: namespace unittest {
46: 
47: /// <summary>
48: /// Test fixture
49: /// 
50: /// Forms a replacement of a test, with the addition of SetUp/TearDown functionality
51: /// </summary>
52: class TestFixture
53: {
54: protected:
55:     TestFixture() = default;
56:     TestFixture(const TestFixture&) = delete;
57:     TestFixture(TestFixture&&) = delete;
58:     virtual ~TestFixture() = default;
59: 
60:     TestFixture& operator = (const TestFixture&) = delete;
61:     TestFixture& operator = (TestFixture&&) = delete;
62: 
63:     /// <summary>
64:     /// Default setup function
65:     /// </summary>
66:     virtual void SetUp() {};
67:     /// <summary>
68:     /// Default teardown function
69:     /// </summary>
70:     virtual void TearDown() {};
71: };
72: 
73: } // namespace unittest
```

- Line 52-71: We declare the `TestFixture` class
  - Line 55: We make the default constructor a default implementation
  - Line 56-57: We remove the copy and move constructor
  - Line 58: We make the destructor a default implementation
  - Line 60-61: We remove the assignment and move assignment operator
  - Line 66-70: We declare and implement the virtual `SetUp()` and `TearDown()` methods

As can be seen, nothing else needs to be added for implementation.

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_TESTFIXTUREINFOH}

So let's declare the `TestFixtureInfo` class.

Create the file `code/libraries/unittest/include/unittest/TestFixtureInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixtureInfo.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestFixtureInfo.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestFixtureInfo
9: //
10: // Description : TestInfo fixture info
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include <unittest/TestInfo.h>
43: #include <unittest/TestResults.h>
44: 
45: /// @file
46: /// Test fixture administration
47: 
48: namespace unittest
49: {
50: 
51: class TestInfo;
52: 
53: /// <summary>
54: /// Test fixture administration
55: /// 
56: /// Holds information on a test fixture, which includes its name and the list of tests that are part of it
57: /// </summary>
58: class TestFixtureInfo
59: {
60: private:
61:     /// @brief Pointer to first test in the list
62:     TestInfo* m_head;
63:     /// @brief Pointer to last test in the list
64:     TestInfo* m_tail;
65:     /// @brief Pointer to next test fixture info in the list
66:     TestFixtureInfo* m_next;
67:     /// @brief Test fixture name
68:     baremetal::string m_fixtureName;
69: 
70: public:
71:     TestFixtureInfo() = delete;
72:     TestFixtureInfo(const TestFixtureInfo&) = delete;
73:     TestFixtureInfo(TestFixtureInfo&&) = delete;
74:     explicit TestFixtureInfo(const baremetal::string& fixtureName);
75:     virtual ~TestFixtureInfo();
76: 
77:     TestFixtureInfo & operator = (const TestFixtureInfo &) = delete;
78:     TestFixtureInfo& operator = (TestFixtureInfo&&) = delete;
79: 
80:     /// <summary>
81:     /// Returns the pointer to the first test in the list for this test fixture
82:     /// </summary>
83:     /// <returns>Pointer to the first test in the list for this test fixture</returns>
84:     TestInfo* Head() const {  return m_head; }
85: 
86:     /// <summary>
87:     /// Returns the test fixture name
88:     /// </summary>
89:     /// <returns>Test fixture name</returns>
90:     const baremetal::string& Name() const { return m_fixtureName; }
91: 
92:     void Run(TestResults& testResults);
93: 
94:     int CountTests();
95: 
96:     void AddTest(TestInfo* test);
97: };
98: 
99: } // namespace unittest
```

- Line 62-64: The member variables `m_head` and `m_tail` store the pointer to the first and last test in the fixture
- Line 66: The member variable `m_next` is the pointer to the next test fixture. Again, test fixtures are stored in linked list
- Line 68: The member variable `m_fixtureName` holds the name of the test fixture
- Line 71: We remove the default constructor
- Line 72-73 We remove the copy and move constructors
- Line 74: We declare the only usable constructor which receives the test fixture name
- Line 75: We declare the destructor
- Line 77-78: We remove the assignment and move assignment operators
- Line 84: The method `Head()` returns the pointer to the first test in the list
- Line 90: The method `Name()` returns the test fixture name
- Line 92: The method `Run()` runs all tests in the test fixture. We'll be revisiting this later
- Line 94: The method `CountTests()` counts and returns the number of tests in the test fixture
- Line 96: The method `AddTest()` adds a test to the list for the test fixture

### TestInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_TESTINFOH}

As the `TestFixtureInfo` class needs access to the `TestInfo` class in order to access the `m_next` pointer, we need to make it a friend class.

Update the file `code/libraries/unittest/include/unittest/TestInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestInfo.h
...
56: class TestInfo
57: {
58: private:
59:     friend class TestFixtureInfo;
60:     /// @brief Test details
61:     TestDetails const m_details;
62:     /// @brief Pointer to actual test
63:     Test* m_testInstance;
64:     /// @brief Pointer to next test case in list
65:     TestInfo* m_next;
66: 
...
```

### TestFixtureInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_TESTFIXTUREINFOCPP}

Let's implement the `TestFixtureInfo` class.

Create the file `code/libraries/unittest/src/TestFixtureInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestFixtureInfo.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestFixtureInfo.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestFixtureInfo
9: //
10: // Description : Test fixture
11: //------------------------------------------------------------------------------
12: //
13: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
14: //
15: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
16: //
17: // Permission is hereby granted, free of charge, to any person
18: // obtaining a copy of this software and associated documentation
19: // files(the "Software"), to deal in the Software without
20: // restriction, including without limitation the rights to use, copy,
21: // modify, merge, publish, distribute, sublicense, and /or sell copies
22: // of the Software, and to permit persons to whom the Software is
23: // furnished to do so, subject to the following conditions :
24: //
25: // The above copyright notice and this permission notice shall be
26: // included in all copies or substantial portions of the Software.
27: //
28: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
29: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
30: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
31: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
32: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
33: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
34: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
35: // DEALINGS IN THE SOFTWARE.
36: //
37: //------------------------------------------------------------------------------
38: 
39: #include <unittest/TestFixtureInfo.h>
40: 
41: #include <baremetal/Assert.h>
42: 
43: /// @file
44: /// Test fixture administration implementation
45: 
46: using namespace baremetal;
47: 
48: namespace unittest {
49: 
50: /// <summary>
51: /// Constructor
52: /// </summary>
53: /// <param name="fixtureName">Test fixture name</param>
54: TestFixtureInfo::TestFixtureInfo(const string& fixtureName)
55:     : m_head{}
56:     , m_tail{}
57:     , m_next{}
58:     , m_fixtureName{ fixtureName }
59: {
60: }
61: 
62: /// <summary>
63: /// Destructor
64: /// 
65: /// Cleans up all registered tests for this test fixture
66: /// </summary>
67: TestFixtureInfo::~TestFixtureInfo()
68: {
69:     TestInfo* test = m_head;
70:     while (test != nullptr)
71:     {
72:         TestInfo* currentTest = test;
73:         test = test->m_next;
74:         delete currentTest;
75:     }
76: }
77: 
78: /// <summary>
79: /// Add a test to the list
80: /// 
81: /// This method is called at static initialization time to register tests
82: /// </summary>
83: /// <param name="test">Test to register</param>
84: void TestFixtureInfo::AddTest(TestInfo* test)
85: {
86:     if (m_tail == nullptr)
87:     {
88:         assert(m_head == nullptr);
89:         m_head = test;
90:         m_tail = test;
91:     }
92:     else
93:     {
94:         m_tail->m_next = test;
95:         m_tail = test;
96:     }
97: }
98: 
99: /// <summary>
100: /// Run tests in test fixture, updating the test results
101: /// </summary>
102: /// <param name="testResults">Test results to use and update</param>
103: void TestFixtureInfo::Run(TestResults& testResults)
104: {
105:     TestInfo* test = Head();
106:     while (test != nullptr)
107:     {
108:         test->Run(testResults);
109:         test = test->m_next;
110:     }
111: }
112: 
113: 
114: /// <summary>
115: /// Count the number of tests in the test fixture
116: /// </summary>
117: /// <returns>Number of tests in the test fixture</returns>
118: int TestFixtureInfo::CountTests()
119: {
120:     int numberOfTests = 0;
121:     TestInfo* test = Head();
122:     while (test != nullptr)
123:     {
124:         ++numberOfTests;
125:         test = test->m_next;
126:     }
127:     return numberOfTests;
128: }
129: 
130: } // namespace unittest
```

- Line 54-60: We implement the constructor
- Line 67-76: We implement the destructor. This goes through the list of tests, and deletes every one of these. Note that we will therefore need to create the tests on the heap.
- Line 84-97: We implement the `AddTest()` method. This will add the test passed in at the end of the list
- Line 103-111: We implement the `Run()` method. This goes through the list of tests, and calls `Run()` on each
- Line 118-128: We implement the `CountTests()` method. This goes through the list of tests, and counts them

### TestResults.h

We will add some methods to the `TestResults` class to indicate what we're moving towards.

Update the file `code/libraries/unittest/include/unittest/TestResults.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResults.h
...
45: namespace baremetal {
46: 
47: class string;
48: 
49: }
50: 
51: namespace unittest
52: {
53: 
54: class TestDetails;
55: 
56: /// <summary>
57: /// Test results class
58: /// </summary>
59: class TestResults
60: {
61: public:
62:     TestResults() = default;
63: 
64:     void OnTestStart(const TestDetails& details);
65:     void OnTestRun(const TestDetails& details, const baremetal::string& message);
66:     void OnTestFinish(const TestDetails& details);
67: };
...
```

- Line 64: We declare the method `OnTestStart()` to indicate the start of a test
- Line 65: We declare the method 'OnTestRun()` to indicate we're running a test (this will be changed later)
- Line 66: We declare the method `OnTestFinish()` to indicate the end of a test

### TestResults.cpp

Let's implement the new methods in the `Testresults` class.

Create the file `code/libraries/unittest/src/TestResults.cpp`

```cpp
File: code/libraries/unittest/src/TestResults.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestResults.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestResults
9: //
10: // Description : Test run results
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
38: //------------------------------------------------------------------------------
39: 
40: #include <unittest/TestResults.h>
41: 
42: #include <baremetal/Format.h>
43: #include <baremetal/Logger.h>
44: #include <baremetal/String.h>
45: #include <unittest/TestDetails.h>
46: 
47: /// @file
48: /// Test results implementation
49: 
50: using namespace baremetal;
51: 
52: /// @brief Define log name
53: LOG_MODULE("TestResults");
54: 
55: namespace unittest
56: {
57: 
58: static string QualifiedTestName(const TestDetails& details)
59: {
60:     return Format("%s::%s::%s",
61:         details.SuiteName().empty() ? "DefaultSuite" : details.SuiteName().c_str(),
62:         details.FixtureName().empty() ? "DefaultFixture" : details.FixtureName().c_str(),
63:         details.TestName().c_str());
64: }
65: 
66: void TestResults::OnTestStart(const TestDetails& details)
67: {
68:     LOG_INFO(QualifiedTestName(details) + " Start test");
69: }
70: 
71: void TestResults::OnTestRun(const TestDetails& details, const string& message)
72: {
73:     string fullMessage = QualifiedTestName(details) + Format(" (%s:%d)",
74:         details.SourceFileName().c_str(),
75:         details.SourceFileLineNumber()) + " --> " + message;
76:     LOG_DEBUG(fullMessage);
77: }
78: 
79: void TestResults::OnTestFinish(const TestDetails& details)
80: {
81:     LOG_INFO(QualifiedTestName(details) + " Finish test");
82: }
83: 
84: } // namespace unittest
```

- Line 58-64: We create a static function to format the fully qualified name of a test (\<suite\>::\<fixture\>::\<test\>)
- Line 66-69: We implement the methos `OnTestStart()`
- Line 71-77: We implement the methos `OnTestRun()`
- Line 79-82: We implement the methos `OnTestFinish()`

As this code will change soon, it's not documented now.

### TestInfo.cpp

As we have added some methods to `TestResults`, we'll update the `Run()` method in `TestInfo` to call these.

Update the file `code/libraries/unittest/src/TestInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestInfo.cpp
...
40: #include <unittest/TestInfo.h>
41: 
42: #include <unittest/CurrentTest.h>
43: #include <unittest/Test.h>
44: #include <unittest/TestResults.h>
...
File: d:\Projects\baremetal.github\code\libraries\unittest\src\TestInfo.cpp
87: /// <summary>
88: /// Run the test instance, and update the test results
89: /// </summary>
90: /// <param name="testResults"></param>
91: void TestInfo::Run(TestResults& testResults)
92: {
93:     CurrentTest::Details() = &Details();
94:     CurrentTest::Results() = &testResults;
95: 
96:     if (m_testInstance != nullptr)
97:     {
98:         testResults.OnTestStart(m_details);
99: 
100:         m_testInstance->RunImpl();
101: 
102:         testResults.OnTestFinish(m_details);
103:     }
104: }
105: 
```

- Line 44: We need to include the header for `TestResults`
- Line 96-103: Next to calling the `RunImpl()` method on the test, we also call the `OnTestStart()` and `OnTestFinish()` methods on the `TestResults` class

### Update CMake file {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_UPDATE_CMAKE_FILE}

As we have added some files to the `unittest` library, we need to update its CMake file.

Update the file `code/libraries/unitttest/CMakeLists.txt`

```cmake
File: code/libraries/unitttest/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Test.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestInfo.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
37:     )
38: 
39: set(PROJECT_INCLUDES_PUBLIC
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Test.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestInfo.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
47:     )
48: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_UPDATE_APPLICATION_CODE}

Let's start using the test fixtures.
We'll add a couple of simple test cases by declaring and implementing a class derived from `TestBase`.
We'll then create a test fixture, and add the tests to the fixture.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/Assert.h>
2: #include <baremetal/Console.h>
3: #include <baremetal/Logger.h>
4: #include <baremetal/System.h>
5: #include <baremetal/Timer.h>
6: 
7: #include <unittest/CurrentTest.h>
8: #include <unittest/Test.h>
9: #include <unittest/TestFixture.h>
10: #include <unittest/TestFixtureInfo.h>
11: #include <unittest/TestInfo.h>
12: #include <unittest/TestResults.h>
13: 
14: LOG_MODULE("main");
15: 
16: using namespace baremetal;
17: using namespace unittest;
18: 
19: class FixtureMyTest
20:     : public TestFixture
21: {
22: public:
23:     void SetUp() override
24:     {
25:         LOG_DEBUG("MyTest SetUp");
26:     }
27:     void TearDown() override
28:     {
29:         LOG_DEBUG("MyTest TearDown");
30:     }
31: };
32: 
33: class FixtureMyTest1Helper
34:     : public FixtureMyTest
35: {
36: public:
37:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
38:     explicit FixtureMyTest1Helper(const TestDetails& details)
39:         : m_details{ details }
40:     {
41:         SetUp();
42:     }
43:     virtual ~FixtureMyTest1Helper()
44:     {
45:         TearDown();
46:     }
47:     void RunImpl() const;
48:     const TestDetails& m_details;
49: };
50: void FixtureMyTest1Helper::RunImpl() const
51: {
52:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 1");
53: }
54: 
55: class MyTest1
56:     : public Test
57: {
58:     void RunImpl() const override
59:     {
60:         LOG_DEBUG("Test 1");
61:         FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
62:         fixtureHelper.RunImpl();
63:     }
64: } myTest1;
65: class MyTestInfo1
66:     : public TestInfo
67: {
68: public:
69:     MyTestInfo1(Test* testInstance)
70:         : TestInfo("MyTest1", "MyFixture", "", __FILE__, __LINE__)
71:     {
72:         SetTest(testInstance);
73:     }
74: };
75: 
76: class FixtureMyTest2Helper
77:     : public FixtureMyTest
78: {
79: public:
80:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
81:     explicit FixtureMyTest2Helper(const TestDetails& details)
82:         : m_details{ details }
83:     {
84:         SetUp();
85:     }
86:     virtual ~FixtureMyTest2Helper()
87:     {
88:         TearDown();
89:     }
90:     void RunImpl() const;
91:     const TestDetails& m_details;
92: };
93: void FixtureMyTest2Helper::RunImpl() const
94: {
95:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 2");
96: }
97: 
98: class MyTest2
99:     : public Test
100: {
101:     void RunImpl() const override
102:     {
103:         LOG_DEBUG("Test 2");
104:         FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
105:         fixtureHelper.RunImpl();
106:     }
107: } myTest2;
108: class MyTestInfo2
109:     : public TestInfo
110: {
111: public:
112:     MyTestInfo2(Test* testInstance)
113:         : TestInfo("MyTest2", "MyFixture", "", __FILE__, __LINE__)
114:     {
115:         SetTest(testInstance);
116:     }
117: };
118: 
119: class FixtureMyTest3Helper
120:     : public FixtureMyTest
121: {
122: public:
123:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
124:     explicit FixtureMyTest3Helper(const TestDetails& details)
125:         : m_details{ details }
126:     {
127:         SetUp();
128:     }
129:     virtual ~FixtureMyTest3Helper()
130:     {
131:         TearDown();
132:     }
133:     void RunImpl() const;
134:     const TestDetails& m_details;
135: };
136: void FixtureMyTest3Helper::RunImpl() const
137: {
138:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 3");
139: }
140: 
141: class MyTest3
142:     : public Test
143: {
144:     void RunImpl() const override
145:     {
146:         LOG_DEBUG("Test 3");
147:         FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
148:         fixtureHelper.RunImpl();
149:     }
150: } myTest3;
151: class MyTestInfo3
152:     : public TestInfo
153: {
154: public:
155:     MyTestInfo3(Test* testInstance)
156:         : TestInfo("MyTest3", "MyFixture", "", __FILE__, __LINE__)
157:     {
158:         SetTest(testInstance);
159:     }
160: };
161: 
162: 
163: class MyTest
164:     : public Test
165: {
166: public:
167:     void RunImpl() const override;
168: } myTest;
169: class MyTestInfo
170:     : public TestInfo
171: {
172: public:
173:     MyTestInfo(Test* testInstance)
174:         : TestInfo("MyTest3", "MyFixture", "", __FILE__, __LINE__)
175:     {
176:         SetTest(testInstance);
177:     }
178: };
179: 
180: void MyTest::RunImpl() const
181: {
182:     CurrentTest::Results()->OnTestRun(*CurrentTest::Details(), "Running test");
183: }
184: 
185: int main()
186: {
187:     auto& console = GetConsole();
188: 
189:     TestInfo* test1 = new MyTestInfo1(&myTest1);
190:     TestInfo* test2 = new MyTestInfo2(&myTest2);
191:     TestInfo* test3 = new MyTestInfo3(&myTest3);
192:     TestFixtureInfo* fixture = new TestFixtureInfo("MyFixture");
193:     unittest::TestResults results;
194:     fixture->AddTest(test1);
195:     fixture->AddTest(test2);
196:     fixture->AddTest(test3);
197:     fixture->Run(results);
198:     delete fixture;
199:     unittest::TestInfo myTestInfo("MyTest", "", "", __FILE__, __LINE__);
200:     myTestInfo.SetTest(&myTest);
201: 
202:     myTestInfo.Run(results);
203: 
204:     LOG_INFO("Wait 5 seconds");
205:     Timer::WaitMilliSeconds(5000);
206: 
207:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
208:     char ch{};
209:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
210:     {
211:         ch = console.ReadChar();
212:         console.WriteChar(ch);
213:     }
214:     if (ch == 'p')
215:         assert(false);
216: 
217:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
218: }
```

- Line 7: We include the header for `CurrentTest`
- Line 9: We include the header for `TestFixture`
- Line 10: We include the header for `TestFixtureInfo`
- Line 12: We include the header for `TestResults`

- Line 19-31: We declare and implement the class `FixtureMyTest` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods, simply by logging a message
- Line 33-53: We declare and implement the class `FixtureMyTest1Helper` which derives from the class `FixtureMyTest` just defined, and implements the constructor and destructor, calling resp. `SetUp()` and `TearDown()`
This class also defines a `RunImpl()` method, which forms the actual test body for the test MyTest1.
It is implemented as a call to the `OnTestRun()` method on the `TestResults` instance
- Line 55-64: We declare and implement the class `MyTest1`, which derives from `Test`, and acts as the placeholders for the fixture test.
We instantiate it as `myTest1`. Its `RunImpl()` method instantiates `FixtureMyTest1Helper`, and runs its `RunImpl()` method
- Line 65-74: We declare and implement the class `MyTestInfo1`, which derives from `TestInfo`. Its constructor links the test instance
- Line 76-96: We declare and implement the class `FixtureTest2Helper` in a similar way as `FixtureMyTest1Helper`
- Line 98-107: We declare and implement the class `MyTest2` in a similar way as `MyTest1`, and instantiate it as `myTest2`
- Line 108-117: We declare and implement the class `MyTestInfo2` in a similar way as `MyTestInfo1`
- Line 119-139: We declare and implement the class `FixtureTest3Helper` in a similar way `FixtureMyTest1Helper`
- Line 141-150: We declare and implement the class `MyTest3` in a similar way as `MyTest1`, and instantiate it as `myTest3`
- Line 151-160: We declare and implement the class `MyTestInfo3` in a similar way as `MyTestInfo1`
- Line 163-168: We declare the class `MyTest`, which derives from `Test`
- Line 169-178: We declare and implement the class `MyTestInfo` in a similar way as `MyTestInfo1`
- Line 180-183: We implement the `RunImpl()` method for `MyTest` as a call to the `OnTestRun()` method on the `TestResults` instance
- Line 189-191: We instantiate each of `MyTestInfo1`, `MyTestInfo2` and `MyTestInfo3`, passing in the instance of the corresponding test
- Line 192: We instantiate `TestFixtureInfo` as our test fixture
- Line 194-196: We add the three tests to the test fixture
- Line 197: We run the test fixture
- Line 198: We clean up the test fixture. Note that the test fixture desctructor deletes all test administration instances, so we don't need to (and shouldn't) do that.
The tests themselves are normally instantiated at static initialization time, so will never be destructed
- Line 199-200: We instantiate a `TestInfo`, and call its `SetTest()` method to link it to the `myTest` instance
- Line 202: We run the test

This all seems like quite a bit of plumbing just to run three tests in a test fixture and one single test.
That is why we'll create macros later to do this work for us.
But it's good to understand what is happening underneath the hood.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests in the test fixture, and therefore show the log output.
You'll see that for each test the `RunImpl()` method of `MyTest<x>` runs.
This then instantiates the `FixtureTest<x>Helper`, and its constructor runs the `FixtureMyTest` method `SetUp()`.
Then the `RunImpl()` of `FixtureTest<x>Helper` is run, and finally the class is destructed again, leading to the `FixtureMyTest` method `TearDown()` begin run.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:201)
Info   DefaultSuite::MyFixture::MyTest1 Start test (TestResults:68)
Debug  Test 1 (main:60)
Debug  MyTest SetUp (main:25)
Debug  DefaultSuite::MyFixture::MyTest1 (../code/applications/demo/src/main.cpp:70) --> MyTestHelper 1 (TestResults:76)
Debug  MyTest TearDown (main:29)
Info   DefaultSuite::MyFixture::MyTest1 Finish test (TestResults:81)
Info   DefaultSuite::MyFixture::MyTest2 Start test (TestResults:68)
Debug  Test 2 (main:103)
Debug  MyTest SetUp (main:25)
Debug  DefaultSuite::MyFixture::MyTest2 (../code/applications/demo/src/main.cpp:113) --> MyTestHelper 2 (TestResults:76)
Debug  MyTest TearDown (main:29)
Info   DefaultSuite::MyFixture::MyTest2 Finish test (TestResults:81)
Info   DefaultSuite::MyFixture::MyTest3 Start test (TestResults:68)
Debug  Test 3 (main:146)
Debug  MyTest SetUp (main:25)
Debug  DefaultSuite::MyFixture::MyTest3 (../code/applications/demo/src/main.cpp:156) --> MyTestHelper 3 (TestResults:76)
Debug  MyTest TearDown (main:29)
Info   DefaultSuite::MyFixture::MyTest3 Finish test (TestResults:81)
Info   DefaultSuite::DefaultFixture::MyTest Start test (TestResults:68)
Debug  DefaultSuite::DefaultFixture::MyTest (../code/applications/demo/src/main.cpp:199) --> Running test (TestResults:76)
Info   DefaultSuite::DefaultFixture::MyTest Finish test (TestResults:81)
Info   Wait 5 seconds (main:204)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Adding test suites - Step 5 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_3}

The final step in building infrasturcture for unit tests is collecting test fixtures in test suites.
Test suites are again different from tests and test fixtures in their structure.

- A TestSuite is actually nothing more than a function returning the suite name. The trick is that the function will be placed inside a namespace, as well as all the test fixtures and tests that belong inside it
- The `TestSuiteInfo` class holds the actual test suite information, such as the pointer to the next test suite, and the pointers to the first and last test fixture in the suite

So the `TestSuiteInfo` class holds the administration of the test suite, like `TestFixtureInfo` and `TestInfo` do for test fixtures and tests.

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_3_TESTSUITEINFOH}

So let's declare the `TestSuiteInfo` class.

Create the file `code/libraries/unittest/include/unittest/TestSuiteInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuiteInfo.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestSuiteInfo.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestSuiteInfo
9: //
10: // Description : Test suite info
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: #include <unittest/TestFixtureInfo.h>
43:
44: namespace unittest
45: {
46:
47: class TestBase;
48: class TestFixtureInfo;
49:
50: class TestSuiteInfo
51: {
52: private:
53:     TestFixtureInfo* m_head;
54:     TestFixtureInfo* m_tail;
55:     TestSuiteInfo* m_next;
56:     baremetal::string m_suiteName;
57:
58: public:
59:     TestSuiteInfo() = delete;
60:     TestSuiteInfo(const TestSuiteInfo&) = delete;
61:     TestSuiteInfo(TestSuiteInfo&&) = delete;
62:     explicit TestSuiteInfo(const baremetal::string& suiteName);
63:     virtual ~TestSuiteInfo();
64:
65:     TestSuiteInfo& operator = (const TestSuiteInfo&) = delete;
66:     TestSuiteInfo& operator = (TestSuiteInfo&&) = delete;
67:
68:     TestFixtureInfo* GetHead() const;
69:
70:     const baremetal::string& Name() const { return m_suiteName; }
71:
72:     void Run();
73:
74:     int CountFixtures();
75:     int CountTests();
76:
77:     void AddFixture(TestFixtureInfo* testFixture);
78:     TestFixtureInfo* GetTestFixture(const baremetal::string& fixtureName);
79: };
80:
81: } // namespace unittest
```

- Line 53-54: The member variables `m_head` and `m_tail` store the pointer to the first and last test fixture in the test suite
- Line 55: The member variable `m_next` is the pointer to the next test suite. Again, test suites are stored in linked list
- Line 56: The member variable `m_suiteName` holds the name of the test suite
- Line 59: We remove the default constructor
- Line 60-61: We remove the copy and move constructors
- Line 62: We declare the only usable constructor which receives the test suite name
- Line 63: We declare the destructor
- Line 65-66: We remove the assignment and move assignment operators
- Line 68: The method `GetHead()` returns the pointer to the first test fixture in the list
- Line 70: The method `Name()` returns the test suite name
- Line 72: The method `Run()` runs all test fixtures in the test suite. We'll be revisiting this later
- Line 74: The method `CountFixtures()` counts and returns the number of test fixtures in the test suite
- Line 75: The method `CountTests()` counts and returns the number of tests in all test fixtures in the test suite
- Line 77: The method `AddFixture()` adds a test fixture to the list for the test suite
- Line 78: The method `GetTestFixture()` finds and returns a test fixture in the list for the test suite, or if not found, creates a new test fixture with the specified name

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_3_TESTFIXTUREINFOH}

As the `TestSuiteInfo` class needs access to the `TestFixtureInfo` in order to access the `m_next` pointer, we need to make it a friend class.

Update the file `code/libraries/unittest/include/unittest/TestFixtureInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixtureInfo.h
...
49: class TestFixtureInfo
50: {
51: private:
52:     friend class TestSuiteInfo;
53:     TestBase* m_head;
54:     TestBase* m_tail;
55:     TestFixtureInfo* m_next;
56:     baremetal::string m_fixtureName;
57:
58: public:
59:     TestFixtureInfo() = delete;
...
```

### TestSuiteInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_3_TESTSUITEINFOCPP}

Let's implement the `TestSuiteInfo` class.

Create the file `code/libraries/unittest/src/TestSuiteInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestSuiteInfo.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestSuiteInfo.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestSuiteInfo
9: //
10: // Description : Test suite info
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
38: //------------------------------------------------------------------------------
39:
40: #include <unittest/TestSuiteInfo.h>
41:
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44:
45: using namespace baremetal;
46:
47: namespace unittest {
48:
49: LOG_MODULE("TestSuiteInfo");
50:
51: TestSuiteInfo::TestSuiteInfo(const string &suiteName)
52:     : m_head{}
53:     , m_tail{}
54:     , m_next{}
55:     , m_suiteName{suiteName}
56: {
57: }
58:
59: TestSuiteInfo::~TestSuiteInfo()
60: {
61:     TestFixtureInfo *testFixture = m_head;
62:     while (testFixture != nullptr)
63:     {
64:         TestFixtureInfo *currentFixture = testFixture;
65:         testFixture                     = testFixture->m_next;
66:         delete currentFixture;
67:     }
68: }
69:
70: TestFixtureInfo *TestSuiteInfo::GetTestFixture(const string &fixtureName)
71: {
72:     TestFixtureInfo *testFixture = m_head;
73:     while ((testFixture != nullptr) && (testFixture->Name() != fixtureName))
74:         testFixture = testFixture->m_next;
75:     if (testFixture == nullptr)
76:     {
77: #ifdef DEBUG_REGISTRY
78:         LOG_DEBUG("Fixture %s not found, creating new object", fixtureName.empty() ? "-" : fixtureName.c_str());
79: #endif
80:         testFixture = new TestFixtureInfo(fixtureName);
81:         AddFixture(testFixture);
82:     }
83:     else
84:     {
85: #ifdef DEBUG_REGISTRY
86:         LOG_DEBUG("Fixture %s found", fixtureName.empty() ? "-" : fixtureName.c_str());
87: #endif
88:     }
89:     return testFixture;
90: }
91:
92: void TestSuiteInfo::AddFixture(TestFixtureInfo *testFixture)
93: {
94:     if (m_tail == nullptr)
95:     {
96:         assert(m_head == nullptr);
97:         m_head = testFixture;
98:         m_tail = testFixture;
99:     }
100:     else
101:     {
102:         m_tail->m_next = testFixture;
103:         m_tail         = testFixture;
104:     }
105: }
106:
107: TestFixtureInfo *TestSuiteInfo::GetHead() const
108: {
109:     return m_head;
110: }
111:
112: void TestSuiteInfo::Run()
113: {
114:     TestFixtureInfo* testFixture = GetHead();
115:     while (testFixture != nullptr)
116:     {
117:         testFixture->Run();
118:         testFixture = testFixture->m_next;
119:     }
120: }
121:
122: int TestSuiteInfo::CountFixtures()
123: {
124:     int              numberOfTestFixtures = 0;
125:     TestFixtureInfo *testFixture          = m_head;
126:     while (testFixture != nullptr)
127:     {
128:         ++numberOfTestFixtures;
129:         testFixture = testFixture->m_next;
130:     }
131:     return numberOfTestFixtures;
132: }
133:
134: int TestSuiteInfo::CountTests()
135: {
136:     int              numberOfTests = 0;
137:     TestFixtureInfo *testFixture   = m_head;
138:     while (testFixture != nullptr)
139:     {
140:         numberOfTests += testFixture->CountTests();
141:         testFixture = testFixture->m_next;
142:     }
143:     return numberOfTests;
144: }
145:
146: } // namespace unittest
```

- Line 51-57: We implement the constructor
- Line 59-68: We implement the destructor. This goes through the list of test fixtures, and deletes every one of these. Note that we will therefore need to create the test fixtures on the heap.
- Line 70-90: We implement the `GetTestFixture()` method. This will try to find the test fixture with the specified name in the test suite. If it is found, the pointer is returned, if not, a new instance iscreated
- Line 92-105: We implement the `AddFixture()` method. This will add the test fixture passed in at the end of the list
- Line 107-110: We implement the `GetHead()` method
- Line 112-120: We implement the `Run()` method. This goes through the list of test fixtures, and calls `Run()` on each
- Line 122-132: We implement the `CountFixtures()` method. This goes through the list of test fixtures, and counts them
- Line 134-144: We implement the `CountTests()` method. This goes through the list of test fixtures, and counts the tests in each of them

### TestSuite.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_3_TESTSUITEH}

The header for the test suite is quite simple, it simple defines the global function `GetSuiteName()`, which is used when not in a namespace.

Create the file `code/libraries/unittest/include/unittest/TestSuite.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuite.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestSuite.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : -
9: //
10: // Description : Test suite functionality
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: inline char const* GetSuiteName()
43: {
44:     return "";
45: }
```

### Update CMake file {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_3_UPDATE_CMAKE_FILE}

As we have added some files to the `unittest` library, we need to update its CMake file.

Update the file `code/libraries/unitttest/CMakeLists.txt`

```cmake
File: code/libraries/unitttest/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
35:     )
36:
37: set(PROJECT_INCLUDES_PUBLIC
38:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
39:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
44:     )
45: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_3_UPDATE_APPLICATION_CODE}

So as a final step let's define test fixtures inside a test suite, and outside any test suite.
We'll add the test fixtures in the test suite, and leave the ones without a suite out of course.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/BCMRegisters.h>
4: #include <baremetal/Console.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/Mailbox.h>
7: #include <baremetal/MemoryManager.h>
8: #include <baremetal/New.h>
9: #include <baremetal/RPIProperties.h>
10: #include <baremetal/Serialization.h>
11: #include <baremetal/String.h>
12: #include <baremetal/SysConfig.h>
13: #include <baremetal/System.h>
14: #include <baremetal/Timer.h>
15: #include <baremetal/Util.h>
16:
17: #include <unittest/TestBase.h>
18: #include <unittest/TestFixture.h>
19: #include <unittest/TestFixtureInfo.h>
20: #include <unittest/TestSuiteInfo.h>
21: #include <unittest/TestSuite.h>
22:
23: LOG_MODULE("main");
24:
25: using namespace baremetal;
26: using namespace unittest;
27:
28: namespace Suite1 {
29:
30: inline char const* GetSuiteName()
31: {
32:     return baremetal::string("Suite1");
33: }
34:
35: class FixtureMyTest1
36:     : public TestFixture
37: {
38: public:
39:     void SetUp() override
40:     {
41:         LOG_DEBUG("FixtureMyTest1 SetUp");
42:     }
43:     void TearDown() override
44:     {
45:         LOG_DEBUG("FixtureMyTest1 TearDown");
46:     }
47: };
48:
49: class FixtureMyTest1Helper
50:     : public FixtureMyTest1
51: {
52: public:
53:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
54:     explicit FixtureMyTest1Helper(const TestDetails& details)
55:         : m_details{ details }
56:     {
57:         SetUp();
58:     }
59:     virtual ~FixtureMyTest1Helper()
60:     {
61:         TearDown();
62:     }
63:     void RunImpl() const;
64:     const TestDetails& m_details;
65: };
66: void FixtureMyTest1Helper::RunImpl() const
67: {
68:     LOG_DEBUG("MyTestHelper 1");
69: }
70:
71: class MyTest1
72:     : public TestBase
73: {
74: public:
75:     MyTest1()
76:         : TestBase("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__)
77:     {
78:
79:     }
80:     void RunImpl() const override
81:     {
82:         LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
83:         FixtureMyTest1Helper fixtureHelper(Details());
84:         fixtureHelper.RunImpl();
85:     }
86: };
87:
88: } // namespace Suite1
89:
90: namespace Suite2 {
91:
92: inline char const* GetSuiteName()
93: {
94:     return baremetal::string("Suite2");
95: }
96:
97: class FixtureMyTest2
98:     : public TestFixture
99: {
100: public:
101:     void SetUp() override
102:     {
103:         LOG_DEBUG("FixtureMyTest2 SetUp");
104:     }
105:     void TearDown() override
106:     {
107:         LOG_DEBUG("FixtureMyTest2 TearDown");
108:     }
109: };
110:
111: class FixtureMyTest2Helper
112:     : public FixtureMyTest2
113: {
114: public:
115:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
116:     explicit FixtureMyTest2Helper(const TestDetails& details)
117:         : m_details{ details }
118:     {
119:         SetUp();
120:     }
121:     virtual ~FixtureMyTest2Helper()
122:     {
123:         TearDown();
124:     }
125:     void RunImpl() const;
126:     const TestDetails& m_details;
127: };
128: void FixtureMyTest2Helper::RunImpl() const
129: {
130:     LOG_DEBUG("MyTestHelper 2");
131: }
132:
133: class MyTest2
134:     : public TestBase
135: {
136: public:
137:     MyTest2()
138:         : TestBase("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__)
139:     {
140:
141:     }
142:     void RunImpl() const override
143:     {
144:         LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
145:         FixtureMyTest2Helper fixtureHelper(Details());
146:         fixtureHelper.RunImpl();
147:     }
148: };
149:
150: } // namespace Suite2
151:
152: class FixtureMyTest3
153:     : public TestFixture
154: {
155: public:
156:     void SetUp() override
157:     {
158:         LOG_DEBUG("FixtureMyTest3 SetUp");
159:     }
160:     void TearDown() override
161:     {
162:         LOG_DEBUG("FixtureMyTest3 TearDown");
163:     }
164: };
165:
166: class FixtureMyTest3Helper
167:     : public FixtureMyTest3
168: {
169: public:
170:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
171:     explicit FixtureMyTest3Helper(const TestDetails& details)
172:         : m_details{ details }
173:     {
174:         SetUp();
175:     }
176:     virtual ~FixtureMyTest3Helper()
177:     {
178:         TearDown();
179:     }
180:     void RunImpl() const;
181:     const TestDetails& m_details;
182: };
183: void FixtureMyTest3Helper::RunImpl() const
184: {
185:     LOG_DEBUG("MyTestHelper 3");
186: }
187:
188: class MyTest3
189:     : public TestBase
190: {
191: public:
192:     MyTest3()
193:         : TestBase("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__)
194:     {
195:
196:     }
197:     void RunImpl() const override
198:     {
199:         LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
200:         FixtureMyTest3Helper fixtureHelper(Details());
201:         fixtureHelper.RunImpl();
202:     }
203: };
204:
205: int main()
206: {
207:     auto& console = GetConsole();
208:     LOG_DEBUG("Hello World!");
209:
210:     TestBase* test1 = new Suite1::MyTest1;
211:     TestBase* test2 = new Suite2::MyTest2;
212:     TestBase* test3 = new MyTest3;
213:     TestFixtureInfo* fixture1 = new TestFixtureInfo("MyFixture1");
214:     fixture1->AddTest(test1);
215:     TestFixtureInfo* fixture2 = new TestFixtureInfo("MyFixture2");
216:     fixture2->AddTest(test2);
217:     TestFixtureInfo* fixture3 = new TestFixtureInfo("MyFixture3");
218:     fixture3->AddTest(test3);
219:     TestSuiteInfo* suite1 = new TestSuiteInfo("MySuite1");
220:     suite1->AddFixture(fixture1);
221:     TestSuiteInfo* suite2 = new TestSuiteInfo("MySuite2");
222:     suite2->AddFixture(fixture2);
223:     TestSuiteInfo* suiteDefault = new TestSuiteInfo("");
224:     suiteDefault->AddFixture(fixture3);
225:     suite1->Run();
226:     suite2->Run();
227:     suiteDefault->Run();
228:     delete suite1;
229:     delete suite2;
230:     delete suiteDefault;
231:
232:     LOG_INFO("Wait 5 seconds");
233:     Timer::WaitMilliSeconds(5000);
234:
235:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
236:     char ch{};
237:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
238:     {
239:         ch = console.ReadChar();
240:         console.WriteChar(ch);
241:     }
242:     if (ch == 'p')
243:         assert(false);
244:
245:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
246: }
```

- Line 20: We include the header for `TestSuiteInfo`
- Line 21: We include the header to define the default `GetSuiteName()` function
- Line 28: We define the namespace Suite1
- Line 30-33: We define the namespace specific version of `GetSuiteName()`
- Line 35-47: We declare and implement a class `FixtureMyTest1` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods.
- Line 49-69: We declare and implement a class `FixtureMyTest1Helper` which derives from the class `FixtureMyTest` just defined, and implements the constructor and destructor, calling resp. `SetUp()` and `TearDown()`.
This class also defines a `RunImpl()` method, which forms the actual test body for the test MyTest1
- Line 71-86: We declare and implement the class `MyTest1`, which derives from `TestBase`, and acts as the placeholders for the fixture test.
Its `RunImpl()` method instantiates `FixtureMyTest1Helper`, and runs its `RunImpl()` method.
Notice that the constructor uses the `GetSuiteName()` function to retrieve the correct test suite name
- Line 88: We end the namespace Suite1
- Line 90: We define the namespace Suite2
- Line 92-95: We define the namespace specific version of `GetSuiteName()`
- Line 97-109: We declare and implement a class `FixtureMyTest2` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods.
- Line 111-131: We declare and implement a class `FixtureMyTest2Helper` which derives from the class `FixtureMyTest` just defined, and implements the constructor and destructor, calling resp. `SetUp()` and `TearDown()`.
This class also defines a `RunImpl()` method, which forms the actual test body for the test MyTest2
- Line 133-148: We declare and implement the class `MyTest2`, which derives from `TestBase`, and acts as the placeholders for the fixture test.
Its `RunImpl()` method instantiates `FixtureMyTest2Helper`, and runs its `RunImpl()` method.
Notice that the constructor uses the `GetSuiteName()` function to retrieve the correct test suite name
- Line 150: We end the namespace Suite2
- Line 152-164: We declare and implement a class `FixtureMyTest3` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods.
- Line 166-186: We declare and implement a class `FixtureMyTest3Helper` which derives from the class `FixtureMyTest` just defined, and implements the constructor and destructor, calling resp. `SetUp()` and `TearDown()`.
This class also defines a `RunImpl()` method, which forms the actual test body for the test MyTest3
- Line 188-203: We declare and implement the class `MyTest3`, which derives from `TestBase`, and acts as the placeholders for the fixture test.
Its `RunImpl()` method instantiates `FixtureMyTest3Helper`, and runs its `RunImpl()` method.
Notice that the constructor uses the `GetSuiteName()` function to retrieve the correct test suite name
- Line 210-212: We instantiate each of `MyTest1` (in namespace `Suite1`), `MyTest2` (in namespace `Suite2`) and `MyTest3` (in global namespace)
- Line 213-214: We instantiate a `TestFixtureInfo` as the first test fixture, and add `MyTest1`
- Line 215-216: We instantiate a `TestFixtureInfo` as our second test fixture, and add `MyTest2`
- Line 217-218: We instantiate a `TestFixtureInfo` as our third test fixture, and add `MyTest3`
- Line 219-220: We instantiate a `TestSuiteInfo` as our first test suite (with name `Suite1`), and add the first test fixture
- Line 221-222: We instantiate a `TestSuiteInfo` as our second test suite (with name `Suite2`), and add the second test fixture
- Line 223-224: We instantiate a `TestSuiteInfo` as our third test suite (with empty name), and add the third test fixture
- Line 225-227: We run the test suites
- Line 228-230: We clean up the test suites. Note that the test suite desctructor deletes all test fixtures and as part of that all tests, so we don't need to (and shouldn't) do that

We've create even more infrastructure to define all test suites, test fixture, tests, and hook them up.
Be patient, we'll create macros later to do this work for us.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests in the test suite, and therefore show the log output.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Info   Starting up (System:201)
Debug  Hello World! (main:208)
Debug  Running MyTest1 in fixture FixtureMyTest1 in suite Suite1 (main:82)
Debug  FixtureMyTest1 SetUp (main:41)
Debug  MyTestHelper 1 (main:68)
Debug  FixtureMyTest1 TearDown (main:45)
Debug  Running MyTest2 in fixture FixtureMyTest2 in suite Suite2 (main:144)
Debug  FixtureMyTest2 SetUp (main:103)
Debug  MyTestHelper 2 (main:130)
Debug  FixtureMyTest2 TearDown (main:107)
Debug  Running MyTest3 in fixture FixtureMyTest3 in suite default (main:199)
Debug  FixtureMyTest3 SetUp (main:158)
Debug  MyTestHelper 3 (main:185)
Debug  FixtureMyTest3 TearDown (main:162)
Info   Wait 5 seconds (main:232)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Test registration - Step 4 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_4}

Now that we have all the classes defined to run tests, we still need to find a way to register them, such that we can ask a test runner to run all (or part of) the tests.
For this we will define a class `TestRegistry`, which will keep administration at the top level (the list of all test suites).
We will also define a class `TestRegistrar` which is used to register tests statically, so that before the application runs, the tests have already been registered.

### TestRegistry.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_4_TESTREGISTRYH}

So let's declare the `TestRegistry` and `TestRegistrar` classes.

Create the file `code/libraries/unittest/include/unittest/TestRegistry.h`

```cpp
File: code/libraries/unittest/include/unittest/TestRegistry.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestRegistry.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestRegistry, TestRegistrar
9: //
10: // Description : Test registration
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: #include <unittest/TestSuiteInfo.h>
43:
44: namespace unittest
45: {
46:
47: class TestBase;
48: class TestSuiteInfo;
49:
50: class TestRegistry
51: {
52: private:
53:     TestSuiteInfo* m_head;
54:     TestSuiteInfo* m_tail;
55:
56: public:
57:     static const char* DefaultFixtureName;
58:     static const char* DefaultSuiteName;
59:
60:     TestRegistry();
61:     TestRegistry(const TestRegistry&) = delete;
62:     TestRegistry(TestRegistry&&) = delete;
63:     virtual ~TestRegistry();
64:
65:     TestRegistry& operator = (const TestRegistry&) = delete;
66:     TestRegistry& operator = (TestRegistry&&) = delete;
67:
68:     TestSuiteInfo* GetTestSuite(const baremetal::string& suiteName);
69:     TestSuiteInfo* GetHead() const;
70:
71:     void Run();
72:     int CountSuites();
73:     int CountFixtures();
74:     int CountTests();
75:
76:     static TestRegistry& GetTestRegistry();
77:
78:     void AddSuite(TestSuiteInfo* testSuite);
79: };
80:
81: class TestRegistrar
82: {
83: public:
84:     TestRegistrar(TestRegistry& registry, TestBase* test);
85: };
86:
87: } // namespace unittest
```

- Line 50-79: We declare the `Testregistry` class
  - Line 53-54: The member variables `m_head` and `m_tail` store the pointer to the first and last test suite
  - Line 57-58: We declare constants to name default test fixture and default test suite
  - Line 60: We declare the default constructor
  - Line 61-62: We remove the copy and move constructors
  - Line 63: We declare the destructor
  - Line 65-66: We remove the assignment and move assignment operators
  - Line 68: The method `GetTestSuite()` finds and returns a test suite in the list, or if not found, creates a new test suite with the specified name
  - Line 69: The method `GetHead()` returns the pointer to the first test suite in the list
  - Line 71: The method `Run()` runs all test suites. We'll be revisiting this later
  - Line 72: The method `CountSuites()` counts and returns the number of test suites in the test suite
  - Line 73: The method `CountFixtures()` counts and returns the number of test fixtures in all test suites
  - Line 74: The method `CountTests()` counts and returns the number of tests in all test fixtures in all test suites
  - Line 76: The static method `GetTestRegistry()` returns the singleton test registry
  - Line 78: The method `AddSuite()` adds a test suite to the list

- Line 81-85: We declare the `TestRegistrar` class
  - Line 84: We declare the only method in the class, the constructor

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_4_TESTSUITEINFOH}

As the `TestRegistry` class needs access to the `TestSuiteInfo` in order to access the `m_next` pointer, we need to make it a friend class.

Update the file `code/libraries/unittest/include/unittest/TestSuiteInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuiteInfo.h
...
50: class TestSuiteInfo
51: {
52: private:
53:     friend class TestRegistry;
54:     TestFixtureInfo* m_head;
55:     TestFixtureInfo* m_tail;
56:     TestSuiteInfo* m_next;
57:     baremetal::string m_suiteName;
58:
59: public:
60:     TestSuiteInfo() = delete;
....
```

### TestRegistry.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_4_TESTREGISTRYCPP}

Let's implement the `TestRegistry` and `TestRegistrar` class.

Create the file `code/libraries/unittest/src/TestRegistry.cpp`

```cpp
File: code/libraries/unittest/src/TestRegistry.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestRegistry.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestRegistry
9: //
10: // Description : Test registration
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
38: //------------------------------------------------------------------------------
39:
40: #include <unittest/TestRegistry.h>
41:
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44:
45: using namespace baremetal;
46:
47: LOG_MODULE("TestRegistry");
48:
49: namespace unittest {
50:
51: const char* TestRegistry::DefaultFixtureName = "DefaultFixture";
52: const char* TestRegistry::DefaultSuiteName = "DefaultSuite";
53:
54: TestRegistry& TestRegistry::GetTestRegistry()
55: {
56:     static TestRegistry s_registry;
57:     return s_registry;
58: }
59:
60: TestRegistry::TestRegistry()
61:     : m_head{}
62:     , m_tail{}
63: {
64: }
65:
66: TestRegistry::~TestRegistry()
67: {
68:     TestSuiteInfo *testSuite = m_head;
69:     while (testSuite != nullptr)
70:     {
71:         TestSuiteInfo *currentSuite = testSuite;
72:         testSuite = testSuite->m_next;
73:         delete currentSuite;
74:     }
75: }
76:
77: TestSuiteInfo *TestRegistry::GetTestSuite(const string &suiteName)
78: {
79:     TestSuiteInfo *testSuite = m_head;
80:     while ((testSuite != nullptr) && (testSuite->Name() != suiteName))
81:         testSuite = testSuite->m_next;
82:     if (testSuite == nullptr)
83:     {
84: #ifdef DEBUG_REGISTRY
85:         LOG_DEBUG("Find suite %s ... not found, creating new object", (suiteName.empty() ? DefaultSuiteName : suiteName.c_str()));
86: #endif
87:         testSuite = new TestSuiteInfo(suiteName);
88:         AddSuite(testSuite);
89:     }
90:     else
91:     {
92: #ifdef DEBUG_REGISTRY
93:         LOG_DEBUG("Find suite %s ... found", (suiteName.empty() ? DefaultSuiteName : suiteName.c_str()));
94: #endif
95:     }
96:     return testSuite;
97: }
98:
99: void TestRegistry::AddSuite(TestSuiteInfo *testSuite)
100: {
101:     if (m_tail == nullptr)
102:     {
103:         assert(m_head == nullptr);
104:         m_head = testSuite;
105:         m_tail = testSuite;
106:     }
107:     else
108:     {
109:         m_tail->m_next = testSuite;
110:         m_tail = testSuite;
111:     }
112: }
113:
114: TestSuiteInfo *TestRegistry::GetHead() const
115: {
116:     return m_head;
117: }
118:
119: void TestRegistry::Run()
120: {
121:     TestSuiteInfo* curTestSuite = GetHead();
122:
123:     while (curTestSuite != nullptr)
124:     {
125:         curTestSuite->Run();
126:         curTestSuite = curTestSuite->m_next;
127:     }
128: }
129:
130: int TestRegistry::CountSuites()
131: {
132:     int            numberOfTestSuites = 0;
133:     TestSuiteInfo *testSuite          = m_head;
134:     while (testSuite != nullptr)
135:     {
136:         ++numberOfTestSuites;
137:         testSuite = testSuite->m_next;
138:     }
139:     return numberOfTestSuites;
140: }
141:
142: int TestRegistry::CountFixtures()
143: {
144:     int            numberOfTestFixtures = 0;
145:     TestSuiteInfo *testSuite            = m_head;
146:     while (testSuite != nullptr)
147:     {
148:         numberOfTestFixtures += testSuite->CountFixtures();
149:         testSuite = testSuite->m_next;
150:     }
151:     return numberOfTestFixtures;
152: }
153:
154: int TestRegistry::CountTests()
155: {
156:     int            numberOfTests = 0;
157:     TestSuiteInfo *testSuite     = m_head;
158:     while (testSuite != nullptr)
159:     {
160:         numberOfTests += testSuite->CountTests();
161:         testSuite = testSuite->m_next;
162:     }
163:     return numberOfTests;
164: }
165:
166: TestRegistrar::TestRegistrar(TestRegistry &registry, TestBase *test)
167: {
168: #ifdef DEBUG_REGISTRY
169:     LOG_DEBUG("Register test %s in fixture %s in suite %s",
170:         test->Details().TestName().c_str(),
171:         (test->Details().FixtureName().empty() ? TestRegistry::DefaultFixtureName : test->Details().FixtureName().c_str()),
172:         (test->Details().SuiteName().empty() ? TestRegistry::DefaultSuiteName : test->Details().SuiteName().c_str()));
173: #endif
174:     TestSuiteInfo   *testSuite   = registry.GetTestSuite(test->Details().SuiteName());
175:     TestFixtureInfo *testFixture = testSuite->GetTestFixture(test->Details().FixtureName());
176:     testFixture->AddTest(test);
177: }
178:
179: } // namespace unittest
```

- Line 54-57: We implement the static method `GetTestRegistry()`
- Line 60-64: We implement the constructor
- Line 66-75: We implement the destructor. This goes through the list of test suites, and deletes every one of these. Note that we will therefore need to create the test suites on the heap.
- Line 77-97: We implement the `GetTestSuite()` method. This will try to find the test suite with the specified name. If it is found, the pointer is returned, if not, a new instance iscreated
- Line 99-112: We implement the `AddSuite()` method. This will add the test suite passed in at the end of the list
- Line 114-117: We implement the `GetHead()` method
- Line 119-128: We implement the `Run()` method. This goes through the list of test suites, and calls `Run()` on each
- Line 130-140: We implement the `CountSuites()` method. This goes through the list of test suites, and counts them
- Line 142-152: We implement the `CountFixtures()` method. This goes through the list of test suites, and counts the test fixtures in each of them
- Line 154-164: We implement the `CountTests()` method. This goes through the list of test suites, and counts the tests in each of them
- Line 166-177: We implement `TestRegistrar` contructor. This is intended for the macros we will get to next.
The constructor receives a reference to the test registry, as well as a pointer to a test (derived from `TestBase`), and adds the test to the registry.
The `TestDetails` which are part of the test are used to lookup and if needed create the surrounding test fixture and test suite

### Debugging registry {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_4_DEBUGGING_REGISTRY}

In the `TestRegistry` implementation we use the definition `DEBUG_REGISTRY` to debug the registration of tests.
As we're interested to see how our new code registers, let's switch on the debugging.

Update the file `CMakeLists.txt`

```cmake
File: CMakeLists.txt
...
66: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" OFF)
67: option(BAREMETAL_DEBUG_UNITTEST_REGISTRY "Enable debug tracing output for unittest registry" ON)
68:
69: message(STATUS "\n** Setting up project **\n--")
...
107: if (BAREMETAL_DEBUG_UNITTEST_REGISTRY)
108:     set(BAREMETAL_UNITTEST_REGISTRY_TRACING 1)
109: else ()
110:     set(BAREMETAL_UNITTEST_REGISTRY_TRACING 0)
111: endif()
112: set(BAREMETAL_LOAD_ADDRESS 0x80000)
113:
114: set(DEFINES_C
115:     PLATFORM_BAREMETAL
116:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
117:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
118:     BAREMETAL_DEBUG_TRACING=${BAREMETAL_DEBUG_TRACING}
119:     BAREMETAL_MEMORY_TRACING=${BAREMETAL_MEMORY_TRACING}
120:     BAREMETAL_MEMORY_TRACING_DETAIL=${BAREMETAL_MEMORY_TRACING_DETAIL}
121:     DEBUG_REGISTRY=${BAREMETAL_UNITTEST_REGISTRY_TRACING}
122:     USE_PHYSICAL_COUNTER
123:     BAREMETAL_MAJOR=${VERSION_MAJOR}
124:     BAREMETAL_MINOR=${VERSION_MINOR}
125:     BAREMETAL_LEVEL=${VERSION_LEVEL}
126:     BAREMETAL_BUILD=${VERSION_BUILD}
127:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
128:     )
```

- Line 67: We add a new variable `BAREMETAL_DEBUG_UNITTEST_REGISTRY` and set it to `ON` by default
- Line 107-111: Dependin on the value of `BAREMETAL_DEBUG_UNITTEST_REGISTRY` we set the variable `BAREMETAL_UNITTEST_REGISTRY_TRACING` to either 1 or 0
- Line 121: We add a definition for `DEBUG_REGISTRY` using the value of variable `BAREMETAL_UNITTEST_REGISTRY_TRACING`

### Update CMake file {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_4_UPDATE_CMAKE_FILE}

As we have added some files to the `unittest` library, we need to update its CMake file.

Update the file `code/libraries/unitttest/CMakeLists.txt`

```cmake
File: code/libraries/unitttest/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
36:     )
37:
38: set(PROJECT_INCLUDES_PUBLIC
39:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
46:     )
47: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_4_UPDATE_APPLICATION_CODE}

Now we can use the TestRegistry. Even though we still need to create quite some classes, the plumbing of registering the classes is now taken care of by the `TestRegistry` class, and instances of the `TestRegistrar` class.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/BCMRegisters.h>
4: #include <baremetal/Console.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/Mailbox.h>
7: #include <baremetal/MemoryManager.h>
8: #include <baremetal/New.h>
9: #include <baremetal/RPIProperties.h>
10: #include <baremetal/Serialization.h>
11: #include <baremetal/String.h>
12: #include <baremetal/SysConfig.h>
13: #include <baremetal/System.h>
14: #include <baremetal/Timer.h>
15: #include <baremetal/Util.h>
16:
17: #include <unittest/TestBase.h>
18: #include <unittest/TestFixture.h>
19: #include <unittest/TestFixtureInfo.h>
20: #include <unittest/TestRegistry.h>
21: #include <unittest/TestSuiteInfo.h>
22: #include <unittest/TestSuite.h>
23:
24: LOG_MODULE("main");
25:
26: using namespace baremetal;
27: using namespace unittest;
28:
29: namespace Suite1 {
30:
31: inline char const* GetSuiteName()
32: {
33:     return baremetal::string("Suite1");
34: }
35:
36: class FixtureMyTest1
37:     : public TestFixture
38: {
39: public:
40:     void SetUp() override
41:     {
42:         LOG_DEBUG("FixtureMyTest1 SetUp");
43:     }
44:     void TearDown() override
45:     {
46:         LOG_DEBUG("FixtureMyTest1 TearDown");
47:     }
48: };
49:
50: class FixtureMyTest1Helper
51:     : public FixtureMyTest1
52: {
53: public:
54:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
55:     explicit FixtureMyTest1Helper(const TestDetails& details)
56:         : m_details{ details }
57:     {
58:         SetUp();
59:     }
60:     virtual ~FixtureMyTest1Helper()
61:     {
62:         TearDown();
63:     }
64:     FixtureMyTest1Helper& operator = (const FixtureMyTest1Helper&) = delete;
65:     void RunImpl() const;
66:     const TestDetails& m_details;
67: };
68: void FixtureMyTest1Helper::RunImpl() const
69: {
70:     LOG_DEBUG(m_details.FixtureName().c_str());
71: }
72:
73: class MyTest1
74:     : public TestBase
75: {
76: public:
77:     MyTest1()
78:         : TestBase("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__)
79:     {
80:
81:     }
82:     void RunImpl() const override;
83: } MyTest1Instance;
84:
85: TestRegistrar registrarFixtureMyTest1(TestRegistry::GetTestRegistry(), &MyTest1Instance);
86:
87: void MyTest1::RunImpl() const
88: {
89:     LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
90:     FixtureMyTest1Helper fixtureHelper(Details());
91:     fixtureHelper.RunImpl();
92: }
93:
94: } // namespace Suite1
95:
96: namespace Suite2 {
97:
98: inline char const* GetSuiteName()
99: {
100:     return baremetal::string("Suite2");
101: }
102:
103: class FixtureMyTest2
104:     : public TestFixture
105: {
106: public:
107:     void SetUp() override
108:     {
109:         LOG_DEBUG("FixtureMyTest2 SetUp");
110:     }
111:     void TearDown() override
112:     {
113:         LOG_DEBUG("FixtureMyTest2 TearDown");
114:     }
115: };
116:
117: class FixtureMyTest2Helper
118:     : public FixtureMyTest2
119: {
120: public:
121:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
122:     explicit FixtureMyTest2Helper(const TestDetails& details)
123:         : m_details{ details }
124:     {
125:         SetUp();
126:     }
127:     virtual ~FixtureMyTest2Helper()
128:     {
129:         TearDown();
130:     }
131:     FixtureMyTest2Helper& operator = (const FixtureMyTest2Helper&) = delete;
132:     void RunImpl() const;
133:     const TestDetails& m_details;
134: };
135: void FixtureMyTest2Helper::RunImpl() const
136: {
137:     LOG_DEBUG(m_details.FixtureName().c_str());
138: }
139:
140: class MyTest2
141:     : public TestBase
142: {
143: public:
144:     MyTest2()
145:         : TestBase("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__)
146:     {
147:
148:     }
149:     void RunImpl() const override;
150: } MyTest2Instance;
151:
152: TestRegistrar registrarFixtureMyTest2(TestRegistry::GetTestRegistry(), &MyTest2Instance);
153:
154: void MyTest2::RunImpl() const
155: {
156:     LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
157:     FixtureMyTest2Helper fixtureHelper(Details());
158:     fixtureHelper.RunImpl();
159: }
160:
161: } // namespace Suite2
162:
163: class FixtureMyTest3
164:     : public TestFixture
165: {
166: public:
167:     void SetUp() override
168:     {
169:         LOG_DEBUG("FixtureMyTest3 SetUp");
170:     }
171:     void TearDown() override
172:     {
173:         LOG_DEBUG("FixtureMyTest3 TearDown");
174:     }
175: };
176:
177: class FixtureMyTest3Helper
178:     : public FixtureMyTest3
179: {
180: public:
181:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
182:     explicit FixtureMyTest3Helper(const TestDetails& details)
183:         : m_details{ details }
184:     {
185:         SetUp();
186:     }
187:     virtual ~FixtureMyTest3Helper()
188:     {
189:         TearDown();
190:     }
191:     FixtureMyTest3Helper& operator = (const FixtureMyTest3Helper&) = delete;
192:     void RunImpl() const;
193:     const TestDetails& m_details;
194: };
195: void FixtureMyTest3Helper::RunImpl() const
196: {
197:     LOG_DEBUG(m_details.FixtureName().c_str());
198: }
199:
200: class MyTest3
201:     : public TestBase
202: {
203: public:
204:     MyTest3()
205:         : TestBase("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__)
206:     {
207:
208:     }
209:     void RunImpl() const override;
210: } MyTest3Instance;
211:
212: TestRegistrar registrarFixtureMyTest3(TestRegistry::GetTestRegistry(), &MyTest3Instance);
213:
214: void MyTest3::RunImpl() const
215: {
216:     LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
217:     FixtureMyTest3Helper fixtureHelper(Details());
218:     fixtureHelper.RunImpl();
219: }
220:
221: int main()
222: {
223:     auto& console = GetConsole();
224:     LOG_DEBUG("Hello World!");
225:
226:     TestRegistry::GetTestRegistry().Run();
227:
228:     LOG_INFO("Wait 5 seconds");
229:     Timer::WaitMilliSeconds(5000);
230:
231:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
232:     char ch{};
233:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
234:     {
235:         ch = console.ReadChar();
236:         console.WriteChar(ch);
237:     }
238:     if (ch == 'p')
239:         assert(false);
240:
241:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
242: }
```

- Line 20: We include the header for `TestRegistry` and `TestRegistrar`
- Line 29: We define the namespace Suite1 as before
- Line 31-34: We define the namespace specific version of `GetSuiteName()` as before
- Line 36-48: We declare and implement a class `FixtureMyTest1` as before
- Line 50-71: We declare and implement a class `FixtureMyTest1Helper` as before.
The only changes are removing the assignment operator, and using the details to print the test fixture name
- Line 73-83: We declare the class `MyTest1` as before.
The `RunImpl()` method is now defined later. Note that we create an instance of `Mytest1` named `MyTest1Instance`
- Line 85: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest1` instance just defined
- Line 87-92: We implement the `RunImpl()` method of `MyTest1` as before
- Line 94: We end the namespace Suite1 as before
- Line 96: We define the namespace Suite2 as before
- Line 98-101: We define the namespace specific version of `GetSuiteName()` as before
- Line 103-115: We declare and implement a class `FixtureMyTest2` as before
- Line 117-138: We declare and implement a class `FixtureMyTest2Helper` as before.
The only changes are removing the assignment operator, and using the details to print the test fixture name
- Line 140-150: We declare the class `MyTest2`as before.
The `RunImpl()` method is now defined later. Note that we create an instance of `Mytest2` named `MyTest2Instance`
- Line 152: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest2` instance just defined
- Line 154-159: We implement the `RunImpl()` method of `MyTest2` as before
- Line 161: We end the namespace Suite2 as before
- Line 163-175: We declare and implement a class `FixtureMyTest3` as before
- Line 177-198: We declare and implement a class `FixtureMyTest3Helper` as before.
The only changes are removing the assignment operator, and using the details to print the test fixture name
- Line 200-210: We declare the class `MyTest3` as before.
The `RunImpl()` method is now defined later. Note that we create an instance of `Mytest3` named `MyTest3Instance`
- Line 212: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest3` instance just defined
- Line 214-219: We implement the `RunImpl()` method of `MyTest3` as before
- Line 226: We retrieve the test registry, and call its `Run()` method

Note that we don't need to clean up anymore, that is all taken care of.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will, at static initialization time, register all the tests, and then run all tests registered.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Debug  Register test MyTest1 in fixture FixtureMyTest1 in suite Suite1 (TestRegistry:169)
Debug  Find suite Suite1 ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest1 not found, creating new object (TestSuiteInfo:78)
Debug  Register test MyTest2 in fixture FixtureMyTest2 in suite Suite2 (TestRegistry:169)
Debug  Find suite Suite2 ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest2 not found, creating new object (TestSuiteInfo:78)
Debug  Register test MyTest3 in fixture FixtureMyTest3 in suite DefaultSuite (TestRegistry:169)
Debug  Find suite DefaultSuite ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest3 not found, creating new object (TestSuiteInfo:78)
Info   Starting up (System:201)
Debug  Hello World! (main:224)
Debug  Running MyTest1 in fixture FixtureMyTest1 in suite Suite1 (main:89)
Debug  FixtureMyTest1 SetUp (main:42)
Debug  FixtureMyTest1 (main:70)
Debug  FixtureMyTest1 TearDown (main:46)
Debug  Running MyTest2 in fixture FixtureMyTest2 in suite Suite2 (main:156)
Debug  FixtureMyTest2 SetUp (main:109)
Debug  FixtureMyTest2 (main:137)
Debug  FixtureMyTest2 TearDown (main:113)
Debug  Running MyTest3 in fixture FixtureMyTest3 in suite default (main:216)
Debug  FixtureMyTest3 SetUp (main:169)
Debug  FixtureMyTest3 (main:197)
Debug  FixtureMyTest3 TearDown (main:173)
Info   Wait 5 seconds (main:228)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Test runner and visitor - Step 5 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5}

### ITestReporter.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_ITESTREPORTERH}

We'll add a visitor interface, which we'll implement later on as a test reporter, which will print the progress and results of the tests.

Create the file `code/libraries/unittest/include/unittest/ITestReporter.h`

```cpp
File: code/libraries/unittest/include/unittest/ITestReporter.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ITestReporter.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : ITestReporter
9: //
10: // Description : Test reporter abstract class
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: #include <baremetal/String.h>
43:
44: namespace unittest
45: {
46:
47: class TestDetails;
48: class TestResults;
49:
50: class ITestReporter
51: {
52: public:
53:     virtual ~ITestReporter() {}
54:
55:     virtual void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
56:     virtual void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
57:     virtual void ReportTestRunSummary(const TestResults& results) = 0;
58:     virtual void ReportTestRunOverview(const TestResults& results) = 0;
59:     virtual void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTests) = 0;
60:     virtual void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTests) = 0;
61:     virtual void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) = 0;
62:     virtual void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests) = 0;
63:     virtual void ReportTestStart(const TestDetails& details) = 0;
64:     virtual void ReportTestFinish(const TestDetails& details, bool success) = 0;
65:     virtual void ReportTestFailure(const TestDetails& details, const baremetal::string& failure) = 0;
66: };
67:
68: } // namespace unittest
```

- Line 50-68: We declare the abstract interface class ITestReporter
  - Line 55: The method `ReportTestRunStart()` will be called before any tests are run, to give an overview of the number of test suites, test fixtures and tests
  - Line 56: The method `ReportTestRunStart()` will be called just after the tests are run, to again give an overview of the number of test suites, test fixtures and tests
  - Line 57: The method `ReportTestSummary()` will be called after all tests are finished, to show a summary of the number of tests run, and the number of failing tests
  - Line 58: The method `ReportTestRunOverview()` will print the list of failed tests, with information of why they failed
  - Line 59: The method `ReportTestSuiteStart()` will print a marker for the start of a test suite run
  - Line 60: The method `ReportTestSuiteFinish()` will print a marker for the end of a test suite run
  - Line 61: The method `ReportTestFixtureStart()` will print a marker for the start of a test fixture run
  - Line 62: The method `ReportTestFixtureFinish()` will print a marker for the end of a test fixture run
  - Line 63: The method `ReportTestStart()` will print a marker for the start of a test run
  - Line 64: The method `ReportTestFinish()` will print a marker for the end of a test run
  - Line 65: The method `ReportTestFailure()` will print a statement that a failure was found, with information

### TestResults.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTRESULTSH}

When running tests, we need to keep track of which tests were run, and which failures occurred.
We will use the class `TestResults` for this purpose.

Create the file `code/libraries/unittest/include/unittest/TestResults.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResults.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestResults.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestResults
9: //
10: // Description : Test results
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: #include <baremetal/String.h>
43:
44: namespace unittest
45: {
46:
47: class ITestReporter;
48: class TestDetails;
49: class TestSuiteInfo;
50: class TestFixtureInfo;
51:
52: class TestResults
53: {
54: public:
55:     explicit TestResults(ITestReporter* reporter = nullptr);
56:     TestResults(const TestResults&) = delete;
57:     TestResults(TestResults&&) = delete;
58:     virtual ~TestResults();
59:
60:     TestResults& operator = (const TestResults&) = delete;
61:     TestResults& operator = (TestResults&&) = delete;
62:
63:     void OnTestSuiteStart(TestSuiteInfo* suite);
64:     void OnTestSuiteFinish(TestSuiteInfo* suite);
65:     void OnTestFixtureStart(TestFixtureInfo* fixture);
66:     void OnTestFixtureFinish(TestFixtureInfo* fixture);
67:     void OnTestStart(const TestDetails& details);
68:     void OnTestFailure(const TestDetails& details, const baremetal::string& message);
69:     void OnTestFinish(const TestDetails& details);
70:
71:     int GetTotalTestCount() const;
72:     int GetFailedTestCount() const;
73:     int GetFailureCount() const;
74:
75: private:
76:     ITestReporter* m_reporter;
77:     int m_totalTestCount;
78:     int m_failedTestCount;
79:     int m_failureCount;
80:     bool m_currentTestFailed;
81: };
82:
83: } // namespace unittest
```

- Line 52-81: We declare the class `TestResults`
  - Line 55: We declare an explicit constructor taking a pointer to a test reporter interface
  - Line 56-57: We remove the copy and move constructors
  - Line 58: We declare the constructor
  - Line 60-61: We remove the assignment and move assignment operators
  - Line 63: The method `OnTestSuiteStart()` marks the start of a test suite run. This will also call `ReportTestSuiteStart()` on the test reporter
  - Line 64: The method `OnTestSuiteFinish()` marks the end of a test suite run. This will also call `ReportTestSuiteFinish()` on the test reporter
  - Line 65: The method `OnTestFixtureStart()` marks the start of a test fixture run. This will also call `ReportTestFixtureStart()` on the test reporter
  - Line 66: The method `OnTestFixtureFinish()` marks the end of a test fixture run. This will also call `ReportTestFixtureFinish()` on the test reporter
  - Line 67: The method `OnTestStart()` marks the start of a test run. This will also call `ReportTestStart()` on the test reporter
  - Line 68: The method `OnTestFailure()` marks the failure in a test run. This will also call `ReportTestFailure()` on the test reporter
  - Line 69: The method `OnTestFinish()` marks the end of a test run. This will also call `ReportTestFinish()` on the test reporter
  - Line 71: The method `GetTotalTestCount()` returns the total number of tests run
  - Line 72: The method `GetFailedTestCount()` returns the total number of tests that failed
  - Line 73: The method `GetFailureCount()` returns the total number of failures found. A test can have more than one failure

### TestResults.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTRESULTSCPP}

Let's implement the `TestResults` class.

Create the file `code/libraries/unittest/src/TestResults.cpp`

```cpp
File: code/libraries/unittest/src/TestResults.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestResults.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestResults
9: //
10: // Description : Test results
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
38: //------------------------------------------------------------------------------
39:
40: #include <unittest/TestResults.h>
41:
42: #include <unittest/ITestReporter.h>
43: #include <unittest/TestSuiteInfo.h>
44:
45: using namespace baremetal;
46:
47: namespace unittest {
48:
49: TestResults::TestResults(ITestReporter* testReporter)
50:     : m_reporter{ testReporter }
51:     , m_totalTestCount{}
52:     , m_failedTestCount{}
53:     , m_failureCount{}
54:     , m_currentTestFailed{}
55: {
56: }
57:
58: TestResults::~TestResults()
59: {
60: }
61:
62: void TestResults::OnTestSuiteStart(TestSuiteInfo* suite)
63: {
64:     if (m_reporter)
65:         m_reporter->ReportTestSuiteStart(suite->Name(), suite->CountFixtures());
66: }
67:
68: void TestResults::OnTestSuiteFinish(TestSuiteInfo* suite)
69: {
70:     if (m_reporter)
71:         m_reporter->ReportTestSuiteFinish(suite->Name(), suite->CountFixtures());
72: }
73:
74: void TestResults::OnTestFixtureStart(TestFixtureInfo* fixture)
75: {
76:     if (m_reporter)
77:         m_reporter->ReportTestFixtureStart(fixture->Name(), fixture->CountTests());
78: }
79:
80: void TestResults::OnTestFixtureFinish(TestFixtureInfo* fixture)
81: {
82:     if (m_reporter)
83:         m_reporter->ReportTestFixtureFinish(fixture->Name(), fixture->CountTests());
84: }
85:
86: void TestResults::OnTestStart(const TestDetails& details)
87: {
88:     ++m_totalTestCount;
89:     m_currentTestFailed = false;
90:     if (m_reporter)
91:         m_reporter->ReportTestStart(details);
92: }
93:
94: void TestResults::OnTestFailure(const TestDetails& details, const string& result)
95: {
96:     ++m_failureCount;
97:     if (!m_currentTestFailed)
98:     {
99:         ++m_failedTestCount;
100:         m_currentTestFailed = true;
101:     }
102:     if (m_reporter)
103:         m_reporter->ReportTestFailure(details, result);
104: }
105:
106: void TestResults::OnTestFinish(const TestDetails& details)
107: {
108:     if (m_reporter)
109:         m_reporter->ReportTestFinish(details, !m_currentTestFailed);
110: }
111:
112: int TestResults::GetTotalTestCount() const
113: {
114:     return m_totalTestCount;
115: }
116:
117: int TestResults::GetFailedTestCount() const
118: {
119:     return m_failedTestCount;
120: }
121:
122: int TestResults::GetFailureCount() const
123: {
124:     return m_failureCount;
125: }
126:
127: } // namespace unittest
```

Most methods are quite straightforward.

- Line 86-92: The `OnTestStart()` method also counts the tests run, and resets the flag whether the test failed
- Line 94-104: The `OnTestFailure()` method increments the failure count, and if this is the first failure in the test, also increments the failed test count

### TestRunner.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTRUNNERH}

Now that we have registered tests, we can define a class `TestRunner` that can run the registered tests.
This will introduce a method for filtering which tests to run.

Create the file `code/libraries/unittest/include/unittest/TestRunner.h`

```cpp
File: code/libraries/unittest/include/unittest/TestRunner.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestRunner.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestRunner
9: //
10: // Description : Test runner
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: #include <unittest/ITestReporter.h>
43: #include <unittest/TestRegistry.h>
44: #include <unittest/TestResults.h>
45:
46: namespace unittest
47: {
48:
49: class ITestReporter;
50: class Test;
51: class TestFixtureInfo;
52: class TestResults;
53: class TestSuiteInfo;
54:
55: struct True
56: {
57:     bool operator()(const TestBase* const ) const
58:     {
59:         return true;
60:     }
61:     bool operator()(const TestFixtureInfo* const ) const
62:     {
63:         return true;
64:     }
65:     bool operator()(const TestSuiteInfo* const ) const
66:     {
67:         return true;
68:     }
69: };
70:
71: class InSelection
72: {
73: private:
74:     const char* m_suiteName;
75:     const char* m_fixtureName;
76:     const char* m_testName;
77:
78: public:
79:     InSelection(const char* suiteName, const char* fixtureName, const char* testName)
80:         : m_suiteName{ suiteName }
81:         , m_fixtureName{ fixtureName }
82:         , m_testName{ testName }
83:     {
84:     }
85:     bool operator()(const TestBase* const test) const;
86:     bool operator()(const TestFixtureInfo* const fixture) const;
87:     bool operator()(const TestSuiteInfo* const suite) const;
88: };
89:
90: class TestRunner
91: {
92: private:
93:     ITestReporter* m_reporter;
94:     TestResults m_testResults;
95:
96: public:
97:     TestRunner(const TestRunner&) = delete;
98:     explicit TestRunner(ITestReporter* reporter);
99:     ~TestRunner();
100:
101:     TestRunner & operator = (const TestRunner&) = delete;
102:
103:     template <class Predicate>
104:     int RunTestsIf(TestRegistry const& registry, const Predicate& predicate);
105:
106: private:
107:     template <class Predicate>
108:     void Start(const Predicate& predicate) const;
109:
110:     template <class Predicate>
111:     int Finish(const Predicate& predicate) const;
112: };
113:
114: template <class Predicate>
115: int TestRunner::RunTestsIf(TestRegistry const& registry, const Predicate& predicate)
116: {
117:     Start(predicate);
118:
119:     TestRegistry::GetTestRegistry().RunIf(predicate, m_testResults);
120:
121:     return Finish(predicate);
122: }
123:
124: template <class Predicate>
125: void TestRunner::Start(const Predicate& predicate) const
126: {
127:     TestRegistry& registry = TestRegistry::GetTestRegistry();
128:     int numberOfTestSuites = registry.CountSuitesIf(predicate);
129:     int numberOfTestFixtures = registry.CountFixturesIf(predicate);
130:     int numberOfTests = registry.CountTestsIf(predicate);
131:     m_reporter->ReportTestRunStart(numberOfTestSuites, numberOfTestFixtures, numberOfTests);
132: }
133:
134: template <class Predicate>
135: int TestRunner::Finish(const Predicate & predicate) const
136: {
137:     m_reporter->ReportTestRunSummary(m_testResults);
138:     m_reporter->ReportTestRunOverview(m_testResults);
139:
140:     TestRegistry& registry = TestRegistry::GetTestRegistry();
141:     int numberOfTestSuites = registry.CountSuitesIf(predicate);
142:     int numberOfTestFixtures = registry.CountFixturesIf(predicate);
143:     int numberOfTests = registry.CountTestsIf(predicate);
144:     m_reporter->ReportTestRunFinish(numberOfTestSuites, numberOfTestFixtures, numberOfTests);
145:
146:     return m_testResults.GetFailureCount();
147: }
148:
149: int RunAllTests(ITestReporter* reporter);
150:
151: template <class Predicate>
152: int RunSelectedTests(ITestReporter* reporter, const Predicate& predicate)
153: {
154:     TestRunner runner(reporter);
155:     return runner.RunTestsIf(TestRegistry::GetTestRegistry(), predicate);
156: }
157:
158: } // namespace unittest
```

- Line 55-69: We declare a struct `True` the holds three operators, that all return true. This isused for filtering test suites, test fixtures and tests
  - Line 57-60: The operator `(const TestBase*)` will match to any test
  - Line 61-64: The operator `(const TestFixtureInfo*)` will match to any test fixture
  - Line 65-68: The operator `(const TestSuiteInfo*)` will match to any test suite
- Line 71-88: We declare a similar struct `InSelection` that hold the same three operators.
This one however will returns true for a method, if the corresponding test name, test fixture name or test suite name is equal to a set value
  - Line 74-76: We declare class variables `m_suitename`, `m_fixtureName` and `m_testName` as a pointer to a string to contain the  test suite name, test fixture name, and test name to use as a filter
  - Line 77-84: We define a constructor, which takes a test suite name, a test fixture name, and a test name, each as a character pointer, which may be nullptr.
The corresponding values are saved as the test suite name, a test fixture name, and a test name to filter on
  - Line 85: We declare the operator `(const TestBase*)` which will return true if the test name is equal to the set filter value `m_testName`
  - Line 86: We declare the operator `(const TestFixtureInfo*)` which will return true if the test fixture name is equal to the set filter value `m_fixtureName`
  - Line 87: We declare the operator `(const TestSuiteInfo*)` which will return true if the test suite name is equal to the set filter value `m_suiteName`
- Line 90-112: We declare the class `TestRunner` which allows to run tests with a set filter, which will use a test reporter instance for reporting
  - Line 93: `m_reporter` stores the passed test reporter instance pointer
  - Line 94: `m_testResults` stores a pointer to the test results
  - Line 97: We remove the copy constructor
  - Line 98: We declare an explicit constructor taking a test reporter instance pointer
  - Line 99: We declare a destructor
  - Line 101: We remove the assignment operator
  - Line 103: We declare a template method `RunTestsIf()` that takes a predicate (which could be the an instance of the `True` class or an instance or the `InSelection` class, or any other class that supportes the same three `()` operators).
This method will run any test that matches the predicate passed
  - Line 107-108: We declare a private template method `Start()` that takes a predicate.
This collects information on the number of test suites, test fixtures and tests, and report the start of the test run
  - Line 110-111: We declare a private template method `Finish()` that takes a predicate.
This reports a test run summary and overview, and reports the end of the test run
- Line 114-122: We implement the `RunTestsIf()` template method.
This will use the method `RunIf()` to run tests matching the predicate, with the test results passed to fill
- Line 124-132: We implement the `Start()` template method.
Note that it runs methods `CountSuitesIf()`, `CountFixturesIf()` and `CountTestsIf()` to count test suites, test fixtures and test, using the predicate.
We will need to implement those in the `TestRegistry` class
- Line 134-147: We implement the `Finish()` template method.
Again, the methods `CountSuitesIf()`, `CountFixturesIf()` and `CountTestsIf()` are used to count tests, etc.
The method returns the number of failures, which is reported back by the `RunTestsIf()` template method
- Line 149: We declare a function `RunAllTests()` which will simply run all tests and use the passed test reporter for reporting
- Line 151-156: We define a template function `RunSelectedTests()` which will run all tests matching a predicate and use the passed test reporter for reporting

### TestRunner.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTRUNNERCPP}

Let's implement the `TestRunner` class. Most of the methods are template methods, so we will only need to imlement part here.

Create the file `code/libraries/unittest/src/TestRunner.cpp`

```cpp
File: code/libraries/unittest/src/TestRunner.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestRunner.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestRunner
9: //
10: // Description : Test runner
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
38: //------------------------------------------------------------------------------
39:
40: #include <unittest/TestRunner.h>
41:
42: namespace unittest {
43:
44: TestRunner::TestRunner(ITestReporter* reporter)
45:     : m_reporter{ reporter }
46:     , m_testResults{ reporter }
47: {
48: }
49:
50: TestRunner::~TestRunner()
51: {
52: }
53:
54: int RunAllTests(ITestReporter* reporter)
55: {
56:     return RunSelectedTests(reporter, True());
57: }
58:
59: } // namespace unittest
```

- Line 44-48: We define the constructor
- Line 50-53: We define the destructor
- Line 54-57: We implement the function `RunAllTests()`

### TestRegistry.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTREGISTRYH}

In order to be able to run tests with a predicate, we need to update the `TestRegistry` class.

Update the file `code/libraries/unittest/include/unittest/TestRegistry.h`

```cpp
File: code/libraries/unittest/include/unittest/TestRegistry.h
...
44: namespace unittest
45: {
46:
47: class TestBase;
48: class TestSuiteInfo;
49: class TestResults;
50:
51: class TestRegistry
52: {
53: private:
54:     TestSuiteInfo* m_head;
55:     TestSuiteInfo* m_tail;
56:
57: public:
58:     static const char* DefaultFixtureName;
59:     static const char* DefaultSuiteName;
60:
61:     TestRegistry();
62:     TestRegistry(const TestRegistry&) = delete;
63:     TestRegistry(TestRegistry&&) = delete;
64:     virtual ~TestRegistry();
65:
66:     TestRegistry& operator = (const TestRegistry&) = delete;
67:     TestRegistry& operator = (TestRegistry&&) = delete;
68:
69:     TestSuiteInfo* GetTestSuite(const baremetal::string& suiteName);
70:     TestSuiteInfo* GetHead() const;
71:
72:     template <class Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
73:     template <typename Predicate> int CountSuitesIf(Predicate predicate);
74:     template <typename Predicate> int CountFixturesIf(Predicate predicate);
75:     template <typename Predicate> int CountTestsIf(Predicate predicate);
76:
77:     static TestRegistry& GetTestRegistry();
78:
79:     void AddSuite(TestSuiteInfo* testSuite);
80: };
81:
82: class TestRegistrar
83: {
84: public:
85:     TestRegistrar(TestRegistry& registry, TestBase* test);
86: };
87:
88: template <class Predicate> void TestRegistry::RunIf(const Predicate& predicate, TestResults& testResults)
89: {
90:     TestSuiteInfo* testSuite = GetHead();
91:
92:     while (testSuite != nullptr)
93:     {
94:         if (predicate(testSuite))
95:             testSuite->RunIf(predicate, testResults);
96:         testSuite = testSuite->m_next;
97:     }
98: }
99:
100: template <typename Predicate> int TestRegistry::CountSuitesIf(Predicate predicate)
101: {
102:     int numberOfTestSuites = 0;
103:     TestSuiteInfo* testSuite = GetHead();
104:     while (testSuite != nullptr)
105:     {
106:         if (predicate(testSuite))
107:             ++numberOfTestSuites;
108:         testSuite = testSuite->m_next;
109:     }
110:     return numberOfTestSuites;
111: }
112:
113: template <typename Predicate> int TestRegistry::CountFixturesIf(Predicate predicate)
114: {
115:     int numberOfTestFixtures = 0;
116:     TestSuiteInfo* testSuite = GetHead();
117:     while (testSuite != nullptr)
118:     {
119:         if (predicate(testSuite))
120:             numberOfTestFixtures += testSuite->CountFixturesIf(predicate);
121:         testSuite = testSuite->m_next;
122:     }
123:     return numberOfTestFixtures;
124: }
125:
126: template <typename Predicate> int TestRegistry::CountTestsIf(Predicate predicate)
127: {
128:     int numberOfTests = 0;
129:     TestSuiteInfo* testSuite = GetHead();
130:     while (testSuite != nullptr)
131:     {
132:         if (predicate(testSuite))
133:             numberOfTests += testSuite->CountTestsIf(predicate);
134:         testSuite = testSuite->m_next;
135:     }
136:     return numberOfTests;
137: }
138:
139: } // namespace unittest
```

- Line 49: We forward declare the `TestResults` class
- Line 72: We replace the `Run()` method with a template version `RunIf()`, which takes a predicate, as well as a `TestResults` reference
- Line 73: We replace then `CountSuites()` method with a template version `CountSuitesIf()`
- Line 74: We replace then `CountFixtures()` method with a template version `CountFixturesIf()`
- Line 75: We replace then `CountTests()` method with a template version `CountTestsIf()`
- Line 88-98: We implement the `RunIf()` template method.
Note that we use the `RunIf()` method on the test suite. We'll need to implement this
- Line 100-111: We implement the `CountSuitesIf()` template method
- Line 113-124: We implement the `CountFixturesIf()` template method.
Note that we use the `CountFixturesIf()` method on the test suite. We'll need to implement this
- Line 126-137: We implement the `CountTestsIf()` template method.
Note that we use the `CountTestsIf()` method on the test suite. We'll need to implement this

### TestRegistry.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTREGISTRYCPP}

We'll also need to update the `TestRegistry` implementation.

Update the file `code/libraries/unittest/src/TestRegistry.cpp`

```cpp
File: code/libraries/unittest/src/TestRegistry.cpp
...
40: #include <unittest/TestRegistry.h>
41:
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44:
45: using namespace baremetal;
46:
47: LOG_MODULE("TestRegistry");
48:
49: namespace unittest {
50:
51: const char* TestRegistry::DefaultFixtureName = "DefaultFixture";
52: const char* TestRegistry::DefaultSuiteName = "DefaultSuite";
53:
54: TestRegistry& TestRegistry::GetTestRegistry()
55: {
56:     static TestRegistry s_registry;
57:     return s_registry;
58: }
59:
60: TestRegistry::TestRegistry()
61:     : m_head{}
62:     , m_tail{}
63: {
64: }
65:
66: TestRegistry::~TestRegistry()
67: {
68:     TestSuiteInfo *testSuite = m_head;
69:     while (testSuite != nullptr)
70:     {
71:         TestSuiteInfo *currentSuite = testSuite;
72:         testSuite = testSuite->m_next;
73:         delete currentSuite;
74:     }
75: }
76:
77: TestSuiteInfo *TestRegistry::GetTestSuite(const string &suiteName)
78: {
79:     TestSuiteInfo *testSuite = m_head;
80:     while ((testSuite != nullptr) && (testSuite->Name() != suiteName))
81:         testSuite = testSuite->m_next;
82:     if (testSuite == nullptr)
83:     {
84: #ifdef DEBUG_REGISTRY
85:         LOG_DEBUG("Find suite %s ... not found, creating new object", (suiteName.empty() ? DefaultSuiteName : suiteName.c_str()));
86: #endif
87:         testSuite = new TestSuiteInfo(suiteName);
88:         AddSuite(testSuite);
89:     }
90:     else
91:     {
92: #ifdef DEBUG_REGISTRY
93:         LOG_DEBUG("Find suite %s ... found", (suiteName.empty() ? DefaultSuiteName : suiteName.c_str()));
94: #endif
95:     }
96:     return testSuite;
97: }
98:
99: void TestRegistry::AddSuite(TestSuiteInfo *testSuite)
100: {
101:     if (m_tail == nullptr)
102:     {
103:         assert(m_head == nullptr);
104:         m_head = testSuite;
105:         m_tail = testSuite;
106:     }
107:     else
108:     {
109:         m_tail->m_next = testSuite;
110:         m_tail = testSuite;
111:     }
112: }
113:
114: TestSuiteInfo *TestRegistry::GetHead() const
115: {
116:     return m_head;
117: }
118:
119: TestRegistrar::TestRegistrar(TestRegistry &registry, TestBase *test)
120: {
121: #ifdef DEBUG_REGISTRY
122:     LOG_DEBUG("Register test %s in fixture %s in suite %s",
123:         test->Details().TestName().c_str(),
124:         (test->Details().FixtureName().empty() ? TestRegistry::DefaultFixtureName : test->Details().FixtureName().c_str()),
125:         (test->Details().SuiteName().empty() ? TestRegistry::DefaultSuiteName : test->Details().SuiteName().c_str()));
126: #endif
127:     TestSuiteInfo   *testSuite   = registry.GetTestSuite(test->Details().SuiteName());
128:     TestFixtureInfo *testFixture = testSuite->GetTestFixture(test->Details().FixtureName());
129:     testFixture->AddTest(test);
130: }
131:
132: } // namespace unittest
```

- Line 116: We remove the implementation for the methods `Run()`, `CountSuites()`, `CountFixtures()` and `CountTests()`.

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTSUITEINFOH}

We also need to update the `TestSuiteInfo` class.

Update the file `code/libraries/unittest/include/unittest/TestSuiteInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuiteInfo.h
...
42: #include <unittest/TestFixtureInfo.h>
43: #include <unittest/TestResults.h>
44:
45: namespace unittest
46: {
47:
48: class TestBase;
49: class TestFixtureInfo;
50:
51: class TestSuiteInfo
52: {
53: private:
54:     friend class TestRegistry;
55:     TestFixtureInfo* m_head;
56:     TestFixtureInfo* m_tail;
57:     TestSuiteInfo* m_next;
58:     baremetal::string m_suiteName;
59:
60: public:
61:     TestSuiteInfo() = delete;
62:     TestSuiteInfo(const TestSuiteInfo&) = delete;
63:     TestSuiteInfo(TestSuiteInfo&&) = delete;
64:     explicit TestSuiteInfo(const baremetal::string& suiteName);
65:     virtual ~TestSuiteInfo();
66:
67:     TestSuiteInfo& operator = (const TestSuiteInfo&) = delete;
68:     TestSuiteInfo& operator = (TestSuiteInfo&&) = delete;
69:
70:     TestFixtureInfo* GetHead() const;
71:
72:     const baremetal::string& Name() const { return m_suiteName; }
73:
74:     template <class Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
75:
76:     int CountFixtures();
77:     int CountTests();
78:     template <typename Predicate> int CountFixturesIf(Predicate predicate);
79:     template <typename Predicate> int CountTestsIf(Predicate predicate);
80:
81:     void AddFixture(TestFixtureInfo* testFixture);
82:     TestFixtureInfo* GetTestFixture(const baremetal::string& fixtureName);
83: };
84:
85: template <class Predicate> void TestSuiteInfo::RunIf(const Predicate& predicate, TestResults& testResults)
86: {
87:     testResults.OnTestSuiteStart(this);
88:
89:     TestFixtureInfo* testFixture = GetHead();
90:     while (testFixture != nullptr)
91:     {
92:         if (predicate(testFixture))
93:             testFixture->RunIf(predicate, testResults);
94:         testFixture = testFixture->m_next;
95:     }
96:
97:     testResults.OnTestSuiteFinish(this);
98: }
99:
100: template <typename Predicate> int TestSuiteInfo::CountFixturesIf(Predicate predicate)
101: {
102:     int numberOfTestFixtures = 0;
103:     TestFixtureInfo* testFixture = GetHead();
104:     while (testFixture != nullptr)
105:     {
106:         if (predicate(testFixture))
107:             numberOfTestFixtures++;
108:         testFixture = testFixture->m_next;
109:     }
110:     return numberOfTestFixtures;
111: }
112:
113: template <typename Predicate> int TestSuiteInfo::CountTestsIf(Predicate predicate)
114: {
115:     int numberOfTests = 0;
116:     TestFixtureInfo* testFixture = GetHead();
117:     while (testFixture != nullptr)
118:     {
119:         if (predicate(testFixture))
120:             numberOfTests += testFixture->CountTestsIf(predicate);
121:         testFixture = testFixture->m_next;
122:     }
123:     return numberOfTests;
124: }
125:
126: } // namespace unittest
```

- Line 43: We include the header for the `TestResults` class as we will need to use it
- Line 74: We replace the `Run()` method with a template `RunIf()` method, taking both a predicate and the `TestResults` to fill
- Line 78: We add a template method `CountFixturesIf()`
- Line 79: We add a template method `CountTestsIf()`
- Line 85-98: We implement the `RunIf()` template method.
Note that we use the method `RunIf()` in the `TestFixtureInfo` class. We'll need to implement it
- Line 100-111: We implement the `CountFixturesIf()` template method
- Line 113-124: We implement the `CountTestsIf()` template method.
Note that we use the method `CountTestsIf()` in the `TestFixtureInfo` class. We'll need to implement it

### TestSuiteInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTSUITEINFOCPP}

We'll also need to update the `TestSuiteInfo` implementation.

Update the file `code/libraries/unittest/src/TestSuiteInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestSuiteInfo.cpp
...
40: #include <unittest/TestSuiteInfo.h>
41:
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44: #include <unittest/TestRegistry.h>
45:
46: using namespace baremetal;
47:
48: namespace unittest {
49:
50: LOG_MODULE("TestSuiteInfo");
51:
52: TestSuiteInfo::TestSuiteInfo(const string &suiteName)
53:     : m_head{}
54:     , m_tail{}
55:     , m_next{}
56:     , m_suiteName{suiteName}
57: {
58: }
59:
60: TestSuiteInfo::~TestSuiteInfo()
61: {
62:     TestFixtureInfo *testFixture = m_head;
63:     while (testFixture != nullptr)
64:     {
65:         TestFixtureInfo *currentFixture = testFixture;
66:         testFixture                     = testFixture->m_next;
67:         delete currentFixture;
68:     }
69: }
70:
71: TestFixtureInfo *TestSuiteInfo::GetTestFixture(const string &fixtureName)
72: {
73:     TestFixtureInfo *testFixture = m_head;
74:     while ((testFixture != nullptr) && (testFixture->Name() != fixtureName))
75:         testFixture = testFixture->m_next;
76:     if (testFixture == nullptr)
77:     {
78: #ifdef DEBUG_REGISTRY
79:         LOG_DEBUG("Fixture %s not found, creating new object", fixtureName.empty() ? TestRegistry::DefaultFixtureName : fixtureName.c_str());
80: #endif
81:         testFixture = new TestFixtureInfo(fixtureName);
82:         AddFixture(testFixture);
83:     }
84:     else
85:     {
86: #ifdef DEBUG_REGISTRY
87:         LOG_DEBUG("Fixture %s found", fixtureName.empty() ? TestRegistry::DefaultFixtureName : fixtureName.c_str());
88: #endif
89:     }
90:     return testFixture;
91: }
92:
93: void TestSuiteInfo::AddFixture(TestFixtureInfo *testFixture)
94: {
95:     if (m_tail == nullptr)
96:     {
97:         assert(m_head == nullptr);
98:         m_head = testFixture;
99:         m_tail = testFixture;
100:     }
101:     else
102:     {
103:         m_tail->m_next = testFixture;
104:         m_tail         = testFixture;
105:     }
106: }
107:
108: TestFixtureInfo *TestSuiteInfo::GetHead() const
109: {
110:     return m_head;
111: }
112:
113: int TestSuiteInfo::CountFixtures()
114: {
115:     int              numberOfTestFixtures = 0;
116:     TestFixtureInfo *testFixture          = m_head;
117:     while (testFixture != nullptr)
118:     {
119:         ++numberOfTestFixtures;
120:         testFixture = testFixture->m_next;
121:     }
122:     return numberOfTestFixtures;
123: }
124:
125: int TestSuiteInfo::CountTests()
126: {
127:     int              numberOfTests = 0;
128:     TestFixtureInfo *testFixture   = m_head;
129:     while (testFixture != nullptr)
130:     {
131:         numberOfTests += testFixture->CountTests();
132:         testFixture = testFixture->m_next;
133:     }
134:     return numberOfTests;
135: }
136:
137: } // namespace unittest
```

- Line 44: We include the header for `TestRegistry` so we can use the default test fixture and test suite names
- Line 79: We using the default test fixture name instead of "-"
- Line 87: We using the default test fixture name instead of "-"
- Line 112: We remove the implementation for the method `Run()`

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTFIXTUREINFOH}

We also need to update the `TestFixtureInfo` class.

Update the file `code/libraries/unittest/include/unittest/TestFixtureInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixtureInfo.h
...
42: #include <unittest/TestBase.h>
43: #include <unittest/TestResults.h>
44:
45: namespace unittest
46: {
47:
48: class TestBase;
49:
50: class TestFixtureInfo
51: {
52: private:
53:     friend class TestSuiteInfo;
54:     TestBase* m_head;
55:     TestBase* m_tail;
56:     TestFixtureInfo* m_next;
57:     baremetal::string m_fixtureName;
58:
59: public:
60:     TestFixtureInfo() = delete;
61:     TestFixtureInfo(const TestFixtureInfo&) = delete;
62:     TestFixtureInfo(TestFixtureInfo&&) = delete;
63:     explicit TestFixtureInfo(const baremetal::string& fixtureName);
64:     virtual ~TestFixtureInfo();
65:
66:     TestFixtureInfo & operator = (const TestFixtureInfo &) = delete;
67:     TestFixtureInfo& operator = (TestFixtureInfo&&) = delete;
68:
69:     TestBase* GetHead() const;
70:
71:     const baremetal::string& Name() const { return m_fixtureName; }
72:
73:     template <class Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
74:
75:     int CountTests();
76:     template <typename Predicate> int CountTestsIf(Predicate predicate);
77:
78:     void AddTest(TestBase* test);
79: };
80:
81: template <class Predicate> void TestFixtureInfo::RunIf(const Predicate& predicate, TestResults& testResults)
82: {
83:     testResults.OnTestFixtureStart(this);
84:
85:     TestBase* test = this->GetHead();
86:     while (test != nullptr)
87:     {
88:         if (predicate(test))
89:             test->Run(testResults);
90:         test = test->m_next;
91:     }
92:
93:     testResults.OnTestFixtureFinish(this);
94: }
95:
96: template <typename Predicate> int TestFixtureInfo::CountTestsIf(Predicate predicate)
97: {
98:     int numberOfTests = 0;
99:     TestBase* test = this->GetHead();
100:     while (test != nullptr)
101:     {
102:         if (predicate(test))
103:             numberOfTests++;
104:         test = test->m_next;
105:     }
106:     return numberOfTests;
107: }
108:
109: } // namespace unittest
```

- Line 43: We include the header for `TestResults`
- Line 73: We replace the `Run()` method with a template `RunIf()` method, taking both a predicate and the `TestResults` to fill
- Line 76: We add a template method `CountTestsIf()`
- Line 81-94: We implement the `RunIf()` template method.
Note that we call the method `Run()` on `TestBase` with a reference to the `TestResults` instance. This method needs to be added
- Line 96-107: We implement the `CountTestsIf()` template method

### TestFixtureInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTFIXTUREINFOCPP}

We'll also need to update the `TestFixtureInfo` implementation.

Update the file `code/libraries/unittest/src/TestFixtureInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestFixtureInfo.cpp
...
45: namespace unittest {
46:
47: TestFixtureInfo::TestFixtureInfo(const string& fixtureName)
48:     : m_head{}
49:     , m_tail{}
50:     , m_next{}
51:     , m_fixtureName{ fixtureName }
52: {
53: }
54:
55: TestFixtureInfo::~TestFixtureInfo()
56: {
57:     TestBase* test = m_head;
58:     while (test != nullptr)
59:     {
60:         TestBase* currentTest = test;
61:         test = test->m_next;
62:         delete currentTest;
63:     }
64: }
65:
66: void TestFixtureInfo::AddTest(TestBase* test)
67: {
68:     if (m_tail == nullptr)
69:     {
70:         assert(m_head == nullptr);
71:         m_head = test;
72:         m_tail = test;
73:     }
74:     else
75:     {
76:         m_tail->m_next = test;
77:         m_tail = test;
78:     }
79: }
80:
81: TestBase* TestFixtureInfo::GetHead() const
82: {
83:     return m_head;
84: }
85:
86: int TestFixtureInfo::CountTests()
87: {
88:     int numberOfTests = 0;
89:     TestBase* test = m_head;
90:     while (test != nullptr)
91:     {
92:         ++numberOfTests;
93:         test = test->m_next;
94:     }
95:     return numberOfTests;
96: }
97:
98: } // namespace unittest
```

- Line 86: We remove the implementation for the method `Run()`

### TestBase.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTBASEH}

We also need to update the `TestBase` class.

Update the file `code/libraries/unittest/include/unittest/TestBase.h`

```cpp
File: code/libraries/unittest/include/unittest/TestBase.h
...
44: namespace unittest
45: {
46:
47: class TestResults;
48:
49: class TestBase
50: {
51: private:
52:     friend class TestFixtureInfo;
53:     TestDetails const m_details;
54:     TestBase* m_next;
55:
56: public:
57:     TestBase();
58:     TestBase(const TestBase&) = delete;
59:     TestBase(TestBase&&) = delete;
60:     explicit TestBase(
61:         const baremetal::string& testName,
62:         const baremetal::string& fixtureName = {},
63:         const baremetal::string& suiteName = {},
64:         const baremetal::string& fileName = {},
65:         int lineNumber = {});
66:     virtual ~TestBase();
67:
68:     TestBase& operator = (const TestBase&) = delete;
69:     TestBase& operator = (TestBase&&) = delete;
70:
71:     const TestDetails& Details() const { return m_details; }
72:
73:     void Run(TestResults& testResults);
74:     void Run();
75:
76:     virtual void RunImpl() const;
77: };
78:
79: } // namespace unittest
```

- Line 47: We forward declare the `TestResults` class
- Line 73: We add the `Run()` method taking a `TestResults` to fill when running the test

### TestBase.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_TESTBASECPP}

We'll also need to update the `TestBase` implementation.

Update the file `code/libraries/unittest/src/TestBase.cpp`

```cpp
File: code/libraries/unittest/src/TestBase.cpp
...
40: #include <unittest/TestBase.h>
41:
42: #include <unittest/CurrentTest.h>
43: #include <unittest/ExecuteTest.h>
44: #include <unittest/TestResults.h>
45:
46: using namespace baremetal;
47:
48: namespace unittest {
49:
50: TestBase::TestBase()
51:     : m_details{}
52:     , m_next{}
53: {
54: }
55:
56: TestBase::TestBase(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
57:     : m_details{ testName, fixtureName, suiteName, fileName, lineNumber }
58:     , m_next{}
59: {
60: }
61:
62: TestBase::~TestBase()
63: {
64: }
65:
66: void TestBase::Run(TestResults& testResults)
67: {
68:     CurrentTest::Results() = &testResults;
69:
70:     testResults.OnTestStart(m_details);
71:
72:     Run();
73:
74:     testResults.OnTestFinish(m_details);
75: }
76:
77: void TestBase::Run()
78: {
79:     ExecuteTest(*this, m_details);
80: }
81:
82: void TestBase::RunImpl() const
83: {
84: }
85:
86: } // namespace unittest
```

- Line 42: We include the header for `CurrentTest`, a set of utility functions which we'll need. We'll declare and define it next
- Line 43: We include the header for `ExecuteTest`, a utility function which we'll need. We'll declare and define it next
- Line 66-75: We implement the variant of the `Run()` method taking a `TestResults` instance.
This will store a pointer the current `TestResults` instance using the utility function `CurrentTest::Results()`.
This will be needed when we wish to actually perofrm test statements.
- Line 79: We now use the `ExecuteTest` utility function to run the actual test. We'll get to that in a minute

### CurrentTest.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_CURRENTTESTH}

We will now declare the utility functions for `CurrentTest`. `CurrentTest` is actually a namespace inside the `unittest` namespace.

Create the file `code/libraries/unittest/include/unittest/CurrentTest.h`

```cpp
File: code/libraries/unittest/include/unittest/CurrentTest.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : CurrentTest.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : CurrentTest
9: //
10: // Description : Current test info
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: namespace unittest
43: {
44:
45: class TestResults;
46: class TestDetails;
47:
48: namespace CurrentTest
49: {
50:     TestResults*& Results();
51:     const TestDetails*& Details();
52: }
53:
54: } // namespace unittest
```

- Line 50: We declare function name `Results() which used to store the current `TestResults` instance
- Line 51: We declare function name `Details() which used to store the current `TestDetails` instance

### CurrentTest.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_CURRENTTESTCPP}

We'll implement the functions for for `CurrentTest`.

Create the file `code/libraries/unittest/src/CurrentTest.cpp`

```cpp
File: code/libraries/unittest/src/CurrentTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : CurrentTest.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : CurrentTest
9: //
10: // Description : Current test info
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
38: //------------------------------------------------------------------------------
39:
40: #include <unittest/CurrentTest.h>
41:
42: namespace unittest
43: {
44:
45: TestResults *& CurrentTest::Results()
46: {
47:     static TestResults* testResults = nullptr;
48:     return testResults;
49: }
50:
51: const TestDetails *& CurrentTest::Details()
52: {
53:     static const TestDetails* testDetails = nullptr;
54:     return testDetails;
55: }
56:
57: } // namespace unittest
```

- Line 45-49: We implement the `Results()` method. It keeps a static pointer to a `TestResults` instance, and returns a reference to that pointer
- Line 51-55: We implement the `Details()` method. It keeps a static pointer to a `TestDetails` instance, and returns a const reference to that pointer

### ExecuteTest.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_EXECUTETESTH}

We will now declare the utility function `ExecuteTest`.

Create the file `code/libraries/unittest/include/unittest/ExecuteTest.h`

```cpp
File: code/libraries/unittest/include/unittest/ExecuteTest.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ExecuteTest.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : ExecuteTest
9: //
10: // Description : Test executor
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: #include <unittest/CurrentTest.h>
43: #include <unittest/TestDetails.h>
44:
45: namespace unittest
46: {
47:
48: template<typename T>
49: void ExecuteTest(T& testObject, const TestDetails& details)
50: {
51:     CurrentTest::Details() = &details;
52:
53:     testObject.RunImpl();
54: }
55:
56: } // namespace unittest
```

Line 48-54: We define the template function `ExecuteTest`.
This function is a template function to allow both `TestBase` and `TestFixture` instances to be run.
The function stores the current test details using `CurrentTest::Details(), and then call `RunImpl()` on the test object

### ConsoleTestReporter.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_CONSOLETESTREPORTERH}

We still need to instance the abstract interface `ITestReporter` to actuall report information.
We'll create a console test reporter class `ConsoleTestReporter` for this purpose, which leans on the `baremetal::Console` class.

Create the file `code/libraries/unittest/include/unittest/ConsoleTestReporter.h`

```cpp
File: code/libraries/unittest/include/unittest/ConsoleTestReporter.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ConsoleTestReporter.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : ConsoleTestReporter
9: //
10: // Description : Console test reporter
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: #include <unittest/ITestReporter.h>
43:
44: namespace unittest
45: {
46:
47: class ConsoleTestReporter : public ITestReporter
48: {
49: public:
50:     static const baremetal::string TestRunSeparator;
51:     static const baremetal::string TestFixtureSeparator;
52:     static const baremetal::string TestSuiteSeparator;
53:     static const baremetal::string TestSuccessSeparator;
54:     static const baremetal::string TestFailSeparator;
55:
56:     ConsoleTestReporter();
57:
58: private:
59:     void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
60:     void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
61:     void ReportTestRunSummary(const TestResults& results) override;
62:     void ReportTestRunOverview(const TestResults& results) override;
63:     void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTestFixtures) override;
64:     void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTestFixtures) override;
65:     void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) override;
66:     void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests) override;
67:     void ReportTestStart(const TestDetails& details) override;
68:     void ReportTestFinish(const TestDetails& details, bool success) override;
69:     void ReportTestFailure(const TestDetails& details, const baremetal::string& failure) override;
70:
71:     baremetal::string TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests);
72:     baremetal::string TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests);
73:     baremetal::string TestSuiteStartMessage(const baremetal::string& suiteName, int numberOfTestFixtures);
74:     baremetal::string TestSuiteFinishMessage(const baremetal::string& suiteName, int numberOfTestFixtures);
75:     baremetal::string TestFixtureStartMessage(const baremetal::string& fixtureName, int numberOfTests);
76:     baremetal::string TestFixtureFinishMessage(const baremetal::string& fixtureName, int numberOfTests);
77:     baremetal::string TestFailureMessage(const TestDetails& details, const baremetal::string& failure);
78:     baremetal::string TestFinishMessage(const TestDetails& test, bool success);
79:     baremetal::string TestRunSummaryMessage(const TestResults& results);
80:     baremetal::string TestRunOverviewMessage(const TestResults& results);
81:     baremetal::string TestName(const baremetal::string& suiteName, const baremetal::string& fixtureName, const baremetal::string& testName);
82:
83: };
84:
85: } // namespace unittest
```

- Line 50-54: We declare some statis constant strings to use as separators in our output
- Line 56: We declare a default constructor
- Line 59-69: We declare the overrides for the `ITestReporter` abstract interface
- Line 71:81: We declare some utility functions to build strings for output

### ConsoleTestReporter.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_CONSOLETESTREPORTERCPP}

We'll implement the class `ConsoleTestReporter`.

Create the file `code/libraries/unittest/src/ConsoleTestReporter.cpp`

```cpp
File: code/libraries/unittest/src/ConsoleTestReporter.cpp
File: d:\Projects\baremetal.github\tutorial\17-unit-tests\code\libraries\unittest\src\ConsoleTestReporter.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : ConsoleTestReporter.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : ConsoleTestReporter
9: //
10: // Description : Test detail
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
38: //------------------------------------------------------------------------------
39:
40: #include <unittest/ConsoleTestReporter.h>
41:
42: #include <baremetal/Console.h>
43: #include <baremetal/Format.h>
44: #include <baremetal/Serialization.h>
45: #include <unittest/TestDetails.h>
46: #include <unittest/TestRegistry.h>
47: #include <unittest/TestResults.h>
48:
49: using namespace baremetal;
50:
51: namespace unittest
52: {
53:
54: const string ConsoleTestReporter::TestRunSeparator = baremetal::string("[===========]");
55: const string ConsoleTestReporter::TestSuiteSeparator = baremetal::string("[   SUITE   ]");
56: const string ConsoleTestReporter::TestFixtureSeparator = baremetal::string("[  FIXTURE  ]");
57: const string ConsoleTestReporter::TestSuccessSeparator = baremetal::string("[ SUCCEEDED ]");
58: const string ConsoleTestReporter::TestFailSeparator = baremetal::string("[  FAILED   ]");
59:
60: ConsoleTestReporter::ConsoleTestReporter()
61: {
62: }
63:
64: void ConsoleTestReporter::ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
65: {
66:     GetConsole().SetTerminalColor(ConsoleColor::Green);
67:     GetConsole().Write(TestRunSeparator);
68:     GetConsole().ResetTerminalColor();
69:
70:     GetConsole().Write(Format(" %s\n", TestRunStartMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
71: }
72:
73: void ConsoleTestReporter::ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
74: {
75:     GetConsole().SetTerminalColor(ConsoleColor::Green);
76:     GetConsole().Write(TestRunSeparator);
77:     GetConsole().ResetTerminalColor();
78:
79:     GetConsole().Write(Format(" %s\n", TestRunFinishMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
80: }
81:
82: void ConsoleTestReporter::ReportTestRunSummary(const TestResults& results)
83: {
84:     if (results.GetFailureCount() > 0)
85:         GetConsole().SetTerminalColor(ConsoleColor::Red);
86:     else
87:         GetConsole().SetTerminalColor(ConsoleColor::Green);
88:     GetConsole().Write(Format("%s\n", TestRunSummaryMessage(results).c_str()));
89:     GetConsole().ResetTerminalColor();
90: }
91:
92: void ConsoleTestReporter::ReportTestRunOverview(const TestResults& results)
93: {
94:     GetConsole().Write(Format("%s\n", TestRunOverviewMessage(results).c_str()));
95: }
96:
97: void ConsoleTestReporter::ReportTestSuiteStart(const string& suiteName, int numberOfTestFixtures)
98: {
99:     GetConsole().SetTerminalColor(ConsoleColor::Cyan);
100:     GetConsole().Write(TestSuiteSeparator);
101:     GetConsole().ResetTerminalColor();
102:
103:     GetConsole().Write(Format(" %s\n", TestSuiteStartMessage(suiteName, numberOfTestFixtures).c_str()));
104: }
105:
106: void ConsoleTestReporter::ReportTestSuiteFinish(const string& suiteName, int numberOfTestFixtures)
107: {
108:     GetConsole().SetTerminalColor(ConsoleColor::Cyan);
109:     GetConsole().Write(TestSuiteSeparator);
110:     GetConsole().ResetTerminalColor();
111:
112:     GetConsole().Write(Format(" %s\n", TestSuiteFinishMessage(suiteName, numberOfTestFixtures).c_str()));
113: }
114:
115: void ConsoleTestReporter::ReportTestFixtureStart(const string& fixtureName, int numberOfTests)
116: {
117:     GetConsole().SetTerminalColor(ConsoleColor::Yellow);
118:     GetConsole().Write(TestFixtureSeparator);
119:     GetConsole().ResetTerminalColor();
120:
121:     GetConsole().Write(Format(" %s\n", TestFixtureStartMessage(fixtureName, numberOfTests).c_str()));
122: }
123:
124: void ConsoleTestReporter::ReportTestFixtureFinish(const string& fixtureName, int numberOfTests)
125: {
126:     GetConsole().SetTerminalColor(ConsoleColor::Yellow);
127:     GetConsole().Write(TestFixtureSeparator);
128:     GetConsole().ResetTerminalColor();
129:
130:     GetConsole().Write(Format(" %s\n", TestFixtureFinishMessage(fixtureName, numberOfTests).c_str()));
131: }
132:
133: void ConsoleTestReporter::ReportTestStart(const TestDetails& /*details*/)
134: {
135: }
136:
137: void ConsoleTestReporter::ReportTestFinish(const TestDetails& details, bool success)
138: {
139:     GetConsole().SetTerminalColor(success ? ConsoleColor::Green : ConsoleColor::Red);
140:     if (success)
141:         GetConsole().Write(TestSuccessSeparator);
142:     else
143:         GetConsole().Write(TestFailSeparator);
144:     GetConsole().ResetTerminalColor();
145:
146:     GetConsole().Write(Format(" %s\n", TestFinishMessage(details, success).c_str()));
147: }
148:
149: void ConsoleTestReporter::ReportTestFailure(const TestDetails& details, const string& failure)
150: {
151:     GetConsole().SetTerminalColor(ConsoleColor::Red);
152:     GetConsole().Write(Format("%s\n", TestFailureMessage(details, failure).c_str()));
153:     GetConsole().ResetTerminalColor();
154: }
155:
156: static string TestLiteral(int numberOfTests)
157: {
158:     return baremetal::string((numberOfTests == 1) ? "test" : "tests");
159: }
160:
161: static string TestFailureLiteral(int numberOfTestFailures)
162: {
163:     return baremetal::string((numberOfTestFailures == 1) ? "failure" : "failures");
164: }
165:
166: static string TestFixtureLiteral(int numberOfTestFixtures)
167: {
168:     return baremetal::string((numberOfTestFixtures == 1) ? "fixture" : "fixtures");
169: }
170:
171: static string TestSuiteLiteral(int numberOfTestSuites)
172: {
173:     return baremetal::string((numberOfTestSuites == 1) ? "suite" : "suites");
174: }
175:
176: static string TestSuiteName(string name)
177: {
178:     return baremetal::string((!name.empty()) ? name : baremetal::string(TestRegistry::DefaultSuiteName));
179: }
180:
181: static string TestFixtureName(string name)
182: {
183:     return baremetal::string((!name.empty()) ? name : baremetal::string(TestRegistry::DefaultFixtureName));
184: }
185:
186: string ConsoleTestReporter::TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
187: {
188:     return Format("Running %s %s from %s %s in %s %s.",
189:         Serialize(numberOfTests).c_str(),
190:         TestLiteral(numberOfTests).c_str(),
191:         Serialize(numberOfTestFixtures).c_str(),
192:         TestFixtureLiteral(numberOfTestFixtures).c_str(),
193:         Serialize(numberOfTestSuites).c_str(),
194:         TestSuiteLiteral(numberOfTestSuites).c_str());
195: }
196:
197: string ConsoleTestReporter::TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
198: {
199:     return Format("%s %s from %s %s in %s %s ran.",
200:         Serialize(numberOfTests).c_str(),
201:         TestLiteral(numberOfTests).c_str(),
202:         Serialize(numberOfTestFixtures).c_str(),
203:         TestFixtureLiteral(numberOfTestFixtures).c_str(),
204:         Serialize(numberOfTestSuites).c_str(),
205:         TestSuiteLiteral(numberOfTestSuites).c_str());
206: }
207:
208: string ConsoleTestReporter::TestRunSummaryMessage(const TestResults& results)
209: {
210:     if (results.GetFailureCount() > 0)
211:     {
212:         return Format("FAILURE: %s out of %s %s failed (%s %s).\n",
213:             Serialize(results.GetFailedTestCount()).c_str(),
214:             Serialize(results.GetTotalTestCount()).c_str(),
215:             TestLiteral(results.GetTotalTestCount()).c_str(),
216:             Serialize(results.GetFailureCount()).c_str(),
217:             TestFailureLiteral(results.GetFailureCount()).c_str());
218:     }
219:     return Format("Success: %s %s passed.\n",
220:             Serialize(results.GetTotalTestCount()).c_str(),
221:             TestLiteral(results.GetTotalTestCount()).c_str());
222: }
223:
224: string ConsoleTestReporter::TestRunOverviewMessage(const TestResults& results)
225: {
226:     if (results.GetFailureCount() > 0)
227:     {
228:         return Format("Failures: %d", results.GetFailureCount());
229:     }
230:     return "No failures";
231: }
232:
233: string ConsoleTestReporter::TestSuiteStartMessage(const string& suiteName, int numberOfTestFixtures)
234: {
235:     return Format("%s (%s %s)",
236:         TestSuiteName(suiteName).c_str(),
237:         Serialize(numberOfTestFixtures).c_str(),
238:         TestFixtureLiteral(numberOfTestFixtures).c_str());
239: }
240:
241: string ConsoleTestReporter::TestSuiteFinishMessage(const string& suiteName, int numberOfTestFixtures)
242: {
243:     return Format("%s %s from %s",
244:         Serialize(numberOfTestFixtures).c_str(),
245:         TestFixtureLiteral(numberOfTestFixtures).c_str(),
246:         TestSuiteName(suiteName).c_str());
247: }
248:
249: string ConsoleTestReporter::TestFixtureStartMessage(const string& fixtureName, int numberOfTests)
250: {
251:     return Format("%s (%s %s)",
252:         TestFixtureName(fixtureName).c_str(),
253:         Serialize(numberOfTests).c_str(),
254:         TestLiteral(numberOfTests).c_str());
255: }
256:
257: string ConsoleTestReporter::TestFixtureFinishMessage(const string& fixtureName, int numberOfTests)
258: {
259:     return Format("%s %s from %s",
260:         Serialize(numberOfTests).c_str(),
261:         TestLiteral(numberOfTests).c_str(),
262:         TestFixtureName(fixtureName).c_str());
263: }
264:
265: string ConsoleTestReporter::TestFinishMessage(const TestDetails& details, bool /*success*/)
266: {
267:     return TestName(details.SuiteName(), details.FixtureName(), details.TestName());
268: }
269:
270: string ConsoleTestReporter::TestFailureMessage(const TestDetails& details, const string& failure)
271: {
272:     return Format("%s failure %s", TestName(details.SuiteName(), details.FixtureName(), details.TestName()).c_str(), failure.c_str());
273: }
274:
275: string ConsoleTestReporter::TestName(const string& suiteName, const string& fixtureName, const string& testName)
276: {
277:     string result;
278:     if (!suiteName.empty())
279:     {
280:         result.append(suiteName);
281:         result.append("::");
282:     }
283:     if (!fixtureName.empty())
284:     {
285:         result.append(fixtureName);
286:         result.append("::");
287:     }
288:     result.append(testName);
289:     return result;
290: }
291:
292: } // namespace unittest
```

- Line 54-58: We initialize the static member variables
- Line 60-62: We implement the constructor
- Line 64-71: We implement the method `ReportTestRunStart()`.
This prints the run separator in green, and uses the method `TestRunStartMessage()` to format the string to be printed
- Line 73-80: We implement the method `ReportTestRunFinish()`.
This prints the run separator in green, and uses the method `TestRunFinishMessage()` to format the string to be printed
- Line 82-90: We implement the method `ReportTestRunSummary()`.
This uses the method `TestRunSummaryMessage()` to format the string to be printed, in green if successful, in red if failures occured
- Line 92-95: We implement the method `ReportTestRunOverview()`.
This uses the method `TestRunOverviewMessage()` to format the string to be printed
- Line 97-104: We implement the method `ReportTestSuiteStart()`.
This prints the test suite separator in cyan, and uses the method `TestSuiteStartMessage()` to format the string to be printed
- Line 106-113: We implement the method `ReportTestSuiteFinish()`.
This prints the test suite separator in cyan, and uses the method `TestSuiteFinishMessage()` to format the string to be printed
- Line 115-122: We implement the method `ReportTestFixtureStart()`.
This prints the test fixture separator in yellow, and uses the method `TestFixtureStartMessage()` to format the string to be printed
- Line 124-131: We implement the method `ReportTestFixtureFinish()`.
This prints the test fixture separator in yellow, and uses the method `TestFixtureFinishMessage()` to format the string to be printed
- Line 133-135: We implement the method `ReportTestStart()`.
This prints nothing
- Line 137-147: We implement the method `ReportTestFinish()`.
This prints the success separator in green if successful, or the failure separator in red if failures occurred, and uses the method `TestFinishMessage()` to format the string to be printed
- Line 149-154: We implement the method `ReportTestFailure()`.
This uses the method `TestFailureMessage()` to format the string to be printed in red
- Line 156-159: We implement a static function `TestLiteral()`, which returns the string "test" if the number of tests equals 1, and "tests" otherwise
- Line 161-164: We implement a static function `TestFailureLiteral()`, which returns the string "failure" if the number of failures equals 1, and "failures" otherwise
- Line 166-169: We implement a static function `TestFixtureLiteral()`, which returns the string "fixture" if the number of test fixtures equals 1, and "fixtures" otherwise
- Line 171-174: We implement a static function `TestSuiteLiteral()`, which returns the string "suite" if the number of test suites equals 1, and "suites" otherwise
- Line 176-179: We implement a static function `TestSuiteName()`, which returns the name of the test suite if not empty, and "DefaultSuite" otherwise
- Line 181-184: We implement a static function `TestFixtureName()`, which returns the name of the test fixture if not empty, and "DefaultFixture" otherwise
- Line 186-195: We implement a static function `TestRunStartMessage()`, which returns a string in the form "Running 1 test from 2 fixtures in 3 suites"
- Line 197-206: We implement a static function `TestRunFinishMessage()`, which returns a string in the form "1 test from 2 fixtures in 3 suites ran."
- Line 208-222: We implement a static function `TestRunSummaryMessage()`, which returns a string in the form "FAILURE: 1 out of 2 tests failed (3 failures).\n" in case of failures,
and "Success: 2 tests passed.\n" otherwise
- Line 224-231: We implement a static function `TestRunOverviewMessage()`, which returns a string in the form "Failures: 2" in case of failures, and "No failures" otherwise
- Line 233-239: We implement a static function `TestSuiteStartMessage()`, which returns a string in the form "2 fixtures from Suite1"
- Line 241-247: We implement a static function `TestSuiteFinishMessage()`, which returns a string in the form "Suite (2 fixtures)"
- Line 249-255: We implement a static function `TestFixtureStartMessage()`, which returns a string in the form "2 test from Fixture1"
- Line 257-263: We implement a static function `TestFixtureFinishMessage()`, which returns a string in the form "Fixture1 (2 tests)"
- Line 265-268: We implement a static function `TestFinishMessage()`, which returns a string containing the fully qualified test name
- Line 270-273: We implement a static function `TestFailureMessage()`, which returns a string in the form "Suite1\:\:Fixture2\:\:Test3 failure failureText"
- Line 275-290: We implement a static function `TestName()`, which returns the fully qualified test name "Suite1\:\:Fixture2\:\:Test3" if the test suite name is not empty,
"Fixture2\:\:Test3" otherwise if the fixture name is not empty, "Test3" otherwise

### unittest.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_UNITTESTH}

As we now need to include quite a few header files to set up and run our tests, let create a single include header that includes everything needed.

Create the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : unittest.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : unittest general include file, with functions to start tests
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: #include <unittest/TestFixture.h>
43: #include <unittest/TestSuite.h>
44:
45: #include <unittest/ITestReporter.h>
46: #include <unittest/ConsoleTestReporter.h>
47: #include <unittest/ExecuteTest.h>
48: #include <unittest/TestBase.h>
49: #include <unittest/TestDetails.h>
50: #include <unittest/TestFixtureInfo.h>
51: #include <unittest/TestRegistry.h>
52: #include <unittest/TestResults.h>
53: #include <unittest/TestRunner.h>
54: #include <unittest/TestSuiteInfo.h>
```

### Update project configuration {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleTestReporter.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRunner.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
40:     )
41:
42: set(PROJECT_INCLUDES_PUBLIC
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ConsoleTestReporter.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ExecuteTest.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ITestReporter.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRunner.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/unittest.h
57:     )
58: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_APPLICATION_CODE}

Now that we have added a test runner and a test report, we can use these to make running the tests even simpler.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/BCMRegisters.h>
4: #include <baremetal/Console.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/Mailbox.h>
7: #include <baremetal/MemoryManager.h>
8: #include <baremetal/New.h>
9: #include <baremetal/RPIProperties.h>
10: #include <baremetal/Serialization.h>
11: #include <baremetal/String.h>
12: #include <baremetal/SysConfig.h>
13: #include <baremetal/System.h>
14: #include <baremetal/Timer.h>
15: #include <baremetal/Util.h>
16:
17: #include <unittest/unittest.h>
18:
19: LOG_MODULE("main");
20:
21: using namespace baremetal;
22: using namespace unittest;
23:
24: namespace Suite1 {
25:
26: inline char const* GetSuiteName()
27: {
28:     return baremetal::string("Suite1");
29: }
30:
31: class FixtureMyTest1
32:     : public TestFixture
33: {
34: public:
35:     void SetUp() override
36:     {
37:         LOG_DEBUG("FixtureMyTest1 SetUp");
38:     }
39:     void TearDown() override
40:     {
41:         LOG_DEBUG("FixtureMyTest1 TearDown");
42:     }
43: };
44:
45: class FixtureMyTest1Helper
46:     : public FixtureMyTest1
47: {
48: public:
49:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
50:     explicit FixtureMyTest1Helper(const TestDetails& details)
51:         : m_details{ details }
52:     {
53:         SetUp();
54:     }
55:     virtual ~FixtureMyTest1Helper()
56:     {
57:         TearDown();
58:     }
59:     FixtureMyTest1Helper& operator = (const FixtureMyTest1Helper&) = delete;
60:     void RunImpl() const;
61:     const TestDetails& m_details;
62: };
63: void FixtureMyTest1Helper::RunImpl() const
64: {
65:     LOG_DEBUG(m_details.FixtureName().c_str());
66: }
67:
68: class MyTest1
69:     : public TestBase
70: {
71: public:
72:     MyTest1()
73:         : TestBase("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__)
74:     {
75:
76:     }
77:     void RunImpl() const override;
78: } MyTest1Instance;
79:
80: TestRegistrar registrarFixtureMyTest1(TestRegistry::GetTestRegistry(), &MyTest1Instance);
81:
82: void MyTest1::RunImpl() const
83: {
84:     LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
85:     FixtureMyTest1Helper fixtureHelper(Details());
86:     fixtureHelper.RunImpl();
87: }
88:
89: } // namespace Suite1
90:
91: namespace Suite2 {
92:
93: inline char const* GetSuiteName()
94: {
95:     return baremetal::string("Suite2");
96: }
97:
98: class FixtureMyTest2
99:     : public TestFixture
100: {
101: public:
102:     void SetUp() override
103:     {
104:         LOG_DEBUG("FixtureMyTest2 SetUp");
105:     }
106:     void TearDown() override
107:     {
108:         LOG_DEBUG("FixtureMyTest2 TearDown");
109:     }
110: };
111:
112: class FixtureMyTest2Helper
113:     : public FixtureMyTest2
114: {
115: public:
116:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
117:     explicit FixtureMyTest2Helper(const TestDetails& details)
118:         : m_details{ details }
119:     {
120:         SetUp();
121:     }
122:     virtual ~FixtureMyTest2Helper()
123:     {
124:         TearDown();
125:     }
126:     FixtureMyTest2Helper& operator = (const FixtureMyTest2Helper&) = delete;
127:     void RunImpl() const;
128:     const TestDetails& m_details;
129: };
130: void FixtureMyTest2Helper::RunImpl() const
131: {
132:     LOG_DEBUG(m_details.FixtureName().c_str());
133: }
134:
135: class MyTest2
136:     : public TestBase
137: {
138: public:
139:     MyTest2()
140:         : TestBase("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__)
141:     {
142:
143:     }
144:     void RunImpl() const override;
145: } MyTest2Instance;
146:
147: TestRegistrar registrarFixtureMyTest2(TestRegistry::GetTestRegistry(), &MyTest2Instance);
148:
149: void MyTest2::RunImpl() const
150: {
151:     LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
152:     FixtureMyTest2Helper fixtureHelper(Details());
153:     fixtureHelper.RunImpl();
154: }
155:
156: } // namespace Suite2
157:
158: class FixtureMyTest3
159:     : public TestFixture
160: {
161: public:
162:     void SetUp() override
163:     {
164:         LOG_DEBUG("FixtureMyTest3 SetUp");
165:     }
166:     void TearDown() override
167:     {
168:         LOG_DEBUG("FixtureMyTest3 TearDown");
169:     }
170: };
171:
172: class FixtureMyTest3Helper
173:     : public FixtureMyTest3
174: {
175: public:
176:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
177:     explicit FixtureMyTest3Helper(const TestDetails& details)
178:         : m_details{ details }
179:     {
180:         SetUp();
181:     }
182:     virtual ~FixtureMyTest3Helper()
183:     {
184:         TearDown();
185:     }
186:     FixtureMyTest3Helper& operator = (const FixtureMyTest3Helper&) = delete;
187:     void RunImpl() const;
188:     const TestDetails& m_details;
189: };
190: void FixtureMyTest3Helper::RunImpl() const
191: {
192:     LOG_DEBUG(m_details.FixtureName().c_str());
193:     CurrentTest::Results()->OnTestFailure(m_details, "Fail");
194: }
195:
196: class MyTest3
197:     : public TestBase
198: {
199: public:
200:     MyTest3()
201:         : TestBase("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__)
202:     {
203:
204:     }
205:     void RunImpl() const override;
206: } MyTest3Instance;
207:
208: TestRegistrar registrarFixtureMyTest3(TestRegistry::GetTestRegistry(), &MyTest3Instance);
209:
210: void MyTest3::RunImpl() const
211: {
212:     LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
213:     FixtureMyTest3Helper fixtureHelper(Details());
214:     fixtureHelper.RunImpl();
215: }
216:
217: int main()
218: {
219:     auto& console = GetConsole();
220:     LOG_DEBUG("Hello World!");
221:
222:     ConsoleTestReporter reporter;
223:     RunAllTests(&reporter);
224:
225:     LOG_INFO("Wait 5 seconds");
226:     Timer::WaitMilliSeconds(5000);
227:
228:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
229:     char ch{};
230:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
231:     {
232:         ch = console.ReadChar();
233:         console.WriteChar(ch);
234:     }
235:     if (ch == 'p')
236:         assert(false);
237:
238:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
239: }
```

- Line 17: We replace all unit test includes by a single one
- Line 193: We inject a failure to get a failing result
- Line 222: We instantiate a test reporter throug the `ConsoleTestReporter` class
- Line 223: We run all tests using the test reporter

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_5_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

You will see all the output being generared by the console test reported using colors.

<img src="images/demo-output-unit-test.png" alt="Tree view" width="800"/>

## Adding macros - Step 6 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_MACROS__STEP_6}

### TestMacros.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_MACROS__STEP_6_TESTMACROSH}

The most cumbersome about the current implementation is that we need to be very verbose when writing tests, test fixture and test suites.
We can easily solve this by creating some macros.

Create the file `code/libraries/unittest/include/unittest/TestMacros.h`

```cpp
File: code/libraries/unittest/include/unittest/TestMacros.h
File: d:\Projects\baremetal.github\tutorial\17-unit-tests\code\libraries\unittest\include\unittest\TestMacros.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : TestMacros.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : -
9: //
10: // Description : Macros for specifyig tests
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
38: //------------------------------------------------------------------------------
39:
40: #pragma once
41:
42: #define TEST_EX(TestName, Registry)                                                                         \
43: class Test##TestName : public unittest::TestBase                                                            \
44: {                                                                                                           \
45: public:                                                                                                     \
46:     Test##TestName() : TestBase(baremetal::string(#TestName), baremetal::string(""), baremetal::string(GetSuiteName()), baremetal::string(__FILE__), __LINE__) {} \
47: private:                                                                                                    \
48:     virtual void RunImpl() const override;                                                                  \
49: } test##TestName##Instance;                                                                                 \
50:                                                                                                             \
51: static unittest::TestRegistrar registrar##TestName(Registry, &test##TestName##Instance);                    \
52:                                                                                                             \
53: void Test##TestName::RunImpl() const
54:
55: #define TEST(TestName) TEST_EX(TestName, unittest::TestRegistry::GetTestRegistry())
56:
57: #define TEST_FIXTURE_EX(FixtureClass,TestName,Registry) \
58: class FixtureClass##TestName##Helper : public FixtureClass                                                  \
59: {                                                                                                           \
60: public:                                                                                                     \
61:     FixtureClass##TestName##Helper(const FixtureClass##TestName##Helper &) = delete;                        \
62:     explicit FixtureClass##TestName##Helper(unittest::TestDetails const & details) :                        \
63:         m_details{ details }                                                                                \
64:     {                                                                                                       \
65:         SetUp();                                                                                            \
66:     }                                                                                                       \
67:     virtual ~FixtureClass##TestName##Helper()                                                               \
68:     {                                                                                                       \
69:         TearDown();                                                                                         \
70:     }                                                                                                       \
71:     FixtureClass##TestName##Helper & operator = (const FixtureClass##TestName##Helper &) = delete;          \
72:     virtual void SetUp() {}                                                                                 \
73:     virtual void TearDown() {}                                                                              \
74:     void RunImpl() const;                                                                                   \
75:     unittest::TestDetails const & m_details;                                                                \
76: };                                                                                                          \
77:                                                                                                             \
78: class Test##FixtureClass##TestName : public unittest::TestBase                                              \
79: {                                                                                                           \
80: public:                                                                                                     \
81:     Test##FixtureClass##TestName() :                                                                        \
82:         TestBase(baremetal::string(#TestName), baremetal::string(#FixtureClass), baremetal::string(GetSuiteName()), baremetal::string(__FILE__), __LINE__) \
83:     {                                                                                                       \
84:     }                                                                                                       \
85: private:                                                                                                    \
86:     void RunImpl() const override;                                                                          \
87: } test##FixtureClass##TestName##Instance;                                                                   \
88:                                                                                                             \
89: unittest::TestRegistrar registrar##FixtureClass##TestName(Registry, &test##FixtureClass##TestName##Instance); \
90:                                                                                                             \
91: void Test##FixtureClass##TestName::RunImpl() const                                                          \
92: {                                                                                                           \
93:     bool ctorOk = false;                                                                                    \
94:     FixtureClass##TestName##Helper fixtureHelper(Details());                                                \
95:     unittest::ExecuteTest(fixtureHelper, Details());                                                        \
96: }                                                                                                           \
97: void FixtureClass##TestName##Helper::RunImpl() const
98:
99: #define TEST_FIXTURE(FixtureClass,TestName) TEST_FIXTURE_EX(FixtureClass,TestName,unittest::TestRegistry::GetTestRegistry())
100:
101: #define TEST_SUITE(SuiteName)                                                                               \
102:     namespace Suite##SuiteName                                                                              \
103:     {                                                                                                       \
104:         inline char const* GetSuiteName()                                                                   \
105:         {                                                                                                   \
106:             return baremetal::string(#SuiteName);                                                           \
107:         }                                                                                                   \
108:     }                                                                                                       \
109:     namespace Suite##SuiteName
```

- Line 42-53: We define the macro `TEST_EX`. This is used by macro `TEST`,
The parameters are the name of the test `TestName` and the reference to the singleton `TestRegistery` instance `Registry`.
It declare the class `Test<TestName>`, which inherits from `TestBase`, and defines an instance named `test<TestName>Instance`.
It then defines a `TestRegistrar` instance named `registrar<TestName>` which register the instance `test<TestName>Instance`.
It then starts the definition of the `RunImpl()` which is expected to be follow by the actual implementation of the test.
Compare this to the application code we wrote before:
```cpp
File: code/applications/demo/src/main.cpp
196: class MyTest3
197:     : public TestBase
198: {
199: public:
200:     MyTest3()
201:         : TestBase("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__)
202:     {
203:
204:     }
205:     void RunImpl() const override;
206: } MyTest3Instance;
207:
208: TestRegistrar registrarFixtureMyTest3(TestRegistry::GetTestRegistry(), &MyTest3Instance);
209:
210: void MyTest3::RunImpl() const
211: {
212:     LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
213:     FixtureMyTest3Helper fixtureHelper(Details());
214:     fixtureHelper.RunImpl();
215: }
```

- Line 55: We define the macro `TEST` which uses `TEST_EX` to register the test named `TestName`.
The only parameter for `TEST` is `TestName`, the name of the test.
The `TEST` macro is intended for tests that do not belong to a test fixture.
The way this macro can be used is as follow:

```cpp
TEST(MyTest)
{
    // Test implementation
}
```

- Line 57-97: We define the macro `TEST_FIXTURE_EX`. This is used by macro `TEST_FIXTURE`,
The parameters are the name of the fixture class `FixtureClass`, the name of the test `TestName` and the reference to the singleton `TestRegistery` instance `Registry`.
It declare the class `<FixtureClass><TestName>Helper`, which inherits from the `FixtureClass`, the class we defined for the fixture.
It then declares a class `Test<FixtureClass><TestName>`, which inherits from `TestBase`, and defines an instance named `test<FixtureClass><TestName>Instance`.
It then defines a `TestRegistrar` instance named `registrar<FixtureClass><TestName>` which register the instance `test<FixtureClass><TestName>Instance`.
It then defines the implement of `RunImpl()` for the class `Test<FixtureClass><TestName>`. This creates and instance of class `<FixtureClass><TestName>Helper`, and then uses the `ExecuteTest()` function to run the test.
Finally, it then starts the definition of the `RunImpl()` for the class `<FixtureClass><TestName>Helper` which is expected to be follow by the actual implementation of the test.
Compare this to the application code we wrote before:

```cpp
File: code/applications/demo/src/main.cpp
172: class FixtureMyTest3Helper
173:     : public FixtureMyTest3
174: {
175: public:
176:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
177:     explicit FixtureMyTest3Helper(const TestDetails& details)
178:         : m_details{ details }
179:     {
180:         SetUp();
181:     }
182:     virtual ~FixtureMyTest3Helper()
183:     {
184:         TearDown();
185:     }
186:     FixtureMyTest3Helper& operator = (const FixtureMyTest3Helper&) = delete;
187:     void RunImpl() const;
188:     const TestDetails& m_details;
189: };
190: void FixtureMyTest3Helper::RunImpl() const
191: {
192:     LOG_DEBUG(m_details.FixtureName().c_str());
193:     CurrentTest::Results()->OnTestFailure(m_details, "Fail");
194: }
195:
196: class MyTest3
197:     : public TestBase
198: {
199: public:
200:     MyTest3()
201:         : TestBase("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__)
202:     {
203:
204:     }
205:     void RunImpl() const override;
206: } MyTest3Instance;
207:
208: TestRegistrar registrarFixtureMyTest3(TestRegistry::GetTestRegistry(), &MyTest3Instance);
209:
210: void MyTest3::RunImpl() const
211: {
212:     LOG_DEBUG("Running %s in fixture %s in suite %s", Details().TestName().c_str(), Details().FixtureName().c_str(), Details().SuiteName().empty() ? "default" : Details().SuiteName().c_str());
213:     FixtureMyTest3Helper fixtureHelper(Details());
214:     fixtureHelper.RunImpl();
215: }
```

- Line 99: We define the macro `TEST_FIXTURE` which uses `TEST_FIXTURE_EX` to register the test named `TestName` for test firture class `FixtureClass`.
The parameters are the name of the fixture class `FixtureClass`, the name of the test `TestName`.
The `TEST_FIXTURE` macro is intended for tests that do belong to a test fixture.
The way this macro can be used is as follow:

```cpp
class MyFixture
    : public TestFixture
{
public:
    void SetUp() override
    {
        // Setup code
    }
    void TearDown() override
    {
        // Teardown code
    }
};

TEST_FIXTURE(MyFixture, MyTest)
{
    // Test implementation
}
```

Of course there can be multiple tests in a fixture, so the `TEST_FIXTURE` macro can be repeated

- Line 101-109: We define the macro `TEST_SUITE` which defined a test suite. It create a namespace for the test suite named `SuiteName`.
The only parameter is the name of the test suite `SuiteName`.
The `TEST_SUITE` macro is intended for create a test suite around tests and test fixtures.
The way this macro can be used is as follow:

```cpp
TEST_SUITE(MySuite)
{

TEST(MyTest)
{
    // Test implementation
}

or

TEST_FIXTURE(MyFixture, MyTest)
{
    // Test implementation
}

}
```

Compare the defintion of this macro to the application code we wrote before:

```cpp
File: code/applications/demo/src/main.cpp
24: namespace Suite1 {
25:
26: inline char const* GetSuiteName()
27: {
28:     return baremetal::string("Suite1");
29: }
...
89: } // namespace Suite1
```

### TestRegistry.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_MACROS__STEP_6_TESTREGISTRYH}

Now that we are using macros to define test suite, test fixture and tests, we can make some of the methods private.
Let's start at the top level with the `TestRegistry` class.

Update the file `code/libraries/unittest/include/unittest/TestRegistry.h`

```cpp
File: code/libraries/unittest/include/unittest/TestRegistry.h
...
51: class TestRegistry
52: {
53: private:
54:     friend class TestRegistrar;
55:     TestSuiteInfo* m_head;
56:     TestSuiteInfo* m_tail;
57:
58: public:
59:     static const char* DefaultFixtureName;
60:     static const char* DefaultSuiteName;
61:
62:     TestRegistry();
63:     TestRegistry(const TestRegistry&) = delete;
64:     TestRegistry(TestRegistry&&) = delete;
65:     virtual ~TestRegistry();
66:
67:     TestRegistry& operator = (const TestRegistry&) = delete;
68:     TestRegistry& operator = (TestRegistry&&) = delete;
69:
70:     TestSuiteInfo* GetHead() const;
71:
72:     template <class Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
73:     template <typename Predicate> int CountSuitesIf(Predicate predicate);
74:     template <typename Predicate> int CountFixturesIf(Predicate predicate);
75:     template <typename Predicate> int CountTestsIf(Predicate predicate);
76:
77:     static TestRegistry& GetTestRegistry();
78:
79: private:
80:     TestSuiteInfo* GetTestSuite(const baremetal::string& suiteName);
81:     void AddSuite(TestSuiteInfo* testSuite);
82: };
```

- Line 54: We make `TestRegistrar` a friend class
- Line 80: We move the method `GetTestSuite() to make it private
- Line 81: We make the method `AddSuite()` private

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_MACROS__STEP_6_TESTSUITEINFOH}

We will also make similar methods in the `TestSuiteInfo` class private.

Update the file `code/libraries/unittest/include/unittest/TestSuiteInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuiteInfo.h
...
51: class TestSuiteInfo
52: {
53: private:
54:     friend class TestRegistry;
55:     friend class TestRegistrar;
56:     TestFixtureInfo* m_head;
57:     TestFixtureInfo* m_tail;
58:     TestSuiteInfo* m_next;
59:     baremetal::string m_suiteName;
60:
61: public:
62:     TestSuiteInfo() = delete;
63:     TestSuiteInfo(const TestSuiteInfo&) = delete;
64:     TestSuiteInfo(TestSuiteInfo&&) = delete;
65:     explicit TestSuiteInfo(const baremetal::string& suiteName);
66:     virtual ~TestSuiteInfo();
67:
68:     TestSuiteInfo& operator = (const TestSuiteInfo&) = delete;
69:     TestSuiteInfo& operator = (TestSuiteInfo&&) = delete;
70:
71:     TestFixtureInfo* GetHead() const;
72:
73:     const baremetal::string& Name() const { return m_suiteName; }
74:
75:     template <class Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
76:
77:     int CountFixtures();
78:     int CountTests();
79:     template <typename Predicate> int CountFixturesIf(Predicate predicate);
80:     template <typename Predicate> int CountTestsIf(Predicate predicate);
81:
82: private:
83:     TestFixtureInfo* GetTestFixture(const baremetal::string& fixtureName);
84:     void AddFixture(TestFixtureInfo* testFixture);
85: };
```

- Line 55: We make `TestRegistrar` a friend class
- Line 83: We move the method `GetTestFixture() to make it private
- Line 84: We make the method `AddFixture()` private

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_MACROS__STEP_6_TESTFIXTUREINFOH}

We will also make similar methods in the `TestFixtureInfo` class private.

Update the file `code/libraries/unittest/include/unittest/TestFixtureInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixtureInfo.h
...
50: class TestFixtureInfo
51: {
52: private:
53:     friend class TestSuiteInfo;
54:     friend class TestRegistrar;
55:     TestBase* m_head;
56:     TestBase* m_tail;
57:     TestFixtureInfo* m_next;
58:     baremetal::string m_fixtureName;
59:
60: public:
61:     TestFixtureInfo() = delete;
62:     TestFixtureInfo(const TestFixtureInfo&) = delete;
63:     TestFixtureInfo(TestFixtureInfo&&) = delete;
64:     explicit TestFixtureInfo(const baremetal::string& fixtureName);
65:     virtual ~TestFixtureInfo();
66:
67:     TestFixtureInfo & operator = (const TestFixtureInfo &) = delete;
68:     TestFixtureInfo& operator = (TestFixtureInfo&&) = delete;
69:
70:     TestBase* GetHead() const;
71:
72:     const baremetal::string& Name() const { return m_fixtureName; }
73:
74:     template <class Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
75:
76:     int CountTests();
77:     template <typename Predicate> int CountTestsIf(Predicate predicate);
78:
79: private:
80:     void AddTest(TestBase* test);
81: };
```

- Line 54: We make `TestRegistrar` a friend class
- Line 80: We make the method `AddTest()` private

### unittest.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_MACROS__STEP_6_UNITTESTH}

We added a header file, so we'll update the `unittest.h` header

Update the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
...
42: #include <unittest/TestFixture.h>
43: #include <unittest/TestSuite.h>
44:
45: #include <unittest/ITestReporter.h>
46: #include <unittest/ConsoleTestReporter.h>
47: #include <unittest/ExecuteTest.h>
48: #include <unittest/TestBase.h>
49: #include <unittest/TestDetails.h>
50: #include <unittest/TestFixtureInfo.h>
51: #include <unittest/TestMacros.h>
52: #include <unittest/TestRegistry.h>
53: #include <unittest/TestResults.h>
54: #include <unittest/TestRunner.h>
55: #include <unittest/TestSuiteInfo.h>
```

### Update project configuration {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_MACROS__STEP_6_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleTestReporter.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRunner.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
40:     )
41:
42: set(PROJECT_INCLUDES_PUBLIC
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ConsoleTestReporter.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ExecuteTest.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ITestReporter.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestMacros.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRunner.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/unittest.h
58:     )
59: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_MACROS__STEP_6_APPLICATION_CODE}

Now that we have added a test runner and a test report, we can use these to make running the tests even simpler.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/BCMRegisters.h>
4: #include <baremetal/Console.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/Mailbox.h>
7: #include <baremetal/MemoryManager.h>
8: #include <baremetal/New.h>
9: #include <baremetal/RPIProperties.h>
10: #include <baremetal/Serialization.h>
11: #include <baremetal/String.h>
12: #include <baremetal/SysConfig.h>
13: #include <baremetal/System.h>
14: #include <baremetal/Timer.h>
15: #include <baremetal/Util.h>
16:
17: #include <unittest/unittest.h>
18:
19: LOG_MODULE("main");
20:
21: using namespace baremetal;
22: using namespace unittest;
23:
24: TEST_SUITE(Suite1)
25: {
26:
27: class FixtureMyTest1
28:     : public TestFixture
29: {
30: public:
31:     void SetUp() override
32:     {
33:         LOG_DEBUG("FixtureMyTest1 SetUp");
34:     }
35:     void TearDown() override
36:     {
37:         LOG_DEBUG("FixtureMyTest1 TearDown");
38:     }
39: };
40:
41: TEST_FIXTURE(FixtureMyTest1,Test1)
42: {
43:     LOG_DEBUG(m_details.FixtureName().c_str());
44: }
45:
46: } // Suite1
47:
48: TEST_SUITE(Suite2)
49: {
50:
51: class FixtureMyTest2
52:     : public TestFixture
53: {
54: public:
55:     void SetUp() override
56:     {
57:         LOG_DEBUG("FixtureMyTest2 SetUp");
58:     }
59:     void TearDown() override
60:     {
61:         LOG_DEBUG("FixtureMyTest2 TearDown");
62:     }
63: };
64:
65: TEST_FIXTURE(FixtureMyTest2, Test2)
66: {
67:     LOG_DEBUG(m_details.FixtureName().c_str());
68: }
69:
70: } // Suite2
71:
72: class FixtureMyTest3
73:     : public TestFixture
74: {
75: public:
76:     void SetUp() override
77:     {
78:         LOG_DEBUG("FixtureMyTest3 SetUp");
79:     }
80:     void TearDown() override
81:     {
82:         LOG_DEBUG("FixtureMyTest3 TearDown");
83:     }
84: };
85:
86: TEST_FIXTURE(FixtureMyTest3, Test3)
87: {
88:     LOG_DEBUG(m_details.FixtureName().c_str());
89:     CurrentTest::Results()->OnTestFailure(m_details, "Fail");
90: }
91:
92: TEST(Test4)
93: {
94:     LOG_DEBUG(Details().FixtureName().c_str());
95:     CurrentTest::Results()->OnTestFailure(Details(), "Fail");
96:     CurrentTest::Results()->OnTestFailure(Details(), "FailAgain");
97: }
98:
99: int main()
100: {
101:     auto& console = GetConsole();
102:     LOG_DEBUG("Hello World!");
103:
104:     ConsoleTestReporter reporter;
105:     RunAllTests(&reporter);
106:
107:     LOG_INFO("Wait 5 seconds");
108:     Timer::WaitMilliSeconds(5000);
109:
110:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
111:     char ch{};
112:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
113:     {
114:         ch = console.ReadChar();
115:         console.WriteChar(ch);
116:     }
117:     if (ch == 'p')
118:         assert(false);
119:
120:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
121: }
```

- Line 24: We replace the namespace Suite1 and its `GetSuiteName()` function by the macro `TEST_SUITE`
- Line 41: We replace the declaration and definition of classes `FixtureMyTest1Helper` and `MyTest1` by the macro `TEST_FIXTURE`.
Note that we still need to defined the class `FixtureMyTest1` as it defines common behaviour for all tests in the test fixture
- Line 48: We replace the namespace Suite2 and its `GetSuiteName()` function by the macro `TEST_SUITE`
- Line 65: We replace the declaration and definition of classes `FixtureMyTest2Helper` and `MyTest2` by the macro `TEST_FIXTURE`.
Note that we still need to defined the class `FixtureMyTest2` as it defines common behaviour for all tests in the test fixture
- Line 86: We replace the declaration and definition of classes `FixtureMyTest3Helper` and `MyTest3` by the macro `TEST_FIXTURE`.
Note that we still need to defined the class `FixtureMyTest3` as it defines common behaviour for all tests in the test fixture
- Line 92-97: We define a test outside of a fixture using the `TEST` macro. Note that its implementation generates two failures

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_MACROS__STEP_6_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. As we have one failure in `Test3`, and two in `Test4`, we expect to see two failing tests, and three failures in total.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Debug  Register test Test1 in fixture FixtureMyTest1 in suite Suite1 (TestRegistry:122)
Debug  Fixture FixtureMyTest1 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test2 in fixture FixtureMyTest2 in suite Suite2 (TestRegistry:122)
Debug  Find suite Suite2 ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest2 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test3 in fixture FixtureMyTest3 in suite DefaultSuite (TestRegistry:122)
Debug  Find suite DefaultSuite ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest3 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test4 in fixture DefaultFixture in suite DefaultSuite (TestRegistry:122)
Debug  Find suite DefaultSuite ... found (TestRegistry:93)
Debug  Fixture DefaultFixture not found, creating new object (TestSuiteInfo:79)
Info   Starting up (System:201)
Debug  Hello World! (main:102)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
Debug  FixtureMyTest1 (main:43)
[ SUCCEEDED ] Suite1::FixtureMyTest1::Test1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
Debug  FixtureMyTest2 (main:67)
[ SUCCEEDED ] Suite2::FixtureMyTest2::Test2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
Debug  FixtureMyTest3 (main:88)
FixtureMyTest3::Test3 failure Fail
[  FAILED   ] FixtureMyTest3::Test3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
Debug   (main:94)
Test4 failure Fail
Test4 failure FailAgain
[  FAILED   ] Test4
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 2 out of 4 tests failed (3 failures).

Failures: 3
[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   Wait 5 seconds (main:107)
Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

## Collecting test information - Step 7 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7}

So far we have been focussing on the structure for running tests, but not on the actual tests.
It would be nice to be able to check using simple statements such as:

```cpp
TEST(MyTest)
{
    bool b{};
    EXPECT_FALSE(b);
    ASSERT_TRUE(!b);
    ASSERT_EQ(b, false);
}
```

Let's create macros to perform these checks, and and a mechanisn to trace back the reason for a failure, for example when we expect to have a certain value, what the expected and actual values were.
Before we do that, let's upgrade our ConsoleTestReporter, such that it keeps the results of the test run, so we can print the information on failures as part of the summary.
That way the test run output itself we be more clean, and we have a simple summary of all failures. For this, we'll introduce a new class `DeferredTestReporter`.

### TestResult.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7_TESTRESULTH}

The `DeferredTestReporter` will use a list of entries holding a `TestResult` to keep track of the results during the test run.
The `TestResult` class will hold the information and result for a single test, which is extracted initially from `TestDetails`.
If a test failure occures, the failure information is attached to the `TestResult`.
We need to declare this class.

Create the file `code/libraries/unittest/include/unittest/TestResult.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResult.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestResult.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestResult
9: //
10: // Description : Test result
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include <baremetal/String.h>
43: #include <unittest/TestDetails.h>
44: 
45: namespace unittest
46: {
47: 
48: class Failure
49: {
50: private:
51:     int m_lineNumber;
52:     baremetal::string m_text;
53: 
54: public:
55:     Failure(int lineNumber, const baremetal::string& text);
56:     int SourceLineNumber() const { return m_lineNumber; }
57:     const baremetal::string& Text() const { return m_text; }
58: };
59: 
60: class FailureEntry
61: {
62: private:
63:     friend class FailureList;
64:     Failure m_failure;
65:     FailureEntry* m_next;
66: 
67: public:
68:     explicit FailureEntry(const Failure& failure);
69:     const Failure& GetFailure() const { return m_failure; }
70:     const FailureEntry* GetNext() const { return m_next; }
71: };
72: 
73: class FailureList
74: {
75: private:
76:     FailureEntry* m_head;
77:     FailureEntry* m_tail;
78: 
79: public:
80:     FailureList();
81:     ~FailureList();
82: 
83:     const FailureEntry* GetHead() const { return m_head; }
84:     void Add(const Failure& failure);
85: };
86: 
87: class TestResult
88: {
89: private:
90:     TestDetails m_details;
91:     FailureList m_failures;
92:     bool m_failed;
93: 
94: public:
95:     TestResult() = delete;
96:     explicit TestResult(const TestDetails & details);
97: 
98:     void AddFailure(const Failure& failure);
99:     const FailureList & Failures() const { return m_failures; }
100:     bool Failed() const { return m_failed; }
101:     const TestDetails& Details() const { return m_details; }
102: };
103: 
104: } // namespace unittest
```

- Line 48-58: We declare a class `Failure` to hold a single failure
  - Line 51: The member variable `m_lineNumber` holds the line number in the source file where the failure occurred
  - Line 52: The member variable `m_text` holds the failure message
  - Line 55: We declare the constructor
  - Line 56: We declare and define the method `SourceLineNumber()` which returns the line number
  - Line 57: We declare and define the method `Text()` which returns the failure message
- Line 60-71: We declare a class `FailureEntry` which holds a failure, and a pointer to the next failure
  - Line 64: The member variable `m_failure` holds the failure
  - Line 65: The member variable `m_next` holds a pointer to the next `FailureEntry` in the list
  - Line 68: We declare the constructor
  - Line 69: We declare and define the method `GetFailure()` which returns a const reference to the failure
  - Line 70: We declare and define the method `GetNext()` which returns a const pointer to the next `FailureEntry` in the list
- Line 73-85: We declare a class `FailureList` which holds a pointer to the beginning and the end of a `FailureEntry` list
  - Line 76-77: The member variables `m_head` and `m_tail` hold a pointer to the beginning and the end of the list, respectively
  - Line 80: We declare the constructor
  - Line 81: We declare the destructor, which will clean up the list of `FailureEntry` instances
  - Line 83: We declare and define the method `GetHead()` which returns a const pointer to the first `FailureEntry` in the list
  - Line 84: We declare and define the method `Add()` which adds a failure to the list (embedded in a `FailureEntry` instance)
- Line 87-104: We declare the class `TestResult`
  - Line 90: The member variable `m_details` holds the test details
  - Line 91: The member variable `m_failures` holds the failure list
  - Line 92: The member variable `m_failed` holds true if at least one failure occurred for this test
  - Line 95: We remove the default constructor
  - Line 96: We declare an explicit constructor
  - Line 98: We declare the method `AddFailure()` which adds a failure to the list
  - Line 99: We declare and define the method `Failures()` which returns a const reference to the failure list
  - Line 100: We declare and define the method `Failed()` which returns true if a failure occurred
  - Line 101: We declare and define the method `Details()` which returns the test details

### TestResult.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7_TESTRESULTCPP}

Let's implement the `TestResult` class.

Create the file `code/libraries/unittest/src/TestResult.cpp`

```cpp
File: code/libraries/unittest/src/TestResult.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestResult.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestResult
9: //
10: // Description : Test result
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
38: //------------------------------------------------------------------------------
39: 
40: #include <unittest/TestResult.h>
41: 
42: #include <unittest/TestDetails.h>
43: 
44: using namespace baremetal;
45: 
46: namespace unittest {
47: 
48: Failure::Failure(int lineNumber, const baremetal::string& text)
49:     : m_lineNumber{lineNumber}
50:     , m_text{text}
51: {
52: }
53: 
54: FailureEntry::FailureEntry(const Failure& failure)
55:     : m_failure{ failure }
56:     , m_next{}
57: {
58: }
59: 
60: FailureList::FailureList()
61:     : m_head{}
62:     , m_tail{}
63: {
64: }
65: 
66: FailureList::~FailureList()
67: {
68:     auto current = m_head;
69:     while (current != nullptr)
70:     {
71:         auto next = current->m_next;
72:         delete current;
73:         current = next;
74:     }
75: }
76: 
77: void FailureList::Add(const Failure& failure)
78: {
79:     auto entry = new FailureEntry(failure);
80:     if (m_head == nullptr)
81:     {
82:         m_head = entry;
83:     }
84:     else
85:     {
86:         auto current = m_head;
87:         while (current->m_next != nullptr)
88:             current = current->m_next;
89:         current->m_next = entry;
90:     }
91:     m_tail = entry;
92: }
93: 
94: TestResult::TestResult(const TestDetails& details)
95:     : m_details{ details }
96:     , m_failures{}
97:     , m_failed{}
98: {
99: }
100: 
101: void TestResult::AddFailure(const Failure& failure)
102: {
103:     m_failures.Add(failure);
104:     m_failed = true;
105: }
106: 
107: } // namespace unittest
```

- Line 48-52: We implement the `Failure` constructor
- Line 54-58: We implement the `FailureEntry` constructor
- Line 60-64: We implement the `FailureList` constructor
- Line 66-75: We implement the `FailureList` destructor. This will delete all `FailureEntry` instances in the list
- Line 77-92: We implement the method `Add()` for `FailureList`. This will create a new `FailureEntry` with the `Failure` in it, and insert at the end of the list
- Line 94-99: We implement the `TestResult` constructor
- Line 101-105: We implement the method `AddFailure()` for `TestResult`. This will add the failure to the list, and set the failed flag to true

### DeferredTestReport.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7_DEFERREDTESTREPORTH}

Let's declare the `DeferredTestReporter` class.

Create the file `code/libraries/unittest/include/unittest/DeferredTestReporter.h`

```cpp
File: code/libraries/unittest/include/unittest/DeferredTestReporter.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : DeferredTestReporter.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : DeferredTestReporter
9: //
10: // Description : Deferred test reporter, which saves test results
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
38: //------------------------------------------------------------------------------
39: 
40: #include <unittest/ITestReporter.h>
41: #include <unittest/TestResult.h>
42: 
43: namespace unittest
44: {
45: 
46: class ResultEntry
47: {
48: private:
49:     friend class ResultList;
50:     TestResult m_result;
51:     ResultEntry* m_next;
52: 
53: public:
54:     explicit ResultEntry(const TestResult& result);
55:     TestResult& GetResult() { return m_result; }
56:     ResultEntry* GetNext() { return m_next; }
57: };
58: 
59: class ResultList
60: {
61: private:
62:     ResultEntry* m_head;
63:     ResultEntry* m_tail;
64: 
65: public:
66: 
67:     ResultList();
68:     ~ResultList();
69: 
70:     void Add(const TestResult& result);
71:     ResultEntry* GetHead() const { return m_head; }
72:     ResultEntry* GetTail() const { return m_tail; }
73: };
74: 
75: class DeferredTestReporter : public ITestReporter
76: {
77: private:
78:     ResultList m_results;
79: 
80: public:
81:     void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
82:     void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
83:     void ReportTestRunSummary(const TestResults& results) override;
84:     void ReportTestRunOverview(const TestResults& results) override;
85:     void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTestFixtures) override;
86:     void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTestFixtures) override;
87:     void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) override;
88:     void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests) override;
89:     void ReportTestStart(const TestDetails& details) override;
90:     void ReportTestFinish(const TestDetails& details, bool success) override;
91:     void ReportTestFailure(const TestDetails& details, const baremetal::string& failure) override;
92: 
93:     ResultList& Results();
94: };
95: 
96: } // namespace unittest
```

- Line 46-57: We declare the struct `ResultEntry`, which holds a `TestResult`, which we'll declare later, and a pointer to the next `ResultEntry`. The results entries form a linked list, and are used to gather results for each test
  - Line 50: The member variable `m_result` holds the test result
  - Line 51: The member variable `m_next` holds the pointer to the next `ResultEntry` in the list
  - Line 54: We declare the constructor
  - Line 55: We declare and define the method `GetResult()` which returns the test result
  - Line 56: We declare and define the method `GetNext()` which returns the pointer to the next `ResultEntry` in the list
- Line 59-73: We declare the class `ResultList` which holds a pointer to the first and last `ResultEntry`
  - Line 62-63-77: The member variables `m_head` and `m_tail` hold a pointer to the beginning and the end of the list, respectively
  - Line 67: We declare the constructor
  - Line 68: We declare the destructor, which will clean up the list of `ResultEntry` instances
  - Line 70: We declare and define the method `Add()` which adds a test result to the list (embedded in a `ResultEntry` instance)
  - Line 71: We declare and define the method `GetHead()` which returns a const pointer to the first `ResultEntry` in the list
  - Line 72: We declare and define the method `GetTail()` which returns a const pointer to the last `ResultEntry` in the list
- Line 75-94: We declare the class `DeferredTestReporter`, which implements the abstract interface `ITestReporter`
  - Line 78: The class variable `m_results` holds the list of test results saved during the test run
  - Line 81-91: We implement the `ITestReporter` interface
  - Line 93: We declare the method `Results()` which returns the `ResultList`

### DeferredTestReport.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7_DEFERREDTESTREPORTCPP}

Let's implement the `DeferredTestReport` class.

Create the file `code/libraries/unittest/src/DeferredTestReport.cpp`

```cpp
File: code/libraries/unittest/src/DeferredTestReport.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : DeferredTestReporter.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : DeferredTestReporter
9: //
10: // Description : Test detail
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
38: //------------------------------------------------------------------------------
39: 
40: #include <unittest/DeferredTestReporter.h>
41: 
42: #include <unittest/TestDetails.h>
43: 
44: using namespace baremetal;
45: 
46: namespace unittest
47: {
48: 
49: ResultEntry::ResultEntry(const TestResult& result)
50:     : m_result{ result }
51:     , m_next{}
52: {
53: }
54: 
55: ResultList::ResultList()
56:     : m_head{}
57:     , m_tail{}
58: {
59: }
60: 
61: ResultList::~ResultList()
62: {
63:     auto current = m_head;
64:     while (current != nullptr)
65:     {
66:         auto next = current->m_next;
67:         delete current;
68:         current = next;
69:     }
70: }
71: 
72: void ResultList::Add(const TestResult& result)
73: {
74:     auto entry = new ResultEntry(result);
75:     if (m_head == nullptr)
76:     {
77:         m_head = entry; 
78:     }
79:     else
80:     {
81:         auto current = m_head;
82:         while (current->m_next != nullptr)
83:             current = current->m_next;
84:         current->m_next = entry;
85:     }
86:     m_tail = entry;
87: }
88: 
89: void DeferredTestReporter::ReportTestRunStart(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
90: {
91: }
92: 
93: void DeferredTestReporter::ReportTestRunFinish(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
94: {
95: }
96: 
97: void DeferredTestReporter::ReportTestRunSummary(const TestResults& /*results*/)
98: {
99: }
100: 
101: void DeferredTestReporter::ReportTestRunOverview(const TestResults& /*results*/)
102: {
103: }
104: 
105: void DeferredTestReporter::ReportTestSuiteStart(const string& /*suiteName*/, int /*numberOfTestFixtures*/)
106: {
107: }
108: 
109: void DeferredTestReporter::ReportTestSuiteFinish(const string& /*suiteName*/, int /*numberOfTests*/)
110: {
111: }
112: 
113: void DeferredTestReporter::ReportTestFixtureStart(const string& /*fixtureName*/, int /*numberOfTests*/)
114: {
115: }
116: 
117: void DeferredTestReporter::ReportTestFixtureFinish(const string& /*fixtureName*/, int /*numberOfTests*/)
118: {
119: }
120: 
121: void DeferredTestReporter::ReportTestStart(const TestDetails& details)
122: {
123:     m_results.Add(TestResult(details));
124: }
125: 
126: void DeferredTestReporter::ReportTestFinish(const TestDetails& /*details*/, bool /*success*/)
127: {
128:     TestResult& result = m_results.GetTail()->GetResult();
129: }
130: 
131: void DeferredTestReporter::ReportTestFailure(const TestDetails& details, const string& failure)
132: {
133:     TestResult& result = m_results.GetTail()->GetResult();
134:     result.AddFailure(Failure(details.SourceFileLineNumber(), failure));
135: }
136: 
137: ResultList& DeferredTestReporter::Results()
138: {
139:     return m_results;
140: }
141: 
142: } // namespace unittest
```

- Line 49-53: We implement the `ResultEntry` constructor
- Line 55-59: We implement the `ResultList` constructor
- Line 72-87: We implement the method `Add` for `ResultList`. This will create a new `ResultEntry` and insert it at the end of the list
- Line 89-91: We implement the method `ReportTestRunStart` for `DeferredTestReporter`.
This does nothing, as `DeferredTestReporter` does not report anything in itself. It simply stores test results 
- Line 93-95: We implement the method `ReportTestRunFinish` for `DeferredTestReporter`. This again does nothing
- Line 97-99: We implement the method `ReportTestRunSummary` for `DeferredTestReporter`. This again does nothing
- Line 101-103: We implement the method `ReportTestRunOverview` for `DeferredTestReporter`. This again does nothing
- Line 105-107: We implement the method `ReportTestSuiteStart` for `DeferredTestReporter`. This again does nothing
- Line 109-111: We implement the method `ReportTestSuiteFinish` for `DeferredTestReporter`. This again does nothing
- Line 113-115: We implement the method `ReportTestFixtureStart` for `DeferredTestReporter`. This again does nothing
- Line 117-119: We implement the method `ReportTestFixtureFinish` for `DeferredTestReporter`. This again does nothing
- Line 117-119: We implement the method `ReportTestStart` for `DeferredTestReporter`. This adds a new result to the list
- Line 117-119: We implement the method `ReportTestFinish` for `DeferredTestReporter`. This again does nothing
- Line 117-119: We implement the method `ReportTestFailure` for `DeferredTestReporter`. This adds a failure to the list for the current result
- Line 117-119: We implement the method `Results` for `DeferredTestReporter`

### ConsoleTestReport.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7_CONSOLETESTREPORTH}

We need to update the class `ConsoleTestReporter` to derive from `DeferredTestReporter`.

Update the file  `code/libraries/unittest/include/unittest/ConsoleTestReport.h`

```cpp
File: code/libraries/unittest/include/unittest/ConsoleTestReport.h
...
42: #include <unittest/DeferredTestReporter.h>
43: 
...
47: class ConsoleTestReporter : public DeferredTestReporter
48: {
...
77:     baremetal::string TestFailureMessage(const TestResult& result, const Failure& failure);
...
```

- Line 42: We need to include the header for `DeferredTestReporter` instead of for the interface
- Line 47: We inherit from `DeferredTestReporter`
- Line 77: We replace the method `TestFailureMessage()` with a version taking a `TestResult` and a `Failure`

### ConsoleTestReport.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7_CONSOLETESTREPORTCPP}

Let's update the implementation for `ConsoleTestReport`.

Update the file `code/libraries/unittest/src/ConsoleTestReport.cpp`

```cpp
File: code/libraries/unittest/src/ConsoleTestReport.cpp
...
133: void ConsoleTestReporter::ReportTestStart(const TestDetails& details)
134: {
135:     DeferredTestReporter::ReportTestStart(details);
136: }
137: 
138: void ConsoleTestReporter::ReportTestFinish(const TestDetails& details, bool success)
139: {
140:     DeferredTestReporter::ReportTestFinish(details, success);
141:     GetConsole().SetTerminalColor(success ? ConsoleColor::Green : ConsoleColor::Red);
142:     if (success)
143:         GetConsole().Write(TestSuccessSeparator);
144:     else
145:         GetConsole().Write(TestFailSeparator);
146:     GetConsole().ResetTerminalColor();
147: 
148:     GetConsole().Write(Format(" %s\n", TestFinishMessage(details, success).c_str()));
149: }
150: 
151: void ConsoleTestReporter::ReportTestFailure(const TestDetails& details, const string& failure)
152: {
153:     DeferredTestReporter::ReportTestFailure(details, failure);
154: }
...
224: string ConsoleTestReporter::TestRunOverviewMessage(const TestResults& results)
225: {
226: 
227:     if (results.GetFailureCount() > 0)
228:     {
229:         string result = "Failures:\n";
230:         auto testResultPtr = Results().GetHead();
231:         while (testResultPtr != nullptr)
232:         {
233:             auto const& testResult = testResultPtr->GetResult();
234:             if (testResult.Failed())
235:             {
236:                 auto failuresPtr = testResult.Failures().GetHead();
237:                 while (failuresPtr != nullptr)
238:                 {
239:                     result.append(TestFailureMessage(testResult, failuresPtr->GetFailure()));
240:                     failuresPtr = failuresPtr->GetNext();
241:                 }
242:             }
243:             testResultPtr = testResultPtr->GetNext();
244:         }
245:         return result;
246:     }
247:     return "No failures";
248: }
249: 
250: string ConsoleTestReporter::TestFailureMessage(const TestResult& result, const Failure& failure)
251: {
252:     return Format("%s:%d : Failure in %s: %s\n",
253:         result.Details().SourceFileName().c_str(),
254:         failure.SourceLineNumber(), 
255:         TestName(result.Details().SuiteName(), result.Details().FixtureName(), result.Details().TestName()).c_str(),
256:         failure.Text().c_str());
257: }
...
296: string ConsoleTestReporter::TestName(const string& suiteName, const string& fixtureName, const string& testName)
297: {
298:     string result;
299:     if (!suiteName.empty())
300:     {
301:         result.append(suiteName);
302:         result.append("::");
303:     }
304:     if (!fixtureName.empty())
305:     {
306:         result.append(fixtureName);
307:         result.append("::");
308:     }
309:     result.append(testName);
310:     return result;
311: }
...
```

- Line 133-136: We implement the method `ReportTestStart` by calling the same method in `DeferredTestReporter`
- Line 138-149: We implement the method `ReportTestFinish` by first calling the same method in `DeferredTestReporter`
- Line 151-154: We implement the method `ReportTestFailure` by calling the same method in `DeferredTestReporter`
- Line 224-248: We implement the method `TestRunOverviewMessage` by going through the list of test results, and for any that have failed, going through the list of failures, and appending a test failure message
- Line 250-257: We implement the method `TestFailureMessage` to print a correct failure message
- Line 296: We remove the old implementation for `TestFailureMessage`

### Update project configuration {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleTestReporter.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/DeferredTestReporter.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResult.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRunner.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
42:     )
43: 
44: set(PROJECT_INCLUDES_PUBLIC
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ConsoleTestReporter.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/DeferredTestReporter.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ExecuteTest.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ITestReporter.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestMacros.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResult.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRunner.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/unittest.h
62:     )
63: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7_APPLICATION_CODE}

We will keep the application code unchanged for now.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_7_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. The reporting will be slightly different, as we now will see a summary of all failures.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Debug  Register test Test1 in fixture FixtureMyTest1 in suite Suite1 (TestRegistry:122)
Debug  Find suite Suite1 ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest1 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test2 in fixture FixtureMyTest2 in suite Suite2 (TestRegistry:122)
Debug  Find suite Suite2 ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest2 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test3 in fixture FixtureMyTest3 in suite DefaultSuite (TestRegistry:122)
Debug  Find suite DefaultSuite ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest3 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test4 in fixture DefaultFixture in suite DefaultSuite (TestRegistry:122)
Debug  Find suite DefaultSuite ... found (TestRegistry:93)
Debug  Fixture DefaultFixture not found, creating new object (TestSuiteInfo:79)
Info   Starting up (System:201)
Debug  Hello World! (main:102)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
Debug  FixtureMyTest1 (main:43)
[ SUCCEEDED ] Suite1::FixtureMyTest1::Test1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
Debug  FixtureMyTest2 (main:67)
[ SUCCEEDED ] Suite2::FixtureMyTest2::Test2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
Debug  FixtureMyTest3 (main:88)
[  FAILED   ] FixtureMyTest3::Test3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
Debug   (main:94)
[  FAILED   ] Test4
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 2 out of 4 tests failed (3 failures).

Failures:
../code/applications/demo/src/main.cpp:86 : Failure in FixtureMyTest3::Test3: Fail
../code/applications/demo/src/main.cpp:92 : Failure in Test4: Fail
../code/applications/demo/src/main.cpp:92 : Failure in Test4: FailAgain

[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   Wait 5 seconds (main:107)
Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

## Test assert macros - Step 8 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8}

Now that we have a test reporter that can summarize the test results, let's move on to the actual tests.
We'll define a set of macros, and we'll need to do some trickery to get it all to work well. So bare with me.

We'll start with simple boolean tests, and equality / inequality of integral types. The rest will follow in the next and final step.

### Checks.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_CHECKSH}

We'll start with some utility functions and classes.

Create the file `code/libraries/unittest/include/unittest/Checks.h`

```cpp
File: code/libraries/unittest/include/unittest/Checks.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Checks.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : AssertionResult
9: //
10: // Description : Check functions
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include <baremetal/String.h>
43: 
44: #include <unittest/PrintValue.h>
45: 
46: namespace unittest
47: {
48: 
49: class TestResults;
50: class TestDetails;
51: 
52: struct AssertionResult
53: {
54:     AssertionResult(bool failed, const baremetal::string& message)
55:         : failed(failed)
56:         , message(message)
57:     {
58:     }
59:     const bool failed;
60:     const baremetal::string message;
61:     operator bool() const { return failed; }
62: };
63: 
64: extern AssertionResult AssertionSuccess();
65: extern AssertionResult BooleanFailure(const baremetal::string& valueExpression,
66:                                       const baremetal::string& expectedValue,
67:                                       const baremetal::string& actualValue);
68: extern AssertionResult EqFailure(const baremetal::string& expectedExpression,
69:                                  const baremetal::string& actualExpression,
70:                                  const baremetal::string& expectedValue,
71:                                  const baremetal::string& actualValue);
72: extern AssertionResult InEqFailure(const baremetal::string& expectedExpression,
73:                                    const baremetal::string& actualExpression,
74:                                    const baremetal::string& expectedValue,
75:                                    const baremetal::string& actualValue);
76: 
77: template<typename Value>
78: bool CheckTrue(const Value value)
79: {
80:     return !!value;
81: }
82: 
83: template<typename Value>
84: bool CheckFalse(const Value value)
85: {
86:     return !value;
87: }
88: 
89: template <typename ToPrint>
90: class FormatForComparison
91: {
92: public:
93:     static baremetal::string Format(const ToPrint& value)
94:     {
95:         return PrintToString(value);
96:     }
97: };
98: 
99: template <typename T1>
100: baremetal::string FormatForComparisonFailureMessage(const T1& value)
101: {
102:     return FormatForComparison<T1>::Format(value);
103: }
104: 
105: template<typename Value>
106: AssertionResult CheckTrue(const baremetal::string& valueName, const Value& value)
107: {
108:     if (!CheckTrue(value))
109:     {
110:         return BooleanFailure(valueName,
111:                               baremetal::string("true"),
112:                               PrintToString(value));
113:     }
114:     return AssertionSuccess();
115: }
116: 
117: template< typename Value >
118: AssertionResult CheckFalse(const baremetal::string& valueName, const Value& value)
119: {
120:     if (!CheckFalse(value))
121:     {
122:         return BooleanFailure(valueName,
123:                               baremetal::string("false"),
124:                               PrintToString(value));
125:     }
126:     return AssertionSuccess();
127: }
128: 
129: template<typename Expected, typename Actual>
130: bool AreEqual(const Expected& expected, const Actual& actual)
131: {
132:     return (expected == actual);
133: }
134: 
135: template< typename Expected, typename Actual >
136: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
137:                                    const Expected& expected, const Actual& actual)
138: {
139:     if (!AreEqual(expected, actual))
140:     {
141:         return EqFailure(expectedExpression, actualExpression,
142:                          FormatForComparisonFailureMessage(expected),
143:                          FormatForComparisonFailureMessage(actual));
144:     }
145:     return AssertionSuccess();
146: }
147: 
148: template< typename Expected, typename Actual >
149: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
150:                                       const Expected& expected, const Actual& actual)
151: {
152:     if (AreEqual(expected, actual))
153:     {
154:         return InEqFailure(expectedExpression, actualExpression,
155:                            FormatForComparisonFailureMessage(expected),
156:                            FormatForComparisonFailureMessage(actual));
157:     }
158:     return AssertionSuccess();
159: }
160: 
161: class EqHelper
162: {
163: public:
164:     // This templatized version is for the general case.
165:     template <typename Expected, typename Actual>
166:     static AssertionResult CheckEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
167:                                       const Expected& expected, const Actual& actual)
168:     {
169:         return CheckEqualInternal(expectedExpression, actualExpression, expected, actual);
170:     }
171: 
172:     template <typename Expected, typename Actual>
173:     static AssertionResult CheckNotEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
174:                                          const Expected& expected, const Actual& actual)
175:     {
176:         return CheckNotEqualInternal(expectedExpression, actualExpression, expected, actual);
177:     }
178: };
179: 
180: } // namespace unittest
```

- Line 52-62: We define a struct `AssertionResult` which holds the status for a single assertion.
  - Line 54-58: We define the constructor
  - Line 59: The member variable `failed` flags whether the assertion failed
  - Line 60: The member vairbale `message` holds the failure message, if any
  - Line 61: The `bool()` operator returns the value of `failed`
- Line 64: We declare a function `AssertionSuccess()` to signify a successful assert
- Line 65-67: We declare a function `BooleanFailure()` to signify a assertion failure on a boolean expression (e.g. `ASSERT_FALSE`, `EXPECT_TRUE`)
- Line 68-71: We declare a function `EqFailure()` to signify a assertion failure on a equality expression (e.g. `EXPECT_EQ`)
- Line 72-75: We declare a function `InEqFailure()` to signify a assertion failure on a inequality expression (e.g. `EXPECT_NE`)
- Line 77-81: We declare a template function `CheckTrue()` to convert a value to a boolean, returning true if the value is true, or not equal to 0
- Line 83-87: We define a template function `CheckFalse()` to convert a value to a boolean, returning true if the value is false, or equal to 0
- Line 89-97: We define a template class `FormatForComparison` which has a single method `Format`, which prints the value passed to a string.
The printing is done using functionality in the header `PrintValue.h` which we'll get to in a minute
- Line 99-103: We define a template function `FormatForComparisonFailureMessage()` which uses the `FormatForComparison` class to print the value passed
- Line 105-115: We define a template function `CheckTrue()` which takes a value and its stringified version, and uses the `CheckTrue()` defined before to check whether the value is seen as true.
If so, `AssertionSuccess()` is returned, otherwise, `BooleanFailure()` is returned
- Line 117-127: We define a template function `CheckFalse()` which takes a value and its stringified version, and uses the `CheckFalse()` defined before to check whether the value is seen as false.
If so, `AssertionSuccess()` is returned, otherwise, `BooleanFailure()` is returned
- Line 129-133: We define a template function `AreEqual()` to compare two values `expected` and `actual`, which can have different types.
The function uses the equality operator to determine if the two values are equal
- Line 135-146: We define a template function `CheckEqualInternal()`, which uses `AreEqual()` to compare two values `expected` and `actual`. Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 148-159: We define a template function `CheckNotEqualInternal()`, which uses `AreEqual()` to compare two values `expected` and `actual`. Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered inequal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 161-178: We declare a class `EqHelper` which has two methods `CheckEqual()` (which uses `CheckEqualInternal()`) and `CheckNotEqual()` (which uses `CheckNotEqualInternal()`)

### Checks.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_CHECKSCPP}

Let's implement the functions for the checks.

Create the file `code/libraries/unittest/src/Checks.cpp`

```cpp
File: code/libraries/unittest/src/Checks.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Checks.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : Checks
9: //
10: // Description : Check functions
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
38: //------------------------------------------------------------------------------
39: 
40: #include "unittest/Checks.h"
41: 
42: #include <baremetal/Format.h>
43: #include <baremetal/Util.h>
44: 
45: using namespace baremetal;
46: 
47: namespace unittest {
48: 
49: AssertionResult AssertionSuccess()
50: {
51:     return AssertionResult(false, string());
52: }
53: 
54: AssertionResult BooleanFailure(const string& valueExpression, const string& expectedValue, const string& actualValue)
55: {
56:     string result = Format("Value of: %s", valueExpression.c_str());
57:     if (actualValue != valueExpression)
58:     {
59:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
60:     }
61: 
62:     result.append(Format("\n  Expected: %s\n", expectedValue.c_str()));
63: 
64:     return AssertionResult(true, result);
65: }
66: 
67: AssertionResult EqFailure(const string& expectedExpression, const string& actualExpression, const string& expectedValue, const string& actualValue)
68: {
69:     string result = Format("Value of: %s", actualExpression.c_str());
70:     if (actualValue != actualExpression)
71:     {
72:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
73:     }
74: 
75:     result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
76:     if (expectedValue != expectedExpression)
77:     {
78:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
79:     }
80:     result.append("\n");
81: 
82:     return AssertionResult(true, result);
83: }
84: 
85: AssertionResult InEqFailure(const string& expectedExpression, const string& actualExpression, const string& expectedValue, const string& actualValue)
86: {
87:     string result = Format("Value of: %s", actualExpression.c_str());
88:     if (actualValue != actualExpression)
89:     {
90:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
91:     }
92: 
93:     result.append(Format("\n  Expected not equal to: %s", expectedExpression.c_str()));
94:     if (expectedValue != expectedExpression)
95:     {
96:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
97:     }
98:     result.append("\n");
99: 
100:     return AssertionResult(true, result);
101: }
102: 
103: } // namespace unittest
```

- Line 49-52: We implement the function `AssertionSuccess()`. It will return an assertion result flagging no failure
- Line 54-65: We implement the function `BooleanFailure()`. This will return an assertion result flagging a failure, with a string explaining that the actual value does not match the expected value
- Line 67-83: We implement the function `EqFailure()`. This will return an assertion result flagging a failure, with a string explaining that the actual value is not equal to the expected value
- Line 85-101: We implement the function `InEqFailure()`. This will return an assertion result flagging a failure, with a string explaining that the actual value is equal to the expected value

### PrintValue.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_PRINTVALUEH}

The `PrintValue` header contains a lot of template magic to print a value of any type. It is simplified for now, but will be extended in the next step.
I'll try to explain as much as possible. Part of this code is based on how Google Test prints values.

Create the file `code/libraries/unittest/include/unittest/PrintValue.h`

```cpp
File: code/libraries/unittest/include/unittest/PrintValue.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : PrintValue.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : -
9: //
10: // Description : Value printer
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include <baremetal/String.h>
43: #include <baremetal/Serialization.h>
44: 
45: template <typename T>
46: void PrintTo(const T& value, baremetal::string& s)
47: {
48:     s = baremetal::Serialize(value);
49: }
50: 
51: inline void PrintTo(bool x, baremetal::string& s)
52: {
53:     s = (x ? "true" : "false");
54: }
55: 
56: template <typename T>
57: class UniversalPrinter
58: {
59: public:
60:     // Note: we deliberately don't call this PrintTo(), as that name
61:     // conflicts with ::testing::internal::PrintTo in the body of the
62:     // function.
63:     static void Print(const T& value, baremetal::string& s)
64:     {
65:         // By default, ::testing::internal::PrintTo() is used for printing
66:         // the value.
67:         //
68:         // Thanks to Koenig look-up, if T is a class and has its own
69:         // PrintTo() function defined in its namespace, that function will
70:         // be visible here.  Since it is more specific than the generic ones
71:         // in ::testing::internal, it will be picked by the compiler in the
72:         // following statement - exactly what we want.
73:         PrintTo(value, s);
74:     }
75: };
76: 
77: template <typename T>
78: void UniversalPrint(const T& value, baremetal::string& s)
79: {
80:     typedef T T1;
81:     UniversalPrinter<T1>::Print(value, s);
82: }
83: 
84: template <typename T>
85: class UniversalTersePrinter
86: {
87: public:
88:     static void Print(const T& value, baremetal::string& s)
89:     {
90:         UniversalPrint(value, s);
91:     }
92: };
93: template <typename T>
94: class UniversalTersePrinter<T&>
95: {
96: public:
97:     static void Print(const T& value, baremetal::string& s)
98:     {
99:         UniversalPrint(value, s);
100:     }
101: };
102: 
103: template <typename T>
104: inline baremetal::string PrintToString(const T& value)
105: {
106:     baremetal::string s;
107:     UniversalTersePrinter<T>::Print(value, s);
108:     return s;
109: }
```

- Line 46-49: We define a template function `PrintTo()` which is a fallback to print any value to a string using a serializer. This uses the polymorphism of the `Serialize` functions.
- Line 51-54: We define a specialization to `PrintTo()` for boolean values
- Line 56-75: We declare a template class `UniversalPrinter` that has a single method `Print()` which calls any defined `PrintTo()` function for the value passed to a string
- Line 84-82: We define a template function `UniversalPrint` which uses the `Print()` method in the `UniversalPrinter` class
- Line 84-92: We declare a template class `UniversalTersePrinter` for type `T` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string
- Line 93-101: We declare a template class `UniversalTersePrinter` for type `T&` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string
- Line 103-109: We define a template function `PrintToString()` which uses the `Print()` method of any matching class `UniversalTersePrinter`

### AssertMacros.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_ASSERTMACROSH}

Let's define the test macros.

Create the file `code/libraries/unittest/include/unittest/AssertMacros.h`

```cpp
File: code/libraries/unittest/include/unittest/AssertMacros.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : AssertMacros.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : Assertion macros
9: //
10: // Description : Test evaluation & assertion macros
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
38: //------------------------------------------------------------------------------
39: 
40: #pragma once
41: 
42: #include "unittest/Checks.h"
43: #include "unittest/CurrentTest.h"
44: 
45: #ifdef ASSERT_TRUE
46:     #error unittest redefines ASSERT_TRUE
47: #endif
48: 
49: #ifdef ASSERT_FALSE
50:     #error unittest redefines ASSERT_FALSE
51: #endif
52: 
53: #ifdef ASSERT_EQ
54:     #error unittest redefines ASSERT_EQ
55: #endif
56: 
57: #ifdef ASSERT_NE
58:     #error unittest redefines ASSERT_NE
59: #endif
60: 
61: #ifdef EXPECT_TRUE
62:     #error unittest redefines EXPECT_TRUE
63: #endif
64: 
65: #ifdef EXPECT_FALSE
66:     #error unittest redefines EXPECT_FALSE
67: #endif
68: 
69: #ifdef EXPECT_EQ
70:     #error unittest redefines EXPECT_EQ
71: #endif
72: 
73: #ifdef EXPECT_NE
74:     #error unittest redefines EXPECT_NE
75: #endif
76: 
77: #define UT_EXPECT_RESULT(value) \
78:     do \
79:     { \
80:         if (const ::unittest::AssertionResult UT_AssertionResult = (value)) \
81:             ::unittest::CurrentTest::Results()->OnTestFailure(::unittest::TestDetails(*::unittest::CurrentTest::Details(), __LINE__), UT_AssertionResult.message); \
82:     } while (0)
83: #define UT_ASSERT_RESULT(value) \
84:     do \
85:     { \
86:         if (const ::unittest::AssertionResult UT_AssertionResult = (value)) \
87:         { \
88:             ::unittest::CurrentTest::Results()->OnTestFailure(::unittest::TestDetails(*::unittest::CurrentTest::Details(), __LINE__), UT_AssertionResult.message); \
89:             /*throw ::unittest::AssertionFailedException(__FILE__, __LINE__);*/ \
90:         } \
91:     } while (0)
92: 
93: 
94: #define EXPECT_PRED_FORMAT1(pred_format, v1) \
95:   UT_EXPECT_RESULT(pred_format(baremetal::string(#v1), v1))
96: #define ASSERT_PRED_FORMAT1(pred_format, v1) \
97:   UT_ASSERT_RESULT(pred_format(baremetal::string(#v1), v1))
98: 
99: #define EXPECT_PRED_FORMAT2(pred_format, v1, v2) \
100:   UT_EXPECT_RESULT(pred_format(baremetal::string(#v1), baremetal::string(#v2), v1, v2))
101: #define ASSERT_PRED_FORMAT2(pred_format, v1, v2) \
102:   UT_ASSERT_RESULT(pred_format(baremetal::string(#v1), baremetal::string(#v2), v1, v2))
103: 
104: #define FAIL() EXPECT_TRUE(false)
105: #define ASSERT_TRUE(value) \
106:     do \
107:     { \
108:         ASSERT_PRED_FORMAT1(::unittest::CheckTrue, value); \
109:     } while (0)
110: #define EXPECT_TRUE(value) \
111:     do \
112:     { \
113:         EXPECT_PRED_FORMAT1(::unittest::CheckTrue, value); \
114:     } while (0)
115: 
116: #define ASSERT_FALSE(value) \
117:     do \
118:     { \
119:         ASSERT_PRED_FORMAT1(::unittest::CheckFalse, value); \
120:     } while (0)
121: #define EXPECT_FALSE(value) \
122:     do \
123:     { \
124:         EXPECT_PRED_FORMAT1(::unittest::CheckFalse, value); \
125:     } while (0)
126: 
127: #define ASSERT_EQ(expected, actual) \
128:     do \
129:     { \
130:         ASSERT_PRED_FORMAT2(::unittest::EqHelper::CheckEqual, expected, actual); \
131:     } while (0)
132: #define EXPECT_EQ(expected, actual) \
133:     do \
134:     { \
135:         EXPECT_PRED_FORMAT2(::unittest::EqHelper::CheckEqual, expected, actual); \
136:     } while (0)
137: 
138: #define ASSERT_NE(expected, actual) \
139:     do \
140:     { \
141:         ASSERT_PRED_FORMAT2(::unittest::EqHelper::CheckNotEqual, expected, actual); \
142:     } while (0)
143: #define EXPECT_NE(expected, actual) \
144:     do \
145:     { \
146:         EXPECT_PRED_FORMAT2(::unittest::EqHelper::CheckNotEqual, expected, actual); \
147:     } while (0)
```

- Line 45-75: We check whether any of the defines defined in this header are already defined. If so compilation ends with an error
- Line 77-82: We define a macro `UT_EXPECT_RESULT` which checks if the parameter passed (which is of type `AssertionResult`) has a failure (using the `bool()` method).
If there is a failure, a test failure is added to the current result
- Line 83-91: We define a macro `UT_ASSERT_RESULT` which checks if the parameter passed (which is of type `AssertionResult`) has a failure (using the `bool()` method).
If there is a failure, a test failure is added to the current result. Normally an exception would be thrown, however as we don't have exceptions enabled yet, that will wait until later
- Line 94-95: We define a macro `EXPECT_PRED_FORMAT1` which is passed a check function and a single parameter.
The check function is called, and the result is passed to `UT_EXPECT_RESULT`.
The single parameter version is used with `CheckTrue()` or `CheckFalse` to check the result of a boolean expression
- Line 96-97: We define a macro `ASSERT_PRED_FORMAT1` which is passed a check function and a single parameter.
The check function is called, and the result is passed to `UT_ASSERT_RESULT`.
The single parameter version is used with `CheckTrue()` or `CheckFalse` to check the result of a boolean expression
- Line 99-100: We define a macro `EXPECT_PRED_FORMAT2` which is passed a check function and two parameters.
The check function is called, and the result is passed to `UT_EXPECT_RESULT`.
The two parameter version is used with `EqHelper::CheckEqual()` or `EqHelper::CheckNotEqual()` to check the result of a comparison expression
- Line 99-100: We define a macro `ASSERT_PRED_FORMAT2` which is passed a check function and two parameters.
The check function is called, and the result is passed to `UT_ASSERT_RESULT`.
The two parameter version is used with `EqHelper::CheckEqual()` or `EqHelper::CheckNotEqual()` to check the result of a comparison expression
- Line 104: We define a macro `FAIL` which returns a simple failure
- Line 105-109: We define a macro `ASSERT_TRUE` which uses `ASSERT_PRED_FORMAT1` with `CheckTrue` to check if the parameter is true, and generate a failure if the check fails
- Line 110-114: We define a macro `EXPECT_TRUE` which uses `EXPECT_PRED_FORMAT1` with `CheckTrue` to check if the parameter is true, and generate a failure if the check fails
- Line 116-120: We define a macro `ASSERT_FALSE` which uses `ASSERT_PRED_FORMAT1` with `CheckFalse` to check if the parameter is false, and generate a failure if the check fails
- Line 121-125: We define a macro `EXPECT_FALSE` which uses `EXPECT_PRED_FORMAT1` with `CheckFalse` to check if the parameter is false, and generate a failure if the check fails
- Line 127-131: We define a macro `ASSERT_EQ` which uses `ASSERT_PRED_FORMAT2` with `EqHelper::CheckEqual` to check if the parameters are equal, and generate a failure if the check fails
- Line 132-136: We define a macro `EXPECT_EQ` which uses `EXPECT_PRED_FORMAT2` with `EqHelper::CheckEqual` to check if the parameters are equal, and generate a failure if the check fails
- Line 138-142: We define a macro `ASSERT_NE` which uses `ASSERT_PRED_FORMAT2` with `EqHelper::CheckNotEqual` to check if the parameters are notequal, and generate a failure if the check fails
- Line 143-147: We define a macro `EXPECT_NE` which uses `EXPECT_PRED_FORMAT2` with `EqHelper::CheckNotEqual` to check if the parameters are not equal, and generate a failure if the check fails

### TestDetails.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_TESTDETAILSH}

We need to add a new constructor for the test macros. In lines 81 and 88 of `AssertMacros.h` we use a constructor which resembles a copy constructor, but which also takes the line number of the assertion 

Update the file `code/libraries/unittest/include/unittest/TestDetails.h`

```cpp
File: code/libraries/unittest/include/unittest/TestDetails.h
...
47: class TestDetails
48: {
...
56: public:
57:     TestDetails();
58:     TestDetails(const baremetal::string& testName, const baremetal::string& fixtureName, const baremetal::string& suiteName, const baremetal::string& fileName, int lineNumber);
59:     TestDetails(const TestDetails& other, int lineNumber);
...
66: };
67: 
68: } // namespace unittest
```

- Line 59: We declare the new constructor which take an existing `TestDetails` instance as well as a line number

### TestDetails.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_TESTDETAILSCPP}

We'll implement the new constructor.

Create the file `code/libraries/unittest/src/TestDetails.cpp`

```cpp
File: code/libraries/unittest/src/TestDetails.cpp
File: d:\Projects\baremetal.github\code\libraries\unittest\src\TestDetails.cpp
...
55: TestDetails::TestDetails(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
56:     : m_suiteName{ suiteName }
57:     , m_fixtureName{ fixtureName }
58:     , m_testName{ testName }
59:     , m_fileName{ fileName }
60:     , m_lineNumber{ lineNumber }
61: {
62: }
63: 
64: TestDetails::TestDetails(const TestDetails& other, int lineNumber)
65:     : m_suiteName{ other.m_suiteName }
66:     , m_fixtureName{ other.m_fixtureName }
67:     , m_testName{ other.m_testName }
68:     , m_fileName{ other.m_fileName }
69:     , m_lineNumber{ lineNumber }
70: {
71: }
72: 
73: } // namespace unittest
```

### unittest.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_UNITTESTH}

We added a header file, so we'll update the `unittest.h` header

Update the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
...
42: #include <unittest/TestFixture.h>
43: #include <unittest/TestSuite.h>
44: 
45: #include <unittest/ITestReporter.h>
46: #include <unittest/AssertMacros.h>
47: #include <unittest/ConsoleTestReporter.h>
48: #include <unittest/ExecuteTest.h>
49: #include <unittest/TestBase.h>
50: #include <unittest/TestDetails.h>
51: #include <unittest/TestFixtureInfo.h>
52: #include <unittest/TestMacros.h>
53: #include <unittest/TestRegistry.h>
54: #include <unittest/TestResults.h>
55: #include <unittest/TestRunner.h>
56: #include <unittest/TestSuiteInfo.h>
```

### Update project configuration {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Checks.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleTestReporter.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/DeferredTestReporter.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResult.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRunner.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
43:     )
44: 
45: set(PROJECT_INCLUDES_PUBLIC
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/AssertMacros.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Checks.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ConsoleTestReporter.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/DeferredTestReporter.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ExecuteTest.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ITestReporter.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/PrintValue.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestMacros.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResult.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRunner.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/unittest.h
66:     )
67: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_APPLICATION_CODE}

Now let's start using the macros we defined.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/BCMRegisters.h>
4: #include <baremetal/Console.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/Mailbox.h>
7: #include <baremetal/MemoryManager.h>
8: #include <baremetal/New.h>
9: #include <baremetal/RPIProperties.h>
10: #include <baremetal/Serialization.h>
11: #include <baremetal/String.h>
12: #include <baremetal/SysConfig.h>
13: #include <baremetal/System.h>
14: #include <baremetal/Timer.h>
15: #include <baremetal/Util.h>
16: 
17: #include <unittest/unittest.h>
18: 
19: LOG_MODULE("main");
20: 
21: using namespace baremetal;
22: using namespace unittest;
23: 
24: TEST_SUITE(Suite1)
25: {
26: 
27: class FixtureMyTest1
28:     : public TestFixture
29: {
30: public:
31:     void SetUp() override
32:     {
33:         LOG_DEBUG("FixtureMyTest1 SetUp");
34:     }
35:     void TearDown() override
36:     {
37:         LOG_DEBUG("FixtureMyTest1 TearDown");
38:     }
39: };
40: 
41: TEST_FIXTURE(FixtureMyTest1,Test1)
42: {
43:     FAIL();
44: }
45: 
46: } // Suite1
47: 
48: TEST_SUITE(Suite2)
49: {
50: 
51: class FixtureMyTest2
52:     : public TestFixture
53: {
54: public:
55:     void SetUp() override
56:     {
57:         LOG_DEBUG("FixtureMyTest2 SetUp");
58:     }
59:     void TearDown() override
60:     {
61:         LOG_DEBUG("FixtureMyTest2 TearDown");
62:     }
63: };
64: 
65: TEST_FIXTURE(FixtureMyTest2, Test2)
66: {
67:     EXPECT_TRUE(true);
68:     EXPECT_FALSE(false);
69:     EXPECT_TRUE(false);
70:     EXPECT_FALSE(true);
71: }
72: 
73: } // Suite2
74: 
75: class FixtureMyTest3
76:     : public TestFixture
77: {
78: public:
79:     void SetUp() override
80:     {
81:         LOG_DEBUG("FixtureMyTest3 SetUp");
82:     }
83:     void TearDown() override
84:     {
85:         LOG_DEBUG("FixtureMyTest3 TearDown");
86:     }
87: };
88: 
89: TEST_FIXTURE(FixtureMyTest3, Test3)
90: {
91:     int x = 0;
92:     int y = 1;
93:     int z = 1;
94:     EXPECT_EQ(x, y);
95:     EXPECT_EQ(y, z);
96:     EXPECT_NE(x, y);
97:     EXPECT_NE(y, z);
98: }
99: 
100: TEST(Test4)
101: {
102:     ASSERT_TRUE(false);
103: }
104: 
105: int main()
106: {
107:     auto& console = GetConsole();
108:     LOG_DEBUG("Hello World!");
109: 
110:     ConsoleTestReporter reporter;
111:     auto numFailures = RunAllTests(&reporter);
112:     LOG_INFO("Failures found: %d", numFailures);
113: 
114:     LOG_INFO("Wait 5 seconds");
115:     Timer::WaitMilliSeconds(5000);
116: 
117:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
118:     char ch{};
119:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
120:     {
121:         ch = console.ReadChar();
122:         console.WriteChar(ch);
123:     }
124:     if (ch == 'p')
125:         assert(false);
126: 
127:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
128: }
```

- Line 41-44: We change the `Test1` function to a call to `FAIL()` this will generate a simple failure
- Line 65-71: We change the `Test2` function to boolean checks. Obviously, `EXPECT_TRUE(false)` and `EXPECT_FALSE(true)` will fail.
- Line 89-98: We change the `Test3` function to equality checks. Obviously, `EXPECT_EQ(x, y)` and ` EXPECT_NE(y, z)` will fail.
- Line 100-103: We change the `Test4` function to a failed assertion.
As said before, this should throw an exception, but for now this is commented out, so it will result in a normal failure, and the test run will continue

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACROS__STEP_8_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Debug  Register test Test1 in fixture FixtureMyTest1 in suite Suite1 (TestRegistry:122)
Debug  Find suite Suite1 ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest1 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test2 in fixture FixtureMyTest2 in suite Suite2 (TestRegistry:122)
Debug  Find suite Suite2 ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest2 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test3 in fixture FixtureMyTest3 in suite DefaultSuite (TestRegistry:122)
Debug  Find suite DefaultSuite ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest3 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test4 in fixture DefaultFixture in suite DefaultSuite (TestRegistry:122)
Debug  Find suite DefaultSuite ... found (TestRegistry:93)
Debug  Fixture DefaultFixture not found, creating new object (TestSuiteInfo:79)
Info   Starting up (System:201)
Debug  Hello World! (main:108)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
[  FAILED   ] Suite1::FixtureMyTest1::Test1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
[  FAILED   ] Suite2::FixtureMyTest2::Test2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
[  FAILED   ] FixtureMyTest3::Test3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
[  FAILED   ] Test4
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 4 out of 4 tests failed (6 failures).

Failures:
../code/applications/demo/src/main.cpp:43 : Failure in Suite1::FixtureMyTest1::Test1: Value of: false
  Expected: true

../code/applications/demo/src/main.cpp:69 : Failure in Suite2::FixtureMyTest2::Test2: Value of: false
  Expected: true

../code/applications/demo/src/main.cpp:70 : Failure in Suite2::FixtureMyTest2::Test2: Value of: true
  Expected: false

../code/applications/demo/src/main.cpp:94 : Failure in FixtureMyTest3::Test3: Value of: y
  Actual: 1
  Expected: x
  Which is: 0

../code/applications/demo/src/main.cpp:97 : Failure in FixtureMyTest3::Test3: Value of: z
  Actual: 1
  Expected not equal to: y
  Which is: 1

../code/applications/demo/src/main.cpp:102 : Failure in Test4: Value of: false
  Expected: true


[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   Failures found: 6 (main:112)
Info   Wait 5 seconds (main:114)
Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

## Test assert macro extension - Step 9 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACRO_EXTENSION__STEP_9}

We can now perform boolean checks and compare integers, but we would also like to be able to compare pointers and strings.
So we'll extend the macros a bit.

### Checks.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACRO_EXTENSION__STEP_9_CHECKSH}

We need to extend the utility functions a bit.

Update the file `code/libraries/unittest/include/unittest/Checks.h`

```cpp
File: code/libraries/unittest/include/unittest/Checks.h
...
76: extern AssertionResult CloseFailure(const baremetal::string& expectedExpression,
77:                                     const baremetal::string& actualExpression,
78:                                     const baremetal::string& toleranceExpression,
79:                                     const baremetal::string& expectedValue,
80:                                     const baremetal::string& actualValue,
81:                                     const baremetal::string& toleranceValue);
82: 
...
167: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
168:                                    char const* expected, char const* actual);
169: 
170: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
171:                                    char* expected, char* actual);
172: 
173: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
174:                                    char* expected, char const* actual);
175: 
176: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
177:                                    char const* expected, char* actual);
178: 
179: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
180:                                       char const* expected, char const* actual);
181: 
182: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
183:                                       char* expected, char* actual);
184: 
185: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
186:                                       char* expected, char const* actual);
187: 
188: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
189:                                       char const* expected, char* actual);
190: 
191: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
192:                                    const baremetal::string& actualExpression,
193:                                    const baremetal::string& expected,
194:                                    const baremetal::string& actual);
195: 
196: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
197:                                    const baremetal::string& actualExpression,
198:                                    const baremetal::string& expected,
199:                                    const char* actual);
200: 
201: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
202:                                    const baremetal::string& actualExpression,
203:                                    const char* expected,
204:                                    const baremetal::string& actual);
205: 
206: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
207:                                       const baremetal::string& actualExpression,
208:                                       const baremetal::string& expected,
209:                                       const baremetal::string& actual);
210: 
211: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
212:                                       const baremetal::string& actualExpression,
213:                                       const baremetal::string& expected,
214:                                       const char* actual);
215: 
216: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
217:                                       const baremetal::string& actualExpression,
218:                                       const char* expected,
219:                                       const baremetal::string& actual);
220: 
221: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
222:                                              char const* expected, char const* actual);
223: 
224: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
225:                                              char* expected, char* actual);
226: 
227: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
228:                                              char* expected, char const* actual);
229: 
230: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
231:                                              char const* expected, char* actual);
232: 
233: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
234:                                                 char const* expected, char const* actual);
235: 
236: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
237:                                                 char* expected, char* actual);
238: 
239: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
240:                                                 char* expected, char const* actual);
241: 
242: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
243:                                                 char const* expected, char* actual);
244: 
245: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
246:                                              const baremetal::string& actualExpression,
247:                                              const baremetal::string& expected,
248:                                              const baremetal::string& actual);
249: 
250: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
251:                                              const baremetal::string& actualExpression,
252:                                              const baremetal::string& expected,
253:                                              const char* actual);
254: 
255: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
256:                                              const baremetal::string& actualExpression,
257:                                              const char* expected,
258:                                              const baremetal::string& actual);
259: 
260: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
261:                                                 const baremetal::string& actualExpression,
262:                                                 const baremetal::string& expected,
263:                                                 const baremetal::string& actual);
264: 
265: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
266:                                                 const baremetal::string& actualExpression,
267:                                                 const baremetal::string& expected,
268:                                                 const char* actual);
269: 
270: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
271:                                                 const baremetal::string& actualExpression,
272:                                                 const char* expected,
273:                                                 const baremetal::string& actual);
274: 
275: // The helper class for {ASSERT|EXPECT}_EQ.  The template argument
276: // lhs_is_null_literal is true iff the first argument to ASSERT_EQ()
277: // is a null pointer literal.  The following default implementation is
278: // for lhs_is_null_literal being false.
279: template <bool lhs_is_null_literal>
280: class EqHelper
281: {
282: public:
283:     // This templatized version is for the general case.
284:     template <typename Expected, typename Actual>
285:     static AssertionResult CheckEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
286:                                       const Expected& expected, const Actual& actual)
287:     {
288:         return CheckEqualInternal(expectedExpression, actualExpression, expected, actual);
289:     }
290: 
291:     template <typename Expected, typename Actual>
292:     static AssertionResult CheckNotEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
293:                                          const Expected& expected, const Actual& actual)
294:     {
295:         return CheckNotEqualInternal(expectedExpression, actualExpression, expected, actual);
296:     }
297: };
298: 
299: class EqHelperStringCaseInsensitive
300: {
301: public:
302:     template <typename Expected, typename Actual>
303:     static AssertionResult CheckEqualIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
304:                                                 const Expected& expected, const Actual& actual)
305:     {
306:         return CheckEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
307:     }
308: 
309:     template <typename Expected, typename Actual>
310:     static AssertionResult CheckNotEqualIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
311:                                                    const Expected& expected, const Actual& actual)
312:     {
313:         return CheckNotEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
314:     }
315: };
316: 
317: template< typename Expected, typename Actual, typename Tolerance >
318: bool AreClose(const Expected& expected, const Actual& actual, Tolerance const& tolerance)
319: {
320:     return (actual >= (expected - tolerance)) && (actual <= (expected + tolerance));
321: }
322: 
323: template< typename Expected, typename Actual, typename Tolerance >
324: AssertionResult CheckClose(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
325:                            const baremetal::string& toleranceExpression,
326:                            const Expected& expected, const Actual& actual, Tolerance const& tolerance)
327: {
328:     if (!AreClose(expected, actual, tolerance))
329:     {
330:         return CloseFailure(expectedExpression, actualExpression, toleranceExpression,
331:                             FormatForComparisonFailureMessage(expected),
332:                             FormatForComparisonFailureMessage(actual),
333:                             FormatForComparisonFailureMessage(tolerance));
334:     }
335:     return AssertionSuccess();
336: }
337: 
338: } // namespace unittest
```

- Line 76-81: We declare a function `CloseFailure()` to signify a assertion failure on comparison between numbers with a fault tolerance
- Line 167-168: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 170-171: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 173-174: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 176-177: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 179-180: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 182-183: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 185-186: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 188-189: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 191-194: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 196-199: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 201-204: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 206-209: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 211-214: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 216-219: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 221-222: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 224-225: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 227-228: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 230-231: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 233-234: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 236-237: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 239-240: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 242-243: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 245-248: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 250-253: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 255-258: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 260-263: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 265-268: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 270-273: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 279: We change the class `EqHelper` into a template class that receives a template parameter signifying whether the expected value is a null pointer
- Line 299-315: We declare a class `EqHelperCaseInsensitive` which has two methods `CheckEqualIgnoreCase()` (which uses `CheckEqualInternalIgnoreCase()`) and `CheckNotEqualIgnoreCase()` (which uses `CheckNotEqualInternalIgnoreCase()`)
- Line 318-321: We define a template function `AreClose()` to compare two values `expected` and `actual`, with a tolerance `tolerance`, which can have different types.
The function compares the absolute difference between `expected` and `actual` with the given tolerance, It returns true of the difference is smaller or equal to the tolerance, false otherwise
- Line 323-336: We define a template function `CheckClose()`, which uses `AreClose()` to compare two values `expected` and `actual`. Their stringified versions are passed as `expectedExpression` and `actualExpression` with tolerance `tolerance`.
If the absolute difference between `expected` and `actual` is smaller or equal to `tolerance`, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned

### Checks.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACRO_EXTENSION__STEP_9_CHECKSCPP}

Let's implement the new functions for the checks.

Update the file `code/libraries/unittest/src/Checks.cpp`

```cpp
File: code/libraries/unittest/src/Checks.cpp
...
47: namespace unittest {
48: 
49: static bool EqualCaseInsensitive(const string& a, const string& b)
50: {
51:     if (a.length() != b.length())
52:         return false;
53:     return strcasecmp(a.data(), b.data()) == 0;
54: }
55: 
...
110: AssertionResult CloseFailure(const string& expectedExpression,
111:                              const string& actualExpression,
112:                              const string& toleranceExpression,
113:                              const string& expectedValue,
114:                              const string& actualValue,
115:                              const string& toleranceValue)
116: {
117:     string result = Format("Value of: %s", actualExpression.c_str());
118:     if (actualValue != actualExpression)
119:     {
120:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
121:     }
122: 
123:     result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
124:     if (expectedValue != expectedExpression)
125:     {
126:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
127:     }
128:     result.append(Format("\n  Tolerance: %s", toleranceExpression.c_str()));
129:     if (toleranceValue != toleranceExpression)
130:     {
131:         result.append(Format("\n  (+/-) %s", toleranceValue.c_str()));
132:     }
133: 
134:     return AssertionResult(true, result);
135: }
136: 
137: namespace internal {
138: 
139: AssertionResult CheckStringsEqual(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
140: {
141:     if (expected == actual)
142:         return AssertionSuccess();
143: 
144:     if (strcmp(expected, actual))
145:     {
146:         return EqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
147:     }
148:     return AssertionSuccess();
149: }
150: 
151: AssertionResult CheckStringsNotEqual(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
152: {
153:     if (expected == actual)
154:         return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
155: 
156:     if (!strcmp(expected, actual))
157:     {
158:         return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
159:     }
160:     return AssertionSuccess();
161: }
162: 
163: AssertionResult CheckStringsEqualIgnoreCase(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
164: {
165:     if (expected == actual)
166:         return AssertionSuccess();
167: 
168:     if (!EqualCaseInsensitive(baremetal::string(expected), baremetal::string(actual)))
169:     {
170:         return EqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
171:     }
172:     return AssertionSuccess();
173: }
174: 
175: AssertionResult CheckStringsNotEqualIgnoreCase(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
176: {
177:     if (expected == actual)
178:         return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
179: 
180:     if (EqualCaseInsensitive(baremetal::string(expected), baremetal::string(actual)))
181:     {
182:         return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
183:     }
184:     return AssertionSuccess();
185: }
186: 
187: } // namespace internal
188: 
189: AssertionResult CheckEqualInternal(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
190: {
191:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
192: }
193: 
194: AssertionResult CheckEqualInternal(const string& expectedExpression, const string& actualExpression, char *expected,
195:                                    char *actual) // cppcheck-suppress constParameterPointer
196: {
197:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
198: }
199: 
200: AssertionResult CheckEqualInternal(const string& expectedExpression, const string& actualExpression, char *expected,
201:                                    char const *actual) // cppcheck-suppress constParameterPointer
202: {
203:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
204: }
205: 
206: AssertionResult CheckEqualInternal(const string& expectedExpression, const string& actualExpression, char const *expected,
207:                                    char *actual) // cppcheck-suppress constParameterPointer
208: {
209:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
210: }
211: 
212: AssertionResult CheckNotEqualInternal(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
213: {
214:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
215: }
216: 
217: AssertionResult CheckNotEqualInternal(const string& expectedExpression, const string& actualExpression, char *expected,
218:                                       char *actual) // cppcheck-suppress constParameterPointer
219: {
220:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
221: }
222: 
223: AssertionResult CheckNotEqualInternal(const string& expectedExpression, const string& actualExpression, char *expected,
224:                                       char const *actual) // cppcheck-suppress constParameterPointer
225: {
226:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
227: }
228: 
229: AssertionResult CheckNotEqualInternal(const string& expectedExpression, const string& actualExpression, char const *expected,
230:                                       char *actual) // cppcheck-suppress constParameterPointer
231: {
232:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
233: }
234: 
235: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
236:                                    const baremetal::string& actualExpression,
237:                                    const baremetal::string& expected,
238:                                    const baremetal::string& actual)
239: {
240:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
241: }
242: 
243: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
244:                                    const baremetal::string& actualExpression,
245:                                    const baremetal::string& expected,
246:                                    const char* actual)
247: {
248:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
249: }
250: 
251: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
252:                                    const baremetal::string& actualExpression,
253:                                    const char* expected,
254:                                    const baremetal::string& actual)
255: {
256:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
257: }
258: 
259: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
260:                                       const baremetal::string& actualExpression,
261:                                       const baremetal::string& expected,
262:                                       const baremetal::string& actual)
263: {
264:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
265: }
266: 
267: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
268:                                       const baremetal::string& actualExpression,
269:                                       const baremetal::string& expected,
270:                                       const char* actual)
271: {
272:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
273: }
274: 
275: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
276:                                       const baremetal::string& actualExpression,
277:                                       const char* expected,
278:                                       const baremetal::string& actual)
279: {
280:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
281: }
282: 
283: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
284:                                              char const* expected, char const* actual)
285: {
286:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
287: }
288: 
289: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
290:                                              char* expected, char* actual)
291: {
292:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
293: }
294: 
295: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
296:                                              char* expected, char const* actual)
297: {
298:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
299: }
300: 
301: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
302:                                              char const* expected, char* actual)
303: {
304:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
305: }
306: 
307: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
308:                                                 char const* expected, char const* actual)
309: {
310:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
311: }
312: 
313: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
314:                                                 char* expected, char* actual)
315: {
316:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
317: }
318: 
319: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
320:                                                 char* expected, char const* actual)
321: {
322:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
323: }
324: 
325: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
326:                                                 char const* expected, char* actual)
327: {
328:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
329: }
330: 
331: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
332:                                              const baremetal::string& actualExpression,
333:                                              const baremetal::string& expected,
334:                                              const baremetal::string& actual)
335: {
336:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
337: }
338: 
339: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
340:                                              const baremetal::string& actualExpression,
341:                                              const baremetal::string& expected,
342:                                              const char* actual)
343: {
344:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
345: }
346: 
347: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
348:                                              const baremetal::string& actualExpression,
349:                                              const char* expected,
350:                                              const baremetal::string& actual)
351: {
352:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
353: }
354: 
355: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
356:                                                 const baremetal::string& actualExpression,
357:                                                 const baremetal::string& expected,
358:                                                 const baremetal::string& actual)
359: {
360:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
361: }
362: 
363: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
364:                                                 const baremetal::string& actualExpression,
365:                                                 const baremetal::string& expected,
366:                                                 const char* actual)
367: {
368:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
369: }
370: 
371: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
372:                                                 const baremetal::string& actualExpression,
373:                                                 const char* expected,
374:                                                 const baremetal::string& actual)
375: {
376:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
377: }
378: 
379: } // namespace unittest
```

- Line 49-54: We define a local function `EqualCaseInsensitive` which compares two strings in a case insensitive way, and returns true if they are considered equal, false otherwise
- Line 110-135: We implement the function `CloseFailure()`. This will return an assertion result flagging a failure, with a string explaining that the absolute difference between the actual value and the expected value is larger than the tolerance
- Line 139-149: We define a function `CheckStringsEqual()` in a local namespace `internal`.
If the two values of type const char* are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 151-161: We define a function `CheckStringsNotEqual()` in a local namespace `internal`.
If the two values of type const char* are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 139-149: We define a function `CheckStringsEqualIgnoreCase()` in a local namespace `internal`.
If the two values of type const char* are considered equal ignoring case, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 151-161: We define a function `CheckStringsNotEqualIgnoreCase()` in a local namespace `internal`.
If the two values of type const char* are considered not equal ignoring case, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 189-192: We implement the function `CheckEqualInternal()` for two value of type const char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 194-198: We implement the function `CheckEqualInternal()` for two value of type char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 200-204: We implement the function `CheckEqualInternal()` for two value of type char* and const char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 206-210: We implement the function `CheckEqualInternal()` for two value of type const char* and char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 212-215: We implement the function `CheckNotEqualInternal()` for two value of type const char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 217-221: We implement the function `CheckNotEqualInternal()` for two value of type char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 223-227: We implement the function `CheckNotEqualInternal()` for two value of type char* and const char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 229-233: We implement the function `CheckNotEqualInternal()` for two value of type const char* and char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 235-241: We implement the function `CheckEqualInternal()` for two value of type string.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 243-249: We implement the function `CheckEqualInternal()` for two value of type char* and string.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 251-257: We implement the function `CheckEqualInternal()` for two value of type string and const char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 259-265: We implement the function `CheckNotEqualInternal()` for two value of type string.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 267-273: We implement the function `CheckNotEqualInternal()` for two value of type char* and string.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 275-281: We implement the function `CheckNotEqualInternal()` for two value of type string and const char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 283-287: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type const char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 289-293: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 295-299: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type char* and const char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 301-305: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type const char* and char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 307-311: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type const char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 313-317: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 319-323: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type char* and const char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 325-329: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type const char* and char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 331-337: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type string.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 339-345: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type char* and string.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 347-353: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type string and const char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 355-361: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type string.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 363-369: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type char* and string.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 371-377: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type string and const char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings

### PrintValue.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACRO_EXTENSION__STEP_9_PRINTVALUEH}

The `PrintValue` header contains a lot of template magic to print a value of any type. It is simplified for now, but will be extended in the next step.
I'll try to explain as much as possible. Part of this code is based on how Google Test prints values.

Update the file `code/libraries/unittest/include/unittest/PrintValue.h`

```cpp
File: code/libraries/unittest/include/unittest/PrintValue.h
...
45: using nullptr_t = decltype(nullptr);
46: 
47: template<typename To>
48: inline To ImplicitCast_(To x) { return x; }
49: 
50: template <typename T>
51: void PrintTo(const T& value, baremetal::string& s)
52: {
53:     s = baremetal::Serialize(value);
54: }
55: void PrintTo(unsigned char c, baremetal::string& s);
56: void PrintTo(signed char c, baremetal::string& s);
57: inline void PrintTo(char c, baremetal::string& s)
58: {
59:     PrintTo(static_cast<unsigned char>(c), s);
60: }
61: 
62: inline void PrintTo(bool x, baremetal::string& s)
63: {
64:     s = (x ? "true" : "false");
65: }
66: 
67: void PrintTo(const char* str, baremetal::string& s);
68: inline void PrintTo(char* str, baremetal::string& s)
69: {
70:     PrintTo(ImplicitCast_<const char*>(str), s);
71: }
72: 
73: // signed/unsigned char is often used for representing binary data, so
74: // we print pointers to it as void* to be safe.
75: inline void PrintTo(const signed char* str, baremetal::string& s)
76: {
77:     PrintTo(ImplicitCast_<const void*>(str), s);
78: }
79: inline void PrintTo(signed char* str, baremetal::string& s)
80: {
81:     PrintTo(ImplicitCast_<const void*>(str), s);
82: }
83: inline void PrintTo(const unsigned char* str, baremetal::string& s)
84: {
85:     PrintTo(ImplicitCast_<const void*>(str), s);
86: }
87: inline void PrintTo(unsigned char* str, baremetal::string& s)
88: {
89:     PrintTo(ImplicitCast_<const void*>(str), s);
90: }
91: 
92: void PrintStringTo(const baremetal::string& str, baremetal::string& s);
93: inline void PrintTo(const baremetal::string& str, baremetal::string& s)
94: {
95:     PrintStringTo(str, s);
96: }
97: 
98: inline void PrintTo(nullptr_t /*p*/, baremetal::string& s)
99: {
100:     PrintStringTo(baremetal::string("null"), s);
101: }
102: 
...
124: // Implements printing a reference type T&.
125: template <typename T>
126: class UniversalPrinter<T&>
127: {
128: public:
129:     static void Print(const T& value, baremetal::string& s)
130:     {
131:         // Prints the address of the value.  We use reinterpret_cast here
132:         // as static_cast doesn't compile when T is a function type.
133:         s = "@";
134:         s.append(baremetal::Serialize(reinterpret_cast<const void*>(&value)));
135:         s.append(" ");
136: 
137:         // Then prints the value itself.
138:         PrintTo(value, s);
139:     }
140: };
141: 
142: template <typename T>
143: void UniversalPrint(const T& value, baremetal::string& s)
144: {
145:     typedef T T1;
146:     UniversalPrinter<T1>::Print(value, s);
147: }
148: 
149: template <typename T>
150: class UniversalTersePrinter
151: {
152: public:
153:     static void Print(const T& value, baremetal::string& s)
154:     {
155:         UniversalPrint(value, s);
156:     }
157: };
158: template <typename T>
159: class UniversalTersePrinter<T&>
160: {
161: public:
162:     static void Print(const T& value, baremetal::string& s)
163:     {
164:         UniversalPrint(value, s);
165:     }
166: };
167: template <>
168: class UniversalTersePrinter<const char*>
169: {
170: public:
171:     static void Print(const char* str, baremetal::string& s)
172:     {
173:         if (str == nullptr)
174:         {
175:             s = "null";
176:         }
177:         else
178:         {
179:             UniversalPrint(baremetal::string(str), s);
180:         }
181:     }
182: };
183: template <>
184: class UniversalTersePrinter<char*>
185: {
186: public:
187:     // cppcheck-suppress constParameterPointer
188:     static void Print(char* str, baremetal::string& s)
189:     {
190:         UniversalTersePrinter<const char*>::Print(str, s);
191:     }
192: };
193: 
...
```

- Line 45: We define a type `nullptr_t` to be used to check for nullptr
- Line 47-48: We define a template function `ImplicitCast_` that casts any type to itself
- Line 55: We declare a function `PrintTo()` to print an unsigned char to string
- Line 56: We declare a function `PrintTo()` to print a signed char to string
- Line 57: We define a function `PrintTo()` to print a character to string using the function for unsigned char
- Line 67: We declare a function `PrintTo()` to print a const char* string to string
- Line 68-71: We define a function `PrintTo()` to print a char* string to string using the print function for const char*
- Line 75-78: We define a function `PrintTo()` to print a const signed char* pointer to string using the print function for const void*
- Line 79-82: We define a function `PrintTo()` to print a signed char* pointer to string using the print function for const void*
- Line 83-86: We define a function `PrintTo()` to print a const unsigned char* pointer to string using the print function for const void*
- Line 87-90: We define a function `PrintTo()` to print a unsigned char* pointer to string using the print function for const void*
- Line 92: We declare a function `PrintStringTo()` to print a string to a string
- Line 93-96: We define a function `PrintTo()` to print a string to a string
- Line 98-101: We define a function `PrintTo()` to print a nullptr to a string
- Line 125-140: We declare a template class `UniversalPrinter` for type `T&` that has a single method `Print()` which calls any defined `PrintTo()` function for the value passed to a string, with a prefix to display the address of the string
- Line 167-182: We declare a template class `UniversalTersePrinter` specialization for type `const char*` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string
- Line 183-192: We declare a template class `UniversalTersePrinter` specialization for type `char*` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string

### PrintValue.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACRO_EXTENSION__STEP_9_PRINTVALUECPP}

Some print functions need to be implemented.

Create the file `code/libraries/unittest/src/PrintValue.cpp`

```cpp
File: code/libraries/unittest/src/PrintValue.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : PrintValue.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : -
9: //
10: // Description : Print values
11: //------------------------------------------------------------------------------
12: //
13: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
14: //
15: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
16: //
17: // Permission is hereby granted, free of charge, to any person
18: // obtaining a copy of this software and associated documentation
19: // files(the "Software"), to deal in the Software without
20: // restriction, including without limitation the rights to use, copy,
21: // modify, merge, publish, distribute, sublicense, and /or sell copies
22: // of the Software, and to permit persons to whom the Software is
23: // furnished to do so, subject to the following conditions :
24: //
25: // The above copyright notice and this permission notice shall be
26: // included in all copies or substantial portions of the Software.
27: //
28: // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
29: // EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
30: // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
31: // NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
32: // HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
33: // WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
34: // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
35: // DEALINGS IN THE SOFTWARE.
36: //
37: //------------------------------------------------------------------------------
38: 
39: #include <unittest/PrintValue.h>
40: 
41: using namespace baremetal;
42: 
43: void PrintStringTo(const baremetal::string& str, baremetal::string& s)
44: {
45:     s = str;
46: }
47: 
48: void PrintTo(unsigned char ch, baremetal::string &s)
49: {
50:     s += ch;
51: }
```

The functions speak for themselves.

### AssertMacros.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACRO_EXTENSION__STEP_9_ASSERTMACROSH}

We need to extend the assertion macros a bit.

Update the file `code/libraries/unittest/include/unittest/AssertMacros.h`

```cpp
File: code/libraries/unittest/include/unittest/AssertMacros.h
...
61: #ifdef ASSERT_NEAR
62:     #error unittest redefines ASSERT_NEAR
63: #endif
64: 
...
81: #ifdef EXPECT_NEAR
82:     #error unittest redefines EXPECT_NEAR
83: #endif
84: 
85: namespace unittest
86: {
87: 
88: namespace internal
89: {
90: 
91: // Two overloaded helpers for checking at compile time whether an
92: // expression is a null pointer literal (i.e. nullptr or any 0-valued
93: // compile-time integral constant).  Their return values have
94: // different sizes, so we can use sizeof() to test which version is
95: // picked by the compiler.  These helpers have no implementations, as
96: // we only need their signatures.
97: //
98: // Given IsNullLiteralHelper(x), the compiler will pick the first
99: // version if x can be implicitly converted to Secret*, and pick the
100: // second version otherwise.  Since Secret is a secret and incomplete
101: // type, the only expression a user can write that has type Secret* is
102: // a null pointer literal.  Therefore, we know that x is a null
103: // pointer literal if and only if the first version is picked by the
104: // compiler.
105: class Secret;
106: char IsNullLiteralHelper(Secret* p);
107: char (&IsNullLiteralHelper(...))[2];
108: 
109: } // namespace internal
110: 
111: } // namespace unittest
112: 
113: #define IS_NULL_LITERAL(x) \
114:      (sizeof(::unittest::internal::IsNullLiteralHelper(x)) == 1)
115: 
...
143: #define EXPECT_PRED_FORMAT3(pred_format, v1, v2, v3) \
144:   UT_EXPECT_RESULT(pred_format(baremetal::string(#v1), baremetal::string(#v2), baremetal::string(#v3), v1, v2, v3))
145: #define ASSERT_PRED_FORMAT3(pred_format, v1, v2, v3) \
146:   UT_ASSERT_RESULT(pred_format(baremetal::string(#v1), baremetal::string(#v2), baremetal::string(#v3), v1, v2, v3))
147: 
...
File: d:\Projects\baremetal.github\code\libraries\unittest\include\unittest\AssertMacros.h
171: #define ASSERT_EQ(expected, actual) \
172:     do \
173:     { \
174:         ASSERT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckEqual, expected, actual); \
175:     } while (0)
176: #define EXPECT_EQ(expected, actual) \
177:     do \
178:     { \
179:         EXPECT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckEqual, expected, actual); \
180:     } while (0)
181: 
182: #define ASSERT_NE(expected, actual) \
183:     do \
184:     { \
185:         ASSERT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckNotEqual, expected, actual); \
186:     } while (0)
187: #define EXPECT_NE(expected, actual) \
188:     do \
189:     { \
190:         EXPECT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckNotEqual, expected, actual); \
191:     } while (0)
192: 
193: #define ASSERT_EQ_IGNORE_CASE(expected, actual) \
194:     do \
195:     { \
196:         ASSERT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckEqualIgnoreCase, expected, actual); \
197:     } while (0)
198: #define EXPECT_EQ_IGNORE_CASE(expected, actual) \
199:     do \
200:     { \
201:         EXPECT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckEqualIgnoreCase, expected, actual); \
202:     } while (0)
203: 
204: #define ASSERT_NE_IGNORE_CASE(expected, actual) \
205:     do \
206:     { \
207:         ASSERT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase, expected, actual); \
208:     } while (0)
209: #define EXPECT_NE_IGNORE_CASE(expected, actual) \
210:     do \
211:     { \
212:         EXPECT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase, expected, actual); \
213:     } while (0)
214: 
215: #define ASSERT_NEAR(expected, actual, tolerance) \
216:     do \
217:     { \
218:         ASSERT_PRED_FORMAT3(::unittest::CheckClose, expected, actual, tolerance); \
219:     } while (0)
220: #define EXPECT_NEAR(expected, actual, tolerance) \
221:     do \
222:     { \
223:         EXPECT_PRED_FORMAT3(::unittest::CheckClose, expected, actual, tolerance); \
224:     } while (0)
225: 
226: #define ASSERT_NULL(value) ASSERT_EQ(nullptr, value)
227: #define EXPECT_NULL(value) EXPECT_EQ(nullptr, value)
228: #define ASSERT_NOT_NULL(value) ASSERT_NE(nullptr, value)
229: #define EXPECT_NOT_NULL(value) EXPECT_NE(nullptr, value)
```

- Line 61-63: We check whether the define `ASSERT_NEAR` is already defined. If so compilation ends with an error
- Line 81-83: We check whether the define `EXPECT_NEAR` is already defined. If so compilation ends with an error
- Line 85-111: We declare two functions `IsNullLiteralHelper()`, of which one takes a pointer, and the other takes a variable argument list.
This is some trickery, to determine if the parameter passed is a nullptr. The first returns a single character, the other a pointer to a char array, resulting in different return types and thus sizes
- Line 114-115: We define a macro `IS_NULL_LITERAL` which is used to check if a pointer is a null pointer
- Line 143-144: We define a macro `EXPECT_PRED_FORMAT3` which is passed a check function and three parameters.
The check function is called, and the result is passed to `UT_EXPECT_RESULT`.
The three parameter version is used with `CheckClose()` to check the result of comparison with a tolerance
- Line 145-146: We define a macro `ASSERT_PRED_FORMAT3` which is passed a check function and three parameters.
The check function is called, and the result is passed to `UT_ASSERT_RESULT`.
The three parameter version is used with `CheckClose()` to check the result of comparison with a tolerance
- Line 171-175: We change the macro `ASSERT_EQ` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 176-180: We change the macro `EXPECT_EQ` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 182-186: We change the macro `ASSERT_NE` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 187-191: We change the macro `EXPECT_NE` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 193-197: We define a macro `ASSERT_EQ_IGNORE_CASE` which uses `ASSERT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckEqualIgnoreCase` to check if the parameters are not equal, and generate a failure if the check fails
- Line 198-202: We define a macro `EXPECT_EQ_IGNORE_CASE` which uses `EXPECT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckEqualIgnoreCase` to check if the parameters are not equal, and generate a failure if the check fails
- Line 204-208: We define a macro `ASSERT_NE_IGNORE_CASE` which uses `ASSERT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase` to check if the parameters are not equal, and generate a failure if the check fails
- Line 209-213: We define a macro `EXPECT_NE_IGNORE_CASE` which uses `EXPECT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase` to check if the parameters are not equal, and generate a failure if the check fails
- Line 215-219: We define a macro `ASSERT_NEAR` which uses `ASSERT_PRED_FORMAT3` with `CheckClose` to check if the absolute difference of compared values is within tolerance, and generate a failure if the check fails
- Line 220-224: We define a macro `EXPECT_NEAR` which uses `EXPECT_PRED_FORMAT3` with `CheckClose` to check if the absolute difference of compared values is within tolerance, and generate a failure if the check fails
- Line 226: We define a macro `ASSERT_NULL` to check if the parameter is a null pointer, and generate a failure if the check fails
- Line 227: We define a macro `EXPECT_NULL` to check if the parameter is a null pointer, and generate a failure if the check fails
- Line 228: We define a macro `ASSERT_NOT_NULL` to check if the parameter is not a null pointer, and generate a failure if the check fails
- Line 229: We define a macro `EXPECT_NOT_NULL` to check if the parameter is not a null pointer, and generate a failure if the check fails

### Update project configuration {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACRO_EXTENSION__STEP_9_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
File: d:\Projects\baremetal.github\code\libraries\unittest\CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Checks.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleTestReporter.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/DeferredTestReporter.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PrintValue.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResult.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRunner.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
44:     )
45: 
46: set(PROJECT_INCLUDES_PUBLIC
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/AssertMacros.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Checks.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ConsoleTestReporter.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/DeferredTestReporter.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ExecuteTest.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ITestReporter.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/PrintValue.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestMacros.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResult.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRunner.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/unittest.h
67:     )
68: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACRO_EXTENSION__STEP_9_APPLICATION_CODE}

We'll use `Test4` to use the new macros defined.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
...
100: TEST(Test4)
101: {
102:     int* p = nullptr;
103:     int dd = 123;
104:     int* q = &dd;
105:     int* r = &dd;
106:     ASSERT_NULL(p);
107:     EXPECT_NULL(p);
108:     ASSERT_NULL(q);
109:     EXPECT_NULL(q);
110:     ASSERT_NOT_NULL(p);
111:     EXPECT_NOT_NULL(p);
112:     ASSERT_NOT_NULL(q);
113:     EXPECT_NOT_NULL(q);
114:     baremetal::string s1 = "A";
115:     baremetal::string s2 = "B";
116:     baremetal::string s3 = "B";
117:     baremetal::string s4 = "b";
118:     ASSERT_EQ(s1, s2);
119:     EXPECT_EQ(s1, s2);
120:     ASSERT_EQ(s2, s3);
121:     EXPECT_EQ(s2, s3);
122:     ASSERT_NE(s1, s2);
123:     EXPECT_NE(s1, s2);
124:     ASSERT_NE(s2, s3);
125:     EXPECT_NE(s2, s3);
126:     ASSERT_EQ_IGNORE_CASE(s1, s2);
127:     EXPECT_EQ_IGNORE_CASE(s1, s2);
128:     ASSERT_EQ_IGNORE_CASE(s2, s3);
129:     EXPECT_EQ_IGNORE_CASE(s2, s3);
130:     ASSERT_NE_IGNORE_CASE(s1, s2);
131:     EXPECT_NE_IGNORE_CASE(s1, s2);
132:     ASSERT_NE_IGNORE_CASE(s2, s3);
133:     EXPECT_NE_IGNORE_CASE(s2, s3);
134:     ASSERT_EQ_IGNORE_CASE(s2, s4);
135:     EXPECT_EQ_IGNORE_CASE(s2, s4);
136:     ASSERT_NE_IGNORE_CASE(s2, s4);
137:     EXPECT_NE_IGNORE_CASE(s2, s4);
138:     char t[] = { 'A', '\0' };
139:     char u[] = { 'B', '\0' };
140:     char v[] = { 'B', '\0' };
141:     char w[] = { 'b', '\0' };
142:     const char* tC = "A";
143:     const char* uC = "B";
144:     const char* vC = "B";
145:     const char* wC = "b";
146:     ASSERT_EQ(t, u);
147:     EXPECT_EQ(t, u);
148:     ASSERT_EQ(u, v);
149:     EXPECT_EQ(u, v);
150:     ASSERT_EQ(t, u);
151:     EXPECT_EQ(t, uC);
152:     ASSERT_EQ(uC, v);
153:     EXPECT_EQ(uC, vC);
154:     ASSERT_EQ(t, w);
155:     EXPECT_EQ(t, wC);
156:     ASSERT_EQ(uC, w);
157:     EXPECT_EQ(uC, wC);
158:     ASSERT_NE(t, u);
159:     EXPECT_NE(t, u);
160:     ASSERT_NE(u, v);
161:     EXPECT_NE(u, v);
162:     ASSERT_NE(t, u);
163:     EXPECT_NE(t, uC);
164:     ASSERT_NE(uC, v);
165:     EXPECT_NE(uC, vC);
166:     ASSERT_NE(t, w);
167:     EXPECT_NE(t, wC);
168:     ASSERT_NE(uC, w);
169:     EXPECT_NE(uC, wC);
170:     ASSERT_EQ_IGNORE_CASE(t, u);
171:     EXPECT_EQ_IGNORE_CASE(t, u);
172:     ASSERT_EQ_IGNORE_CASE(u, v);
173:     EXPECT_EQ_IGNORE_CASE(u, v);
174:     ASSERT_EQ_IGNORE_CASE(t, u);
175:     EXPECT_EQ_IGNORE_CASE(t, uC);
176:     ASSERT_EQ_IGNORE_CASE(uC, v);
177:     EXPECT_EQ_IGNORE_CASE(uC, vC);
178:     ASSERT_EQ_IGNORE_CASE(t, w);
179:     EXPECT_EQ_IGNORE_CASE(t, wC);
180:     ASSERT_EQ_IGNORE_CASE(uC, w);
181:     EXPECT_EQ_IGNORE_CASE(uC, wC);
182:     ASSERT_NE_IGNORE_CASE(t, u);
183:     EXPECT_NE_IGNORE_CASE(t, u);
184:     ASSERT_NE_IGNORE_CASE(u, v);
185:     EXPECT_NE_IGNORE_CASE(u, v);
186:     ASSERT_NE_IGNORE_CASE(t, u);
187:     EXPECT_NE_IGNORE_CASE(t, uC);
188:     ASSERT_NE_IGNORE_CASE(uC, v);
189:     EXPECT_NE_IGNORE_CASE(uC, vC);
190:     ASSERT_NE_IGNORE_CASE(t, w);
191:     EXPECT_NE_IGNORE_CASE(t, wC);
192:     ASSERT_NE_IGNORE_CASE(uC, w);
193:     EXPECT_NE_IGNORE_CASE(uC, wC);
194: 
195:     double a = 0.123;
196:     double b = 0.122;
197:     ASSERT_EQ(a, b);
198:     EXPECT_EQ(a, b);
199:     ASSERT_NEAR(a, b, 0.0001);
200:     EXPECT_NEAR(a, b, 0.0001);
201:     ASSERT_NEAR(a, b, 0.001);
202:     EXPECT_NEAR(a, b, 0.001);
203: }
...
```

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ASSERT_MACRO_EXTENSION__STEP_9_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Debug  Register test Test1 in fixture FixtureMyTest1 in suite Suite1 (TestRegistry:122)
Debug  Find suite Suite1 ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest1 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test2 in fixture FixtureMyTest2 in suite Suite2 (TestRegistry:122)
Debug  Find suite Suite2 ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest2 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test3 in fixture FixtureMyTest3 in suite DefaultSuite (TestRegistry:122)
Debug  Find suite DefaultSuite ... not found, creating new object (TestRegistry:85)
Debug  Fixture FixtureMyTest3 not found, creating new object (TestSuiteInfo:79)
Debug  Register test Test4 in fixture DefaultFixture in suite DefaultSuite (TestRegistry:122)
Debug  Find suite DefaultSuite ... found (TestRegistry:93)
Debug  Fixture DefaultFixture not found, creating new object (TestSuiteInfo:79)
Info   Starting up (System:201)
Debug  Hello World! (main:208)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
[  FAILED   ] Suite1::FixtureMyTest1::Test1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
[  FAILED   ] Suite2::FixtureMyTest2::Test2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
[  FAILED   ] FixtureMyTest3::Test3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
[  FAILED   ] Test4
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 4 out of 4 tests failed (47 failures).

Failures:
../code/applications/demo/src/main.cpp:43 : Failure in Suite1::FixtureMyTest1::Test1: Value of: false
  Expected: true

../code/applications/demo/src/main.cpp:69 : Failure in Suite2::FixtureMyTest2::Test2: Value of: false
  Expected: true

../code/applications/demo/src/main.cpp:70 : Failure in Suite2::FixtureMyTest2::Test2: Value of: true
  Expected: false

../code/applications/demo/src/main.cpp:94 : Failure in FixtureMyTest3::Test3: Value of: y
  Actual: 1
  Expected: x
  Which is: 0

../code/applications/demo/src/main.cpp:97 : Failure in FixtureMyTest3::Test3: Value of: z
  Actual: 1
  Expected not equal to: y
  Which is: 1

../code/applications/demo/src/main.cpp:108 : Failure in Test4: Value of: q
  Actual: 0x000000000029CC04
  Expected: nullptr
  Which is: null

../code/applications/demo/src/main.cpp:109 : Failure in Test4: Value of: q
  Actual: 0x000000000029CC04
  Expected: nullptr
  Which is: null

../code/applications/demo/src/main.cpp:110 : Failure in Test4: Value of: p
  Actual: null
  Expected not equal to: nullptr
  Which is: null

../code/applications/demo/src/main.cpp:111 : Failure in Test4: Value of: p
  Actual: null
  Expected not equal to: nullptr
  Which is: null

../code/applications/demo/src/main.cpp:118 : Failure in Test4: Value of: s2
  Actual: B
  Expected: s1
  Which is: A

../code/applications/demo/src/main.cpp:119 : Failure in Test4: Value of: s2
  Actual: B
  Expected: s1
  Which is: A

../code/applications/demo/src/main.cpp:124 : Failure in Test4: Value of: s3
  Actual: B
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:125 : Failure in Test4: Value of: s3
  Actual: B
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:126 : Failure in Test4: Value of: s2
  Actual: B
  Expected: s1
  Which is: A

../code/applications/demo/src/main.cpp:127 : Failure in Test4: Value of: s2
  Actual: B
  Expected: s1
  Which is: A

../code/applications/demo/src/main.cpp:132 : Failure in Test4: Value of: s3
  Actual: B
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:133 : Failure in Test4: Value of: s3
  Actual: B
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:136 : Failure in Test4: Value of: s4
  Actual: b
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:137 : Failure in Test4: Value of: s4
  Actual: b
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:146 : Failure in Test4: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:147 : Failure in Test4: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:150 : Failure in Test4: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:151 : Failure in Test4: Value of: uC
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:154 : Failure in Test4: Value of: w
  Actual: b
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:155 : Failure in Test4: Value of: wC
  Actual: b
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:156 : Failure in Test4: Value of: w
  Actual: b
  Expected: uC
  Which is: B

../code/applications/demo/src/main.cpp:157 : Failure in Test4: Value of: wC
  Actual: b
  Expected: uC
  Which is: B

../code/applications/demo/src/main.cpp:160 : Failure in Test4: Value of: v
  Actual: B
  Expected not equal to: u
  Which is: B

../code/applications/demo/src/main.cpp:161 : Failure in Test4: Value of: v
  Actual: B
  Expected not equal to: u
  Which is: B

../code/applications/demo/src/main.cpp:164 : Failure in Test4: Value of: v
  Actual: B
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:165 : Failure in Test4: Value of: vC
  Actual: B
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:170 : Failure in Test4: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:171 : Failure in Test4: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:174 : Failure in Test4: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:175 : Failure in Test4: Value of: uC
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:178 : Failure in Test4: Value of: w
  Actual: b
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:179 : Failure in Test4: Value of: wC
  Actual: b
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:184 : Failure in Test4: Value of: v
  Actual: B
  Expected not equal to: u
  Which is: B

../code/applications/demo/src/main.cpp:185 : Failure in Test4: Value of: v
  Actual: B
  Expected not equal to: u
  Which is: B

../code/applications/demo/src/main.cpp:188 : Failure in Test4: Value of: v
  Actual: B
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:189 : Failure in Test4: Value of: vC
  Actual: B
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:192 : Failure in Test4: Value of: w
  Actual: b
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:193 : Failure in Test4: Value of: wC
  Actual: b
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:197 : Failure in Test4: Value of: b
  Actual: 0.12200000000000
  Expected: a
  Which is: 0.12300000000000

../code/applications/demo/src/main.cpp:198 : Failure in Test4: Value of: b
  Actual: 0.12200000000000
  Expected: a
  Which is: 0.12300000000000

../code/applications/demo/src/main.cpp:199 : Failure in Test4: Value of: b
  Actual: 0.12200000000000
  Expected: a
  Which is: 0.12300000000000
  Tolerance: 0.0001
  (+/-) 0.10000000000000
../code/applications/demo/src/main.cpp:200 : Failure in Test4: Value of: b
  Actual: 0.12200000000000
  Expected: a
  Which is: 0.12300000000000
  Tolerance: 0.0001
  (+/-) 0.10000000000000

[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   Failures found: 47 (main:212)
Info   Wait 5 seconds (main:214)Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

Next: [18-writing-unit-tests](18-writing-unit-tests.md)


