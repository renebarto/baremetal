# Tutorial 17: Unit test infrastructure {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE}

@tableofcontents

## New tutorial setup {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/17-unit-test-infrastructure`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-17.a`
- an application `output/Debug/bin/17-unit-test-infrastructure.elf`
- an image in `deploy/Debug/17-unit-test-infrastructure-image`

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
File: code/libraries/unittest/CMakeLists.txt
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

### CurrentTest.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_CURRENTTESTH}

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
51: /// <summary>
52: /// Container class for current test administration
53: /// </summary>
54: class CurrentTest
55: {
56: public:
57:     static TestResults*& Results();
58:     static const TestDetails*& Details();
59: };
60: 
61: } // namespace unittest
```

### CurrentTest.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION__STEP_3_CURRENTTESTCPP}

We'll implement the `CurrentTest` class.

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
70:     explicit TestInfo(Test* testInstance, const TestDetails& details);
71: 
72:     TestInfo& operator = (const TestInfo&) = delete;
73:     TestInfo& operator = (TestInfo&&) = delete;
74: 
75:     /// <summary>
76:     /// Returns the test details
77:     /// </summary>
78:     /// <returns>Test details</returns>
79:     const TestDetails& Details() const { return m_details; }
80: 
81:     void Run(TestResults& testResults);
82: };
83: 
84: } // namespace unittest
```

The `TestInfo` class is again added to the `unittest` namespace.

- Line 50: We forward declare the `Test` class.
- Line 51: We forward declare a class `TestResults`. This is the container for all test results during a test run.
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
65: /// <param name="testInstance">Test instance</param>
66: /// <param name="details">Test details</param>
67: TestInfo::TestInfo(Test* testInstance, const TestDetails& details)
68:     : m_details{ details }
69:     , m_testInstance{ testInstance }
70:     , m_next{}
71: {
72: }
73: 
74: /// <summary>
75: /// Run the test instance, and update the test results
76: /// </summary>
77: /// <param name="testResults"></param>
78: void TestInfo::Run(TestResults& testResults)
79: {
80:     if (m_testInstance != nullptr)
81:         m_testInstance->RunImpl();
82: }
83: 
84: } // namespace unittest
```

- Line 55-60: We implement the default constructor
- Line 67-72: We implement the non default constructor
- Line 78-82: We implement the `Run()` method

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
14: using namespace unittest;
15: 
16: class MyTest
17:     : public unittest::Test
18: {
19: public:
20:     void RunImpl() const override;
21: };
22: 
23: void MyTest::RunImpl() const
24: {
25:     LOG_INFO("Running test");
26: }
27: 
28: int main()
29: {
30:     auto& console = GetConsole();
31: 
32:     MyTest myTest;
33:     TestInfo myTestInfo(&myTest, TestDetails("MyTest", "DummyFixture", "DummySuite", __FILE__, __LINE__));
34:     TestResults results;
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
- Line 92: The method `Run()` runs all tests in the test fixture and update the test results. We'll be revisiting this later
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
113: /// <summary>
114: /// Count the number of tests in the test fixture
115: /// </summary>
116: /// <returns>Number of tests in the test fixture</returns>
117: int TestFixtureInfo::CountTests()
118: {
119:     int numberOfTests = 0;
120:     TestInfo* test = Head();
121:     while (test != nullptr)
122:     {
123:         ++numberOfTests;
124:         test = test->m_next;
125:     }
126:     return numberOfTests;
127: }
128: 
129: } // namespace unittest
```

- Line 54-60: We implement the constructor
- Line 67-76: We implement the destructor. This goes through the list of tests, and deletes every one of these. Note that we will therefore need to create the tests on the heap.
- Line 84-97: We implement the `AddTest()` method. This will add the test passed in at the end of the list
- Line 103-111: We implement the `Run()` method. This goes through the list of tests, and calls `Run()` on each
- Line 117-127: We implement the `CountTests()` method. This goes through the list of tests, and counts them

### TestResults.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_TESTRESULTSH}

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

### TestResults.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_TESTRESULTSCPP}

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

### TestInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES__STEP_4_TESTINFOCPP}

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
75: /// <summary>
76: /// Run the test instance, and update the test results
77: /// </summary>
78: /// <param name="testResults"></param>
79: void TestInfo::Run(TestResults& testResults)
80: {
81:     CurrentTest::Details() = &Details();
82:     CurrentTest::Results() = &testResults;
83: 
84:     if (m_testInstance != nullptr)
85:     {
86:         testResults.OnTestStart(m_details);
87: 
88:         m_testInstance->RunImpl();
89: 
90:         testResults.OnTestFinish(m_details);
91:     }
92: }
...
```

- Line 44: We need to include the header for `TestResults`
- Line 79-92: Next to calling the `RunImpl()` method on the test, we also call the `OnTestStart()` and `OnTestFinish()` methods on the `TestResults` class

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
70:         : TestInfo(testInstance, TestDetails("MyTest1", "FixtureMyTest", "", __FILE__, __LINE__))
71:     {
72:     }
73: };
74: 
75: class FixtureMyTest2Helper
76:     : public FixtureMyTest
77: {
78: public:
79:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
80:     explicit FixtureMyTest2Helper(const TestDetails& details)
81:         : m_details{ details }
82:     {
83:         SetUp();
84:     }
85:     virtual ~FixtureMyTest2Helper()
86:     {
87:         TearDown();
88:     }
89:     void RunImpl() const;
90:     const TestDetails& m_details;
91: };
92: void FixtureMyTest2Helper::RunImpl() const
93: {
94:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 2");
95: }
96: 
97: class MyTest2
98:     : public Test
99: {
100:     void RunImpl() const override
101:     {
102:         LOG_DEBUG("Test 2");
103:         FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
104:         fixtureHelper.RunImpl();
105:     }
106: } myTest2;
107: class MyTestInfo2
108:     : public TestInfo
109: {
110: public:
111:     MyTestInfo2(Test* testInstance)
112:         : TestInfo(testInstance, TestDetails("MyTest2", "FixtureMyTest", "", __FILE__, __LINE__))
113:     {
114:     }
115: };
116: 
117: class FixtureMyTest3Helper
118:     : public FixtureMyTest
119: {
120: public:
121:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
122:     explicit FixtureMyTest3Helper(const TestDetails& details)
123:         : m_details{ details }
124:     {
125:         SetUp();
126:     }
127:     virtual ~FixtureMyTest3Helper()
128:     {
129:         TearDown();
130:     }
131:     void RunImpl() const;
132:     const TestDetails& m_details;
133: };
134: void FixtureMyTest3Helper::RunImpl() const
135: {
136:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 3");
137: }
138: 
139: class MyTest3
140:     : public Test
141: {
142:     void RunImpl() const override
143:     {
144:         LOG_DEBUG("Test 3");
145:         FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
146:         fixtureHelper.RunImpl();
147:     }
148: } myTest3;
149: class MyTestInfo3
150:     : public TestInfo
151: {
152: public:
153:     MyTestInfo3(Test* testInstance)
154:         : TestInfo(testInstance, TestDetails("MyTest3", "FixtureMyTest", "", __FILE__, __LINE__))
155:     {
156:     }
157: };
158: 
159: class MyTest
160:     : public Test
161: {
162: public:
163:     void RunImpl() const override;
164: } myTest;
165: 
166: void MyTest::RunImpl() const
167: {
168:     CurrentTest::Results()->OnTestRun(*CurrentTest::Details(), "Running test");
169: }
170: 
171: int main()
172: {
173:     auto& console = GetConsole();
174: 
175:     TestInfo* test1 = new MyTestInfo1(&myTest1);
176:     TestInfo* test2 = new MyTestInfo2(&myTest2);
177:     TestInfo* test3 = new MyTestInfo3(&myTest3);
178:     TestFixtureInfo* fixture = new TestFixtureInfo("MyFixture");
179:     TestResults results;
180:     fixture->AddTest(test1);
181:     fixture->AddTest(test2);
182:     fixture->AddTest(test3);
183:     fixture->Run(results);
184:     delete fixture;
185:     TestInfo myTestInfo(&myTest, TestDetails("MyTest", "", "", __FILE__, __LINE__));
186: 
187:     myTestInfo.Run(results);
188: 
189:     LOG_INFO("Wait 5 seconds");
190:     Timer::WaitMilliSeconds(5000);
191: 
192:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
193:     char ch{};
194:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
195:     {
196:         ch = console.ReadChar();
197:         console.WriteChar(ch);
198:     }
199:     if (ch == 'p')
200:         assert(false);
201: 
202:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
203: }
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
- Line 65-73: We declare and implement the class `MyTestInfo1`, which derives from `TestInfo`. Its constructor links the test instance
- Line 75-95: We declare and implement the class `FixtureTest2Helper` in a similar way as `FixtureMyTest1Helper`
- Line 97-106: We declare and implement the class `MyTest2` in a similar way as `MyTest1`, and instantiate it as `myTest2`
- Line 107-115: We declare and implement the class `MyTestInfo2` in a similar way as `MyTestInfo1`
- Line 117-137: We declare and implement the class `FixtureTest3Helper` in a similar way `FixtureMyTest1Helper`
- Line 139-148: We declare and implement the class `MyTest3` in a similar way as `MyTest1`, and instantiate it as `myTest3`
- Line 149-157: We declare and implement the class `MyTestInfo3` in a similar way as `MyTestInfo1`
- Line 159-164: We declare the class `MyTest`, which derives from `Test`
- Line 166-169: We implement the `RunImpl()` method for `MyTest` as a call to the `OnTestRun()` method on the `TestResults` instance
- Line 175-177: We instantiate each of `MyTestInfo1`, `MyTestInfo2` and `MyTestInfo3`, passing in the instance of the corresponding test
- Line 178: We instantiate `TestFixtureInfo` as our test fixture
- Line 180-182: We add the three tests to the test fixture
- Line 183: We run the test fixture
- Line 184: We clean up the test fixture. Note that the test fixture desctructor deletes all test administration instances, so we don't need to (and shouldn't) do that.
The tests themselves are normally instantiated at static initialization time, so will never be destructed
- Line 185: We instantiate a `TestInfo`, linked to the `myTest` instance
- Line 187: We run the test

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
Info   DefaultSuite::FixtureMyTest::MyTest1 Start test (TestResults:68)
Debug  Test 1 (main:60)
Debug  MyTest SetUp (main:25)
Debug  DefaultSuite::FixtureMyTest::MyTest1 (../code/applications/demo/src/main.cpp:70) --> MyTestHelper 1 (TestResults:76)
Debug  MyTest TearDown (main:29)
Info   DefaultSuite::FixtureMyTest::MyTest1 Finish test (TestResults:81)
Info   DefaultSuite::FixtureMyTest::MyTest2 Start test (TestResults:68)
Debug  Test 2 (main:103)
Debug  MyTest SetUp (main:25)
Debug  DefaultSuite::FixtureMyTest::MyTest2 (../code/applications/demo/src/main.cpp:113) --> MyTestHelper 2 (TestResults:76)
Debug  MyTest TearDown (main:29)
Info   DefaultSuite::FixtureMyTest::MyTest2 Finish test (TestResults:81)
Info   DefaultSuite::FixtureMyTest::MyTest3 Start test (TestResults:68)
Debug  Test 3 (main:146)
Debug  MyTest SetUp (main:25)
Debug  DefaultSuite::FixtureMyTest::MyTest3 (../code/applications/demo/src/main.cpp:156) --> MyTestHelper 3 (TestResults:76)
Debug  MyTest TearDown (main:29)
Info   DefaultSuite::FixtureMyTest::MyTest3 Finish test (TestResults:81)
Info   DefaultSuite::DefaultFixture::MyTest Start test (TestResults:68)
Debug  DefaultSuite::DefaultFixture::MyTest (../code/applications/demo/src/main.cpp:199) --> Running test (TestResults:76)
Info   DefaultSuite::DefaultFixture::MyTest Finish test (TestResults:81)
Info   Wait 5 seconds (main:204)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Adding test suites - Step 5 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5}

The final step in building infrasturcture for unit tests is collecting test fixtures in test suites.
Test suites are again different from tests and test fixtures in their structure.

- A TestSuite is actually nothing more than a function returning the suite name. The trick is that the function will be placed inside a namespace, as well as all the test fixtures and tests that belong inside it
- The `TestSuiteInfo` class holds the actual test suite information, such as the pointer to the next test suite, and the pointers to the first and last test fixture in the suite

So the `TestSuiteInfo` class holds the administration of the test suite, like `TestFixtureInfo` and `TestInfo` do for test fixtures and tests.

### TestSuite.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_TESTSUITEH}

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
42: /// @file
43: /// Test suite
44: 
45: /// <summary>
46: /// Return the default test suite name (which will be converted to the actual default name by the test reporter)
47: /// </summary>
48: /// <returns></returns>
49: inline char const* GetSuiteName()
50: {
51:     return "";
52: }
```

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_TESTSUITEINFOH}

