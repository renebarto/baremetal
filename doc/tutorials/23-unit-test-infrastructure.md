# Tutorial 23: Unit test infrastructure {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE}

@tableofcontents

## Tutorial setup {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/23-unit-test-infrastructure`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-23.a`
- a library `output/Debug/lib/device-23.a`
- a library `output/Debug/lib/stdlib-23.a`
- a library `output/Debug/lib/unittest-23.a`
- an application `output/Debug/bin/23-unit-test-infrastructure.elf`
- an image in `deploy/Debug/23-unit-test-infrastructure-image`

## Creating a framework for unit testing {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_CREATING_A_FRAMEWORK_FOR_UNIT_TESTING}

In the last tutorials we have been adding quite some code, and added assertion to verify correctness.
We also wrote stubs to verify correct handling for GPIO, I2C, and the MCP23017 device.

It would be nice if we could continuously verify correctness for our code, so we know if we accidentally break something.

It would be convenient to write unit tests, using a standard framework. GoogleTest / GoogleMock is such a framework, however this leans heavily on the C++ standard library, which we cannot use yet.
So we'll be creating our own simplified framework to execute unit tests, in which they are automatically registered, and organized in different groups.
We'll follow the following concept here:

- Tests are classes that execute a single test (with possibly multiple test cases), and are self-contained and isolated.
I.e. they do not need any context, and don't influence the context.
- Test fixtures are collections of tests that belong together, e.g. because they test a specific class or group of functionality.
A test fixture has a class, which can be used to hold common variables, and allows for a common test setup / teardown.
- Test suites are collections of test fixtures that belong together, e.g. because they test a complete library.

We'll use a result class to gather test results, which will then be collected in a results class.
A test reporter is a visitor class that is called to print the status of the test run and the results of every test.

This concept is similar although not exactly the same as the one used by Google Test.

For an overview, see the image below.

In the next chapter we will use the example of the MCP23017 device to test logic functionality, using something similar to the stubs.

<img src="images/unittest-class-structure.png" alt="Tree view" width="800"/>

Let's start off by implementing a simple test class, and using a macro to define the test.

## Creating the unittest library - Step 1 {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_CREATING_THE_UNITTEST_LIBRARY___STEP_1}

We'll place the unit testing functionality in a new library, named `unittest`.
The structure is similar to the other libraries we have introduced such as `baremetal` (see [Creating the baremetal library structure - Step 1](#TUTORIAL_06_FIRST_APPLICATION___USING_THE_CONSOLE___UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE___STEP_1))

<img src="images/treeview-unittest-library.png" alt="Tree view" width="300"/>

### CMake file for unittest {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_CREATING_THE_UNITTEST_LIBRARY___STEP_1_CMAKE_FILE_FOR_UNITTEST}

We need to create the unittest project CMake file

Create the file `code/libraries/unittest/CMakeLists.txt`.

```cmake
File: code/libraries/unittest/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3:
4: project(unittest
5:     DESCRIPTION "UNit test library"
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
22:     baremetal
23:     )
24:
25: set(PROJECT_LIBS
26:     ${LINKER_LIBRARIES}
27:     ${PROJECT_DEPENDENCIES}
28:     )
29:
30: file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp src/*.S)
31: set(GLOB_RECURSE PROJECT_INCLUDES_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/*.h)
32: set(PROJECT_INCLUDES_PRIVATE )
33:
34: set(PROJECT_INCLUDES_PRIVATE )
35:
36: if (CMAKE_VERBOSE_MAKEFILE)
37:     display_list("Package                           : " ${PROJECT_NAME} )
38:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
39:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
40:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
41:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
42:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
43:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
44:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
45:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
46:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
47:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
48:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
49:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
50:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
51:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
52:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
53:     display_list("Link libs                         : " ${PROJECT_LIBS} )
54:     display_list("Source files                      : " ${PROJECT_SOURCES} )
55:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
56:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
57: endif()
58:
59: add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
60: target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
61: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
62: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
63: target_compile_definitions(${PROJECT_NAME} PRIVATE
64:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
65:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
66:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
67:     )
68: target_compile_definitions(${PROJECT_NAME} PUBLIC
69:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
70:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
71:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
72:     )
73: target_compile_options(${PROJECT_NAME} PRIVATE
74:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
75:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
76:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
77:     )
78: target_compile_options(${PROJECT_NAME} PUBLIC
79:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
80:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
81:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
82:     )
83:
84: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
85:
86: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
87: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
88:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
89: endif()
90:
91: link_directories(${LINK_DIRECTORIES})
92: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
93: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
94:
95: show_target_properties(${PROJECT_NAME})
```

As no source files are added yet, configuring will fail, but we'll get to that in a while.

### Update libraries CMake file {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_CREATING_THE_UNITTEST_LIBRARY___STEP_1_UPDATE_LIBRARIES_CMAKE_FILE}

First we need to include the unittest project in the libraries CMake file.

Update the file `code/libraries/CMakeLists.txt`

```cmake
message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(baremetal)
add_subdirectory(device)
add_subdirectory(stdlib)
add_subdirectory(unittest)
```

## Adding a test - Step 2 {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST___STEP_2}

### Test.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST___STEP_2_TESTH}

We will start by adding a test class. All tests (and later test fixtures) will derive from this class, and implement its `RunImpl()` method to run the actual test.

Create the file `code/libraries/unittest/include/unittest/Test.h`

```cpp
File: code/libraries/unittest/include/unittest/Test.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: /// @file
43: /// Test
44:
45: namespace unittest {
46:
47: /// <summary>
48: /// Test class
49: /// </summary>
50: class Test
51: {
52: public:
53:     Test() = default;
54:     Test(const Test&) = delete;
55:     Test(Test&&) = delete;
56:     virtual ~Test() = default;
57:
58:     Test& operator=(const Test&) = delete;
59:     Test& operator=(Test&&) = delete;
60:
61:     virtual void RunImpl() const;
62: };
63:
64: } // namespace unittest
```

The `Test` class is added to the `unittest` namespace.

- Line 47-62: We declare the class `Test`
  - Line 53: We declare a default constructor with default implementation
  - Line 54-55: We remove the copy constructor and move constructor
  - Line 56: We declare the destructor with default implementation. This may be important as we will be inheriting from this class
  - Line 58-59: We remove the assignment operators
  - Line 61: We declare the overridable `RunImpl()` method

### Test.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST___STEP_2_TESTCPP}

We'll implement the `Test` class.

Create the file `code/libraries/unittest/src/Test.cpp`

```cpp
File: code/libraries/unittest/src/Test.cpp
File: d:\Projects\RaspberryPi\baremetal.github\code\libraries\unittest\src\Test.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/Test.h"
41:
42: /// @file
43: /// Test implementation
44:
45: namespace unittest {
46:
47: /// <summary>
48: /// Actual test implementation
49: /// </summary>
50: void Test::RunImpl() const
51: {
52: }
53:
54: } // namespace unittest
```

As you can see this is a very simple implementation, we simply implement the `RunImpl()` method with an default implementation.

### Update application CMake file {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST___STEP_2_UPDATE_APPLICATION_CMAKE_FILE}

As we now depend on the new `unittest` library, we need to add that dependency.

Update the file `code/applications/demo/CMakeLists.txt`

```
File: code/applications/demo/CMakeLists.txt
27: set(PROJECT_DEPENDENCIES
28:     unittest
29:     device
30:     baremetal
31:     )
```

### Update application code {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST___STEP_2_UPDATE_APPLICATION_CODE}

Let's start using the class we just created. We'll add a simple test case by declaring and implementing a class derived from `TestBase`.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Assert.h"
2: #include "baremetal/Console.h"
3: #include "baremetal/Logger.h"
4: #include "baremetal/System.h"
5: #include "baremetal/Timer.h"
6:
7: #include "unittest/Test.h"
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
35:     console.Write("Press r to reboot, h to halt\n");
36:     char ch{};
37:     while ((ch != 'r') && (ch != 'h'))
38:     {
39:         ch = console.ReadChar();
40:         console.WriteChar(ch);
41:     }
42:
43:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
44: }
```

- Line 7: We include the header for `Test`
- Line 13-18: We declare the class `MyTest` based on `Test`
  - Line 17: We declare an override for the `RunImpl()` method
- Line 20-23: We implement the `RunImpl()` method for `MyTest`. It simply logs a string
- Line 29-30: We define an instance of MyTest, and then run the test.

### Configuring, building and debugging {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_A_TEST___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging. We'll first switch off the memory debug output to get a cleaner console.

The application will run the test, and therefore show the log output.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
Info   0.00:00:00.010 Running test (main:22)
Info   0.00:00:00.020 Wait 5 seconds (main:32)
Press r to reboot, h to halt
hInfo   0.00:00:12.500 Halt (System:121)
```

## Test administration - Step 3 {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3}

In order to register tests, we need to add an extra class that holds information on a test.
For this we introduce the `TestInfo` class.
This class will depend on another information class, `TestDetails` which holds the name of the test, the source file information, etc.
We'll also keep track of the current test and its results, captured in the class `TestResults`.

### TestDetails.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3_TESTDETAILSH}

First we'll add the `TestDetails` class to describe a test.
This will hold its test suite name, test fixture name, test name, source file, and line number.

Create the file `code/libraries/unittest/include/unittest/TestDetails.h`

```cpp
File: code/libraries/unittest/include/unittest/TestDetails.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "baremetal/String.h"
43:
44: /// @file
45: /// Test details
46:
47: namespace unittest {
48:
49: /// <summary>
50: /// Details of a test
51: /// </summary>
52: class TestDetails
53: {
54: private:
55:     /// @brief Name of test suite test is part of
56:     const baremetal::string m_suiteName;
57:     /// @brief Name of test fixture test is part of
58:     const baremetal::string m_fixtureName;
59:     /// @brief Test name
60:     const baremetal::string m_testName;
61:     /// @brief Source file name of test
62:     const baremetal::string m_fileName;
63:     /// @brief Source line number of test
64:     const int m_lineNumber;
65:
66: public:
67:     /// @brief Name of default test fixture. Used for tests that are not in a test fixture
68:     static const char* DefaultFixtureName;
69:     /// @brief Name of default test suite. Used for tests and test fixtures that are not in a test suite
70:     static const char* DefaultSuiteName;
71:
72:     TestDetails();
73:     TestDetails(const baremetal::string& testName, const baremetal::string& fixtureName, const baremetal::string& suiteName, const baremetal::string& fileName, int lineNumber);
74:     TestDetails(const TestDetails& other, int lineNumber);
75:
76:     baremetal::string SuiteName() const;
77:     baremetal::string FixtureName() const;
78:     baremetal::string TestName() const;
79:     baremetal::string QualifiedTestName() const;
80:     baremetal::string SourceFileName() const;
81:     int SourceFileLineNumber() const;
82: };
83:
84: } // namespace unittest
```

The `TestDetails` class is added to the `unittest` namespace.

- Line 42: We use strings, so we need to include the header for the `String` class
- Line 49-82: We declare the class `TestDetails` which will hold information on a test
  - Line 55-56: The class member variable `m_suiteName` is the test suite name
  - Line 57-58: The class member variable `m_fixtureName` is the test fixture name
  - Line 59-60: The class member variable `m_testName` is the test name
  - Line 61-62: The class member variable `m_fileName` is the source file in which the actual test is defined
  - Line 63-64: The class member variable `m_lineNumber` is the source line in which the actual test is defined
  - Line 67-68: The public constant member variable `DefaultFixtureName` is the default name of a test fixture, if none is specified
  - Line 69-70: The public constant member variable `DefaultSuiteName` is the default name of a test suite, if none is specified
  - Line 72: We declare the default constructor
  - Line 73: We declare the normal constructor which specifies all the needed information
  - Line 74: We declare the kind of copy constructor, but with an override for the source line number
  - Line 76: We declare an accessor `SuiteName()` for the test suite name
  - Line 77: We declare an accessor `FixtureName()` for the test fixture name
  - Line 78: We declare an accessor `TestName()` for the test name
  - Line 79: We declare an accessor `QualifiedTestName()` which is a combination of the test suite, test fixture and test name, concatenated by double colons '::'
  - Line 80: We declare an accessor `SourceFileName()` for the source file name
  - Line 81: We declare an accessor `SourceFileLineNumber()` for the source line number

### TestDetails.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3_TESTDETAILSCPP}

Let's implement the `TestDetails` class.

Create the file `code/libraries/unittest/src/TestDetails.cpp`

```cpp
File: code/libraries/unittest/src/TestDetails.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/TestDetails.h"
41:
42: #include "baremetal/Format.h"
43:
44: /// @file
45: /// Test details implementation
46:
47: using namespace baremetal;
48:
49: namespace unittest {
50:
51: const char* TestDetails::DefaultFixtureName = "DefaultFixture";
52: const char* TestDetails::DefaultSuiteName = "DefaultSuite";
53:
54: /// <summary>
55: /// Default constructor
56: /// </summary>
57: TestDetails::TestDetails()
58:     : m_suiteName{}
59:     , m_fixtureName{}
60:     , m_testName{}
61:     , m_fileName{}
62:     , m_lineNumber{}
63: {
64: }
65:
66: /// <summary>
67: /// Explicit constructor
68: /// </summary>
69: /// <param name="testName">Test name</param>
70: /// <param name="fixtureName">Name of test fixture test is part of</param>
71: /// <param name="suiteName">Name of test suite test is part of</param>
72: /// <param name="fileName">Source file name of test</param>
73: /// <param name="lineNumber">Source line number of test</param>
74: TestDetails::TestDetails(const String& testName, const String& fixtureName, const String& suiteName, const String& fileName, int lineNumber)
75:     : m_suiteName{suiteName}
76:     , m_fixtureName{fixtureName}
77:     , m_testName{testName}
78:     , m_fileName{fileName}
79:     , m_lineNumber{lineNumber}
80: {
81: }
82:
83: /// <summary>
84: /// Construct from other test details, override source line number
85: /// </summary>
86: /// <param name="other">Test details to copy from</param>
87: /// <param name="lineNumber">Source line number to set</param>
88: TestDetails::TestDetails(const TestDetails& other, int lineNumber)
89:     : m_suiteName{other.m_suiteName}
90:     , m_fixtureName{other.m_fixtureName}
91:     , m_testName{other.m_testName}
92:     , m_fileName{other.m_fileName}
93:     , m_lineNumber{lineNumber}
94: {
95: }
96:
97: /// <summary>
98: /// Returns test suite name
99: /// </summary>
100: /// <returns>Test suite name</returns>
101: String TestDetails::SuiteName() const
102: {
103:     return m_suiteName.empty() ? String(DefaultSuiteName) : m_suiteName;
104: }
105:
106: /// <summary>
107: /// Returns test fixture name
108: /// </summary>
109: /// <returns>Test fixture name</returns>
110: String TestDetails::FixtureName() const
111: {
112:     return m_fixtureName.empty() ? String(DefaultFixtureName) : m_fixtureName;
113: }
114:
115: /// <summary>
116: /// Returns test name
117: /// </summary>
118: /// <returns>Test name</returns>
119: String TestDetails::TestName() const
120: {
121:     return m_testName;
122: }
123:
124: /// <summary>
125: /// Return fully qualified test name in format [suite]::[fixture]::[test]
126: /// </summary>
127: /// <returns>Resulting String</returns>
128: String TestDetails::QualifiedTestName() const
129: {
130:     return Format("%s::%s::%s", SuiteName().c_str(), FixtureName().c_str(), TestName().c_str());
131: }
132:
133: /// <summary>
134: /// Returns test source file name
135: /// </summary>
136: /// <returns>Test source file name</returns>
137: String TestDetails::SourceFileName() const
138: {
139:     return m_fileName;
140: }
141:
142: /// <summary>
143: /// Returns test source line number
144: /// </summary>
145: /// <returns>Test source line number</returns>
146: int TestDetails::SourceFileLineNumber() const
147: {
148:     return m_lineNumber;
149: }
150:
151: } // namespace unittest
```

- Line 51: We initialize the constant `DefaultFixtureName`
- Line 52: We initialize the constant `DefaultSuiteName`
- Line 54-64: We implement the default constructor
- Line 66-81: We implement the non default constructor
- Line 83-95: We implement the "copy" constructor
- Line 97-104: We implement the `SuiteName()` method, which has a fallback to use the default if needed
- Line 106-113: We implement the `FixtureName()` method, which has a fallback to use the default if needed
- Line 115-122: We implement the `TestName()` method
- Line 124-131: We implement the `QualifiedTestName()` method
- Line 133-140: We implement the `SourceFileName()` method
- Line 142-149: We implement the `SourceFileLineNumber()` method

### TestResults.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3_TESTRESULTSH}

In order to use the `TestInfo` class, we need to declare `TestResults`.
For now we'll simply declare a simple class with a default constructor.
We'll fill in the details later.

Create the file `code/libraries/unittest/include/unittest/TestResults.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResults.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: /// @file
43: /// Test results
44: ///
45: /// Results for a complete test run
46:
47: namespace unittest {
48:
49: /// <summary>
50: /// Test results
51: ///
52: /// Holds the test results for a complete test run
53: /// </summary>
54: class TestResults
55: {
56: public:
57:     explicit TestResults() = default;
58:     TestResults(const TestResults&) = delete;
59:     TestResults(TestResults&&) = delete;
60:     virtual ~TestResults() = default;
61:
62:     TestResults& operator=(const TestResults&) = delete;
63:     TestResults& operator=(TestResults&&) = delete;
64: };
65:
66: } // namespace unittest
```

### CurrentTest.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3_CURRENTTESTH}

When running tests, we need a way to keep track of the details for the current test, as well as the test run results.
We therefore introduce a simple class `CurrentTest` that holds pointers to both.

Create the file `code/libraries/unittest/include/unittest/CurrentTest.h`

```cpp
File: code/libraries/unittest/include/unittest/CurrentTest.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
45: /// @brief unittest namespace
46: namespace unittest {
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

### CurrentTest.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3_CURRENTTESTCPP}

We'll implement the `CurrentTest` class.

Create the file `code/libraries/unittest/src/CurrentTest.cpp`

```cpp
File: code/libraries/unittest/src/CurrentTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/CurrentTest.h"
41:
42: /// @file
43: /// Current test information implementation
44:
45: namespace unittest {
46:
47: /// <summary>
48: /// Returns reference to static test results pointer
49: ///
50: /// As a reference is returned, the pointer can also be set. This pointer is used to keep track of test results during the test run.
51: /// </summary>
52: /// <returns>A reference to the current test results pointer</returns>
53: TestResults*& CurrentTest::Results()
54: {
55:     static TestResults* testResults = nullptr;
56:     return testResults;
57: }
58:
59: /// <summary>
60: /// Returns reference to static test details pointer
61: ///
62: /// As a reference is returned, the pointer can also be set. This pointer is used to keep track of test details during the test run.
63: /// </summary>
64: /// <returns>A reference to the current test details pointer</returns>
65: const TestDetails*& CurrentTest::Details()
66: {
67:     static const TestDetails* testDetails = nullptr;
68:     return testDetails;
69: }
70:
71: } // namespace unittest
```

- Line 47-57: We implement the `Results()` method.
This simply returns a pointer to the static variable inside the function, which can the be set and used
- Line 59-69: We implement the `Details()` method.
This simply returns a pointer to the static variable inside the function, which can the be set and used

### TestInfo.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3_TESTINFOH}

Next we will add the test administration class `TestInfo`.
This class will be used to register information for a test.

Create the file `code/libraries/unittest/include/unittest/TestInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestInfo.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "unittest/TestDetails.h"
43:
44: /// @file
45: /// Test administration
46:
47: namespace unittest {
48:
49: class Test;
50: class TestResults;
51:
52: /// <summary>
53: /// Test administration class
54: /// </summary>
55: class TestInfo
56: {
57: private:
58:     /// @brief Test details
59:     const TestDetails m_details;
60:     /// @brief Pointer to actual test
61:     Test* m_testInstance;
62:     /// @brief Pointer to next test in list
63:     TestInfo* m_next;
64:
65: public:
66:     TestInfo();
67:     TestInfo(const TestInfo&) = delete;
68:     TestInfo(TestInfo&&) = delete;
69:     explicit TestInfo(Test* testInstance, const TestDetails& details);
70:
71:     TestInfo& operator=(const TestInfo&) = delete;
72:     TestInfo& operator=(TestInfo&&) = delete;
73:
74:     /// <summary>
75:     /// Returns the test details
76:     /// </summary>
77:     /// <returns>Test details</returns>
78:     const TestDetails& Details() const
79:     {
80:         return m_details;
81:     }
82:
83:     /// <summary>
84:     /// Returns the pointer to the next test in the list
85:     /// </summary>
86:     /// <returns>Pointer to next test</returns>
87:     const TestInfo* Next() const
88:     {
89:         return m_next;
90:     }
91:
92:     /// <summary>
93:     /// Returns a reference to the pointer to the next test in the list
94:     /// </summary>
95:     /// <returns>Reference to pointer to next test</returns>
96:     TestInfo*& Next()
97:     {
98:         return m_next;
99:     }
100:
101:     void Run(TestResults& testResults);
102: };
103:
104: } // namespace unittest
```

The `TestInfo` class is again added to the `unittest` namespace.

- Line 49: We forward declare the `Test` class.
- Line 50: We forward declare the `TestResults` class.
- Line 52-84: We declare the class `TestInfo`
  - Line 58-59: We declare the details for the test `m_details`
  - Line 60-61: We declare a pointer to the actual test to run `m_testInstance`
  - Line 62-63: We declare a pointer to the next test `m_next`.
Tests administration classes will be stored in a linked list
  - Line 66: We declare a default constructor
  - Line 67-68: We remove the copy constructor and move constructor
  - Line 69: We declare an explicit constructor
  - Line 71-72: We remove the assignment operators
  - Line 74-81: We declare and implement a method `Details()` to retrieve details
  - Line 83-90: We declare and implement a method `Next()` to retrieve the pointer to the next test in the list.
This is a const function returning a const pointer
  - Line 92-99: We declare and implement a method `Next()` to retrieve a reference to the pointer to the next test in the list.
This is a non-const function returning a reference to a non-const pointer
  - Line 101: We declare a method `Run()` to run the test and update the test results

### TestInfo.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3_TESTINFOCPP}

We'll implement the `TestInfo` class.

Create the file `code/libraries/unittest/src/TestInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestInfo.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/TestInfo.h"
41:
42: #include "unittest/CurrentTest.h"
43: #include "unittest/Test.h"
44: #include "unittest/TestResults.h"
45:
46: /// @file
47: /// Test case administration implementation
48:
49: using namespace baremetal;
50:
51: namespace unittest {
52:
53: /// <summary>
54: /// Default constructor
55: /// </summary>
56: TestInfo::TestInfo()
57:     : m_details{}
58:     , m_testInstance{}
59:     , m_next{}
60: {
61: }
62:
63: /// <summary>
64: /// Explicit constructor
65: /// </summary>
66: /// <param name="testInstance">Test instance</param>
67: /// <param name="details">Test details</param>
68: TestInfo::TestInfo(Test* testInstance, const TestDetails& details)
69:     : m_details{details}
70:     , m_testInstance{testInstance}
71:     , m_next{}
72: {
73: }
74:
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
86:         m_testInstance->RunImpl();
87:     }
88: }
89:
90: } // namespace unittest
```

- Line 53-61: We implement the default constructor
- Line 63-73: We implement the non default constructor
- Line 75-88: We implement the `Run()` method

### Update application code {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3_UPDATE_APPLICATION_CODE}

Let's start using the class we just created. We'll add a simple test case by declaring and implementing a class derived from `TestBase`.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Console.h"
2: #include "baremetal/Logger.h"
3: #include "baremetal/System.h"
4: #include "baremetal/Timer.h"
5:
6: #include "unittest/CurrentTest.h"
7: #include "unittest/Test.h"
8: #include "unittest/TestInfo.h"
9: #include "unittest/TestResults.h"
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
35:     CurrentTest::Details() = &myTestInfo.Details();
36:     CurrentTest::Results() = &results;
37:
38:     myTestInfo.Run(results);
39:
40:     LOG_INFO("Wait 5 seconds");
41:     Timer::WaitMilliSeconds(5000);
42:
43:     console.Write("Press r to reboot, h to halt\n");
44:     char ch{};
45:     while ((ch != 'r') && (ch != 'h'))
46:     {
47:         ch = console.ReadChar();
48:         console.WriteChar(ch);
49:     }
50:
51:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
52: }
```

- Line 6: We include the header for `CurrentTest`
- Line 8: We include the header for `TestInfo`
- Line 9: We include the header for `TestResults`
- Line 33: We define an instance of `TestInfo`, specifying the test name, test fixture name, test suite name, and the source location.
We link the test instance `myTest` to the `TestInfo` instance
- Line 34: We define an instance of `TestResults`
- Line 35-36: We set the current details and results for the test.
- Line 38: We run the test through the `Run()` method of `TestInfo`, passing the `TestResults` instance

