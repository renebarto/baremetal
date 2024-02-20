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

Let's start using the class we just created. We'll add a simple test case by declaring and implementing a class derived from `TestBase`.
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

- Line 17: We include the header for `TestBase`
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

## Adding test fixtures - Step 2 {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2}

Now that we have a test we can run, let's add the test fixture, to hold multiple tests, and provide for a setup / teardown method call.
Test fixtures are slightly different from tests in their structure.
- A `TestBase` class holds, next to the `RunImpl()` method that can be overridden, also the pointer to the next test, and the `TestDetails`.
- The `TestDetails` class holds only information on a test, such as the test name, test fixture name, etc.

For test fixtures this is different:
- The `TestFixture` class is only a class that can be overriden to implement the `SetUp()` and `TearDown()` methods.
It is overridden by a fixture helper class for each test in the fixture, that acts a test, but is constructed before running the test, and destructed afterwards.
Its `RunImpl()` method will run the actual test. The constructor will run the `SetUp()` method, the destructor will run the `TearDown()` method.
- Next to a fixture helper class and actual `TestBase` derived class is created which is registered as the test.
Its `RunImpl()` method will instantiate the fixture helper class, thus running its `SetUp()` method, then running the test, and finally destructing the fixture helper class again, running its `TearDown()` method.
- The `TestFixtureInfo` class holds the actual test fixture information, such as the pointer to the next fixture, and the pointers to the first and last test in the fixture.

So the `TestFixtureInfo` class holds the administration of the test fixture, like `TestBase` does for tests, and for each test in the test fixture, a pair of a fixture helper class deriving from `TestFixture` and an actual test class deriving from `TestBase` is created.

See also the image below.

<img src="images/unittest-class-structure-test-fixture.png" alt="Tree view" width="800"/>