Now let's declare the `TestSuiteInfo` class.

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
43: #include <unittest/TestResults.h>
44: 
45: /// @file
46: /// Test suite administration
47: 
48: namespace unittest
49: {
50: 
51: class TestFixtureInfo;
52: 
53: /// <summary>
54: /// Test suite administration
55: ///
56: /// Holds information on a test suite, which includes its name and the list of test fixtures that are part of it
57: /// </summary>
58: class TestSuiteInfo
59: {
60: private:
61:     /// @brief Pointer to first test fixture in the list
62:     TestFixtureInfo* m_head;
63:     /// @brief Pointer to last test fixture in the list
64:     TestFixtureInfo* m_tail;
65:     /// @brief Pointer to next test suite info in the list
66:     TestSuiteInfo* m_next;
67:     /// @brief Test suite name
68:     baremetal::string m_suiteName;
69: 
70: public:
71:     TestSuiteInfo() = delete;
72:     TestSuiteInfo(const TestSuiteInfo&) = delete;
73:     TestSuiteInfo(TestSuiteInfo&&) = delete;
74:     explicit TestSuiteInfo(const baremetal::string& suiteName);
75:     virtual ~TestSuiteInfo();
76: 
77:     TestSuiteInfo& operator = (const TestSuiteInfo&) = delete;
78:     TestSuiteInfo& operator = (TestSuiteInfo&&) = delete;
79: 
80:     /// <summary>
81:     /// Returns the pointer to the first test fixture in the list for this test suite
82:     /// </summary>
83:     /// <returns>Pointer to the first test fixture in the list for this test suite</returns>
84:     TestFixtureInfo* Head() const { return m_head; }
85: 
86:     /// <summary>
87:     /// Returns the test suite name
88:     /// </summary>
89:     /// <returns>Test suite name</returns>
90:     const baremetal::string& Name() const { return m_suiteName; }
91: 
92:     void Run(TestResults& testResults);
93: 
94:     int CountFixtures();
95:     int CountTests();
96: 
97:     void AddFixture(TestFixtureInfo* testFixture);
98: };
99: 
100: } // namespace unittest
```

- Line 62-64: The member variables `m_head` and `m_tail` store the pointer to the first and last test fixture in the test suite
- Line 66: The member variable `m_next` is the pointer to the next test suite. Again, test suites are stored in linked list
- Line 68: The member variable `m_suiteName` holds the name of the test suite
- Line 71: We remove the default constructor
- Line 72-73: We remove the copy and move constructors
- Line 75: We declare the only usable constructor which receives the test suite name
- Line 76: We declare the destructor
- Line 77-78: We remove the assignment and move assignment operators
- Line 84: The method `Head()` returns the pointer to the first test fixture in the list
- Line 90: The method `Name()` returns the test suite name
- Line 92: The method `Run()` runs all test fixtures in the test suite and update the test results. We'll be revisiting this later
- Line 94: The method `CountFixtures()` counts and returns the number of test fixtures in the test suite
- Line 95: The method `CountTests()` counts and returns the number of tests in all test fixtures in the test suite
- Line 97: The method `GetTestFixture()` finds and returns a test fixture in the list for the test suite, or if not found, creates a new test fixture with the specified name
- Line 98: The method `AddFixture()` adds a test fixture to the list for the test suite

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_TESTFIXTUREINFOH}

As the `TestSuiteInfo` class needs access to the `TestFixtureInfo` in order to access the `m_next` pointer, we need to make it a friend class.

Update the file `code/libraries/unittest/include/unittest/TestFixtureInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixtureInfo.h
...
58: class TestFixtureInfo
59: {
60: private:
61:     friend class TestSuiteInfo;
62:     /// @brief Pointer to first test in the list
63:     TestInfo* m_head;
64:     /// @brief Pointer to last test in the list
65:     TestInfo* m_tail;
66:     /// @brief Pointer to next test fixture info in the list
67:     TestFixtureInfo* m_next;
68:     /// @brief Test fixture name
69:     baremetal::string m_fixtureName;
70: 
71: public:
...
```

### TestResults.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_TESTRESULTSH}

We will add some more methods to the `TestResults` class for displaying start and finish of both test fixtures and test suites.

Update the file `code/libraries/unittest/include/unittest/TestResults.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResults.h
...
51: namespace unittest
52: {
53: 
54: class TestSuiteInfo;
55: class TestFixtureInfo;
56: class TestDetails;
57: 
58: /// <summary>
59: /// Test results class
60: /// </summary>
61: class TestResults
62: {
63: public:
64:     TestResults() = default;
65: 
66:     void OnTestSuiteStart(TestSuiteInfo* suite);
67:     void OnTestSuiteFinish(TestSuiteInfo* suite);
68:     void OnTestFixtureStart(TestFixtureInfo* fixture);
69:     void OnTestFixtureFinish(TestFixtureInfo* fixture);
70:     void OnTestStart(const TestDetails& details);
71:     void OnTestRun(const TestDetails& details, const baremetal::string& message);
72:     void OnTestFinish(const TestDetails& details);
73: };
74: 
75: } // namespace unittest
...
```

- Line 66: We declare the method `OnTestSuiteStart()` to indicate the start of a test suite
- Line 67: We declare the method `OnTestSuiteFinish()` to indicate the finish of a test suite
- Line 68: We declare the method `OnTestFixtureStart()` to indicate the start of a test fixture
- Line 69: We declare the method `OnTestFixtureFinish()` to indicate the finish of a test fixture

### TestResults.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_TESTRESULTSCPP}

Let's implement the new methods in the `Testresults` class.

Update the file `code/libraries/unittest/src/TestResults.cpp`

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
46: #include <unittest/TestFixtureInfo.h>
47: #include <unittest/TestSuiteInfo.h>
48: 
49: /// @file
50: /// Test results implementation
51: 
52: using namespace baremetal;
53: 
54: /// @brief Define log name
55: LOG_MODULE("TestResults");
56: 
57: namespace unittest
58: {
59: 
60: /// <summary>
61: /// Return fully qualified test name in format [suite]::[fixture]::[test]
62: /// </summary>
63: /// <param name="details">Test details</param>
64: /// <returns>Resulting string</returns>
65: static string QualifiedTestName(const TestDetails& details)
66: {
67:     return Format("%s::%s::%s",
68:         details.SuiteName().empty() ? "DefaultSuite" : details.SuiteName().c_str(),
69:         details.FixtureName().empty() ? "DefaultFixture" : details.FixtureName().c_str(),
70:         details.TestName().c_str());
71: }
72: 
73: /// <summary>
74: /// Start a test suite run
75: /// </summary>
76: /// <param name="suite">Test suite to start</param>
77: void TestResults::OnTestSuiteStart(TestSuiteInfo* suite)
78: {
79:     LOG_INFO(suite->Name() + " Start suite");
80: }
81: 
82: /// <summary>
83: /// Finish a test suite run
84: /// </summary>
85: /// <param name="suite">Test suite to finish</param>
86: void TestResults::OnTestSuiteFinish(TestSuiteInfo* suite)
87: {
88:     LOG_INFO(suite->Name() + " Finish suite");
89: }
90: 
91: /// <summary>
92: /// Start a test fixture run
93: /// </summary>
94: /// <param name="fixture">Test fixture to start</param>
95: void TestResults::OnTestFixtureStart(TestFixtureInfo* fixture)
96: {
97:     LOG_INFO(fixture->Name() + " Start fixture");
98: }
99: 
100: /// <summary>
101: /// Finish a test fixture run
102: /// </summary>
103: /// <param name="fixture">Test fixture to finish</param>
104: void TestResults::OnTestFixtureFinish(TestFixtureInfo* fixture)
105: {
106:     LOG_INFO(fixture->Name() + " Finish fixture");
107: }
108: 
109: /// <summary>
110: /// Start a test
111: /// </summary>
112: /// <param name="details">Test details of test to start</param>
113: void TestResults::OnTestStart(const TestDetails& details)
114: {
115:     LOG_INFO(QualifiedTestName(details) + " Start test");
116: }
117: 
118: void TestResults::OnTestRun(const TestDetails& details, const string& message)
119: {
120:     string fullMessage = QualifiedTestName(details) + Format(" (%s:%d)",
121:         details.SourceFileName().c_str(),
122:         details.SourceFileLineNumber()) + " --> " + message;
123:     LOG_DEBUG(fullMessage);
124: }
125: 
126: /// <summary>
127: /// Finish a test
128: /// </summary>
129: /// <param name="details">Test details of test to finish</param>
130: void TestResults::OnTestFinish(const TestDetails& details)
131: {
132:     LOG_INFO(QualifiedTestName(details) + " Finish test");
133: }
134: 
135: } // namespace unittest
```

- Line 77-80: We implement the methos `OnTestSuiteStart()`
- Line 86-89: We implement the methos `OnTestSuiteFinish()`
- Line 95-98: We implement the methos `OnTestFixtureStart()`
- Line 104-107: We implement the methos `OnTestFixtureFinish()`

You will notice, that now we have documented the code as well.

### TestFixtureInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_TESTFIXTUREINFOCPP}

We'll change the `Run()` method in the `TestFixtureInfo` class to call the corresponding methods in the `TestResults` class.

Update the file `code/libraries/unittest/src/TestFixtureInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestResults.cpp
...
99: /// <summary>
100: /// Run tests in test fixture, updating the test results
101: /// </summary>
102: /// <param name="testResults">Test results to use and update</param>
103: void TestFixtureInfo::Run(TestResults& testResults)
104: {
105:     testResults.OnTestFixtureStart(this);
106: 
107:     TestInfo* test = Head();
108:     while (test != nullptr)
109:     {
110:         test->Run(testResults);
111:         test = test->m_next;
112:     }
113: 
114:     testResults.OnTestFixtureFinish(this);
115: }
...
```

### TestSuiteInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_TESTSUITEINFOCPP}

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
45: /// @file
46: /// Test suite administration implementation
47: 
48: using namespace baremetal;
49: 
50: namespace unittest {
51: 
52: /// @brief Define log name
53: LOG_MODULE("TestSuiteInfo");
54: 
55: /// <summary>
56: /// Constructor
57: /// </summary>
58: /// <param name="suiteName">Test suite name</param>
59: TestSuiteInfo::TestSuiteInfo(const string &suiteName)
60:     : m_head{}
61:     , m_tail{}
62:     , m_next{}
63:     , m_suiteName{suiteName}
64: {
65: }
66: 
67: /// <summary>
68: /// Destructor
69: ///
70: /// Cleans up all registered tests and test fixtures for this test suite
71: /// </summary>
72: TestSuiteInfo::~TestSuiteInfo()
73: {
74:     TestFixtureInfo *testFixture = m_head;
75:     while (testFixture != nullptr)
76:     {
77:         TestFixtureInfo *currentFixture = testFixture;
78:         testFixture                     = testFixture->m_next;
79:         delete currentFixture;
80:     }
81: }
82: 
83: /// <summary>
84: /// Find a test fixture with specified name, register a new one if not found
85: /// </summary>
86: /// <param name="fixtureName">Test fixture name to search for</param>
87: /// <returns>Found or created test fixture</returns>
88: TestFixtureInfo *TestSuiteInfo::GetTestFixture(const string &fixtureName)
89: {
90:     TestFixtureInfo *testFixture = m_head;
91:     while ((testFixture != nullptr) && (testFixture->Name() != fixtureName))
92:         testFixture = testFixture->m_next;
93:     if (testFixture == nullptr)
94:     {
95:         testFixture = new TestFixtureInfo(fixtureName);
96:         AddFixture(testFixture);
97:     }
98:     return testFixture;
99: }
100: 
101: /// <summary>
102: /// Add a test fixture
103: /// </summary>
104: /// <param name="testFixture">Test fixture to add</param>
105: void TestSuiteInfo::AddFixture(TestFixtureInfo *testFixture)
106: {
107:     if (m_tail == nullptr)
108:     {
109:         assert(m_head == nullptr);
110:         m_head = testFixture;
111:         m_tail = testFixture;
112:     }
113:     else
114:     {
115:         m_tail->m_next = testFixture;
116:         m_tail         = testFixture;
117:     }
118: }
119: 
120: /// <summary>
121: /// Run tests in test suite, updating the test results
122: /// </summary>
123: /// <param name="testResults">Test results to use and update</param>
124: void TestSuiteInfo::Run(TestResults& testResults)
125: {
126:     testResults.OnTestSuiteStart(this);
127: 
128:     TestFixtureInfo* testFixture = Head();
129:     while (testFixture != nullptr)
130:     {
131:         testFixture->Run(testResults);
132:         testFixture = testFixture->m_next;
133:     }
134: 
135:     testResults.OnTestSuiteFinish(this);
136: }
137: 
138: /// <summary>
139: /// Count the number of test fixtures in the test suite
140: /// </summary>
141: /// <returns>Number of test fixtures in the test suite</returns>
142: int TestSuiteInfo::CountFixtures()
143: {
144:     int numberOfTestFixtures = 0;
145:     TestFixtureInfo *testFixture = Head();
146:     while (testFixture != nullptr)
147:     {
148:         ++numberOfTestFixtures;
149:         testFixture = testFixture->m_next;
150:     }
151:     return numberOfTestFixtures;
152: }
153: 
154: /// <summary>
155: /// Count the number of tests in the test suite
156: /// </summary>
157: /// <returns>Number of tests in the test suite</returns>
158: int TestSuiteInfo::CountTests()
159: {
160:     int numberOfTests = 0;
161:     TestFixtureInfo *testFixture = Head();
162:     while (testFixture != nullptr)
163:     {
164:         numberOfTests += testFixture->CountTests();
165:         testFixture = testFixture->m_next;
166:     }
167:     return numberOfTests;
168: }
169: 
170: } // namespace unittest
```

- Line 59-65: We implement the constructor
- Line 72-81: We implement the destructor. This goes through the list of test fixtures, and deletes every one of these. Note that we will therefore need to create the test fixtures on the heap
- Line 88-99: We implement the `GetTestFixture()` method. This will try to find the test fixture with the specified name in the test suite. If it is found, the pointer is returned, if not, a new instance iscreated
- Line 105-118: We implement the `AddFixture()` method. This will add the test fixture passed in at the end of the list
- Line 124-136: We implement the `Run()` method. This goes through the list of test fixtures, and calls `Run()` on each.
Note that it also calls the methods `OnTestSuiteStart()` and `OnTestSuiteFinish()` on the `TestResults` instance
- Line 142-152: We implement the `CountFixtures()` method. This goes through the list of test fixtures, and counts them
- Line 158-168: We implement the `CountTests()` method. This goes through the list of test fixtures, and counts the tests in each of them

### Update CMake file {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_UPDATE_CMAKE_FILE}

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
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
38:     )
39: 
40: set(PROJECT_INCLUDES_PUBLIC
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Test.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestInfo.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
50:     )
51: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_UPDATE_APPLICATION_CODE}

So as a final step let's define test fixtures inside a test suite, and outside any test suite.
We'll add the test fixtures in the test suite, and leave the ones without a suite out of course.

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
13: #include <unittest/TestSuite.h>
14: #include <unittest/TestSuiteInfo.h>
15: 
16: LOG_MODULE("main");
17: 
18: using namespace baremetal;
19: using namespace unittest;
20: 
21: namespace Suite1 {
22: 
23: inline char const* GetSuiteName()
24: {
25:     return baremetal::string("Suite1");
26: }
27: 
28: class FixtureMyTest1
29:     : public TestFixture
30: {
31: public:
32:     void SetUp() override
33:     {
34:         LOG_DEBUG("MyTest SetUp");
35:     }
36:     void TearDown() override
37:     {
38:         LOG_DEBUG("MyTest TearDown");
39:     }
40: };
41: 
42: class FixtureMyTest1Helper
43:     : public FixtureMyTest1
44: {
45: public:
46:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
47:     explicit FixtureMyTest1Helper(const TestDetails& details)
48:         : m_details{ details }
49:     {
50:         SetUp();
51:     }
52:     virtual ~FixtureMyTest1Helper()
53:     {
54:         TearDown();
55:     }
56:     void RunImpl() const;
57:     const TestDetails& m_details;
58: };
59: void FixtureMyTest1Helper::RunImpl() const
60: {
61:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 1");
62: }
63: 
64: class MyTest1
65:     : public Test
66: {
67:     void RunImpl() const override
68:     {
69:         LOG_DEBUG("Test 1");
70:         FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
71:         fixtureHelper.RunImpl();
72:     }
73: } myTest1;
74: class MyTestInfo1
75:     : public TestInfo
76: {
77: public:
78:     MyTestInfo1(Test* testInstance)
79:         : TestInfo(testInstance, TestDetails("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__))
80:     {
81:     }
82: };
83: 
84: } // namespace Suite1
85: 
86: namespace Suite2 {
87: 
88: inline char const* GetSuiteName()
89: {
90:     return baremetal::string("Suite2");
91: }
92: 
93: class FixtureMyTest2
94:     : public TestFixture
95: {
96: public:
97:     void SetUp() override
98:     {
99:         LOG_DEBUG("FixtureMyTest2 SetUp");
100:     }
101:     void TearDown() override
102:     {
103:         LOG_DEBUG("FixtureMyTest2 TearDown");
104:     }
105: };
106: 
107: class FixtureMyTest2Helper
108:     : public FixtureMyTest2
109: {
110: public:
111:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
112:     explicit FixtureMyTest2Helper(const TestDetails& details)
113:         : m_details{ details }
114:     {
115:         SetUp();
116:     }
117:     virtual ~FixtureMyTest2Helper()
118:     {
119:         TearDown();
120:     }
121:     void RunImpl() const;
122:     const TestDetails& m_details;
123: };
124: void FixtureMyTest2Helper::RunImpl() const
125: {
126:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 2");
127: }
128: 
129: class MyTest2
130:     : public Test
131: {
132:     void RunImpl() const override
133:     {
134:         LOG_DEBUG("Test 2");
135:         FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
136:         fixtureHelper.RunImpl();
137:     }
138: } myTest2;
139: class MyTestInfo2
140:     : public TestInfo
141: {
142: public:
143:     MyTestInfo2(Test* testInstance)
144:         : TestInfo(testInstance, TestDetails("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__))
145:     {
146:     }
147: };
148: 
149: } // namespace Suite2
150: 
151: class FixtureMyTest3
152:     : public TestFixture
153: {
154: public:
155:     void SetUp() override
156:     {
157:         LOG_DEBUG("FixtureMyTest3 SetUp");
158:     }
159:     void TearDown() override
160:     {
161:         LOG_DEBUG("FixtureMyTest3 TearDown");
162:     }
163: };
164: 
165: class FixtureMyTest3Helper
166:     : public FixtureMyTest3
167: {
168: public:
169:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
170:     explicit FixtureMyTest3Helper(const TestDetails& details)
171:         : m_details{ details }
172:     {
173:         SetUp();
174:     }
175:     virtual ~FixtureMyTest3Helper()
176:     {
177:         TearDown();
178:     }
179:     void RunImpl() const;
180:     const TestDetails& m_details;
181: };
182: void FixtureMyTest3Helper::RunImpl() const
183: {
184:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 3");
185: }
186: 
187: class MyTest3
188:     : public Test
189: {
190:     void RunImpl() const override
191:     {
192:         LOG_DEBUG("Test 3");
193:         FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
194:         fixtureHelper.RunImpl();
195:     }
196: } myTest3;
197: class MyTestInfo3
198:     : public TestInfo
199: {
200: public:
201:     MyTestInfo3(Test* testInstance)
202:         : TestInfo(testInstance, TestDetails("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__))
203:     {
204:     }
205: };
206: 
207: 
208: class MyTest
209:     : public Test
210: {
211: public:
212:     void RunImpl() const override;
213: } myTest;
214: 
215: void MyTest::RunImpl() const
216: {
217:     CurrentTest::Results()->OnTestRun(*CurrentTest::Details(), "Running test");
218: }
219: 
220: int main()
221: {
222:     auto& console = GetConsole();
223: 
224:     TestInfo* test1 = new Suite1::MyTestInfo1(&Suite1::myTest1);
225:     TestInfo* test2 = new Suite2::MyTestInfo2(&Suite2::myTest2);
226:     TestInfo* test3 = new MyTestInfo3(&myTest3);
227:     TestFixtureInfo* fixture1 = new TestFixtureInfo("MyFixture1");
228:     fixture1->AddTest(test1);
229:     TestFixtureInfo* fixture2 = new TestFixtureInfo("MyFixture2");
230:     fixture2->AddTest(test2);
231:     TestFixtureInfo* fixture3 = new TestFixtureInfo("MyFixture3");
232:     fixture3->AddTest(test3);
233:     unittest::TestResults results;
234:     TestSuiteInfo* suite1 = new TestSuiteInfo("MySuite1");
235:     suite1->AddFixture(fixture1);
236:     TestSuiteInfo* suite2 = new TestSuiteInfo("MySuite2");
237:     suite2->AddFixture(fixture2);
238:     TestSuiteInfo* suiteDefault = new TestSuiteInfo("");
239:     suiteDefault->AddFixture(fixture3);
240:     suite1->Run(results);
241:     suite2->Run(results);
242:     suiteDefault->Run(results);
243:     delete suite1;
244:     delete suite2;
245:     delete suiteDefault;
246:     unittest::TestInfo myTestInfo(&myTest, TestDetails("MyTest", "", "", __FILE__, __LINE__));
247: 
248:     myTestInfo.Run(results);
249: 
250:     LOG_INFO("Wait 5 seconds");
251:     Timer::WaitMilliSeconds(5000);
252: 
253:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
254:     char ch{};
255:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
256:     {
257:         ch = console.ReadChar();
258:         console.WriteChar(ch);
259:     }
260:     if (ch == 'p')
261:         assert(false);
262: 
263:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
264: }
```

- Line 13: We include the header to define the default `GetSuiteName()` function
- Line 14: We include the header for `TestSuiteInfo`
- Line 21: We define the namespace Suite1
- Line 23-26: We define the namespace specific version of `GetSuiteName()`
- Line 28-40: We declare and implement the class `FixtureMyTest1` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods, simply by logging a message
- Line 42-62: We declare and implement the class `FixtureMyTest1Helper` which derives from the class `FixtureMyTest` just defined, and implements the constructor and destructor, calling resp. `SetUp()` and `TearDown()`
This class also defines a `RunImpl()` method, which forms the actual test body for the test MyTest1.
It is implemented as a call to the `OnTestRun()` method on the `TestResults` instance
- Line 64-73: We declare and implement the class `MyTest1`, which derives from `Test`, and acts as the placeholders for the fixture test.
We instantiate it as `myTest1`. Its `RunImpl()` method instantiates `FixtureMyTest1Helper`, and runs its `RunImpl()` method
- Line 74-82: We declare and implement the class `MyTestInfo1`, which derives from `TestInfo`. Its constructor links the test instance.
Notice that the constructor uses the `GetSuiteName()` function in `Suite1` to retrieve the correct test suite name
- Line 84: We end the namespace Suite1
- Line 86: We define the namespace Suite2
- Line 88-91: We define the namespace specific version of `GetSuiteName()`
- Line 93-105: We declare and implement the class `FixtureMyTest2` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods, simply by logging a message.
Notice that we now have to declare a new class, as we are in a different namespace
- Line 107-127: We declare and implement the class `FixtureTest2Helper` in a similar way as `FixtureMyTest1Helper`
- Line 129-138: We declare and implement the class `MyTest2` in a similar way as `MyTest1`, and instantiate it as `myTest2`
- Line 139-147: We declare and implement the class `MyTestInfo2` in a similar way as `MyTestInfo1`.
Notice that the constructor uses the `GetSuiteName()` function in `Suite2` to retrieve the correct test suite name
- Line 149: We end the namespace Suite2
- Line 151-163: We declare and implement the class `FixtureMyTest3` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods, simply by logging a message
- Line 165-185: We declare and implement the class `FixtureTest3Helper` in a similar way `FixtureMyTest1Helper`
- Line 187-196: We declare and implement the class `MyTest3` in a similar way as `MyTest1`, and instantiate it as `myTest3`
- Line 197-205: We declare and implement the class `MyTestInfo3` in a similar way as `MyTestInfo1`.
Notice that the constructor uses the default `GetSuiteName()` function to retrieve the correct test suite name
- Line 208-213: We declare the class `MyTest`, which derives from `Test`
- Line 215-218: We implement the `RunImpl()` method for `MyTest` as a call to the `OnTestRun()` method on the `TestResults` instance
- Line 224-226: We instantiate each of `Suite1::MyTestInfo1`, `Suite2::MyTestInfo2` and `MyTestInfo3`, passing in the instance of the corresponding test
- Line 227-228: We instantiate a `TestFixtureInfo` as the first test fixture, and add `MyTest1`
- Line 229-230: We instantiate a `TestFixtureInfo` as our second test fixture, and add `MyTest2`
- Line 231-232: We instantiate a `TestFixtureInfo` as our third test fixture, and add `MyTest3`
- Line 234-235: We instantiate a `TestSuiteInfo` as our first test suite (with name `Suite1`), and add the first test fixture
- Line 236-237: We instantiate a `TestSuiteInfo` as our second test suite (with name `Suite2`), and add the second test fixture
- Line 238-239: We instantiate a `TestSuiteInfo` as our third test suite (with empty name), and add the third test fixture
- Line 240-242: We run the test suites
- Line 243-245: We clean up the test suites. Note that the test suite desctructor deletes all test fixtures and as part of that all tests, so we don't need to (and shouldn't) do that
- Line 246: We instantiate a `TestInfo`, linked to the `myTest` instance
- Line 248: We run the test

We've create even more infrastructure to define all test suites, test fixture, tests, and hook them up.
Be patient, we'll create macros later to do this work for us.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES__STEP_5_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests in the test suite, and therefore show the log output.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:201)
Info   MySuite1 Start suite (TestResults:79)
Info   MyFixture1 Start fixture (TestResults:97)
Info   DefaultSuite::MyFixture::MyTest1 Start test (TestResults:115)
Debug  Test 1 (main:68)
Debug  MyTest SetUp (main:33)
Debug  DefaultSuite::MyFixture::MyTest1 (../code/applications/demo/src/main.cpp:78) --> MyTestHelper 1 (TestResults:123)
Debug  MyTest TearDown (main:37)
Info   DefaultSuite::MyFixture::MyTest1 Finish test (TestResults:132)
Info   MyFixture1 Finish fixture (TestResults:106)
Info   MySuite1 Finish suite (TestResults:88)
Info   MySuite2 Start suite (TestResults:79)
Info   MyFixture2 Start fixture (TestResults:97)
Info   DefaultSuite::MyFixture::MyTest2 Start test (TestResults:115)
Debug  Test 2 (main:134)
Debug  FixtureMyTest2 SetUp (main:99)
Debug  DefaultSuite::MyFixture::MyTest2 (../code/applications/demo/src/main.cpp:144) --> MyTestHelper 2 (TestResults:123)
Debug  FixtureMyTest2 TearDown (main:103)
Info   DefaultSuite::MyFixture::MyTest2 Finish test (TestResults:132)
Info   MyFixture2 Finish fixture (TestResults:106)
Info   MySuite2 Finish suite (TestResults:88)
Info    Start suite (TestResults:79)
Info   MyFixture3 Start fixture (TestResults:97)
Info   DefaultSuite::MyFixture::MyTest3 Start test (TestResults:115)
Debug  Test 3 (main:193)
Debug  FixtureMyTest3 SetUp (main:158)
Debug  DefaultSuite::MyFixture::MyTest3 (../code/applications/demo/src/main.cpp:203) --> MyTestHelper 3 (TestResults:123)
Debug  FixtureMyTest3 TearDown (main:162)
Info   DefaultSuite::MyFixture::MyTest3 Finish test (TestResults:132)
Info   MyFixture3 Finish fixture (TestResults:106)
Info    Finish suite (TestResults:88)
Info   DefaultSuite::DefaultFixture::MyTest Start test (TestResults:115)
Debug  DefaultSuite::DefaultFixture::MyTest (../code/applications/demo/src/main.cpp:258) --> Running test (TestResults:123)
Info   DefaultSuite::DefaultFixture::MyTest Finish test (TestResults:132)
Info   Wait 5 seconds (main:263)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Test registration - Step 6 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6}

Now that we have all the classes defined to run tests, we still need to find a way to register them, such that we can ask a test runner to run all (or part of) the tests.
For this we will define a class `TestRegistry`, which will keep administration at the top level (the list of all test suites).
We will also define a class `TestRegistrar` which is used to register tests statically, so that before the application runs, the tests have already been registered.

### TestDetails.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_TESTDETAILSH}

We'll define some constants to identify the default test fixture and default test suite.
Then we'll update the methods to retrieve the test suite name and test fixture name.
We'll also add a methods to return the fully qualified test name, i.e. \<suite\>::\<fixture\>::\<test\>

Update the file `code/libraries/unittest/include/unittest/TestDetails.h`

```cpp
File: code/libraries/unittest/include/unittest/TestDetails.h
...
67: public:
68:     /// @brief Name of default test fixture. Used for tests that are not in a test fixture
69:     static const char* DefaultFixtureName;
70:     /// @brief Name of default test suite. Used for tests and test fixtures that are not in a test suite
71:     static const char* DefaultSuiteName;
72: 
73:     TestDetails();
74:     TestDetails(const baremetal::string& testName, const baremetal::string& fixtureName, const baremetal::string& suiteName, const baremetal::string& fileName, int lineNumber);
75:     TestDetails(const TestDetails& other, int lineNumber);
76: 
77:     baremetal::string SuiteName() const;
78:     baremetal::string FixtureName() const;
79:     baremetal::string TestName() const;
80:     baremetal::string QualifiedTestName() const;
81:     baremetal::string SourceFileName() const;
82:     int SourceFileLineNumber() const;
83: };
...
```

