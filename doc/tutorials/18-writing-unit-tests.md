# Tutorial 18: Writing unit tests {#TUTORIAL_18_WRITING_UNIT_TESTS}

@tableofcontents

## New tutorial setup {#TUTORIAL_18_WRITING_UNIT_TESTS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/17-unit-tests`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_18_WRITING_UNIT_TESTS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-17.a`
- an application `output/Debug/bin/17-unit-tests.elf`
- an image in `deploy/Debug/17-unit-tests-image`

## Creating a framework for unit testing {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_A_FRAMEWORK_FOR_UNIT_TESTING}

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

As there is a lot of work to be done, this tutorial is split up into two parts:
- Createing an infrastructure for unit tests
- Creating macros to write unit tests and actually creating our first tests

Let's start off by implementing a simple test class.

## Creating the unittest library - Step 1 {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1}

We'll place the unit testing functionality in a new library, named `unittest`.
We'll also use the same convention of using the namespace `unittest` inside this library.

The structure is similar to what we set up for `baremetal` (see [Creating the baremetal library structure - Step 1](#TUTORIAL_05_FIRST_APPLICATION__USING_THE_CONSOLE__UART1_CREATING_THE_BAREMETAL_LIBRARY_STRUCTURE__STEP_1))

<img src="images/treeview-unittest-library.png" alt="Tree view" width="300"/>

### CMake file for unittest {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_CMAKE_FILE_FOR_UNITTEST}

We need to create the unittest project CMake file.

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

### Update libraries CMake file {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_UPDATE_LIBRARIES_CMAKE_FILE}

First we need to include the unittest project in the libraries CMake file.

Update the file `code/libraries/CMakeLists.txt`

```cmake
message(STATUS "\n**********************************************************************************\n")
message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(baremetal)
add_subdirectory(unittest)
```

### TestDetails.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTDETAILSH}

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

### TestDetails.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTDETAILSCPP}

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

### TestBase.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTBASEH}

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

- Line 47-73: We declare the class `TestBase`
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

### TestBase.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_TESTBASECPP}

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

### Update CMake file {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_UPDATE_CMAKE_FILE}

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

### Update application code {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_UPDATE_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_THE_UNITTEST_LIBRARY__STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

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

## Adding test fixtures - Step 2 {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2}

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

### TestFixtureInfo.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_TESTFIXTUREINFOH}

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
42: #include <unittest/TestBase.h>
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
67:     TestBase* GetHead() const { return m_head; }
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

### TestBase.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_TESTBASEH}

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

### TestFixtureInfo.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_TESTFIXTUREINFOCPP}

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
43: using namespace baremetal;
44:
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
81: void TestFixtureInfo::Run()
82: {
83:     TestBase* test = this->GetHead();
84:     while (test != nullptr)
85:     {
86:         test->Run();
87:         test = test->m_next;
88:     }
89: }
90:
91: int TestFixtureInfo::CountTests()
92: {
93:     int numberOfTests = 0;
94:     TestBase* test = m_head;
95:     while (test != nullptr)
96:     {
97:         ++numberOfTests;
98:         test = test->m_next;
99:     }
100:     return numberOfTests;
101: }
102:
103: } // namespace unittest
```

- Line 47-53: We implement the constructor
- Line 55-64: We implement the destructor. This goes through the list of tests, and deletes every one of these. Note that we will therefore need to create the tests on the heap.
- Line 66-79: We implement the `AddTest()` method. This will add the test passed in at the end of the list
- Line 81-84: We implement the `GetHead()` method
- Line 86-94: We implement the `Run()` method. This goes through the list of tests, and calls `Run()` on each
- Line 96-106: We implement the `CountTests()` method. This goes through the list of tests, and counts them

### TestFixture.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_TESTFIXTUREH}

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

### Update CMake file {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_UPDATE_CMAKE_FILE}

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

### Update application code {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_UPDATE_APPLICATION_CODE}

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
45:     explicit FixtureMyTest1Helper(const TestDetails& details)
46:         : m_details{ details }
47:     {
48:         SetUp();
49:     }
50:     virtual ~FixtureMyTest1Helper()
51:     {
52:         TearDown();
53:     }
54:     void RunImpl() const;
55:     const TestDetails& m_details;
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
67:     explicit FixtureMyTest2Helper(const TestDetails& details)
68:         : m_details{ details }
69:     {
70:         SetUp();
71:     }
72:     virtual ~FixtureMyTest2Helper()
73:     {
74:         TearDown();
75:     }
76:     void RunImpl() const;
77:     const TestDetails& m_details;
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
89:     explicit FixtureMyTest3Helper(const TestDetails& details)
90:         : m_details{ details }
91:     {
92:         SetUp();
93:     }
94:     virtual ~FixtureMyTest3Helper()
95:     {
96:         TearDown();
97:     }
98:     void RunImpl() const;
99:     const TestDetails& m_details;
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

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_FIXTURES__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests in the test fixture, and therefore show the log output.
You'll see that for each test the `RunImpl()` method of `MyTest<x>` runs.
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

## Adding test suites - Step 3 {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3}

The final step is collecting test fixtures in test suites.
Test suites are again different from tests and test fixtures in their structure.

- A TestSuite is actually nothing more than a function returning the suite name. The trick is that the function will be placed inside a namespace, as well as all the test fixtures that belong inside it
- The `TestSuiteInfo` class holds the actual test suite information, such as the pointer to the next test suite, and the pointers to the first and last test fixture in the suite.

So the `TestSuiteInfo` class holds the administration of the test suite, like `TestFixtureInfo` and `TestBase` do for test fixtures and tests.

### TestSuiteInfo.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_TESTSUITEINFOH}

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

### TestFixtureInfo.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_TESTFIXTUREINFOH}

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

### TestSuiteInfo.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_TESTSUITEINFOCPP}

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

### TestSuite.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_TESTSUITEH}

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

### Update CMake file {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_UPDATE_CMAKE_FILE}

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

### Update application code {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_UPDATE_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_TEST_SUITES__STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

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

## Test registration - Step 4 {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_REGISTRATION__STEP_4}

Now that we have all the classes defined to run tests, we still need to find a way to register them, such that we can ask a test runner to run all (or part of) the tests.
For this we will define a class `TestRegistry`, which will keep administration at the top level (the list of all test suites).
We will also define a class `TestRegistrar` which is used to register tests statically, so that before the application runs, the tests have already been registered.

### TestRegistry.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_REGISTRATION__STEP_4_TESTREGISTRYH}

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

### TestSuiteInfo.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_REGISTRATION__STEP_4_TESTSUITEINFOH}

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

### TestRegistry.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_REGISTRATION__STEP_4_TESTREGISTRYCPP}

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

### Debugging registry {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_REGISTRATION__STEP_4_DEBUGGING_REGISTRY}

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

### Update CMake file {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_REGISTRATION__STEP_4_UPDATE_CMAKE_FILE}

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

### Update application code {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_REGISTRATION__STEP_4_UPDATE_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_REGISTRATION__STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

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

## Test runner and visitor - Step 5 {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5}

### ITestReporter.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_ITESTREPORTERH}

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

### TestResults.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTRESULTSH}

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

### TestResults.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTRESULTSCPP}

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

### TestRunner.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTRUNNERH}

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

### TestRunner.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTRUNNERCPP}

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

### TestRegistry.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTREGISTRYH}

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

### TestRegistry.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTREGISTRYCPP}

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

### TestSuiteInfo.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTSUITEINFOH}

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

### TestSuiteInfo.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTSUITEINFOCPP}

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

### TestFixtureInfo.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTFIXTUREINFOH}

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

### TestFixtureInfo.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTFIXTUREINFOCPP}

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

### TestBase.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTBASEH}

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

### TestBase.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_TESTBASECPP}

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

### CurrentTest.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_CURRENTTESTH}

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

### CurrentTest.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_CURRENTTESTCPP}

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

### ExecuteTest.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_EXECUTETESTH}

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

### ConsoleTestReporter.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_CONSOLETESTREPORTERH}

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

### ConsoleTestReporter.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_CONSOLETESTREPORTERCPP}

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

### unittest.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_UNITTESTH}

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

### Update project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_UPDATE_PROJECT_CONFIGURATION}

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

### Application code {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_RUNNER_AND_VISITOR__STEP_5_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

You will see all the output being generared by the console test reported using colors.

<img src="images/demo-output-unit-test.png" alt="Tree view" width="800"/>

Next: [18-exceptions-and-interrupts](18-exceptions-and-interrupts.md)