Things will become more clear when we start using the test fixtures.

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_TESTFIXTUREINFOH}

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
43: 
44: namespace unittest
45: {
46: 
47: class TestBase;
48: 
49: class TestFixtureInfo
50: {
51: private:
52:     TestBase* m_head;
53:     TestBase* m_tail;
54:     TestFixtureInfo* m_next;
55:     baremetal::string m_fixtureName;
56: 
57: public:
58:     TestFixtureInfo() = delete;
59:     TestFixtureInfo(const TestFixtureInfo&) = delete;
60:     TestFixtureInfo(TestFixtureInfo&&) = delete;
61:     explicit TestFixtureInfo(const baremetal::string& fixtureName);
62:     virtual ~TestFixtureInfo();
63: 
64:     TestFixtureInfo & operator = (const TestFixtureInfo &) = delete;
65:     TestFixtureInfo& operator = (TestFixtureInfo&&) = delete;
66: 
67:     TestBase* GetHead() const;
68: 
69:     const baremetal::string& Name() const { return m_fixtureName; }
70: 
71:     void Run();
72: 
73:     int CountTests();
74: 
75:     void AddTest(TestBase* test);
76: };
77: 
78: } // namespace unittest
```

- Line 52-53: The member variables `m_head` and `m_tail` store the pointer to the first and last test in the fixture
- Line 54: The member variable `m_next` is the pointer to the next test fixture. Again, test fixtures are stored in linked list
- Line 55: The member variable `m_fixtureName` holds the name of the test fixture
- Line 58: We remove the default constructor
- Line 59-60: We remove the copy and move constructors
- Line 61: We declare the only usable constructor which receives the test fixture name
- Line 62: We declare the destructor
- Line 64-65: We remove the assignment and move assignment operators
- Line 67: The method `GetHead()` returns the pointer to the first test in the list
- Line 69: The method `Name()` returns the test fixture name
- Line 71: The method `Run()` runs all tests in the test fixture. We'll be revisiting this later
- Line 73: The method `CountTests()` counts and returns the number of tests in the test fixture
- Line 75: The method `AddTest()` adds a test to the list for the test fixture

### TestBase.h {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_TESTBASEH}

As the `TestFixtureInfo` class needs access to the `TestBase` class in order to access the `m_next` pointer, we need to make it a friend class.
Update the file `code/libraries/unittest/include/unittest/TestBase.h`

```cpp
File: code/libraries/unittest/include/unittest/TestBase.h
...
47: class TestBase
48: {
49: private:
50:     friend class TestFixtureInfo;
51:     TestDetails const m_details;
52:     TestBase* m_next;
53: 
54: public:
55:     TestBase();
...
```

### TestFixtureInfo.cpp {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_TESTFIXTUREINFOCPP}

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
86: void TestFixtureInfo::Run()
87: {
88:     TestBase* test = this->GetHead();
89:     while (test != nullptr)
90:     {
91:         test->Run();
92:         test = test->m_next;
93:     }
94: }
95: 
96: int TestFixtureInfo::CountTests()
97: {
98:     int numberOfTests = 0;
99:     TestBase* test = m_head;
100:     while (test != nullptr)
101:     {
102:         ++numberOfTests;
103:         test = test->m_next;
104:     }
105:     return numberOfTests;
106: }
107: 
108: } // namespace unittest
```

- Line 47-53: We implement the constructor
- Line 55-64: We implement the destructor. This goes through the list of tests, and deletes every one of these. Note that we will therefore need to create the tests on the heap.
- Line 66-79: We implement the `AddTest()` method. This will add the test passed in at the end of the list
- Line 81-84: We implement the `GetHead()` method
- Line 86-94: We implement the `Run()` method. This goes through the list of tests, and calls `Run()` on each
- Line 96-106: We implement the `CountTests()` method. This goes through the list of tests, and counts them

### TestFixture.h {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_TESTFIXTUREH}

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
42: namespace unittest {
43: 
44: class TestFixture
45: {
46: protected:
47:     TestFixture() = default;
48:     TestFixture(const TestFixture&) = delete;
49:     ~TestFixture() = default;
50: 
51:     TestFixture& operator = (const TestFixture&) = delete;
52: 
53:     virtual void SetUp() {};
54:     virtual void TearDown() {};
55: };
56: 
57: } // namespace unittest
```

- Line 44-57: We declare the `TestFixture` class
  - Line 47: We make the default constructor a default implementation
  - Line 48: We remove the copy constructor
  - Line 49: We make the destructor a default implementation
  - Line 51: We remove the assignment operator
  - Line 53-54: We declare and implement the virtual `SetUp()` and `TearDown()` methods

As can be seen, nothing else needs to be added for implementation.

### Update CMake file {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_UPDATE_CMAKE_FILE}

As we have added some files to the `unittest` library, we need to update its CMake file.
Update the file `code/libraries/unitttest/CMakeLists.txt`

```cmake
File: code/libraries/unitttest/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestBase.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
34:     )
35: 
36: set(PROJECT_INCLUDES_PUBLIC
37:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestBase.h
38:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
39:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
41:     )
42: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Update application code {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_UPDATE_APPLICATION_CODE}