- Line 69-71: We declare constants to name default test fixture and default test suite
- Line 77: We remove the implementation for the method `SuiteName`, and change it to return a string, instead of a const string reference
- Line 78: We remove the implementation for the method `FixtureName`, and change it to return a string, instead of a const string reference
- Line 79: We remove the implementation for the method `TestName`, and change it to return a string, instead of a const string reference
- Line 80: We declare the method `QualifiedTestName`
- Line 81: We remove the implementation for the method `SourceFileName`, and change it to return a string, instead of a const string reference
- Line 82: We remove the implementation for the method `SourceFileLineNumber`

### TestDetails.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_TESTDETAILSCPP}

Let's update the implementation of `TestDetails`.

Update the file `code/libraries/unittest/src/TestDetails.cpp`

```cpp
File: code/libraries/unittest/src/TestDetails.cpp
...
40: #include <unittest/TestDetails.h>
41: 
42: #include <baremetal/Format.h>
...
51: const char* TestDetails::DefaultFixtureName = "DefaultFixture";
52: const char* TestDetails::DefaultSuiteName = "DefaultSuite";
...
97: /// <summary>
98: /// Returns test suite name
99: /// </summary>
100: /// <returns>Test suite name</returns>
101: string TestDetails::SuiteName() const
102: {
103:     return m_suiteName.empty() ? string(DefaultSuiteName) : m_suiteName;
104: }
105: 
106: /// <summary>
107: /// Returns test fixture name
108: /// </summary>
109: /// <returns>Test fixture name</returns>
110: string TestDetails::FixtureName() const
111: {
112:     return m_fixtureName.empty() ? string(DefaultFixtureName) : m_fixtureName;
113: }
114: 
115: /// <summary>
116: /// Returns test name
117: /// </summary>
118: /// <returns>Test name</returns>
119: string TestDetails::TestName() const
120: {
121:     return m_testName;
122: }
123: 
124: /// <summary>
125: /// Return fully qualified test name in format [suite]::[fixture]::[test]
126: /// </summary>
127: /// <returns>Resulting string</returns>
128: string TestDetails::QualifiedTestName() const
129: {
130:     return Format("%s::%s::%s",
131:         SuiteName().c_str(),
132:         FixtureName().c_str(),
133:         TestName().c_str());
134: }
135: 
136: /// <summary>
137: /// Returns test source file name
138: /// </summary>
139: /// <returns>Test source file name</returns>
140: string TestDetails::SourceFileName() const
141: {
142:     return m_fileName;
143: }
144: 
145: /// <summary>
146: /// Returns test source line number
147: /// </summary>
148: /// <returns>Test source line number</returns>
149: int TestDetails::SourceFileLineNumber() const
150: {
151:     return m_lineNumber;
152: }
153: 
154: } // namespace unittest
```

- Line 42: We include the header for `Format()`
- Line 51-52: We define the constants for the default text fixture name and test suite name
- Line 101-104: We implement the method `SuiteName()`, now using the default suite name if the name is empty
- Line 110-113: We implement the method `FixtureName()`, now using the default fixture name if the name is empty
- Line 119-122: We implement the method `TestName()`
- Line 128-134: We implement the method `QualifiedTestName()`
- Line 140-143: We implement the method `SourceFileName()`
- Line 149-152: We implement the method `SourceFileLineNumber()`

### TestRegistry.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_TESTREGISTRYH}

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
44: /// @file
45: /// Test registry
46: 
47: namespace unittest
48: {
49: 
50: class TestInfo;
51: class TestSuiteInfo;
52: class TestResults;
53: 
54: /// <summary>
55: /// Test registry
56: /// </summary>
57: class TestRegistry
58: {
59: private:
60:     friend class TestRegistrar;
61:     /// @brief Pointer to first test suite in the list
62:     TestSuiteInfo* m_head;
63:     /// @brief Pointer to last test suite in the list
64:     TestSuiteInfo* m_tail;
65: 
66: public:
67:     TestRegistry();
68:     TestRegistry(const TestRegistry&) = delete;
69:     TestRegistry(TestRegistry&&) = delete;
70:     virtual ~TestRegistry();
71: 
72:     TestRegistry& operator = (const TestRegistry&) = delete;
73:     TestRegistry& operator = (TestRegistry&&) = delete;
74: 
75:     /// <summary>
76:     /// Returns a pointer to the first test suite in the list
77:     /// </summary>
78:     /// <returns>Pointer to the first test suite in the list</returns>
79:     TestSuiteInfo* Head() const { return m_head; }
80: 
81:     void Run(TestResults& testResults);
82:     int CountSuites();
83:     int CountFixtures();
84:     int CountTests();
85: 
86:     static TestRegistry& GetTestRegistry();
87: 
88: private:
89:     TestSuiteInfo* GetTestSuite(const baremetal::string& suiteName);
90:     void AddSuite(TestSuiteInfo* testSuite);
91: };
92: 
93: /// <summary>
94: /// Test registrar
95: ///
96: /// This is a utility class to register a test to the registry, as part of a test declaration
97: /// </summary>
98: class TestRegistrar
99: {
100: public:
101:     TestRegistrar(TestRegistry& registry, Test* testInstance, const TestDetails& details);
102: };
103: 
104: } // namespace unittest
```

- Line 57-91: We declare the `Testregistry` class
  - Line 62-64: The member variables `m_head` and `m_tail` store the pointer to the first and last test suite
  - Line 67: We declare the default constructor
  - Line 68-69: We remove the copy and move constructors
  - Line 70: We declare the destructor
  - Line 72-73: We remove the assignment and move assignment operators
  - Line 79: The method `Head()` returns the pointer to the first test suite in the list
  - Line 81: The method `Run()` runs all test suites. We'll be revisiting this later
  - Line 82: The method `CountSuites()` counts and returns the number of test suites in the test suite
  - Line 83: The method `CountFixtures()` counts and returns the number of test fixtures in all test suites
  - Line 84: The method `CountTests()` counts and returns the number of tests in all test fixtures in all test suites
  - Line 86: The static method `GetTestRegistry()` returns the singleton test registry
  - Line 89: The method `GetTestSuite()` finds and returns a test suite in the list, or if not found, creates a new test suite with the specified name.
  Note that this method is private, as the `TestRegistrar` can access it as a friend class
  - Line 90: The method `AddSuite()` adds a test suite to the list.
  Note that this method is private, as the `TestRegistrar` can access it as a friend class

- Line 98-102: We declare the `TestRegistrar` class
  - Line 101: We declare the only method in the class, the constructor

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_TESTSUITEINFOH}

As the `TestRegistry` class needs access to the `TestSuiteInfo` in order to access the `m_next` pointer, we need to make it a friend class.

Update the file `code/libraries/unittest/include/unittest/TestSuiteInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuiteInfo.h
...
58: class TestSuiteInfo
59: {
60: private:
61:     friend class TestRegistry;
62:     /// @brief Pointer to first test fixture in the list
63:     TestFixtureInfo* m_head;
64:     /// @brief Pointer to last test fixture in the list
65:     TestFixtureInfo* m_tail;
66:     /// @brief Pointer to next test suite info in the list
67:     TestSuiteInfo* m_next;
68:     /// @brief Test suite name
69:     baremetal::string m_suiteName;
....
```

### TestRegistry.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_TESTREGISTRYCPP}

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
44: #include <unittest/TestInfo.h>
45: 
46: /// @file
47: /// Test registry implementation
48: 
49: using namespace baremetal;
50: 
51: /// @brief Define log name
52: LOG_MODULE("TestRegistry");
53: 
54: namespace unittest {
55: 
56: /// <summary>
57: /// Returns the test registry (singleton)
58: /// </summary>
59: /// <returns>Test registry reference</returns>
60: TestRegistry& TestRegistry::GetTestRegistry()
61: {
62:     static TestRegistry s_registry;
63:     return s_registry;
64: }
65: 
66: /// <summary>
67: /// Constructor
68: /// </summary>
69: TestRegistry::TestRegistry()
70:     : m_head{}
71:     , m_tail{}
72: {
73: }
74: 
75: /// <summary>
76: /// Destructor
77: ///
78: /// Cleans up all registered test suites, test fixtures and tests
79: /// </summary>
80: TestRegistry::~TestRegistry()
81: {
82:     TestSuiteInfo *testSuite = m_head;
83:     while (testSuite != nullptr)
84:     {
85:         TestSuiteInfo *currentSuite = testSuite;
86:         testSuite = testSuite->m_next;
87:         delete currentSuite;
88:     }
89: }
90: 
91: /// <summary>
92: /// Run tests, updating the test results
93: /// </summary>
94: /// <param name="testResults">Test results to use and update</param>
95: void TestRegistry::Run(TestResults& testResults)
96: {
97:     TestSuiteInfo* testSuite = Head();
98: 
99:     while (testSuite != nullptr)
100:     {
101:         testSuite->Run(testResults);
102:         testSuite = testSuite->m_next;
103:     }
104: }
105: 
106: /// <summary>
107: /// Count the number of test suites selected by the predicate
108: /// </summary>
109: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
110: /// <param name="predicate">Test selection predicate</param>
111: /// <returns>Number of test suites selected by the predicate</returns>
112: int TestRegistry::CountSuites()
113: {
114:     int numberOfTestSuites = 0;
115:     TestSuiteInfo* testSuite = Head();
116:     while (testSuite != nullptr)
117:     {
118:         ++numberOfTestSuites;
119:         testSuite = testSuite->m_next;
120:     }
121:     return numberOfTestSuites;
122: }
123: 
124: /// <summary>
125: /// Count the number of tests
126: /// </summary>
127: /// <returns>Number of test fixtures</returns>
128: int TestRegistry::CountFixtures()
129: {
130:     int numberOfTestFixtures = 0;
131:     TestSuiteInfo* testSuite = Head();
132:     while (testSuite != nullptr)
133:     {
134:         numberOfTestFixtures += testSuite->CountFixtures();
135:         testSuite = testSuite->m_next;
136:     }
137:     return numberOfTestFixtures;
138: }
139: 
140: /// <summary>
141: /// Count the number of tests
142: /// </summary>
143: /// <returns>Number of tests</returns>
144: int TestRegistry::CountTests()
145: {
146:     int numberOfTests = 0;
147:     TestSuiteInfo* testSuite = Head();
148:     while (testSuite != nullptr)
149:     {
150:         numberOfTests += testSuite->CountTests();
151:         testSuite = testSuite->m_next;
152:     }
153:     return numberOfTests;
154: }
155: 
156: /// <summary>
157: /// Find a test suite with specified name, register a new one if not found
158: /// </summary>
159: /// <param name="suiteName">Test suite name to search for</param>
160: /// <returns>Found or created test suite</returns>
161: TestSuiteInfo *TestRegistry::GetTestSuite(const string &suiteName)
162: {
163:     TestSuiteInfo *testSuite = m_head;
164:     while ((testSuite != nullptr) && (testSuite->Name() != suiteName))
165:         testSuite = testSuite->m_next;
166:     if (testSuite == nullptr)
167:     {
168: #if DEBUG_REGISTRY
169:         LOG_DEBUG("Find suite %s ... not found, creating new object", suiteName.c_str());
170: #endif
171:         testSuite = new TestSuiteInfo(suiteName);
172:         AddSuite(testSuite);
173:     }
174:     else
175:     {
176: #if DEBUG_REGISTRY
177:         LOG_DEBUG("Find suite %s ... found", suiteName.c_str());
178: #endif
179:     }
180:     return testSuite;
181: }
182: 
183: /// <summary>
184: /// Add a test suite
185: /// </summary>
186: /// <param name="testSuite">Test suite to add</param>
187: void TestRegistry::AddSuite(TestSuiteInfo *testSuite)
188: {
189:     if (m_tail == nullptr)
190:     {
191:         assert(m_head == nullptr);
192:         m_head = testSuite;
193:         m_tail = testSuite;
194:     }
195:     else
196:     {
197:         m_tail->m_next = testSuite;
198:         m_tail = testSuite;
199:     }
200: }
201: 
202: /// <summary>
203: /// Constructor
204: ///
205: /// Finds or registers the test suite specified in the test details of the test.
206: /// Finds or registers the test fixture specified in the test details of the test.
207: /// Adds the test to the fixture found or created.
208: /// </summary>
209: /// <param name="registry">Test registry</param>
210: /// <param name="testInstance">Test instance to link to</param>
211: /// <param name="details">Test details</param>
212: TestRegistrar::TestRegistrar(TestRegistry &registry, Test *testInstance, const TestDetails& details)
213: {
214: #if DEBUG_REGISTRY
215:     LOG_DEBUG("Register test %s in fixture %s in suite %s",
216:         details.TestName().c_str(),
217:         (details.FixtureName().c_str()),
218:         (details.SuiteName().c_str()));
219: #endif
220:     TestSuiteInfo   *testSuite   = registry.GetTestSuite(details.SuiteName());
221:     TestFixtureInfo *testFixture = testSuite->GetTestFixture(details.FixtureName());
222:     TestInfo        *test = new TestInfo(testInstance, details);
223:     testFixture->AddTest(test);
224: }
225: 
226: } // namespace unittest
```

- Line 60-64: We implement the static method `GetTestRegistry()`
- Line 69-73: We implement the constructor
- Line 80-89: We implement the destructor. This goes through the list of test suites, and deletes every one of these. Note that we will therefore need to create the test suites on the heap.
- Line 95-104: We implement the `Run()` method. This goes through the list of test suites, and calls `Run()` on each
- Line 112-122: We implement the `CountSuites()` method. This goes through the list of test suites, and counts them
- Line 128-138: We implement the `CountFixtures()` method. This goes through the list of test suites, and counts the test fixtures in each of them
- Line 144-154: We implement the `CountTests()` method. This goes through the list of test suites, and counts the tests in each of them
- Line 161-181: We implement the `GetTestSuite()` method. This will try to find the test suite with the specified name. If it is found, the pointer is returned, if not, a new instance iscreated
- Line 187-200: We implement the `AddSuite()` method. This will add the test suite passed in at the end of the list
- Line 212-224: We implement `TestRegistrar` contructor. This is intended for the macros we will get to next.
The constructor receives a reference to the test registry, as well as a pointer to a test to link to (derived from `Test`) and test details, and adds the test to the registry.
The `TestDetails` are used to create a test administraton instance, and lookup and if needed create the surrounding test fixture and test suite

### TestSuiteInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_TESTSUITEINFOCPP}

Now that we have a `TestRegistry`, we can update the method `GetTestFixture` in `TestSuiteInfo` to add some debug info.

Update the file `code/libraries/unittest/src/TestSuiteInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestSuiteInfo.cpp
...
84: /// <summary>
85: /// Find a test fixture with specified name, register a new one if not found
86: /// </summary>
87: /// <param name="fixtureName">Test fixture name to search for</param>
88: /// <returns>Found or created test fixture</returns>
89: TestFixtureInfo *TestSuiteInfo::GetTestFixture(const string &fixtureName)
90: {
91:     TestFixtureInfo *testFixture = m_head;
92:     while ((testFixture != nullptr) && (testFixture->Name() != fixtureName))
93:         testFixture = testFixture->m_next;
94:     if (testFixture == nullptr)
95:     {
96: #if DEBUG_REGISTRY
97:         LOG_DEBUG("Fixture %s not found, creating new object", fixtureName.c_str());
98: #endif
99:         testFixture = new TestFixtureInfo(fixtureName);
100:         AddFixture(testFixture);
101:     }
102:     else
103:     {
104: #if DEBUG_REGISTRY
105:         LOG_DEBUG("Fixture %s found", fixtureName.c_str());
106: #endif
107:     }
108:     return testFixture;
109: }
...
```

### Debugging registry {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_DEBUGGING_REGISTRY}

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

### Update CMake file {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_UPDATE_CMAKE_FILE}

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
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
39:     )
40: 
41: set(PROJECT_INCLUDES_PUBLIC
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Test.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestInfo.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
52:     )
53: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_UPDATE_APPLICATION_CODE}

