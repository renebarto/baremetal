# Tutorial 17: Unit tests {#TUTORIAL_17_UNIT_TESTS}

@tableofcontents

## New tutorial setup {#TUTORIAL_17_UNIT_TESTS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/17-unit-tests`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_17_UNIT_TESTS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-17.a`
- an application `output/Debug/bin/17-unit-tests.elf`
- an image in `deploy/Debug/17-unit-tests-image`

## Creating a framework for unit testing {#TUTORIAL_17_UNIT_TESTS_CREATING_A_FRAMEWORK_FOR_UNIT_TESTING}

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

## Creating the unittest library - Step 1 {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1}

We'll place the unit testing functionality in a new library, named `unittest`. 
The structure is similar to what we set up for `baremetal` (see [Creating the baremetal library structure - Step 1](#TUTORIAL_05_FIRST_APPLICATION__USING_THE_CONSOLE__UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE__STEP_1))

<img src="images/treeview-unittest-library.png" alt="Tree view" width="300"/>

### CMake file for unittest {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_CMAKE_FILE_FOR_UNITTEST}

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

### Update libraries CMake file {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_UPDATE_LIBRARIES_CMAKE_FILE}

First we need to include the unittest project in the libraries CMake file.
Update the file `code/libraries/CMakeLists.txt`

```cmake
message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(baremetal)
add_subdirectory(unittest)
```

### TestDetails.h {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTDETAILSH}

First we'll add a class to describe a test. This will hold its test suite name, test fixture name, test name, source file, and line number.
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
44: namespace unittest
45: {
46: 
47: class TestDetails
48: {
49: private:
50:     const baremetal::string m_suiteName;
51:     const baremetal::string m_fixtureName;
52:     const baremetal::string m_testName;
53:     const baremetal::string m_fileName;
54:     const int m_lineNumber;
55: 
56: public:
57:     TestDetails();
58:     TestDetails(const baremetal::string& testName, const baremetal::string& fixtureName, const baremetal::string& suiteName, const baremetal::string& fileName, int lineNumber);
59:     TestDetails(const TestDetails&) = delete;
60:     TestDetails(TestDetails&&) = delete;
61: 
62:     TestDetails& operator = (const TestDetails&) = delete;
63:     TestDetails& operator = (TestDetails&&) = delete;
64: 
65:     const baremetal::string& SuiteName() const { return m_suiteName; }
66:     const baremetal::string& FixtureName() const { return m_fixtureName; }
67:     const baremetal::string& TestName() const { return m_testName; }
68:     const baremetal::string& SourceFileName() const { return m_fileName; }
69:     int SourceFileLineNumber() const { return m_lineNumber; }
70: };
71: 
72: } // namespace unittest
```

The `TestDetails` class is added to the `unittest` namespace.

- Line 42: We use strings, so we need to include the header for the `string` class
- Line 47-72: We declare the class `TestDetails` which will hold information on a test
  - Line 50: The class member variable `m_suiteName` is the test suite name
  - Line 51: The class member variable `m_fixtureName` is the test fixture name
  - Line 52: The class member variable `m_testName` is the test name
  - Line 53: The class member variable `m_fileName` is the source file in which the actual test is defined
  - Line 54: The class member variable `m_lineNumber` is the source line in which the actual test is defined
  - Line 57: We declare the default constructor
  - Line 58: We declare the normal constructor which specifies al the needed information
  - Line 59-60: We remove the copy and move constructors
  - Line 62-63: We remove the assignment and move assignment operators
  - Line 65: We declare an accessor `SuiteName()` for the test suite name
  - Line 66: We declare an accessor `FixtureName()` for the test fixture name
  - Line 67: We declare an accessor `TestName()` for the test name
  - Line 68: We declare an accessor `SourceFileName()` for the source file name
  - Line 69: We declare an accessor `SourceFileLineNumber()` for the source line number

### TestDetails.cpp {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTDETAILSCPP}

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
42: using namespace baremetal;
43: 
44: namespace unittest {
45: 
46: TestDetails::TestDetails()
47:     : m_suiteName{}
48:     , m_fixtureName{}
49:     , m_testName{}
50:     , m_fileName{}
51:     , m_lineNumber{}
52: {
53: }
54: 
55: TestDetails::TestDetails(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
56:     : m_suiteName{ suiteName }
57:     , m_fixtureName{ fixtureName }
58:     , m_testName{ testName }
59:     , m_fileName{ fileName }
60:     , m_lineNumber{ lineNumber }
61: {
62: }
63: 
64: } // namespace unittest
```

- Line 46-53: We implement the default constructor
- Line 55-62: We implement the non default constructor

### TestBase.h {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTBASEH}

We will add a base class for each test. All tests will derive from this class, and implement its `RunImpl()` method to run the actual test.
Create the file `code/libraries/unittest/include/unittest/TestBase.h`

```cpp
File: code/libraries/unittest/include/unittest/TestBase.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestBase.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestBase
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
44: namespace unittest
45: {
46: 
47: class TestBase
48: {
49: private:
50:     TestDetails const m_details;
51:     TestBase* m_next;
52: 
53: public:
54:     TestBase();
55:     TestBase(const TestBase&) = delete;
56:     TestBase(TestBase&&) = delete;
57:     explicit TestBase(
58:         const baremetal::string& testName,
59:         const baremetal::string& fixtureName = {},
60:         const baremetal::string& suiteName = {},
61:         const baremetal::string& fileName = {},
62:         int lineNumber = {});
63:     virtual ~TestBase();
64: 
65:     TestBase& operator = (const TestBase&) = delete;
66:     TestBase& operator = (TestBase&&) = delete;
67: 
68:     const TestDetails& Details() const { return m_details; }
69: 
70:     void Run();
71: 
72:     virtual void RunImpl() const;
73: };
74: 
75: } // namespace unittest
```

The `TestBase` class is added to the `unittest` namespace.

- Line 50: We declare the details for the test
- Line 51: We declare a pointer to the next test. Tests will be stored in a linked list
- Line 54: We declare a default constructor
- Line 55-56: We remove the copy constructor and move constructor
- Line 57: We declare an explicit constructor
- Line 63: We declare the destructor. This may be important as we will be inheriting from this class
- Line 65-66: We remove the assignment operators
- Line 68: We declare methods to retrieve details
- Line 70: We declare a method `Run()` to run the test.
This will ultimately invoke the `RunImpl()` virtual method, which is expected to be overriden by an actual test.
- Line 72: We declare the overridable `RunImpl()` method

### TestBase.cpp {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTBASECPP}

We'll implement the `TestBase` class.
Create the file `code/libraries/unittest/src/TestBase.cpp`

```cpp
File: code/libraries/unittest/src/TestBase.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : TestBase.cpp
5: //
6: // Namespace   : unittest
7: //
8: // Class       : TestBase
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
File: d:\Projects\baremetal.github\code\libraries\unittest\src\TestBase.cpp
40: #include <unittest/TestBase.h>
41: 
42: using namespace baremetal;
43: 
44: namespace unittest {
45: 
46: TestBase::TestBase()
47:     : m_details{}
48:     , m_next{}
49: {
50: }
51: 
52: TestBase::TestBase(const string& testName, const string& fixtureName, const string& suiteName, const string& fileName, int lineNumber)
53:     : m_details{ testName, fixtureName, suiteName, fileName, lineNumber }
54:     , m_next{}
55: {
56: }
57: 
58: TestBase::~TestBase()
59: {
60: }
61: 
62: void TestBase::Run()
63: {
64:     RunImpl();
65: }
66: 
67: void TestBase::RunImpl() const
68: {
69: }
70: 
71: } // namespace unittest
```

- Line 46-50: We implement the default constructor
- Line 52-56: We implement the non default constructor
- Line 58-60: We implement the destructor
- Line 62-65: We provide a first implementation for the `Run()` method
- Line 67-69: We provide a default implementation for the `RunImpl()` method

### Update CMake file {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_UPDATE_CMAKE_FILE}

As we have now added some source files to the `unittest` library, we need to update its CMake file.
Update the file `code/libraries/unitttest/CMakeLists.txt`

```cmake
File: code/libraries/unitttest/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
33:     )
34: 
35: set(PROJECT_INCLUDES_PUBLIC
36:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
37:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
38:     )
39: set(PROJECT_INCLUDES_PRIVATE )
40: 
...
```
 
### Update application code {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_UPDATE_APPLICATION_CODE}

Let's start using the class we just created. We'll add a simple test case by declaring and implementing a class derived from `Testbase`.
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
18: 
19: LOG_MODULE("main");
20: 
21: using namespace baremetal;
22: using namespace unittest;
23: 
24: class MyTest
25:     : public TestBase
26: {
27: public:
28:     MyTest()
29:         : TestBase("MyTest", "", "", __FILE__, __LINE__)
30:     {
31: 
32:     }
33:     void RunImpl() const override
34:     {
35:         LOG_DEBUG("In RunImpl");
36:     }
37: };
38: 
39: int main()
40: {
41:     auto& console = GetConsole();
42:     LOG_DEBUG("Hello World!");
43: 
44:     MyTest test;
45:     test.Run();
46: 
47:     LOG_INFO("Wait 5 seconds");
48:     Timer::WaitMilliSeconds(5000);
49: 
50:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
51:     char ch{};
52:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
53:     {
54:         ch = console.ReadChar();
55:         console.WriteChar(ch);
56:     }
57:     if (ch == 'p')
58:         assert(false);
59: 
60:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
61: }
```

- Line 24-38: We declare and implement the class `MyTest` based on `TestBase`
  - Line 28-32: We declare and implement the constructor. We use the class name as the test name, and set the test fixture name and test suite name to an empty string. The file name and line number are taken from the actual source location
  - Line 33-37: We declare and implement an override for the `RunImpl()` method. It simply logs a string
- Line 45-46: We define an instance of MyTest, and then run the test.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

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
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Info   Starting up (System:201)
Debug  Hello World! (main:42)
Debug  In RunImpl (main:35)
Info   Wait 5 seconds (main:47)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Adding test fixtures - Step 2 {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1}

Now that we have a test we can run, let's add the test fixture, to hold multiple tests, and provide for a setup / teardown method call.

### TestFixtureInfo.h

### TestFixtureInfo.cpp

### TestFixture.h

### TestFixture.cpp

======================================================================================================

### CurrentTest.h {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_CURRENTTESTH}

The `CurrentTest.h` header is used to keep track of the current test, and the overall results.
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

Notice that we keep pointers here so we can update the information.

### ExecuteTest.h {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_EXECUTETESTH}

The `ExecuteTest.h` header simply declares a template function to run a test.
Create the file `code/libraries/unittest/include/unittest/ExecuteTest.h`

```cpp
File: code/libraries/unittest/include/unittest/ExecuteTest.h
File: d:\Projects\baremetal.github\code\libraries\unittest\include\unittest\ExecuteTest.h
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
44: #include <unittest/TestResults.h>
45: 
46: 
47: namespace unittest
48: {
49: 
50: template<typename T>
51: void ExecuteTest(T& testObject, const TestDetails& details)
52: {
53:     CurrentTest::Details() = &details;
54: 
55:     testObject.RunImpl();
56: }
57: 
58: } // namespace unittest
```

### TestResults.h {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTRESULTSH}

The `TestResults.h` header file declares the overall test results.
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
51: class TestFailure;
52: class TestResult;
53: 
54: class TestResults
55: {
56: public:
57:     explicit TestResults(ITestReporter* reporter = nullptr);
58:     TestResults(const TestResults&) = delete;
59:     TestResults(TestResults&&) = delete;
60:     virtual ~TestResults();
61: 
62:     TestResults& operator = (const TestResults&) = delete;
63:     TestResults& operator = (TestResults&&) = delete;
64: 
65:     void OnTestSuiteStart(TestSuiteInfo* suite);
66:     void OnTestSuiteFinish(TestSuiteInfo* suite, int milliSecondsElapsed);
67:     void OnTestFixtureStart(TestFixtureInfo* fixture);
68:     void OnTestFixtureFinish(TestFixtureInfo* fixture, int milliSecondsElapsed);
69:     void OnTestStart(TestDetails const&details);
70:     void OnTestFailure(TestDetails const&details, const baremetal::string& message);
71:     void OnTestFinish(TestDetails const&details, int milliSecondsElapsed);
72: 
73:     int GetTotalTestCount() const;
74:     int GetFailedTestCount() const;
75:     int GetFailureCount() const;
76: 
77: private:
78:     ITestReporter* m_reporter;
79:     int m_totalTestCount;
80:     int m_failedTestCount;
81:     int m_failureCount;
82:     bool m_currentTestFailed;
83: };
84: 
85: } // namespace unittest
```

The `TestResults` class keeps counts of all tests, and counts of failed tests.
It also holds a pointer to a visitor, the `ITestReporter` instance.

- Line 57: We declare an explicit constructor. As it has a default parameter, it will replace the default constructor
- Line 58-59: We remove the copy and move constructors
- Line 60: We declare a destructor. This is not strictly necessary
- Line 62-63: We also remove the assignment and move assignment operators
- Line 65-71: We declare methods that will call the visitor such that it can output information.
We have a call to start and stop a single test, and mark a failure.
We also have a call signalling the start and end of a test fixture or a test suite
- Line 73-75: We declare methods to return the total test count, the failed test count, and the failure test.
- A single test can have multiple failures

### CurrentTest.cpp {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_CURRENTTESTCPP}

Let's implement the `CurrentTest` class.
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
40: #include "unittest/CurrentTest.h"
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

### TestResults.cpp {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTRESULTSCPP}

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
68: void TestResults::OnTestSuiteFinish(TestSuiteInfo* suite, int milliSecondsElapsed)
69: {
70:     if (m_reporter)
71:         m_reporter->ReportTestSuiteFinish(suite->Name(), suite->CountFixtures(), milliSecondsElapsed);
72: }
73: 
74: void TestResults::OnTestFixtureStart(TestFixtureInfo* fixture)
75: {
76:     if (m_reporter)
77:         m_reporter->ReportTestFixtureStart(fixture->Name(), fixture->CountTests());
78: }
79: 
80: void TestResults::OnTestFixtureFinish(TestFixtureInfo* fixture, int milliSecondsElapsed)
81: {
82:     if (m_reporter)
83:         m_reporter->ReportTestFixtureFinish(fixture->Name(), fixture->CountTests(), milliSecondsElapsed);
84: }
85: 
86: void TestResults::OnTestStart(TestDetails const& details)
87: {
88:     ++m_totalTestCount;
89:     m_currentTestFailed = false;
90:     if (m_reporter)
91:         m_reporter->ReportTestStart(details);
92: }
93: 
94: void TestResults::OnTestFailure(TestDetails const& details, const string& result)
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
106: void TestResults::OnTestFinish(TestDetails const& details, int milliSecondsElapsed)
107: {
108:     if (m_reporter)
109:         m_reporter->ReportTestFinish(details, !m_currentTestFailed, milliSecondsElapsed);
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

We have also used the classes `ITestReporter`, `TestSuiteInfo`, `TestFixtureInfo` and `TestFailure`.
We'll need to declare them as well.

### ITestReporter.h {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_ITESTREPORTERH}

The `ITestReporter` class is an abstraction of the visitor.
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
56:     virtual void ReportTestRunFinish(int numberOfTestSuites, int numberOfTestFixtures, int numberOfTests, int milliSecondsElapsed) = 0;
57:     virtual void ReportTestRunSummary(const TestResults* results, int milliSecondsElapsed) = 0;
58:     virtual void ReportTestRunOverview(const TestResults* results) = 0;
59:     virtual void ReportTestSuiteStart(const baremetal::string& suiteName, int numberOfTests) = 0;
60:     virtual void ReportTestSuiteFinish(const baremetal::string& suiteName, int numberOfTests, int milliSecondsElapsed) = 0;
61:     virtual void ReportTestFixtureStart(const baremetal::string& fixtureName, int numberOfTests) = 0;
62:     virtual void ReportTestFixtureFinish(const baremetal::string& fixtureName, int numberOfTests, int milliSecondsElapsed) = 0;
63:     virtual void ReportTestStart(TestDetails const& details) = 0;
64:     virtual void ReportTestFinish(TestDetails const& details, bool success, int milliSecondsElapsed) = 0;
65:     virtual void ReportTestFailure(TestDetails const& details, const baremetal::string& failure) = 0;
66: };
67: 
68: } // namespace unittest
69: 
```

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTSUITEINFOH}

The `TestSuiteInfo` class holds administation for a test suite.
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
48: class TestResults;
49: class TestFixtureInfo;
50: 
51: class TestSuiteInfo
52: {
53: public:
54:     friend class TestRegistry;
55:     friend class TestRegistrar;
56:     TestSuiteInfo* m_next;
57: private:
58:     TestFixtureInfo* m_head;
59:     TestFixtureInfo* m_tail;
60:     baremetal::string m_suiteName;
61: 
62:     void AddFixture(TestFixtureInfo* testFixture);
63:     TestFixtureInfo* GetTestFixture(const baremetal::string& fixtureName);
64: 
65: public:
66:     TestSuiteInfo() = delete;
67:     TestSuiteInfo(const TestSuiteInfo&) = delete;
68:     TestSuiteInfo(TestSuiteInfo&&) = delete;
69:     explicit TestSuiteInfo(const baremetal::string& suiteName);
70:     virtual ~TestSuiteInfo();
71: 
72:     TestSuiteInfo& operator = (const TestSuiteInfo&) = delete;
73:     TestSuiteInfo& operator = (TestSuiteInfo&&) = delete;
74: 
75:     TestFixtureInfo* GetHead() const;
76: 
77:     const baremetal::string& Name() const { return m_suiteName; }
78: 
79:     template <class Predicate> void RunIf(const Predicate& predicate, int maxTestTimeInMs, TestResults* testResults);
80: 
81:     int CountFixtures();
82:     int CountTests();
83:     template <typename Predicate> int CountFixturesIf(Predicate predicate);
84:     template <typename Predicate> int CountTestsIf(Predicate predicate);
85: };
86: 
87: template <class Predicate> void TestSuiteInfo::RunIf(const Predicate& predicate, int maxTestTimeInMs, TestResults* testResults)
88: {
89:     testResults->OnTestSuiteStart(this);
90: 
91:     TestFixtureInfo* testFixture = GetHead();
92:     while (testFixture != nullptr)
93:     {
94:         if (predicate(testFixture))
95:             testFixture->RunIf(predicate, maxTestTimeInMs, testResults);
96:         testFixture = testFixture->m_next;
97:     }
98: 
99:     testResults->OnTestSuiteFinish(this, 0/*testTimer.GetTimeInMilliSeconds()*/);
100: }
101: 
102: template <typename Predicate> int TestSuiteInfo::CountFixturesIf(Predicate predicate)
103: {
104:     int numberOfTestFixtures = 0;
105:     TestFixtureInfo* testFixture = m_head;
106:     while (testFixture != nullptr)
107:     {
108:         if (predicate(testFixture))
109:             numberOfTestFixtures++;
110:         testFixture = testFixture->m_next;
111:     }
112:     return numberOfTestFixtures;
113: }
114: 
115: template <typename Predicate> int TestSuiteInfo::CountTestsIf(Predicate predicate)
116: {
117:     int numberOfTests = 0;
118:     TestFixtureInfo* testFixture = m_head;
119:     while (testFixture != nullptr)
120:     {
121:         if (predicate(testFixture))
122:             numberOfTests += testFixture->CountTestsIf(predicate);
123:         testFixture = testFixture->m_next;
124:     }
125:     return numberOfTests;
126: }
127: 
128: } // namespace unittest
```

- Line 56: We keep a pointer to the next `TestSuiteInfo`, as the suites are organized as a linked list
- Line 58-59: We keep a pointer to the first and last `TestFixtureInfo` instance, so we can iterate over them
- Line 62: We declare a private method `AddFixture()` which is used by the registry to add fixtures at initialization time
- Line 63: We declare the method `GetTestFixture()` to find a specific fixture by name.
This is used by the registry. If no fixture can be found, a new one with the name is created
- Line 66-68: Remove the default, copy and move constructors
- Line 69: We instead only define an explicit constructor
- Line 70: We declare a destructor which is needed to clean up after ourselves. The `TestRegistry` will allocate `TestSuiteInfo`, `TestFixtureInfo` and `TestBase` instances at initialization time, we will clean them up in the destructor
- Line 72-73: We remove the assignment and move assignment operators
- Line 75: We declare the method `GetHead()` to get the first fixture in the list
- Line 77: We declare the method `Name()` to get the suite name
- Line 79: We declare a template method `RunIf()` to run tests satisfying a certain predicate.
All tests that fit with the predicate will be run, and the results will be stored
- Line 81: We declare the method `CountFixtures()` which counts and returns the number of test fixtures in the suite
- Line 82: We declare the method `CountTests()` which counts and returns the number of tests in the suite
- Line 83: We declare the template method `CountFixturesIf()` which counts and returns the number of test fixtures satisfying the predicate in the suite
- Line 84: We declare the template method `CountTestsIf()` which counts and returns the number of tests satisfying the predicate in the suite
- Line 87-100: We implement the template method `RunIf()`.
This will invoke the start and end suite calls on the results, and in between calls the `RunIf()` method on each of the test fixtures that satisfy the predicate
- Line 102-113: We implement the template method `CountFixturesIf()`.
This will count the test fixtures that satisfy the predicate
- Line 115-126: We implement the template method `CountTestsIf()`.
This will calls the `CountTestsIf()` method on each of the test fixtures that satisfy the predicate

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTFIXTUREINFOH}