Let's start using the test fixtures.
We'll add a couple of simple test cases by declaring and implementing a class derived from `TestBase`.
We'll then create a test fixture, and add the tests to the fixture.
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
20: 
21: LOG_MODULE("main");
22: 
23: using namespace baremetal;
24: using namespace unittest;
25: 
26: class FixtureMyTest
27:     : public TestFixture
28: {
29: public:
30:     void SetUp() override
31:     {
32:         LOG_DEBUG("MyTest SetUp");
33:     }
34:     void TearDown() override
35:     {
36:         LOG_DEBUG("MyTest TearDown");
37:     }
38: };
39: 
40: class FixtureMyTest1Helper
41:     : public FixtureMyTest
42: {
43: public:
44:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
45:     explicit FixtureMyTest1Helper(unittest::TestDetails const& details)
46:         : m_details{ details }
47:     {
48:         SetUp();
49:     }
50:     virtual ~FixtureMyTest1Helper()
51:     {
52:         TearDown();
53:     }
54:     void RunImpl() const;
55:     unittest::TestDetails const& m_details;
56: };
57: void FixtureMyTest1Helper::RunImpl() const
58: {
59:     LOG_DEBUG("MyTestHelper 1");
60: }
61: 
62: class FixtureMyTest2Helper
63:     : public FixtureMyTest
64: {
65: public:
66:     FixtureMyTest2Helper(const FixtureMyTest2Helper&) = delete;
67:     explicit FixtureMyTest2Helper(unittest::TestDetails const& details)
68:         : m_details{ details }
69:     {
70:         SetUp();
71:     }
72:     virtual ~FixtureMyTest2Helper()
73:     {
74:         TearDown();
75:     }
76:     void RunImpl() const;
77:     unittest::TestDetails const& m_details;
78: };
79: void FixtureMyTest2Helper::RunImpl() const
80: {
81:     LOG_DEBUG("MyTestHelper 2");
82: }
83: 
84: class FixtureMyTest3Helper
85:     : public FixtureMyTest
86: {
87: public:
88:     FixtureMyTest3Helper(const FixtureMyTest3Helper&) = delete;
89:     explicit FixtureMyTest3Helper(unittest::TestDetails const& details)
90:         : m_details{ details }
91:     {
92:         SetUp();
93:     }
94:     virtual ~FixtureMyTest3Helper()
95:     {
96:         TearDown();
97:     }
98:     void RunImpl() const;
99:     unittest::TestDetails const& m_details;
100: };
101: void FixtureMyTest3Helper::RunImpl() const
102: {
103:     LOG_DEBUG("MyTestHelper 3");
104: }
105: 
106: class MyTest1
107:     : public TestBase
108: {
109: public:
110:     MyTest1()
111:         : TestBase("MyTest1", "MyFixture", "", __FILE__, __LINE__)
112:     {
113: 
114:     }
115:     void RunImpl() const override
116:     {
117:         LOG_DEBUG("Test 1");
118:         FixtureMyTest1Helper fixtureHelper(Details());
119:         fixtureHelper.RunImpl();
120:     }
121: };
122: 
123: class MyTest2
124:     : public TestBase
125: {
126: public:
127:     MyTest2()
128:         : TestBase("MyTest2", "MyFixture", "", __FILE__, __LINE__)
129:     {
130: 
131:     }
132:     void RunImpl() const override
133:     {
134:         LOG_DEBUG("Test 2");
135:         FixtureMyTest2Helper fixtureHelper(Details());
136:         fixtureHelper.RunImpl();
137:     }
138: };
139: 
140: class MyTest3
141:     : public TestBase
142: {
143: public:
144:     MyTest3()
145:         : TestBase("MyTest3", "MyFixture", "", __FILE__, __LINE__)
146:     {
147: 
148:     }
149:     void RunImpl() const override
150:     {
151:         LOG_DEBUG("Test 3");
152:         FixtureMyTest3Helper fixtureHelper(Details());
153:         fixtureHelper.RunImpl();
154:     }
155: };
156: 
157: int main()
158: {
159:     auto& console = GetConsole();
160:     LOG_DEBUG("Hello World!");
161: 
162:     TestBase* test1 = new MyTest1;
163:     TestBase* test2 = new MyTest2;
164:     TestBase* test3 = new MyTest3;
165:     TestFixtureInfo* fixture = new TestFixtureInfo("MyFixture");
166:     fixture->AddTest(test1);
167:     fixture->AddTest(test2);
168:     fixture->AddTest(test3);
169:     fixture->Run();
170:     delete fixture;
171: 
172:     LOG_INFO("Wait 5 seconds");
173:     Timer::WaitMilliSeconds(5000);
174: 
175:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
176:     char ch{};
177:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
178:     {
179:         ch = console.ReadChar();
180:         console.WriteChar(ch);
181:     }
182:     if (ch == 'p')
183:         assert(false);
184: 
185:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
186: }
```

- Line 18: We include the header for `TestFixture`
- Line 19: We include the header for `TestFixtureInfo`

- Line 26-38: We declare and implement a class `FixtureMyTest` which derives from `TestFixture` and implements the `SetUp()` and `TearDown()` methods.
- Line 40-60: We declare and implement a class `FixtureMyTest1Helper` which derives from the class `FixtureMyTest` just defined, and implements the constructor and destructor, calling resp. `SetUp()` and `TearDown()`.
This class also defines a `RunImpl()` method, which forms the actual test body for the test MyTest1
- Line 62-82:: We do the similar thing for `FixtureTest2Helper`
- Line 84-104: We do the similar thing for `FixtureTest3Helper`
- Line 106-121: We declare and implement the class `MyTest1`, which derives from `TestBase`, and acts as the placeholders for the fixture test.
Its `RunImpl()` method instantiates `FixtureMyTest1Helper`, and runs its `RunImpl()` method
- Line 123-138: We do the similar thing for `MyTest2`
- Line 140-155: We do the similar thing for `MyTest3`
- Line 162-164: We instantiate each of `MyTest1`, `MyTest2` and `MyTest3`
- Line 165: We instantiate `TestFixtureInfo` as our test fixture
- Line 166-168: We add the three tests to the test fixture
- Line 169: We run the test fixture
- Line 170: We clean up the test fixture. Note that the test fixture desctructor deletes all tests, so we don't need to (and shouldn't) do that

This all seems like quite a bit of plumbing just to run three tests.
That is why we'll create macros later to do this work for us.
But it's good to understand what is happening underneath the hood.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging. 

The application will run the tests in the test fixture, and therefore show the log output.
You'll se that for each test the `RunImpl()` method of `MyTest<x>` runs.
This then instantiates the `FixtureTest<x>Helper`, and its constructor runs the `FixtureMyTest` method `SetUp()`.
Then the `RunImpl()` of `FixtureTest<x>Helper` is run, and finally the class is destructed again, leading to the `FixtureMyTest` method `TearDown()` begin run.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Info   Starting up (System:201)
Debug  Hello World! (main:160)
Debug  Test 1 (main:117)
Debug  MyTest SetUp (main:32)
Debug  MyTestHelper 1 (main:59)
Debug  MyTest TearDown (main:36)
Debug  Test 2 (main:134)
Debug  MyTest SetUp (main:32)
Debug  MyTestHelper 2 (main:81)
Debug  MyTest TearDown (main:36)
Debug  Test 3 (main:151)
Debug  MyTest SetUp (main:32)
Debug  MyTestHelper 3 (main:103)
Debug  MyTest TearDown (main:36)
Info   Wait 5 seconds (main:172)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Adding test suites - Step 3 {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3}

The final step is collecting test fixtures in test suites. 
Test suites are again different from tests and test fixtures in their structure.

- A TestSuite is actually nothing more than a function returning the suite name. The trick is that the function will be placed inside a namespace, as well as all the test fixtures that belong inside it
- The `TestSuiteInfo` class holds the actual test suite information, such as the pointer to the next test suite, and the pointers to the first and last test fixture in the suite.

So the `TestSuiteInfo` class holds the administration of the test suite, like `TestFixtureInfo` and `TestBase` do for test fixtures and tests.

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_TESTSUITEINFOH}

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

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_TESTFIXTUREINFOH}

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

### TestSuiteInfo.cpp {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_TESTSUITEINFOCPP}

Let's implement the `TestSuiteInfo` class.
Create the file `code/libraries/unittest/src/TestSuiteInfo.cpp`

```cpp
File: code/libraries/unittest/src/TestSuiteInfo.cpp
File: d:\Projects\baremetal.github\code\libraries\unittest\src\TestSuiteInfo.cpp
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
78:         LOG_DEBUG("Fixture %s not found, creating new object", fixtureName.empty() ? "-" : fixtureName);
79: #endif
80:         testFixture = new TestFixtureInfo(fixtureName);
81:         AddFixture(testFixture);
82:     }
83:     else
84:     {
85: #ifdef DEBUG_REGISTRY
86:         LOG_DEBUG("Fixture %s found", fixtureName.empty() ? "-" : fixtureName);
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
- Line 122-132: We implement the `CountTests()` method. This goes through the list of test fixtures, and counts the tests in each of them

### TestSuite.h {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_TESTSUITEH}

The header for the test suite is quite simple, it simple defines the global function `GetSuiteName()`, which is used when not in a namespace.
Create the file `code/libraries/unittest/include/unittest/TestSuite.h`

```cpp
File: code/libraries/unittest/include/unittest/TestSuite.h
File: d:\Projects\baremetal.github\code\libraries\unittest\include\unittest\TestSuite.h
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

### Update CMake file {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_UPDATE_CMAKE_FILE}

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

### Update application code {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_UPDATE_APPLICATION_CODE}

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
54:     explicit FixtureMyTest1Helper(unittest::TestDetails const& details)
55:         : m_details{ details }
56:     {
57:         SetUp();
58:     }
59:     virtual ~FixtureMyTest1Helper()
60:     {
61:         TearDown();
62:     }
63:     void RunImpl() const;
64:     unittest::TestDetails const& m_details;
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
116:     explicit FixtureMyTest2Helper(unittest::TestDetails const& details)
117:         : m_details{ details }
118:     {
119:         SetUp();
120:     }
121:     virtual ~FixtureMyTest2Helper()
122:     {
123:         TearDown();
124:     }
125:     void RunImpl() const;
126:     unittest::TestDetails const& m_details;
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
171:     explicit FixtureMyTest3Helper(unittest::TestDetails const& details)
172:         : m_details{ details }
173:     {
174:         SetUp();
175:     }
176:     virtual ~FixtureMyTest3Helper()
177:     {
178:         TearDown();
179:     }
180:     void RunImpl() const;
181:     unittest::TestDetails const& m_details;
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
- Line 90: We define the namespace Suite1
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
In the next step we'll create macros later to do this work for us.

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging. 

The application will run the tests in the test fixture, and therefore show the log output.
You'll se that for each test the `RunImpl()` method of `MyTest<x>` runs.
This then instantiates the `FixtureTest<x>Helper`, and its constructor runs the `FixtureMyTest` method `SetUp()`.
Then the `RunImpl()` of `FixtureTest<x>Helper` is run, and finally the class is destructed again, leading to the `FixtureMyTest` method `TearDown()` begin run.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Info   Starting up (System:201)
Debug  Hello World! (main:209)
Debug  Running MyTest1 in fixture FixtureMyTest1 in suite Suite1 (main:82)
Debug  FixtureMyTest1 SetUp (main:41)
Debug  MyTestHelper 1 (main:68)
Debug  FixtureMyTest1 TearDown (main:45)
Debug  Running MyTest2 in fixture FixtureMyTest2 in suite Suite2 (main:144)
Debug  FixtureMyTest2 SetUp (main:103)
Debug  MyTestHelper 2 (main:130)
Debug  FixtureMyTest2 TearDown (main:107)
Debug  Running MyTest3 in fixture FixtureMyTest3 in suite default (main:200)
Debug  FixtureMyTest3 SetUp (main:159)
Debug  MyTestHelper 3 (main:186)
Debug  FixtureMyTest3 TearDown (main:163)
Info   Wait 5 seconds (main:233)
Press r to reboot, h to halt, p to fail assertion and panic
```

## The rest {#TUTORIAL_17_UNIT_TESTS_THE_REST}

======================================================================================================

### CurrentTest.h {#TUTORIAL_17_UNIT_TESTS_THE_REST_CURRENTTESTH}

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

### ExecuteTest.h {#TUTORIAL_17_UNIT_TESTS_THE_REST_EXECUTETESTH}

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

### TestResults.h {#TUTORIAL_17_UNIT_TESTS_THE_REST_TESTRESULTSH}

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

### CurrentTest.cpp {#TUTORIAL_17_UNIT_TESTS_THE_REST_CURRENTTESTCPP}

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

### TestResults.cpp {#TUTORIAL_17_UNIT_TESTS_THE_REST_TESTRESULTSCPP}

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

### ITestReporter.h {#TUTORIAL_17_UNIT_TESTS_THE_REST_ITESTREPORTERH}

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

### TestSuiteInfo.h {#TUTORIAL_17_UNIT_TESTS_THE_REST_TESTSUITEINFOH}

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

### TestFixtureInfo.h {#TUTORIAL_17_UNIT_TESTS_THE_REST_TESTFIXTUREINFOH}

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

### TestSuiteInfo.cpp {#TUTORIAL_17_UNIT_TESTS_THE_REST_TESTSUITEINFOCPP}

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

### TestFixtureInfo.cpp {#TUTORIAL_17_UNIT_TESTS_THE_REST_TESTFIXTUREINFOCPP}

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

### TestRegistry.h {#TUTORIAL_17_UNIT_TESTS_THE_REST_TESTREGISTRYH}

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

### Application code {#TUTORIAL_17_UNIT_TESTS_THE_REST_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_17_UNIT_TESTS_THE_REST_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will print the characters in the string "bbbb" in log statements.
Next to this nothing special is shown, as the tests will all succeed. If a test were to fail, the assertion would fire, and the application would crash.
This is a way to test code, however we'd like to see all of the failure found in our code, not have the first one crash the application.
This is the subject of the next tutorial.

```text
```

Next: [16-unit-tests](16-unit-tests.md)