Now we can use the TestRegistry. Even though we still need to create quite some classes, the plumbing of registering the classes is now taken care of by the `TestRegistry` class, and instances of the `TestRegistrar` class.

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
12: #include <unittest/TestRegistry.h>
13: #include <unittest/TestResults.h>
14: #include <unittest/TestSuite.h>
15: #include <unittest/TestSuiteInfo.h>
16: 
17: LOG_MODULE("main");
18: 
19: using namespace baremetal;
20: using namespace unittest;
21: 
22: namespace Suite1 {
23: 
24: inline char const* GetSuiteName()
25: {
26:     return baremetal::string("Suite1");
27: }
28: 
29: class FixtureMyTest1
30:     : public TestFixture
31: {
32: public:
33:     void SetUp() override
34:     {
35:         LOG_DEBUG("MyTest SetUp");
36:     }
37:     void TearDown() override
38:     {
39:         LOG_DEBUG("MyTest TearDown");
40:     }
41: };
42: 
43: class FixtureMyTest1Helper
44:     : public FixtureMyTest1
45: {
46: public:
47:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
48:     explicit FixtureMyTest1Helper(const TestDetails& details)
49:         : m_details{ details }
50:     {
51:         SetUp();
52:     }
53:     virtual ~FixtureMyTest1Helper()
54:     {
55:         TearDown();
56:     }
57:     void RunImpl() const;
58:     const TestDetails& m_details;
59: };
60: void FixtureMyTest1Helper::RunImpl() const
61: {
62:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 1");
63: }
64: 
65: class MyTest1
66:     : public Test
67: {
68:     void RunImpl() const override;
69: } myTest1;
70: 
71: TestRegistrar registrarFixtureMyTest1(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__));
72: 
73: void MyTest1::RunImpl() const
74: {
75:     LOG_DEBUG("Test 1");
76:     FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
77:     fixtureHelper.RunImpl();
78: }
79: 
80: } // namespace Suite1
81: 
82: namespace Suite2 {
83: 
84: inline char const* GetSuiteName()
85: {
86:     return baremetal::string("Suite2");
87: }
88: 
89: class FixtureMyTest2
90:     : public TestFixture
91: {
92: public:
93:     void SetUp() override
94:     {
95:         LOG_DEBUG("FixtureMyTest2 SetUp");
96:     }
97:     void TearDown() override
98:     {
99:         LOG_DEBUG("FixtureMyTest2 TearDown");
100:     }
101: };
102: 
103: class FixtureMyTest2Helper
104:     : public FixtureMyTest2
105: {
106: public:
107:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
108:     explicit FixtureMyTest2Helper(const TestDetails& details)
109:         : m_details{ details }
110:     {
111:         SetUp();
112:     }
113:     virtual ~FixtureMyTest2Helper()
114:     {
115:         TearDown();
116:     }
117:     void RunImpl() const;
118:     const TestDetails& m_details;
119: };
120: void FixtureMyTest2Helper::RunImpl() const
121: {
122:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 2");
123: }
124: 
125: class MyTest2
126:     : public Test
127: {
128:     void RunImpl() const override;
129: } myTest1;
130: 
131: TestRegistrar registrarFixtureMyTest2(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__));
132: 
133: void MyTest2::RunImpl() const
134: {
135:     LOG_DEBUG("Test 2");
136:     FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
137:     fixtureHelper.RunImpl();
138: }
139: 
140: } // namespace Suite2
141: 
142: class FixtureMyTest3
143:     : public TestFixture
144: {
145: public:
146:     void SetUp() override
147:     {
148:         LOG_DEBUG("FixtureMyTest3 SetUp");
149:     }
150:     void TearDown() override
151:     {
152:         LOG_DEBUG("FixtureMyTest3 TearDown");
153:     }
154: };
155: 
156: class FixtureMyTest3Helper
157:     : public FixtureMyTest3
158: {
159: public:
160:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
161:     explicit FixtureMyTest3Helper(const TestDetails& details)
162:         : m_details{ details }
163:     {
164:         SetUp();
165:     }
166:     virtual ~FixtureMyTest3Helper()
167:     {
168:         TearDown();
169:     }
170:     void RunImpl() const;
171:     const TestDetails& m_details;
172: };
173: void FixtureMyTest3Helper::RunImpl() const
174: {
175:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 3");
176: }
177: 
178: class MyTest3
179:     : public Test
180: {
181:     void RunImpl() const override;
182: } myTest3;
183: 
184: TestRegistrar registrarFixtureMyTest3(TestRegistry::GetTestRegistry(), &myTest3, TestDetails("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__));
185: 
186: void MyTest3::RunImpl() const
187: {
188:     LOG_DEBUG("Test 3");
189:     FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
190:     fixtureHelper.RunImpl();
191: }
192: 
193: class MyTest
194:     : public Test
195: {
196: public:
197:     void RunImpl() const override;
198: } myTest;
199: 
200: TestRegistrar registrarMyTest(TestRegistry::GetTestRegistry(), &myTest, TestDetails("MyTest", "", "", __FILE__, __LINE__));
201: 
202: void MyTest::RunImpl() const
203: {
204:     CurrentTest::Results()->OnTestRun(*CurrentTest::Details(), "Running test");
205: }
206: 
207: int main()
208: {
209:     auto& console = GetConsole();
210: 
211:     unittest::TestResults results;
212:     TestRegistry::GetTestRegistry().Run(results);
213: 
214:     LOG_INFO("Wait 5 seconds");
215:     Timer::WaitMilliSeconds(5000);
216: 
217:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
218:     char ch{};
219:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
220:     {
221:         ch = console.ReadChar();
222:         console.WriteChar(ch);
223:     }
224:     if (ch == 'p')
225:         assert(false);
226: 
227:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
228: }
```

- Line 12: We include the header for `TestRegistry` and `TestRegistrar`
- Line 22: We define the namespace Suite1 as before
- Line 24-27: We define the namespace specific version of `GetSuiteName()` as before
- Line 29-41: We declare and implement a class `FixtureMyTest1` as before
- Line 43-63: We declare and implement a class `FixtureMyTest1Helper` as before
- Line 65-69: We declare the class `MyTest1` as before.
The `RunImpl()` method is now defined later
- Line 71: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest1` instance just defined, as well as constructed test details
- Line 73-78: We implement the `RunImpl()` method of `MyTest1` as before
- Line 80: We end the namespace Suite1 as before
- Line 82: We define the namespace Suite2 as before
- Line 84-87: We define the namespace specific version of `GetSuiteName()` as before
- Line 89-101: We declare and implement a class `FixtureMyTest2` as before
- Line 103-123: We declare and implement a class `FixtureMyTest2Helper` as before
- Line 125-129: We declare the class `MyTest2`as before.
The `RunImpl()` method is now defined later
- Line 131: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest2` instance just defined
- Line 133-138: We implement the `RunImpl()` method of `MyTest2` as before
- Line 140: We end the namespace Suite2 as before
- Line 142-154: We declare and implement a class `FixtureMyTest3` as before
- Line 156-176: We declare and implement a class `FixtureMyTest3Helper` as before
- Line 178-182: We declare the class `MyTest3` as before.
The `RunImpl()` method is now defined later
- Line 184: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest3` instance just defined
- Line 186-191: We implement the `RunImpl()` method of `MyTest3` as before
- Line 226: We retrieve the test registry, and call its `Run()` method
- Line 193-198: We declare the class `MyTest` as before.
The `RunImpl()` method is now defined later
- Line 200: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest` instance just defined
- Line 202-205: We implement the `RunImpl()` method of `MyTest3` as before
- Line 212: We retrieve the test registry, and call its `Run()` method

Note that we don't need to clean up anymore, that is all taken care of.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION__STEP_6_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will, at static initialization time, register all the tests, and then run all tests registered.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Debug  Register test MyTest1 in fixture FixtureMyTest1 in suite Suite1 (TestRegistry:218)
Debug  Find suite Suite1 ... not found, creating new object (TestRegistry:172)
Debug  Fixture FixtureMyTest1 not found, creating new object (TestSuiteInfo:97)
Debug  Register test MyTest2 in fixture FixtureMyTest2 in suite Suite2 (TestRegistry:218)
Debug  Find suite Suite2 ... not found, creating new object (TestRegistry:172)
Debug  Fixture FixtureMyTest2 not found, creating new object (TestSuiteInfo:97)
Debug  Register test MyTest3 in fixture FixtureMyTest3 in suite DefaultSuite (TestRegistry:218)
Debug  Find suite DefaultSuite ... not found, creating new object (TestRegistry:172)
Debug  Fixture FixtureMyTest3 not found, creating new object (TestSuiteInfo:97)
Debug  Register test MyTest in fixture DefaultFixture in suite DefaultSuite (TestRegistry:218)
Debug  Find suite DefaultSuite ... found (TestRegistry:180)
Debug  Fixture DefaultFixture not found, creating new object (TestSuiteInfo:97)
Info   Starting up (System:201)
Info   Suite1 Start suite (TestResults:100)
Info   FixtureMyTest1 Start fixture (TestResults:118)
Info   Suite1::FixtureMyTest1::MyTest1 Start test (TestResults:136)
Debug  Test 1 (main:75)
Debug  MyTest SetUp (main:35)
Debug  Suite1::FixtureMyTest1::MyTest1 (../code/applications/demo/src/main.cpp:71) --> MyTestHelper 1 (TestResults:144)
Debug  MyTest TearDown (main:39)
Info   Suite1::FixtureMyTest1::MyTest1 Finish test (TestResults:153)
Info   FixtureMyTest1 Finish fixture (TestResults:127)
Info   Suite1 Finish suite (TestResults:109)
Info   Suite2 Start suite (TestResults:100)
Info   FixtureMyTest2 Start fixture (TestResults:118)
Info   Suite2::FixtureMyTest2::MyTest2 Start test (TestResults:136)
Debug  Test 2 (main:135)
Debug  FixtureMyTest2 SetUp (main:95)
Debug  Suite2::FixtureMyTest2::MyTest2 (../code/applications/demo/src/main.cpp:131) --> MyTestHelper 2 (TestResults:144)
Debug  FixtureMyTest2 TearDown (main:99)
Info   Suite2::FixtureMyTest2::MyTest2 Finish test (TestResults:153)
Info   FixtureMyTest2 Finish fixture (TestResults:127)
Info   Suite2 Finish suite (TestResults:109)
Info   DefaultSuite Start suite (TestResults:100)
Info   FixtureMyTest3 Start fixture (TestResults:118)
Info   DefaultSuite::FixtureMyTest3::MyTest3 Start test (TestResults:136)
Debug  Test 3 (main:188)
Debug  FixtureMyTest3 SetUp (main:148)
Debug  DefaultSuite::FixtureMyTest3::MyTest3 (../code/applications/demo/src/main.cpp:184) --> MyTestHelper 3 (TestResults:144)
Debug  FixtureMyTest3 TearDown (main:152)
Info   DefaultSuite::FixtureMyTest3::MyTest3 Finish test (TestResults:153)
Info   FixtureMyTest3 Finish fixture (TestResults:127)
Info   DefaultFixture Start fixture (TestResults:118)
Info   DefaultSuite::DefaultFixture::MyTest Start test (TestResults:136)
Debug  DefaultSuite::DefaultFixture::MyTest (../code/applications/demo/src/main.cpp:200) --> Running test (TestResults:144)
Info   DefaultSuite::DefaultFixture::MyTest Finish test (TestResults:153)
Info   DefaultFixture Finish fixture (TestResults:127)
Info   DefaultSuite Finish suite (TestResults:109)
Info   Wait 5 seconds (main:214)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Test runner and visitor - Step 7 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7}

### ITestReporter.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_ITESTREPORTERH}

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
44: /// @file
45: /// Abstract test reporter interface
46: 
47: namespace unittest
48: {
49: 
50: class TestDetails;
51: class TestResults;
52: 
53: /// <summary>
54: /// Test reporter abstract interface
55: /// </summary>
56: class ITestReporter
57: {
58: public:
59:     /// <summary>
60:     /// Destructor
61:     /// </summary>
62:     virtual ~ITestReporter() {}
63: 
64:     /// <summary>
65:     /// Start of test run callback
66:     /// </summary>
67:     /// <param name="numberOfTestSuites">Number of test suites to be run</param>
68:     /// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
69:     /// <param name="numberOfTests">Number of tests to be run</param>
70:     virtual void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
71:     /// <summary>
72:     /// Finish of test run callback
73:     /// </summary>
74:     /// <param name="numberOfTestSuites">Number of test suites run</param>
75:     /// <param name="numberOfTestFixtures">Number of test fixtures run</param>
76:     /// <param name="numberOfTests">Number of tests run</param>
77:     virtual void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
78:     /// <summary>
79:     /// Test summary callback
80:     /// </summary>
81:     /// <param name="results">Test run results</param>
82:     virtual void ReportTestRunSummary(const TestResults& results) = 0;
83:     /// <summary>
84:     /// Test run overview callback
85:     /// </summary>
86:     /// <param name="results">Test run results</param>
87:     virtual void ReportTestRunOverview(const TestResults& results) = 0;
88:     /// <summary>
89:     /// Test suite start callback
90:     /// </summary>
91:     /// <param name="suiteName">Test suite name</param>
92:     /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
93:     virtual void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTestFixtures) = 0;
94:     /// <summary>
95:     /// Test suite finish callback
96:     /// </summary>
97:     /// <param name="suiteName">Test suite name</param>
98:     /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
99:     virtual void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTestFixtures) = 0;
100:     /// <summary>
101:     /// Test fixture start callback
102:     /// </summary>
103:     /// <param name="fixtureName">Test fixture name</param>
104:     /// <param name="numberOfTests">Number of tests within test fixture</param>
105:     virtual void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) = 0;
106:     /// <summary>
107:     /// Test fixture finish callback
108:     /// </summary>
109:     /// <param name="fixtureName">Test fixture name</param>
110:     /// <param name="numberOfTests">Number of tests within test fixture</param>
111:     virtual void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests) = 0;
112:     /// <summary>
113:     /// Test start callback
114:     /// </summary>
115:     /// <param name="details">Test details</param>
116:     virtual void ReportTestStart(const TestDetails& details) = 0;
117:     /// <summary>
118:     /// Test finish callback
119:     /// </summary>
120:     /// <param name="details">Test details</param>
121:     /// <param name="success">Test result, true is successful, false is failed</param>
122:     virtual void ReportTestFinish(const TestDetails& details, bool success) = 0;
123:     /// <summary>
124:     /// Test failure callback
125:     /// </summary>
126:     /// <param name="details">Test details</param>
127:     /// <param name="failure">Test failure message</param>
128:     virtual void ReportTestFailure(const TestDetails& details, const baremetal::string& failure) = 0;
129: };
130: 
131: } // namespace unittest
```

- Line 56-129: We declare the abstract interface class ITestReporter
  - Line 70: The method `ReportTestRunStart()` will be called before any tests are run, to give an overview of the number of test suites, test fixtures and tests
  - Line 77: The method `ReportTestRunStart()` will be called just after the tests are run, to again give an overview of the number of test suites, test fixtures and tests
  - Line 82: The method `ReportTestSummary()` will be called after all tests are finished, to show a summary of the number of tests run, and the number of failing tests
  - Line 87: The method `ReportTestRunOverview()` will print the list of failed tests, with information of why they failed
  - Line 93: The method `ReportTestSuiteStart()` will print a marker for the start of a test suite run
  - Line 99: The method `ReportTestSuiteFinish()` will print a marker for the end of a test suite run
  - Line 105: The method `ReportTestFixtureStart()` will print a marker for the start of a test fixture run
  - Line 111: The method `ReportTestFixtureFinish()` will print a marker for the end of a test fixture run
  - Line 116: The method `ReportTestStart()` will print a marker for the start of a test run
  - Line 122: The method `ReportTestFinish()` will print a marker for the end of a test run
  - Line 128: The method `ReportTestFailure()` will print a statement that a failure was found, with information

### TestResults.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTRESULTSH}

We introduced the `TestResults` class before, we will now extend it to keep hold of the test reporter, and to count tests, test fixtures and test suites.

Update the file `code/libraries/unittest/include/unittest/TestResults.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResults.h
40: #pragma once
41: 
42: #include <baremetal/String.h>
43: 
44: /// @file
45: /// Test results
46: ///
47: /// Results for a complete test run
48: 
49: namespace unittest
50: {
51: 
52: class ITestReporter;
53: class TestDetails;
54: class TestSuiteInfo;
55: class TestFixtureInfo;
56: 
57: /// <summary>
58: /// Test results
59: ///
60: /// Holds the test results for a complete test run
61: /// </summary>
62: class TestResults
63: {
64: private:
65:     /// @brief Selected test reporter
66:     ITestReporter* m_reporter;
67:     /// @brief Total count fo tests in test run
68:     int m_totalTestCount;
69:     /// @brief Total count of failed tests in test run
70:     int m_failedTestCount;
71:     /// @brief Total count of failures in test run
72:     int m_failureCount;
73:     /// @brief Flag for failure in current test, set to true if at least one failure occurred in the current test
74:     bool m_currentTestFailed;
75: 
76: public:
77:     explicit TestResults(ITestReporter* reporter = nullptr);
78:     TestResults(const TestResults&) = delete;
79:     TestResults(TestResults&&) = delete;
80:     virtual ~TestResults();
81: 
82:     TestResults& operator = (const TestResults&) = delete;
83:     TestResults& operator = (TestResults&&) = delete;
84: 
85:     void OnTestSuiteStart(TestSuiteInfo* suite);
86:     void OnTestSuiteFinish(TestSuiteInfo* suite);
87:     void OnTestFixtureStart(TestFixtureInfo* fixture);
88:     void OnTestFixtureFinish(TestFixtureInfo* fixture);
89:     void OnTestStart(const TestDetails& details);
90:     void OnTestFailure(const TestDetails& details, const baremetal::string& message);
91:     void OnTestFinish(const TestDetails& details);
92: 
93:     /// <summary>
94:     /// Returns the number of tests in the test run
95:     /// </summary>
96:     /// <returns>Number of tests in the test run</returns>
97:     int GetTotalTestCount() const { return m_totalTestCount; }
98:     /// <summary>
99:     /// Returns the number of failed tests in the test run
100:     /// </summary>
101:     /// <returns>Number of failed tests in the test run</returns>
102:     int GetFailedTestCount() const { return m_failedTestCount; }
103:     /// <summary>
104:     /// Returns the number of failures in the test run
105:     /// </summary>
106:     /// <returns>Number of failures in the test run</returns>
107:     int GetFailureCount() const { return m_failureCount; }
108: 
109: };
110: 
111: } // namespace unittest
```

- Line 66: The member `m_reporter` holds the pointer to the test reporter interface being used
- Line 68: The member `m_failedTestCount` holds the total failed test count
- Line 70: The member `m_failureCount` holds the total failure count. A test can have multiple failures
- Line 72: The member `m_currentTestFailed` is true if any test case for the current test failed
- Line 77: We declare an explicit constructor taking a pointer to a test reporter interface
- Line 78-79: We remove the copy and move constructors
- Line 80: We declare the destructor
- Line 82-83: We remove the assignment and move assignment operators
- Line 85: The method `OnTestSuiteStart()` marks the start of a test suite run. This will also call `ReportTestSuiteStart()` on the test reporter
- Line 86: The method `OnTestSuiteFinish()` marks the end of a test suite run. This will also call `ReportTestSuiteFinish()` on the test reporter
- Line 87: The method `OnTestFixtureStart()` marks the start of a test fixture run. This will also call `ReportTestFixtureStart()` on the test reporter
- Line 88: The method `OnTestFixtureFinish()` marks the end of a test fixture run. This will also call `ReportTestFixtureFinish()` on the test reporter
- Line 89: The method `OnTestStart()` marks the start of a test run. This will also call `ReportTestStart()` on the test reporter
- Line 90: The method `OnTestFailure()` marks the failure in a test run.
This will also call `ReportTestFailure()` on the test reporter. This is the only method from the previous declaration that changed
- Line 91: The method `OnTestFinish()` marks the end of a test run. This will also call `ReportTestFinish()` on the test reporter
- Line 97: The method `GetTotalTestCount()` returns the total number of tests run
- Line 102: The method `GetFailedTestCount()` returns the total number of tests that failed
- Line 107: The method `GetFailureCount()` returns the total number of failures found. A test can have more than one failure

### TestResults.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTRESULTSCPP}

Let's implement additional methods of the `TestResults` class.

Update the file `code/libraries/unittest/src/TestResults.cpp`

```cpp
File: code/libraries/unittest/src/TestResults.cpp
40: #include <unittest/TestResults.h>
41: 
42: #include <unittest/ITestReporter.h>
43: #include <unittest/TestDetails.h>
44: #include <unittest/TestFixtureInfo.h>
45: #include <unittest/TestRegistry.h>
46: #include <unittest/TestSuiteInfo.h>
47: 
48: /// @file
49: /// Test results implementation
50: 
51: using namespace baremetal;
52: 
53: namespace unittest
54: {
55: 
56: /// <summary>
57: /// Constructor
58: /// </summary>
59: /// <param name="testReporter">Test reporter to use</param>
60: TestResults::TestResults(ITestReporter* testReporter)
61:     : m_reporter{ testReporter }
62:     , m_totalTestCount{}
63:     , m_failedTestCount{}
64:     , m_failureCount{}
65:     , m_currentTestFailed{}
66: {
67: }
68: 
69: /// <summary>
70: /// Destructor
71: /// </summary>
72: TestResults::~TestResults()
73: {
74: }
75: 
76: /// <summary>
77: /// Start a test suite run
78: /// </summary>
79: /// <param name="suite">Test suite to start</param>
80: void TestResults::OnTestSuiteStart(TestSuiteInfo* suite)
81: {
82:     if (m_reporter)
83:         m_reporter->ReportTestSuiteStart(suite->Name(), suite->CountFixtures());
84: }
85: 
86: /// <summary>
87: /// Finish a test suite run
88: /// </summary>
89: /// <param name="suite">Test suite to finish</param>
90: void TestResults::OnTestSuiteFinish(TestSuiteInfo* suite)
91: {
92:     if (m_reporter)
93:         m_reporter->ReportTestSuiteFinish(suite->Name(), suite->CountFixtures());
94: }
95: 
96: /// <summary>
97: /// Start a test fixture run
98: /// </summary>
99: /// <param name="fixture">Test fixture to start</param>
100: void TestResults::OnTestFixtureStart(TestFixtureInfo* fixture)
101: {
102:     if (m_reporter)
103:         m_reporter->ReportTestFixtureStart(fixture->Name(), fixture->CountTests());
104: }
105: 
106: /// <summary>
107: /// Finish a test fixture run
108: /// </summary>
109: /// <param name="fixture">Test fixture to finish</param>
110: void TestResults::OnTestFixtureFinish(TestFixtureInfo* fixture)
111: {
112:     if (m_reporter)
113:         m_reporter->ReportTestFixtureFinish(fixture->Name(), fixture->CountTests());
114: }
115: 
116: /// <summary>
117: /// Start a test
118: /// </summary>
119: /// <param name="details">Test details of test to start</param>
120: void TestResults::OnTestStart(const TestDetails& details)
121: {
122:     ++m_totalTestCount;
123:     m_currentTestFailed = false;
124:     if (m_reporter)
125:         m_reporter->ReportTestStart(details);
126: }
127: 
128: /// <summary>
129: /// Add a test failure
130: /// </summary>
131: /// <param name="details"></param>
132: /// <param name="message">Test failure string</param>
133: void TestResults::OnTestFailure(const TestDetails& details, const string& message)
134: {
135:     ++m_failureCount;
136:     if (!m_currentTestFailed)
137:     {
138:         ++m_failedTestCount;
139:         m_currentTestFailed = true;
140:     }
141:     if (m_reporter)
142:         m_reporter->ReportTestFailure(details, message);
143: }
144: 
145: /// <summary>
146: /// Finish a test
147: /// </summary>
148: /// <param name="details">Test details of test to finish</param>
149: void TestResults::OnTestFinish(const TestDetails& details)
150: {
151:     if (m_reporter)
152:         m_reporter->ReportTestFinish(details, !m_currentTestFailed);
153: }
154: 
155: } // namespace unittest
```

Most methods are quite straightforward.

- Line 120-126: The `OnTestStart()` method also counts the tests run, and resets the flag whether the test failed
- Line 133-143: The `OnTestFailure()` method increments the failure count, and if this is the first failure in the test, also increments the failed test count

### TestRunner.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTRUNNERH}

Next we define a class `TestRunner` that can run the registered tests.
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
50: class TestInfo;
51: class TestFixtureInfo;
52: class TestResults;
53: class TestSuiteInfo;
54:
55: struct True
56: {
57:     bool operator()(const TestInfo* const ) const
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
85:     bool operator()(const TestInfo* const test) const;
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

- Line 62-88: We declare a struct `True` the holds three operators, that all return true. This isused for filtering test suites, test fixtures and tests
  - Line 68-71: The operator `(const TestInfo*)` will match to any test
  - Line 76-79: The operator `(const TestFixtureInfo*)` will match to any test fixture
  - Line 84-87: The operator `(const TestSuiteInfo*)` will match to any test suite
- Line 95-121: We declare a similar struct `InSelection` that holds the same three operators.
This one however will returns true for a method, if the corresponding test name, test fixture name or test suite name is equal to a set value
  - Line 99-103: We declare class variables `m_suitename`, `m_fixtureName` and `m_testName` as a pointer to a string to contain the  test suite name, test fixture name, and test name to use as a filter
  - Line 112-117: We define a constructor, which takes a test suite name, a test fixture name, and a test name, each as a character pointer, which may be nullptr.
The corresponding values are saved as the test suite name, a test fixture name, and a test name to filter on
  - Line 118: We declare the operator `(const TestInfo*)` which will return true if the test name is equal to the set filter value `m_testName`
  - Line 119: We declare the operator `(const TestFixtureInfo*)` which will return true if the test fixture name is equal to the set filter value `m_fixtureName`
  - Line 120: We declare the operator `(const TestSuiteInfo*)` which will return true if the test suite name is equal to the set filter value `m_suiteName`
- Line 128-152: We declare the class `TestRunner` which allows to run tests with a set filter, which will use a test reporter instance for reporting
  - Line 132: `m_reporter` stores the passed test reporter instance pointer
  - Line 134: `m_testResults` stores a pointer to the test results
  - Line 137: We remove the copy constructor
  - Line 138: We declare an explicit constructor taking a test reporter instance pointer
  - Line 139: We declare a destructor
  - Line 141: We remove the assignment operator
  - Line 144: We declare a template method `RunTestsIf()` that takes a predicate (which could be an instance of the `True` class or an instance or the `InSelection` class, or any other class that supportes the same three `()` operators).
This method will run any test that matches the predicate passed
  - Line 147-148: We declare a private template method `Start()` that takes a predicate.
This collects information on the number of test suites, test fixtures and tests, and report the start of the test run
  - Line 150-151: We declare a private template method `Finish()` that takes a predicate.
This reports a test run summary and overview, and reports the end of the test run
- Line 163-171: We implement the `RunTestsIf()` template method.
This will use the method `RunIf()` to run tests matching the predicate, with the test results passed to fill
- Line 178-186: We implement the `Start()` template method.
Note that it runs methods `CountSuitesIf()`, `CountFixturesIf()` and `CountTestsIf()` to count test suites, test fixtures and test, using the predicate.
We will need to implement those in the `TestRegistry` class
- Line 194-207: We implement the `Finish()` template method.
Again, the methods `CountSuitesIf()`, `CountFixturesIf()` and `CountTestsIf()` are used to count tests, etc.
The method returns the number of failures, which is reported back by the `RunTestsIf()` template method
- Line 209: We declare a function `RunAllTests()` which will simply run all tests and use the passed test reporter for reporting
- Line 218-223: We define a template function `RunSelectedTests()` which will run all tests matching a predicate and use the passed test reporter for reporting

### TestRunner.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTRUNNERCPP}

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
42: /// @file
43: /// Test runner implementation
44: 
45: namespace unittest {
46: 
47: /// <summary>
48: /// Returns test selection value
49: /// </summary>
50: /// <param name="test">Test to check against selection</param>
51: /// <returns>Returns true if the test name selection is set to nullptr, or the test name matches the selection</returns>
52: bool InSelection::operator()(const TestInfo* const test) const
53: {
54:     return (m_testName == nullptr) || (test->Details().TestName() == m_testName);
55: }
56: 
57: /// <summary>
58: /// Returns test fixture selection value
59: /// </summary>
60: /// <param name="fixture">Test fixture to check against selection</param>
61: /// <returns>Returns true if the test fixture name selection is set to nullptr, or the test fixture name matches the selection</returns>
62: bool InSelection::operator()(const TestFixtureInfo* const fixture) const
63: {
64:     return (m_fixtureName == nullptr) || (fixture->Name() == m_fixtureName);
65: }
66: 
67: /// <summary>
68: /// Returns test suite selection value
69: /// </summary>
70: /// <param name="suite">Test suite to check against selection</param>
71: /// <returns>Returns true if the test suite name selection is set to nullptr, or the test suite name matches the selection</returns>
72: bool InSelection::operator()(const TestSuiteInfo* const suite) const
73: {
74:     return (m_suiteName == nullptr) || (suite->Name() == m_suiteName);
75: }
76: 
77: /// <summary>
78: /// Constructor
79: /// </summary>
80: /// <param name="reporter">Test reporter to use, can be nullptr, in which case no reporting is done</param>
81: TestRunner::TestRunner(ITestReporter* reporter)
82:     : m_reporter{ reporter }
83:     , m_testResults{ reporter }
84: {
85: }
86: 
87: /// <summary>
88: /// Destructor
89: /// </summary>
90: TestRunner::~TestRunner()
91: {
92: }
93: 
94: /// <summary>
95: /// Run all tests with specified test reporter
96: /// </summary>
97: /// <param name="reporter">Test reporter to use</param>
98: /// <returns></returns>
99: int RunAllTests(ITestReporter* reporter)
100: {
101:     return RunSelectedTests(reporter, True());
102: }
103: 
104: } // namespace unittest
```

- Line 52-55: We implement the operator `(const TestInfo*)` for `InSelection`. This will return true if the test name matches the filter
- Line 62-65: We implement the operator `(const TestFixtureInfo*)` for `InSelection`. This will return true if the test fixture name matches the filter
- Line 72-75: We implement the operator `(const TestSuiteInfo*)` for `InSelection`. This will return true if the test suite name matches the filter
- Line 81-85: We define the constructor for `TestRunner`
- Line 90-92: We define the destructor for `TestRunner`
- Line 99-102: We implement the function `RunAllTests()`

### TestRegistry.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTREGISTRYH}

In order to be able to run tests with a predicate, we need to update the `TestRegistry` class.

Update the file `code/libraries/unittest/include/unittest/TestRegistry.h`

```cpp
File: code/libraries/unittest/include/unittest/TestRegistry.h
...
57: class TestRegistry
58: {
59: private:
60:     friend class TestRegistrar;
61:     /// @brief Pointer to first test suite in the list
62:     TestSuiteInfo* m_head;
63:     /// @brief Pointer to last test suite in the list
64:     TestSuiteInfo* m_tail;
65: 
66: public:
67:     TestRegistry();
68:     TestRegistry(const TestRegistry&) = delete;
69:     TestRegistry(TestRegistry&&) = delete;
70:     virtual ~TestRegistry();
71: 
72:     TestRegistry& operator = (const TestRegistry&) = delete;
73:     TestRegistry& operator = (TestRegistry&&) = delete;
74: 
75:     /// <summary>
76:     /// Returns a pointer to the first test suite in the list
77:     /// </summary>
78:     /// <returns>Pointer to the first test suite in the list</returns>
79:     TestSuiteInfo* Head() const { return m_head; }
80: 
81:     template <typename Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
82:     template <typename Predicate> int CountSuitesIf(Predicate predicate);
83:     template <typename Predicate> int CountFixturesIf(Predicate predicate);
84:     template <typename Predicate> int CountTestsIf(Predicate predicate);
85: 
86:     static TestRegistry& GetTestRegistry();
87: 
88: private:
89:     TestSuiteInfo* GetTestSuite(const baremetal::string& suiteName);
90:     void AddSuite(TestSuiteInfo* testSuite);
91: };
92: 
93: /// <summary>
94: /// Test registrar
95: ///
96: /// This is a utility class to register a test to the registry, as part of a test declaration
97: /// </summary>
98: class TestRegistrar
99: {
100: public:
101:     TestRegistrar(TestRegistry& registry, Test* testInstance, const TestDetails& details);
102: };
103: 
104: /// <summary>
105: /// Run tests selected by the predicate
106: /// </summary>
107: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
108: /// <param name="predicate">Test selection predicate</param>
109: /// <param name="testResults">Test results to be returned</param>
110: template <typename Predicate> void TestRegistry::RunIf(const Predicate& predicate, TestResults& testResults)
111: {
112:     TestSuiteInfo* testSuite = Head();
113: 
114:     while (testSuite != nullptr)
115:     {
116:         if (predicate(testSuite))
117:             testSuite->RunIf(predicate, testResults);
118:         testSuite = testSuite->m_next;
119:     }
120: }
121: 
122: /// <summary>
123: /// Count the number of test suites selected by the predicate
124: /// </summary>
125: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
126: /// <param name="predicate">Test selection predicate</param>
127: /// <returns>Number of test suites selected by the predicate</returns>
128: template <typename Predicate> int TestRegistry::CountSuitesIf(Predicate predicate)
129: {
130:     int numberOfTestSuites = 0;
131:     TestSuiteInfo* testSuite = Head();
132:     while (testSuite != nullptr)
133:     {
134:         if (predicate(testSuite))
135:             ++numberOfTestSuites;
136:         testSuite = testSuite->m_next;
137:     }
138:     return numberOfTestSuites;
139: }
140: 
141: /// <summary>
142: /// Count the number of tests fixtures selected by the predicate
143: /// </summary>
144: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
145: /// <param name="predicate">Test selection predicate</param>
146: /// <returns>Number of test fixtures selected by the predicate</returns>
147: template <typename Predicate> int TestRegistry::CountFixturesIf(Predicate predicate)
148: {
149:     int numberOfTestFixtures = 0;
150:     TestSuiteInfo* testSuite = Head();
151:     while (testSuite != nullptr)
152:     {
153:         if (predicate(testSuite))
154:             numberOfTestFixtures += testSuite->CountFixturesIf(predicate);
155:         testSuite = testSuite->m_next;
156:     }
157:     return numberOfTestFixtures;
158: }
159: 
160: /// <summary>
161: /// Count the number of tests selected by the predicate
162: /// </summary>
163: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
164: /// <param name="predicate">Test selection predicate</param>
165: /// <returns>Number of tests selected by the predicate</returns>
166: template <typename Predicate> int TestRegistry::CountTestsIf(Predicate predicate)
167: {
168:     int numberOfTests = 0;
169:     TestSuiteInfo* testSuite = Head();
170:     while (testSuite != nullptr)
171:     {
172:         if (predicate(testSuite))
173:             numberOfTests += testSuite->CountTestsIf(predicate);
174:         testSuite = testSuite->m_next;
175:     }
176:     return numberOfTests;
177: }
178: 
179: } // namespace unittest
```

- Line 81: We replace the `Run()` method with a template version `RunIf()`, which takes a predicate, as well as a `TestResults` reference
- Line 82: We replace then `CountSuites()` method with a template version `CountSuitesIf()`
- Line 83: We replace then `CountFixtures()` method with a template version `CountFixturesIf()`
- Line 84: We replace then `CountTests()` method with a template version `CountTestsIf()`
- Line 110-120: We implement the `RunIf()` template method.
Note that we use the `RunIf()` method on the test suite. We'll need to implement this
- Line 128-139: We implement the `CountSuitesIf()` template method
- Line 147-158: We implement the `CountFixturesIf()` template method.
Note that we use the `CountFixturesIf()` method on the test suite. We'll need to implement this
- Line 166-177: We implement the `CountTestsIf()` template method.
Note that we use the `CountTestsIf()` method on the test suite. We'll need to implement this

### TestRegistry.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTREGISTRYCPP}

We'll also need to update the `TestRegistry` implementation.

Update the file `code/libraries/unittest/src/TestRegistry.cpp`

```cpp
File: code/libraries/unittest/src/TestRegistry.cpp
...
40: #include <unittest/TestRegistry.h>
41: 
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44: #include <unittest/TestInfo.h>
45: 
46: /// @file
47: /// Test registry implementation
48: 
49: using namespace baremetal;
50: 
51: /// @brief Define log name
52: LOG_MODULE("TestRegistry");
53: 
54: namespace unittest {
55: 
56: /// <summary>
57: /// Returns the test registry (singleton)
58: /// </summary>
59: /// <returns>Test registry reference</returns>
60: TestRegistry& TestRegistry::GetTestRegistry()
61: {
62:     static TestRegistry s_registry;
63:     return s_registry;
64: }
65: 
66: /// <summary>
67: /// Constructor
68: /// </summary>
69: TestRegistry::TestRegistry()
70:     : m_head{}
71:     , m_tail{}
72: {
73: }
74: 
75: /// <summary>
76: /// Destructor
77: ///
78: /// Cleans up all registered test suites, test fixtures and tests
79: /// </summary>
80: TestRegistry::~TestRegistry()
81: {
82:     TestSuiteInfo *testSuite = m_head;
83:     while (testSuite != nullptr)
84:     {
85:         TestSuiteInfo *currentSuite = testSuite;
86:         testSuite = testSuite->m_next;
87:         delete currentSuite;
88:     }
89: }
90: 
91: /// <summary>
92: /// Find a test suite with specified name, register a new one if not found
93: /// </summary>
94: /// <param name="suiteName">Test suite name to search for</param>
95: /// <returns>Found or created test suite</returns>
96: TestSuiteInfo *TestRegistry::GetTestSuite(const string &suiteName)
97: {
98:     TestSuiteInfo *testSuite = m_head;
99:     while ((testSuite != nullptr) && (testSuite->Name() != suiteName))
100:         testSuite = testSuite->m_next;
101:     if (testSuite == nullptr)
102:     {
103: #if DEBUG_REGISTRY
104:         LOG_DEBUG("Find suite %s ... not found, creating new object", suiteName.c_str());
105: #endif
106:         testSuite = new TestSuiteInfo(suiteName);
107:         AddSuite(testSuite);
108:     }
109:     else
110:     {
111: #if DEBUG_REGISTRY
112:         LOG_DEBUG("Find suite %s ... found", suiteName.c_str());
113: #endif
114:     }
115:     return testSuite;
116: }
117: 
118: /// <summary>
119: /// Add a test suite
120: /// </summary>
121: /// <param name="testSuite">Test suite to add</param>
122: void TestRegistry::AddSuite(TestSuiteInfo *testSuite)
123: {
124:     if (m_tail == nullptr)
125:     {
126:         assert(m_head == nullptr);
127:         m_head = testSuite;
128:         m_tail = testSuite;
129:     }
130:     else
131:     {
132:         m_tail->m_next = testSuite;
133:         m_tail = testSuite;
134:     }
135: }
136: 
137: /// <summary>
138: /// Constructor
139: ///
140: /// Finds or registers the test suite specified in the test details of the test.
141: /// Finds or registers the test fixture specified in the test details of the test.
142: /// Adds the test to the fixture found or created.
143: /// </summary>
144: /// <param name="registry">Test registry</param>
145: /// <param name="testInstance">Test instance to link to</param>
146: /// <param name="details">Test details</param>
147: TestRegistrar::TestRegistrar(TestRegistry &registry, Test *testInstance, const TestDetails& details)
148: {
149: #if DEBUG_REGISTRY
150:     LOG_DEBUG("Register test %s in fixture %s in suite %s",
151:         details.TestName().c_str(),
152:         (details.FixtureName().c_str()),
153:         (details.SuiteName().c_str()));
154: #endif
155:     TestSuiteInfo   *testSuite   = registry.GetTestSuite(details.SuiteName());
156:     TestFixtureInfo *testFixture = testSuite->GetTestFixture(details.FixtureName());
157:     TestInfo        *test = new TestInfo(testInstance, details);
158:     testFixture->AddTest(test);
159: }
160: 
161: } // namespace unittest
```

- Line 91: We remove the implementation for the methods `Run()`, `CountSuites()`, `CountFixtures()` and `CountTests()`.

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTSUITEINFOH}

We also need to update the `TestSuiteInfo` class.

Update the file `code/libraries/unittest/include/unittest/TestSuiteInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuiteInfo.h
...
58: class TestSuiteInfo
59: {
60: private:
61:     friend class TestRegistry;
62:     friend class TestRegistrar;
63:     /// @brief Pointer to first test fixture in the list
64:     TestFixtureInfo* m_head;
65:     /// @brief Pointer to last test fixture in the list
66:     TestFixtureInfo* m_tail;
67:     /// @brief Pointer to next test suite info in the list
68:     TestSuiteInfo* m_next;
69:     /// @brief Test suite name
70:     baremetal::string m_suiteName;
71: 
72: public:
73:     TestSuiteInfo() = delete;
74:     TestSuiteInfo(const TestSuiteInfo&) = delete;
75:     TestSuiteInfo(TestSuiteInfo&&) = delete;
76:     explicit TestSuiteInfo(const baremetal::string& suiteName);
77:     virtual ~TestSuiteInfo();
78: 
79:     TestSuiteInfo& operator = (const TestSuiteInfo&) = delete;
80:     TestSuiteInfo& operator = (TestSuiteInfo&&) = delete;
81: 
82:     /// <summary>
83:     /// Returns the pointer to the first test fixture in the list for this test suite
84:     /// </summary>
85:     /// <returns>Pointer to the first test fixture in the list for this test suite</returns>
86:     TestFixtureInfo* Head() const { return m_head; }
87: 
88:     /// <summary>
89:     /// Returns the test suite name
90:     /// </summary>
91:     /// <returns>Test suite name</returns>
92:     const baremetal::string& Name() const { return m_suiteName; }
93: 
94:     template <class Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
95: 
96:     int CountFixtures();
97:     int CountTests();
98:     template <typename Predicate> int CountFixturesIf(Predicate predicate);
99:     template <typename Predicate> int CountTestsIf(Predicate predicate);
100: 
101: private:
102:     TestFixtureInfo* GetTestFixture(const baremetal::string& fixtureName);
103:     void AddFixture(TestFixtureInfo* testFixture);
104: };
105: 
106: /// <summary>
107: /// Run tests in test suite using the selection predicate, updating the test results
108: /// </summary>
109: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
110: /// <param name="predicate">Test selection predicate</param>
111: /// <param name="testResults">Test results to use and update</param>
112: template <class Predicate> void TestSuiteInfo::RunIf(const Predicate& predicate, TestResults& testResults)
113: {
114:     testResults.OnTestSuiteStart(this);
115: 
116:     TestFixtureInfo* testFixture = Head();
117:     while (testFixture != nullptr)
118:     {
119:         if (predicate(testFixture))
120:             testFixture->RunIf(predicate, testResults);
121:         testFixture = testFixture->m_next;
122:     }
123: 
124:     testResults.OnTestSuiteFinish(this);
125: }
126: 
127: /// <summary>
128: /// Count the number of tests fixtures in the test suite selected by the predicate
129: /// </summary>
130: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
131: /// <param name="predicate">Test selection predicate</param>
132: /// <returns>Number of test fixtures in the test suite selected by the predicate</returns>
133: template <typename Predicate> int TestSuiteInfo::CountFixturesIf(Predicate predicate)
134: {
135:     int numberOfTestFixtures = 0;
136:     TestFixtureInfo* testFixture = Head();
137:     while (testFixture != nullptr)
138:     {
139:         if (predicate(testFixture))
140:             numberOfTestFixtures++;
141:         testFixture = testFixture->m_next;
142:     }
143:     return numberOfTestFixtures;
144: }
145: 
146: /// <summary>
147: /// Count the number of tests in the test suite selected by the predicate
148: /// </summary>
149: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
150: /// <param name="predicate">Test selection predicate</param>
151: /// <returns>Number of tests in the test suite selected by the predicate</returns>
152: template <typename Predicate> int TestSuiteInfo::CountTestsIf(Predicate predicate)
153: {
154:     int numberOfTests = 0;
155:     TestFixtureInfo* testFixture = Head();
156:     while (testFixture != nullptr)
157:     {
158:         if (predicate(testFixture))
159:             numberOfTests += testFixture->CountTestsIf(predicate);
160:         testFixture = testFixture->m_next;
161:     }
162:     return numberOfTests;
163: }
164: 
165: } // namespace unittest
```

- Line 62: We make `TestRegistrar` a friend class
- Line 94: We replace the `Run()` method with a template `RunIf()` method, taking both a predicate and the `TestResults` to fill
- Line 98: We add a template method `CountFixturesIf()`
- Line 99: We add a template method `CountTestsIf()`
- Line 102: We make the method `GetTestFixture` private, this is why `TestRegistrar` has to become a friend class
- Line 103: We also make the method `AddFixture` private
- Line 112-125: We implement the `RunIf()` template method.
Note that we use the method `RunIf()` in the `TestFixtureInfo` class. We'll need to implement it
- Line 133-144: We implement the `CountFixturesIf()` template method
- Line 152-163: We implement the `CountTestsIf()` template method.
Note that we use the method `CountTestsIf()` in the `TestFixtureInfo` class. We'll need to implement it

### TestSuiteInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTSUITEINFOCPP}

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
46: /// @file
47: /// Test suite administration implementation
48: 
49: using namespace baremetal;
50: 
51: namespace unittest {
52: 
53: /// @brief Define log name
54: LOG_MODULE("TestSuiteInfo");
55: 
56: /// <summary>
57: /// Constructor
58: /// </summary>
59: /// <param name="suiteName">Test suite name</param>
60: TestSuiteInfo::TestSuiteInfo(const string &suiteName)
61:     : m_head{}
62:     , m_tail{}
63:     , m_next{}
64:     , m_suiteName{suiteName}
65: {
66: }
67: 
68: /// <summary>
69: /// Destructor
70: ///
71: /// Cleans up all registered tests and test fixtures for this test suite
72: /// </summary>
73: TestSuiteInfo::~TestSuiteInfo()
74: {
75:     TestFixtureInfo *testFixture = m_head;
76:     while (testFixture != nullptr)
77:     {
78:         TestFixtureInfo *currentFixture = testFixture;
79:         testFixture                     = testFixture->m_next;
80:         delete currentFixture;
81:     }
82: }
83: 
84: /// <summary>
85: /// Find a test fixture with specified name, register a new one if not found
86: /// </summary>
87: /// <param name="fixtureName">Test fixture name to search for</param>
88: /// <returns>Found or created test fixture</returns>
89: TestFixtureInfo *TestSuiteInfo::GetTestFixture(const string &fixtureName)
90: {
91:     TestFixtureInfo *testFixture = m_head;
92:     while ((testFixture != nullptr) && (testFixture->Name() != fixtureName))
93:         testFixture = testFixture->m_next;
94:     if (testFixture == nullptr)
95:     {
96: #if DEBUG_REGISTRY
97:         LOG_DEBUG("Fixture %s not found, creating new object", fixtureName.c_str());
98: #endif
99:         testFixture = new TestFixtureInfo(fixtureName);
100:         AddFixture(testFixture);
101:     }
102:     else
103:     {
104: #if DEBUG_REGISTRY
105:         LOG_DEBUG("Fixture %s found", fixtureName.c_str());
106: #endif
107:     }
108:     return testFixture;
109: }
110: 
111: /// <summary>
112: /// Add a test fixture
113: /// </summary>
114: /// <param name="testFixture">Test fixture to add</param>
115: void TestSuiteInfo::AddFixture(TestFixtureInfo *testFixture)
116: {
117:     if (m_tail == nullptr)
118:     {
119:         assert(m_head == nullptr);
120:         m_head = testFixture;
121:         m_tail = testFixture;
122:     }
123:     else
124:     {
125:         m_tail->m_next = testFixture;
126:         m_tail         = testFixture;
127:     }
128: }
129: 
130: /// <summary>
131: /// Count the number of test fixtures in the test suite
132: /// </summary>
133: /// <returns>Number of test fixtures in the test suite</returns>
134: int TestSuiteInfo::CountFixtures()
135: {
136:     int numberOfTestFixtures = 0;
137:     TestFixtureInfo *testFixture = Head();
138:     while (testFixture != nullptr)
139:     {
140:         ++numberOfTestFixtures;
141:         testFixture = testFixture->m_next;
142:     }
143:     return numberOfTestFixtures;
144: }
145: 
146: /// <summary>
147: /// Count the number of tests in the test suite
148: /// </summary>
149: /// <returns>Number of tests in the test suite</returns>
150: int TestSuiteInfo::CountTests()
151: {
152:     int numberOfTests = 0;
153:     TestFixtureInfo *testFixture = Head();
154:     while (testFixture != nullptr)
155:     {
156:         numberOfTests += testFixture->CountTests();
157:         testFixture = testFixture->m_next;
158:     }
159:     return numberOfTests;
160: }
161: 
162: } // namespace unittest
```

- Line 130: We remove the implementation for the method `Run()`

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTFIXTUREINFOH}