### Configuring, building and debugging {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_ADMINISTRATION___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the test again, and the output is almost identical to the previous step.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
Info   0.00:00:00.020 Running test (main:25)
Info   0.00:00:00.030 Wait 5 seconds (main:40)
Press r to reboot, h to halt
hInfo   0.00:00:19.810 Halt (System:121)
```

## Adding test fixtures - Step 4 {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES___STEP_4}

Now that we have a test we can run, let's add the test fixture, to hold multiple tests, and provide for a setup / teardown method call.
Test fixtures are slightly different from tests in their structure.
- A `Test` only has a `RunImpl()` method which can be overridden.
A `Test` class is overridden by a test implementation
- A `TestFixture` class has a `RunImpl()` method which can be overridden, as well as a `SetUp()` and `TearDown()` method.
These methods can be overridden to implement setting up and tearing down context for all tests that are part of the test fixture.
The `TestFixture` class is inherited by a fixture helper class for each test in the fixture, that acts a test, but is constructed before running the test, and destructed afterwards.
The inheriting class will have a `RunImpl()` method that will run the actual test. Its constructor will run the `SetUp()` method, the destructor will run the `TearDown()` method.
A `TestFixture` will have an accompanying class inheriting from `Test`.
Its `RunImpl()` method will instantiate the inheriting `TextFixture` class, run its `RunImpl()` method and destruct it again

- A `TestInfo` class holds the test details, a pointer to the actual `Test` instantiation and a pointer to the next test in the list
The `TestDetails` holds only information on a test, such as the test name, test fixture name, etc.
- A `TestFixtureInfo` class holds the a pointer to the next fixture, and the pointers to the first and last test in the fixture, as pointers to `TestInfo`.

See also the image below.

<img src="images/unittest-class-structure-test-fixture.png" alt="Tree view" width="800"/>

Things will become more clear when we start using the test fixtures.

### TestFixture.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES___STEP_4_TESTFIXTUREH}

The `TestFixture` class is as said simply a base class for helper derivatives.

Create the file `code/libraries/unittest/include/unittest/TestFixture.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixture.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
60:     TestFixture& operator=(const TestFixture&) = delete;
61:     TestFixture& operator=(TestFixture&&) = delete;
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

- Line 47-71: We declare the `TestFixture` class
  - Line 55: We give the default constructor a default implementation
  - Line 56-57: We remove the copy and move constructor
  - Line 58: We give the destructor a default implementation
  - Line 60-61: We remove the assignment and move assignment operator
  - Line 66-70: We declare and implement the virtual `SetUp()` and `TearDown()` methods.
The default implementation does nothing

As can be seen, nothing else needs to be added for implementation.

### TestFixtureInfo.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES___STEP_4_TESTFIXTUREINFOH}

So let's declare the `TestFixtureInfo` class.

Create the file `code/libraries/unittest/include/unittest/TestFixtureInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixtureInfo.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : TestFixtureInfo.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestFixtureInfo
9: //
10: // Description : Test fixture info
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
42: #include "unittest/TestInfo.h"
43: #include "unittest/TestResults.h"
44:
45: /// @file
46: /// Test fixture administration
47:
48: namespace unittest {
49:
50: class TestInfo;
51:
52: /// <summary>
53: /// Test fixture administration
54: ///
55: /// Holds information on a test fixture, which includes its name and the list of tests that are part of it
56: /// </summary>
57: class TestFixtureInfo
58: {
59: private:
60:     /// @brief Pointer to first test in the list
61:     TestInfo* m_head;
62:     /// @brief Pointer to last test in the list
63:     TestInfo* m_tail;
64:     /// @brief Pointer to next test fixture info in the list
65:     TestFixtureInfo* m_next;
66:     /// @brief Test fixture name
67:     baremetal::String m_fixtureName;
68:
69: public:
70:     TestFixtureInfo() = delete;
71:     TestFixtureInfo(const TestFixtureInfo&) = delete;
72:     TestFixtureInfo(TestFixtureInfo&&) = delete;
73:     explicit TestFixtureInfo(const baremetal::String& fixtureName);
74:     virtual ~TestFixtureInfo();
75:
76:     TestFixtureInfo& operator=(const TestFixtureInfo&) = delete;
77:     TestFixtureInfo& operator=(TestFixtureInfo&&) = delete;
78:
79:     /// <summary>
80:     /// Returns the pointer to the first test in the list for this test fixture
81:     /// </summary>
82:     /// <returns>Pointer to the first test in the list for this test fixture</returns>
83:     TestInfo* FirstTest() const
84:     {
85:         return m_head;
86:     }
87:
88:     /// <summary>
89:     /// Returns the pointer to the next test in the list from the current pointer
90:     /// </summary>
91:     /// <param name="current">Pointer to current test in the list</param>
92:     /// <returns>Pointer to the next test in the list</returns>
93:     TestInfo* NextTest(TestInfo* current) const
94:     {
95:         return current->Next();
96:     }
97:
98:     /// <summary>
99:     /// Returns the pointer to the next test fixture in the list
100:     /// </summary>
101:     /// <returns>Pointer to the next test fixture in the list</returns>
102:     TestFixtureInfo* Next() const
103:     {
104:         return m_next;
105:     }
106:
107:     /// <summary>
108:     /// Returns a reference to the pointer to the next test fixture in the list
109:     /// </summary>
110:     /// <returns>Reference to pointer to next test fixture</returns>
111:     TestFixtureInfo*& Next()
112:     {
113:         return m_next;
114:     }
115:
116:     baremetal::String Name() const;
117:
118:     void Run(TestResults& testResults);
119:
120:     int CountTests();
121:
122:     void AddTest(TestInfo* test);
123: };
124:
125: } // namespace unittest
```

- Line 60-61: The member variable `m_head` stores the pointer to the first test in the fixture
- Line 62-63: The member variable `m_tail` stores the pointer to the last test in the fixture
- Line 64-65: The member variable `m_next` is the pointer to the next test fixture.
Again, test fixtures are stored in linked list
- Line 66-67: The member variable `m_fixtureName` holds the name of the test fixture
- Line 70: We remove the default constructor
- Line 71-72 We remove the copy and move constructors
- Line 73: We declare the only usable constructor which receives the test fixture name
- Line 74: We declare the destructor
- Line 76-77: We remove the assignment and move assignment operators
- Line 79-86: The method `FirstTest()` returns the pointer to the first test in the list
- Line 88-96: The method `NextTest()` returns the pointer to the next test in the list starting from the current pointer
- Line 98-105: The method `Next()` returns the pointer to the next test fixture in the list
- Line 107-114: The method `Next()` returns a reference to the pointer to the next test fixture in the list
- Line 116: The method `Name()` returns the test fixture name
- Line 118: The method `Run()` runs all tests in the test fixture and update the test results. We'll be revisiting this later
- Line 120: The method `CountTests()` counts and returns the number of tests in the test fixture
- Line 122: The method `AddTest()` adds a test to the list for the test fixture

### TestFixtureInfo.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES___STEP_4_TESTFIXTUREINFOCPP}

Let's implement the `TestFixtureInfo` class.

Create the file `code/libraries/unittest/src/TestFixtureInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestFixtureInfo.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
39: #include "unittest/TestFixtureInfo.h"
40:
41: #include "baremetal/Assert.h"
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
54: TestFixtureInfo::TestFixtureInfo(const String& fixtureName)
55:     : m_head{}
56:     , m_tail{}
57:     , m_next{}
58:     , m_fixtureName{fixtureName}
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
69:     TestInfo* test = FirstTest();
70:     while (test != nullptr)
71:     {
72:         const TestInfo* currentTest = test;
73:         test = NextTest(test);
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
94:         m_tail->Next() = test;
95:         m_tail = test;
96:     }
97: }
98:
99: /// <summary>
100: /// Returns the test fixture name
101: /// </summary>
102: /// <returns>Test fixture name</returns>
103: String TestFixtureInfo::Name() const
104: {
105:     return m_fixtureName.empty() ? String(TestDetails::DefaultFixtureName) : m_fixtureName;
106: }
107:
108: /// <summary>
109: /// Run tests in test fixture, updating the test results
110: /// </summary>
111: /// <param name="testResults">Test results to use and update</param>
112: void TestFixtureInfo::Run(TestResults& testResults)
113: {
114:     testResults.OnTestFixtureStart(this);
115:
116:     TestInfo* test = FirstTest();
117:     while (test != nullptr)
118:     {
119:         test->Run(testResults);
120:         test = NextTest(test);
121:     }
122:
123:     testResults.OnTestFixtureFinish(this);
124: }
125:
126: /// <summary>
127: /// Count the number of tests in the test fixture
128: /// </summary>
129: /// <returns>Number of tests in the test fixture</returns>
130: int TestFixtureInfo::CountTests()
131: {
132:     int numberOfTests = 0;
133:     TestInfo* test = FirstTest();
134:     while (test != nullptr)
135:     {
136:         ++numberOfTests;
137:         test = NextTest(test);
138:     }
139:     return numberOfTests;
140: }
141:
142: } // namespace unittest
```

- Line 50-60: We implement the constructor
- Line 62-76: We implement the destructor. This goes through the list of tests, and deletes every one of these. Note that we will therefore need to create the tests on the heap.
- Line 78-97 We implement the `AddTest()` method. This will add the test passed in at the end of the list
- Line 99-106: We implement the `Name()` method.
This returns the test fixture name, unless it is empty, in which case we return the default fixture name
- Line 108-124: We implement the `Run()` method.
This goes through the list of tests, and calls `Run()` on each
- Line 126-140: We implement the `CountTests()` method.
This goes through the list of tests, and counts them

### TestResults.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES___STEP_4_TESTRESULTSH}

We will add some methods to the `TestResults` class to indicate what we're moving towards.

Update the file `code/libraries/unittest/include/unittest/TestResults.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResults.h
...
42: #include "baremetal/String.h"
43:
44: /// @file
45: /// Test results
46: ///
47: /// Results for a complete test run
48:
49: namespace unittest {
50:
51: class TestDetails;
52:
53: /// <summary>
54: /// Test results
55: ///
56: /// Holds the test results for a complete test run
57: /// </summary>
58: class TestResults
59: {
60: public:
61:     explicit TestResults() = default;
62:     TestResults(const TestResults&) = delete;
63:     TestResults(TestResults&&) = delete;
64:     virtual ~TestResults() = default;
65:
66:     TestResults& operator=(const TestResults&) = delete;
67:     TestResults& operator=(TestResults&&) = delete;
68:
69:     void OnTestStart(const TestDetails& details);
70:     void OnTestRun(const TestDetails& details, const baremetal::String& message);
71:     void OnTestFinish(const TestDetails& details);
72: };
73:
74: } // namespace unittest
...
```

- Line 69: We declare the method `OnTestStart()` to indicate the start of a test
- Line 70: We declare the method `OnTestRun()` to indicate we're running a test (this will be changed later)
- Line 71: We declare the method `OnTestFinish()` to indicate the end of a test

### TestResults.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES___STEP_4_TESTRESULTSCPP}

Let's implement the new methods in the `Testresults` class.

Create the file `code/libraries/unittest/src/TestResults.cpp`

```cpp
File: code/libraries/unittest/src/TestResults.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/TestResults.h"
41:
42: #include "baremetal/Format.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/String.h"
45: #include "unittest/TestDetails.h"
46:
47: /// @file
48: /// Test results implementation
49:
50: using namespace baremetal;
51:
52: /// @brief Define log name
53: LOG_MODULE("TestResults");
54:
55: namespace unittest {
56:
57: /// <summary>
58: /// Start a test
59: /// </summary>
60: /// <param name="details">Test details of test to start</param>
61: void TestResults::OnTestStart(const TestDetails& details)
62: {
63:     LOG_INFO(details.QualifiedTestName() + " Start test");
64: }
65:
66: /// <summary>
67: /// Run a test
68: /// </summary>
69: /// <param name="details"></param>
70: /// <param name="message">Test message string</param>
71: void TestResults::OnTestRun(const TestDetails& details, const String& message)
72: {
73:     String fullMessage = details.QualifiedTestName() + Format(" (%s:%d)", details.SourceFileName().c_str(), details.SourceFileLineNumber()) + " --> " + message;
74:     LOG_DEBUG(fullMessage);
75: }
76:
77: /// <summary>
78: /// Finish a test
79: /// </summary>
80: /// <param name="details">Test details of test to finish</param>
81: void TestResults::OnTestFinish(const TestDetails& details)
82: {
83:     LOG_INFO(details.QualifiedTestName() + " Finish test");
84: }
85:
86: } // namespace unittest
```

- Line 57-64: We implement the methos `OnTestStart()`
- Line 66-75: We implement the methos `OnTestRun()`
- Line 77-84: We implement the methos `OnTestFinish()`

As this code will change soon, it's not documented now.

### TestInfo.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES___STEP_4_TESTINFOCPP}

As we have added some methods to `TestResults`, we'll update the `Run()` method in `TestInfo` to call these.

Update the file `code/libraries/unittest/src/TestInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestInfo.cpp
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

- Line 75-92: Next to calling the `RunImpl()` method on the test, we also call the `OnTestStart()` and `OnTestFinish()` methods on the `TestResults` class

### Update application code {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES___STEP_4_UPDATE_APPLICATION_CODE}

Let's start using the test fixtures.
We'll add a couple of simple test cases by declaring and implementing a class derived from `TestBase`.
We'll then create a test fixture, and add the tests to the fixture.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Console.h"
2: #include "baremetal/Logger.h"
3: #include "baremetal/System.h"
4: #include "baremetal/Timer.h"
5:
6: #include "unittest/CurrentTest.h"
7: #include "unittest/Test.h"
8: #include "unittest/TestFixture.h"
9: #include "unittest/TestFixtureInfo.h"
10: #include "unittest/TestInfo.h"
11: #include "unittest/TestResults.h"
12:
13: LOG_MODULE("main");
14:
15: using namespace baremetal;
16: using namespace unittest;
17:
18: class FixtureMyTest
19:     : public TestFixture
20: {
21: public:
22:     void SetUp() override
23:     {
24:         LOG_DEBUG("MyTest SetUp");
25:     }
26:     void TearDown() override
27:     {
28:         LOG_DEBUG("MyTest TearDown");
29:     }
30: };
31:
32: class FixtureMyTest1Helper
33:     : public FixtureMyTest
34: {
35: public:
36:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
37:     explicit FixtureMyTest1Helper(const TestDetails& details)
38:         : m_details{ details }
39:     {
40:         SetUp();
41:     }
42:     virtual ~FixtureMyTest1Helper()
43:     {
44:         TearDown();
45:     }
46:     void RunImpl() const;
47:     const TestDetails& m_details;
48: };
49: void FixtureMyTest1Helper::RunImpl() const
50: {
51:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 1");
52: }
53:
54: class MyTest1
55:     : public Test
56: {
57:     void RunImpl() const override
58:     {
59:         LOG_DEBUG("Test 1");
60:         FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
61:         fixtureHelper.RunImpl();
62:     }
63: } myTest1;
64: class MyTestInfo1
65:     : public TestInfo
66: {
67: public:
68:     MyTestInfo1(Test* testInstance)
69:         : TestInfo(testInstance, TestDetails("MyTest1", "FixtureMyTest", "", __FILE__, __LINE__))
70:     {
71:     }
72: };
73:
74: class FixtureMyTest2Helper
75:     : public FixtureMyTest
76: {
77: public:
78:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
79:     explicit FixtureMyTest2Helper(const TestDetails& details)
80:         : m_details{ details }
81:     {
82:         SetUp();
83:     }
84:     virtual ~FixtureMyTest2Helper()
85:     {
86:         TearDown();
87:     }
88:     void RunImpl() const;
89:     const TestDetails& m_details;
90: };
91: void FixtureMyTest2Helper::RunImpl() const
92: {
93:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 2");
94: }
95:
96: class MyTest2
97:     : public Test
98: {
99:     void RunImpl() const override
100:     {
101:         LOG_DEBUG("Test 2");
102:         FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
103:         fixtureHelper.RunImpl();
104:     }
105: } myTest2;
106: class MyTestInfo2
107:     : public TestInfo
108: {
109: public:
110:     MyTestInfo2(Test* testInstance)
111:         : TestInfo(testInstance, TestDetails("MyTest2", "FixtureMyTest", "", __FILE__, __LINE__))
112:     {
113:     }
114: };
115:
116: class FixtureMyTest3Helper
117:     : public FixtureMyTest
118: {
119: public:
120:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
121:     explicit FixtureMyTest3Helper(const TestDetails& details)
122:         : m_details{ details }
123:     {
124:         SetUp();
125:     }
126:     virtual ~FixtureMyTest3Helper()
127:     {
128:         TearDown();
129:     }
130:     void RunImpl() const;
131:     const TestDetails& m_details;
132: };
133: void FixtureMyTest3Helper::RunImpl() const
134: {
135:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 3");
136: }
137:
138: class MyTest3
139:     : public Test
140: {
141:     void RunImpl() const override
142:     {
143:         LOG_DEBUG("Test 3");
144:         FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
145:         fixtureHelper.RunImpl();
146:     }
147: } myTest3;
148: class MyTestInfo3
149:     : public TestInfo
150: {
151: public:
152:     MyTestInfo3(Test* testInstance)
153:         : TestInfo(testInstance, TestDetails("MyTest3", "FixtureMyTest", "", __FILE__, __LINE__))
154:     {
155:     }
156: };
157:
158: class MyTest
159:     : public Test
160: {
161: public:
162:     void RunImpl() const override;
163: } myTest;
164:
165: void MyTest::RunImpl() const
166: {
167:     CurrentTest::Results()->OnTestRun(*CurrentTest::Details(), "Running test");
168: }
169:
170: int main()
171: {
172:     auto& console = GetConsole();
173:
174:     TestInfo* test1 = new MyTestInfo1(&myTest1);
175:     TestInfo* test2 = new MyTestInfo2(&myTest2);
176:     TestInfo* test3 = new MyTestInfo3(&myTest3);
177:     TestFixtureInfo* fixture = new TestFixtureInfo("MyFixture");
178:     TestResults results;
179:     fixture->AddTest(test1);
180:     fixture->AddTest(test2);
181:     fixture->AddTest(test3);
182:     fixture->Run(results);
183:     delete fixture;
184:     TestInfo myTestInfo(&myTest, TestDetails("MyTest", "", "", __FILE__, __LINE__));
185:
186:     myTestInfo.Run(results);
187:
188:     LOG_INFO("Wait 5 seconds");
189:     Timer::WaitMilliSeconds(5000);
190:
191:     console.Write("Press r to reboot, h to halt\n");
192:     char ch{};
193:     while ((ch != 'r') && (ch != 'h'))
194:     {
195:         ch = console.ReadChar();
196:         console.WriteChar(ch);
197:     }
198:
199:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
200: }
```

We create a slightly more complicated test setup:
- Three test `MyTest1`, `MyTest2`, `MyTest3` which are part of a fixture
- A test `MyTest` which is not part of a fixture

This is largely similar to how we will be creating test fixtures and tests later on.

- Line 6: We include the header for `CurrentTest`
- Line 8: We include the header for `TestFixture`
- Line 9: We include the header for `TestFixtureInfo`
- Line 11: We include the header for `TestResults`
- Line 18-30: We declare and implement the class `FixtureMyTest` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods, simply by logging a message
- Line 32-52: We declare and implement the class `FixtureMyTest1Helper` which derives from the class `FixtureMyTest` just defined, and implements the constructor and destructor, calling resp. `SetUp()` and `TearDown()`.
This class also defines a `RunImpl()` method, which forms the actual test body for the test MyTest1.
It is implemented as a call to the `OnTestRun()` method on the `TestResults` instance
- Line 54-63: We declare and implement the class `MyTest1`, which derives from `Test`, and acts as the placeholders for the fixture test.
We instantiate it as `myTest1`.
Its `RunImpl()` method instantiates `FixtureMyTest1Helper`, and runs its `RunImpl()` method
- Line 64-72: We declare and implement the class `MyTestInfo1`, which derives from `TestInfo`.
Its constructor links the test instance
- Line 74-94: We declare and implement the class `FixtureTest2Helper` similar to `FixtureMyTest1Helper`
- Line 96-105: We declare and implement the class `MyTest2` similar to `MyTest1`, and instantiate it as `myTest2`
- Line 106-114: We declare and implement the class `MyTestInfo2` similar to `MyTestInfo1`
- Line 116-136: We declare and implement the class `FixtureTest3Helper` similar to `FixtureMyTest1Helper`
- Line 138-147: We declare and implement the class `MyTest3` similar to `MyTest1`, and instantiate it as `myTest3`
- Line 148-156: We declare and implement the class `MyTestInfo3` similar to `MyTestInfo1`
- Line 158-163: We declare the class `MyTest`, which derives from `Test`, and instantiate it as `myTest`
- Line 165-168: We implement the `RunImpl()` method for `MyTest` as a call to the `OnTestRun()` method on the `TestResults` instance
- Line 174-176: We instantiate each of `MyTestInfo1`, `MyTestInfo2` and `MyTestInfo3`, passing in the instance of the corresponding test
- Line 177: We instantiate `TestFixtureInfo` as our test fixture, this will be the container for our tests
- Line 179-181: We add the three tests to the test fixture
- Line 182: We run the test fixture
- Line 183: We clean up the test fixture. Note that the test fixture destructor deletes all test administration instances, so we don't need to (and shouldn't) do that.
The tests themselves are normally instantiated at static initialization time, so will never be destructed
- Line 184: We instantiate a `TestInfo`, linked to the `myTest` instance
- Line 186: We run the test

This all seems like quite a bit of plumbing just to run three tests in a test fixture and one single test.
That is why we'll create macros later to do this work for us.
But it's good to understand what is happening under the hood.

### Configuring, building and debugging {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_FIXTURES___STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests in the test fixture, and therefore show the log output.
You'll see that for each test the `RunImpl()` method of `MyTest<x>` runs.
This then instantiates the `FixtureTest<x>Helper`, and its constructor runs the `FixtureMyTest` method `SetUp()`.
Then the `RunImpl()` of `FixtureTest<x>Helper` is run, and finally the class is destructed again, leading to the `FixtureMyTest` method `TearDown()` begin run.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
Info   0.00:00:00.020 DefaultSuite::FixtureMyTest::MyTest1 Start test (TestResults:60)
Debug  0.00:00:00.030 Test 1 (main:59)
Debug  0.00:00:00.030 MyTest SetUp (main:24)
Debug  0.00:00:00.040 DefaultSuite::FixtureMyTest::MyTest1 (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:69) --> MyTestHelper 1 (TestResults:68)
Debug  0.00:00:00.040 MyTest TearDown (main:28)
Info   0.00:00:00.050 DefaultSuite::FixtureMyTest::MyTest1 Finish test (TestResults:73)
Info   0.00:00:00.050 DefaultSuite::FixtureMyTest::MyTest2 Start test (TestResults:60)
Debug  0.00:00:00.050 Test 2 (main:101)
Debug  0.00:00:00.060 MyTest SetUp (main:24)
Debug  0.00:00:00.060 DefaultSuite::FixtureMyTest::MyTest2 (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:111) --> MyTestHelper 2 (TestResults:68)
Debug  0.00:00:00.070 MyTest TearDown (main:28)
Info   0.00:00:00.070 DefaultSuite::FixtureMyTest::MyTest2 Finish test (TestResults:73)
Info   0.00:00:00.070 DefaultSuite::FixtureMyTest::MyTest3 Start test (TestResults:60)
Debug  0.00:00:00.080 Test 3 (main:143)
Debug  0.00:00:00.080 MyTest SetUp (main:24)
Debug  0.00:00:00.080 DefaultSuite::FixtureMyTest::MyTest3 (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:153) --> MyTestHelper 3 (TestResults:68)
Debug  0.00:00:00.090 MyTest TearDown (main:28)
Info   0.00:00:00.090 DefaultSuite::FixtureMyTest::MyTest3 Finish test (TestResults:73)
Info   0.00:00:00.100 DefaultSuite::DefaultFixture::MyTest Start test (TestResults:60)
Debug  0.00:00:00.100 DefaultSuite::DefaultFixture::MyTest (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:185) --> Running test (TestResults:68)
Info   0.00:00:00.110 DefaultSuite::DefaultFixture::MyTest Finish test (TestResults:73)
Info   0.00:00:00.110 Wait 5 seconds (main:189)
Press r to reboot, h to halt
```

## Adding test suites - Step 5 {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES___STEP_5}

The final step in building infrasturcture for unit tests is collecting test fixtures in test suites.
Test suites are again different from tests and test fixtures in their structure.

- A TestSuite is actually nothing more than a function returning the suite name. The trick is that the function will be placed inside a namespace, as well as all the test fixtures and tests that belong inside it
- The `TestSuiteInfo` class holds the actual test suite information, such as the pointer to the next test suite, and the pointers to the first and last test fixture in the suite

So the `TestSuiteInfo` class holds the administration of the test suite, like `TestFixtureInfo` and `TestInfo` do for test fixtures and tests.

### TestSuite.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES___STEP_5_TESTSUITEH}

The header for the test suite is quite simple, it simple defines the global function `GetSuiteName()`, which is used when not in a namespace.

Create the file `code/libraries/unittest/include/unittest/TestSuite.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuite.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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

### TestSuiteInfo.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES___STEP_5_TESTSUITEINFOH}

Now let's declare the `TestSuiteInfo` class.

Create the file `code/libraries/unittest/include/unittest/TestSuiteInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuiteInfo.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "baremetal/String.h"
43: #include "unittest/TestFixtureInfo.h"
44: #include "unittest/TestResults.h"
45:
46: /// @file
47: /// Test suite administration
48:
49: namespace unittest {
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
68:     baremetal::String m_suiteName;
69:
70: public:
71:     TestSuiteInfo() = delete;
72:     TestSuiteInfo(const TestSuiteInfo&) = delete;
73:     TestSuiteInfo(TestSuiteInfo&&) = delete;
74:     explicit TestSuiteInfo(const baremetal::String& suiteName);
75:     virtual ~TestSuiteInfo();
76:
77:     TestSuiteInfo& operator=(const TestSuiteInfo&) = delete;
78:     TestSuiteInfo& operator=(TestSuiteInfo&&) = delete;
79:
80:     /// <summary>
81:     /// Returns the pointer to the first test fixture in the list for this test suite
82:     /// </summary>
83:     /// <returns>Pointer to the first test fixture in the list for this test suite</returns>
84:     TestFixtureInfo* FirstTestFixture() const
85:     {
86:         return m_head;
87:     }
88:
89:     /// <summary>
90:     /// Returns the pointer to the next test fixture in the list from the current pointer
91:     /// </summary>
92:     /// <param name="current">Pointer to current test fixture in the list</param>
93:     /// <returns>Pointer to the next test fixture in the list</returns>
94:     TestFixtureInfo* NextTestFixture(TestFixtureInfo* current) const
95:     {
96:         return current->Next();
97:     }
98:
99:     /// <summary>
100:     /// Returns the pointer to the next test suite in the list
101:     /// </summary>
102:     /// <returns>Pointer to the next test suite in the list</returns>
103:     TestSuiteInfo* Next() const
104:     {
105:         return m_next;
106:     }
107:
108:     /// <summary>
109:     /// Returns a reference to the pointer to the next test suite in the list
110:     /// </summary>
111:     /// <returns>Reference to pointer to next test suite</returns>
112:     TestSuiteInfo*& Next()
113:     {
114:         return m_next;
115:     }
116:
117:     baremetal::String Name() const;
118:
119:     TestFixtureInfo* GetTestFixture(const baremetal::String& fixtureName);
120:
121:     void Run(TestResults& testResults);
122:
123:     int CountFixtures();
124:     int CountTests();
125:
126:     void AddFixture(TestFixtureInfo* testFixture);
127: };
128:
129: } // namespace unittest
```