The `TestFixtureInfo` class holds administration for a test fixture.
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
42: #include "unittest/TestBase.h"
43: #include "unittest/TestResults.h"
44: 
45: namespace unittest
46: {
47: 
48: class TestBase;
49: 
50: class TestFixtureInfo
51: {
52: public:
53:     friend class TestRegistry;
54:     friend class TestRegistrar;
55:     TestFixtureInfo * m_next;
56: private:
57:     TestBase * m_head;
58:     TestBase * m_tail;
59:     baremetal::string m_fixtureName;
60: 
61:     void AddTest(TestBase* test);
62: 
63: public:
64:     TestFixtureInfo() = delete;
65:     TestFixtureInfo(const TestFixtureInfo&) = delete;
66:     TestFixtureInfo(TestFixtureInfo&&) = delete;
67:     explicit TestFixtureInfo(const baremetal::string& fixtureName);
68:     virtual ~TestFixtureInfo();
69: 
70:     TestFixtureInfo & operator = (const TestFixtureInfo &) = delete;
71:     TestFixtureInfo& operator = (TestFixtureInfo&&) = delete;
72: 
73:     TestBase* GetHead() const;
74: 
75:     const baremetal::string& Name() const { return m_fixtureName; }
76: 
77:     template <class Predicate> void RunIf(const Predicate & predicate, int maxTestTimeInMs, TestResults * testResults);
78: 
79:     int CountTests();
80:     template <typename Predicate> int CountTestsIf(Predicate predicate);
81: };
82: 
83: template <class Predicate> void TestFixtureInfo::RunIf(const Predicate & predicate, int maxTestTimeInMs, TestResults * testResults)
84: {
85:     testResults->OnTestFixtureStart(this);
86: 
87:     TestBase* test = this->GetHead();
88:     while (test != nullptr)
89:     {
90:         if (predicate(test))
91:             test->Run(maxTestTimeInMs, testResults);
92:         test = test->m_next;
93:     }
94: 
95:     testResults->OnTestFixtureFinish(this, 0/*testTimer.GetTimeInMilliSeconds()*/);
96: }
97: 
98: template <typename Predicate> int TestFixtureInfo::CountTestsIf(Predicate predicate)
99: {
100:     int numberOfTests = 0;
101:     TestBase* test = this->GetHead();
102:     while (test != nullptr)
103:     {
104:         if (predicate(test))
105:             numberOfTests++;
106:         test = test->m_next;
107:     }
108:     return numberOfTests;
109: }
110: 
111: } // namespace unittest
```

- Line 55: We keep a pointer to the next `TestFixtureInfo`, as the fixture are organized as a linked list
- Line 57-58: We keep a pointer to the first and last `TestBase` instance, so we can iterate over them
- Line 61: We declare a private method `AddTest()` which is used by the registry to add tests at initialization time
- Line 64-66: Remove the default, copy and move constructors
- Line 67: We instead only define an explicit constructor
- Line 68: We declare a destructor which is needed to clean up after ourselves. The `TestRegistry` will allocate `TestSuiteInfo`, `TestFixtureInfo` and `TestBase` instances at initialization time, we will clean them up in the destructor
- Line 70-71: We remove the assignment and move assignment operators
- Line 73: We declare the method `GetHead()` to get the first fixture in the list
- Line 75: We declare the method `Name()` to get the fixture name
- Line 77: We declare a template method `RunIf()` to run tests satisfying a certain predicate.
All tests that fit with the predicate will be run, and the results will be stored
- Line 79: We declare the method `CountTests()` which counts and returns the number of tests in the fixture
- Line 80: We declare the template method `CountTestsIf()` which counts and returns the number of tests satisfying the predicate in the fixture
- Line 83-96: We implement the template method `RunIf()`.
This will invoke the start and end fixture calls on the results, and in between calls the `Run()` method on each of the tests that satisfy the predicate
- Line 98-109: We implement the template method `CountTestsIf()`.
This will count the tests that satisfy the predicate

### TestSuiteInfo.cpp {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTSUITEINFOCPP}

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
52:     : m_next{}
53:     , m_head{}
54:     , m_tail{}
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
78:         LOG_DEBUG("Find fixture %s not found, creating new object", fixtureName.empty() ? "-" : fixtureName);
79: #endif
80:         testFixture = new TestFixtureInfo(fixtureName);
81:         AddFixture(testFixture);
82:     }
83:     else
84:     {
85: #ifdef DEBUG_REGISTRY
86:         LOG_DEBUG("Find fixture %s found", fixtureName.empty() ? "-" : fixtureName);
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
112: int TestSuiteInfo::CountFixtures()
113: {
114:     int              numberOfTestFixtures = 0;
115:     TestFixtureInfo *testFixture          = m_head;
116:     while (testFixture != nullptr)
117:     {
118:         ++numberOfTestFixtures;
119:         testFixture = testFixture->m_next;
120:     }
121:     return numberOfTestFixtures;
122: }
123: 
124: int TestSuiteInfo::CountTests()
125: {
126:     int              numberOfTests = 0;
127:     TestFixtureInfo *testFixture   = m_head;
128:     while (testFixture != nullptr)
129:     {
130:         numberOfTests += testFixture->CountTests();
131:         testFixture = testFixture->m_next;
132:     }
133:     return numberOfTests;
134: }
135: 
136: } // namespace unittest
```