We also need to update the `TestFixtureInfo` class.

Update the file `code/libraries/unittest/include/unittest/TestFixtureInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixtureInfo.h
...
58: class TestFixtureInfo
59: {
60: private:
61:     friend class TestSuiteInfo;
62:     friend class TestRegistrar;
63:     /// @brief Pointer to first test in the list
64:     TestInfo* m_head;
65:     /// @brief Pointer to last test in the list
66:     TestInfo* m_tail;
67:     /// @brief Pointer to next test fixture info in the list
68:     TestFixtureInfo* m_next;
69:     /// @brief Test fixture name
70:     baremetal::string m_fixtureName;
71: 
72: public:
73:     TestFixtureInfo() = delete;
74:     TestFixtureInfo(const TestFixtureInfo&) = delete;
75:     TestFixtureInfo(TestFixtureInfo&&) = delete;
76:     explicit TestFixtureInfo(const baremetal::string& fixtureName);
77:     virtual ~TestFixtureInfo();
78: 
79:     TestFixtureInfo & operator = (const TestFixtureInfo &) = delete;
80:     TestFixtureInfo& operator = (TestFixtureInfo&&) = delete;
81: 
82:     /// <summary>
83:     /// Returns the pointer to the first test in the list for this test fixture
84:     /// </summary>
85:     /// <returns>Pointer to the first test in the list for this test fixture</returns>
86:     TestInfo* Head() const {  return m_head; }
87: 
88:     /// <summary>
89:     /// Returns the test fixture name
90:     /// </summary>
91:     /// <returns>Test fixture name</returns>
92:     const baremetal::string& Name() const { return m_fixtureName; }
93: 
94:     template <class Predicate> void RunIf(const Predicate& predicate, TestResults& testResults);
95: 
96:     int CountTests();
97:     template <typename Predicate> int CountTestsIf(Predicate predicate);
98: 
99: private:
100:     void AddTest(TestInfo* test);
101: };
102: 
103: /// <summary>
104: /// Run tests in test fixture using the selection predicate, updating the test results
105: /// </summary>
106: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
107: /// <param name="predicate">Test selection predicate</param>
108: /// <param name="testResults">Test results to use and update</param>
109: template <class Predicate> void TestFixtureInfo::RunIf(const Predicate& predicate, TestResults& testResults)
110: {
111:     testResults.OnTestFixtureStart(this);
112: 
113:     TestInfo* test = this->Head();
114:     while (test != nullptr)
115:     {
116:         if (predicate(test))
117:             test->Run(testResults);
118:         test = test->m_next;
119:     }
120: 
121:     testResults.OnTestFixtureFinish(this);
122: }
123: 
124: /// <summary>
125: /// Count the number of tests in the test fixture selected by the predicate
126: /// </summary>
127: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
128: /// <param name="predicate">Test selection predicate</param>
129: /// <returns>Number of tests in the test fixture selected by the predicate</returns>
130: template <typename Predicate> int TestFixtureInfo::CountTestsIf(Predicate predicate)
131: {
132:     int numberOfTests = 0;
133:     TestInfo* test = this->Head();
134:     while (test != nullptr)
135:     {
136:         if (predicate(test))
137:             numberOfTests++;
138:         test = test->m_next;
139:     }
140:     return numberOfTests;
141: }
142: 
143: } // namespace unittest
```