- Line 61-62: The member variable `m_head` stores the pointer to the first test fixture in the test suite
- Line 63-64: The member variable `m_tail` stores the pointer to the last test fixture in the test suite
- Line 65-66: The member variable `m_next` is the pointer to the next test suite.
Again, test suites are stored in linked list
- Line 67-68: The member variable `m_suiteName` holds the name of the test suite
- Line 71: We remove the default constructor
- Line 72-73: We remove the copy and move constructors
- Line 74: We declare the only usable constructor which receives the test suite name
- Line 75: We declare the destructor
- Line 77-78: We remove the assignment and move assignment operators
- Line 80-87: The method `FirstTestFixture()` returns the pointer to the first test fixture in the list
- Line 89-97: The method `NextTestFixture()` returns the pointer to the next test fixture in the list starting from the current pointer
- Line 99-106: The method `Next()` returns the pointer to the next test suite in the list
- Line 108-115: The method `Next()` returns a reference to the pointer to the next test suite in the list
- Line 117: The method `Name()` returns the test suite name
- Line 119: The method `GetTestFixture()` finds and returns a test fixture in the list for the test suite, or if not found, creates a new test fixture with the specified name
- Line 121: The method `Run()` runs all test fixtures in the test suite and update the test results. We'll be revisiting this later
- Line 123: The method `CountFixtures()` counts and returns the number of test fixtures in the test suite
- Line 124: The method `CountTests()` counts and returns the number of tests in all test fixtures in the test suite
- Line 126: The method `AddFixture()` adds a test fixture to the list for the test suite

### TestResults.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES___STEP_5_TESTRESULTSH}

We will add some more methods to the `TestResults` class for displaying start and finish of both test fixtures and test suites.

Update the file `code/libraries/unittest/include/unittest/TestResults.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResults.h
...
49: namespace unittest {
50:
51: class TestSuiteInfo;
52: class TestFixtureInfo;
53: class TestDetails;
54:
55: /// <summary>
56: /// Test results
57: ///
58: /// Holds the test results for a complete test run
59: /// </summary>
60: class TestResults
61: {
62: public:
63:     explicit TestResults() = default;
64:     TestResults(const TestResults&) = delete;
65:     TestResults(TestResults&&) = delete;
66:     virtual ~TestResults() = default;
67:
68:     TestResults& operator=(const TestResults&) = delete;
69:     TestResults& operator=(TestResults&&) = delete;
70:
71:     void OnTestSuiteStart(TestSuiteInfo* suite);
72:     void OnTestSuiteFinish(TestSuiteInfo* suite);
73:     void OnTestFixtureStart(TestFixtureInfo* fixture);
74:     void OnTestFixtureFinish(TestFixtureInfo* fixture);
75:     void OnTestStart(const TestDetails& details);
76:     void OnTestRun(const TestDetails& details, const baremetal::String& message);
77:     void OnTestFinish(const TestDetails& details);
78: };
79:
80: } // namespace unittest
...
```

- Line 71: We declare the method `OnTestSuiteStart()` to indicate the start of a test suite
- Line 72: We declare the method `OnTestSuiteFinish()` to indicate the finish of a test suite
- Line 73: We declare the method `OnTestFixtureStart()` to indicate the start of a test fixture
- Line 74: We declare the method `OnTestFixtureFinish()` to indicate the finish of a test fixture

### TestResults.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES___STEP_5_TESTRESULTSCPP}

Let's implement the new methods in the `Testresults` class.

Update the file `code/libraries/unittest/src/TestResults.cpp`

```cpp
File: code/libraries/unittest/src/TestResults.cpp
...
40: #include "unittest/TestResults.h"
41:
42: #include "baremetal/Format.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/String.h"
45: #include "unittest/TestDetails.h"
46: #include "unittest/TestFixtureInfo.h"
47: #include "unittest/TestSuiteInfo.h"
48:
...
59: /// <summary>
60: /// Start a test suite run
61: /// </summary>
62: /// <param name="suite">Test suite to start</param>
63: void TestResults::OnTestSuiteStart(TestSuiteInfo* suite)
64: {
65:     LOG_INFO(suite->Name() + " Start suite");
66: }
67:
68: /// <summary>
69: /// Finish a test suite run
70: /// </summary>
71: /// <param name="suite">Test suite to finish</param>
72: void TestResults::OnTestSuiteFinish(TestSuiteInfo* suite)
73: {
74:     LOG_INFO(suite->Name() + " Finish suite");
75: }
76:
77: /// <summary>
78: /// Start a test fixture run
79: /// </summary>
80: /// <param name="fixture">Test fixture to start</param>
81: void TestResults::OnTestFixtureStart(TestFixtureInfo* fixture)
82: {
83:     LOG_INFO(fixture->Name() + " Start fixture");
84: }
85:
86: /// <summary>
87: /// Finish a test fixture run
88: /// </summary>
89: /// <param name="fixture">Test fixture to finish</param>
90: void TestResults::OnTestFixtureFinish(TestFixtureInfo* fixture)
91: {
92:     LOG_INFO(fixture->Name() + " Finish fixture");
93: }
94:
95: /// <summary>
96: /// Start a test
97: /// </summary>
98: /// <param name="details">Test details of test to start</param>
99: void TestResults::OnTestStart(const TestDetails& details)
100: {
101:     LOG_INFO(details.QualifiedTestName() + " Start test");
102: }
103:
104: /// <summary>
105: /// Run a test
106: /// </summary>
107: /// <param name="details"></param>
108: /// <param name="message">Test message string</param>
109: void TestResults::OnTestRun(const TestDetails& details, const String& message)
110: {
111:     String fullMessage = details.QualifiedTestName() + Format(" (%s:%d)", details.SourceFileName().c_str(), details.SourceFileLineNumber()) + " --> " + message;
112:     LOG_DEBUG(fullMessage);
113: }
114:
115: /// <summary>
116: /// Finish a test
117: /// </summary>
118: /// <param name="details">Test details of test to finish</param>
119: void TestResults::OnTestFinish(const TestDetails& details)
120: {
121:     LOG_INFO(details.QualifiedTestName() + " Finish test");
122: }
123:
124: } // namespace unittest
```

- Line 46-47: We need to include the header for `TestFixtureInfo` and `TestSuiteInfo`
- Line 59-66: We implement the method `OnTestSuiteStart()`
- Line 68-75: We implement the method `OnTestSuiteFinish()`
- Line 77-84: We implement the method `OnTestFixtureStart()`
- Line 86-93: We implement the method `OnTestFixtureFinish()`

You will notice, that now we have documented the code as well.

### TestFixtureInfo.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES___STEP_5_TESTFIXTUREINFOCPP}

We'll change the `Run()` method in the `TestFixtureInfo` class to call the corresponding methods in the `TestResults` class.

Update the file `code/libraries/unittest/src/TestFixtureInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestFixtureInfo.cpp
...
99: /// <summary>
100: /// Run tests in test fixture, updating the test results
101: /// </summary>
102: /// <param name="testResults">Test results to use and update</param>
103: void TestFixtureInfo::Run(TestResults& testResults)
104: {
105:     testResults.OnTestFixtureStart(this);
106:
107:     TestInfo* test = FirstTest();
108:     while (test != nullptr)
109:     {
110:         test->Run(testResults);
111:         test = NextTest(test);
112:     }
113:
114:     testResults.OnTestFixtureFinish(this);
115: }
...
```

### TestSuiteInfo.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES___STEP_5_TESTSUITEINFOCPP}

Let's implement the `TestSuiteInfo` class.

Create the file `code/libraries/unittest/src/TestSuiteInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestSuiteInfo.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/TestSuiteInfo.h"
41:
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
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
59: TestSuiteInfo::TestSuiteInfo(const String& suiteName)
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
74:     TestFixtureInfo* testFixture = FirstTestFixture();
75:     while (testFixture != nullptr)
76:     {
77:         const TestFixtureInfo* currentFixture = testFixture;
78:         testFixture = NextTestFixture(testFixture);
79:         delete currentFixture;
80:     }
81: }
82:
83: /// <summary>
84: /// Find a test fixture with specified name, register a new one if not found
85: /// </summary>
86: /// <param name="fixtureName">Test fixture name to search for</param>
87: /// <returns>Found or created test fixture</returns>
88: TestFixtureInfo* TestSuiteInfo::GetTestFixture(const String& fixtureName)
89: {
90:     TestFixtureInfo* testFixture = FirstTestFixture();
91:     while ((testFixture != nullptr) && (testFixture->Name() != fixtureName))
92:         testFixture = NextTestFixture(testFixture);
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
105: void TestSuiteInfo::AddFixture(TestFixtureInfo* testFixture)
106: {
107:     if (m_tail == nullptr)
108:     {
109:         assert(m_head == nullptr);
110:         m_head = testFixture;
111:         m_tail = testFixture;
112:     }
113:     else
114:     {
115:         m_tail->Next() = testFixture;
116:         m_tail = testFixture;
117:     }
118: }
119:
120: /// <summary>
121: /// Returns the test suite name
122: /// </summary>
123: /// <returns>Test suite name</returns>
124: String TestSuiteInfo::Name() const
125: {
126:     return m_suiteName.empty() ? String(TestDetails::DefaultSuiteName) : m_suiteName;
127: }
128:
129: /// <summary>
130: /// Run tests in test suite, updating the test results
131: /// </summary>
132: /// <param name="testResults">Test results to use and update</param>
133: void TestSuiteInfo::Run(TestResults& testResults)
134: {
135:     testResults.OnTestSuiteStart(this);
136:
137:     TestFixtureInfo* testFixture = FirstTestFixture();
138:     while (testFixture != nullptr)
139:     {
140:         testFixture->Run(testResults);
141:         testFixture = NextTestFixture(testFixture);
142:     }
143:
144:     testResults.OnTestSuiteFinish(this);
145: }
146:
147: /// <summary>
148: /// Count the number of test fixtures in the test suite
149: /// </summary>
150: /// <returns>Number of test fixtures in the test suite</returns>
151: int TestSuiteInfo::CountFixtures()
152: {
153:     int numberOfTestFixtures = 0;
154:     TestFixtureInfo* testFixture = FirstTestFixture();
155:     while (testFixture != nullptr)
156:     {
157:         ++numberOfTestFixtures;
158:         testFixture = NextTestFixture(testFixture);
159:     }
160:     return numberOfTestFixtures;
161: }
162:
163: /// <summary>
164: /// Count the number of tests in the test suite
165: /// </summary>
166: /// <returns>Number of tests in the test suite</returns>
167: int TestSuiteInfo::CountTests()
168: {
169:     int numberOfTests = 0;
170:     TestFixtureInfo* testFixture = FirstTestFixture();
171:     while (testFixture != nullptr)
172:     {
173:         numberOfTests += testFixture->CountTests();
174:         testFixture = NextTestFixture(testFixture);
175:     }
176:     return numberOfTests;
177: }
178:
179: } // namespace unittest
```

- Line 55-65: We implement the constructor
- Line 67-81: We implement the destructor. This goes through the list of test fixtures, and deletes every one of these. Note that we will therefore need to create the test fixtures on the heap
- Line 83-99: We implement the `GetTestFixture()` method. This will try to find the test fixture with the specified name in the test suite. If it is found, the pointer is returned, if not, a new instance iscreated
- Line 101-118: We implement the `AddFixture()` method. This will add the test fixture passed in at the end of the list
- Line 120-127: We implement the `Name()` method.
This returns the test fixture name, unless it is empty, in which case we return the default fixture name
- Line 129-145: We implement the `Run()` method.
This goes through the list of test fixtures, and calls `Run()` on each.
Note that it also calls the methods `OnTestSuiteStart()` and `OnTestSuiteFinish()` on the `TestResults` instance
- Line 147-161: We implement the `CountFixtures()` method.
This goes through the list of test fixtures, and counts them
- Line 163-177: We implement the `CountTests()` method.
This goes through the list of test fixtures, and counts the tests in each of them

### Update application code {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES___STEP_5_UPDATE_APPLICATION_CODE}