### TestFixtureInfo.cpp {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTFIXTUREINFOCPP}

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
39: #include "unittest/TestFixtureInfo.h"
40: 
41: #include <baremetal/Assert.h>
42: 
43: using namespace baremetal;
44: 
45: namespace unittest {
46: 
47: TestFixtureInfo::TestFixtureInfo(const string& fixtureName)
48:     : m_next{}
49:     , m_head{}
50:     , m_tail{}
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

### TestRegistry.h {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTREGISTRYH}

The last part which we kept until now is the registration of tests at initialization time.
Registering tests is done through a combination of two classes: `TestRegistry` and `TestRegistrar`.
This works as shown below. 

The `TestFixtureInfo` class holds administration for a test fixture.
Create the file `code/libraries/unittest/include/unittest/TestFixtureInfo.h`

```cpp
File: code/libraries/unittest/include/unittest/TestFixtureInfo.h
1: //------------------------------------------------------------------------------

### Update project configuration {#TUTORIAL_16_UNIT_TESTS_CREATING_A_FRAMEWORK_FOR_UNIT_TESTING_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.
Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
51:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
52:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
53:     )
54: 
55: set(PROJECT_INCLUDES_PUBLIC
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/StdArg.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
86:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
87:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
88:     )
89: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_APPLICATION_CODE}

We'll start making use of the string class we just added, but we'll do it in a way that shows that the string methods function as expected.
This is a first attempt at creating class / micro / unit tests for our code, which we will start doing in the next tutorial.

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
17: LOG_MODULE("main");
18: 
19: using namespace baremetal;
20: 
21: int main()
22: {
23:     auto& console = GetConsole();
24:     LOG_DEBUG("Hello World!");
25: 
26:     string s1{ "a" };
27:     string s2{ "a" };
28:     string s3{ "aaaa", 3 };
29:     string s4{ 4, 'b' };
30:     string s5{ s3 };
31:     string s6{ s3, 1 };
32:     string s7{ s3, 1, 1 };
33:     string s8{ nullptr };
34:     string s9{ "" };
35:     string s10{ nullptr, 3 };
36: 
37:     LOG_INFO("s1");
38:     for (auto ch : s1)
39:     {
40:         LOG_INFO("%c", ch);
41:     }
42:     assert(strcmp(s1, "a") == 0);
43:     assert(strcmp(s2, "a") == 0);
44:     assert(strcmp(s3, "aaa") == 0);
45:     assert(strcmp(s4, "bbbb") == 0);
46:     assert(strcmp(s5, "aaa") == 0);
47:     assert(strcmp(s6, "aa") == 0);
48:     assert(strcmp(s7, "a") == 0);
49:     assert(strcmp(s8, "") == 0);
50:     assert(strcmp(s9, "") == 0);
51:     assert(strcmp(s10, "") == 0);
52: 
53:     s7 = "abcde";
54:     assert(strcmp(s7, "abcde") == 0);
55:     s7 = nullptr;
56:     assert(strcmp(s7, "") == 0);
57:     s7 = "";
58:     assert(strcmp(s7, "") == 0);
59:     s6 = s4;
60:     assert(strcmp(s6, "bbbb") == 0);
61:     {
62:         string s99{ "cccc" };
63:         s6 = s99;
64:     }
65:     assert(strcmp(s6, "cccc") == 0);
66: 
67:     const char* s = "abcdefghijklmnopqrstuvwxyz";
68:     s1.assign(s);
69:     assert(strcmp(s1, "abcdefghijklmnopqrstuvwxyz") == 0);
70:     s1.assign("");
71:     assert(strcmp(s1, "") == 0);
72:     s1.assign(nullptr);
73:     assert(strcmp(s1, "") == 0);
74:     s1.assign(s, 6);
75:     assert(strcmp(s1, "abcdef") == 0);
76:     s1.assign("", 6);
77:     assert(strcmp(s1, "") == 0);
78:     s1.assign(nullptr, 6);
79:     assert(strcmp(s1, "") == 0);
80:     s8 = s;
81:     s1.assign(s8, 3);
82:     assert(strcmp(s1, "defghijklmnopqrstuvwxyz") == 0);
83:     s1.assign(s8, 4, 6);
84:     assert(strcmp(s1, "efghij") == 0);
85:     s1.assign(6, 'c');
86:     assert(strcmp(s1, "cccccc") == 0);
87: 
88:     const string s8c{ s8 };
89:     assert(s8.at(3) == 'd');
90:     assert(s8c.at(3) == 'd');
91:     assert(s8.front() == 'a');
92:     assert(s8c.front() == 'a');
93:     assert(s8.back() == 'z');
94:     assert(s8c.back() == 'z');
95:     assert(s8[3] == 'd');
96:     assert(s8c[3] == 'd');
97:     assert(s8c.capacity() == 64);
98:     assert(s8.reserve(128) == 128);
99:     assert(s8.capacity() == 128);
100: 
101:     s1 = "a";
102:     assert(strcmp(s1, "a") == 0);
103:     s1 += 'b';
104:     assert(strcmp(s1, "ab") == 0);
105:     s2 = "a";
106:     s1 += s2;
107:     assert(strcmp(s1, "aba") == 0);
108:     s1 += "abcde";
109:     assert(strcmp(s1, "abaabcde") == 0);
110:     s1 = "a";
111:     s1 += "";
112:     assert(strcmp(s1, "a") == 0);
113:     s1 += nullptr;
114:     assert(strcmp(s1, "a") == 0);
115: 
116:     s3 = "";
117:     s4 = s1 + s2;
118:     assert(strcmp(s4, "aa") == 0);
119:     s4 = s1 + s3;
120:     assert(strcmp(s4, "a") == 0);
121:     s4 = s1 + "b";
122:     assert(strcmp(s4, "ab") == 0);
123:     s4 = s1 + "";
124:     assert(strcmp(s4, "a") == 0);
125:     s4 = s1 + nullptr;
126:     assert(strcmp(s4, "a") == 0);
127:     s4 = "b" + s1;
128:     assert(strcmp(s4, "ba") == 0);
129:     s4 = "" + s1;
130:     assert(strcmp(s4, "a") == 0);
131:     s4 = nullptr + s1;
132:     assert(strcmp(s4, "a") == 0);
133: 
134:     s1 = "a";
135:     s1.append(4, 'b');
136:     assert(strcmp(s1, "abbbb") == 0);
137:     s1.append(s2);
138:     assert(strcmp(s1, "abbbba") == 0);
139:     s1.append(s8, 3, 5);
140:     assert(strcmp(s1, "abbbbadefgh") == 0);
141:     s1.append("ccc");
142:     assert(strcmp(s1, "abbbbadefghccc") == 0);
143:     s1.append("dddddd", 3);
144:     assert(strcmp(s1, "abbbbadefghcccddd") == 0);
145:     s1.clear();
146:     assert(strcmp(s1, "") == 0);
147:     s1.append("");
148:     assert(strcmp(s1, "") == 0);
149:     s1.append(nullptr);
150:     assert(strcmp(s1, "") == 0);
151:     s1.append("", 3);
152:     assert(strcmp(s1, "") == 0);
153:     s1.append(nullptr, 3);
154:     assert(strcmp(s1, "") == 0);
155: 
156:     s1 = s;
157:     s2 = "c";
158:     auto pos = s1.find(s2);
159:     assert(pos == 2);
160:     pos = s1.find(s2, 1);
161:     assert(pos == 2);
162:     pos = s1.find(s2, 3);
163:     assert(pos == string::npos);
164:     s2 = "deg";
165:     pos = s1.find(s2, 3);
166:     assert(pos == string::npos);
167:     pos = s1.find(s2, 3, 2);
168:     assert(pos == 3);
169:     s2 = "xyz";
170:     pos = s1.find(s2);
171:     assert(pos == 23);
172: 
173:     pos = s1.find("d");
174:     assert(pos == 3);
175:     pos = s1.find("d", 1);
176:     assert(pos == 3);
177:     pos = s1.find("d", 4);
178:     assert(pos == string::npos);
179:     pos = s1.find("def", 2);
180:     assert(pos == 3);
181:     pos = s1.find("deg", 2);
182:     assert(pos == string::npos);
183:     pos = s1.find("deg", 2, 2);
184:     assert(pos == 3);
185:     pos = s1.find("xyz");
186:     assert(pos == 23);
187:     pos = s1.find("");
188:     assert(pos == 0);
189:     pos = s1.find(nullptr);
190:     assert(pos == 0);
191:     pos = s1.find("", 2);
192:     assert(pos == 2);
193:     pos = s1.find(nullptr, 2);
194:     assert(pos == 2);
195:     pos = s1.find(nullptr, 26);
196:     assert(pos == string::npos);
197:     pos = s1.find("", 2, 2);
198:     assert(pos == 2);
199:     pos = s1.find(nullptr, 2, 2);
200:     assert(pos == 2);
201:     pos = s1.find(nullptr, 26, 1);
202:     assert(pos == string::npos);
203: 
204:     pos = s1.find('d');
205:     assert(pos == 3);
206:     pos = s1.find('d', 2);
207:     assert(pos == 3);
208:     pos = s1.find('d', 4);
209:     assert(pos == string::npos);
210:     pos = s1.find('A');
211:     assert(pos == string::npos);
212:     pos = s1.find("z");
213:     assert(pos == 25);
214: 
215:     s2 = "abc";
216:     s3 = "xyz";
217:     auto isTrue = s1.starts_with('a');
218:     assert(isTrue);
219:     isTrue = s1.starts_with('z');
220:     assert(!isTrue);
221:     isTrue = s1.starts_with("abc");
222:     assert(isTrue);
223:     isTrue = s1.starts_with("xyz");
224:     assert(!isTrue);
225:     isTrue = s1.starts_with("");
226:     assert(isTrue);
227:     isTrue = s1.starts_with(nullptr);
228:     assert(isTrue);
229:     isTrue = s1.starts_with(s2);
230:     assert(isTrue);
231:     isTrue = s1.starts_with(s3);
232:     assert(!isTrue);
233: 
234:     isTrue = s1.ends_with('a');
235:     assert(!isTrue);
236:     isTrue = s1.ends_with('z');
237:     assert(isTrue);
238:     isTrue = s1.ends_with("abc");
239:     assert(!isTrue);
240:     isTrue = s1.ends_with("xyz");
241:     assert(isTrue);
242:     isTrue = s1.ends_with("");
243:     assert(isTrue);
244:     isTrue = s1.ends_with(nullptr);
245:     assert(isTrue);
246:     isTrue = s1.ends_with(s2);
247:     assert(!isTrue);
248:     isTrue = s1.ends_with(s3);
249:     assert(isTrue);
250: 
251:     isTrue = s1.contains('a');
252:     assert(isTrue);
253:     isTrue = s1.contains('A');
254:     assert(!isTrue);
255:     isTrue = s1.contains("abc");
256:     assert(isTrue);
257:     isTrue = s1.contains("XYZ");
258:     assert(!isTrue);
259:     isTrue = s1.contains("");
260:     assert(isTrue);
261:     isTrue = s1.contains(nullptr);
262:     assert(isTrue);
263:     isTrue = s1.contains(s2);
264:     assert(isTrue);
265:     isTrue = s1.contains(s3);
266:     assert(isTrue);
267: 
268:     s2 = s1.substr();
269:     assert(strcmp(s2, "abcdefghijklmnopqrstuvwxyz") == 0);
270:     s2 = s1.substr(6);
271:     assert(strcmp(s2, "ghijklmnopqrstuvwxyz") == 0);
272:     s2 = s1.substr(6, 6);
273:     assert(strcmp(s2, "ghijkl") == 0);
274: 
275:     s1 = "abcdefg";
276:     s2 = "abcdefG";
277:     s3 = "abcdefg";
278:     isTrue = s1.equals(s2);
279:     assert(!isTrue);
280:     isTrue = s1.equals(s3);
281:     assert(isTrue);
282:     isTrue = s1.equals("abcefg");
283:     assert(!isTrue);
284:     isTrue = s1.equals("abcdefg");
285:     assert(isTrue);
286:     isTrue = s1.equals("");
287:     assert(!isTrue);
288:     isTrue = s1.equals(nullptr);
289:     assert(!isTrue);
290:     s4 = "";
291:     isTrue = s4.equals_case_insensitive(s3);
292:     assert(!isTrue);
293:     isTrue = s4.equals("");
294:     assert(isTrue);
295:     isTrue = s4.equals(nullptr);
296:     assert(isTrue);
297: 
298:     isTrue = s1.equals_case_insensitive(s2);
299:     assert(isTrue);
300:     isTrue = s1.equals_case_insensitive(s3);
301:     assert(isTrue);
302:     isTrue = s1.equals_case_insensitive("abcefg");
303:     assert(!isTrue);
304:     isTrue = s1.equals_case_insensitive("abcdefg");
305:     assert(isTrue);
306:     isTrue = s1.equals_case_insensitive("");
307:     assert(!isTrue);
308:     isTrue = s1.equals_case_insensitive(nullptr);
309:     assert(!isTrue);
310:     s4 = "";
311:     isTrue = s4.equals_case_insensitive(s3);
312:     assert(!isTrue);
313:     isTrue = s4.equals_case_insensitive("");
314:     assert(isTrue);
315:     isTrue = s4.equals_case_insensitive(nullptr);
316:     assert(isTrue);
317: 
318:     assert(s1 == s3);
319:     assert(s1 != s2);
320:     assert(s1 == "abcdefg");
321:     assert(s1 != "abcdefG");
322:     assert(s1 != "");
323:     assert(s1 != nullptr);
324:     assert("abcdefg" == s1);
325:     assert("abcdefG" != s1);
326:     assert("" != s1);
327:     assert(nullptr != s1);
328:     assert(s4 != s3);
329:     assert(s4 == "");
330:     assert(s4 == nullptr);
331:     assert("" == s4);
332:     assert(nullptr == s4);
333: 
334:     s4 = "bcdefg";
335:     s5 = "def";
336:     auto result = s1.compare(s2);
337:     assert(result == 1);
338:     result = s2.compare(s1);
339:     assert(result == -1);
340:     result = s1.compare(s3);
341:     assert(result == 0);
342:     result = s3.compare(s1);
343:     assert(result == 0);
344:     result = s1.compare(1, 6, s4);
345:     assert(result == 0);
346:     result = s1.compare(1, 5, s4);
347:     assert(result == -1);
348:     result = s1.compare(3, 6, s4, 2);
349:     assert(result == 0);
350:     result = s1.compare(3, 6, s4, 2, 1);
351:     assert(result == 1);
352:     result = s1.compare(3, 3, s4, 2, 6);
353:     assert(result == -1);
354: 
355:     result = s1.compare("a");
356:     assert(result == 1);
357:     result = s1.compare("Abcdefg");
358:     assert(result == 1);
359:     result = s1.compare("abdecfg");
360:     assert(result == -1);
361:     result = s1.compare("");
362:     assert(result == 1);
363:     result = s1.compare(nullptr);
364:     assert(result == 1);
365:     s2 = "";
366:     result = s2.compare("a");
367:     assert(result == -1);
368:     result = s2.compare("");
369:     assert(result == 0);
370:     result = s2.compare(nullptr);
371:     assert(result == 0);
372: 
373:     s1 = "abcde";
374:     s2 = "fghijk";
375:     s3 = s1.replace(0, 1, s2);
376:     assert(s1.equals("fghijkbcde"));
377:     assert(s3.equals("fghijkbcde"));
378:     s1 = "abcde";
379:     s3 = s1.replace(1, 2, s2, 2);
380:     assert(s1.equals("ahijkde"));
381:     assert(s3.equals("ahijkde"));
382:     s1 = "abcde";
383:     s3 = s1.replace(1, 2, s2, 2, 2);
384:     assert(s1.equals("ahide"));
385:     assert(s3.equals("ahide"));
386:     s1 = "abcde";
387:     s3 = s1.replace(0, 1, "uvwxyz");
388:     assert(s1.equals("uvwxyzbcde"));
389:     assert(s3.equals("uvwxyzbcde"));
390:     s1 = "abcde";
391:     s3 = s1.replace(1, 2, "uvwxyz", 2);
392:     assert(s1.equals("auvde"));
393:     assert(s3.equals("auvde"));
394:     s1 = "abcde";
395:     s3 = s1.replace(0, 1, 'x');
396:     assert(s1.equals("xbcde"));
397:     assert(s3.equals("xbcde"));
398:     s1 = "abcde";
399:     s3 = s1.replace(1, 2, 'x', 3);
400:     assert(s1.equals("axxxde"));
401:     assert(s3.equals("axxxde"));
402: 
403:     s1 = "abcde";
404:     s2 = "cd";
405:     s3 = "xy";
406:     int count = s1.replace(s2, s3);
407:     assert(count == 1);
408:     assert(s1.equals("abxye"));
409:     s1 = "abababab";
410:     s2 = "ab";
411:     s3 = "cd";
412:     count = s1.replace(s2, s3);
413:     assert(count == 4);
414:     assert(s1.equals("cdcdcdcd"));
415:     s1 = "abcde";
416:     count = s1.replace("cd", "xy");
417:     assert(count == 1);
418:     assert(s1.equals("abxye"));
419:     s1 = "abababab";
420:     count = s1.replace("ab", "cd");
421:     assert(count == 4);
422:     assert(s1.equals("cdcdcdcd"));
423: 
424:     s1 = "abcd";
425:     s2 = s1.align(8);
426:     assert(s2.equals("    abcd"));
427:     s2 = s1.align(-8);
428:     assert(s2.equals("abcd    "));
429:     s2 = s1.align(0);
430:     assert(s2.equals("abcd"));
431: 
432:     LOG_INFO("Wait 5 seconds");
433:     Timer::WaitMilliSeconds(5000);
434: 
435:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
436:     char ch{};
437:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
438:     {
439:         ch = console.ReadChar();
440:         console.WriteChar(ch);
441:     }
442:     if (ch == 'p')
443:         assert(false);
444: 
445:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
446: }
```

As you can see the code has grown quite abit due to all the tests that we perform.

- Line 11: we need to include `String.h`
- Line 26-35: We construct some strings, in different ways, to cover all the variants of constructors
- Line 37-41: We use a for loop to iterate through the characters in the string. This will use the `begin()` and `end()` methods
- Line 42-51: We check whether the strings are initialized as expected. Notice that we use the `assert` macro here
- Line 53-65: We test assignment operators
- Line 67-86: We test the `assign()` methods
- Line 88-99: We test the `at()`, `front()`, `back()`, `capacity()` and `reserve()` methods, as well as the index operator `[]`
- Line 101-114: We test the addition assignment (in this case concatenation) operator `+=`
- Line 116-132: We test the addition (concatenation) operator `+`
- Line 134-154: We test the `append()` methods
- Line 156-213: We test the `find()` methods
- Line 215-232: We test the `starts_with()` methods
- Line 234-249: We test the `ends_with()` methods
- Line 251-266: We test the `contains()` methods
- Line 268-273: We test the `substr()` methods
- Line 275-316: We test the `equals()` and `equals_case_insensitive()` methods
- Line 318-332: We test the equality operators `==` and the inequality operators `!=`
- Line 334-371: We test the `compare()` methods
- Line 373-422: We test the `replace()` methods
- Line 424-430: We test the `align()` method

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will print the characters in the string "bbbb" in log statements.
Next to this nothing special is shown, as the tests will all succeed. If a test were to fail, the assertion would fire, and the application would crash.
This is a way to test code, however we'd like to see all of the failure found in our code, not have the first one crash the application.
This is the subject of the next tutorial.

```text
```

Next: [16-unit-tests](16-unit-tests.md)