- Line 62: We make `TestRegistrar` a friend class
- Line 94: We replace the `Run()` method with a template `RunIf()` method, taking both a predicate and the `TestResults` to fill
- Line 97: We add a template method `CountTestsIf()`
- Line 100: We make the method `AddTest()` private. This is the reason `TestRegistrar` needs to become a friend class
- Line 109-122: We implement the `RunIf()` template method
- Line 130-141: We implement the `CountTestsIf()` template method

### TestFixtureInfo.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_TESTFIXTUREINFOCPP}

We'll also need to update the `TestFixtureInfo` implementation.

Update the file `code/libraries/unittest/src/TestFixtureInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestFixtureInfo.cpp
...
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
100: /// Count the number of tests in the test fixture
101: /// </summary>
102: /// <returns>Number of tests in the test fixture</returns>
103: int TestFixtureInfo::CountTests()
104: {
105:     int numberOfTests = 0;
106:     TestInfo* test = Head();
107:     while (test != nullptr)
108:     {
109:         ++numberOfTests;
110:         test = test->m_next;
111:     }
112:     return numberOfTests;
113: }
114: 
115: } // namespace unittest
```

- Line 99: We remove the implementation for the method `Run()`

### ConsoleTestReporter.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_CONSOLETESTREPORTERH}