So as a final step let's define test fixtures inside a test suite, and outside any test suite.
We'll add the test fixtures in the test suite, and leave the ones without a suite out of course.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Console.h"
2: #include "baremetal/Logger.h"
3: #include "baremetal/System.h"
4: #include "baremetal/Timer.h"
5:
6: #include "unittest/CurrentTest.h"
7: #include "unittest/Test.h"
8: #include "unittest/TestFixture.h"
9: #include "unittest/TestFixtureInfo.h"
10: #include "unittest/TestInfo.h"
11: #include "unittest/TestResults.h"
12: #include "unittest/TestSuite.h"
13: #include "unittest/TestSuiteInfo.h"
14:
15: LOG_MODULE("main");
16:
17: using namespace baremetal;
18: using namespace unittest;
19:
20: namespace Suite1 {
21:
22: inline char const* GetSuiteName()
23: {
24:     return "Suite1";
25: }
26:
27: class FixtureMyTest1
28:     : public TestFixture
29: {
30: public:
31:     void SetUp() override
32:     {
33:         LOG_DEBUG("MyTest SetUp");
34:     }
35:     void TearDown() override
36:     {
37:         LOG_DEBUG("MyTest TearDown");
38:     }
39: };
40:
41: class FixtureMyTest1Helper
42:     : public FixtureMyTest1
43: {
44: public:
45:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
46:     explicit FixtureMyTest1Helper(const TestDetails& details)
47:         : m_details{ details }
48:     {
49:         SetUp();
50:     }
51:     virtual ~FixtureMyTest1Helper()
52:     {
53:         TearDown();
54:     }
55:     void RunImpl() const;
56:     const TestDetails& m_details;
57: };
58: void FixtureMyTest1Helper::RunImpl() const
59: {
60:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 1");
61: }
62:
63: class MyTest1
64:     : public Test
65: {
66:     void RunImpl() const override
67:     {
68:         LOG_DEBUG("Test 1");
69:         FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
70:         fixtureHelper.RunImpl();
71:     }
72: } myTest1;
73: class MyTestInfo1
74:     : public TestInfo
75: {
76: public:
77:     MyTestInfo1(Test* testInstance)
78:         : TestInfo(testInstance, TestDetails("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__))
79:     {
80:     }
81: };
82:
83: } // namespace Suite1
84:
85: namespace Suite2 {
86:
87: inline char const* GetSuiteName()
88: {
89:     return "Suite2";
90: }
91:
92: class FixtureMyTest2
93:     : public TestFixture
94: {
95: public:
96:     void SetUp() override
97:     {
98:         LOG_DEBUG("FixtureMyTest2 SetUp");
99:     }
100:     void TearDown() override
101:     {
102:         LOG_DEBUG("FixtureMyTest2 TearDown");
103:     }
104: };
105:
106: class FixtureMyTest2Helper
107:     : public FixtureMyTest2
108: {
109: public:
110:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
111:     explicit FixtureMyTest2Helper(const TestDetails& details)
112:         : m_details{ details }
113:     {
114:         SetUp();
115:     }
116:     virtual ~FixtureMyTest2Helper()
117:     {
118:         TearDown();
119:     }
120:     void RunImpl() const;
121:     const TestDetails& m_details;
122: };
123: void FixtureMyTest2Helper::RunImpl() const
124: {
125:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 2");
126: }
127:
128: class MyTest2
129:     : public Test
130: {
131:     void RunImpl() const override
132:     {
133:         LOG_DEBUG("Test 2");
134:         FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
135:         fixtureHelper.RunImpl();
136:     }
137: } myTest2;
138: class MyTestInfo2
139:     : public TestInfo
140: {
141: public:
142:     MyTestInfo2(Test* testInstance)
143:         : TestInfo(testInstance, TestDetails("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__))
144:     {
145:     }
146: };
147:
148: } // namespace Suite2
149:
150: class FixtureMyTest3
151:     : public TestFixture
152: {
153: public:
154:     void SetUp() override
155:     {
156:         LOG_DEBUG("FixtureMyTest3 SetUp");
157:     }
158:     void TearDown() override
159:     {
160:         LOG_DEBUG("FixtureMyTest3 TearDown");
161:     }
162: };
163:
164: class FixtureMyTest3Helper
165:     : public FixtureMyTest3
166: {
167: public:
168:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
169:     explicit FixtureMyTest3Helper(const TestDetails& details)
170:         : m_details{ details }
171:     {
172:         SetUp();
173:     }
174:     virtual ~FixtureMyTest3Helper()
175:     {
176:         TearDown();
177:     }
178:     void RunImpl() const;
179:     const TestDetails& m_details;
180: };
181: void FixtureMyTest3Helper::RunImpl() const
182: {
183:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 3");
184: }
185:
186: class MyTest3
187:     : public Test
188: {
189:     void RunImpl() const override
190:     {
191:         LOG_DEBUG("Test 3");
192:         FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
193:         fixtureHelper.RunImpl();
194:     }
195: } myTest3;
196: class MyTestInfo3
197:     : public TestInfo
198: {
199: public:
200:     MyTestInfo3(Test* testInstance)
201:         : TestInfo(testInstance, TestDetails("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__))
202:     {
203:     }
204: };
205:
206:
207: class MyTest
208:     : public Test
209: {
210: public:
211:     void RunImpl() const override;
212: } myTest;
213:
214: void MyTest::RunImpl() const
215: {
216:     CurrentTest::Results()->OnTestRun(*CurrentTest::Details(), "Running test");
217: }
218:
219: int main()
220: {
221:     auto& console = GetConsole();
222:     GetLogger().SetLogLevel(LogSeverity::Debug);
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
253:     console.Write("Press r to reboot, h to halt\n");
254:     char ch{};
255:     while ((ch != 'r') && (ch != 'h'))
256:     {
257:         ch = console.ReadChar();
258:         console.WriteChar(ch);
259:     }
260:
261:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
262: }
```

- Line 12: We include the header to define the default `GetSuiteName()` function
- Line 13: We include the header for `TestSuiteInfo`
- Line 20: We define the namespace Suite1
- Line 22-25: We define the namespace specific version of `GetSuiteName()`
- Line 27-39: We declare and implement the class `FixtureMyTest1` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods, simply by logging a message
- Line 41-61: We declare and implement the class `FixtureMyTest1Helper` which derives from the class `FixtureMyTest1` just defined, and implements the constructor and destructor, calling resp. `SetUp()` and `TearDown()`.
This class also defines a `RunImpl()` method, which forms the actual test body for the test `MyTest1`.
It is implemented as a call to the `OnTestRun()` method on the `TestResults` instance
- Line 63-72: We declare and implement the class `MyTest1`, which derives from `Test`, and acts as the placeholders for the fixture test.
We instantiate it as `myTest1`.
- Its `RunImpl()` method instantiates `FixtureMyTest1Helper`, and runs its `RunImpl()` method
- Line 73-81: We declare and implement the class `MyTestInfo1`, which derives from `TestInfo`.
Its constructor links the test instance.
Notice that the constructor uses the `GetSuiteName()` function in `Suite1` to retrieve the correct test suite name
- Line 83: We end the namespace Suite1
- Line 85: We define the namespace Suite2
- Line 87-90: We define the namespace specific version of `GetSuiteName()`
- Line 92-104: We declare and implement the class `FixtureMyTest2` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods, simply by logging a message.
Notice that we now have to declare a new class, as we are in a different namespace
- Line 106-126: We declare and implement the class `FixtureTest2Helper` similar to `FixtureMyTest1Helper`
- Line 128-137: We declare and implement the class `MyTest2` similar to `MyTest1`, and instantiate it as `myTest2`
- Line 138-146: We declare and implement the class `MyTestInfo2` similar to `MyTestInfo1`.
Notice that the constructor uses the `GetSuiteName()` function in `Suite2` to retrieve the correct test suite name
- Line 148: We end the namespace Suite2
- Line 150-162: We declare and implement the class `FixtureMyTest3` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods, simply by logging a message.
Notice that this fixture is defined in the global namespace
- Line 164-184: We declare and implement the class `FixtureTest3Helper` similar to `FixtureMyTest1Helper`
- Line 186-195: We declare and implement the class `MyTest3` similar to `MyTest1`, and instantiate it as `myTest3`
- Line 196-204: We declare and implement the class `MyTestInfo3` similar to `MyTestInfo1`.
Notice that the constructor uses the default `GetSuiteName()` function to retrieve the correct test suite name, as we are in the global namespace
- Line 207-212: We declare the class `MyTest`, which derives from `Test`
- Line 214-217: We implement the `RunImpl()` method for `MyTest` as a call to the `OnTestRun()` method on the `TestResults` instance
- Line 224-226: We instantiate each of `Suite1::MyTestInfo1`, `Suite2::MyTestInfo2` and `MyTestInfo3`, passing in the instance of the corresponding test
- Line 227-228: We instantiate a `TestFixtureInfo` named `fixture1` as the first test fixture, and add `MyTest1`
- Line 229-230: We instantiate a `TestFixtureInfo` named `fixture2` as our second test fixture, and add `MyTest2`
- Line 231-232: We instantiate a `TestFixtureInfo` named `fixture3` as our third test fixture, and add `MyTest3`
- Line 234-235: We instantiate a `TestSuiteInfo` named `suite1` as our first test suite (with name `Suite1`), and add the test fixture `fixture1`
- Line 236-237: We instantiate a `TestSuiteInfo` named `suite2` as our second test suite (with name `Suite2`), and add the test fixture `fixture2`
- Line 238-239: We instantiate a `TestSuiteInfo` named `suite3` as our third test suite (with empty name), and add the test fixture `fixture3`
- Line 240-242: We run the test suites
- Line 243-245: We clean up the test suites. Note that the test suite desctructor deletes all test fixtures and as part of that all tests, so we don't need to (and shouldn't) do that
- Line 246: We instantiate a `TestInfo`, linked to the `myTest` instance
- Line 248: We run the test

We've create even more infrastructure to define all test suites, test fixture, tests, and hook them up.
Be patient, we'll create macros later to do this work for us.

### Configuring, building and debugging {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_ADDING_TEST_SUITES___STEP_5_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests in the test suite, and therefore show the log output.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
Info   0.00:00:00.020 MySuite1 Start suite (TestResults:65)
Info   0.00:00:00.020 MyFixture1 Start fixture (TestResults:83)
Info   0.00:00:00.030 Suite1::FixtureMyTest1::MyTest1 Start test (TestResults:101)
Debug  0.00:00:00.030 Test 1 (main:68)
Debug  0.00:00:00.030 MyTest SetUp (main:33)
Debug  0.00:00:00.040 Suite1::FixtureMyTest1::MyTest1 (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:78) --> MyTestHelper 1 (TestResults:112)
Debug  0.00:00:00.040 MyTest TearDown (main:37)
Info   0.00:00:00.050 Suite1::FixtureMyTest1::MyTest1 Finish test (TestResults:121)
Info   0.00:00:00.050 MyFixture1 Finish fixture (TestResults:92)
Info   0.00:00:00.050 MySuite1 Finish suite (TestResults:74)
Info   0.00:00:00.060 MySuite2 Start suite (TestResults:65)
Info   0.00:00:00.060 MyFixture2 Start fixture (TestResults:83)
Info   0.00:00:00.060 Suite2::FixtureMyTest2::MyTest2 Start test (TestResults:101)
Debug  0.00:00:00.060 Test 2 (main:133)
Debug  0.00:00:00.070 FixtureMyTest2 SetUp (main:98)
Debug  0.00:00:00.070 Suite2::FixtureMyTest2::MyTest2 (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:143) --> MyTestHelper 2 (TestResults:112)
Debug  0.00:00:00.080 FixtureMyTest2 TearDown (main:102)
Info   0.00:00:00.080 Suite2::FixtureMyTest2::MyTest2 Finish test (TestResults:121)
Info   0.00:00:00.090 MyFixture2 Finish fixture (TestResults:92)
Info   0.00:00:00.090 MySuite2 Finish suite (TestResults:74)
Info   0.00:00:00.090 DefaultSuite Start suite (TestResults:65)
Info   0.00:00:00.100 MyFixture3 Start fixture (TestResults:83)
Info   0.00:00:00.100 DefaultSuite::FixtureMyTest3::MyTest3 Start test (TestResults:101)
Debug  0.00:00:00.110 Test 3 (main:191)
Debug  0.00:00:00.110 FixtureMyTest3 SetUp (main:156)
Debug  0.00:00:00.110 DefaultSuite::FixtureMyTest3::MyTest3 (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:201) --> MyTestHelper 3 (TestResults:112)
Debug  0.00:00:00.120 FixtureMyTest3 TearDown (main:160)
Info   0.00:00:00.130 DefaultSuite::FixtureMyTest3::MyTest3 Finish test (TestResults:121)
Info   0.00:00:00.130 MyFixture3 Finish fixture (TestResults:92)
Info   0.00:00:00.140 DefaultSuite Finish suite (TestResults:74)
Info   0.00:00:00.140 DefaultSuite::DefaultFixture::MyTest Start test (TestResults:101)
Debug  0.00:00:00.150 DefaultSuite::DefaultFixture::MyTest (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:246) --> Running test (TestResults:112)
Info   0.00:00:00.150 DefaultSuite::DefaultFixture::MyTest Finish test (TestResults:121)
Info   0.00:00:00.160 Wait 5 seconds (main:250)
Press r to reboot, h to halt
```

## Test registration - Step 6 {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION___STEP_6}

Now that we have all the classes defined to run tests, we still need to find a way to register them, such that we can ask a test runner to run all (or part of) the tests.
For this we will define a class `TestRegistry`, which will keep administration at the top level (the list of all test suites).
We will also define a class `TestRegistrar` which is used to register tests statically, so that before the application runs, the tests have already been registered.

### TestRegistry.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION___STEP_6_TESTREGISTRYH}

So let's declare the `TestRegistry` and `TestRegistrar` classes.

Create the file `code/libraries/unittest/include/unittest/TestRegistry.h`

```cpp
File: code/libraries/unittest/include/unittest/TestRegistry.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "unittest/TestSuiteInfo.h"
43: 
44: /// @file
45: /// Test registry
46: 
47: namespace unittest {
48: 
49: class Test;
50: class TestSuiteInfo;
51: class TestResults;
52: 
53: /// <summary>
54: /// Test registry
55: /// </summary>
56: class TestRegistry
57: {
58: private:
59:     friend class TestRegistrar;
60:     /// @brief Pointer to first test suite in the list
61:     TestSuiteInfo* m_head;
62:     /// @brief Pointer to last test suite in the list
63:     TestSuiteInfo* m_tail;
64: 
65: public:
66:     TestRegistry();
67:     TestRegistry(const TestRegistry&) = delete;
68:     TestRegistry(TestRegistry&&) = delete;
69:     virtual ~TestRegistry();
70: 
71:     TestRegistry& operator=(const TestRegistry&) = delete;
72:     TestRegistry& operator=(TestRegistry&&) = delete;
73: 
74:     /// <summary>
75:     /// Returns a pointer to the first test suite in the list
76:     /// </summary>
77:     /// <returns>Pointer to the first test suite in the list</returns>
78:     TestSuiteInfo* FirstTestSuite() const
79:     {
80:         return m_head;
81:     }
82: 
83:     /// <summary>
84:     /// Returns the pointer to the next test suite in the list from the current pointer
85:     /// </summary>
86:     /// <param name="current">Pointer to current test suite in the list</param>
87:     /// <returns>Pointer to the next test suite in the list</returns>
88:     TestSuiteInfo* NextTestSuite(TestSuiteInfo* current) const
89:     {
90:         return current->Next();
91:     }
92: 
93:     void Run(TestResults& testResults);
94:     int CountSuites();
95:     int CountFixtures();
96:     int CountTests();
97: 
98:     static TestRegistry& GetTestRegistry();
99: 
100: private:
101:     TestSuiteInfo* GetTestSuite(const baremetal::String& suiteName);
102:     void AddSuite(TestSuiteInfo* testSuite);
103: };
104: 
105: /// <summary>
106: /// Test registrar
107: ///
108: /// This is a utility class to register a test to the registry, as part of a test declaration
109: /// </summary>
110: class TestRegistrar
111: {
112: public:
113:     TestRegistrar(TestRegistry& registry, Test* testInstance, const TestDetails& details);
114: };
115: 
116: } // namespace unittest
```

- Line 53-108: We declare the `Testregistry` class
  - Line 60-61: The member variable `m_head` stores the pointer to the first test suite
  - Line 62-63: The member variable `m_tail` stores the pointer to the last test suite
  - Line 66: We declare the default constructor
  - Line 67-68: We remove the copy and move constructors
  - Line 69: We declare the destructor
  - Line 71-72: We remove the assignment and move assignment operators
  - Line 74-81: The method `FirstTestSuite()` returns the pointer to the first test suite in the list
  - Line 83-91: The method `NextTestSuite()` returns the pointer to the next test suite in the list starting from the current pointer
  - Line 93: The method `Run()` runs all test suites. We'll be revisiting this later
  - Line 94: The method `CountSuites()` counts and returns the number of test suites in the test suite
  - Line 95: The method `CountFixtures()` counts and returns the number of test fixtures in all test suites
  - Line 96: The method `CountTests()` counts and returns the number of tests in all test fixtures in all test suites
  - Line 98: The static method `GetTestRegistry()` returns the singleton test registry
  - Line 101: The method `GetTestSuite()` finds and returns a test suite in the list, or if not found, creates a new test suite with the specified name.
  Note that this method is private, as the `TestRegistrar` can access it as a friend class
  - Line 102: The method `AddSuite()` adds a test suite to the list.
  Note that this method is private, as the `TestRegistrar` can access it as a friend class

- Line 105-114: We declare the `TestRegistrar` class
  - Line 103: We declare the only method in the class, the constructor

### TestRegistry.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION___STEP_6_TESTREGISTRYCPP}

Let's implement the `TestRegistry` and `TestRegistrar` class.

Create the file `code/libraries/unittest/src/TestRegistry.cpp`

```cpp
File: code/libraries/unittest/src/TestRegistry.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/TestRegistry.h"
41:
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
44: #include "unittest/TestInfo.h"
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
82:     TestSuiteInfo* testSuite = FirstTestSuite();
83:     while (testSuite != nullptr)
84:     {
85:         const TestSuiteInfo* currentSuite = testSuite;
86:         testSuite = NextTestSuite(testSuite);
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
97:     TestSuiteInfo* testSuite = FirstTestSuite();
98:
99:     while (testSuite != nullptr)
100:     {
101:         testSuite->Run(testResults);
102:         testSuite = NextTestSuite(testSuite);
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
115:     TestSuiteInfo* testSuite = FirstTestSuite();
116:     while (testSuite != nullptr)
117:     {
118:         ++numberOfTestSuites;
119:         testSuite = NextTestSuite(testSuite);
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
131:     TestSuiteInfo* testSuite = FirstTestSuite();
132:     while (testSuite != nullptr)
133:     {
134:         numberOfTestFixtures += testSuite->CountFixtures();
135:         testSuite = NextTestSuite(testSuite);
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
147:     TestSuiteInfo* testSuite = FirstTestSuite();
148:     while (testSuite != nullptr)
149:     {
150:         numberOfTests += testSuite->CountTests();
151:         testSuite = NextTestSuite(testSuite);
152:     }
153:     return numberOfTests;
154: }
155:
156: /// <summary>
157: /// Find a test suite with specified name, register a new one if not found
158: /// </summary>
159: /// <param name="suiteName">Test suite name to search for</param>
160: /// <returns>Found or created test suite</returns>
161: TestSuiteInfo* TestRegistry::GetTestSuite(const String& suiteName)
162: {
163:     TestSuiteInfo* testSuite = FirstTestSuite();
164:     while ((testSuite != nullptr) && (testSuite->Name() != suiteName))
165:         testSuite = NextTestSuite(testSuite);
166:     if (testSuite == nullptr)
167:     {
168:         TRACE_DEBUG("Find suite %s ... not found, creating new object", suiteName.c_str());
169:         testSuite = new TestSuiteInfo(suiteName);
170:         AddSuite(testSuite);
171:     }
172:     else
173:     {
174:         TRACE_DEBUG("Find suite %s ... found", suiteName.c_str());
175:     }
176:     return testSuite;
177: }
178:
179: /// <summary>
180: /// Add a test suite
181: /// </summary>
182: /// <param name="testSuite">Test suite to add</param>
183: void TestRegistry::AddSuite(TestSuiteInfo* testSuite)
184: {
185:     if (m_tail == nullptr)
186:     {
187:         assert(m_head == nullptr);
188:         m_head = testSuite;
189:         m_tail = testSuite;
190:     }
191:     else
192:     {
193:         m_tail->Next() = testSuite;
194:         m_tail = testSuite;
195:     }
196: }
197:
198: /// <summary>
199: /// Constructor
200: ///
201: /// Finds or registers the test suite specified in the test details of the test.
202: /// Finds or registers the test fixture specified in the test details of the test.
203: /// Adds the test to the fixture found or created.
204: /// </summary>
205: /// <param name="registry">Test registry</param>
206: /// <param name="testInstance">Test instance to link to</param>
207: /// <param name="details">Test details</param>
208: TestRegistrar::TestRegistrar(TestRegistry& registry, Test* testInstance, const TestDetails& details)
209: {
210:     TRACE_DEBUG("Register test %s in fixture %s in suite %s", details.TestName().c_str(), (details.FixtureName().c_str()), (details.SuiteName().c_str()));
211:     TestSuiteInfo* testSuite = registry.GetTestSuite(details.SuiteName());
212:     TestFixtureInfo* testFixture = testSuite->GetTestFixture(details.FixtureName());
213:     TestInfo* test = new TestInfo(testInstance, details);
214:     testFixture->AddTest(test);
215: }
216:
217: } // namespace unittest
```

- Line 56-64: We implement the static method `GetTestRegistry()`.
This returns the singleton instance of the test registry
- Line 66-73: We implement the constructor
- Line 75-89: We implement the destructor.
This goes through the list of test suites, and deletes every one of these.
Note that we will therefore need to create the test suites on the heap.
- Line 91-104: We implement the `Run()` method.
This goes through the list of test suites, and calls `Run()` on each
- Line 106-122: We implement the `CountSuites()` method.
This goes through the list of test suites, and counts them
- Line 124-138: We implement the `CountFixtures()` method.
This goes through the list of test suites, and counts the test fixtures in each of them
- Line 140-154: We implement the `CountTests()` method.
This goes through the list of test suites, and counts the tests in each of them
- Line 156-177: We implement the `GetTestSuite()` method.
This will try to find the test suite with the specified name. If it is found, the pointer is returned, if not, a new instance is created.
Notice that we add tracing in this method. We'll do the same for the test suite in a minute
- Line 179-196: We implement the `AddSuite()` method. This will insert the test suite passed in at the end of the list
- Line 198-215: We implement `TestRegistrar` contructor. This is intended for the macros we will get to next.
The constructor receives a reference to the test registry, as well as a pointer to a test to link to (derived from `Test`) and test details, and adds the test to the registry.
The `TestDetails` are used to create a test administraton instance, and lookup and if needed create the surrounding test fixture and test suite

### TestSuiteInfo.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION___STEP_6_TESTSUITEINFOCPP}

Now that we have a `TestRegistry`, we can update the method `GetTestFixture` in `TestSuiteInfo` to add some debug info.

Update the file `code/libraries/unittest/src/TestSuiteInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestSuiteInfo.cpp
...
83: /// <summary>
84: /// Find a test fixture with specified name, register a new one if not found
85: /// </summary>
86: /// <param name="fixtureName">Test fixture name to search for</param>
87: /// <returns>Found or created test fixture</returns>
88: TestFixtureInfo* TestSuiteInfo::GetTestFixture(const String& fixtureName)
89: {
90:     TestFixtureInfo* testFixture = FirstTestFixture();
91:     while ((testFixture != nullptr) && (testFixture->Name() != fixtureName))
92:         testFixture = NextTestFixture(testFixture);
93:     if (testFixture == nullptr)
94:     {
95:         TRACE_DEBUG("Fixture %s not found, creating new object", fixtureName.c_str());
96:         testFixture = new TestFixtureInfo(fixtureName);
97:         AddFixture(testFixture);
98:     }
99:     else
100:     {
101:         TRACE_DEBUG("Fixture %s found", fixtureName.c_str());
102:     }
103:     return testFixture;
104: }
...
```

### Update application code {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION___STEP_6_UPDATE_APPLICATION_CODE}

Now we can use the TestRegistry. Even though we still need to create quite some classes, the plumbing of registering the classes is now taken care of by the `TestRegistry` class, and instances of the `TestRegistrar` class.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/Console.h"
2: #include "baremetal/Logger.h"
3: #include "baremetal/System.h"
4: #include "baremetal/Timer.h"
5:
6: #include "unittest/CurrentTest.h"
7: #include "unittest/Test.h"
8: #include "unittest/TestFixture.h"
9: #include "unittest/TestFixtureInfo.h"
10: #include "unittest/TestInfo.h"
11: #include "unittest/TestRegistry.h"
12: #include "unittest/TestResults.h"
13: #include "unittest/TestSuite.h"
14: #include "unittest/TestSuiteInfo.h"
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
25:     return "Suite1";
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
67:     void RunImpl() const override;
68: } myTest1;
69:
70: TestRegistrar registrarFixtureMyTest1(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__));
71:
72: void MyTest1::RunImpl() const
73: {
74:     LOG_DEBUG("Test 1");
75:     FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
76:     fixtureHelper.RunImpl();
77: }
78:
79: } // namespace Suite1
80:
81: namespace Suite2 {
82:
83: inline char const* GetSuiteName()
84: {
85:     return "Suite2";
86: }
87:
88: class FixtureMyTest2
89:     : public TestFixture
90: {
91: public:
92:     void SetUp() override
93:     {
94:         LOG_DEBUG("FixtureMyTest2 SetUp");
95:     }
96:     void TearDown() override
97:     {
98:         LOG_DEBUG("FixtureMyTest2 TearDown");
99:     }
100: };
101:
102: class FixtureMyTest2Helper
103:     : public FixtureMyTest2
104: {
105: public:
106:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
107:     explicit FixtureMyTest2Helper(const TestDetails& details)
108:         : m_details{ details }
109:     {
110:         SetUp();
111:     }
112:     virtual ~FixtureMyTest2Helper()
113:     {
114:         TearDown();
115:     }
116:     void RunImpl() const;
117:     const TestDetails& m_details;
118: };
119: void FixtureMyTest2Helper::RunImpl() const
120: {
121:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 2");
122: }
123:
124: class MyTest2
125:     : public Test
126: {
127:     void RunImpl() const override;
128: } myTest1;
129:
130: TestRegistrar registrarFixtureMyTest2(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__));
131:
132: void MyTest2::RunImpl() const
133: {
134:     LOG_DEBUG("Test 2");
135:     FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
136:     fixtureHelper.RunImpl();
137: }
138:
139: } // namespace Suite2
140:
141: class FixtureMyTest3
142:     : public TestFixture
143: {
144: public:
145:     void SetUp() override
146:     {
147:         LOG_DEBUG("FixtureMyTest3 SetUp");
148:     }
149:     void TearDown() override
150:     {
151:         LOG_DEBUG("FixtureMyTest3 TearDown");
152:     }
153: };
154:
155: class FixtureMyTest3Helper
156:     : public FixtureMyTest3
157: {
158: public:
159:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
160:     explicit FixtureMyTest3Helper(const TestDetails& details)
161:         : m_details{ details }
162:     {
163:         SetUp();
164:     }
165:     virtual ~FixtureMyTest3Helper()
166:     {
167:         TearDown();
168:     }
169:     void RunImpl() const;
170:     const TestDetails& m_details;
171: };
172: void FixtureMyTest3Helper::RunImpl() const
173: {
174:     CurrentTest::Results()->OnTestRun(m_details, "MyTestHelper 3");
175: }
176:
177: class MyTest3
178:     : public Test
179: {
180:     void RunImpl() const override;
181: } myTest3;
182:
183: TestRegistrar registrarFixtureMyTest3(TestRegistry::GetTestRegistry(), &myTest3, TestDetails("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__));
184:
185: void MyTest3::RunImpl() const
186: {
187:     LOG_DEBUG("Test 3");
188:     FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
189:     fixtureHelper.RunImpl();
190: }
191:
192: class MyTest
193:     : public Test
194: {
195: public:
196:     void RunImpl() const override;
197: } myTest;
198:
199: TestRegistrar registrarMyTest(TestRegistry::GetTestRegistry(), &myTest, TestDetails("MyTest", "", "", __FILE__, __LINE__));
200:
201: void MyTest::RunImpl() const
202: {
203:     CurrentTest::Results()->OnTestRun(*CurrentTest::Details(), "Running test");
204: }
205:
206: int main()
207: {
208:     auto& console = GetConsole();
209:     GetLogger().SetLogLevel(LogSeverity::Debug);
210:
211:     unittest::TestResults results;
212:     TestRegistry::GetTestRegistry().Run(results);
213:
214:     LOG_INFO("Wait 5 seconds");
215:     Timer::WaitMilliSeconds(5000);
216:
217:     console.Write("Press r to reboot, h to halt\n");
218:     char ch{};
219:     while ((ch != 'r') && (ch != 'h'))
220:     {
221:         ch = console.ReadChar();
222:         console.WriteChar(ch);
223:     }
224:
225:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
226: }
```

- Line 11: We include the header for `TestRegistry` and `TestRegistrar`
- Line 64-68: We declare the class `MyTest1` and create an instance as before.
The `RunImpl()` method is now defined later
- Line 70: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest1` instance just defined, as well as constructed test details
- Line 72-77: We implement the `RunImpl()` method of `MyTest1` as before
- Line 124-128: We declare the class `MyTest2` and create an instance as before.
The `RunImpl()` method is now defined later
- Line 130: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest2` instance just defined
- Line 132-137: We implement the `RunImpl()` method of `MyTest2` as before
- Line 177-181: We declare the class `MyTest3` and create an instance as before.
The `RunImpl()` method is now defined later
- Line 183: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest3` instance just defined
- Line 185-190: We implement the `RunImpl()` method of `MyTest3` as before
- Line 192-197: We declare the class `MyTest` and create an instance as before.
The `RunImpl()` method is now defined later
- Line 199: We create an instance of `TestRegistrar`, which gets the singleton `TestRegistry` instance and a pointer to the `MyTest` instance just defined
- Line 201-204: We implement the `RunImpl()` method of `MyTest` as before
- Line 212: We retrieve the test registry, and call its `Run()` method

Note that we don't need to clean up anymore, that is all taken care of.

### Configuring, building and debugging {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_REGISTRATION___STEP_6_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will, at static initialization time, register all the tests, and then run all tests registered.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.000 Starting up (System:213)
Info   0.00:00:00.010 Suite1 Start suite (TestResults:65)
Info   0.00:00:00.020 FixtureMyTest1 Start fixture (TestResults:83)
Info   0.00:00:00.030 Suite1::FixtureMyTest1::MyTest1 Start test (TestResults:101)
Debug  0.00:00:00.030 Test 1 (main:74)
Debug  0.00:00:00.030 MyTest SetUp (main:34)
Debug  0.00:00:00.040 Suite1::FixtureMyTest1::MyTest1 (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:70) --> MyTestHelper 1 (TestResults:112)
Debug  0.00:00:00.040 MyTest TearDown (main:38)
Info   0.00:00:00.050 Suite1::FixtureMyTest1::MyTest1 Finish test (TestResults:121)
Info   0.00:00:00.050 FixtureMyTest1 Finish fixture (TestResults:92)
Info   0.00:00:00.060 Suite1 Finish suite (TestResults:74)
Info   0.00:00:00.060 Suite2 Start suite (TestResults:65)
Info   0.00:00:00.060 FixtureMyTest2 Start fixture (TestResults:83)
Info   0.00:00:00.060 Suite2::FixtureMyTest2::MyTest2 Start test (TestResults:101)
Debug  0.00:00:00.070 Test 2 (main:134)
Debug  0.00:00:00.070 FixtureMyTest2 SetUp (main:94)
Debug  0.00:00:00.080 Suite2::FixtureMyTest2::MyTest2 (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:130) --> MyTestHelper 2 (TestResults:112)
Debug  0.00:00:00.080 FixtureMyTest2 TearDown (main:98)
Info   0.00:00:00.090 Suite2::FixtureMyTest2::MyTest2 Finish test (TestResults:121)
Info   0.00:00:00.090 FixtureMyTest2 Finish fixture (TestResults:92)
Info   0.00:00:00.100 Suite2 Finish suite (TestResults:74)
Info   0.00:00:00.100 DefaultSuite Start suite (TestResults:65)
Info   0.00:00:00.100 FixtureMyTest3 Start fixture (TestResults:83)
Info   0.00:00:00.110 DefaultSuite::FixtureMyTest3::MyTest3 Start test (TestResults:101)
Debug  0.00:00:00.120 Test 3 (main:187)
Debug  0.00:00:00.130 FixtureMyTest3 SetUp (main:147)
Debug  0.00:00:00.130 DefaultSuite::FixtureMyTest3::MyTest3 (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:183) --> MyTestHelper 3 (TestResults:112)
Debug  0.00:00:00.140 FixtureMyTest3 TearDown (main:151)
Info   0.00:00:00.150 DefaultSuite::FixtureMyTest3::MyTest3 Finish test (TestResults:121)
Info   0.00:00:00.150 FixtureMyTest3 Finish fixture (TestResults:92)
Info   0.00:00:00.150 DefaultFixture Start fixture (TestResults:83)
Info   0.00:00:00.160 DefaultSuite::DefaultFixture::MyTest Start test (TestResults:101)
Debug  0.00:00:00.160 DefaultSuite::DefaultFixture::MyTest (D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:199) --> Running test (TestResults:112)
Info   0.00:00:00.170 DefaultSuite::DefaultFixture::MyTest Finish test (TestResults:121)
Info   0.00:00:00.170 DefaultFixture Finish fixture (TestResults:92)
Info   0.00:00:00.170 DefaultSuite Finish suite (TestResults:74)
Info   0.00:00:00.180 Wait 5 seconds (main:214)
Press r to reboot, h to halt
```

## Test runner and visitor - Step 7 {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7}

### ITestReporter.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_ITESTREPORTERH}

We'll add a visitor interface, which we'll implement later on as a test reporter, which will print the progress and results of the tests.

Create the file `code/libraries/unittest/include/unittest/ITestReporter.h`

```cpp
File: code/libraries/unittest/include/unittest/ITestReporter.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "baremetal/String.h"
43: 
44: /// @file
45: /// Abstract test reporter interface
46: 
47: namespace unittest {
48: 
49: class TestDetails;
50: class TestResults;
51: 
52: /// <summary>
53: /// Test reporter abstract interface
54: /// </summary>
55: class ITestReporter
56: {
57: public:
58:     /// <summary>
59:     /// Destructor
60:     /// </summary>
61:     virtual ~ITestReporter()
62:     {
63:     }
64: 
65:     /// <summary>
66:     /// Start of test run callback
67:     /// </summary>
68:     /// <param name="numberOfTestSuites">Number of test suites to be run</param>
69:     /// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
70:     /// <param name="numberOfTests">Number of tests to be run</param>
71:     virtual void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
72:     /// <summary>
73:     /// Finish of test run callback
74:     /// </summary>
75:     /// <param name="numberOfTestSuites">Number of test suites run</param>
76:     /// <param name="numberOfTestFixtures">Number of test fixtures run</param>
77:     /// <param name="numberOfTests">Number of tests run</param>
78:     virtual void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) = 0;
79:     /// <summary>
80:     /// Test summary callback
81:     /// </summary>
82:     /// <param name="results">Test run results</param>
83:     virtual void ReportTestRunSummary(const TestResults& results) = 0;
84:     /// <summary>
85:     /// Test run overview callback
86:     /// </summary>
87:     /// <param name="results">Test run results</param>
88:     virtual void ReportTestRunOverview(const TestResults& results) = 0;
89: /// <summary>
90:     /// Test suite start callback
91:     /// </summary>
92:     /// <param name="suiteName">Test suite name</param>
93:     /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
94:     virtual void ReportTestSuiteStart(const baremetal::String& suiteName, int numberOfTestFixtures) = 0;
95:     /// <summary>
96:     /// Test suite finish callback
97:     /// </summary>
98:     /// <param name="suiteName">Test suite name</param>
99:     /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
100:     virtual void ReportTestSuiteFinish(const baremetal::String& suiteName, int numberOfTestFixtures) = 0;
101:     /// <summary>
102:     /// Test fixture start callback
103:     /// </summary>
104:     /// <param name="fixtureName">Test fixture name</param>
105:     /// <param name="numberOfTests">Number of tests within test fixture</param>
106:     virtual void ReportTestFixtureStart(const baremetal::String& fixtureName, int numberOfTests) = 0;
107:     /// <summary>
108:     /// Test fixture finish callback
109:     /// </summary>
110:     /// <param name="fixtureName">Test fixture name</param>
111:     /// <param name="numberOfTests">Number of tests within test fixture</param>
112:     virtual void ReportTestFixtureFinish(const baremetal::String& fixtureName, int numberOfTests) = 0;
113:     /// <summary>
114:     /// Test start callback
115:     /// </summary>
116:     /// <param name="details">Test details</param>
117:     virtual void ReportTestStart(const TestDetails& details) = 0;
118:     /// <summary>
119:     /// Test finish callback
120:     /// </summary>
121:     /// <param name="details">Test details</param>
122:     /// <param name="success">Test result, true is successful, false is failed</param>
123:     virtual void ReportTestFinish(const TestDetails& details, bool success) = 0;
124:     /// <summary>
125:     /// Test failure callback
126:     /// </summary>
127:     /// <param name="details">Test details</param>
128:     /// <param name="failure">Test failure message</param>
129:     virtual void ReportTestFailure(const TestDetails& details, const baremetal::String& failure) = 0;
130: };
131: 
132: } // namespace unittest
```

- Line 52-130: We declare the abstract interface class ITestReporter
  - Line 65-71: The method `ReportTestRunStart()` will be called before any tests are run, to give an overview of the number of test suites, test fixtures and tests
  - Line 72-78: The method `ReportTestRunFinish()` will be called just after the tests are run, to again give an overview of the number of test suites, test fixtures and tests
  - Line 79-83: The method `ReportTestSummary()` will be called after all tests are finished, to show a summary of the number of tests run, and the number of failing tests
  - Line 84-88: The method `ReportTestRunOverview()` will print the list of failed tests, with information of why they failed
  - Line 89-94: The method `ReportTestSuiteStart()` will print a marker for the start of a test suite run
  - Line 95-100: The method `ReportTestSuiteFinish()` will print a marker for the end of a test suite run
  - Line 101-106: The method `ReportTestFixtureStart()` will print a marker for the start of a test fixture run
  - Line 107-112: The method `ReportTestFixtureFinish()` will print a marker for the end of a test fixture run
  - Line 113-117: The method `ReportTestStart()` will print a marker for the start of a test run
  - Line 118-124: The method `ReportTestFinish()` will print a marker for the end of a test run
  - Line 125-129: The method `ReportTestFailure()` will print a statement that a failure was found, with information

### TestResults.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTRESULTSH}

We introduced the `TestResults` class before, we will now extend it to keep hold of the test reporter, and to count tests, test fixtures and test suites.

Update the file `code/libraries/unittest/include/unittest/TestResults.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResults.h
40: #pragma once
41: 
42: #include "baremetal/String.h"
43: 
44: /// @file
45: /// Test results
46: ///
47: /// Results for a complete test run
48: 
49: namespace unittest {
50: 
51: class ITestReporter;
52: class TestSuiteInfo;
53: class TestFixtureInfo;
54: class TestDetails;
55: 
56: /// <summary>
57: /// Test results
58: ///
59: /// Holds the test results for a complete test run
60: /// </summary>
61: class TestResults
62: {
63: private:
64:     /// @brief Selected test reporter
65:     ITestReporter* m_reporter;
66:     /// @brief Total count fo tests in test run
67:     int m_totalTestCount;
68:     /// @brief Total count of failed tests in test run
69:     int m_failedTestCount;
70:     /// @brief Total count of failures in test run
71:     int m_failureCount;
72:     /// @brief Flag for failure in current test, set to true if at least one failure occurred in the current test
73:     bool m_currentTestFailed;
74: 
75: public:
76:     explicit TestResults(ITestReporter* reporter = nullptr);
77:     TestResults(const TestResults&) = delete;
78:     TestResults(TestResults&&) = delete;
79:     virtual ~TestResults() = default;
80: 
81:     TestResults& operator=(const TestResults&) = delete;
82:     TestResults& operator=(TestResults&&) = delete;
83: 
84:     void OnTestSuiteStart(TestSuiteInfo* suite);
85:     void OnTestSuiteFinish(TestSuiteInfo* suite);
86:     void OnTestFixtureStart(TestFixtureInfo* fixture);
87:     void OnTestFixtureFinish(TestFixtureInfo* fixture);
88:     void OnTestStart(const TestDetails& details);
89:     void OnTestRun(const TestDetails& details, const baremetal::String& message);
90:     void OnTestFinish(const TestDetails& details);
91: 
92:     /// <summary>
93:     /// Returns the number of tests in the test run
94:     /// </summary>
95:     /// <returns>Number of tests in the test run</returns>
96:     int GetTotalTestCount() const
97:     {
98:         return m_totalTestCount;
99:     }
100:     /// <summary>
101:     /// Returns the number of failed tests in the test run
102:     /// </summary>
103:     /// <returns>Number of failed tests in the test run</returns>
104:     int GetFailedTestCount() const
105:     {
106:         return m_failedTestCount;
107:     }
108:     /// <summary>
109:     /// Returns the number of failures in the test run
110:     /// </summary>
111:     /// <returns>Number of failures in the test run</returns>
112:     int GetFailureCount() const
113:     {
114:         return m_failureCount;
115:     }
116: };
117: 
118: } // namespace unittest
```

- Line 64-65: The member `m_reporter` holds the pointer to the test reporter interface being used
- Line 66-67: The member `m_failedTestCount` holds the total failed test count
- Line 68-69: The member `m_failureCount` holds the total failure count. A test can have multiple failures
- Line 70-71: The member `m_currentTestFailed` is true if any test case for the current test failed
- Line 76: We change the constructor to take a pointer to a test reporter interface
- Line 90: The method `OnTestFailure()` replaces `OnTestRun()`, and marks the failure in a test run.
This will also call `ReportTestFailure()` on the test reporter. This is the only method from the previous declaration that changed
- Line 92-99: The method `GetTotalTestCount()` returns the total number of tests run
- Line 100-107: The method `GetFailedTestCount()` returns the total number of tests that failed
- Line 108-115: The method `GetFailureCount()` returns the total number of failures found. A test can have more than one failure

### TestResults.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTRESULTSCPP}

Let's implement additional and changed methods of the `TestResults` class.

Update the file `code/libraries/unittest/src/TestResults.cpp`

```cpp
File: code/libraries/unittest/src/TestResults.cpp
File: d:\Projects\RaspberryPi\baremetal.github\code\libraries\unittest\src\TestResults.cpp
40: #include "unittest/TestResults.h"
41: 
42: #include "unittest/ITestReporter.h"
43: #include "unittest/TestDetails.h"
44: #include "unittest/TestFixtureInfo.h"
45: #include "unittest/TestRegistry.h"
46: #include "unittest/TestSuiteInfo.h"
47: 
48: /// @file
49: /// Test results implementation
50: 
51: using namespace baremetal;
52: 
53: namespace unittest {
54: 
55: /// <summary>
56: /// Constructor
57: /// </summary>
58: /// <param name="testReporter">Test reporter to use</param>
59: TestResults::TestResults(ITestReporter* testReporter)
60:     : m_reporter{testReporter}
61:     , m_totalTestCount{}
62:     , m_failedTestCount{}
63:     , m_failureCount{}
64:     , m_currentTestFailed{}
65: {
66: }
67: 
68: /// <summary>
69: /// Start a test suite run
70: /// </summary>
71: /// <param name="suite">Test suite to start</param>
72: void TestResults::OnTestSuiteStart(TestSuiteInfo* suite)
73: {
74:     if (m_reporter)
75:         m_reporter->ReportTestSuiteStart(suite->Name(), suite->CountFixtures());
76: }
77: 
78: /// <summary>
79: /// Finish a test suite run
80: /// </summary>
81: /// <param name="suite">Test suite to finish</param>
82: void TestResults::OnTestSuiteFinish(TestSuiteInfo* suite)
83: {
84:     if (m_reporter)
85:         m_reporter->ReportTestSuiteFinish(suite->Name(), suite->CountFixtures());
86: }
87: 
88: /// <summary>
89: /// Start a test fixture run
90: /// </summary>
91: /// <param name="fixture">Test fixture to start</param>
92: void TestResults::OnTestFixtureStart(TestFixtureInfo* fixture)
93: {
94:     if (m_reporter)
95:         m_reporter->ReportTestFixtureStart(fixture->Name(), fixture->CountTests());
96: }
97: 
98: /// <summary>
99: /// Finish a test fixture run
100: /// </summary>
101: /// <param name="fixture">Test fixture to finish</param>
102: void TestResults::OnTestFixtureFinish(TestFixtureInfo* fixture)
103: {
104:     if (m_reporter)
105:         m_reporter->ReportTestFixtureFinish(fixture->Name(), fixture->CountTests());
106: }
107: 
108: /// <summary>
109: /// Start a test
110: /// </summary>
111: /// <param name="details">Test details of test to start</param>
112: void TestResults::OnTestStart(const TestDetails& details)
113: {
114:     ++m_totalTestCount;
115:     m_currentTestFailed = false;
116:     if (m_reporter)
117:         m_reporter->ReportTestStart(details);
118: }
119: 
120: /// <summary>
121: /// Add a test failure
122: /// </summary>
123: /// <param name="details"></param>
124: /// <param name="message">Test failure string</param>
125: void TestResults::OnTestFailure(const TestDetails& details, const String& message)
126: {
127:     ++m_failureCount;
128:     if (!m_currentTestFailed)
129:     {
130:         ++m_failedTestCount;
131:         m_currentTestFailed = true;
132:     }
133:     if (m_reporter)
134:         m_reporter->ReportTestFailure(details, message);
135: }
136: 
137: /// <summary>
138: /// Finish a test
139: /// </summary>
140: /// <param name="details">Test details of test to finish</param>
141: void TestResults::OnTestFinish(const TestDetails& details)
142: {
143:     if (m_reporter)
144:         m_reporter->ReportTestFinish(details, !m_currentTestFailed);
145: }
146: 
147: } // namespace unittest
```

Most methods are quite straightforward.

- Line 108-118: The `OnTestStart()` method also counts the tests run, and resets the flag whether the test failed
- Line 120-135: The `OnTestFailure()` method increments the failure count, and if this is the first failure in the test, also increments the failed test count

\todo

### TestRunner.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTRUNNERH}

Next we define a class `TestRunner` that can run the registered tests.
This will introduce a method for filtering which tests to run, based on templates.

Create the file `code/libraries/unittest/include/unittest/TestRunner.h`

```cpp
File: code/libraries/unittest/include/unittest/TestRunner.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "unittest/ITestReporter.h"
43: #include "unittest/TestRegistry.h"
44: #include "unittest/TestResults.h"
45: 
46: /// @file
47: /// Test runner
48: 
49: namespace unittest {
50: 
51: class ITestReporter;
52: class TestInfo;
53: class TestFixtureInfo;
54: class TestResults;
55: class TestSuiteInfo;
56: 
57: /// <summary>
58: /// True predicate
59: ///
60: /// Is used as a predicate to select all tests in all test fixtures in all test suites
61: /// </summary>
62: struct True
63: {
64:     /// <summary>
65:     /// Returns test selection value
66:     /// </summary>
67:     /// <returns>Returns true for all tests, meaning that all tests are selected</returns>
68:     bool operator()(const TestInfo* const) const
69:     {
70:         return true;
71:     }
72:     /// <summary>
73:     /// Returns test fixture selection value
74:     /// </summary>
75:     /// <returns>Returns true for all test fixtures, meaning that all test fixtures are selected</returns>
76:     bool operator()(const TestFixtureInfo* const) const
77:     {
78:         return true;
79:     }
80:     /// <summary>
81:     /// Returns test suite selection value
82:     /// </summary>
83:     /// <returns>Returns true for all test suites, meaning that all test suites are selected</returns>
84:     bool operator()(const TestSuiteInfo* const) const
85:     {
86:         return true;
87:     }
88: };
89: 
90: /// <summary>
91: /// Selection predicate
92: ///
93: /// Is used as a predicate to include specific tests in specific test fixtures in specific test suites
94: /// </summary>
95: class InSelection
96: {
97: private:
98:     /// @brief Test suite name to select
99:     const char* m_suiteName;
100:     /// @brief Test fixture name to select
101:     const char* m_fixtureName;
102:     /// @brief Test name to select
103:     const char* m_testName;
104: 
105: public:
106:     /// <summary>
107:     /// Constructor
108:     /// </summary>
109:     /// <param name="suiteName">Test suite name to select (nullptr to select all)</param>
110:     /// <param name="fixtureName">Test fixture name to select (nullptr to select all)</param>
111:     /// <param name="testName">Test name to select (nullptr to select all)</param>
112:     InSelection(const char* suiteName, const char* fixtureName, const char* testName)
113:         : m_suiteName{suiteName}
114:         , m_fixtureName{fixtureName}
115:         , m_testName{testName}
116:     {
117:     }
118:     bool operator()(const TestInfo* const test) const;
119:     bool operator()(const TestFixtureInfo* const fixture) const;
120:     bool operator()(const TestSuiteInfo* const suite) const;
121: };
122: 
123: /// <summary>
124: /// Test runner
125: ///
126: /// Runs a selected set of tests, and reports using a test reporter
127: /// </summary>
128: class TestRunner
129: {
130: private:
131:     /// @brief Selected test reports
132:     ITestReporter* m_reporter;
133:     /// @brief Accumulated results for all tests run
134:     TestResults m_testResults;
135: 
136: public:
137:     TestRunner(const TestRunner&) = delete;
138:     explicit TestRunner(ITestReporter* reporter);
139:     ~TestRunner() = default;
140: 
141:     TestRunner& operator=(const TestRunner&) = delete;
142: 
143:     template <class Predicate>
144:     int RunTestsIf(TestRegistry const& registry, const Predicate& predicate);
145: 
146: private:
147:     template <class Predicate>
148:     void Start(const Predicate& predicate) const;
149: 
150:     template <class Predicate>
151:     int Finish(const Predicate& predicate) const;
152: };
153: 
154: /// <summary>
155: /// Runs all tests selected by the predicate
156: ///
157: /// The test registry is built at static initialization time and contains all registered tests.
158: /// </summary>
159: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
160: /// <param name="registry">Test registry</param>
161: /// <param name="predicate">Test selection predicate</param>
162: /// <returns>Number of test failures</returns>
163: template <class Predicate>
164: int TestRunner::RunTestsIf(TestRegistry const& registry, const Predicate& predicate)
165: {
166:     Start(predicate);
167: 
168:     TestRegistry::GetTestRegistry().RunIf(predicate, m_testResults);
169: 
170:     return Finish(predicate);
171: }
172: 
173: /// <summary>
174: /// Start a test run
175: /// </summary>
176: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
177: /// <param name="predicate">Test selection predicate</param>
178: template <class Predicate>
179: void TestRunner::Start(const Predicate& predicate) const
180: {
181:     TestRegistry& registry = TestRegistry::GetTestRegistry();
182:     int numberOfTestSuites = registry.CountSuitesIf(predicate);
183:     int numberOfTestFixtures = registry.CountFixturesIf(predicate);
184:     int numberOfTests = registry.CountTestsIf(predicate);
185:     m_reporter->ReportTestRunStart(numberOfTestSuites, numberOfTestFixtures, numberOfTests);
186: }
187: 
188: /// <summary>
189: /// Finish a test run
190: /// </summary>
191: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
192: /// <param name="predicate">Test selection predicate</param>
193: /// <returns>Number of test failures</returns>
194: template <class Predicate>
195: int TestRunner::Finish(const Predicate& predicate) const
196: {
197:     m_reporter->ReportTestRunSummary(m_testResults);
198:     m_reporter->ReportTestRunOverview(m_testResults);
199: 
200:     TestRegistry& registry = TestRegistry::GetTestRegistry();
201:     int numberOfTestSuites = registry.CountSuitesIf(predicate);
202:     int numberOfTestFixtures = registry.CountFixturesIf(predicate);
203:     int numberOfTests = registry.CountTestsIf(predicate);
204:     m_reporter->ReportTestRunFinish(numberOfTestSuites, numberOfTestFixtures, numberOfTests);
205: 
206:     return m_testResults.GetFailureCount();
207: }
208: 
209: int RunAllTests(ITestReporter* reporter = nullptr);
210: 
211: /// <summary>
212: /// Run tests selected by predicate
213: /// </summary>
214: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
215: /// <param name="reporter">Test reporter to use</param>
216: /// <param name="predicate">Test selection predicate</param>
217: /// <returns>Number of test failures</returns>
218: template <class Predicate>
219: int RunSelectedTests(ITestReporter* reporter, const Predicate& predicate)
220: {
221:     TestRunner runner(reporter);
222:     return runner.RunTestsIf(TestRegistry::GetTestRegistry(), predicate);
223: }
224: 
225: } // namespace unittest
```

- Line 57-88: We declare a struct `True` the holds three operators, that all return true.
This is used for filtering test suites, test fixtures and tests
  - Line 64-71: The operator `(const TestInfo*)` will match to any test
  - Line 72-79: The operator `(const TestFixtureInfo*)` will match to any test fixture
  - Line 80-87: The operator `(const TestSuiteInfo*)` will match to any test suite
- Line 90-121: We declare a similar struct `InSelection` that holds the same three operators.
This one however will returns true for a method, if the corresponding test name, test fixture name or test suite name is equal to a set value
  - Line 98-103: We declare class variables `m_suitename`, `m_fixtureName` and `m_testName` containing the test suite name, test fixture name, and test name to use as a filter
  - Line 106-117: We define a constructor, which takes a test suite name, a test fixture name, and a test name, each as a character pointer, which may be nullptr.
The corresponding values are saved as the test suite name, a test fixture name, and a test name to filter by
  - Line 118: We declare the operator `(const TestInfo*)` which will return true if the test name is equal to the set filter value `m_testName`, or `m_testName` is a nullptr
  - Line 119: We declare the operator `(const TestFixtureInfo*)` which will return true if the test fixture name is equal to the set filter value `m_fixtureName`, or `m_fixtureName` is a nullptr
  - Line 120: We declare the operator `(const TestSuiteInfo*)` which will return true if the test suite name is equal to the set filter value `m_suiteName`, or `m_suiteName` is a nullptr
- Line 123-152: We declare the class `TestRunner` which allows to run tests with a set filter, which will use a test reporter instance for reporting
  - Line 131-132: `m_reporter` stores the passed test reporter instance pointer
  - Line 133-134: `m_testResults` stores the test results
  - Line 137: We remove the copy constructor
  - Line 138: We declare an explicit constructor taking a test reporter instance pointer
  - Line 139: We declare a destructor with default implementation
  - Line 141: We remove the assignment operator
  - Line 144: We declare a template method `RunTestsIf()` that takes a predicate (which could be an instance of the `True` class or an instance or the `InSelection` class, or any other class that supportes the same three `()` operators).
This method will run any test that matches the predicate passed
  - Line 147-148: We declare a private template method `Start()` that takes a predicate.
This collects information on the number of test suites, test fixtures and tests, and reports the start of the test run
  - Line 150-151: We declare a private template method `Finish()` that takes a predicate.
This reports a test run summary and overview, and the end of the test run
- Line 154-171: We implement the `RunTestsIf()` template method.
This will use the method `RunIf()` to run tests matching the predicate, with the test results passed to fill.
We will need to implement this method in the `TestRegistry` class
- Line 173-186: We implement the `Start()` template method.
Note that it runs methods `CountSuitesIf()`, `CountFixturesIf()` and `CountTestsIf()` to count test suites, test fixtures and test, using the predicate.
We will need to implement those in the `TestRegistry` class
- Line 188-207: We implement the `Finish()` template method.
Again, the methods `CountSuitesIf()`, `CountFixturesIf()` and `CountTestsIf()` are used to count tests, etc.
The method returns the number of failures, which is reported back by the `RunTestsIf()` template method
- Line 209: We declare a function `RunAllTests()` which will simply run all tests and use the passed test reporter for reporting
- Line 211-223: We define a template function `RunSelectedTests()` which will run all tests matching a predicate and use the passed test reporter for reporting

### TestRunner.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTRUNNERCPP}

Let's implement the `TestRunner` class. Most of the methods are template methods, so we will only need to imlement part here.

Create the file `code/libraries/unittest/src/TestRunner.cpp`

```cpp
File: code/libraries/unittest/src/TestRunner.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/TestRunner.h"
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
82:     : m_reporter{reporter}
83:     , m_testResults{reporter}
84: {
85: }
86: 
87: /// <summary>
88: /// Run all tests with specified test reporter
89: /// </summary>
90: /// <param name="reporter">Test reporter to use</param>
91: /// <returns></returns>
92: int RunAllTests(ITestReporter* reporter)
93: {
94:     return RunSelectedTests(reporter, True());
95: }
96: 
97: } // namespace unittest
```

- Line 47-55: We implement the operator `(const TestInfo*)` for `InSelection`. This will return true if the test name matches the filter, or `m_testName` is a nullptr
- Line 57-65: We implement the operator `(const TestFixtureInfo*)` for `InSelection`. This will return true if the test fixture name matches the filter, or `m_fixtureName` is a nullptr
- Line 67-75: We implement the operator `(const TestSuiteInfo*)` for `InSelection`. This will return true if the test suite name matches the filter, or `m_suiteName` is a nullptr
- Line 77-85: We define the constructor for `TestRunner`
- Line 87-95: We implement the function `RunAllTests()`, which simple calls `RunSelectedTests` with the `True` predicate

### TestRegistry.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTREGISTRYH}

In order to be able to run tests with a predicate, we need to update the `TestRegistry` class.

Update the file `code/libraries/unittest/include/unittest/TestRegistry.h`

```cpp
File: code/libraries/unittest/include/unittest/TestRegistry.h
...
53: /// <summary>
54: /// Test registry
55: /// </summary>
56: class TestRegistry
57: {
58: private:
59:     friend class TestRegistrar;
60:     /// @brief Pointer to first test suite in the list
61:     TestSuiteInfo* m_head;
62:     /// @brief Pointer to last test suite in the list
63:     TestSuiteInfo* m_tail;
64: 
65: public:
66:     TestRegistry();
67:     TestRegistry(const TestRegistry&) = delete;
68:     TestRegistry(TestRegistry&&) = delete;
69:     virtual ~TestRegistry();
70: 
71:     TestRegistry& operator=(const TestRegistry&) = delete;
72:     TestRegistry& operator=(TestRegistry&&) = delete;
73: 
74:     /// <summary>
75:     /// Returns a pointer to the first test suite in the list
76:     /// </summary>
77:     /// <returns>Pointer to the first test suite in the list</returns>
78:     TestSuiteInfo* FirstTestSuite() const
79:     {
80:         return m_head;
81:     }
82: 
83:     /// <summary>
84:     /// Returns the pointer to the next test suite in the list from the current pointer
85:     /// </summary>
86:     /// <param name="current">Pointer to current test suite in the list</param>
87:     /// <returns>Pointer to the next test suite in the list</returns>
88:     TestSuiteInfo* NextTestSuite(TestSuiteInfo* current) const
89:     {
90:         return current->Next();
91:     }
92: 
93:     template <typename Predicate>
94:     void RunIf(const Predicate& predicate, TestResults& testResults);
95:     template <typename Predicate>
96:     int CountSuitesIf(Predicate predicate);
97:     template <typename Predicate>
98:     int CountFixturesIf(Predicate predicate);
99:     template <typename Predicate>
100:     int CountTestsIf(Predicate predicate);
101: 
102:     static TestRegistry& GetTestRegistry();
103: 
104: private:
105:     TestSuiteInfo* GetTestSuite(const baremetal::String& suiteName);
106:     void AddSuite(TestSuiteInfo* testSuite);
107: };
108: 
109: /// <summary>
110: /// Test registrar
111: ///
112: /// This is a utility class to register a test to the registry, as part of a test declaration
113: /// </summary>
114: class TestRegistrar
115: {
116: public:
117:     TestRegistrar(TestRegistry& registry, Test* testInstance, const TestDetails& details);
118: };
119: 
120: /// <summary>
121: /// Run tests selected by the predicate
122: /// </summary>
123: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
124: /// <param name="predicate">Test selection predicate</param>
125: /// <param name="testResults">Test results to be returned</param>
126: template <typename Predicate>
127: void TestRegistry::RunIf(const Predicate& predicate, TestResults& testResults)
128: {
129:     TestSuiteInfo* testSuite = FirstTestSuite();
130: 
131:     while (testSuite != nullptr)
132:     {
133:         if (predicate(testSuite))
134:             testSuite->RunIf(predicate, testResults);
135:         testSuite = NextTestSuite(testSuite);
136:     }
137: }
138: 
139: /// <summary>
140: /// Count the number of test suites selected by the predicate
141: /// </summary>
142: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
143: /// <param name="predicate">Test selection predicate</param>
144: /// <returns>Number of test suites selected by the predicate</returns>
145: template <typename Predicate>
146: int TestRegistry::CountSuitesIf(Predicate predicate)
147: {
148:     int numberOfTestSuites = 0;
149:     TestSuiteInfo* testSuite = FirstTestSuite();
150:     while (testSuite != nullptr)
151:     {
152:         if (predicate(testSuite))
153:             ++numberOfTestSuites;
154:         testSuite = NextTestSuite(testSuite);
155:     }
156:     return numberOfTestSuites;
157: }
158: 
159: /// <summary>
160: /// Count the number of tests fixtures selected by the predicate
161: /// </summary>
162: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
163: /// <param name="predicate">Test selection predicate</param>
164: /// <returns>Number of test fixtures selected by the predicate</returns>
165: template <typename Predicate>
166: int TestRegistry::CountFixturesIf(Predicate predicate)
167: {
168:     int numberOfTestFixtures = 0;
169:     TestSuiteInfo* testSuite = FirstTestSuite();
170:     while (testSuite != nullptr)
171:     {
172:         if (predicate(testSuite))
173:             numberOfTestFixtures += testSuite->CountFixturesIf(predicate);
174:         testSuite = NextTestSuite(testSuite);
175:     }
176:     return numberOfTestFixtures;
177: }
178: 
179: /// <summary>
180: /// Count the number of tests selected by the predicate
181: /// </summary>
182: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
183: /// <param name="predicate">Test selection predicate</param>
184: /// <returns>Number of tests selected by the predicate</returns>
185: template <typename Predicate>
186: int TestRegistry::CountTestsIf(Predicate predicate)
187: {
188:     int numberOfTests = 0;
189:     TestSuiteInfo* testSuite = FirstTestSuite();
190:     while (testSuite != nullptr)
191:     {
192:         if (predicate(testSuite))
193:             numberOfTests += testSuite->CountTestsIf(predicate);
194:         testSuite = NextTestSuite(testSuite);
195:     }
196:     return numberOfTests;
197: }
198: 
199: } // namespace unittest
```

- Line 93-94: We replace the `Run()` method with a template version `RunIf()`, which takes a predicate, as well as a `TestResults` reference
- Line 95-96: We replace then `CountSuites()` method with a template version `CountSuitesIf()`, which takes a predicate
- Line 97-98: We replace then `CountFixtures()` method with a template version `CountFixturesIf()`, which takes a predicate
- Line 99-100: We replace then `CountTests()` method with a template version `CountTestsIf()`, which takes a predicate
- Line 120-137: We implement the `RunIf()` template method.
Note that we use the `RunIf()` method on the test suite. We'll need to implement this
- Line 139-157: We implement the `CountSuitesIf()` template method
- Line 159-177: We implement the `CountFixturesIf()` template method.
Note that we use the `CountFixturesIf()` method on the test suite. We'll need to implement this
- Line 179-197: We implement the `CountTestsIf()` template method.
Note that we use the `CountTestsIf()` method on the test suite. We'll need to implement this

### TestRegistry.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTREGISTRYCPP}

We'll also need to update the `TestRegistry` implementation.
We'll remove the replaced methods `Run()`, `CountSuites()`, `CountFixtures()` and `CountTests()`.

Update the file `code/libraries/unittest/src/TestRegistry.cpp`

```cpp
File: code/libraries/unittest/src/TestRegistry.cpp
...
40: #include "unittest/TestRegistry.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
44: #include "unittest/TestInfo.h"
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
82:     TestSuiteInfo* testSuite = FirstTestSuite();
83:     while (testSuite != nullptr)
84:     {
85:         const TestSuiteInfo* currentSuite = testSuite;
86:         testSuite = NextTestSuite(testSuite);
87:         delete currentSuite;
88:     }
89: }
90: 
91: /// <summary>
92: /// Find a test suite with specified name, register a new one if not found
93: /// </summary>
94: /// <param name="suiteName">Test suite name to search for</param>
95: /// <returns>Found or created test suite</returns>
96: TestSuiteInfo* TestRegistry::GetTestSuite(const String& suiteName)
97: {
98:     TestSuiteInfo* testSuite = FirstTestSuite();
99:     while ((testSuite != nullptr) && (testSuite->Name() != suiteName))
100:         testSuite = NextTestSuite(testSuite);
101:     if (testSuite == nullptr)
102:     {
103:         TRACE_DEBUG("Find suite %s ... not found, creating new object", suiteName.c_str());
104:         testSuite = new TestSuiteInfo(suiteName);
105:         AddSuite(testSuite);
106:     }
107:     else
108:     {
109:         TRACE_DEBUG("Find suite %s ... found", suiteName.c_str());
110:     }
111:     return testSuite;
112: }
113: 
114: /// <summary>
115: /// Add a test suite
116: /// </summary>
117: /// <param name="testSuite">Test suite to add</param>
118: void TestRegistry::AddSuite(TestSuiteInfo* testSuite)
119: {
120:     if (m_tail == nullptr)
121:     {
122:         assert(m_head == nullptr);
123:         m_head = testSuite;
124:         m_tail = testSuite;
125:     }
126:     else
127:     {
128:         m_tail->Next() = testSuite;
129:         m_tail = testSuite;
130:     }
131: }
132: 
133: /// <summary>
134: /// Constructor
135: ///
136: /// Finds or registers the test suite specified in the test details of the test.
137: /// Finds or registers the test fixture specified in the test details of the test.
138: /// Adds the test to the fixture found or created.
139: /// </summary>
140: /// <param name="registry">Test registry</param>
141: /// <param name="testInstance">Test instance to link to</param>
142: /// <param name="details">Test details</param>
143: TestRegistrar::TestRegistrar(TestRegistry& registry, Test* testInstance, const TestDetails& details)
144: {
145:     TRACE_DEBUG("Register test %s in fixture %s in suite %s", details.TestName().c_str(), (details.FixtureName().c_str()), (details.SuiteName().c_str()));
146:     TestSuiteInfo* testSuite = registry.GetTestSuite(details.SuiteName());
147:     TestFixtureInfo* testFixture = testSuite->GetTestFixture(details.FixtureName());
148:     TestInfo* test = new TestInfo(testInstance, details);
149:     testFixture->AddTest(test);
150: }
151: 
152: } // namespace unittest
```

- Line 91: We remove the implementation for the methods `Run()`, `CountSuites()`, `CountFixtures()` and `CountTests()`.

### TestSuiteInfo.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTSUITEINFOH}

We also need to update the `TestSuiteInfo` class.
We'

Update the file `code/libraries/unittest/include/unittest/TestSuiteInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuiteInfo.h
...
53: /// <summary>
54: /// Test suite administration
55: ///
56: /// Holds information on a test suite, which includes its name and the list of test fixtures that are part of it
57: /// </summary>
58: class TestSuiteInfo
59: {
60: private:
61:     friend class TestRegistrar;
62:     /// @brief Pointer to first test fixture in the list
63:     TestFixtureInfo* m_head;
64:     /// @brief Pointer to last test fixture in the list
65:     TestFixtureInfo* m_tail;
66:     /// @brief Pointer to next test suite info in the list
67:     TestSuiteInfo* m_next;
68:     /// @brief Test suite name
69:     baremetal::String m_suiteName;
70: 
71: public:
72:     TestSuiteInfo() = delete;
73:     TestSuiteInfo(const TestSuiteInfo&) = delete;
74:     TestSuiteInfo(TestSuiteInfo&&) = delete;
75:     explicit TestSuiteInfo(const baremetal::String& suiteName);
76:     virtual ~TestSuiteInfo();
77: 
78:     TestSuiteInfo& operator=(const TestSuiteInfo&) = delete;
79:     TestSuiteInfo& operator=(TestSuiteInfo&&) = delete;
80: 
81:     /// <summary>
82:     /// Returns the pointer to the first test fixture in the list for this test suite
83:     /// </summary>
84:     /// <returns>Pointer to the first test fixture in the list for this test suite</returns>
85:     TestFixtureInfo* FirstTestFixture() const
86:     {
87:         return m_head;
88:     }
89: 
90:     /// <summary>
91:     /// Returns the pointer to the next test fixture in the list from the current pointer
92:     /// </summary>
93:     /// <param name="current">Pointer to current test fixture in the list</param>
94:     /// <returns>Pointer to the next test fixture in the list</returns>
95:     TestFixtureInfo* NextTestFixture(TestFixtureInfo* current) const
96:     {
97:         return current->Next();
98:     }
99: 
100:     /// <summary>
101:     /// Returns the pointer to the next test suite in the list
102:     /// </summary>
103:     /// <returns>Pointer to the next test suite in the list</returns>
104:     TestSuiteInfo* Next() const
105:     {
106:         return m_next;
107:     }
108: 
109:     /// <summary>
110:     /// Returns a reference to the pointer to the next test suite in the list
111:     /// </summary>
112:     /// <returns>Reference to pointer to next test suite</returns>
113:     TestSuiteInfo*& Next()
114:     {
115:         return m_next;
116:     }
117: 
118:     baremetal::String Name() const;
119: 
120:     template <class Predicate>
121:     void RunIf(const Predicate& predicate, TestResults& testResults);
122: 
123:     int CountFixtures();
124:     int CountTests();
125:     template <typename Predicate>
126:     int CountFixturesIf(Predicate predicate);
127:     template <typename Predicate>
128:     int CountTestsIf(Predicate predicate);
129: 
130: private:
131:     TestFixtureInfo* GetTestFixture(const baremetal::String& fixtureName);
132:     void AddFixture(TestFixtureInfo* testFixture);
133: };
134: 
135: /// <summary>
136: /// Run tests in test suite using the selection predicate, updating the test results
137: /// </summary>
138: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
139: /// <param name="predicate">Test selection predicate</param>
140: /// <param name="testResults">Test results to use and update</param>
141: template <class Predicate>
142: void TestSuiteInfo::RunIf(const Predicate& predicate, TestResults& testResults)
143: {
144:     testResults.OnTestSuiteStart(this);
145: 
146:     TestFixtureInfo* testFixture = FirstTestFixture();
147:     while (testFixture != nullptr)
148:     {
149:         if (predicate(testFixture))
150:             testFixture->RunIf(predicate, testResults);
151:         testFixture = NextTestFixture(testFixture);
152:     }
153: 
154:     testResults.OnTestSuiteFinish(this);
155: }
156: 
157: /// <summary>
158: /// Count the number of tests fixtures in the test suite selected by the predicate
159: /// </summary>
160: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
161: /// <param name="predicate">Test selection predicate</param>
162: /// <returns>Number of test fixtures in the test suite selected by the predicate</returns>
163: template <typename Predicate>
164: int TestSuiteInfo::CountFixturesIf(Predicate predicate)
165: {
166:     int numberOfTestFixtures = 0;
167:     TestFixtureInfo* testFixture = FirstTestFixture();
168:     while (testFixture != nullptr)
169:     {
170:         if (predicate(testFixture))
171:             numberOfTestFixtures++;
172:         testFixture = NextTestFixture(testFixture);
173:     }
174:     return numberOfTestFixtures;
175: }
176: 
177: /// <summary>
178: /// Count the number of tests in the test suite selected by the predicate
179: /// </summary>
180: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
181: /// <param name="predicate">Test selection predicate</param>
182: /// <returns>Number of tests in the test suite selected by the predicate</returns>
183: template <typename Predicate>
184: int TestSuiteInfo::CountTestsIf(Predicate predicate)
185: {
186:     int numberOfTests = 0;
187:     TestFixtureInfo* testFixture = FirstTestFixture();
188:     while (testFixture != nullptr)
189:     {
190:         if (predicate(testFixture))
191:             numberOfTests += testFixture->CountTestsIf(predicate);
192:         testFixture = NextTestFixture(testFixture);
193:     }
194:     return numberOfTests;
195: }
196: 
197: } // namespace unittest
```

- Line 61: We make `TestRegistrar` a friend class
- Line 120-121: We replace the `Run()` method with a template `RunIf()` method, taking both a predicate and the `TestResults` to fill
- Line 125-126: We add a template method `CountFixturesIf()`
- Line 127-128: We add a template method `CountTestsIf()`
- Line 131: We make the method `GetTestFixture` private, this is why `TestRegistrar` has to become a friend class
- Line 132: We also make the method `AddFixture` private
- Line 135-155: We implement the `RunIf()` template method.
Note that we use the method `RunIf()` in the `TestFixtureInfo` class. We'll need to implement it
- Line 157-175: We implement the `CountFixturesIf()` template method
- Line 177-195: We implement the `CountTestsIf()` template method.
Note that we use the method `CountTestsIf()` in the `TestFixtureInfo` class. We'll need to implement it

### TestSuiteInfo.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTSUITEINFOCPP}

We'll also need to update the `TestSuiteInfo` implementation.
We'll remove the replaced `Run()` method.

Update the file `code/libraries/unittest/src/TestSuiteInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestSuiteInfo.cpp
...
40: #include "unittest/TestSuiteInfo.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
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
59: TestSuiteInfo::TestSuiteInfo(const String& suiteName)
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
74:     TestFixtureInfo* testFixture = FirstTestFixture();
75:     while (testFixture != nullptr)
76:     {
77:         const TestFixtureInfo* currentFixture = testFixture;
78:         testFixture = NextTestFixture(testFixture);
79:         delete currentFixture;
80:     }
81: }
82: 
83: /// <summary>
84: /// Find a test fixture with specified name, register a new one if not found
85: /// </summary>
86: /// <param name="fixtureName">Test fixture name to search for</param>
87: /// <returns>Found or created test fixture</returns>
88: TestFixtureInfo* TestSuiteInfo::GetTestFixture(const String& fixtureName)
89: {
90:     TestFixtureInfo* testFixture = FirstTestFixture();
91:     while ((testFixture != nullptr) && (testFixture->Name() != fixtureName))
92:         testFixture = NextTestFixture(testFixture);
93:     if (testFixture == nullptr)
94:     {
95:         TRACE_DEBUG("Fixture %s not found, creating new object", fixtureName.c_str());
96:         testFixture = new TestFixtureInfo(fixtureName);
97:         AddFixture(testFixture);
98:     }
99:     else
100:     {
101:         TRACE_DEBUG("Fixture %s found", fixtureName.c_str());
102:     }
103:     return testFixture;
104: }
105: 
106: /// <summary>
107: /// Add a test fixture
108: /// </summary>
109: /// <param name="testFixture">Test fixture to add</param>
110: void TestSuiteInfo::AddFixture(TestFixtureInfo* testFixture)
111: {
112:     if (m_tail == nullptr)
113:     {
114:         assert(m_head == nullptr);
115:         m_head = testFixture;
116:         m_tail = testFixture;
117:     }
118:     else
119:     {
120:         m_tail->Next() = testFixture;
121:         m_tail = testFixture;
122:     }
123: }
124: 
125: /// <summary>
126: /// Returns the test suite name
127: /// </summary>
128: /// <returns>Test suite name</returns>
129: String TestSuiteInfo::Name() const
130: {
131:     return m_suiteName.empty() ? String(TestDetails::DefaultSuiteName) : m_suiteName;
132: }
133: 
134: /// <summary>
135: /// Count the number of test fixtures in the test suite
136: /// </summary>
137: /// <returns>Number of test fixtures in the test suite</returns>
138: int TestSuiteInfo::CountFixtures()
139: {
140:     int numberOfTestFixtures = 0;
141:     TestFixtureInfo* testFixture = FirstTestFixture();
142:     while (testFixture != nullptr)
143:     {
144:         ++numberOfTestFixtures;
145:         testFixture = NextTestFixture(testFixture);
146:     }
147:     return numberOfTestFixtures;
148: }
149: 
150: /// <summary>
151: /// Count the number of tests in the test suite
152: /// </summary>
153: /// <returns>Number of tests in the test suite</returns>
154: int TestSuiteInfo::CountTests()
155: {
156:     int numberOfTests = 0;
157:     TestFixtureInfo* testFixture = FirstTestFixture();
158:     while (testFixture != nullptr)
159:     {
160:         numberOfTests += testFixture->CountTests();
161:         testFixture = NextTestFixture(testFixture);
162:     }
163:     return numberOfTests;
164: }
165: 
166: } // namespace unittest
```

- Line 134: We remove the implementation for the method `Run()`

### TestFixtureInfo.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTFIXTUREINFOH}

We also need to update the `TestFixtureInfo` class.

Update the file `code/libraries/unittest/include/unittest/TestFixtureInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixtureInfo.h
...
52: /// <summary>
53: /// Test fixture administration
54: ///
55: /// Holds information on a test fixture, which includes its name and the list of tests that are part of it
56: /// </summary>
57: class TestFixtureInfo
58: {
59: private:
60:     friend class TestRegistrar;
61:     /// @brief Pointer to first test in the list
62:     TestInfo* m_head;
63:     /// @brief Pointer to last test in the list
64:     TestInfo* m_tail;
65:     /// @brief Pointer to next test fixture info in the list
66:     TestFixtureInfo* m_next;
67:     /// @brief Test fixture name
68:     baremetal::String m_fixtureName;
69: 
70: public:
71:     TestFixtureInfo() = delete;
72:     TestFixtureInfo(const TestFixtureInfo&) = delete;
73:     TestFixtureInfo(TestFixtureInfo&&) = delete;
74:     explicit TestFixtureInfo(const baremetal::String& fixtureName);
75:     virtual ~TestFixtureInfo();
76: 
77:     TestFixtureInfo& operator=(const TestFixtureInfo&) = delete;
78:     TestFixtureInfo& operator=(TestFixtureInfo&&) = delete;
79: 
80:     /// <summary>
81:     /// Returns the pointer to the first test in the list for this test fixture
82:     /// </summary>
83:     /// <returns>Pointer to the first test in the list for this test fixture</returns>
84:     TestInfo* FirstTest() const
85:     {
86:         return m_head;
87:     }
88: 
89:     /// <summary>
90:     /// Returns the pointer to the next test in the list from the current pointer
91:     /// </summary>
92:     /// <param name="current">Pointer to current test in the list</param>
93:     /// <returns>Pointer to the next test in the list</returns>
94:     TestInfo* NextTest(TestInfo* current) const
95:     {
96:         return current->Next();
97:     }
98: 
99:     /// <summary>
100:     /// Returns the pointer to the next test fixture in the list
101:     /// </summary>
102:     /// <returns>Pointer to the next test fixture in the list</returns>
103:     TestFixtureInfo* Next() const
104:     {
105:         return m_next;
106:     }
107: 
108:     /// <summary>
109:     /// Returns a reference to the pointer to the next test fixture in the list
110:     /// </summary>
111:     /// <returns>Reference to pointer to next test fixture</returns>
112:     TestFixtureInfo*& Next()
113:     {
114:         return m_next;
115:     }
116: 
117:     baremetal::String Name() const;
118: 
119:     template <class Predicate>
120:     void RunIf(const Predicate& predicate, TestResults& testResults);
121: 
122:     int CountTests();
123:     template <typename Predicate>
124:     int CountTestsIf(Predicate predicate);
125: 
126: private:
127:     void AddTest(TestInfo* test);
128: };
129: 
130: /// <summary>
131: /// Run tests in test fixture using the selection predicate, updating the test results
132: /// </summary>
133: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
134: /// <param name="predicate">Test selection predicate</param>
135: /// <param name="testResults">Test results to use and update</param>
136: template <class Predicate>
137: void TestFixtureInfo::RunIf(const Predicate& predicate, TestResults& testResults)
138: {
139:     testResults.OnTestFixtureStart(this);
140: 
141:     TestInfo* test = this->FirstTest();
142:     while (test != nullptr)
143:     {
144:         if (predicate(test))
145:             test->Run(testResults);
146:         test = test->m_next;
147:     }
148: 
149:     testResults.OnTestFixtureFinish(this);
150: }
151: 
152: /// <summary>
153: /// Count the number of tests in the test fixture selected by the predicate
154: /// </summary>
155: /// <typeparam name="Predicate">Predicate class for test selected</typeparam>
156: /// <param name="predicate">Test selection predicate</param>
157: /// <returns>Number of tests in the test fixture selected by the predicate</returns>
158: template <typename Predicate>
159: int TestFixtureInfo::CountTestsIf(Predicate predicate)
160: {
161:     int numberOfTests = 0;
162:     TestInfo* test = this->FirstTest();
163:     while (test != nullptr)
164:     {
165:         if (predicate(test))
166:             numberOfTests++;
167:         test = test->m_next;
168:     }
169:     return numberOfTests;
170: }
171: 
172: } // namespace unittest
```

- Line 60: We make `TestRegistrar` a friend class
- Line 119-120: We replace the `Run()` method with a template `RunIf()` method, taking both a predicate and the `TestResults` to fill
- Line 123-124: We add a template method `CountTestsIf()`
- Line 127: We make the method `AddTest()` private. This is the reason `TestRegistrar` needs to become a friend class
- Line 130-150: We implement the `RunIf()` template method
- Line 152-170: We implement the `CountTestsIf()` template method

### TestFixtureInfo.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_TESTFIXTUREINFOCPP}

We'll also need to update the `TestFixtureInfo` implementation.
We'll remove the `Run()` method.

Update the file `code/libraries/unittest/src/TestFixtureInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestFixtureInfo.cpp
...
39: #include "unittest/TestFixtureInfo.h"
40: 
41: #include "baremetal/Assert.h"
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
54: TestFixtureInfo::TestFixtureInfo(const String& fixtureName)
55:     : m_head{}
56:     , m_tail{}
57:     , m_next{}
58:     , m_fixtureName{fixtureName}
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
69:     TestInfo* test = FirstTest();
70:     while (test != nullptr)
71:     {
72:         const TestInfo* currentTest = test;
73:         test = NextTest(test);
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
94:         m_tail->Next() = test;
95:         m_tail = test;
96:     }
97: }
98: 
99: /// <summary>
100: /// Returns the test fixture name
101: /// </summary>
102: /// <returns>Test fixture name</returns>
103: String TestFixtureInfo::Name() const
104: {
105:     return m_fixtureName.empty() ? String(TestDetails::DefaultFixtureName) : m_fixtureName;
106: }
107: 
108: /// <summary>
109: /// Count the number of tests in the test fixture
110: /// </summary>
111: /// <returns>Number of tests in the test fixture</returns>
112: int TestFixtureInfo::CountTests()
113: {
114:     int numberOfTests = 0;
115:     TestInfo* test = FirstTest();
116:     while (test != nullptr)
117:     {
118:         ++numberOfTests;
119:         test = NextTest(test);
120:     }
121:     return numberOfTests;
122: }
123: 
124: } // namespace unittest
```

- Line 108: We remove the implementation for the method `Run()`

### ConsoleTestReporter.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_CONSOLETESTREPORTERH}

We still need to implement the abstract interface `ITestReporter` to actually report information.
We'll create a console test reporter class `ConsoleTestReporter` for this purpose, which leans on the `baremetal::Console` class.

Create the file `code/libraries/unittest/include/unittest/ConsoleTestReporter.h`

```cpp
File: code/libraries/unittest/include/unittest/ConsoleTestReporter.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "unittest/ITestReporter.h"
43: 
44: /// @file
45: /// Console test reporter
46: ///
47: /// Prints test information to the console, using color.
48: /// Uses DeferredTestReporter to save failures so they can be printed after running the test, as an overview
49: 
50: namespace unittest {
51: 
52: /// <summary>
53: /// Console test reporter
54: /// </summary>
55: class ConsoleTestReporter : public ITestReporter
56: {
57: public:
58:     /// @brief Separator for complete test run
59:     static const baremetal::String TestRunSeparator;
60:     /// @brief Separator for test fixture
61:     static const baremetal::String TestFixtureSeparator;
62:     /// @brief Separator for test suite
63:     static const baremetal::String TestSuiteSeparator;
64:     /// @brief Indicator for successful test
65:     static const baremetal::String TestSuccessSeparator;
66:     /// @brief Indicator for failed test
67:     static const baremetal::String TestFailSeparator;
68: 
69:     ConsoleTestReporter();
70: 
71: private:
72:     void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
73:     void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
74:     void ReportTestRunSummary(const TestResults& results) override;
75:     void ReportTestRunOverview(const TestResults& results) override;
76:     void ReportTestSuiteStart(const baremetal::String& suiteName, int numberOfTestFixtures) override;
77:     void ReportTestSuiteFinish(const baremetal::String& suiteName, int numberOfTestFixtures) override;
78:     void ReportTestFixtureStart(const baremetal::String& fixtureName, int numberOfTests) override;
79:     void ReportTestFixtureFinish(const baremetal::String& fixtureName, int numberOfTests) override;
80:     void ReportTestStart(const TestDetails& details) override;
81:     void ReportTestFinish(const TestDetails& details, bool success) override;
82:     void ReportTestFailure(const TestDetails& details, const baremetal::String& failure) override;
83: 
84:     baremetal::String TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests);
85:     baremetal::String TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests);
86:     baremetal::String TestSuiteStartMessage(const baremetal::String& suiteName, int numberOfTestFixtures);
87:     baremetal::String TestSuiteFinishMessage(const baremetal::String& suiteName, int numberOfTestFixtures);
88:     baremetal::String TestFixtureStartMessage(const baremetal::String& fixtureName, int numberOfTests);
89:     baremetal::String TestFixtureFinishMessage(const baremetal::String& fixtureName, int numberOfTests);
90:     baremetal::String TestFailureMessage(const TestDetails& details, const baremetal::String& failure);
91:     baremetal::String TestFinishMessage(const TestDetails& details, bool success);
92:     baremetal::String TestRunSummaryMessage(const TestResults& results);
93:     baremetal::String TestRunOverviewMessage(const TestResults& results);
94: };
95: 
96: } // namespace unittest
```

- Line 58-67: We declare some static constant strings to use as separators in our output
- Line 69: We declare a default constructor
- Line 72-82: We declare the overrides for the `ITestReporter` abstract interface
- Line 84-93: We declare some utility functions to build strings for output

### ConsoleTestReporter.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_CONSOLETESTREPORTERCPP}

We'll implement the class `ConsoleTestReporter`.

Create the file `code/libraries/unittest/src/ConsoleTestReporter.cpp`

```cpp
File: code/libraries/unittest/src/ConsoleTestReporter.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/ConsoleTestReporter.h"
41: 
42: #include "baremetal/Console.h"
43: #include "baremetal/Format.h"
44: #include "baremetal/Serialization.h"
45: #include "unittest/TestDetails.h"
46: #include "unittest/TestRegistry.h"
47: #include "unittest/TestResults.h"
48: 
49: /// @file
50: /// Console test reporter implementation
51: 
52: using namespace baremetal;
53: 
54: namespace unittest {
55: 
56: const String ConsoleTestReporter::TestRunSeparator = baremetal::String("[===========]");
57: const String ConsoleTestReporter::TestSuiteSeparator = baremetal::String("[   SUITE   ]");
58: const String ConsoleTestReporter::TestFixtureSeparator = baremetal::String("[  FIXTURE  ]");
59: const String ConsoleTestReporter::TestSuccessSeparator = baremetal::String("[ SUCCEEDED ]");
60: const String ConsoleTestReporter::TestFailSeparator = baremetal::String("[  FAILED   ]");
61: 
62: /// <summary>
63: /// Constructor
64: /// </summary>
65: ConsoleTestReporter::ConsoleTestReporter()
66: {
67: }
68: 
69: /// <summary>
70: /// Start of test run callback
71: /// </summary>
72: /// <param name="numberOfTestSuites">Number of test suites to be run</param>
73: /// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
74: /// <param name="numberOfTests">Number of tests to be run</param>
75: void ConsoleTestReporter::ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
76: {
77:     GetConsole().SetTerminalColor(ConsoleColor::Green);
78:     GetConsole().Write(TestRunSeparator);
79:     GetConsole().ResetTerminalColor();
80: 
81:     GetConsole().Write(Format(" %s\n", TestRunStartMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
82: }
83: 
84: /// <summary>
85: /// Finish of test run callback
86: /// </summary>
87: /// <param name="numberOfTestSuites">Number of test suites run</param>
88: /// <param name="numberOfTestFixtures">Number of test fixtures run</param>
89: /// <param name="numberOfTests">Number of tests run</param>
90: void ConsoleTestReporter::ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
91: {
92:     GetConsole().SetTerminalColor(ConsoleColor::Green);
93:     GetConsole().Write(TestRunSeparator);
94:     GetConsole().ResetTerminalColor();
95: 
96:     GetConsole().Write(Format(" %s\n", TestRunFinishMessage(numberOfTestSuites, numberOfTestFixtures, numberOfTests).c_str()));
97: }
98: 
99: /// <summary>
100: /// Test summary callback
101: /// </summary>
102: /// <param name="results">Test run results</param>
103: void ConsoleTestReporter::ReportTestRunSummary(const TestResults& results)
104: {
105:     if (results.GetFailureCount() > 0)
106:         GetConsole().SetTerminalColor(ConsoleColor::Red);
107:     else
108:         GetConsole().SetTerminalColor(ConsoleColor::Green);
109:     GetConsole().Write(Format("%s\n", TestRunSummaryMessage(results).c_str()));
110:     GetConsole().ResetTerminalColor();
111: }
112: 
113: /// <summary>
114: /// Test run overview callback
115: /// </summary>
116: /// <param name="results">Test run results</param>
117: void ConsoleTestReporter::ReportTestRunOverview(const TestResults& results)
118: {
119:     GetConsole().Write(Format("%s\n", TestRunOverviewMessage(results).c_str()));
120: }
121: 
122: /// <summary>
123: /// Test suite start callback
124: /// </summary>
125: /// <param name="suiteName">Test suite name</param>
126: /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
127: void ConsoleTestReporter::ReportTestSuiteStart(const String& suiteName, int numberOfTestFixtures)
128: {
129:     GetConsole().SetTerminalColor(ConsoleColor::Cyan);
130:     GetConsole().Write(TestSuiteSeparator);
131:     GetConsole().ResetTerminalColor();
132: 
133:     GetConsole().Write(Format(" %s\n", TestSuiteStartMessage(suiteName, numberOfTestFixtures).c_str()));
134: }
135: 
136: /// <summary>
137: /// Test suite finish callback
138: /// </summary>
139: /// <param name="suiteName">Test suite name</param>
140: /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
141: void ConsoleTestReporter::ReportTestSuiteFinish(const String& suiteName, int numberOfTestFixtures)
142: {
143:     GetConsole().SetTerminalColor(ConsoleColor::Cyan);
144:     GetConsole().Write(TestSuiteSeparator);
145:     GetConsole().ResetTerminalColor();
146: 
147:     GetConsole().Write(Format(" %s\n", TestSuiteFinishMessage(suiteName, numberOfTestFixtures).c_str()));
148: }
149: 
150: /// <summary>
151: /// Test fixture start callback
152: /// </summary>
153: /// <param name="fixtureName">Test fixture name</param>
154: /// <param name="numberOfTests">Number of tests within test fixture</param>
155: void ConsoleTestReporter::ReportTestFixtureStart(const String& fixtureName, int numberOfTests)
156: {
157:     GetConsole().SetTerminalColor(ConsoleColor::Yellow);
158:     GetConsole().Write(TestFixtureSeparator);
159:     GetConsole().ResetTerminalColor();
160: 
161:     GetConsole().Write(Format(" %s\n", TestFixtureStartMessage(fixtureName, numberOfTests).c_str()));
162: }
163: 
164: /// <summary>
165: /// Test fixture finish callback
166: /// </summary>
167: /// <param name="fixtureName">Test fixture name</param>
168: /// <param name="numberOfTests">Number of tests within test fixture</param>
169: void ConsoleTestReporter::ReportTestFixtureFinish(const String& fixtureName, int numberOfTests)
170: {
171:     GetConsole().SetTerminalColor(ConsoleColor::Yellow);
172:     GetConsole().Write(TestFixtureSeparator);
173:     GetConsole().ResetTerminalColor();
174: 
175:     GetConsole().Write(Format(" %s\n", TestFixtureFinishMessage(fixtureName, numberOfTests).c_str()));
176: }
177: 
178: /// <summary>
179: /// Test start callback
180: /// </summary>
181: /// <param name="details">Test details</param>
182: void ConsoleTestReporter::ReportTestStart(const TestDetails& details)
183: {
184: }
185: 
186: /// <summary>
187: /// Test finish callback
188: /// </summary>
189: /// <param name="details">Test details</param>
190: /// <param name="success">Test result, true is successful, false is failed</param>
191: void ConsoleTestReporter::ReportTestFinish(const TestDetails& details, bool success)
192: {
193:     GetConsole().SetTerminalColor(success ? ConsoleColor::Green : ConsoleColor::Red);
194:     if (success)
195:         GetConsole().Write(TestSuccessSeparator);
196:     else
197:         GetConsole().Write(TestFailSeparator);
198:     GetConsole().ResetTerminalColor();
199: 
200:     GetConsole().Write(Format(" %s\n", TestFinishMessage(details, success).c_str()));
201: }
202: 
203: /// <summary>
204: /// Test failure callback
205: /// </summary>
206: /// <param name="details">Test details</param>
207: /// <param name="failure">Test failure message</param>
208: void ConsoleTestReporter::ReportTestFailure(const TestDetails& details, const String& failure)
209: {
210:     GetConsole().SetTerminalColor(ConsoleColor::Red);
211:     GetConsole().Write(Format("%s\n", TestFailureMessage(details, failure).c_str()));
212:     GetConsole().ResetTerminalColor();
213: }
214: 
215: /// <summary>
216: /// Return the correct singular or plural form for "test"
217: /// </summary>
218: /// <param name="numberOfTests">Number of tests</param>
219: /// <returns></returns>
220: static String TestLiteral(int numberOfTests)
221: {
222:     return baremetal::String((numberOfTests == 1) ? "test" : "tests");
223: }
224: 
225: /// <summary>
226: /// Return the correct singular or plural form for "failure"
227: /// </summary>
228: /// <param name="numberOfTestFailures">Number of test failures</param>
229: /// <returns></returns>
230: static String TestFailureLiteral(int numberOfTestFailures)
231: {
232:     return baremetal::String((numberOfTestFailures == 1) ? "failure" : "failures");
233: }
234: 
235: /// <summary>
236: /// Return the correct singular or plural form for "fixture"
237: /// </summary>
238: /// <param name="numberOfTestFixtures">Number of test fixtures</param>
239: /// <returns></returns>
240: static String TestFixtureLiteral(int numberOfTestFixtures)
241: {
242:     return baremetal::String((numberOfTestFixtures == 1) ? "fixture" : "fixtures");
243: }
244: 
245: /// <summary>
246: /// Return the correct singular or plural form for "suite"
247: /// </summary>
248: /// <param name="numberOfTestSuites">Number of test suites</param>
249: /// <returns></returns>
250: static String TestSuiteLiteral(int numberOfTestSuites)
251: {
252:     return baremetal::String((numberOfTestSuites == 1) ? "suite" : "suites");
253: }
254: 
255: /// <summary>
256: /// Create a message for test run start
257: /// </summary>
258: /// <param name="numberOfTestSuites">Number of test suites to be run</param>
259: /// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
260: /// <param name="numberOfTests">Number of tests to be run</param>
261: /// <returns>Resulting message</returns>
262: String ConsoleTestReporter::TestRunStartMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
263: {
264:     // clang-format off
265:     return Format("Running %s %s from %s %s in %s %s.",
266:         Serialize(numberOfTests).c_str(),
267:         TestLiteral(numberOfTests).c_str(),
268:         Serialize(numberOfTestFixtures).c_str(),
269:         TestFixtureLiteral(numberOfTestFixtures).c_str(),
270:         Serialize(numberOfTestSuites).c_str(),
271:         TestSuiteLiteral(numberOfTestSuites).c_str());
272:     // clang-format on
273: }
274: 
275: /// <summary>
276: /// Create a message for test run finish
277: /// </summary>
278: /// <param name="numberOfTestSuites">Number of test suites run</param>
279: /// <param name="numberOfTestFixtures">Number of test fixtures run</param>
280: /// <param name="numberOfTests">Number of tests run</param>
281: /// <returns>Resulting message</returns>
282: String ConsoleTestReporter::TestRunFinishMessage(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests)
283: {
284:     // clang-format off
285:     return Format("%s %s from %s %s in %s %s ran.",
286:         Serialize(numberOfTests).c_str(),
287:         TestLiteral(numberOfTests).c_str(),
288:         Serialize(numberOfTestFixtures).c_str(),
289:         TestFixtureLiteral(numberOfTestFixtures).c_str(),
290:         Serialize(numberOfTestSuites).c_str(),
291:         TestSuiteLiteral(numberOfTestSuites).c_str());
292:     // clang-format on
293: }
294: 
295: /// <summary>
296: /// Create a message for test run summary
297: /// </summary>
298: /// <param name="results">Test run results</param>
299: /// <returns>Resulting message</returns>
300: String ConsoleTestReporter::TestRunSummaryMessage(const TestResults& results)
301: {
302:     // clang-format off
303:     if (results.GetFailureCount() > 0)
304:     {
305:         return Format("FAILURE: %s out of %s %s failed (%s %s).\n",
306:             Serialize(results.GetFailedTestCount()).c_str(),
307:             Serialize(results.GetTotalTestCount()).c_str(),
308:             TestLiteral(results.GetTotalTestCount()).c_str(),
309:             Serialize(results.GetFailureCount()).c_str(),
310:             TestFailureLiteral(results.GetFailureCount()).c_str());
311:     }
312:     return Format("Success: %s %s passed.\n",
313:         Serialize(results.GetTotalTestCount()).c_str(),
314:         TestLiteral(results.GetTotalTestCount()).c_str());
315:     // clang-format on
316: }
317: 
318: /// <summary>
319: /// Create a message for test run overview
320: /// </summary>
321: /// <param name="results">Test run results</param>
322: /// <returns>Resulting message</returns>
323: String ConsoleTestReporter::TestRunOverviewMessage(const TestResults& results)
324: {
325: 
326:     if (results.GetFailureCount() > 0)
327:     {
328:         return Format("Failures: %d", results.GetFailureCount());
329:     }
330:     return "No failures";
331: }
332: 
333: /// <summary>
334: /// Create a message for test failure
335: /// </summary>
336: /// <param name="result">Test run results</param>
337: /// <param name="failure">Failure that occurred</param>
338: /// <returns>Resulting message</returns>
339: String ConsoleTestReporter::TestFailureMessage(const TestDetails& details, const String& failure)
340: {
341:     return Format("%s failure %s", details.QualifiedTestName().c_str(), failure.c_str());
342: }
343: 
344: /// <summary>
345: /// Create a message for test suite start
346: /// </summary>
347: /// <param name="suiteName">Name of test suite</param>
348: /// <param name="numberOfTestFixtures">Number of test fixtures within test suite</param>
349: /// <returns>Resulting message</returns>
350: String ConsoleTestReporter::TestSuiteStartMessage(const String& suiteName, int numberOfTestFixtures)
351: {
352:     // clang-format off
353:     return Format("%s (%s %s)",
354:         suiteName.c_str(),
355:         Serialize(numberOfTestFixtures).c_str(),
356:         TestFixtureLiteral(numberOfTestFixtures).c_str());
357:     // clang-format on
358: }
359: 
360: /// <summary>
361: /// Create a message for test suite finish
362: /// </summary>
363: /// <param name="suiteName">Name of test suite</param>
364: /// <param name="numberOfTestFixtures">Number of test fixtures within test suite</param>
365: /// <returns>Resulting message</returns>
366: String ConsoleTestReporter::TestSuiteFinishMessage(const String& suiteName, int numberOfTestFixtures)
367: {
368:     // clang-format off
369:     return Format("%s %s from %s",
370:         Serialize(numberOfTestFixtures).c_str(),
371:         TestFixtureLiteral(numberOfTestFixtures).c_str(),
372:         suiteName.c_str());
373:     // clang-format on
374: }
375: 
376: /// <summary>
377: /// Create a message for test fixture start
378: /// </summary>
379: /// <param name="fixtureName">Test fixture name</param>
380: /// <param name="numberOfTests">Number of tests within test fixture</param>
381: /// <returns>Resulting message</returns>
382: String ConsoleTestReporter::TestFixtureStartMessage(const String& fixtureName, int numberOfTests)
383: {
384:     // clang-format off
385:     return Format("%s (%s %s)",
386:         fixtureName.c_str(),
387:         Serialize(numberOfTests).c_str(),
388:         TestLiteral(numberOfTests).c_str());
389:     // clang-format on
390: }
391: 
392: /// <summary>
393: /// Create a message for test fixture finish
394: /// </summary>
395: /// <param name="fixtureName">Test fixture name</param>
396: /// <param name="numberOfTests">Number of tests within test fixture</param>
397: /// <returns>Resulting message</returns>
398: String ConsoleTestReporter::TestFixtureFinishMessage(const String& fixtureName, int numberOfTests)
399: {
400:     // clang-format off
401:     return Format("%s %s from %s",
402:         Serialize(numberOfTests).c_str(),
403:         TestLiteral(numberOfTests).c_str(),
404:         fixtureName.c_str());
405:     // clang-format on
406: }
407: 
408: /// <summary>
409: /// Create a message for test finish
410: /// </summary>
411: /// <param name="details">Test details</param>
412: /// <param name="success">Test result, true is successful, false is failed</param>
413: /// <returns>Resulting message</returns>
414: String ConsoleTestReporter::TestFinishMessage(const TestDetails& details, bool /*success*/)
415: {
416:     return details.QualifiedTestName();
417: }
418: 
419: } // namespace unittest
```

- Line 56-60: We initialize the static member variables
- Line 62-67: We implement the constructor
- Line 69-82: We implement the method `ReportTestRunStart()`.
This prints the run separator in green, and uses the method `TestRunStartMessage()` to format the string to be printed
- Line 84-97: We implement the method `ReportTestRunFinish()`.
This prints the run separator in green, and uses the method `TestRunFinishMessage()` to format the string to be printed
- Line 99-111: We implement the method `ReportTestRunSummary()`.
This uses the method `TestRunSummaryMessage()` to format the string to be printed, in green if successful, in red if failures occured
- Line 113-120: We implement the method `ReportTestRunOverview()`.
This uses the method `TestRunOverviewMessage()` to format the string to be printed
- Line 122-134: We implement the method `ReportTestSuiteStart()`.
This prints the test suite separator in cyan, and uses the method `TestSuiteStartMessage()` to format the string to be printed
- Line 136-148: We implement the method `ReportTestSuiteFinish()`.
This prints the test suite separator in cyan, and uses the method `TestSuiteFinishMessage()` to format the string to be printed
- Line 150-162: We implement the method `ReportTestFixtureStart()`.
This prints the test fixture separator in yellow, and uses the method `TestFixtureStartMessage()` to format the string to be printed
- Line 164-176: We implement the method `ReportTestFixtureFinish()`.
This prints the test fixture separator in yellow, and uses the method `TestFixtureFinishMessage()` to format the string to be printed
- Line 178-184: We implement the method `ReportTestStart()`.
This prints nothing
- Line 186-201: We implement the method `ReportTestFinish()`.
This prints the success separator in green if successful, or the failure separator in red if failures occurred, and uses the method `TestFinishMessage()` to format the string to be printed
- Line 203-213: We implement the method `ReportTestFailure()`.
This uses the method `TestFailureMessage()` to format the string to be printed in red
- Line 215-223: We implement a static function `TestLiteral()`, which returns the string "test" if the number of tests equals 1, and "tests" otherwise
- Line 225-233: We implement a static function `TestFailureLiteral()`, which returns the string "failure" if the number of failures equals 1, and "failures" otherwise
- Line 235-243: We implement a static function `TestFixtureLiteral()`, which returns the string "fixture" if the number of test fixtures equals 1, and "fixtures" otherwise
- Line 245-253: We implement a static function `TestSuiteLiteral()`, which returns the string "suite" if the number of test suites equals 1, and "suites" otherwise
- Line 255-273: We implement a static function `TestRunStartMessage()`, which returns a string in the form "Running 1 test from 2 fixtures in 3 suites"
- Line 275-293: We implement a static function `TestRunFinishMessage()`, which returns a string in the form "1 test from 2 fixtures in 3 suites ran."
- Line 295-316: We implement a static function `TestRunSummaryMessage()`, which returns a string in the form "FAILURE: 1 out of 2 tests failed (3 failures).\n" in case of failures,
and "Success: 2 tests passed.\n" otherwise
- Line 318-331: We implement a static function `TestRunOverviewMessage()`, which returns a string in the form "Failures: 2" in case of failures, and "No failures" otherwise
- Line 333-342: We implement a static function `TestFailureMessage()`, which returns a string in the form "Suite1\:\:Fixture2\:\:Test3 failure failureText"
- Line 344-358: We implement a static function `TestSuiteStartMessage()`, which returns a string in the form "Suite1 (2 fixtures)"
- Line 360-374: We implement a static function `TestSuiteFinishMessage()`, which returns a string in the form "2 fixtures from Suite1"
- Line 376-390: We implement a static function `TestFixtureStartMessage()`, which returns a string in the form "Fixture1 (2 tests)"
- Line 392-406: We implement a static function `TestFixtureFinishMessage()`, which returns a string in the form "2 tests from Fixture1"
- Line 408-417: We implement a static function `TestFinishMessage()`, which returns a string containing the fully qualified test name

### unittest.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_UNITTESTH}

As we now need to include quite a few header files to set up and run our tests, let create a single include header that includes everything needed.

Create the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
45: #include "unittest/TestFixture.h"
46: #include "unittest/TestSuite.h"
47: 
48: #include "unittest/ConsoleTestReporter.h"
49: #include "unittest/CurrentTest.h"
50: #include "unittest/ITestReporter.h"
51: #include "unittest/Test.h"
52: #include "unittest/TestDetails.h"
53: #include "unittest/TestFixtureInfo.h"
54: #include "unittest/TestInfo.h"
55: #include "unittest/TestRegistry.h"
56: #include "unittest/TestResults.h"
57: #include "unittest/TestRunner.h"
58: #include "unittest/TestSuiteInfo.h"
```

### Application code {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_APPLICATION_CODE}

Now that we have added a test runner and a test report, we can use these to make running the tests even simpler.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include "baremetal/Console.h"
2: #include "baremetal/Logger.h"
3: #include "baremetal/System.h"
4: #include "baremetal/Timer.h"
5: 
6: #include "unittest/unittest.h"
7: 
8: LOG_MODULE("main");
9: 
10: using namespace baremetal;
11: using namespace unittest;
12: 
13: namespace Suite1 {
14: 
15: inline char const* GetSuiteName()
16: {
17:     return "Suite1";
18: }
19: 
20: class FixtureMyTest1
21:     : public TestFixture
22: {
23: public:
24:     void SetUp() override
25:     {
26:         LOG_DEBUG("MyTest SetUp");
27:     }
28:     void TearDown() override
29:     {
30:         LOG_DEBUG("MyTest TearDown");
31:     }
32: };
33: 
34: class FixtureMyTest1Helper
35:     : public FixtureMyTest1
36: {
37: public:
38:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
39:     explicit FixtureMyTest1Helper(const TestDetails& details)
40:         : m_details{ details }
41:     {
42:         SetUp();
43:     }
44:     virtual ~FixtureMyTest1Helper()
45:     {
46:         TearDown();
47:     }
48:     void RunImpl() const;
49:     const TestDetails& m_details;
50: };
51: void FixtureMyTest1Helper::RunImpl() const
52: {
53:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 1");
54: }
55: 
56: class MyTest1
57:     : public Test
58: {
59:     void RunImpl() const override;
60: } myTest1;
61: 
62: TestRegistrar registrarFixtureMyTest1(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__));
63: 
64: void MyTest1::RunImpl() const
65: {
66:     LOG_DEBUG("Test 1");
67:     FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
68:     fixtureHelper.RunImpl();
69: }
70: 
71: } // namespace Suite1
72: 
73: namespace Suite2 {
74: 
75: inline char const* GetSuiteName()
76: {
77:     return "Suite2";
78: }
79: 
80: class FixtureMyTest2
81:     : public TestFixture
82: {
83: public:
84:     void SetUp() override
85:     {
86:         LOG_DEBUG("FixtureMyTest2 SetUp");
87:     }
88:     void TearDown() override
89:     {
90:         LOG_DEBUG("FixtureMyTest2 TearDown");
91:     }
92: };
93: 
94: class FixtureMyTest2Helper
95:     : public FixtureMyTest2
96: {
97: public:
98:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
99:     explicit FixtureMyTest2Helper(const TestDetails& details)
100:         : m_details{ details }
101:     {
102:         SetUp();
103:     }
104:     virtual ~FixtureMyTest2Helper()
105:     {
106:         TearDown();
107:     }
108:     void RunImpl() const;
109:     const TestDetails& m_details;
110: };
111: void FixtureMyTest2Helper::RunImpl() const
112: {
113:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 2");
114: }
115: 
116: class MyTest2
117:     : public Test
118: {
119:     void RunImpl() const override;
120: } myTest1;
121: 
122: TestRegistrar registrarFixtureMyTest2(TestRegistry::GetTestRegistry(), &myTest1, TestDetails("MyTest2", "FixtureMyTest2", GetSuiteName(), __FILE__, __LINE__));
123: 
124: void MyTest2::RunImpl() const
125: {
126:     LOG_DEBUG("Test 2");
127:     FixtureMyTest2Helper fixtureHelper(*CurrentTest::Details());
128:     fixtureHelper.RunImpl();
129: }
130: 
131: } // namespace Suite2
132: 
133: class FixtureMyTest3
134:     : public TestFixture
135: {
136: public:
137:     void SetUp() override
138:     {
139:         LOG_DEBUG("FixtureMyTest3 SetUp");
140:     }
141:     void TearDown() override
142:     {
143:         LOG_DEBUG("FixtureMyTest3 TearDown");
144:     }
145: };
146: 
147: class FixtureMyTest3Helper
148:     : public FixtureMyTest3
149: {
150: public:
151:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
152:     explicit FixtureMyTest3Helper(const TestDetails& details)
153:         : m_details{ details }
154:     {
155:         SetUp();
156:     }
157:     virtual ~FixtureMyTest3Helper()
158:     {
159:         TearDown();
160:     }
161:     void RunImpl() const;
162:     const TestDetails& m_details;
163: };
164: void FixtureMyTest3Helper::RunImpl() const
165: {
166:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 3");
167: }
168: 
169: class MyTest3
170:     : public Test
171: {
172:     void RunImpl() const override;
173: } myTest3;
174: 
175: TestRegistrar registrarFixtureMyTest3(TestRegistry::GetTestRegistry(), &myTest3, TestDetails("MyTest3", "FixtureMyTest3", GetSuiteName(), __FILE__, __LINE__));
176: 
177: void MyTest3::RunImpl() const
178: {
179:     LOG_DEBUG("Test 3");
180:     FixtureMyTest3Helper fixtureHelper(*CurrentTest::Details());
181:     fixtureHelper.RunImpl();
182: }
183: 
184: class MyTest
185:     : public Test
186: {
187: public:
188:     void RunImpl() const override;
189: } myTest;
190: 
191: TestRegistrar registrarMyTest(TestRegistry::GetTestRegistry(), &myTest, TestDetails("MyTest", "", "", __FILE__, __LINE__));
192: 
193: void MyTest::RunImpl() const
194: {
195:     LOG_DEBUG("Running test");
196: }
197: 
198: int main()
199: {
200:     auto& console = GetConsole();
201: 
202:     ConsoleTestReporter reporter;
203:     RunAllTests(&reporter);
204: 
205:     LOG_INFO("Wait 5 seconds");
206:     Timer::WaitMilliSeconds(5000);
207: 
208:     console.Write("Press r to reboot, h to halt\n");
209:     char ch{};
210:     while ((ch != 'r') && (ch != 'h'))
211:     {
212:         ch = console.ReadChar();
213:         console.WriteChar(ch);
214:     }
215: 
216:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
217: }
```

- Line 6: We replace all unit test includes by a single one
- Line 196: We inject a failure to get a failing result
- Line 203: We instantiate a test reporter throug the `ConsoleTestReporter` class
- Line 204: We run all tests using the test reporter using the `RunAllTests()` function

### Configuring, building and debugging {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_TEST_RUNNER_AND_VISITOR___STEP_7_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

You will see all the output being generared by the console test reported using colors.

<img src="images/demo-output-unit-test.png" alt="Tree view" width="800"/>

## Collecting test results - Step 8 {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8}

So far, we can run tests, but the output is written immediately as we run. A better way would be to collect information and print it after the test run has completed.
That way the test run output itself will be more clean, and we have a simple summary of all failures.
For this, we'll introduce the `DeferredTestReporter` class, and make `ConsoleTestReporter` inherit from this.

### TestResult.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8_TESTRESULTH}

The `DeferredTestReporter` will use a list of entries holding a `TestResult` to keep track of the results during the test run.
The `TestResult` class will hold the information and result for a single test, which is extracted initially from `TestDetails`.
If a test failure occures, the failure information is attached to the `TestResult`.
We need to declare this class.

Create the file `code/libraries/unittest/include/unittest/TestResult.h`

```cpp
File: code/libraries/unittest/include/unittest/TestResult.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "baremetal/String.h"
43: #include "unittest/TestDetails.h"
44: 
45: /// @file
46: /// Test result
47: ///
48: /// Result of a single test
49: 
50: namespace unittest {
51: 
52: /// <summary>
53: /// Failure
54: ///
55: /// Holds information on an occurrred test failure
56: /// </summary>
57: class Failure
58: {
59: private:
60:     /// @brief Line number on which failure occurred
61:     int m_lineNumber;
62:     /// @brief Failure message
63:     baremetal::String m_text;
64: 
65: public:
66:     /// <summary>
67:     /// Constructor
68:     /// </summary>
69:     /// <param name="lineNumber">Line number on which failure occurred</param>
70:     /// <param name="text">Failure message, can be empty</param>
71:     Failure(int lineNumber, const baremetal::String& text);
72:     /// <summary>
73:     /// Return line number on which failure occurred
74:     /// </summary>
75:     /// <returns>Line number on which failure occurred</returns>
76:     int SourceLineNumber() const
77:     {
78:         return m_lineNumber;
79:     }
80:     /// <summary>
81:     /// Returns failure message
82:     /// </summary>
83:     /// <returns>Failure message</returns>
84:     const baremetal::String& Text() const
85:     {
86:         return m_text;
87:     }
88: };
89: 
90: /// <summary>
91: /// Container for failure
92: /// </summary>
93: class FailureEntry
94: {
95: private:
96:     friend class FailureList;
97:     /// @brief Failure information
98:     Failure m_failure;
99:     /// @brief Pointer to next failure entry in the list
100:     FailureEntry* m_next;
101: 
102: public:
103:     /// <summary>
104:     /// Constructor
105:     /// </summary>
106:     /// <param name="failure">Failure information to set</param>
107:     explicit FailureEntry(const Failure& failure);
108:     /// <summary>
109:     /// Returns failure information
110:     /// </summary>
111:     /// <returns>Failure information</returns>
112:     const Failure& GetFailure() const
113:     {
114:         return m_failure;
115:     }
116:     /// <summary>
117:     /// Return pointer to next failure entry
118:     /// </summary>
119:     /// <returns>Pointer to next failure entry</returns>
120:     const FailureEntry* GetNext() const
121:     {
122:         return m_next;
123:     }
124: };
125: 
126: /// <summary>
127: /// List of failures
128: /// </summary>
129: class FailureList
130: {
131: private:
132:     /// @brief Pointer to first failure entry in the list
133:     FailureEntry* m_head;
134:     /// @brief Pointer to last failure entry in the list
135:     FailureEntry* m_tail;
136: 
137: public:
138:     /// <summary>
139:     /// Constructor
140:     /// </summary>
141:     FailureList();
142:     /// <summary>
143:     /// Destructor
144:     /// </summary>
145:     ~FailureList();
146: 
147:     /// <summary>
148:     /// Returns pointer to first failure in the list
149:     /// </summary>
150:     /// <returns>Pointer to first failure in the list</returns>
151:     const FailureEntry* GetHead() const
152:     {
153:         return m_head;
154:     }
155:     /// <summary>
156:     /// Add a failure to the list
157:     /// </summary>
158:     /// <param name="failure">Failure information to add</param>
159:     void Add(const Failure& failure);
160: };
161: 
162: /// <summary>
163: /// Results for a single test
164: /// </summary>
165: class TestResult
166: {
167: private:
168:     /// @brief Details of the test
169:     TestDetails m_details;
170:     /// @brief List of failure for the test
171:     FailureList m_failures;
172:     /// @brief Failure flag, true if at least one failure occurred
173:     bool m_failed;
174: 
175: public:
176:     TestResult() = delete;
177:     /// <summary>
178:     /// Constructor
179:     /// </summary>
180:     /// <param name="details">Test details</param>
181:     explicit TestResult(const TestDetails& details);
182: 
183:     /// <summary>
184:     /// Add a failure to the list for this test
185:     /// </summary>
186:     /// <param name="failure">Failure information</param>
187:     void AddFailure(const Failure& failure);
188:     /// <summary>
189:     /// Return the list of failures for this test
190:     /// </summary>
191:     /// <returns>List of failures for this test</returns>
192:     const FailureList& Failures() const
193:     {
194:         return m_failures;
195:     }
196:     /// <summary>
197:     /// Returns failure flag
198:     /// </summary>
199:     /// <returns>Failure flag, true if at least one failure occurred</returns>
200:     bool Failed() const
201:     {
202:         return m_failed;
203:     }
204:     /// <summary>
205:     /// Returns the test details
206:     /// </summary>
207:     /// <returns>Test details</returns>
208:     const TestDetails& Details() const
209:     {
210:         return m_details;
211:     }
212: };
213: 
214: } // namespace unittest
```

- Line 52-88: We declare a class `Failure` to hold a single failure
  - Line 60-61: The member variable `m_lineNumber` holds the line number in the source file where the failure occurred
  - Line 62-63: The member variable `m_text` holds the failure message
  - Line 66-71: We declare the constructor
  - Line 72-79: We declare and define the method `SourceLineNumber()` which returns the line number
  - Line 80-87: We declare and define the method `Text()` which returns the failure message
- Line 90-124: We declare a class `FailureEntry` which holds a failure, and a pointer to the next failure
  - Line 97-98: The member variable `m_failure` holds the failure
  - Line 99-100: The member variable `m_next` holds a pointer to the next `FailureEntry` in the list.
I.e. we form a linked list
  - Line 103-107: We declare the constructor
  - Line 108-115: We declare and define the method `GetFailure()` which returns a const reference to the failure
  - Line 116-123: We declare and define the method `GetNext()` which returns a const pointer to the next `FailureEntry` in the list.
In order for the list to be built the class doing this will therefore need to be a friend class, see Line 96
- Line 126-160: We declare a class `FailureList` which holds a pointer to the beginning and the end of a `FailureEntry` linked list
  - Line 132-135: The member variables `m_head` and `m_tail` hold a pointer to the beginning and the end of the list, respectively
  - Line 138-141: We declare the constructor
  - Line 142-145: We declare the destructor, which will clean up the list of `FailureEntry` instances
  - Line 147-154: We declare and define the method `GetHead()` which returns a const pointer to the first `FailureEntry` in the list
  - Line 155-159: We declare and define the method `Add()` which adds a failure to the list (embedded in a `FailureEntry` instance)
- Line 162-212: We declare the class `TestResult`
  - Line 168-169: The member variable `m_details` holds the test details
  - Line 170-171: The member variable `m_failures` holds the failure list
  - Line 172-173: The member variable `m_failed` holds true if at least one failure occurred for this test
  - Line 176: We remove the default constructor
  - Line 177-181: We declare an explicit constructor
  - Line 183-187: We declare the method `AddFailure()` which adds a failure to the list
  - Line 188-195: We declare and define the method `Failures()` which returns a const reference to the failure list
  - Line 196-203: We declare and define the method `Failed()` which returns true if a failure occurred
  - Line 204-211: We declare and define the method `Details()` which returns a const reference to the test details

### TestResult.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8_TESTRESULTCPP}

Let's implement the `TestResult` class.

Create the file `code/libraries/unittest/src/TestResult.cpp`

```cpp
File: code/libraries/unittest/src/TestResult.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/TestResult.h"
41: 
42: #include "unittest/TestDetails.h"
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
53: Failure::Failure(int lineNumber, const baremetal::String& text)
54:     : m_lineNumber{lineNumber}
55:     , m_text{text}
56: {
57: }
58: 
59: FailureEntry::FailureEntry(const Failure& failure)
60:     : m_failure{failure}
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
100:     : m_details{details}
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

### DeferredTestReporter.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8_DEFERREDTESTREPORTERH}

Let's declare the `DeferredTestReporter` class.

Create the file `code/libraries/unittest/include/unittest/DeferredTestReporter.h`

```cpp
File: code/libraries/unittest/include/unittest/DeferredTestReporter.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "unittest/ITestReporter.h"
43: #include "unittest/TestResult.h"
44: 
45: /// @file
46: /// Deferred test reporter
47: ///
48: /// Saves failures during the test run, so they can be sown in the overview after the complete test run
49: 
50: namespace unittest {
51: 
52: /// <summary>
53: /// Test result entry
54: /// </summary>
55: class ResultEntry
56: {
57: private:
58:     friend class ResultList;
59:     /// @brief Test result
60:     TestResult m_result;
61:     /// @brief Pointer to next entry in list
62:     ResultEntry* m_next;
63: 
64: public:
65:     explicit ResultEntry(const TestResult& result);
66:     /// <summary>
67:     /// Return test result
68:     /// </summary>
69:     /// <returns>Test result</returns>
70:     TestResult& GetResult()
71:     {
72:         return m_result;
73:     }
74:     /// <summary>
75:     /// Return next entry pointer
76:     /// </summary>
77:     /// <returns>Next entry pointer</returns>
78:     ResultEntry* GetNext()
79:     {
80:         return m_next;
81:     }
82: };
83: 
84: /// <summary>
85: /// Test result entry list
86: /// </summary>
87: class ResultList
88: {
89: private:
90:     /// @brief Start of list
91:     ResultEntry* m_head;
92:     /// @brief End of list
93:     ResultEntry* m_tail;
94: 
95: public:
96:     ResultList();
97:     ~ResultList();
98: 
99:     void Add(const TestResult& result);
100:     /// <summary>
101:     /// Return start of list pointer
102:     /// </summary>
103:     /// <returns>Start of list pointer</returns>
104:     ResultEntry* GetHead() const
105:     {
106:         return m_head;
107:     }
108:     /// <summary>
109:     /// Return end of list pointer
110:     /// </summary>
111:     /// <returns>End of list pointer</returns>
112:     ResultEntry* GetTail() const
113:     {
114:         return m_tail;
115:     }
116: };
117: 
118: /// <summary>
119: /// Deferred test reporter
120: ///
121: /// Implements abstract ITestReporter interface
122: /// </summary>
123: class DeferredTestReporter : public ITestReporter
124: {
125: private:
126:     /// @brief Test result list for current test run
127:     ResultList m_results;
128: 
129: public:
130:     void ReportTestRunStart(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
131:     void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests) override;
132:     void ReportTestRunSummary(const TestResults& results) override;
133:     void ReportTestRunOverview(const TestResults& results) override;
134:     void ReportTestSuiteStart(const baremetal::String& suiteName, int numberOfTestFixtures) override;
135:     void ReportTestSuiteFinish(const baremetal::String& suiteName, int numberOfTestFixtures) override;
136:     void ReportTestFixtureStart(const baremetal::String& fixtureName, int numberOfTests) override;
137:     void ReportTestFixtureFinish(const baremetal::String& fixtureName, int numberOfTests) override;
138:     void ReportTestStart(const TestDetails& details) override;
139:     void ReportTestFinish(const TestDetails& details, bool success) override;
140:     void ReportTestFailure(const TestDetails& details, const baremetal::String& failure) override;
141: 
142:     ResultList& Results();
143: };
144: 
145: } // namespace unittest
```

- Line 52-82: We declare the struct `ResultEntry`, which holds a `TestResult`, and a pointer to the next `ResultEntry`.
The results entries again form a linked list, and are used to gather results for each test
  - Line 58: As the class `ResultList` is used to build the linked list of `ResultEntry` instances, we make it a friend class
  - Line 59-60: The member variable `m_result` holds the test result
  - Line 61-62: The member variable `m_next` holds the pointer to the next `ResultEntry` in the list
  - Line 65: We declare the (explicit) constructor
  - Line 66-73: We declare and define the method `GetResult()` which returns the test result
  - Line 74-81: We declare and define the method `GetNext()` which returns a pointer to the next `ResultEntry` in the list
- Line 84-116: We declare the class `ResultList` which forms the linked list and holds a pointer to the first and last `ResultEntry`
  - Line 90-93: The member variables `m_head` and `m_tail` hold a pointer to the beginning and the end of the list, respectively
  - Line 96: We declare the constructor
  - Line 97: We declare the destructor, which will clean up the list of `ResultEntry` instances
  - Line 99: We declare and define the method `Add()` which adds a test result to the list (embedded in a `ResultEntry` instance)
  - Line 100-107: We declare and define the method `GetHead()` which returns a const pointer to the first `ResultEntry` in the list
  - Line 108-115: We declare and define the method `GetTail()` which returns a const pointer to the last `ResultEntry` in the list
- Line 118-143: We declare the class `DeferredTestReporter`, which implements the abstract interface `ITestReporter`
  - Line 126-127: The class variable `m_results` holds the list of test results saved during the test run
  - Line 130-140: We implement the `ITestReporter` interface
  - Line 142: We declare the method `Results()` which returns the `ResultList`

### DeferredTestReporter.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8_DEFERREDTESTREPORTERCPP}

Let's implement the `DeferredTestReporter` class.

Create the file `code/libraries/unittest/src/DeferredTestReporter.cpp`

```cpp
File: code/libraries/unittest/src/DeferredTestReporter.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include "unittest/DeferredTestReporter.h"
41: 
42: #include "unittest/TestDetails.h"
43: 
44: /// @file
45: /// Deferred test reporter implementation
46: 
47: using namespace baremetal;
48: 
49: namespace unittest {
50: 
51: /// <summary>
52: /// Constructor
53: /// </summary>
54: /// <param name="result">Test result to be stored</param>
55: ResultEntry::ResultEntry(const TestResult& result)
56:     : m_result{result}
57:     , m_next{}
58: {
59: }
60: 
61: /// <summary>
62: /// Constructor
63: /// </summary>
64: ResultList::ResultList()
65:     : m_head{}
66:     , m_tail{}
67: {
68: }
69: 
70: /// <summary>
71: /// Destructor
72: /// </summary>
73: ResultList::~ResultList()
74: {
75:     auto current = m_head;
76:     while (current != nullptr)
77:     {
78:         auto next = current->m_next;
79:         delete current;
80:         current = next;
81:     }
82: }
83: 
84: /// <summary>
85: /// Add a test result to the list
86: /// </summary>
87: /// <param name="result">Test result to add</param>
88: void ResultList::Add(const TestResult& result)
89: {
90:     auto entry = new ResultEntry(result);
91:     if (m_head == nullptr)
92:     {
93:         m_head = entry;
94:     }
95:     else
96:     {
97:         auto current = m_head;
98:         while (current->m_next != nullptr)
99:             current = current->m_next;
100:         current->m_next = entry;
101:     }
102:     m_tail = entry;
103: }
104: 
105: /// <summary>
106: /// Start of test run callback (empty)
107: /// </summary>
108: /// <param name="numberOfTestSuites">Number of test suites to be run</param>
109: /// <param name="numberOfTestFixtures">Number of test fixtures to be run</param>
110: /// <param name="numberOfTests">Number of tests to be run</param>
111: void DeferredTestReporter::ReportTestRunStart(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
112: {
113: }
114: 
115: /// <summary>
116: /// Finish of test run callback (empty)
117: /// </summary>
118: /// <param name="numberOfTestSuites">Number of test suites run</param>
119: /// <param name="numberOfTestFixtures">Number of test fixtures run</param>
120: /// <param name="numberOfTests">Number of tests run</param>
121: void DeferredTestReporter::ReportTestRunFinish(int /*numberOfTestSuites*/, int /*numberOfTestFixtures*/, int /*numberOfTests*/)
122: {
123: }
124: 
125: /// <summary>
126: /// Test summary callback (empty)
127: /// </summary>
128: /// <param name="results">Test run results</param>
129: void DeferredTestReporter::ReportTestRunSummary(const TestResults& /*results*/)
130: {
131: }
132: 
133: /// <summary>
134: /// Test run overview callback (empty)
135: /// </summary>
136: /// <param name="results">Test run results</param>
137: void DeferredTestReporter::ReportTestRunOverview(const TestResults& /*results*/)
138: {
139: }
140: 
141: /// <summary>
142: /// Test suite start callback (empty)
143: /// </summary>
144: /// <param name="suiteName">Test suite name</param>
145: /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
146: void DeferredTestReporter::ReportTestSuiteStart(const String& /*suiteName*/, int /*numberOfTestFixtures*/)
147: {
148: }
149: 
150: /// <summary>
151: /// Test suite finish callback (empty)
152: /// </summary>
153: /// <param name="suiteName">Test suite name</param>
154: /// <param name="numberOfTestFixtures">Number of fixtures within test suite</param>
155: void DeferredTestReporter::ReportTestSuiteFinish(const String& /*suiteName*/, int /*numberOfTestFixtures*/)
156: {
157: }
158: 
159: /// <summary>
160: /// Test fixture start callback (empty)
161: /// </summary>
162: /// <param name="fixtureName">Test fixture name</param>
163: /// <param name="numberOfTests">Number of tests within test fixture</param>
164: void DeferredTestReporter::ReportTestFixtureStart(const String& /*fixtureName*/, int /*numberOfTests*/)
165: {
166: }
167: 
168: /// <summary>
169: /// Test fixture finish callback (empty)
170: /// </summary>
171: /// <param name="fixtureName">Test fixture name</param>
172: /// <param name="numberOfTests">Number of tests within test fixture</param>
173: void DeferredTestReporter::ReportTestFixtureFinish(const String& /*fixtureName*/, int /*numberOfTests*/)
174: {
175: }
176: 
177: /// <summary>
178: /// Test start callback
179: /// </summary>
180: /// <param name="details">Test details</param>
181: void DeferredTestReporter::ReportTestStart(const TestDetails& details)
182: {
183:     m_results.Add(TestResult(details));
184: }
185: 
186: /// <summary>
187: /// Test finish callback (empty)
188: /// </summary>
189: /// <param name="details">Test details</param>
190: /// <param name="success">Test result, true is successful, false is failed</param>
191: void DeferredTestReporter::ReportTestFinish(const TestDetails& /*details*/, bool /*success*/)
192: {
193: }
194: 
195: /// <summary>
196: /// Test failure callback
197: /// </summary>
198: /// <param name="details">Test details</param>
199: /// <param name="failure">Test failure message</param>
200: void DeferredTestReporter::ReportTestFailure(const TestDetails& details, const String& failure)
201: {
202:     TestResult& result = m_results.GetTail()->GetResult();
203:     result.AddFailure(Failure(details.SourceFileLineNumber(), failure));
204: }
205: 
206: /// <summary>
207: /// Return test result list
208: /// </summary>
209: /// <returns>Test result list</returns>
210: ResultList& DeferredTestReporter::Results()
211: {
212:     return m_results;
213: }
214: 
215: } // namespace unittest
```

- Line 51-59: We implement the `ResultEntry` constructor
- Line 61-68: We implement the `ResultList` constructor
- Line 70-82: We implement the `ResultList` denstructor
- Line 84-103: We implement the method `Add` for `ResultList`. This will create a new `ResultEntry` and insert it at the end of the list
- Line 105-113: We implement the method `ReportTestRunStart` for `DeferredTestReporter`.
This does nothing, as `DeferredTestReporter` does not report anything in itself. It simply stores test results
- Line 115-123: We implement the method `ReportTestRunFinish` for `DeferredTestReporter`. This again does nothing
- Line 125-131: We implement the method `ReportTestRunSummary` for `DeferredTestReporter`. This again does nothing
- Line 138-140: We implement the method `ReportTestRunOverview` for `DeferredTestReporter`. This again does nothing
- Line 141-148: We implement the method `ReportTestSuiteStart` for `DeferredTestReporter`. This again does nothing
- Line 150-157: We implement the method `ReportTestSuiteFinish` for `DeferredTestReporter`. This again does nothing
- Line 159-166: We implement the method `ReportTestFixtureStart` for `DeferredTestReporter`. This again does nothing
- Line 168-175: We implement the method `ReportTestFixtureFinish` for `DeferredTestReporter`. This again does nothing
- Line 177-183: We implement the method `ReportTestStart` for `DeferredTestReporter`. This adds a new result to the list to be filled for the test to be started
- Line 186-193: We implement the method `ReportTestFinish` for `DeferredTestReporter`. This again does nothing
- Line 195-204: We implement the method `ReportTestFailure` for `DeferredTestReporter`. This adds a failure to the list for the current result
- Line 206-213: We implement the method `Results` for `DeferredTestReporter`

### ConsoleTestReport.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8_CONSOLETESTREPORTH}

We need to update the class `ConsoleTestReporter` to derive from `DeferredTestReporter`.

Update the file  `code/libraries/unittest/include/unittest/ConsoleTestReport.h`

```cpp
File: code/libraries/unittest/include/unittest/ConsoleTestReport.h
...
42: #include "unittest/DeferredTestReporter.h"
43: 
44: /// @file
45: /// Console test reporter
46: ///
47: /// Prints test information to the console, using color.
48: /// Uses DeferredTestReporter to save failures so they can be printed after running the test, as an overview
49: 
50: namespace unittest {
51: 
52: /// <summary>
53: /// Console test reporter
54: /// </summary>
55: class ConsoleTestReporter : public DeferredTestReporter
56: {
...
90:     baremetal::String TestFailureMessage(const TestResult& result, const Failure& failure);
...
```

- Line 42: We need to include the header for `DeferredTestReporter` instead of for the interface
- Line 55: We inherit from `DeferredTestReporter`
- Line 90: We replace the method `TestFailureMessage()` with a version taking a `TestResult` and a `Failure`

### ConsoleTestReport.cpp {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8_CONSOLETESTREPORTCPP}

Let's update the implementation for `ConsoleTestReport`.

Update the file `code/libraries/unittest/src/ConsoleTestReport.cpp`

```cpp
File: code/libraries/unittest/src/ConsoleTestReport.cpp
...
178: /// <summary>
179: /// Test start callback
180: /// </summary>
181: /// <param name="details">Test details</param>
182: void ConsoleTestReporter::ReportTestStart(const TestDetails& details)
183: {
184:     DeferredTestReporter::ReportTestStart(details);
185: }
186: 
187: /// <summary>
188: /// Test finish callback
189: /// </summary>
190: /// <param name="details">Test details</param>
191: /// <param name="success">Test result, true is successful, false is failed</param>
192: void ConsoleTestReporter::ReportTestFinish(const TestDetails& details, bool success)
193: {
194:     DeferredTestReporter::ReportTestFinish(details, success);
195:     GetConsole().SetTerminalColor(success ? ConsoleColor::Green : ConsoleColor::Red);
196:     if (success)
197:         GetConsole().Write(TestSuccessSeparator);
198:     else
199:         GetConsole().Write(TestFailSeparator);
200:     GetConsole().ResetTerminalColor();
201: 
202:     GetConsole().Write(Format(" %s\n", TestFinishMessage(details, success).c_str()));
203: }
204: 
205: /// <summary>
206: /// Test failure callback
207: /// </summary>
208: /// <param name="details">Test details</param>
209: /// <param name="failure">Test failure message</param>
210: void ConsoleTestReporter::ReportTestFailure(const TestDetails& details, const String& failure)
211: {
212:     DeferredTestReporter::ReportTestFailure(details, failure);
213: }
...
318: /// <summary>
319: /// Create a message for test run overview
320: /// </summary>
321: /// <param name="results">Test run results</param>
322: /// <returns>Resulting message</returns>
323: String ConsoleTestReporter::TestRunOverviewMessage(const TestResults& results)
324: {
325:     if (results.GetFailureCount() > 0)
326:     {
327:         String result = "Failures:\n";
328:         auto testResultPtr = Results().GetHead();
329:         while (testResultPtr != nullptr)
330:         {
331:             auto const& testResult = testResultPtr->GetResult();
332:             if (testResult.Failed())
333:             {
334:                 auto failuresPtr = testResult.Failures().GetHead();
335:                 while (failuresPtr != nullptr)
336:                 {
337:                     result.append(TestFailureMessage(testResult, failuresPtr->GetFailure()));
338:                     failuresPtr = failuresPtr->GetNext();
339:                 }
340:             }
341:             testResultPtr = testResultPtr->GetNext();
342:         }
343:         return result;
344:     }
345:     return "No failures";
346: }
347: 
348: /// <summary>
349: /// Create a message for test failure
350: /// </summary>
351: /// <param name="result">Test run results</param>
352: /// <param name="failure">Failure that occurred</param>
353: /// <returns>Resulting message</returns>
354: String ConsoleTestReporter::TestFailureMessage(const TestResult& result, const Failure& failure)
355: {
356:     return Format("%s:%d : Failure in %s: %s\n", result.Details().SourceFileName().c_str(), failure.SourceLineNumber(),
357:                   result.Details().QualifiedTestName().c_str(), failure.Text().c_str());
358: }
...
```

- Line 178-185: We reimplement the method `ReportTestStart` by calling the same method in `DeferredTestReporter`
- Line 187-203: We reimplement the method `ReportTestFinish` by first calling the same method in `DeferredTestReporter`
- Line 205-213: We reimplement the method `ReportTestFailure` by calling the same method in `DeferredTestReporter`
- Line 318-346: We reimplement the method `TestRunOverviewMessage` by going through the list of test results, and for any that have failed, going through the list of failures, and appending a test failure message
- Line 348-358: We reimplement the method `TestFailureMessage` to print a correct failure message

### unittest.h {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8_UNITTESTH}

We've added a header, so let's include that in the common header as well.

Update the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
45: #include "unittest/TestFixture.h"
46: #include "unittest/TestSuite.h"
47: 
48: #include "unittest/ConsoleTestReporter.h"
49: #include "unittest/CurrentTest.h"
50: #include "unittest/DeferredTestReporter.h"
51: #include "unittest/ITestReporter.h"
52: #include "unittest/Test.h"
53: #include "unittest/TestDetails.h"
54: #include "unittest/TestFixtureInfo.h"
55: #include "unittest/TestInfo.h"
56: #include "unittest/TestRegistry.h"
57: #include "unittest/TestResults.h"
58: #include "unittest/TestRunner.h"
59: #include "unittest/TestSuiteInfo.h"
```

### Application code {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8_APPLICATION_CODE}

We will keep the application code unchanged for now.

### Configuring, building and debugging {#TUTORIAL_23_UNIT_TEST_INFRASTRUCTURE_COLLECTING_TEST_RESULTS___STEP_8_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. The reporting will be slightly different, as we now will see a summary of all failures.
The debug logging will still show, but normally test cases do not log.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
Debug  0.00:00:00.030 Test 1 (main:66)
Debug  0.00:00:00.030 MyTest SetUp (main:26)
Debug  0.00:00:00.030 Suite1::FixtureMyTest1::MyTest1MyTestHelper 1 (main:53)
Debug  0.00:00:00.040 MyTest TearDown (main:30)
[ SUCCEEDED ] Suite1::FixtureMyTest1::MyTest1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
Debug  0.00:00:00.050 Test 2 (main:126)
Debug  0.00:00:00.060 FixtureMyTest2 SetUp (main:86)
Debug  0.00:00:00.060 Suite2::FixtureMyTest2::MyTest2MyTestHelper 2 (main:113)
Debug  0.00:00:00.060 FixtureMyTest2 TearDown (main:90)
[ SUCCEEDED ] Suite2::FixtureMyTest2::MyTest2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
Debug  0.00:00:00.080 Test 3 (main:179)
Debug  0.00:00:00.080 FixtureMyTest3 SetUp (main:139)
Debug  0.00:00:00.080 DefaultSuite::FixtureMyTest3::MyTest3MyTestHelper 3 (main:166)
Debug  0.00:00:00.090 FixtureMyTest3 TearDown (main:143)
[ SUCCEEDED ] DefaultSuite::FixtureMyTest3::MyTest3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
Debug  0.00:00:00.100 Running test (main:195)
[  FAILED   ] DefaultSuite::DefaultFixture::MyTest
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 1 out of 4 tests failed (1 failure).

Failures:
D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:191 : Failure in DefaultSuite::DefaultFixture::MyTest: Failure message example

[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   0.00:00:00.120 Wait 5 seconds (main:207)
Press r to reboot, h to halt
```

In the next tutorial we'll start replacing the very verbose code for setting up tests, test fixtures and test suites with macros.
We'll also introduce macros for the actual test cases, and use the example of the `MemoryAccessStubMCP23017LEDs` see [the tutorial](#TUTORIAL_22_I2C_FAKING_MCP23017___STEP_3) to show how tests can be written.

Next: [24-writing-unit-tests](24-writing-unit-tests.md)