We still need to implement the abstract interface `ITestReporter` to actually report information.
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
44: /// @file
45: /// Console test reporter
46: ///
47: /// Prints test information to the console, using color.
48: /// Uses DeferredTestReporter to save failures so they can be printed after running the test, as an overview
49: 
50: namespace unittest
51: {
52: 
53: /// <summary>
54: /// Console test reporter
55: /// </summary>
56: class ConsoleTestReporter
57:     : public ITestReporter
58: {
59: public:
60:     /// @brief Separator for complete test run
61:     static const baremetal::string TestRunSeparator;
62:     /// @brief Separator for test fixture
63:     static const baremetal::string TestFixtureSeparator;
64:     /// @brief Separator for test suite
65:     static const baremetal::string TestSuiteSeparator;
66:     /// @brief Indicator for successful test
67:     static const baremetal::string TestSuccessSeparator;
68:     /// @brief Indicator for failed test
69:     static const baremetal::string TestFailSeparator;
70: 
71:     ConsoleTestReporter();
72: 
73: private:
74:     void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
75:     void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
76:     void ReportTestRunSummary(const TestResults& results) override;
77:     void ReportTestRunOverview(const TestResults& results) override;
78:     void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTestFixtures) override;
79:     void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTestFixtures) override;
80:     void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) override;
81:     void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests) override;
82:     void ReportTestStart(const TestDetails& details) override;
83:     void ReportTestFinish(const TestDetails& details, bool success) override;
84:     void ReportTestFailure(const TestDetails& details, const baremetal::string& failure) override;
85: 
86:     baremetal::string TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests);
87:     baremetal::string TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests);
88:     baremetal::string TestSuiteStartMessage(const baremetal::string& suiteName, int numberOfTestFixtures);
89:     baremetal::string TestSuiteFinishMessage(const baremetal::string& suiteName, int numberOfTestFixtures);
90:     baremetal::string TestFixtureStartMessage(const baremetal::string& fixtureName, int numberOfTests);
91:     baremetal::string TestFixtureFinishMessage(const baremetal::string& fixtureName, int numberOfTests);
92:     baremetal::string TestFailureMessage(const TestDetails& details, const baremetal::string& failure);
93:     baremetal::string TestFinishMessage(const TestDetails& test, bool success);
94:     baremetal::string TestRunSummaryMessage(const TestResults& results);
95:     baremetal::string TestRunOverviewMessage(const TestResults& results);
96: };
97: 
98: } // namespace unittest
```

- Line 61-69: We declare some static constant strings to use as separators in our output
- Line 71: We declare a default constructor
- Line 74-84: We declare the overrides for the `ITestReporter` abstract interface
- Line 86-95: We declare some utility functions to build strings for output

### ConsoleTestReporter.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_CONSOLETESTREPORTERCPP}

We'll implement the class `ConsoleTestReporter`.

Create the file `code/libraries/unittest/src/ConsoleTestReporter.cpp`

```cpp
File: code/libraries/unittest/src/ConsoleTestReporter.cpp
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
49: /// @file
50: /// Console test reporter implementation
51: 
52: using namespace baremetal;
53: 
54: namespace unittest
55: {
56: 
57: const string ConsoleTestReporter::TestRunSeparator = baremetal::string("[===========]");
58: const string ConsoleTestReporter::TestSuiteSeparator = baremetal::string("[   SUITE   ]");
59: const string ConsoleTestReporter::TestFixtureSeparator = baremetal::string("[  FIXTURE  ]");
60: const string ConsoleTestReporter::TestSuccessSeparator = baremetal::string("[ SUCCEEDED ]");
61: const string ConsoleTestReporter::TestFailSeparator = baremetal::string("[  FAILED   ]");
62: 
63: /// <summary>
64: /// Constructor
65: /// </summary>
66: ConsoleTestReporter::ConsoleTestReporter()
67: {
68: }
69: 
70: /// <summary>
71: /// Start of test run callback
72: /// </summary>
73: /// <param name="numberOfTestSuites">Number of test suites to be run</param>
74: /// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
75: /// <param name="numberOfTests">Number of tests to be run</param>
76: void ConsoleTestReporter::ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
77: {
78:     GetConsole().SetTerminalColor(ConsoleColor::Green);
79:     GetConsole().Write(TestRunSeparator);
80:     GetConsole().ResetTerminalColor();
81: 
82:     GetConsole().Write(Format(" %s\n", TestRunStartMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
83: }
84: 
85: /// <summary>
86: /// Finish of test run callback
87: /// </summary>
88: /// <param name="numberOfTestSuites">Number of test suites run</param>
89: /// <param name="numberOfTestFixtures">Number of test fixtures run</param>
90: /// <param name="numberOfTests">Number of tests run</param>
91: void ConsoleTestReporter::ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
92: {
93:     GetConsole().SetTerminalColor(ConsoleColor::Green);
94:     GetConsole().Write(TestRunSeparator);
95:     GetConsole().ResetTerminalColor();
96: 
97:     GetConsole().Write(Format(" %s\n", TestRunFinishMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
98: }
99: 
100: /// <summary>
101: /// Test summary callback
102: /// </summary>
103: /// <param name="results">Test run results</param>
104: void ConsoleTestReporter::ReportTestRunSummary(const TestResults& results)
105: {
106:     if (results.GetFailureCount() > 0)
107:         GetConsole().SetTerminalColor(ConsoleColor::Red);
108:     else
109:         GetConsole().SetTerminalColor(ConsoleColor::Green);
110:     GetConsole().Write(Format("%s\n", TestRunSummaryMessage(results).c_str()));
111:     GetConsole().ResetTerminalColor();
112: }
113: 
114: /// <summary>
115: /// Test run overview callback
116: /// </summary>
117: /// <param name="results">Test run results</param>
118: void ConsoleTestReporter::ReportTestRunOverview(const TestResults& results)
119: {
120:     GetConsole().Write(Format("%s\n", TestRunOverviewMessage(results).c_str()));
121: }
122: 
123: /// <summary>
124: /// Test suite start callback
125: /// </summary>
126: /// <param name="suiteName">Test suite name</param>
127: /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
128: void ConsoleTestReporter::ReportTestSuiteStart(const string& suiteName, int numberOfTestFixtures)
129: {
130:     GetConsole().SetTerminalColor(ConsoleColor::Cyan);
131:     GetConsole().Write(TestSuiteSeparator);
132:     GetConsole().ResetTerminalColor();
133: 
134:     GetConsole().Write(Format(" %s\n", TestSuiteStartMessage(suiteName, numberOfTestFixtures).c_str()));
135: }
136: 
137: /// <summary>
138: /// Test suite finish callback
139: /// </summary>
140: /// <param name="suiteName">Test suite name</param>
141: /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
142: void ConsoleTestReporter::ReportTestSuiteFinish(const string& suiteName, int numberOfTestFixtures)
143: {
144:     GetConsole().SetTerminalColor(ConsoleColor::Cyan);
145:     GetConsole().Write(TestSuiteSeparator);
146:     GetConsole().ResetTerminalColor();
147: 
148:     GetConsole().Write(Format(" %s\n", TestSuiteFinishMessage(suiteName, numberOfTestFixtures).c_str()));
149: }
150: 
151: /// <summary>
152: /// Test fixture start callback
153: /// </summary>
154: /// <param name="fixtureName">Test fixture name</param>
155: /// <param name="numberOfTests">Number of tests within test fixture</param>
156: void ConsoleTestReporter::ReportTestFixtureStart(const string& fixtureName, int numberOfTests)
157: {
158:     GetConsole().SetTerminalColor(ConsoleColor::Yellow);
159:     GetConsole().Write(TestFixtureSeparator);
160:     GetConsole().ResetTerminalColor();
161: 
162:     GetConsole().Write(Format(" %s\n", TestFixtureStartMessage(fixtureName, numberOfTests).c_str()));
163: }
164: 
165: /// <summary>
166: /// Test fixture finish callback
167: /// </summary>
168: /// <param name="fixtureName">Test fixture name</param>
169: /// <param name="numberOfTests">Number of tests within test fixture</param>
170: void ConsoleTestReporter::ReportTestFixtureFinish(const string& fixtureName, int numberOfTests)
171: {
172:     GetConsole().SetTerminalColor(ConsoleColor::Yellow);
173:     GetConsole().Write(TestFixtureSeparator);
174:     GetConsole().ResetTerminalColor();
175: 
176:     GetConsole().Write(Format(" %s\n", TestFixtureFinishMessage(fixtureName, numberOfTests).c_str()));
177: }
178: 
179: /// <summary>
180: /// Test start callback
181: /// </summary>
182: /// <param name="details">Test details</param>
183: void ConsoleTestReporter::ReportTestStart(const TestDetails& /*details*/)
184: {
185: }
186: 
187: /// <summary>
188: /// Test finish callback
189: /// </summary>
190: /// <param name="details">Test details</param>
191: /// <param name="success">Test result, true is successful, false is failed</param>
192: void ConsoleTestReporter::ReportTestFinish(const TestDetails& details, bool success)
193: {
194:     GetConsole().SetTerminalColor(success ? ConsoleColor::Green : ConsoleColor::Red);
195:     if (success)
196:         GetConsole().Write(TestSuccessSeparator);
197:     else
198:         GetConsole().Write(TestFailSeparator);
199:     GetConsole().ResetTerminalColor();
200: 
201:     GetConsole().Write(Format(" %s\n", TestFinishMessage(details, success).c_str()));
202: }
203: 
204: /// <summary>
205: /// Test failure callback
206: /// </summary>
207: /// <param name="details">Test details</param>
208: /// <param name="failure">Test failure message</param>
209: void ConsoleTestReporter::ReportTestFailure(const TestDetails& details, const string& failure)
210: {
211:     GetConsole().SetTerminalColor(ConsoleColor::Red);
212:     GetConsole().Write(Format("%s\n", TestFailureMessage(details, failure).c_str()));
213:     GetConsole().ResetTerminalColor();
214: }
215: 
216: /// <summary>
217: /// Return the correct singular or plural form for "test"
218: /// </summary>
219: /// <param name="numberOfTests">Number of tests</param>
220: /// <returns></returns>
221: static string TestLiteral(int numberOfTests)
222: {
223:     return baremetal::string((numberOfTests == 1) ? "test" : "tests");
224: }
225: 
226: /// <summary>
227: /// Return the correct singular or plural form for "failure"
228: /// </summary>
229: /// <param name="numberOfTestFailures">Number of test failures</param>
230: /// <returns></returns>
231: static string TestFailureLiteral(int numberOfTestFailures)
232: {
233:     return baremetal::string((numberOfTestFailures == 1) ? "failure" : "failures");
234: }
235: 
236: /// <summary>
237: /// Return the correct singular or plural form for "fixture"
238: /// </summary>
239: /// <param name="numberOfTestFixtures">Number of test fixtures</param>
240: /// <returns></returns>
241: static string TestFixtureLiteral(int numberOfTestFixtures)
242: {
243:     return baremetal::string((numberOfTestFixtures == 1) ? "fixture" : "fixtures");
244: }
245: 
246: /// <summary>
247: /// Return the correct singular or plural form for "suite"
248: /// </summary>
249: /// <param name="numberOfTestSuites">Number of test suites</param>
250: /// <returns></returns>
251: static string TestSuiteLiteral(int numberOfTestSuites)
252: {
253:     return baremetal::string((numberOfTestSuites == 1) ? "suite" : "suites");
254: }
255: 
256: /// <summary>
257: /// Create a message for test run start
258: /// </summary>
259: /// <param name="numberOfTestSuites">Number of test suites to be run</param>
260: /// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
261: /// <param name="numberOfTests">Number of tests to be run</param>
262: /// <returns>Resulting message</returns>
263: string ConsoleTestReporter::TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
264: {
265:     return Format("Running %s %s from %s %s in %s %s.",
266:         Serialize(numberOfTests).c_str(),
267:         TestLiteral(numberOfTests).c_str(),
268:         Serialize(numberOfTestFixtures).c_str(),
269:         TestFixtureLiteral(numberOfTestFixtures).c_str(),
270:         Serialize(numberOfTestSuites).c_str(),
271:         TestSuiteLiteral(numberOfTestSuites).c_str());
272: }
273: 
274: /// <summary>
275: /// Create a message for test run finish
276: /// </summary>
277: /// <param name="numberOfTestSuites">Number of test suites run</param>
278: /// <param name="numberOfTestFixtures">Number of test fixtures run</param>
279: /// <param name="numberOfTests">Number of tests run</param>
280: /// <returns>Resulting message</returns>
281: string ConsoleTestReporter::TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
282: {
283:     return Format("%s %s from %s %s in %s %s ran.",
284:         Serialize(numberOfTests).c_str(),
285:         TestLiteral(numberOfTests).c_str(),
286:         Serialize(numberOfTestFixtures).c_str(),
287:         TestFixtureLiteral(numberOfTestFixtures).c_str(),
288:         Serialize(numberOfTestSuites).c_str(),
289:         TestSuiteLiteral(numberOfTestSuites).c_str());
290: }
291: 
292: /// <summary>
293: /// Create a message for test run summary
294: /// </summary>
295: /// <param name="results">Test run results</param>
296: /// <returns>Resulting message</returns>
297: string ConsoleTestReporter::TestRunSummaryMessage(const TestResults& results)
298: {
299:     if (results.GetFailureCount() > 0)
300:     {
301:         return Format("FAILURE: %s out of %s %s failed (%s %s).\n",
302:             Serialize(results.GetFailedTestCount()).c_str(),
303:             Serialize(results.GetTotalTestCount()).c_str(),
304:             TestLiteral(results.GetTotalTestCount()).c_str(),
305:             Serialize(results.GetFailureCount()).c_str(),
306:             TestFailureLiteral(results.GetFailureCount()).c_str());
307:     }
308:     return Format("Success: %s %s passed.\n",
309:             Serialize(results.GetTotalTestCount()).c_str(),
310:             TestLiteral(results.GetTotalTestCount()).c_str());
311: }
312: 
313: /// <summary>
314: /// Create a message for test run overview
315: /// </summary>
316: /// <param name="results">Test run results</param>
317: /// <returns>Resulting message</returns>
318: string ConsoleTestReporter::TestRunOverviewMessage(const TestResults& results)
319: {
320:     if (results.GetFailureCount() > 0)
321:     {
322:         return Format("Failures: %d", results.GetFailureCount());
323:     }
324:     return "No failures";
325: }
326: 
327: /// <summary>
328: /// Create a message for test failure
329: /// </summary>
330: /// <param name="result">Test run results</param>
331: /// <param name="failure">Failure that occurred</param>
332: /// <returns>Resulting message</returns>
333: string ConsoleTestReporter::TestFailureMessage(const TestDetails& details, const string& failure)
334: {
335:     return Format("%s failure %s", details.QualifiedTestName().c_str(), failure.c_str());
336: }
337: 
338: /// <summary>
339: /// Create a message for test suite start
340: /// </summary>
341: /// <param name="suiteName">Name of test suite</param>
342: /// <param name="numberOfTestFixtures">Number of test fixtures within test suite</param>
343: /// <returns>Resulting message</returns>
344: string ConsoleTestReporter::TestSuiteStartMessage(const string& suiteName, int numberOfTestFixtures)
345: {
346:     return Format("%s (%s %s)",
347:         suiteName.c_str(),
348:         Serialize(numberOfTestFixtures).c_str(),
349:         TestFixtureLiteral(numberOfTestFixtures).c_str());
350: }
351: 
352: /// <summary>
353: /// Create a message for test suite finish
354: /// </summary>
355: /// <param name="suiteName">Name of test suite</param>
356: /// <param name="numberOfTestFixtures">Number of test fixtures within test suite</param>
357: /// <returns>Resulting message</returns>
358: string ConsoleTestReporter::TestSuiteFinishMessage(const string& suiteName, int numberOfTestFixtures)
359: {
360:     return Format("%s %s from %s",
361:         Serialize(numberOfTestFixtures).c_str(),
362:         TestFixtureLiteral(numberOfTestFixtures).c_str(),
363:         suiteName.c_str());
364: }
365: 
366: /// <summary>
367: /// Create a message for test fixture start
368: /// </summary>
369: /// <param name="fixtureName">Test fixture name</param>
370: /// <param name="numberOfTests">Number of tests within test fixture</param>
371: /// <returns>Resulting message</returns>
372: string ConsoleTestReporter::TestFixtureStartMessage(const string& fixtureName, int numberOfTests)
373: {
374:     return Format("%s (%s %s)",
375:         fixtureName.c_str(),
376:         Serialize(numberOfTests).c_str(),
377:         TestLiteral(numberOfTests).c_str());
378: }
379: 
380: /// <summary>
381: /// Create a message for test fixture finish
382: /// </summary>
383: /// <param name="fixtureName">Test fixture name</param>
384: /// <param name="numberOfTests">Number of tests within test fixture</param>
385: /// <returns>Resulting message</returns>
386: string ConsoleTestReporter::TestFixtureFinishMessage(const string& fixtureName, int numberOfTests)
387: {
388:     return Format("%s %s from %s",
389:         Serialize(numberOfTests).c_str(),
390:         TestLiteral(numberOfTests).c_str(),
391:         fixtureName.c_str());
392: }
393: 
394: /// <summary>
395: /// Create a message for test finish
396: /// </summary>
397: /// <param name="details">Test details</param>
398: /// <param name="success">Test result, true is successful, false is failed</param>
399: /// <returns>Resulting message</returns>
400: string ConsoleTestReporter::TestFinishMessage(const TestDetails& details, bool /*success*/)
401: {
402:     return details.QualifiedTestName();
403: }
404: 
405: } // namespace unittest
```

- Line 57-61: We initialize the static member variables
- Line 66-68: We implement the constructor
- Line 76-83: We implement the method `ReportTestRunStart()`.
This prints the run separator in green, and uses the method `TestRunStartMessage()` to format the string to be printed
- Line 91-98: We implement the method `ReportTestRunFinish()`.
This prints the run separator in green, and uses the method `TestRunFinishMessage()` to format the string to be printed
- Line 104-112: We implement the method `ReportTestRunSummary()`.
This uses the method `TestRunSummaryMessage()` to format the string to be printed, in green if successful, in red if failures occured
- Line 118-121: We implement the method `ReportTestRunOverview()`.
This uses the method `TestRunOverviewMessage()` to format the string to be printed
- Line 128-135: We implement the method `ReportTestSuiteStart()`.
This prints the test suite separator in cyan, and uses the method `TestSuiteStartMessage()` to format the string to be printed
- Line 142-149: We implement the method `ReportTestSuiteFinish()`.
This prints the test suite separator in cyan, and uses the method `TestSuiteFinishMessage()` to format the string to be printed
- Line 156-163: We implement the method `ReportTestFixtureStart()`.
This prints the test fixture separator in yellow, and uses the method `TestFixtureStartMessage()` to format the string to be printed
- Line 170-177: We implement the method `ReportTestFixtureFinish()`.
This prints the test fixture separator in yellow, and uses the method `TestFixtureFinishMessage()` to format the string to be printed
- Line 183-185: We implement the method `ReportTestStart()`.
This prints nothing
- Line 192-202: We implement the method `ReportTestFinish()`.
This prints the success separator in green if successful, or the failure separator in red if failures occurred, and uses the method `TestFinishMessage()` to format the string to be printed
- Line 209-214: We implement the method `ReportTestFailure()`.
This uses the method `TestFailureMessage()` to format the string to be printed in red
- Line 221-224: We implement a static function `TestLiteral()`, which returns the string "test" if the number of tests equals 1, and "tests" otherwise
- Line 231-234: We implement a static function `TestFailureLiteral()`, which returns the string "failure" if the number of failures equals 1, and "failures" otherwise
- Line 241-244: We implement a static function `TestFixtureLiteral()`, which returns the string "fixture" if the number of test fixtures equals 1, and "fixtures" otherwise
- Line 251-254: We implement a static function `TestSuiteLiteral()`, which returns the string "suite" if the number of test suites equals 1, and "suites" otherwise
- Line 263-272: We implement a static function `TestRunStartMessage()`, which returns a string in the form "Running 1 test from 2 fixtures in 3 suites"
- Line 281-290: We implement a static function `TestRunFinishMessage()`, which returns a string in the form "1 test from 2 fixtures in 3 suites ran."
- Line 297-311: We implement a static function `TestRunSummaryMessage()`, which returns a string in the form "FAILURE: 1 out of 2 tests failed (3 failures).\n" in case of failures,
and "Success: 2 tests passed.\n" otherwise
- Line 318-325: We implement a static function `TestRunOverviewMessage()`, which returns a string in the form "Failures: 2" in case of failures, and "No failures" otherwise
- Line 333-336: We implement a static function `TestFailureMessage()`, which returns a string in the form "Suite1\:\:Fixture2\:\:Test3 failure failureText"
- Line 344-350: We implement a static function `TestSuiteStartMessage()`, which returns a string in the form "2 fixtures from Suite1"
- Line 358-364: We implement a static function `TestSuiteFinishMessage()`, which returns a string in the form "Suite (2 fixtures)"
- Line 372-378: We implement a static function `TestFixtureStartMessage()`, which returns a string in the form "2 test from Fixture1"
- Line 386-392: We implement a static function `TestFixtureFinishMessage()`, which returns a string in the form "Fixture1 (2 tests)"
- Line 400-403: We implement a static function `TestFinishMessage()`, which returns a string containing the fully qualified test name

### unittest.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_UNITTESTH}

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
42: /// @file
43: /// Unit test generic header
44: 
45: #include <unittest/TestFixture.h>
46: #include <unittest/TestSuite.h>
47: 
48: #include <unittest/ITestReporter.h>
49: #include <unittest/ConsoleTestReporter.h>
50: #include <unittest/CurrentTest.h>
51: #include <unittest/Test.h>
52: #include <unittest/TestInfo.h>
53: #include <unittest/TestDetails.h>
54: #include <unittest/TestFixtureInfo.h>
55: #include <unittest/TestRegistry.h>
56: #include <unittest/TestResults.h>
57: #include <unittest/TestRunner.h>
58: #include <unittest/TestSuiteInfo.h>
```

### Update project configuration {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleTestReporter.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Test.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestInfo.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRunner.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
41:     )
42: 
43: set(PROJECT_INCLUDES_PUBLIC
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ConsoleTestReporter.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Test.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestInfo.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRunner.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
56:     )
57: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_APPLICATION_CODE}

Now that we have added a test runner and a test report, we can use these to make running the tests even simpler.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include <baremetal/Assert.h>
2: #include <baremetal/Console.h>
3: #include <baremetal/Logger.h>
4: #include <baremetal/System.h>
5: #include <baremetal/Timer.h>
6: 
7: #include <unittest/unittest.h>
8: 
9: LOG_MODULE("main");
10: 
11: using namespace baremetal;
12: using namespace unittest;
13: 
14: namespace Suite1 {
15: 
16: inline char const* GetSuiteName()
17: {
18:     return baremetal::string("Suite1");
19: }
20: 
21: class FixtureMyTest1
22:     : public TestFixture
23: {
24: public:
25:     void SetUp() override
26:     {
27:         LOG_DEBUG("MyTest SetUp");
28:     }
29:     void TearDown() override
30:     {
31:         LOG_DEBUG("MyTest TearDown");
32:     }
33: };
34: 
35: class FixtureMyTest1Helper
36:     : public FixtureMyTest1
37: {
38: public:
39:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
40:     explicit FixtureMyTest1Helper(const TestDetails& details)
41:         : m_details{ details }
42:     {
43:         SetUp();
44:     }
45:     virtual ~FixtureMyTest1Helper()
46:     {
47:         TearDown();
48:     }
49:     void RunImpl() const;
50:     const TestDetails& m_details;
51: };
52: void FixtureMyTest1Helper::RunImpl() const
53: {
54:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 1");
55: }
56: 
57: class MyTest1
58:     : public Test
59: {
60:     void RunImpl() const override;
61: } myTest1;
62: 
63: TestRegistrar registrarFixtureMyTest1(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__));
64: 
65: void MyTest1::RunImpl() const
66: {
67:     LOG_DEBUG("Test 1");
68:     FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
69:     fixtureHelper.RunImpl();
70: }
71: 
72: } // namespace Suite1
73: 
74: namespace Suite2 {
75: 
76: inline char const* GetSuiteName()
77: {
78:     return baremetal::string("Suite2");
79: }
80: 
81: class FixtureMyTest2
82:     : public TestFixture
83: {
84: public:
85:     void SetUp() override
86:     {
87:         LOG_DEBUG("FixtureMyTest2 SetUp");
88:     }
89:     void TearDown() override
90:     {
91:         LOG_DEBUG("FixtureMyTest2 TearDown");
92:     }
93: };
94: 
95: class FixtureMyTest2Helper
96:     : public FixtureMyTest2
97: {
98: public:
99:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
100:     explicit FixtureMyTest2Helper(const TestDetails& details)
101:         : m_details{ details }
102:     {
103:         SetUp();
104:     }
105:     virtual ~FixtureMyTest2Helper()
106:     {
107:         TearDown();
108:     }
109:     void RunImpl() const;
110:     const TestDetails& m_details;
111: };
112: void FixtureMyTest2Helper::RunImpl() const
113: {
114:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 2");
115: }
116: 
117: class MyTest2
118:     : public Test
119: {
120:     void RunImpl() const override;
121: } myTest1;
122: 
123: TestRegistrar registrarFixtureMyTest2(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__));
124: 
125: void MyTest2::RunImpl() const
126: {
127:     LOG_DEBUG("Test 2");
128:     FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
129:     fixtureHelper.RunImpl();
130: }
131: 
132: } // namespace Suite2
133: 
134: class FixtureMyTest3
135:     : public TestFixture
136: {
137: public:
138:     void SetUp() override
139:     {
140:         LOG_DEBUG("FixtureMyTest3 SetUp");
141:     }
142:     void TearDown() override
143:     {
144:         LOG_DEBUG("FixtureMyTest3 TearDown");
145:     }
146: };
147: 
148: class FixtureMyTest3Helper
149:     : public FixtureMyTest3
150: {
151: public:
152:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
153:     explicit FixtureMyTest3Helper(const TestDetails& details)
154:         : m_details{ details }
155:     {
156:         SetUp();
157:     }
158:     virtual ~FixtureMyTest3Helper()
159:     {
160:         TearDown();
161:     }
162:     void RunImpl() const;
163:     const TestDetails& m_details;
164: };
165: void FixtureMyTest3Helper::RunImpl() const
166: {
167:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 3");
168: }
169: 
170: class MyTest3
171:     : public Test
172: {
173:     void RunImpl() const override;
174: } myTest3;
175: 
176: TestRegistrar registrarFixtureMyTest3(TestRegistry::GetTestRegistry(), &myTest3, TestDetails("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__));
177: 
178: void MyTest3::RunImpl() const
179: {
180:     LOG_DEBUG("Test 3");
181:     FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
182:     fixtureHelper.RunImpl();
183: }
184: 
185: class MyTest
186:     : public Test
187: {
188: public:
189:     void RunImpl() const override;
190: } myTest;
191: 
192: TestRegistrar registrarMyTest(TestRegistry::GetTestRegistry(), &myTest, TestDetails("MyTest", "", "", __FILE__, __LINE__));
193: 
194: void MyTest::RunImpl() const
195: {
196:     LOG_DEBUG("Running test");
197: }
198: 
199: int main()
200: {
201:     auto& console = GetConsole();
202: 
203:     ConsoleTestReporter reporter;
204:     RunAllTests(&reporter);
205: 
206:     LOG_INFO("Wait 5 seconds");
207:     Timer::WaitMilliSeconds(5000);
208: 
209:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
210:     char ch{};
211:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
212:     {
213:         ch = console.ReadChar();
214:         console.WriteChar(ch);
215:     }
216:     if (ch == 'p')
217:         assert(false);
218: 
219:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
220: }
```

- Line 7: We replace all unit test includes by a single one
- Line 197: We inject a failure to get a failing result
- Line 204: We instantiate a test reporter throug the `ConsoleTestReporter` class
- Line 205: We run all tests using the test reporter using the `RunAllTests()` function

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR__STEP_7_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

You will see all the output being generared by the console test reported using colors.

<img src="images/demo-output-unit-test.png" alt="Tree view" width="800"/>

## Collecting test information - Step 8 {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8}

So far, we can run tests, but the output is written immediately as we run. A better way would be to collect information and print it after the test run has completed.
That way the test run output itself will be more clean, and we have a simple summary of all failures.
For this, we'll introduce the `DeferredTestReporter` class, and make `ConsoleTestReporter` inherit from this.

### TestResult.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_TESTRESULTH}

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
45: /// @file
46: /// Test result
47: /// 
48: /// Result of a single test
49: 
50: namespace unittest
51: {
52: 
53: /// <summary>
54: /// Failure
55: /// 
56: /// Holds information on an occurrred test failure
57: /// </summary>
58: class Failure
59: {
60: private:
61:     /// @brief Line number on which failure occurred
62:     int m_lineNumber;
63:     /// @brief Failure message
64:     baremetal::string m_text;
65: 
66: public:
67:     /// <summary>
68:     /// Constructor
69:     /// </summary>
70:     /// <param name="lineNumber">Line number on which failure occurred</param>
71:     /// <param name="text">Failure message, can be empty</param>
72:     Failure(int lineNumber, const baremetal::string& text);
73:     /// <summary>
74:     /// Return line number on which failure occurred
75:     /// </summary>
76:     /// <returns>Line number on which failure occurred</returns>
77:     int SourceLineNumber() const { return m_lineNumber; }
78:     /// <summary>
79:     /// Returns failure message
80:     /// </summary>
81:     /// <returns>Failure message</returns>
82:     const baremetal::string& Text() const { return m_text; }
83: };
84: 
85: /// <summary>
86: /// Container for failure
87: /// </summary>
88: class FailureEntry
89: {
90: private:
91:     friend class FailureList;
92:     /// @brief Failure information
93:     Failure m_failure;
94:     /// @brief Pointer to next failure entry in the list
95:     FailureEntry* m_next;
96: 
97: public:
98:     /// <summary>
99:     /// Constructor
100:     /// </summary>
101:     /// <param name="failure">Failure information to set</param>
102:     explicit FailureEntry(const Failure& failure);
103:     /// <summary>
104:     /// Returns failure information
105:     /// </summary>
106:     /// <returns>Failure information</returns>
107:     const Failure& GetFailure() const { return m_failure; }
108:     /// <summary>
109:     /// Return pointer to next failure entry
110:     /// </summary>
111:     /// <returns>Pointer to next failure entry</returns>
112:     const FailureEntry* GetNext() const { return m_next; }
113: };
114: 
115: /// <summary>
116: /// List of failures
117: /// </summary>
118: class FailureList
119: {
120: private:
121:     /// @brief Pointer to first failure entry in the list
122:     FailureEntry* m_head;
123:     /// @brief Pointer to last failure entry in the list
124:     FailureEntry* m_tail;
125: 
126: public:
127:     /// <summary>
128:     /// Constructor
129:     /// </summary>
130:     FailureList();
131:     /// <summary>
132:     /// Destructor
133:     /// </summary>
134:     ~FailureList();
135: 
136:     /// <summary>
137:     /// Returns pointer to first failure in the list
138:     /// </summary>
139:     /// <returns>Pointer to first failure in the list</returns>
140:     const FailureEntry* GetHead() const { return m_head; }
141:     /// <summary>
142:     /// Add a failure to the list
143:     /// </summary>
144:     /// <param name="failure">Failure information to add</param>
145:     void Add(const Failure& failure);
146: };
147: 
148: /// <summary>
149: /// Results for a single test
150: /// </summary>
151: class TestResult
152: {
153: private:
154:     /// @brief Details of the test
155:     TestDetails m_details;
156:     /// @brief List of failure for the test
157:     FailureList m_failures;
158:     /// @brief Failure flag, true if at least one failure occurred
159:     bool m_failed;
160: 
161: public:
162:     TestResult() = delete;
163:     /// <summary>
164:     /// Constructor
165:     /// </summary>
166:     /// <param name="details">Test details</param>
167:     explicit TestResult(const TestDetails & details);
168: 
169:     /// <summary>
170:     /// Add a failure to the list for this test
171:     /// </summary>
172:     /// <param name="failure">Failure information</param>
173:     void AddFailure(const Failure& failure);
174:     /// <summary>
175:     /// Return the list of failures for this test
176:     /// </summary>
177:     /// <returns>List of failures for this test</returns>
178:     const FailureList & Failures() const { return m_failures; }
179:     /// <summary>
180:     /// Returns failure flag
181:     /// </summary>
182:     /// <returns>Failure flag, true if at least one failure occurred</returns>
183:     bool Failed() const { return m_failed; }
184:     /// <summary>
185:     /// Returns the test details
186:     /// </summary>
187:     /// <returns>Test details</returns>
188:     const TestDetails& Details() const { return m_details; }
189: };
190: 
191: } // namespace unittest
```

- Line 58-83: We declare a class `Failure` to hold a single failure
  - Line 62: The member variable `m_lineNumber` holds the line number in the source file where the failure occurred
  - Line 64: The member variable `m_text` holds the failure message
  - Line 72: We declare the constructor
  - Line 77: We declare and define the method `SourceLineNumber()` which returns the line number
  - Line 82: We declare and define the method `Text()` which returns the failure message
- Line 88-113: We declare a class `FailureEntry` which holds a failure, and a pointer to the next failure
  - Line 93: The member variable `m_failure` holds the failure
  - Line 95: The member variable `m_next` holds a pointer to the next `FailureEntry` in the list
  - Line 102: We declare the constructor
  - Line 107: We declare and define the method `GetFailure()` which returns a const reference to the failure
  - Line 112: We declare and define the method `GetNext()` which returns a const pointer to the next `FailureEntry` in the list
- Line 118-146: We declare a class `FailureList` which holds a pointer to the beginning and the end of a `FailureEntry` list
  - Line 122-124: The member variables `m_head` and `m_tail` hold a pointer to the beginning and the end of the list, respectively
  - Line 130: We declare the constructor
  - Line 134: We declare the destructor, which will clean up the list of `FailureEntry` instances
  - Line 140: We declare and define the method `GetHead()` which returns a const pointer to the first `FailureEntry` in the list
  - Line 145: We declare and define the method `Add()` which adds a failure to the list (embedded in a `FailureEntry` instance)
- Line 151-189: We declare the class `TestResult`
  - Line 155: The member variable `m_details` holds the test details
  - Line 157: The member variable `m_failures` holds the failure list
  - Line 159: The member variable `m_failed` holds true if at least one failure occurred for this test
  - Line 162: We remove the default constructor
  - Line 167: We declare an explicit constructor
  - Line 173: We declare the method `AddFailure()` which adds a failure to the list
  - Line 178: We declare and define the method `Failures()` which returns a const reference to the failure list
  - Line 183: We declare and define the method `Failed()` which returns true if a failure occurred
  - Line 188: We declare and define the method `Details()` which returns the test details

### TestResult.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_TESTRESULTCPP}

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
44: /// @file
45: /// Test result implementation
46: /// 
47: /// Result of a single test
48: 
49: using namespace baremetal;
50: 
51: namespace unittest {
52: 
53: Failure::Failure(int lineNumber, const baremetal::string& text)
54:     : m_lineNumber{lineNumber}
55:     , m_text{text}
56: {
57: }
58: 
59: FailureEntry::FailureEntry(const Failure& failure)
60:     : m_failure{ failure }
61:     , m_next{}
62: {
63: }
64: 
65: FailureList::FailureList()
66:     : m_head{}
67:     , m_tail{}
68: {
69: }
70: 
71: FailureList::~FailureList()
72: {
73:     auto current = m_head;
74:     while (current != nullptr)
75:     {
76:         auto next = current->m_next;
77:         delete current;
78:         current = next;
79:     }
80: }
81: 
82: void FailureList::Add(const Failure& failure)
83: {
84:     auto entry = new FailureEntry(failure);
85:     if (m_head == nullptr)
86:     {
87:         m_head = entry;
88:     }
89:     else
90:     {
91:         auto current = m_head;
92:         while (current->m_next != nullptr)
93:             current = current->m_next;
94:         current->m_next = entry;
95:     }
96:     m_tail = entry;
97: }
98: 
99: TestResult::TestResult(const TestDetails& details)
100:     : m_details{ details }
101:     , m_failures{}
102:     , m_failed{}
103: {
104: }
105: 
106: void TestResult::AddFailure(const Failure& failure)
107: {
108:     m_failures.Add(failure);
109:     m_failed = true;
110: }
111: 
112: } // namespace unittest
```

- Line 53-57: We implement the `Failure` constructor
- Line 59-63: We implement the `FailureEntry` constructor
- Line 65-69: We implement the `FailureList` constructor
- Line 71-80: We implement the `FailureList` destructor. This will delete all `FailureEntry` instances in the list
- Line 82-97: We implement the method `Add()` for `FailureList`. This will create a new `FailureEntry` with the `Failure` in it, and insert at the end of the list
- Line 99-104: We implement the `TestResult` constructor
- Line 106-110: We implement the method `AddFailure()` for `TestResult`. This will add the failure to the list, and set the failed flag to true

### DeferredTestReporter.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_DEFERREDTESTREPORTERH}

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
40: #pragma once
41: 
42: #include <unittest/ITestReporter.h>
43: #include <unittest/TestResult.h>
44: 
45: /// @file
46: /// Deferred test reporter
47: ///
48: /// Saves failures during the test run, so they can be sown in the overview after the complete test run
49: 
50: namespace unittest
51: {
52: 
53: /// <summary>
54: /// Test result entry
55: /// </summary>
56: class ResultEntry
57: {
58: private:
59:     friend class ResultList;
60:     /// @brief Test result
61:     TestResult m_result;
62:     /// @brief Pointer to next entry in list
63:     ResultEntry* m_next;
64: 
65: public:
66:     explicit ResultEntry(const TestResult& result);
67:     /// <summary>
68:     /// Return test result
69:     /// </summary>
70:     /// <returns>Test result</returns>
71:     TestResult& GetResult() { return m_result; }
72:     /// <summary>
73:     /// Return next entry pointer
74:     /// </summary>
75:     /// <returns>Next entry pointer</returns>
76:     ResultEntry* GetNext() { return m_next; }
77: };
78: 
79: /// <summary>
80: /// Test result entry list
81: /// </summary>
82: class ResultList
83: {
84: private:
85:     /// @brief Start of list
86:     ResultEntry* m_head;
87:     /// @brief End of list
88:     ResultEntry* m_tail;
89: 
90: public:
91:     ResultList();
92:     ~ResultList();
93: 
94:     void Add(const TestResult& result);
95:     /// <summary>
96:     /// Return start of list pointer
97:     /// </summary>
98:     /// <returns>Start of list pointer</returns>
99:     ResultEntry* GetHead() const { return m_head; }
100:     /// <summary>
101:     /// Return end of list pointer
102:     /// </summary>
103:     /// <returns>End of list pointer</returns>
104:     ResultEntry* GetTail() const { return m_tail; }
105: };
106: 
107: /// <summary>
108: /// Deferred test reporter
109: ///
110: /// Implements abstract ITestReporter interface
111: /// </summary>
112: class DeferredTestReporter : public ITestReporter
113: {
114: private:
115:     /// @brief Test result list for current test run
116:     ResultList m_results;
117: 
118: public:
119:     void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
120:     void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
121:     void ReportTestRunSummary(const TestResults& results) override;
122:     void ReportTestRunOverview(const TestResults& results) override;
123:     void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTestFixtures) override;
124:     void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTestFixtures) override;
125:     void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) override;
126:     void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests) override;
127:     void ReportTestStart(const TestDetails& details) override;
128:     void ReportTestFinish(const TestDetails& details, bool success) override;
129:     void ReportTestFailure(const TestDetails& details, const baremetal::string& failure) override;
130: 
131:     ResultList& Results();
132: };
133: 
134: } // namespace unittest
```

- Line 56-77: We declare the struct `ResultEntry`, which holds a `TestResult`, which we'll declare later, and a pointer to the next `ResultEntry`. The results entries form a linked list, and are used to gather results for each test
  - Line 61: The member variable `m_result` holds the test result
  - Line 63: The member variable `m_next` holds the pointer to the next `ResultEntry` in the list
  - Line 66: We declare the constructor
  - Line 71: We declare and define the method `GetResult()` which returns the test result
  - Line 76: We declare and define the method `GetNext()` which returns the pointer to the next `ResultEntry` in the list
- Line 79-105: We declare the class `ResultList` which holds a pointer to the first and last `ResultEntry`
  - Line 86-88: The member variables `m_head` and `m_tail` hold a pointer to the beginning and the end of the list, respectively
  - Line 91: We declare the constructor
  - Line 92: We declare the destructor, which will clean up the list of `ResultEntry` instances
  - Line 94: We declare and define the method `Add()` which adds a test result to the list (embedded in a `ResultEntry` instance)
  - Line 99: We declare and define the method `GetHead()` which returns a const pointer to the first `ResultEntry` in the list
  - Line 104: We declare and define the method `GetTail()` which returns a const pointer to the last `ResultEntry` in the list
- Line 112-132: We declare the class `DeferredTestReporter`, which implements the abstract interface `ITestReporter`
  - Line 116: The class variable `m_results` holds the list of test results saved during the test run
  - Line 119-129: We implement the `ITestReporter` interface
  - Line 131: We declare the method `Results()` which returns the `ResultList`

### DeferredTestReporter.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_DEFERREDTESTREPORTERCPP}

Let's implement the `DeferredTestReporter` class.

Create the file `code/libraries/unittest/src/DeferredTestReporter.cpp`

```cpp
File: code/libraries/unittest/src/DeferredTestReporter.cpp
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
44: /// @file
45: /// Deferred test reporter implementation
46: 
47: using namespace baremetal;
48: 
49: namespace unittest
50: {
51: 
52: /// <summary>
53: /// Constructor
54: /// </summary>
55: /// <param name="result">Test result to be stored</param>
56: ResultEntry::ResultEntry(const TestResult& result)
57:     : m_result{ result }
58:     , m_next{}
59: {
60: }
61: 
62: /// <summary>
63: /// Constructor
64: /// </summary>
65: ResultList::ResultList()
66:     : m_head{}
67:     , m_tail{}
68: {
69: }
70: 
71: /// <summary>
72: /// Destructor
73: /// </summary>
74: ResultList::~ResultList()
75: {
76:     auto current = m_head;
77:     while (current != nullptr)
78:     {
79:         auto next = current->m_next;
80:         delete current;
81:         current = next;
82:     }
83: }
84: 
85: /// <summary>
86: /// Add a test result to the list
87: /// </summary>
88: /// <param name="result">Test result to add</param>
89: void ResultList::Add(const TestResult& result)
90: {
91:     auto entry = new ResultEntry(result);
92:     if (m_head == nullptr)
93:     {
94:         m_head = entry;
95:     }
96:     else
97:     {
98:         auto current = m_head;
99:         while (current->m_next != nullptr)
100:             current = current->m_next;
101:         current->m_next = entry;
102:     }
103:     m_tail = entry;
104: }
105: 
106: /// <summary>
107: /// Start of test run callback (empty)
108: /// </summary>
109: /// <param name="numberOfTestSuites">Number of test suites to be run</param>
110: /// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
111: /// <param name="numberOfTests">Number of tests to be run</param>
112: void DeferredTestReporter::ReportTestRunStart(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
113: {
114: }
115: 
116: /// <summary>
117: /// Finish of test run callback (empty)
118: /// </summary>
119: /// <param name="numberOfTestSuites">Number of test suites run</param>
120: /// <param name="numberOfTestFixtures">Number of test fixtures run</param>
121: /// <param name="numberOfTests">Number of tests run</param>
122: void DeferredTestReporter::ReportTestRunFinish(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
123: {
124: }
125: 
126: /// <summary>
127: /// Test summary callback (empty)
128: /// </summary>
129: /// <param name="results">Test run results</param>
130: void DeferredTestReporter::ReportTestRunSummary(const TestResults& /*results*/)
131: {
132: }
133: 
134: /// <summary>
135: /// Test run overview callback (empty)
136: /// </summary>
137: /// <param name="results">Test run results</param>
138: void DeferredTestReporter::ReportTestRunOverview(const TestResults& /*results*/)
139: {
140: }
141: 
142: /// <summary>
143: /// Test suite start callback (empty)
144: /// </summary>
145: /// <param name="suiteName">Test suite name</param>
146: /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
147: void DeferredTestReporter::ReportTestSuiteStart(const string& /*suiteName*/, int /*numberOfTestFixtures*/)
148: {
149: }
150: 
151: /// <summary>
152: /// Test suite finish callback (empty)
153: /// </summary>
154: /// <param name="suiteName">Test suite name</param>
155: /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
156: void DeferredTestReporter::ReportTestSuiteFinish(const string& /*suiteName*/, int /*numberOfTestFixtures*/)
157: {
158: }
159: 
160: /// <summary>
161: /// Test fixture start callback (empty)
162: /// </summary>
163: /// <param name="fixtureName">Test fixture name</param>
164: /// <param name="numberOfTests">Number of tests within test fixture</param>
165: void DeferredTestReporter::ReportTestFixtureStart(const string& /*fixtureName*/, int /*numberOfTests*/)
166: {
167: }
168: 
169: /// <summary>
170: /// Test fixture finish callback (empty)
171: /// </summary>
172: /// <param name="fixtureName">Test fixture name</param>
173: /// <param name="numberOfTests">Number of tests within test fixture</param>
174: void DeferredTestReporter::ReportTestFixtureFinish(const string& /*fixtureName*/, int /*numberOfTests*/)
175: {
176: }
177: 
178: /// <summary>
179: /// Test start callback
180: /// </summary>
181: /// <param name="details">Test details</param>
182: void DeferredTestReporter::ReportTestStart(const TestDetails& details)
183: {
184:     m_results.Add(TestResult(details));
185: }
186: 
187: /// <summary>
188: /// Test finish callback (empty)
189: /// </summary>
190: /// <param name="details">Test details</param>
191: /// <param name="success">Test result, true is successful, false is failed</param>
192: void DeferredTestReporter::ReportTestFinish(const TestDetails& /*details*/, bool /*success*/)
193: {
194: }
195: 
196: /// <summary>
197: /// Test failure callback
198: /// </summary>
199: /// <param name="details">Test details</param>
200: /// <param name="failure">Test failure message</param>
201: void DeferredTestReporter::ReportTestFailure(const TestDetails& details, const string& failure)
202: {
203:     TestResult& result = m_results.GetTail()->GetResult();
204:     result.AddFailure(Failure(details.SourceFileLineNumber(), failure));
205: }
206: 
207: /// <summary>
208: /// Return test result list
209: /// </summary>
210: /// <returns>Test result list</returns>
211: ResultList& DeferredTestReporter::Results()
212: {
213:     return m_results;
214: }
215: 
216: } // namespace unittest
```

- Line 56-60: We implement the `ResultEntry` constructor
- Line 65-69: We implement the `ResultList` constructor
- Line 74-83: We implement the `ResultList` denstructor
- Line 89-104: We implement the method `Add` for `ResultList`. This will create a new `ResultEntry` and insert it at the end of the list
- Line 112-114: We implement the method `ReportTestRunStart` for `DeferredTestReporter`.
This does nothing, as `DeferredTestReporter` does not report anything in itself. It simply stores test results 
- Line 122-124: We implement the method `ReportTestRunFinish` for `DeferredTestReporter`. This again does nothing
- Line 130-132: We implement the method `ReportTestRunSummary` for `DeferredTestReporter`. This again does nothing
- Line 138-140: We implement the method `ReportTestRunOverview` for `DeferredTestReporter`. This again does nothing
- Line 147-149: We implement the method `ReportTestSuiteStart` for `DeferredTestReporter`. This again does nothing
- Line 156-158: We implement the method `ReportTestSuiteFinish` for `DeferredTestReporter`. This again does nothing
- Line 165-167: We implement the method `ReportTestFixtureStart` for `DeferredTestReporter`. This again does nothing
- Line 174-176: We implement the method `ReportTestFixtureFinish` for `DeferredTestReporter`. This again does nothing
- Line 182-184: We implement the method `ReportTestStart` for `DeferredTestReporter`. This adds a new result to the list
- Line 192-194: We implement the method `ReportTestFinish` for `DeferredTestReporter`. This again does nothing
- Line 201-205: We implement the method `ReportTestFailure` for `DeferredTestReporter`. This adds a failure to the list for the current result
- Line 211-214: We implement the method `Results` for `DeferredTestReporter`

### ConsoleTestReport.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_CONSOLETESTREPORTH}

We need to update the class `ConsoleTestReporter` to derive from `DeferredTestReporter`.

Update the file  `code/libraries/unittest/include/unittest/ConsoleTestReport.h`

```cpp
File: code/libraries/unittest/include/unittest/ConsoleTestReport.h
...
42: #include <unittest/DeferredTestReporter.h>
...
53: /// <summary>
54: /// Console test reporter
55: /// </summary>
56: class ConsoleTestReporter
57:     : public DeferredTestReporter
...
92:     baremetal::string TestFailureMessage(const TestResult& result, const Failure& failure);
...
```

- Line 42: We need to include the header for `DeferredTestReporter` instead of for the interface
- Line 56-57: We inherit from `DeferredTestReporter`
- Line 77: We replace the method `TestFailureMessage()` with a version taking a `TestResult` and a `Failure`

### ConsoleTestReport.cpp {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_CONSOLETESTREPORTCPP}

Let's update the implementation for `ConsoleTestReport`.

Update the file `code/libraries/unittest/src/ConsoleTestReport.cpp`

```cpp
File: code/libraries/unittest/src/ConsoleTestReport.cpp
...
179: /// <summary>
180: /// Test start callback
181: /// </summary>
182: /// <param name="details">Test details</param>
183: void ConsoleTestReporter::ReportTestStart(const TestDetails& details)
184: {
185:     DeferredTestReporter::ReportTestStart(details);
186: }
187: 
188: /// <summary>
189: /// Test finish callback
190: /// </summary>
191: /// <param name="details">Test details</param>
192: /// <param name="success">Test result, true is successful, false is failed</param>
193: void ConsoleTestReporter::ReportTestFinish(const TestDetails& details, bool success)
194: {
195:     DeferredTestReporter::ReportTestFinish(details, success);
196:     GetConsole().SetTerminalColor(success ? ConsoleColor::Green : ConsoleColor::Red);
197:     if (success)
198:         GetConsole().Write(TestSuccessSeparator);
199:     else
200:         GetConsole().Write(TestFailSeparator);
201:     GetConsole().ResetTerminalColor();
202: 
203:     GetConsole().Write(Format(" %s\n", TestFinishMessage(details, success).c_str()));
204: }
205: 
206: /// <summary>
207: /// Test failure callback
208: /// </summary>
209: /// <param name="details">Test details</param>
210: /// <param name="failure">Test failure message</param>
211: void ConsoleTestReporter::ReportTestFailure(const TestDetails& details, const string& failure)
212: {
213:     DeferredTestReporter::ReportTestFailure(details, failure);
214: }
...
313: /// <summary>
314: /// Create a message for test run overview
315: /// </summary>
316: /// <param name="results">Test run results</param>
317: /// <returns>Resulting message</returns>
318: string ConsoleTestReporter::TestRunOverviewMessage(const TestResults& results)
319: {
320: 
321:     if (results.GetFailureCount() > 0)
322:     {
323:         string result = "Failures:\n";
324:         auto testResultPtr = Results().GetHead();
325:         while (testResultPtr != nullptr)
326:         {
327:             auto const& testResult = testResultPtr->GetResult();
328:             if (testResult.Failed())
329:             {
330:                 auto failuresPtr = testResult.Failures().GetHead();
331:                 while (failuresPtr != nullptr)
332:                 {
333:                     result.append(TestFailureMessage(testResult, failuresPtr->GetFailure()));
334:                     failuresPtr = failuresPtr->GetNext();
335:                 }
336:             }
337:             testResultPtr = testResultPtr->GetNext();
338:         }
339:         return result;
340:     }
341:     return "No failures";
342: }
343: 
344: /// <summary>
345: /// Create a message for test failure
346: /// </summary>
347: /// <param name="result">Test run results</param>
348: /// <param name="failure">Failure that occurred</param>
349: /// <returns>Resulting message</returns>
350: string ConsoleTestReporter::TestFailureMessage(const TestResult& result, const Failure& failure)
351: {
352:     return Format("%s:%d : Failure in %s: %s\n",
353:         result.Details().SourceFileName().c_str(),
354:         failure.SourceLineNumber(),
355:         result.Details().QualifiedTestName().c_str(),
356:         failure.Text().c_str());
357: }
...
```

- Line 183-186: We reimplement the method `ReportTestStart` by calling the same method in `DeferredTestReporter`
- Line 193-204: We reimplement the method `ReportTestFinish` by first calling the same method in `DeferredTestReporter`
- Line 211-214: We reimplement the method `ReportTestFailure` by calling the same method in `DeferredTestReporter`
- Line 318-342: We reimplement the method `TestRunOverviewMessage` by going through the list of test results, and for any that have failed, going through the list of failures, and appending a test failure message
- Line 350-357: We reimplement the method `TestFailureMessage` to print a correct failure message

### unittest.h {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_UNITTESTH}

We've added a header, so let's include that in the common header as well.

Update the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
45: #include <unittest/TestFixture.h>
46: #include <unittest/TestSuite.h>
47: 
48: #include <unittest/ITestReporter.h>
49: #include <unittest/ConsoleTestReporter.h>
50: #include <unittest/CurrentTest.h>
51: #include <unittest/DeferredTestReporter.h>
52: #include <unittest/Test.h>
53: #include <unittest/TestDetails.h>
54: #include <unittest/TestFixtureInfo.h>
55: #include <unittest/TestInfo.h>
56: #include <unittest/TestRegistry.h>
57: #include <unittest/TestResults.h>
58: #include <unittest/TestRunner.h>
59: #include <unittest/TestSuiteInfo.h>
```

### Update project configuration {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleTestReporter.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/DeferredTestReporter.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Test.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestInfo.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResult.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRunner.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
43:     )
44: 
45: set(PROJECT_INCLUDES_PUBLIC
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ConsoleTestReporter.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/DeferredTestReporter.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Test.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestInfo.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResult.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRunner.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
60:     )
61: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_APPLICATION_CODE}

We will keep the application code unchanged for now.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_INFORMATION__STEP_8_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. The reporting will be slightly different, as we now will see a summary of all failures.
The debug logging will still show, but normally test cases do not log.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Debug  Register test MyTest1 in fixture FixtureMyTest1 in suite Suite1 (TestRegistry:150)
Debug  Find suite Suite1 ... not found, creating new object (TestRegistry:104)
Debug  Fixture FixtureMyTest1 not found, creating new object (TestSuiteInfo:97)
Debug  Register test MyTest2 in fixture FixtureMyTest2 in suite Suite2 (TestRegistry:150)
Debug  Find suite Suite2 ... not found, creating new object (TestRegistry:104)
Debug  Fixture FixtureMyTest2 not found, creating new object (TestSuiteInfo:97)
Debug  Register test MyTest3 in fixture FixtureMyTest3 in suite DefaultSuite (TestRegistry:150)
Debug  Find suite DefaultSuite ... not found, creating new object (TestRegistry:104)
Debug  Fixture FixtureMyTest3 not found, creating new object (TestSuiteInfo:97)
Debug  Register test MyTest in fixture DefaultFixture in suite DefaultSuite (TestRegistry:150)
Debug  Find suite DefaultSuite ... found (TestRegistry:112)
Debug  Fixture DefaultFixture not found, creating new object (TestSuiteInfo:97)
Info   Starting up (System:201)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
Debug  Test 1 (main:67)
Debug  MyTest SetUp (main:27)
Debug  Suite1::FixtureMyTest1::MyTest1MyTestHelper 1 (main:54)
Debug  MyTest TearDown (main:31)
[ SUCCEEDED ] Suite1::FixtureMyTest1::MyTest1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
Debug  Test 2 (main:127)
Debug  FixtureMyTest2 SetUp (main:87)
Debug  Suite2::FixtureMyTest2::MyTest2MyTestHelper 2 (main:114)
Debug  FixtureMyTest2 TearDown (main:91)
[ SUCCEEDED ] Suite2::FixtureMyTest2::MyTest2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
Debug  Test 3 (main:180)
Debug  FixtureMyTest3 SetUp (main:140)
Debug  DefaultSuite::FixtureMyTest3::MyTest3MyTestHelper 3 (main:167)
Debug  FixtureMyTest3 TearDown (main:144)
[ SUCCEEDED ] DefaultSuite::FixtureMyTest3::MyTest3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
Debug  Running test (main:196)
[  FAILED   ] DefaultSuite::DefaultFixture::MyTest
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 1 out of 4 tests failed (1 failure).

Failures:
../code/applications/demo/src/main.cpp:192 : Failure in DefaultSuite::DefaultFixture::MyTest: Failure

[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   Wait 5 seconds (main:207)
Press r to reboot, h to halt, p to fail assertion and panic
```

In the next tutorial we'll start replacing the very verbose code for setting up tests, test fixtures and test suites with macros.
We'll also introduce macros for the actual test cases, and we'll convert the test we made before for strings and serialization into proper class tests.

Next: [18-writing-unit-tests](18-writing-unit-tests.md)


