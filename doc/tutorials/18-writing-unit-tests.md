# Tutorial 18: Writing unit tests {#TUTORIAL_18_WRITING_UNIT_TESTS}

@tableofcontents

## New tutorial setup {#TUTORIAL_18_WRITING_UNIT_TESTS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/18-writing-unit-tests`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_18_WRITING_UNIT_TESTS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-18.a`
- an application `output/Debug/bin/18-writing-unit-tests.elf`
- an image in `deploy/Debug/18-writing-unit-tests-image`

## Creating actual unit tests {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS}

As said in the previous tutorial, in tutorials [15-string](#TUTORIAL_15_STRING) and [16-serializing-and-formatting](#TUTORIAL_16_SERIALIZATION_AND_FORMATTING) we have been adding quite some code, and added assertion to verify correctness.

We'll now focus on creating macros to create test suites, test fixtures and test, and for creating actual test cases.
When that is done, we'll convert the tests in the two mentioned tutorials to actual class tests.

## Adding macros for creating tests, fixtures and suites - Step 1 {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1}

### TestMacros.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_TESTMACROSH}

The most cumbersome about the current implementation is that we need to be very verbose when writing tests, test fixture and test suites.
We can easily solve this by creating some macros.

Create the file `code/libraries/unittest/include/unittest/TestMacros.h`

```cpp
File: code/libraries/unittest/include/unittest/TestMacros.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : TestMacros.h
5: //
6: // Namespace   : unittest
7: //
8: // Class       : -
9: //
10: // Description : Macros for specifying tests
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
43: /// Test macros
44: 
45: /// @brief Register test
46: ///
47: /// Register the test named TestName with the registry Registry. This will declare a test class Test[TestName] and instantiate it as test[TestName]Instance.
48: /// It will also instantiate a TestRegistrar which will register the test instance with the test registry.
49: /// It will then define the RunImpl() method of the Test[TestName] class, which is formed by the following code between curly brackets
50: #define TEST_EX(TestName, Registry)                                                                         \
51: class Test##TestName : public unittest::Test                                                                \
52: {                                                                                                           \
53: private:                                                                                                    \
54:     virtual void RunImpl() const override;                                                                  \
55: } test##TestName##Instance;                                                                                 \
56:                                                                                                             \
57: static unittest::TestRegistrar registrar##TestName(Registry, &test##TestName##Instance, unittest::TestDetails(baremetal::string(#TestName), baremetal::string(""), baremetal::string(GetSuiteName()), baremetal::string(__FILE__), __LINE__)); \
58:                                                                                                             \
59: void Test##TestName::RunImpl() const
60: 
61: /// @brief Register test
62: ///
63: /// Register the test named TestName with the singleton test registry. This will use the TEST_EX macro to do the actual test registration
64: #define TEST(TestName) TEST_EX(TestName, unittest::TestRegistry::GetTestRegistry())
65: 
66: /// @brief Register test inside test fixture
67: ///
68: /// Register the test inside a test fixture named FixtureClass for the test named TestName with the registry Registry.
69: /// This will declare a class [FixtureClass][TestName]Helper, which calls the Setup() method in the constructor, and the TearDown() method in the destructor.
70: /// It also declares a test class Test[FixtureClass][TestName] and instantiate it as test[FixtureClass][TestName]Instance.
71: /// It will also instantiate a TestRegistrar which will register the test  with the test registry.
72: /// The RunImpl() method of the Test[FixtureClass][TestName] is implemented as creating an instance of the test fixture, and invoking its RunImpl() method.
73: /// It will then define the RunImpl() method of the Test[FixtureClass][TestName] class, which is formed by the following code between curly brackets
74: #define TEST_FIXTURE_EX(FixtureClass,TestName,Registry)                                                     \
75: class FixtureClass##TestName##Helper : public FixtureClass                                                  \
76: {                                                                                                           \
77: public:                                                                                                     \
78:     FixtureClass##TestName##Helper(const FixtureClass##TestName##Helper &) = delete;                        \
79:     explicit FixtureClass##TestName##Helper(unittest::TestDetails const & details) :                        \
80:         m_details{ details }                                                                                \
81:     {                                                                                                       \
82:         SetUp();                                                                                            \
83:     }                                                                                                       \
84:     virtual ~FixtureClass##TestName##Helper()                                                               \
85:     {                                                                                                       \
86:         TearDown();                                                                                         \
87:     }                                                                                                       \
88:     FixtureClass##TestName##Helper & operator = (const FixtureClass##TestName##Helper &) = delete;          \
89:     void RunImpl() const;                                                                                   \
90:     unittest::TestDetails const & m_details;                                                                \
91: };                                                                                                          \
92:                                                                                                             \
93: class Test##FixtureClass##TestName : public unittest::Test                                                  \
94: {                                                                                                           \
95: private:                                                                                                    \
96:     void RunImpl() const override;                                                                          \
97: } test##FixtureClass##TestName##Instance;                                                                   \
98:                                                                                                             \
99: unittest::TestRegistrar registrar##FixtureClass##TestName(Registry, &test##FixtureClass##TestName##Instance, TestDetails(baremetal::string(#TestName), baremetal::string(#FixtureClass), baremetal::string(GetSuiteName()), baremetal::string(__FILE__), __LINE__)); \
100:                                                                                                             \
101: void Test##FixtureClass##TestName::RunImpl() const                                                          \
102: {                                                                                                           \
103:     FixtureClass##TestName##Helper fixtureHelper(*CurrentTest::Details());                                  \
104:     fixtureHelper.RunImpl();                                                                                \
105: }                                                                                                           \
106: void FixtureClass##TestName##Helper::RunImpl() const
107: 
108: /// @brief Register test inside test fixture
109: ///
110: /// Register the test named TestName inside a test fixture class named Fixture with the singleton test registry. This will use the TEST_FIXTURE_EX macro to do the actual test registration
111: #define TEST_FIXTURE(FixtureClass,TestName) TEST_FIXTURE_EX(FixtureClass,TestName,unittest::TestRegistry::GetTestRegistry())
112: 
113: /// @brief Create test suite
114: ///
115: /// Creates a test suite named SuiteName. This simply creates a namespace inside which tests and test fixtures are placed
116: #define TEST_SUITE(SuiteName)                                                                               \
117:     namespace Suite##SuiteName                                                                              \
118:     {                                                                                                       \
119:         inline char const* GetSuiteName()                                                                   \
120:         {                                                                                                   \
121:             return baremetal::string(#SuiteName);                                                           \
122:         }                                                                                                   \
123:     }                                                                                                       \
124:     namespace Suite##SuiteName
```

- Line 50-59: We define the macro `TEST_EX`. This is used by macro `TEST`,
The parameters are the name of the test `TestName` and the reference to the singleton `TestRegistry` instance `Registry`.
- It declares the class `Test<TestName>`, which inherits from `Test`, and defines an instance named `test<TestName>Instance`.
- It then defines a `TestRegistrar` instance named `registrar<TestName>` which registers the instance `test<TestName>Instance`.
- It then starts the definition of the `RunImpl()` which is expected to be followed by the actual implementation of the test.
Compare this to the application code we wrote before:

```cpp
File: code/applications/demo/src/main.cpp
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
197:     CurrentTest::Results()->OnTestFailure(*CurrentTest::Details(), "Failure");
198: }
```

- Here:
    - `MyTest` is the `Test##TestName`
    - `myTest` is `test##TestName##Instance`
    - `registrarMyTest` is `registrar##TestName`

- Line 64: We define the macro `TEST` which uses `TEST_EX` to register the test named `TestName`.
The only parameter for `TEST` is `TestName`, the name of the test.
The `TEST` macro is intended for tests that do not belong to a test fixture.
The way this macro can be used is as follows:

```cpp
TEST(MyTest)
{
    // Test implementation
}
```

- Line 74-106: We define the macro `TEST_FIXTURE_EX`. This is used by macro `TEST_FIXTURE`,
The parameters are the name of the fixture class `FixtureClass`, the name of the test `TestName` and the reference to the singleton `TestRegistry` instance `Registry`.
- It declares the class `<FixtureClass><TestName>Helper`, which inherits from the `FixtureClass`, the class we defined for the fixture.
- It then declares a class `Test<FixtureClass><TestName>`, which inherits from `TestBase`, and defines an instance named `test<FixtureClass><TestName>Instance`.
- It then defines a `TestRegistrar` instance named `registrar<FixtureClass><TestName>` which registers the instance `test<FixtureClass><TestName>Instance`.
- It then defines the implement of `RunImpl()` for the class `Test<FixtureClass><TestName>`. This creates and instance of class `<FixtureClass><TestName>Helper`.
Through the constructor, the `SetUp()` method is called
- Finally, it then starts the definition of the `RunImpl()` for the class `<FixtureClass><TestName>Helper` which is expected to be followed by the actual implementation of the test.
Compare this to the application code we wrote before:
- When the `RunImpl()` method for the class `Test<FixtureClass><TestName>` exits, the desctructor for `<FixtureClass><TestName>Helper` is called, which runs the `TearDown()` method

```cpp
File: code/applications/demo/src/main.cpp
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
```

- Here
  - `FixtureMyTest3` is the `FixtureClass` we pass into the macro (so this still has to be defined)
  - `FixtureMyTest3Helper` is the class `FixtureClass##TestName##Helper`
  - `MyTest3` is `Test##FixtureClass##TestName`
  - `myTest3` is `test##FixtureClass##TestName##Instance`
  - `registrarFixtureMyTest3` is `registrar##FixtureClass##TestName`

- Line 113: We define the macro `TEST_FIXTURE` which uses `TEST_FIXTURE_EX` to register the test named `TestName` for test firture class `FixtureClass`.
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

- Line 118-126: We define the macro `TEST_SUITE` which defined a test suite. It create a namespace for the test suite named `SuiteName`, as wel the `GetSuiteName()` function inside the namespace.
The only parameter is the name of the test suite `SuiteName`.
The `TEST_SUITE` macro is intended for create a test suite around tests and test fixtures.
Compare the defintion of this macro to the application code we wrote before:

```cpp
File: code/applications/demo/src/main.cpp
14: namespace Suite1 {
15: 
16: inline char const* GetSuiteName()
17: {
18:     return baremetal::string("Suite1");
19: }
...
72: } // namespace Suite1
```

The way this macro can be used is as follows:

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

### unittest.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_UNITTESTH}

We added a header file, so we'll update the `unittest.h` header

Update the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
...
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
56: #include <unittest/TestMacros.h>
57: #include <unittest/TestRegistry.h>
58: #include <unittest/TestResults.h>
59: #include <unittest/TestRunner.h>
60: #include <unittest/TestSuiteInfo.h>
```

### Update project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_UPDATE_PROJECT_CONFIGURATION}

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
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestMacros.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResult.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRunner.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
61:     )
62: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_APPLICATION_CODE}

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
14: TEST_SUITE(Suite1)
15: {
16: 
17: class FixtureMyTest1
18:     : public TestFixture
19: {
20: public:
21:     void SetUp() override
22:     {
23:         LOG_DEBUG("MyTest SetUp");
24:     }
25:     void TearDown() override
26:     {
27:         LOG_DEBUG("MyTest TearDown");
28:     }
29: };
30: 
31: TEST_FIXTURE(FixtureMyTest1, MyTest1)
32: {
33:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 1");
34: }
35: 
36: } // Suite1
37: 
38: TEST_SUITE(Suite2)
39: {
40: 
41: class FixtureMyTest2
42:     : public TestFixture
43: {
44: public:
45:     void SetUp() override
46:     {
47:         LOG_DEBUG("FixtureMyTest2 SetUp");
48:     }
49:     void TearDown() override
50:     {
51:         LOG_DEBUG("FixtureMyTest2 TearDown");
52:     }
53: };
54: 
55: TEST_FIXTURE(FixtureMyTest2, MyTest2)
56: {
57:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 2");
58: }
59: 
60: } // Suite2
61: 
62: class FixtureMyTest3
63:     : public TestFixture
64: {
65: public:
66:     void SetUp() override
67:     {
68:         LOG_DEBUG("FixtureMyTest3 SetUp");
69:     }
70:     void TearDown() override
71:     {
72:         LOG_DEBUG("FixtureMyTest3 TearDown");
73:     }
74: };
75: 
76: TEST_FIXTURE(FixtureMyTest3, MyTest3)
77: {
78:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 3");
79: }
80: 
81: TEST(MyTest)
82: {
83:     LOG_DEBUG("Running test");
84:     CurrentTest::Results()->OnTestFailure(*CurrentTest::Details(), "Failure");
85: }
86: 
87: int main()
88: {
89:     auto& console = GetConsole();
90: 
91:     ConsoleTestReporter reporter;
92:     RunAllTests(&reporter);
93: 
94:     LOG_INFO("Wait 5 seconds");
95:     Timer::WaitMilliSeconds(5000);
96: 
97:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
98:     char ch{};
99:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
100:     {
101:         ch = console.ReadChar();
102:         console.WriteChar(ch);
103:     }
104:     if (ch == 'p')
105:         assert(false);
106: 
107:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
108: }
```

- Line 14: We replace the namespace Suite1 and its `GetSuiteName()` function by the macro `TEST_SUITE`
- Line 31: We replace the declaration and definition of classes `FixtureMyTest1Helper` and `MyTest1` by the macro `TEST_FIXTURE`.
Note that we still need to defined the class `FixtureMyTest1` as it defines common behaviour for all tests in the test fixture
- Line 38: We replace the namespace Suite2 and its `GetSuiteName()` function by the macro `TEST_SUITE`
- Line 55: We replace the declaration and definition of classes `FixtureMyTest2Helper` and `MyTest2` by the macro `TEST_FIXTURE`.
Note that we still need to defined the class `FixtureMyTest2` as it defines common behaviour for all tests in the test fixture
- Line 76: We replace the declaration and definition of classes `FixtureMyTest3Helper` and `MyTest3` by the macro `TEST_FIXTURE`.
Note that we still need to defined the class `FixtureMyTest3` as it defines common behaviour for all tests in the test fixture
- Line 81: We replace the declaration and definition of class `MyTest` by the macro `TEST`.

### CMakeLists.txt {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_CMAKELISTSTXT}

We still have debug tracing for the test registration, which is no longer needed, so let's switch that off now.

Update the file `CMakeLists.txt`

```cpp
File: CMakeLists.txt
67: option(BAREMETAL_DEBUG_UNITTEST_REGISTRY "Enable debug tracing output for unittest registry" OFF)
```

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. As we have one failure in `Test3`, and two in `Test4`, we expect to see two failing tests, and three failures in total.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:201)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
Debug  Suite1::FixtureMyTest1::MyTest1MyTestHelper 1 (main:33)
[ SUCCEEDED ] Suite1::FixtureMyTest1::MyTest1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
Debug  Suite2::FixtureMyTest2::MyTest2MyTestHelper 2 (main:57)
[ SUCCEEDED ] Suite2::FixtureMyTest2::MyTest2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
Debug  DefaultSuite::FixtureMyTest3::MyTest3MyTestHelper 3 (main:78)
[ SUCCEEDED ] DefaultSuite::FixtureMyTest3::MyTest3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
Debug  Running test (main:83)
[  FAILED   ] DefaultSuite::DefaultFixture::MyTest
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 1 out of 4 tests failed (1 failure).

Failures:
../code/applications/demo/src/main.cpp:81 : Failure in DefaultSuite::DefaultFixture::MyTest: Failure

[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   Wait 5 seconds (main:94)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Creating test cases - Step 2 {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES___STEP_2}

So far we have been focusing on the structure for running tests, but not on the actual tests.
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

Let's create macros to perform these checks, and and a mechanism to trace back the reason for a failure, for example when we expect to have a certain value, what the expected and actual values were.

We'll define a set of macros, and we'll need to do some trickery to get it all to work well. So bare with me.

We'll start with simple boolean tests, and equality / inequality of integral types. The rest will follow in the next and final step.

### Checks.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES___STEP_2_CHECKSH}

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
8: // Class       : -
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
46: /// @file
47: /// Assertion checks
48: 
49: namespace unittest
50: {
51: 
52: class TestResults;
53: class TestDetails;
54: 
55: /// <summary>
56: /// Assertion result
57: ///
58: /// Contains the result of an assertion, flags whether it failed, and what the message was
59: /// </summary>
60: struct AssertionResult
61: {
62:     /// <summary>
63:     /// Constructor
64:     /// </summary>
65:     /// <param name="failed">If true, the assertion failed, if false the assertion was successful</param>
66:     /// <param name="message">Message for the assertion</param>
67:     AssertionResult(bool failed, const baremetal::string& message)
68:         : failed(failed)
69:         , message(message)
70:     {
71:     }
72:     /// @brief If true, the assertion failed, if false the assertion was successful
73:     const bool failed;
74:     /// @brief Message for the assertion
75:     const baremetal::string message;
76:     /// @brief bool case operator.
77:     /// @return true, if the assertion failed, false if the assertion was successful
78:     operator bool() const { return failed; }
79: };
80: 
81: extern AssertionResult AssertionSuccess();
82: extern AssertionResult GenericFailure(const baremetal::string& message);
83: extern AssertionResult BooleanFailure(const baremetal::string& valueExpression,
84:                                       const baremetal::string& expectedValue,
85:                                       const baremetal::string& actualValue);
86: extern AssertionResult EqFailure(const baremetal::string& expectedExpression,
87:                                  const baremetal::string& actualExpression,
88:                                  const baremetal::string& expectedValue,
89:                                  const baremetal::string& actualValue);
90: extern AssertionResult InEqFailure(const baremetal::string& expectedExpression,
91:                                    const baremetal::string& actualExpression,
92:                                    const baremetal::string& expectedValue,
93:                                    const baremetal::string& actualValue);
94: 
95: /// <summary>
96: /// Check whether argument is evaluated as true
97: /// </summary>
98: /// <typeparam name="Value">Type of argument</typeparam>
99: /// <param name="value">Argument</param>
100: /// <returns>Returns true if the argument can be evaluated as true, false otherwise</returns>
101: template<typename Value>
102: bool CheckTrue(const Value value)
103: {
104:     return !!value;
105: }
106: 
107: /// <summary>
108: /// Check whether argument is evaluated as false
109: /// </summary>
110: /// <typeparam name="Value">Type of argument</typeparam>
111: /// <param name="value">Argument</param>
112: /// <returns>Returns true if the argument can be evaluated as false, false otherwise</returns>
113: template<typename Value>
114: bool CheckFalse(const Value value)
115: {
116:     return !value;
117: }
118: 
119: /// <summary>
120: /// Utility class to convert a value to a string for comparison
121: /// </summary>
122: /// <typeparam name="ToPrint">Type of value to be converted to string</typeparam>
123: template <typename ToPrint>
124: class FormatForComparison
125: {
126: public:
127:     /// <summary>
128:     /// Convert value to a string
129:     /// </summary>
130:     /// <param name="value">Value to be converted to string</param>
131:     /// <returns>Resulting string</returns>
132:     static baremetal::string Format(const ToPrint& value)
133:     {
134:         return PrintToString(value);
135:     }
136: };
137: 
138: /// <summary>
139: /// Format a value for a failure message
140: /// </summary>
141: /// <typeparam name="T1">Type of value to be formatted</typeparam>
142: /// <param name="value">Value to be formatted</param>
143: /// <returns>Resulting string</returns>
144: template <typename T1>
145: baremetal::string FormatForComparisonFailureMessage(const T1& value)
146: {
147:     return FormatForComparison<T1>::Format(value);
148: }
149: 
150: /// <summary>
151: /// Evaluate whether a value can be evaluated as true, generate a success object if successful, otherwise a failure object
152: /// </summary>
153: /// <typeparam name="Value">Type of value to be evaluated</typeparam>
154: /// <param name="valueName">String representation of value for failure message</param>
155: /// <param name="value">Value to be evaluated</param>
156: /// <returns>Result object</returns>
157: template<typename Value>
158: AssertionResult CheckTrue(const baremetal::string& valueName, const Value& value)
159: {
160:     if (!CheckTrue(value))
161:     {
162:         return BooleanFailure(valueName,
163:                               baremetal::string("true"),
164:                               PrintToString(value));
165:     }
166:     return AssertionSuccess();
167: }
168: 
169: /// <summary>
170: /// Evaluate whether a value can be evaluated as false, generate a success object if successful, otherwise a failure object
171: /// </summary>
172: /// <typeparam name="Value">Type of value to be evaluated</typeparam>
173: /// <param name="valueName">String representation of value for failure message</param>
174: /// <param name="value">Value to be evaluated</param>
175: /// <returns>Result object</returns>
176: template<typename Value>
177: AssertionResult CheckFalse(const baremetal::string& valueName, const Value& value)
178: {
179:     if (!CheckFalse(value))
180:     {
181:         return BooleanFailure(valueName,
182:                               baremetal::string("false"),
183:                               PrintToString(value));
184:     }
185:     return AssertionSuccess();
186: }
187: 
188: /// <summary>
189: /// Compare an expected and actual value
190: /// </summary>
191: /// <typeparam name="Expected">Type of expected value</typeparam>
192: /// <typeparam name="Actual">Type of actual value</typeparam>
193: /// <param name="expected">Expected value</param>
194: /// <param name="actual">Actual value</param>
195: /// <returns>True if the values are considered equal, false otherwise</returns>
196: template<typename Expected, typename Actual>
197: bool AreEqual(const Expected& expected, const Actual& actual)
198: {
199:     return (expected == actual);
200: }
201: 
202: /// <summary>
203: /// Evaluate whether an expected value is equal to an actual value, generate a success object if successful, otherwise a failure object
204: /// </summary>
205: /// <typeparam name="Expected">Type of the expected value</typeparam>
206: /// <typeparam name="Actual">Type of the actual value</typeparam>
207: /// <param name="expectedExpression">String representation of the expected value</param>
208: /// <param name="actualExpression">String representation of the actual value</param>
209: /// <param name="expected">Expected value</param>
210: /// <param name="actual">Actual value</param>
211: /// <returns>Result object</returns>
212: template< typename Expected, typename Actual >
213: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
214:                                    const Expected& expected, const Actual& actual)
215: {
216:     if (!AreEqual(expected, actual))
217:     {
218:         return EqFailure(expectedExpression, actualExpression,
219:                          FormatForComparisonFailureMessage(expected),
220:                          FormatForComparisonFailureMessage(actual));
221:     }
222:     return AssertionSuccess();
223: }
224: 
225: /// <summary>
226: /// Evaluate whether an expected value is not equal to an actual value, generate a success object if successful, otherwise a failure object
227: /// </summary>
228: /// <typeparam name="Expected">Type of the expected value</typeparam>
229: /// <typeparam name="Actual">Type of the actual value</typeparam>
230: /// <param name="expectedExpression">String representation of the not expected value</param>
231: /// <param name="actualExpression">String representation of the actual value</param>
232: /// <param name="expected">Expected value</param>
233: /// <param name="actual">Actual value</param>
234: /// <returns>Result object</returns>
235: template< typename Expected, typename Actual >
236: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
237:                                       const Expected& expected, const Actual& actual)
238: {
239:     if (AreEqual(expected, actual))
240:     {
241:         return InEqFailure(expectedExpression, actualExpression,
242:                            FormatForComparisonFailureMessage(expected),
243:                            FormatForComparisonFailureMessage(actual));
244:     }
245:     return AssertionSuccess();
246: }
247: 
248: /// <summary>
249: /// Helper class for {ASSERT|EXPECT}_EQ/NE
250: ///
251: /// Forms generalized mechanism for calling polymorphic check functions.
252: /// The template argument lhs_is_null_literal is true iff the first argument to ASSERT_EQ()
253: /// is a null pointer literal.  The following default implementation is
254: /// for lhs_is_null_literal being false.
255: /// </summary>
256: class EqHelper
257: {
258: public:
259:     /// <summary>
260:     /// Evaluate whether an expected value is equal to an actual value, generate a success object if successful, otherwise a failure object
261:     /// </summary>
262:     /// <typeparam name="Expected">Type of the expected value</typeparam>
263:     /// <typeparam name="Actual">Type of the actual value</typeparam>
264:     /// <param name="expectedExpression">String representation of the expected value</param>
265:     /// <param name="actualExpression">String representation of the actual value</param>
266:     /// <param name="expected">Expected value</param>
267:     /// <param name="actual">Actual value</param>
268:     /// <returns>Result object</returns>
269:     template <typename Expected, typename Actual>
270:     static AssertionResult CheckEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
271:                                       const Expected& expected, const Actual& actual)
272:     {
273:         return CheckEqualInternal(expectedExpression, actualExpression, expected, actual);
274:     }
275: 
276:     /// <summary>
277:     /// Evaluate whether an expected value is not equal to an actual value, generate a success object if successful, otherwise a failure object
278:     /// </summary>
279:     /// <typeparam name="Expected">Type of the expected value</typeparam>
280:     /// <typeparam name="Actual">Type of the actual value</typeparam>
281:     /// <param name="expectedExpression">String representation of the not expected value</param>
282:     /// <param name="actualExpression">String representation of the actual value</param>
283:     /// <param name="expected">Expected value</param>
284:     /// <param name="actual">Actual value</param>
285:     /// <returns>Result object</returns>
286:     template <typename Expected, typename Actual>
287:     static AssertionResult CheckNotEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
288:                                          const Expected& expected, const Actual& actual)
289:     {
290:         return CheckNotEqualInternal(expectedExpression, actualExpression, expected, actual);
291:     }
292: };
293: 
294: } // namespace unittest
```

- Line 60-79: We define a struct `AssertionResult` which holds the status for a single assertion.
  - Line 67-71: We define the constructor
  - Line 73: The member variable `failed` flags whether the assertion failed
  - Line 75: The member vairbale `message` holds the failure message, if any
  - Line 78: The `bool()` operator returns the value of `failed`
- Line 81: We declare a function `AssertionSuccess()` to signify a successful assert
- Line 82: We declare a function `GenericFailure()` to signify a generic assertion failure (e.g. `FAIL`)
- Line 83: We declare a function `BooleanFailure()` to signify a assertion failure on a boolean expression (e.g. `ASSERT_FALSE`, `EXPECT_TRUE`)
- Line 86: We declare a function `EqFailure()` to signify a assertion failure on a equality expression (e.g. `EXPECT_EQ`)
- Line 90: We declare a function `InEqFailure()` to signify a assertion failure on a inequality expression (e.g. `EXPECT_NE`)
- Line 101-105: We declare a template function `CheckTrue()` to convert a value to a boolean, returning true if the value is true, or not equal to 0
- Line 113-117: We define a template function `CheckFalse()` to convert a value to a boolean, returning true if the value is false, or equal to 0
- Line 123-136: We define a template class `FormatForComparison` which has a single method `Format`, which prints the value passed to a string.
The printing is done using functionality in the header `PrintValue.h` which we'll get to in a minute
- Line 144-148: We define a template function `FormatForComparisonFailureMessage()` which uses the `FormatForComparison` class to print the value passed
- Line 157-167: We define a template function `CheckTrue()` which takes a value and its stringified version, and uses the `CheckTrue()` defined before to check whether the value is seen as true.
If so, `AssertionSuccess()` is returned, otherwise, `BooleanFailure()` is returned
- Line 176-186: We define a template function `CheckFalse()` which takes a value and its stringified version, and uses the `CheckFalse()` defined before to check whether the value is seen as false.
If so, `AssertionSuccess()` is returned, otherwise, `BooleanFailure()` is returned
- Line 196-200: We define a template function `AreEqual()` to compare two values `expected` and `actual`, which can have different types.
The function uses the equality operator to determine if the two values are equal
- Line 212-223: We define a template function `CheckEqualInternal()`, which uses `AreEqual()` to compare two values `expected` and `actual`. Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 235-246: We define a template function `CheckNotEqualInternal()`, which uses `AreEqual()` to compare two values `expected` and `actual`. Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered inequal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 256-293: We declare a class `EqHelper` which has two methods `CheckEqual()` (which uses `CheckEqualInternal()`) and `CheckNotEqual()` (which uses `CheckNotEqualInternal()`)

### Checks.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES___STEP_2_CHECKSCPP}

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
8: // Class       : -
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
49: /// <summary>
50: /// Create a success object
51: /// </summary>
52: /// <returns>Result object</returns>
53: AssertionResult AssertionSuccess()
54: {
55:     return AssertionResult(false, string());
56: }
57: 
58: /// <summary>
59: /// Create a generic failure object with the provided message
60: /// </summary>
61: /// <param name="message">Message to be included</param>
62: /// <returns>Result object</returns>
63: AssertionResult GenericFailure(const string& message)
64: {
65:     return AssertionResult(true, message);
66: }
67: 
68: /// <summary>
69: /// Create a boolean failure object
70: /// </summary>
71: /// <param name="valueExpression">String representation of the actual value</param>
72: /// <param name="expectedValue">Expected value</param>
73: /// <param name="actualValue">Actual value</param>
74: /// <returns>Result object</returns>
75: AssertionResult BooleanFailure(const string& valueExpression, const string& expectedValue, const string& actualValue)
76: {
77:     string result = Format("Value of: %s", valueExpression.c_str());
78:     if (actualValue != valueExpression)
79:     {
80:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
81:     }
82: 
83:     result.append(Format("\n  Expected: %s\n", expectedValue.c_str()));
84: 
85:     return AssertionResult(true, result);
86: }
87: 
88: /// <summary>
89: /// Create a equality comparison failure object
90: /// </summary>
91: /// <param name="expectedExpression">String representation of the expected value</param>
92: /// <param name="actualExpression">String representation of the actual value</param>
93: /// <param name="expectedValue">Expected value</param>
94: /// <param name="actualValue">Actual value</param>
95: /// <returns>Result object</returns>
96: AssertionResult EqFailure(const string& expectedExpression, const string& actualExpression, const string& expectedValue, const string& actualValue)
97: {
98:     string result = Format("Value of: %s", actualExpression.c_str());
99:     if (actualValue != actualExpression)
100:     {
101:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
102:     }
103: 
104:     result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
105:     if (expectedValue != expectedExpression)
106:     {
107:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
108:     }
109:     result.append("\n");
110: 
111:     return AssertionResult(true, result);
112: }
113: 
114: /// <summary>
115: /// Create a inequality comparison failure object
116: /// </summary>
117: /// <param name="expectedExpression">String representation of the not expected value</param>
118: /// <param name="actualExpression">String representation of the actual value</param>
119: /// <param name="expectedValue">Expected value</param>
120: /// <param name="actualValue">Actual value</param>
121: /// <returns>Result object</returns>
122: AssertionResult InEqFailure(const string& expectedExpression, const string& actualExpression, const string& expectedValue, const string& actualValue)
123: {
124:     string result = Format("Value of: %s", actualExpression.c_str());
125:     if (actualValue != actualExpression)
126:     {
127:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
128:     }
129: 
130:     result.append(Format("\n  Expected not equal to: %s", expectedExpression.c_str()));
131:     if (expectedValue != expectedExpression)
132:     {
133:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
134:     }
135:     result.append("\n");
136: 
137:     return AssertionResult(true, result);
138: }
139: 
140: } // namespace unittest
```

- Line 53-56: We implement the function `AssertionSuccess()`. It will return an assertion result flagging no failure
- Line 63-66: We implement the function `GenericFailure()`. This will return an assertion result flagging a failure, with a string explaining the reason
- Line 75-86: We implement the function `BooleanFailure()`. This will return an assertion result flagging a failure, with a string explaining that the actual value does not match the expected value
- Line 96-112: We implement the function `EqFailure()`. This will return an assertion result flagging a failure, with a string explaining that the actual value is not equal to the expected value
- Line 122-138: We implement the function `InEqFailure()`. This will return an assertion result flagging a failure, with a string explaining that the actual value is equal to the expected value

### PrintValue.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES___STEP_2_PRINTVALUEH}

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
45: /// @file
46: /// Print values to string
47: 
48: /// <summary>
49: /// Print a value to string using a serializer
50: /// </summary>
51: /// <typeparam name="T">Type of value to print</typeparam>
52: /// <param name="value">Value to print</param>
53: /// <param name="s">Resulting string</param>
54: template <typename T>
55: void PrintTo(const T& value, baremetal::string& s)
56: {
57:     s = baremetal::Serialize(value);
58: }
59: 
60: /// <summary>
61: /// Print a boolean value to string
62: /// </summary>
63: /// <param name="x">Value to print</param>
64: /// <param name="s">Resulting string</param>
65: inline void PrintTo(bool x, baremetal::string& s)
66: {
67:     s = (x ? "true" : "false");
68: }
69: 
70: /// <summary>
71: /// Universal printer class, using PrintTo()
72: /// </summary>
73: /// <typeparam name="T">Type value to print</typeparam>
74: template <typename T>
75: class UniversalPrinter
76: {
77: public:
78:     /// <summary>
79:     /// Print a type T to string
80:     /// </summary>
81:     /// <param name="value">Value to print</param>
82:     /// <param name="s">Resulting string</param>
83:     static void Print(const T& value, baremetal::string& s)
84:     {
85:         // By default, ::testing::internal::PrintTo() is used for printing
86:         // the value.
87:         //
88:         // Thanks to Koenig look-up, if T is a class and has its own
89:         // PrintTo() function defined in its namespace, that function will
90:         // be visible here.  Since it is more specific than the generic ones
91:         // in ::testing::internal, it will be picked by the compiler in the
92:         // following statement - exactly what we want.
93:         PrintTo(value, s);
94:     }
95: };
96: 
97: /// <summary>
98: /// Universal print to string function, uses UniversalPrinter
99: /// </summary>
100: /// <typeparam name="T">Type value to print</typeparam>
101: /// <param name="value">Value to print</param>
102: /// <param name="s">Resulting string</param>
103: template <typename T>
104: void UniversalPrint(const T& value, baremetal::string& s)
105: {
106:     typedef T T1;
107:     UniversalPrinter<T1>::Print(value, s);
108: }
109: 
110: /// <summary>
111: /// Universal terse printer class, uses UniversalPrint
112: /// </summary>
113: /// <typeparam name="T">Type value to print</typeparam>
114: template <typename T>
115: class UniversalTersePrinter
116: {
117: public:
118:     /// <summary>
119:     /// Print a type T to string
120:     /// </summary>
121:     /// <param name="value">Value to print</param>
122:     /// <param name="s">Resulting string</param>
123:     static void Print(const T& value, baremetal::string& s)
124:     {
125:         UniversalPrint(value, s);
126:     }
127: };
128: /// <summary>
129: /// Universal terse printer class for reference, uses UniversalPrint
130: /// </summary>
131: /// <typeparam name="T">Type value to print</typeparam>
132: template <typename T>
133: class UniversalTersePrinter<T&>
134: {
135: public:
136:     /// <summary>
137:     /// Print a reference to string
138:     /// </summary>
139:     /// <param name="value">Value to print</param>
140:     /// <param name="s">Resulting string</param>
141:     static void Print(const T& value, baremetal::string& s)
142:     {
143:         UniversalPrint(value, s);
144:     }
145: };
146: 
147: /// <summary>
148: /// String print, uses UniversalPrinter
149: /// </summary>
150: /// <typeparam name="T">Type value to print</typeparam>
151: /// <param name="value">Value to print</param>
152: /// <returns>Resulting string</returns>
153: template <typename T>
154: inline baremetal::string PrintToString(const T& value)
155: {
156:     baremetal::string s;
157:     UniversalTersePrinter<T>::Print(value, s);
158:     return s;
159: }
```

- Line 54-58: We define a template function `PrintTo()` which is a fallback to print any value to a string using a serializer. This uses the polymorphism of the `Serialize` functions.
- Line 65-68: We define a specialization to `PrintTo()` for boolean values
- Line 74-95: We declare a template class `UniversalPrinter` that has a single method `Print()` which calls any defined `PrintTo()` function for the value passed to a string
- Line 103-108: We define a template function `UniversalPrint` which uses the `Print()` method in the `UniversalPrinter` class
- Line 114-127: We declare a template class `UniversalTersePrinter` for type `T` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string
- Line 132-145: We declare a specialization of `UniversalTersePrinter` for type `T&` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string
- Line 153-159: We define a template function `PrintToString()` which uses the `Print()` method of any matching class `UniversalTersePrinter`

### AssertMacros.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES___STEP_2_ASSERTMACROSH}

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
45: /// @file
46: /// Assertion macros
47: 
48: #ifdef ASSERT_TRUE
49:     #error unittest redefines ASSERT_TRUE
50: #endif
51: 
52: #ifdef ASSERT_FALSE
53:     #error unittest redefines ASSERT_FALSE
54: #endif
55: 
56: #ifdef ASSERT_EQ
57:     #error unittest redefines ASSERT_EQ
58: #endif
59: 
60: #ifdef ASSERT_NE
61:     #error unittest redefines ASSERT_NE
62: #endif
63: 
64: #ifdef EXPECT_TRUE
65:     #error unittest redefines EXPECT_TRUE
66: #endif
67: 
68: #ifdef EXPECT_FALSE
69:     #error unittest redefines EXPECT_FALSE
70: #endif
71: 
72: #ifdef EXPECT_EQ
73:     #error unittest redefines EXPECT_EQ
74: #endif
75: 
76: #ifdef EXPECT_NE
77:     #error unittest redefines EXPECT_NE
78: #endif
79: 
80: /// @brief Generic expect macro. Checks if the argument is true, generates a failure if the check fails
81: #define UT_EXPECT_RESULT(value) \
82:     do \
83:     { \
84:         if (const ::unittest::AssertionResult UT_AssertionResult = (value)) \
85:             ::unittest::CurrentTest::Results()->OnTestFailure(::unittest::TestDetails(*::unittest::CurrentTest::Details(), __LINE__), UT_AssertionResult.message); \
86:     } while (0)
87: /// @brief Generic assert macro. Checks if the argument is true, generates a failure if the check fails, and throws an exception (not implemented yet)
88: #define UT_ASSERT_RESULT(value) \
89:     do \
90:     { \
91:         if (const ::unittest::AssertionResult UT_AssertionResult = (value)) \
92:         { \
93:             ::unittest::CurrentTest::Results()->OnTestFailure(::unittest::TestDetails(*::unittest::CurrentTest::Details(), __LINE__), UT_AssertionResult.message); \
94:             /*throw ::unittest::AssertionFailedException(__FILE__, __LINE__);*/ \
95:         } \
96:     } while (0)
97: 
98: /// @brief Expect predicate function with one parameter (CheckTrue, CheckFalse), generates a failure using UT_EXPECT_RESULT if predicate function returns false
99: #define EXPECT_PRED_FORMAT1(pred_format, v1) \
100:   UT_EXPECT_RESULT(pred_format(baremetal::string(#v1), v1))
101: /// @brief Assert predicate function with one parameter (CheckTrue, CheckFalse), generates a failure using UT_ASSERT_RESULT if predicate function returns false
102: #define ASSERT_PRED_FORMAT1(pred_format, v1) \
103:   UT_ASSERT_RESULT(pred_format(baremetal::string(#v1), v1))
104: 
105: /// @brief Expect predicate function with two parameters (CheckEqual(IgnoreCase), CheckNotEqual(IgnoreCase)), generates a failure using UT_EXPECT_RESULT if predicate function returns false
106: #define EXPECT_PRED_FORMAT2(pred_format, v1, v2) \
107:   UT_EXPECT_RESULT(pred_format(baremetal::string(#v1), baremetal::string(#v2), v1, v2))
108: /// @brief Assert predicate function with two parameters (CheckEqual(IgnoreCase), CheckNotEqual(IgnoreCase)), generates a failure using UT_ASSERT_RESULT if predicate function returns false
109: #define ASSERT_PRED_FORMAT2(pred_format, v1, v2) \
110:   UT_ASSERT_RESULT(pred_format(baremetal::string(#v1), baremetal::string(#v2), v1, v2))
111: 
112: /// @brief Force failure with message
113: #define FAIL(message) UT_EXPECT_RESULT(GenericFailure(message))
114: /// @brief Assert that value is true
115: #define ASSERT_TRUE(value) \
116:     do \
117:     { \
118:         ASSERT_PRED_FORMAT1(::unittest::CheckTrue, value); \
119:     } while (0)
120: /// @brief Expect that value is true
121: #define EXPECT_TRUE(value) \
122:     do \
123:     { \
124:         EXPECT_PRED_FORMAT1(::unittest::CheckTrue, value); \
125:     } while (0)
126: 
127: /// @brief Assert that value is false
128: #define ASSERT_FALSE(value) \
129:     do \
130:     { \
131:         ASSERT_PRED_FORMAT1(::unittest::CheckFalse, value); \
132:     } while (0)
133: /// @brief Expect that value is false
134: #define EXPECT_FALSE(value) \
135:     do \
136:     { \
137:         EXPECT_PRED_FORMAT1(::unittest::CheckFalse, value); \
138:     } while (0)
139: 
140: /// @brief Assert that actual value is equal to expected value
141: #define ASSERT_EQ(expected, actual) \
142:     do \
143:     { \
144:         ASSERT_PRED_FORMAT2(::unittest::EqHelper::CheckEqual, expected, actual); \
145:     } while (0)
146: /// @brief Expect that actual value is equal to expected value
147: #define EXPECT_EQ(expected, actual) \
148:     do \
149:     { \
150:         EXPECT_PRED_FORMAT2(::unittest::EqHelper::CheckEqual, expected, actual); \
151:     } while (0)
152: 
153: /// @brief Assert that actual value is not equal to expected value
154: #define ASSERT_NE(expected, actual) \
155:     do \
156:     { \
157:         ASSERT_PRED_FORMAT2(::unittest::EqHelper::CheckNotEqual, expected, actual); \
158:     } while (0)
159: /// @brief Expect that actual value is not equal to expected value
160: #define EXPECT_NE(expected, actual) \
161:     do \
162:     { \
163:         EXPECT_PRED_FORMAT2(::unittest::EqHelper::CheckNotEqual, expected, actual); \
164:     } while (0)
```

- Line 48-78: We check whether any of the defines defined in this header are already defined. If so compilation ends with an error
- Line 81-86: We define a macro `UT_EXPECT_RESULT` which checks if the parameter passed (which is of type `AssertionResult`) has a failure (using the `bool()` method).
If there is a failure, a test failure is added to the current result
- Line 88-96: We define a macro `UT_ASSERT_RESULT` which checks if the parameter passed (which is of type `AssertionResult`) has a failure (using the `bool()` method).
If there is a failure, a test failure is added to the current result. Normally an exception would be thrown, however as we don't have exceptions enabled yet, that will wait until later
- Line 99-100: We define a macro `EXPECT_PRED_FORMAT1` which is passed a check function and a single parameter.
The check function is called, and the result is passed to `UT_EXPECT_RESULT`.
The single parameter version is used with `CheckTrue()` or `CheckFalse` to check the result of a boolean expression
- Line 102-103: We define a macro `ASSERT_PRED_FORMAT1` which is passed a check function and a single parameter.
The check function is called, and the result is passed to `UT_ASSERT_RESULT`.
The single parameter version is used with `CheckTrue()` or `CheckFalse` to check the result of a boolean expression
- Line 106-107: We define a macro `EXPECT_PRED_FORMAT2` which is passed a check function and two parameters.
The check function is called, and the result is passed to `UT_EXPECT_RESULT`.
The two parameter version is used with `EqHelper::CheckEqual()` or `EqHelper::CheckNotEqual()` to check the result of a comparison expression
- Line 109-110: We define a macro `ASSERT_PRED_FORMAT2` which is passed a check function and two parameters.
The check function is called, and the result is passed to `UT_ASSERT_RESULT`.
The two parameter version is used with `EqHelper::CheckEqual()` or `EqHelper::CheckNotEqual()` to check the result of a comparison expression
- Line 113: We define a macro `FAIL` which uses `UT_EXPECT_RESULT` to return a `GenericFailure`
- Line 115-119: We define a macro `ASSERT_TRUE` which uses `ASSERT_PRED_FORMAT1` with `CheckTrue` to check if the parameter is true, and generate a `BooleanFailure` if the check fails
- Line 121-125: We define a macro `EXPECT_TRUE` which uses `EXPECT_PRED_FORMAT1` with `CheckTrue` to check if the parameter is true, and generate a `BooleanFailure` if the check fails
- Line 128-132: We define a macro `ASSERT_FALSE` which uses `ASSERT_PRED_FORMAT1` with `CheckFalse` to check if the parameter is false, and generate a `BooleanFailure` if the check fails
- Line 134-138: We define a macro `EXPECT_FALSE` which uses `EXPECT_PRED_FORMAT1` with `CheckFalse` to check if the parameter is false, and generate a `BooleanFailure` if the check fails
- Line 141-145: We define a macro `ASSERT_EQ` which uses `ASSERT_PRED_FORMAT2` with `EqHelper::CheckEqual` to check if the parameters are equal, and generate a `EqFailure` if the check fails
- Line 147-151: We define a macro `EXPECT_EQ` which uses `EXPECT_PRED_FORMAT2` with `EqHelper::CheckEqual` to check if the parameters are equal, and generate a `EqFailure` if the check fails
- Line 154-158: We define a macro `ASSERT_NE` which uses `ASSERT_PRED_FORMAT2` with `EqHelper::CheckNotEqual` to check if the parameters are not equal, and generate a `InEqFailure` if the check fails
- Line 160-164: We define a macro `EXPECT_NE` which uses `EXPECT_PRED_FORMAT2` with `EqHelper::CheckNotEqual` to check if the parameters are not equal, and generate a `InEqFailure` if the check fails

### unittest.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES___STEP_2_UNITTESTH}

We added a header file, so we'll update the `unittest.h` header

Update the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
...
45: #include <unittest/TestFixture.h>
46: #include <unittest/TestSuite.h>
47: 
48: #include <unittest/ITestReporter.h>
49: #include <unittest/AssertMacros.h>
50: #include <unittest/ConsoleTestReporter.h>
51: #include <unittest/CurrentTest.h>
52: #include <unittest/DeferredTestReporter.h>
53: #include <unittest/Test.h>
54: #include <unittest/TestDetails.h>
55: #include <unittest/TestFixtureInfo.h>
56: #include <unittest/TestInfo.h>
57: #include <unittest/TestMacros.h>
58: #include <unittest/TestRegistry.h>
59: #include <unittest/TestResults.h>
60: #include <unittest/TestRunner.h>
61: #include <unittest/TestSuiteInfo.h>
```

### Update project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES___STEP_2_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
30:     set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Checks.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleTestReporter.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/DeferredTestReporter.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Test.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestInfo.cpp
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
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/DeferredTestReporter.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/PrintValue.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Test.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestInfo.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRegistry.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResult.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestResults.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestRunner.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuite.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestSuiteInfo.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/unittest.h
65:     )
66: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES___STEP_2_APPLICATION_CODE}

Now let's start using the macros we defined.

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
14: TEST_SUITE(Suite1)
15: {
16: 
17: class FixtureMyTest1
18:     : public TestFixture
19: {
20: public:
21:     void SetUp() override
22:     {
23:         LOG_DEBUG("MyTest SetUp");
24:     }
25:     void TearDown() override
26:     {
27:         LOG_DEBUG("MyTest TearDown");
28:     }
29: };
30: 
31: TEST_FIXTURE(FixtureMyTest1, MyTest1)
32: {
33:     FAIL("For some reason");
34: }
35: 
36: } // Suite1
37: 
38: TEST_SUITE(Suite2)
39: {
40: 
41: class FixtureMyTest2
42:     : public TestFixture
43: {
44: public:
45:     void SetUp() override
46:     {
47:         LOG_DEBUG("FixtureMyTest2 SetUp");
48:     }
49:     void TearDown() override
50:     {
51:         LOG_DEBUG("FixtureMyTest2 TearDown");
52:     }
53: };
54: 
55: TEST_FIXTURE(FixtureMyTest2, MyTest2)
56: {
57:     EXPECT_TRUE(true);
58:     EXPECT_FALSE(false);
59:     EXPECT_TRUE(false);
60:     EXPECT_FALSE(true);
61: }
62: 
63: } // Suite2
64: 
65: class FixtureMyTest3
66:     : public TestFixture
67: {
68: public:
69:     void SetUp() override
70:     {
71:         LOG_DEBUG("FixtureMyTest3 SetUp");
72:     }
73:     void TearDown() override
74:     {
75:         LOG_DEBUG("FixtureMyTest3 TearDown");
76:     }
77: };
78: 
79: TEST_FIXTURE(FixtureMyTest3, MyTest3)
80: {
81:     int x = 0;
82:     int y = 1;
83:     int z = 1;
84:     EXPECT_EQ(x, y);
85:     EXPECT_EQ(y, z);
86:     EXPECT_NE(x, y);
87:     EXPECT_NE(y, z);
88: }
89: 
90: TEST(MyTest)
91: {
92:     ASSERT_TRUE(false);
93: }
94: 
95: int main()
96: {
97:     auto& console = GetConsole();
98: 
99:     ConsoleTestReporter reporter;
100:     RunAllTests(&reporter);
101: 
102:     LOG_INFO("Wait 5 seconds");
103:     Timer::WaitMilliSeconds(5000);
104: 
105:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
106:     char ch{};
107:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
108:     {
109:         ch = console.ReadChar();
110:         console.WriteChar(ch);
111:     }
112:     if (ch == 'p')
113:         assert(false);
114: 
115:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
116: }
```

- Line 31-34: We change the `Test1` function to a call to `FAIL()`, this will generate a simple failure
- Line 55-61: We change the `Test2` function to boolean checks. Obviously, `EXPECT_TRUE(false)` and `EXPECT_FALSE(true)` will fail.
- Line 79-88: We change the `Test3` function to equality checks. Obviously, `EXPECT_EQ(x, y)` and ` EXPECT_NE(y, z)` will fail.
- Line 90-93: We change the `Test4` function to a failed assertion.
As said before, this should throw an exception, but for now this is commented out, so it will result in a normal failure, and the test run will continue

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:201)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
[  FAILED   ] Suite1::FixtureMyTest1::MyTest1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
[  FAILED   ] Suite2::FixtureMyTest2::MyTest2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
[  FAILED   ] DefaultSuite::FixtureMyTest3::MyTest3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
[  FAILED   ] DefaultSuite::DefaultFixture::MyTest
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 4 out of 4 tests failed (6 failures).

Failures:
../code/applications/demo/src/main.cpp:33 : Failure in Suite1::FixtureMyTest1::MyTest1: For some reason
../code/applications/demo/src/main.cpp:59 : Failure in Suite2::FixtureMyTest2::MyTest2: Value of: false
  Expected: true

../code/applications/demo/src/main.cpp:60 : Failure in Suite2::FixtureMyTest2::MyTest2: Value of: true
  Expected: false

../code/applications/demo/src/main.cpp:84 : Failure in DefaultSuite::FixtureMyTest3::MyTest3: Value of: y
  Actual: 1
  Expected: x
  Which is: 0

../code/applications/demo/src/main.cpp:87 : Failure in DefaultSuite::FixtureMyTest3::MyTest3: Value of: z
  Actual: 1
  Expected not equal to: y
  Which is: 1

../code/applications/demo/src/main.cpp:92 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: false
  Expected: true


[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   Wait 5 seconds (main:102)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Test assert macro extension - Step 3 {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION___STEP_3}

We can now perform boolean checks and compare integers, but we would also like to be able to compare pointers and strings.
So we'll extend the macros a bit.

### Checks.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_CHECKSH}

We need to extend the utility functions a bit.

Update the file `code/libraries/unittest/include/unittest/Checks.h`

```cpp
File: code/libraries/unittest/include/unittest/Checks.h
...
94: extern AssertionResult CloseFailure(const baremetal::string& expectedExpression,
95:                                     const baremetal::string& actualExpression,
96:                                     const baremetal::string& toleranceExpression,
97:                                     const baremetal::string& expectedValue,
98:                                     const baremetal::string& actualValue,
99:                                     const baremetal::string& toleranceValue);
100: 
...
254: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
255:                                    char const* expected, char const* actual);
256: 
257: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
258:                                    char* expected, char* actual);
259: 
260: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
261:                                    char* expected, char const* actual);
262: 
263: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
264:                                    char const* expected, char* actual);
265: 
266: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
267:                                       char const* expected, char const* actual);
268: 
269: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
270:                                       char* expected, char* actual);
271: 
272: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
273:                                       char* expected, char const* actual);
274: 
275: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
276:                                       char const* expected, char* actual);
277: 
278: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
279:                                    const baremetal::string& actualExpression,
280:                                    const baremetal::string& expected,
281:                                    const baremetal::string& actual);
282: 
283: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
284:                                    const baremetal::string& actualExpression,
285:                                    const baremetal::string& expected,
286:                                    const char* actual);
287: 
288: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
289:                                    const baremetal::string& actualExpression,
290:                                    const char* expected,
291:                                    const baremetal::string& actual);
292: 
293: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
294:                                       const baremetal::string& actualExpression,
295:                                       const baremetal::string& expected,
296:                                       const baremetal::string& actual);
297: 
298: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
299:                                       const baremetal::string& actualExpression,
300:                                       const baremetal::string& expected,
301:                                       const char* actual);
302: 
303: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
304:                                       const baremetal::string& actualExpression,
305:                                       const char* expected,
306:                                       const baremetal::string& actual);
307: 
308: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
309:                                              char const* expected, char const* actual);
310: 
311: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
312:                                              char* expected, char* actual);
313: 
314: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
315:                                              char* expected, char const* actual);
316: 
317: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
318:                                              char const* expected, char* actual);
319: 
320: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
321:                                                 char const* expected, char const* actual);
322: 
323: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
324:                                                 char* expected, char* actual);
325: 
326: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
327:                                                 char* expected, char const* actual);
328: 
329: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
330:                                                 char const* expected, char* actual);
331: 
332: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
333:                                              const baremetal::string& actualExpression,
334:                                              const baremetal::string& expected,
335:                                              const baremetal::string& actual);
336: 
337: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
338:                                              const baremetal::string& actualExpression,
339:                                              const baremetal::string& expected,
340:                                              const char* actual);
341: 
342: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
343:                                              const baremetal::string& actualExpression,
344:                                              const char* expected,
345:                                              const baremetal::string& actual);
346: 
347: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
348:                                                 const baremetal::string& actualExpression,
349:                                                 const baremetal::string& expected,
350:                                                 const baremetal::string& actual);
351: 
352: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
353:                                                 const baremetal::string& actualExpression,
354:                                                 const baremetal::string& expected,
355:                                                 const char* actual);
356: 
357: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
358:                                                 const baremetal::string& actualExpression,
359:                                                 const char* expected,
360:                                                 const baremetal::string& actual);
361: 
362: 
363: /// <summary>
364: /// Helper class for {ASSERT|EXPECT}_EQ/NE
365: ///
366: /// Forms generalized mechanism for calling polymorphic check functions.
367: /// The template argument lhs_is_null_literal is true iff the first argument to ASSERT_EQ()
368: /// is a null pointer literal.  The following default implementation is
369: /// for lhs_is_null_literal being false.
370: /// </summary>
371: template <bool lhs_is_null_literal>
372: class EqHelper
373: {
374: public:
375:     /// <summary>
376:     /// Evaluate whether an expected value is equal to an actual value, generate a success object if successful, otherwise a failure object
377:     /// </summary>
378:     /// <typeparam name="Expected">Type of the expected value</typeparam>
379:     /// <typeparam name="Actual">Type of the actual value</typeparam>
380:     /// <param name="expectedExpression">String representation of the expected value</param>
381:     /// <param name="actualExpression">String representation of the actual value</param>
382:     /// <param name="expected">Expected value</param>
383:     /// <param name="actual">Actual value</param>
384:     /// <returns>Result object</returns>
385:     template <typename Expected, typename Actual>
386:     static AssertionResult CheckEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
387:                                       const Expected& expected, const Actual& actual)
388:     {
389:         return CheckEqualInternal(expectedExpression, actualExpression, expected, actual);
390:     }
391: 
392:     /// <summary>
393:     /// Evaluate whether an expected value is not equal to an actual value, generate a success object if successful, otherwise a failure object
394:     /// </summary>
395:     /// <typeparam name="Expected">Type of the expected value</typeparam>
396:     /// <typeparam name="Actual">Type of the actual value</typeparam>
397:     /// <param name="expectedExpression">String representation of the not expected value</param>
398:     /// <param name="actualExpression">String representation of the actual value</param>
399:     /// <param name="expected">Expected value</param>
400:     /// <param name="actual">Actual value</param>
401:     /// <returns>Result object</returns>
402:     template <typename Expected, typename Actual>
403:     static AssertionResult CheckNotEqual(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
404:                                          const Expected& expected, const Actual& actual)
405:     {
406:         return CheckNotEqualInternal(expectedExpression, actualExpression, expected, actual);
407:     }
408: };
409: 
410: /// <summary>
411: /// Helper class for {ASSERT|EXPECT}_EQ/NE_IGNORE_CASE
412: /// 
413: /// Forms generalized mechanism for calling polymorphic check functions for string comparisons ignoring case.
414: /// </summary>
415: class EqHelperStringCaseInsensitive
416: {
417: public:
418:     /// <summary>
419:     /// Evaluate whether an expected value is equal to an actual value, ignoring case, generate a success object if successful, otherwise a failure object
420:     /// </summary>
421:     /// <typeparam name="Expected">Type of the expected value</typeparam>
422:     /// <typeparam name="Actual">Type of the actual value</typeparam>
423:     /// <param name="expectedExpression">String representation of the expected value</param>
424:     /// <param name="actualExpression">String representation of the actual value</param>
425:     /// <param name="expected">Expected value</param>
426:     /// <param name="actual">Actual value</param>
427:     /// <returns>Result object</returns>
428:     template <typename Expected, typename Actual>
429:     static AssertionResult CheckEqualIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
430:                                                 const Expected& expected, const Actual& actual)
431:     {
432:         return CheckEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
433:     }
434: 
435:     /// <summary>
436:     /// Evaluate whether an expected value is not equal to an actual value, ignoring case, generate a success object if successful, otherwise a failure object
437:     /// </summary>
438:     /// <typeparam name="Expected">Type of the expected value</typeparam>
439:     /// <typeparam name="Actual">Type of the actual value</typeparam>
440:     /// <param name="expectedExpression">String representation of the not expected value</param>
441:     /// <param name="actualExpression">String representation of the actual value</param>
442:     /// <param name="expected">Expected value</param>
443:     /// <param name="actual">Actual value</param>
444:     /// <returns>Result object</returns>
445:     template <typename Expected, typename Actual>
446:     static AssertionResult CheckNotEqualIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
447:                                                    const Expected& expected, const Actual& actual)
448:     {
449:         return CheckNotEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
450:     }
451: };
452: 
453: /// <summary>
454: /// Compare an expected and actual value, with tolerance
455: /// 
456: /// Compares two floating point values, and returns true if the absolute difference is within tolerance
457: /// </summary>
458: /// <typeparam name="Expected">Type of expected value</typeparam>
459: /// <typeparam name="Actual">Type of actual value</typeparam>
460: /// <typeparam name="Tolerance">Type of tolerance value</typeparam>
461: /// <param name="expected">Expected value</param>
462: /// <param name="actual">Actual value</param>
463: /// <param name="tolerance">Tolerance value</param>
464: /// <returns>True if the values are considered equal, false otherwise</returns>
465: template< typename Expected, typename Actual, typename Tolerance >
466: bool AreClose(const Expected& expected, const Actual& actual, Tolerance const& tolerance)
467: {
468:     return (actual >= (expected - tolerance)) && (actual <= (expected + tolerance));
469: }
470: 
471: /// <summary>
472: /// Evaluate whether an expected value is equal to an actual value within tolerance, generate a success object if successful, otherwise a failure object
473: /// </summary>
474: /// <typeparam name="Expected">Type of the expected value</typeparam>
475: /// <typeparam name="Actual">Type of the actual value</typeparam>
476: /// <typeparam name="Tolerance">Type of the tolerance value</typeparam>
477: /// <param name="expectedExpression">String representation of the expected value</param>
478: /// <param name="actualExpression">String representation of the actual value</param>
479: /// <param name="toleranceExpression">String representation of the tolerance value</param>
480: /// <param name="expected">Expected value</param>
481: /// <param name="actual">Actual value</param>
482: /// <param name="tolerance">Tolerance value</param>
483: /// <returns>Result object</returns>
484: template< typename Expected, typename Actual, typename Tolerance >
485: AssertionResult CheckClose(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
486:                            const baremetal::string& toleranceExpression,
487:                            const Expected& expected, const Actual& actual, Tolerance const& tolerance)
488: {
489:     if (!AreClose(expected, actual, tolerance))
490:     {
491:         return CloseFailure(expectedExpression, actualExpression, toleranceExpression,
492:                             FormatForComparisonFailureMessage(expected),
493:                             FormatForComparisonFailureMessage(actual),
494:                             FormatForComparisonFailureMessage(tolerance));
495:     }
496:     return AssertionSuccess();
497: }
498: 
499: } // namespace unittest
```

- Line 94-99: We declare a function `CloseFailure()` to signify a assertion failure on comparison between numbers with a fault tolerance
- Line 254-255: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 257-258: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 260-261: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 263-264: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 266-267: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 269-270: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 272-273: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 275-276: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 278-281: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 283-286: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 288-291: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 293-296: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 298-301: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 303-306: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 308-309: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 311-312: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 314-315: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 317-318: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 320-321: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 323-324: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 326-327: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 329-330: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 332-335: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 337-340: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 342-345: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 347-350: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 352-355: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 356-360: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 371: We change the class `EqHelper` into a template class that receives a template parameter signifying whether the expected value is a null pointer
- Line 415-451: We declare a class `EqHelperCaseInsensitive` which has two methods `CheckEqualIgnoreCase()` (which uses `CheckEqualInternalIgnoreCase()`) and `CheckNotEqualIgnoreCase()` (which uses `CheckNotEqualInternalIgnoreCase()`)
- Line 465-469: We define a template function `AreClose()` to compare two values `expected` and `actual`, with a tolerance `tolerance`, which can have different types.
The function compares the absolute difference between `expected` and `actual` with the given tolerance, It returns true of the difference is smaller or equal to the tolerance, false otherwise
- Line 484-497: We define a template function `CheckClose()`, which uses `AreClose()` to compare two values `expected` and `actual`. Their stringified versions are passed as `expectedExpression` and `actualExpression` with tolerance `tolerance`.
If the absolute difference between `expected` and `actual` is smaller or equal to `tolerance`, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned

### Checks.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_CHECKSCPP}

Let's implement the new functions for the checks.

Update the file `code/libraries/unittest/src/Checks.cpp`

```cpp
File: code/libraries/unittest/src/Checks.cpp
...
45: /// @file
46: /// Assertion checks implementation
47: 
48: using namespace baremetal;
49: 
50: namespace unittest {
51: 
52: /// <summary>
53: /// Compare two strings ignoring case
54: /// </summary>
55: /// <param name="a">Left hand side of comparison</param>
56: /// <param name="b">Right hand side of comparison</param>
57: /// <returns>True if the strings are equal ignoring case, false otherwise</returns>
58: static bool EqualCaseInsensitive(const string& a, const string& b)
59: {
60:     if (a.length() != b.length())
61:         return false;
62:     return strcasecmp(a.data(), b.data()) == 0;
63: }
64: 
...
84: /// <summary>
85: /// Create a boolean failure object
86: /// </summary>
87: /// <param name="valueExpression">String representation of the actual value</param>
88: /// <param name="expectedValue">Expected value</param>
89: /// <param name="actualValue">Actual value</param>
90: /// <returns>Result object</returns>
91: AssertionResult BooleanFailure(const baremetal::string& valueExpression, const baremetal::string& expectedValue, const baremetal::string& actualValue)
92: {
93:     string result = Format("Value of: %s", valueExpression.c_str());
94:     if (actualValue != valueExpression)
95:     {
96:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
97:     }
98: 
99:     result.append(Format("\n  Expected: %s\n", expectedValue.c_str()));
100: 
101:     return AssertionResult(true, result);
102: }
103: 
104: /// <summary>
105: /// Create a equality comparison failure object
106: /// </summary>
107: /// <param name="expectedExpression">String representation of the expected value</param>
108: /// <param name="actualExpression">String representation of the actual value</param>
109: /// <param name="expectedValue">Expected value</param>
110: /// <param name="actualValue">Actual value</param>
111: /// <returns>Result object</returns>
112: AssertionResult EqFailure(
113:     const baremetal::string& expectedExpression,
114:     const baremetal::string& actualExpression,
115:     const baremetal::string& expectedValue,
116:     const baremetal::string& actualValue)
117: {
118:     string result = Format("Value of: %s", actualExpression.c_str());
119:     if (actualValue != actualExpression)
120:     {
121:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
122:     }
123: 
124:     result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
125:     if (expectedValue != expectedExpression)
126:     {
127:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
128:     }
129:     result.append("\n");
130: 
131:     return AssertionResult(true, result);
132: }
133: 
134: /// <summary>
135: /// Create a inequality comparison failure object
136: /// </summary>
137: /// <param name="expectedExpression">String representation of the not expected value</param>
138: /// <param name="actualExpression">String representation of the actual value</param>
139: /// <param name="expectedValue">Expected value</param>
140: /// <param name="actualValue">Actual value</param>
141: /// <returns>Result object</returns>
142: AssertionResult InEqFailure(
143:     const baremetal::string& expectedExpression,
144:     const baremetal::string& actualExpression,
145:     const baremetal::string& expectedValue,
146:     const baremetal::string& actualValue)
147: {
148:     string result = Format("Value of: %s", actualExpression.c_str());
149:     if (actualValue != actualExpression)
150:     {
151:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
152:     }
153: 
154:     result.append(Format("\n  Expected not equal to: %s", expectedExpression.c_str()));
155:     if (expectedValue != expectedExpression)
156:     {
157:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
158:     }
159:     result.append("\n");
160: 
161:     return AssertionResult(true, result);
162: }
163: 
164: /// <summary>
165: /// Create a comparison with tolerance failure object
166: /// </summary>
167: /// <param name="expectedExpression">String representation of the expected value</param>
168: /// <param name="actualExpression">String representation of the actual value</param>
169: /// <param name="toleranceExpression">String representation of the tolerance value</param>
170: /// <param name="expectedValue">Expected value</param>
171: /// <param name="actualValue">Actual value</param>
172: /// <param name="toleranceValue">Tolerance value</param>
173: /// <returns>Result object</returns>
174: AssertionResult CloseFailure(
175:     const baremetal::string& expectedExpression,
176:     const baremetal::string& actualExpression,
177:     const baremetal::string& toleranceExpression,
178:     const baremetal::string& expectedValue,
179:     const baremetal::string& actualValue,
180:     const baremetal::string& toleranceValue)
181: {
182:     string result = Format("Value of: %s", actualExpression.c_str());
183:     if (actualValue != actualExpression)
184:     {
185:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
186:     }
187: 
188:     result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
189:     if (expectedValue != expectedExpression)
190:     {
191:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
192:     }
193:     result.append(Format("\n  Tolerance: %s", toleranceExpression.c_str()));
194:     if (toleranceValue != toleranceExpression)
195:     {
196:         result.append(Format("\n  (+/-) %s", toleranceValue.c_str()));
197:     }
198: 
199:     return AssertionResult(true, result);
200: }
201: 
202: namespace internal {
203: 
204: /// <summary>
205: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
206: /// </summary>
207: /// <param name="expectedExpression">String representation of expected value</param>
208: /// <param name="actualExpression">String representation of actual value</param>
209: /// <param name="expected">Expected value</param>
210: /// <param name="actual">Actual value</param>
211: /// <returns>Result object</returns>
212: AssertionResult CheckStringsEqual(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
213: {
214:     if (expected == actual)
215:         return AssertionSuccess();
216: 
217:     if (strcmp(expected, actual))
218:     {
219:         return EqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
220:     }
221:     return AssertionSuccess();
222: }
223: 
224: /// <summary>
225: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
226: /// </summary>
227: /// <param name="expectedExpression">String representation of expected value</param>
228: /// <param name="actualExpression">String representation of actual value</param>
229: /// <param name="expected">Expected value</param>
230: /// <param name="actual">Actual value</param>
231: /// <returns>Result object</returns>
232: AssertionResult CheckStringsNotEqual(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
233: {
234:     if (expected == actual)
235:         return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
236: 
237:     if (!strcmp(expected, actual))
238:     {
239:         return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
240:     }
241:     return AssertionSuccess();
242: }
243: 
244: /// <summary>
245: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
246: /// </summary>
247: /// <param name="expectedExpression">String representation of expected value</param>
248: /// <param name="actualExpression">String representation of actual value</param>
249: /// <param name="expected">Expected value</param>
250: /// <param name="actual">Actual value</param>
251: /// <returns>Result object</returns>
252: AssertionResult CheckStringsEqualIgnoreCase(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
253: {
254:     if (expected == actual)
255:         return AssertionSuccess();
256: 
257:     if (!EqualCaseInsensitive(baremetal::string(expected), baremetal::string(actual)))
258:     {
259:         return EqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
260:     }
261:     return AssertionSuccess();
262: }
263: 
264: /// <summary>
265: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
266: /// </summary>
267: /// <param name="expectedExpression">String representation of expected value</param>
268: /// <param name="actualExpression">String representation of actual value</param>
269: /// <param name="expected">Expected value</param>
270: /// <param name="actual">Actual value</param>
271: /// <returns>Result object</returns>
272: AssertionResult CheckStringsNotEqualIgnoreCase(const string& expectedExpression, const string& actualExpression, char const *expected, char const *actual)
273: {
274:     if (expected == actual)
275:         return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
276: 
277:     if (EqualCaseInsensitive(baremetal::string(expected), baremetal::string(actual)))
278:     {
279:         return InEqFailure(expectedExpression, actualExpression, baremetal::string(expected), baremetal::string(actual));
280:     }
281:     return AssertionSuccess();
282: }
283: 
284: } // namespace internal
285: 
286: /// <summary>
287: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
288: /// </summary>
289: /// <param name="expectedExpression">String representation of expected value</param>
290: /// <param name="actualExpression">String representation of actual value</param>
291: /// <param name="expected">Expected value</param>
292: /// <param name="actual">Actual value</param>
293: /// <returns>Result object</returns>
294: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression, char const *expected, char const *actual)
295: {
296:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
297: }
298: 
299: /// <summary>
300: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
301: /// </summary>
302: /// <param name="expectedExpression">String representation of expected value</param>
303: /// <param name="actualExpression">String representation of actual value</param>
304: /// <param name="expected">Expected value</param>
305: /// <param name="actual">Actual value</param>
306: /// <returns>Result object</returns>
307: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression, char *expected,
308:                                    char *actual) // cppcheck-suppress constParameterPointer
309: {
310:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
311: }
312: 
313: /// <summary>
314: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
315: /// </summary>
316: /// <param name="expectedExpression">String representation of expected value</param>
317: /// <param name="actualExpression">String representation of actual value</param>
318: /// <param name="expected">Expected value</param>
319: /// <param name="actual">Actual value</param>
320: /// <returns>Result object</returns>
321: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression, char *expected,
322:                                    char const *actual) // cppcheck-suppress constParameterPointer
323: {
324:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
325: }
326: 
327: /// <summary>
328: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
329: /// </summary>
330: /// <param name="expectedExpression">String representation of expected value</param>
331: /// <param name="actualExpression">String representation of actual value</param>
332: /// <param name="expected">Expected value</param>
333: /// <param name="actual">Actual value</param>
334: /// <returns>Result object</returns>
335: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression, char const *expected,
336:                                    char *actual) // cppcheck-suppress constParameterPointer
337: {
338:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
339: }
340: 
341: /// <summary>
342: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
343: /// </summary>
344: /// <param name="expectedExpression">String representation of expected value</param>
345: /// <param name="actualExpression">String representation of actual value</param>
346: /// <param name="expected">Expected value</param>
347: /// <param name="actual">Actual value</param>
348: /// <returns>Result object</returns>
349: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression, char const *expected, char const *actual)
350: {
351:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
352: }
353: 
354: /// <summary>
355: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
356: /// </summary>
357: /// <param name="expectedExpression">String representation of expected value</param>
358: /// <param name="actualExpression">String representation of actual value</param>
359: /// <param name="expected">Expected value</param>
360: /// <param name="actual">Actual value</param>
361: /// <returns>Result object</returns>
362: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression, char *expected,
363:                                       char *actual) // cppcheck-suppress constParameterPointer
364: {
365:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
366: }
367: 
368: /// <summary>
369: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
370: /// </summary>
371: /// <param name="expectedExpression">String representation of expected value</param>
372: /// <param name="actualExpression">String representation of actual value</param>
373: /// <param name="expected">Expected value</param>
374: /// <param name="actual">Actual value</param>
375: /// <returns>Result object</returns>
376: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression, char *expected,
377:                                       char const *actual) // cppcheck-suppress constParameterPointer
378: {
379:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
380: }
381: 
382: /// <summary>
383: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
384: /// </summary>
385: /// <param name="expectedExpression">String representation of expected value</param>
386: /// <param name="actualExpression">String representation of actual value</param>
387: /// <param name="expected">Expected value</param>
388: /// <param name="actual">Actual value</param>
389: /// <returns>Result object</returns>
390: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression, const baremetal::string& actualExpression, char const *expected,
391:                                       char *actual) // cppcheck-suppress constParameterPointer
392: {
393:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
394: }
395: 
396: /// <summary>
397: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
398: /// </summary>
399: /// <param name="expectedExpression">String representation of expected value</param>
400: /// <param name="actualExpression">String representation of actual value</param>
401: /// <param name="expected">Expected value</param>
402: /// <param name="actual">Actual value</param>
403: /// <returns>Result object</returns>
404: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
405:                                    const baremetal::string& actualExpression,
406:                                    const baremetal::string& expected,
407:                                    const baremetal::string& actual)
408: {
409:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
410: }
411: 
412: /// <summary>
413: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
414: /// </summary>
415: /// <param name="expectedExpression">String representation of expected value</param>
416: /// <param name="actualExpression">String representation of actual value</param>
417: /// <param name="expected">Expected value</param>
418: /// <param name="actual">Actual value</param>
419: /// <returns>Result object</returns>
420: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
421:                                    const baremetal::string& actualExpression,
422:                                    const baremetal::string& expected,
423:                                    const char* actual)
424: {
425:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
426: }
427: 
428: /// <summary>
429: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
430: /// </summary>
431: /// <param name="expectedExpression">String representation of expected value</param>
432: /// <param name="actualExpression">String representation of actual value</param>
433: /// <param name="expected">Expected value</param>
434: /// <param name="actual">Actual value</param>
435: /// <returns>Result object</returns>
436: AssertionResult CheckEqualInternal(const baremetal::string& expectedExpression,
437:                                    const baremetal::string& actualExpression,
438:                                    const char* expected,
439:                                    const baremetal::string& actual)
440: {
441:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
442: }
443: 
444: /// <summary>
445: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
446: /// </summary>
447: /// <param name="expectedExpression">String representation of expected value</param>
448: /// <param name="actualExpression">String representation of actual value</param>
449: /// <param name="expected">Expected value</param>
450: /// <param name="actual">Actual value</param>
451: /// <returns>Result object</returns>
452: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
453:                                       const baremetal::string& actualExpression,
454:                                       const baremetal::string& expected,
455:                                       const baremetal::string& actual)
456: {
457:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
458: }
459: 
460: /// <summary>
461: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
462: /// </summary>
463: /// <param name="expectedExpression">String representation of expected value</param>
464: /// <param name="actualExpression">String representation of actual value</param>
465: /// <param name="expected">Expected value</param>
466: /// <param name="actual">Actual value</param>
467: /// <returns>Result object</returns>
468: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
469:                                       const baremetal::string& actualExpression,
470:                                       const baremetal::string& expected,
471:                                       const char* actual)
472: {
473:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
474: }
475: 
476: /// <summary>
477: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
478: /// </summary>
479: /// <param name="expectedExpression">String representation of expected value</param>
480: /// <param name="actualExpression">String representation of actual value</param>
481: /// <param name="expected">Expected value</param>
482: /// <param name="actual">Actual value</param>
483: /// <returns>Result object</returns>
484: AssertionResult CheckNotEqualInternal(const baremetal::string& expectedExpression,
485:                                       const baremetal::string& actualExpression,
486:                                       const char* expected,
487:                                       const baremetal::string& actual)
488: {
489:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
490: }
491: 
492: /// <summary>
493: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
494: /// </summary>
495: /// <param name="expectedExpression">String representation of expected value</param>
496: /// <param name="actualExpression">String representation of actual value</param>
497: /// <param name="expected">Expected value</param>
498: /// <param name="actual">Actual value</param>
499: /// <returns>Result object</returns>
500: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
501:                                              char const* expected, char const* actual)
502: {
503:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
504: }
505: 
506: /// <summary>
507: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
508: /// </summary>
509: /// <param name="expectedExpression">String representation of expected value</param>
510: /// <param name="actualExpression">String representation of actual value</param>
511: /// <param name="expected">Expected value</param>
512: /// <param name="actual">Actual value</param>
513: /// <returns>Result object</returns>
514: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
515:                                              char* expected, char* actual)
516: {
517:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
518: }
519: 
520: /// <summary>
521: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
522: /// </summary>
523: /// <param name="expectedExpression">String representation of expected value</param>
524: /// <param name="actualExpression">String representation of actual value</param>
525: /// <param name="expected">Expected value</param>
526: /// <param name="actual">Actual value</param>
527: /// <returns>Result object</returns>
528: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
529:                                              char* expected, char const* actual)
530: {
531:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
532: }
533: 
534: /// <summary>
535: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
536: /// </summary>
537: /// <param name="expectedExpression">String representation of expected value</param>
538: /// <param name="actualExpression">String representation of actual value</param>
539: /// <param name="expected">Expected value</param>
540: /// <param name="actual">Actual value</param>
541: /// <returns>Result object</returns>
542: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
543:                                              char const* expected, char* actual)
544: {
545:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
546: }
547: 
548: /// <summary>
549: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
550: /// </summary>
551: /// <param name="expectedExpression">String representation of expected value</param>
552: /// <param name="actualExpression">String representation of actual value</param>
553: /// <param name="expected">Expected value</param>
554: /// <param name="actual">Actual value</param>
555: /// <returns>Result object</returns>
556: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
557:                                                 char const* expected, char const* actual)
558: {
559:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
560: }
561: 
562: /// <summary>
563: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
564: /// </summary>
565: /// <param name="expectedExpression">String representation of expected value</param>
566: /// <param name="actualExpression">String representation of actual value</param>
567: /// <param name="expected">Expected value</param>
568: /// <param name="actual">Actual value</param>
569: /// <returns>Result object</returns>
570: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
571:                                                 char* expected, char* actual)
572: {
573:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
574: }
575: 
576: /// <summary>
577: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
578: /// </summary>
579: /// <param name="expectedExpression">String representation of expected value</param>
580: /// <param name="actualExpression">String representation of actual value</param>
581: /// <param name="expected">Expected value</param>
582: /// <param name="actual">Actual value</param>
583: /// <returns>Result object</returns>
584: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
585:                                                 char* expected, char const* actual)
586: {
587:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
588: }
589: 
590: /// <summary>
591: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
592: /// </summary>
593: /// <param name="expectedExpression">String representation of expected value</param>
594: /// <param name="actualExpression">String representation of actual value</param>
595: /// <param name="expected">Expected value</param>
596: /// <param name="actual">Actual value</param>
597: /// <returns>Result object</returns>
598: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression, const baremetal::string& actualExpression,
599:                                                 char const* expected, char* actual)
600: {
601:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
602: }
603: 
604: /// <summary>
605: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
606: /// </summary>
607: /// <param name="expectedExpression">String representation of expected value</param>
608: /// <param name="actualExpression">String representation of actual value</param>
609: /// <param name="expected">Expected value</param>
610: /// <param name="actual">Actual value</param>
611: /// <returns>Result object</returns>
612: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
613:                                              const baremetal::string& actualExpression,
614:                                              const baremetal::string& expected,
615:                                              const baremetal::string& actual)
616: {
617:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
618: }
619: 
620: /// <summary>
621: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
622: /// </summary>
623: /// <param name="expectedExpression">String representation of expected value</param>
624: /// <param name="actualExpression">String representation of actual value</param>
625: /// <param name="expected">Expected value</param>
626: /// <param name="actual">Actual value</param>
627: /// <returns>Result object</returns>
628: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
629:                                              const baremetal::string& actualExpression,
630:                                              const baremetal::string& expected,
631:                                              const char* actual)
632: {
633:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
634: }
635: 
636: /// <summary>
637: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
638: /// </summary>
639: /// <param name="expectedExpression">String representation of expected value</param>
640: /// <param name="actualExpression">String representation of actual value</param>
641: /// <param name="expected">Expected value</param>
642: /// <param name="actual">Actual value</param>
643: /// <returns>Result object</returns>
644: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
645:                                              const baremetal::string& actualExpression,
646:                                              const char* expected,
647:                                              const baremetal::string& actual)
648: {
649:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
650: }
651: 
652: /// <summary>
653: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
654: /// </summary>
655: /// <param name="expectedExpression">String representation of expected value</param>
656: /// <param name="actualExpression">String representation of actual value</param>
657: /// <param name="expected">Expected value</param>
658: /// <param name="actual">Actual value</param>
659: /// <returns>Result object</returns>
660: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
661:                                                 const baremetal::string& actualExpression,
662:                                                 const baremetal::string& expected,
663:                                                 const baremetal::string& actual)
664: {
665:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
666: }
667: 
668: /// <summary>
669: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
670: /// </summary>
671: /// <param name="expectedExpression">String representation of expected value</param>
672: /// <param name="actualExpression">String representation of actual value</param>
673: /// <param name="expected">Expected value</param>
674: /// <param name="actual">Actual value</param>
675: /// <returns>Result object</returns>
676: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
677:                                                 const baremetal::string& actualExpression,
678:                                                 const baremetal::string& expected,
679:                                                 const char* actual)
680: {
681:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
682: }
683: 
684: /// <summary>
685: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
686: /// </summary>
687: /// <param name="expectedExpression">String representation of expected value</param>
688: /// <param name="actualExpression">String representation of actual value</param>
689: /// <param name="expected">Expected value</param>
690: /// <param name="actual">Actual value</param>
691: /// <returns>Result object</returns>
692: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::string& expectedExpression,
693:                                                 const baremetal::string& actualExpression,
694:                                                 const char* expected,
695:                                                 const baremetal::string& actual)
696: {
697:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
698: }
699: 
700: } // namespace unittest
```

- Line 58-63: We define a local function `EqualCaseInsensitive` which compares two strings in a case insensitive way, and returns true if they are considered equal, false otherwise
- Line 91-102: Due to doxygen warnings, the `baremetal` namespace was added in the signature of the `BooleanFailure()` function, although it is not needed for C++
- Line 112-132: Due to doxygen warnings, the `baremetal` namespace was added in the signature of the `EqFailure()` function, although it is not needed for C++
- Line 142-162: Due to doxygen warnings, the `baremetal` namespace was added in the signature of the `InEqFailure()` function, although it is not needed for C++
- Line 174-200: We implement the function `CloseFailure()`. This will return an assertion result flagging a failure, with a string explaining that the absolute difference between the actual value and the expected value is larger than the tolerance
- Line 212-222: We define a function `CheckStringsEqual()` in a local namespace `internal`.
If the two values of type const char* are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 232-242: We define a function `CheckStringsNotEqual()` in a local namespace `internal`.
If the two values of type const char* are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 252-262: We define a function `CheckStringsEqualIgnoreCase()` in a local namespace `internal`.
If the two values of type const char* are considered equal ignoring case, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 272-282: We define a function `CheckStringsNotEqualIgnoreCase()` in a local namespace `internal`.
If the two values of type const char* are considered not equal ignoring case, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 294-297: We implement the function `CheckEqualInternal()` for two value of type const char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 307-311: We implement the function `CheckEqualInternal()` for two value of type char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 321-325: We implement the function `CheckEqualInternal()` for two value of type char* and const char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 335-339: We implement the function `CheckEqualInternal()` for two value of type const char* and char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 349-352: We implement the function `CheckNotEqualInternal()` for two value of type const char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 362-366: We implement the function `CheckNotEqualInternal()` for two value of type char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 376-380: We implement the function `CheckNotEqualInternal()` for two value of type char* and const char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 390-394: We implement the function `CheckNotEqualInternal()` for two value of type const char* and char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 404-410: We implement the function `CheckEqualInternal()` for two value of type string.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 420-426: We implement the function `CheckEqualInternal()` for two value of type char* and string.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 436-442: We implement the function `CheckEqualInternal()` for two value of type string and const char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 452-458: We implement the function `CheckNotEqualInternal()` for two value of type string.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 468-474: We implement the function `CheckNotEqualInternal()` for two value of type char* and string.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 484-490: We implement the function `CheckNotEqualInternal()` for two value of type string and const char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 500-504: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type const char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 514-518: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 528-532: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type char* and const char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 542-546: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type const char* and char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 556-560: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type const char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 570-574: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 584-588: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type char* and const char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 598-602: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type const char* and char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 612-618: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type string.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 628-634: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type char* and string.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 644-650: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type string and const char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 660-666: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type string.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 676-682: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type char* and string.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 692-698: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type string and const char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings

### PrintValue.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_PRINTVALUEH}

The `PrintValue` header will now be extended to support printing pointers and strings.

Update the file `code/libraries/unittest/include/unittest/PrintValue.h`

```cpp
File: code/libraries/unittest/include/unittest/PrintValue.h
...
49: using nullptr_t = decltype(nullptr);
50: 
51: /// <summary>
52: /// Direct cast to another type
53: /// </summary>
54: /// <typeparam name="To">Type to cast to</typeparam>
55: /// <param name="x">Value to be casted</param>
56: /// <returns>Casted value</returns>
57: template<typename To>
58: inline To ImplicitCast_(To x) { return x; }
59: 
60: /// <summary>
61: /// Print a value to string using a serializer
62: /// </summary>
63: /// <typeparam name="T">Type of value to print</typeparam>
64: /// <param name="value">Value to print</param>
65: /// <param name="s">Resulting string</param>
66: template <typename T>
67: void PrintTo(const T& value, baremetal::string& s)
68: {
69:     s = baremetal::Serialize(value);
70: }
71: /// <summary>
72: /// Print a unsigned character value to string
73: /// </summary>
74: /// <param name="c">Value to print</param>
75: /// <param name="s">Resulting string</param>
76: void PrintTo(unsigned char c, baremetal::string& s);
77: /// <summary>
78: /// Print a signed character value to string
79: /// </summary>
80: /// <param name="c">Value to print</param>
81: /// <param name="s">Resulting string</param>
82: void PrintTo(signed char c, baremetal::string& s);
83: /// <summary>
84: /// Print a character value to string
85: /// </summary>
86: /// <param name="c">Value to print</param>
87: /// <param name="s">Resulting string</param>
88: inline void PrintTo(char c, baremetal::string& s)
89: {
90:     PrintTo(static_cast<unsigned char>(c), s);
91: }
92: 
93: /// <summary>
94: /// Print a boolean value to string
95: /// </summary>
96: /// <param name="x">Value to print</param>
97: /// <param name="s">Resulting string</param>
98: inline void PrintTo(bool x, baremetal::string& s)
99: {
100:     s = (x ? "true" : "false");
101: }
102: 
103: /// <summary>
104: /// Print a const char* to string
105: /// </summary>
106: /// <param name="str">Value to print</param>
107: /// <param name="s">Resulting string</param>
108: void PrintTo(const char* str, baremetal::string& s);
109: /// <summary>
110: /// Print a char* to string
111: /// </summary>
112: /// <param name="str">Value to print</param>
113: /// <param name="s">Resulting string</param>
114: inline void PrintTo(char* str, baremetal::string& s)
115: {
116:     PrintTo(ImplicitCast_<const char*>(str), s);
117: }
118: 
119: /// <summary>
120: /// Print a signed char* to string
121: ///
122: /// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
123: /// </summary>
124: /// <param name="str">Value to print</param>
125: /// <param name="s">Resulting string</param>
126: inline void PrintTo(const signed char* str, baremetal::string& s)
127: {
128:     PrintTo(ImplicitCast_<const void*>(str), s);
129: }
130: /// <summary>
131: /// Print a signed char* to string
132: ///
133: /// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
134: /// </summary>
135: /// <param name="str">Value to print</param>
136: /// <param name="s">Resulting string</param>
137: inline void PrintTo(signed char* str, baremetal::string& s)
138: {
139:     PrintTo(ImplicitCast_<const void*>(str), s);
140: }
141: /// <summary>
142: /// Print a signed char* to string
143: ///
144: /// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
145: /// </summary>
146: /// <param name="str">Value to print</param>
147: /// <param name="s">Resulting string</param>
148: inline void PrintTo(const unsigned char* str, baremetal::string& s)
149: {
150:     PrintTo(ImplicitCast_<const void*>(str), s);
151: }
152: /// <summary>
153: /// Print a signed char* to string
154: ///
155: /// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
156: /// </summary>
157: /// <param name="str">Value to print</param>
158: /// <param name="s">Resulting string</param>
159: inline void PrintTo(unsigned char* str, baremetal::string& s)
160: {
161:     PrintTo(ImplicitCast_<const void*>(str), s);
162: }
163: 
164: /// <summary>
165: /// Print a string to string
166: /// </summary>
167: /// <param name="str">Value to print</param>
168: /// <param name="s">Resulting string</param>
169: void PrintStringTo(const baremetal::string& str, baremetal::string& s);
170: /// <summary>
171: /// Print a string to string
172: /// </summary>
173: /// <param name="str">Value to print</param>
174: /// <param name="s">Resulting string</param>
175: inline void PrintTo(const baremetal::string& str, baremetal::string& s)
176: {
177:     PrintStringTo(str, s);
178: }
179: 
180: /// <summary>
181: /// Print a nullptr to string
182: /// </summary>
183: /// <param name="s">Resulting string</param>
184: inline void PrintTo(nullptr_t /*p*/, baremetal::string& s)
185: {
186:     PrintStringTo(baremetal::string("null"), s);
187: }
...
216: /// <summary>
217: /// Universal printer class for reference type, using PrintTo()
218: /// </summary>
219: /// <typeparam name="T">Type value to print</typeparam>
220: template <typename T>
221: class UniversalPrinter<T&>
222: {
223: public:
224:     /// <summary>
225:     /// Print a reference to string
226:     /// </summary>
227:     /// <param name="value">Value to print</param>
228:     /// <param name="s">Resulting string</param>
229:     static void Print(const T& value, baremetal::string& s)
230:     {
231:         // Prints the address of the value.  We use reinterpret_cast here
232:         // as static_cast doesn't compile when T is a function type.
233:         s = "@";
234:         s.append(baremetal::Serialize(reinterpret_cast<const void*>(&value)));
235:         s.append(" ");
236: 
237:         // Then prints the value itself.
238:         PrintTo(value, s);
239:     }
240: };
241: 
242: /// <summary>
243: /// Universal print to string function, uses UniversalPrinter
244: /// </summary>
245: /// <typeparam name="T">Type value to print</typeparam>
246: /// <param name="value">Value to print</param>
247: /// <param name="s">Resulting string</param>
248: template <typename T>
249: void UniversalPrint(const T& value, baremetal::string& s)
250: {
251:     typedef T T1;
252:     UniversalPrinter<T1>::Print(value, s);
253: }
254: 
255: /// <summary>
256: /// Universal terse printer class, uses UniversalPrint
257: /// </summary>
258: /// <typeparam name="T">Type value to print</typeparam>
259: template <typename T>
260: class UniversalTersePrinter
261: {
262: public:
263:     /// <summary>
264:     /// Print a type T to string
265:     /// </summary>
266:     /// <param name="value">Value to print</param>
267:     /// <param name="s">Resulting string</param>
268:     static void Print(const T& value, baremetal::string& s)
269:     {
270:         UniversalPrint(value, s);
271:     }
272: };
273: /// <summary>
274: /// Universal terse printer class for reference, uses UniversalPrint
275: /// </summary>
276: /// <typeparam name="T">Type value to print</typeparam>
277: template <typename T>
278: class UniversalTersePrinter<T&>
279: {
280: public:
281:     /// <summary>
282:     /// Print a reference to string
283:     /// </summary>
284:     /// <param name="value">Value to print</param>
285:     /// <param name="s">Resulting string</param>
286:     static void Print(const T& value, baremetal::string& s)
287:     {
288:         UniversalPrint(value, s);
289:     }
290: };
291: /// <summary>
292: /// Universal terse printer class for const char*, uses UniversalPrint
293: /// </summary>
294: /// <typeparam name="T">Type value to print</typeparam>
295: template <>
296: class UniversalTersePrinter<const char*>
297: {
298: public:
299:     /// <summary>
300:     /// Print a reference to string
301:     /// </summary>
302:     /// <param name="str">Value to print</param>
303:     /// <param name="s">Resulting string</param>
304:     static void Print(const char* str, baremetal::string& s)
305:     {
306:         if (str == nullptr)
307:         {
308:             s = "null";
309:         }
310:         else
311:         {
312:             UniversalPrint(baremetal::string(str), s);
313:         }
314:     }
315: };
316: /// <summary>
317: /// Universal terse printer class for char*, uses UniversalPrint
318: /// </summary>
319: /// <typeparam name="T">Type value to print</typeparam>
320: template <>
321: class UniversalTersePrinter<char*>
322: {
323: public:
324:     /// <summary>
325:     /// Print a reference to string
326:     /// </summary>
327:     /// <param name="str">Value to print</param>
328:     /// <param name="s">Resulting string</param>
329:     static void Print(char* str, baremetal::string& s)
330:     {
331:         UniversalTersePrinter<const char*>::Print(str, s);
332:     }
333: };
...
```

- Line 49: We define a type `nullptr_t` to check for nullptr
- Line 57-58: We define a template function `ImplicitCast_` that casts any type to itself
- Line 76: We declare a variant of `PrintTo()` to print an unsigned char to string
- Line 82: We declare a variant of `PrintTo()` to print a signed char to string
- Line 88-91: We define a variant of `PrintTo()` to print a character to string using the function for unsigned char
- Line 168: We declare a variant of  `PrintTo()` to print a const char* string to string
- Line 114-117: We define a variant of  `PrintTo()` to print a char* string to string using the print function for const char*
- Line 126-129: We define a variant of  `PrintTo()` to print a const signed char* pointer to string using the print function for const void*
- Line 137-140: We define a variant of  `PrintTo()` to print a signed char* pointer to string using the print function for const void*
- Line 148-151: We define a variant of  `PrintTo()` to print a const unsigned char* pointer to string using the print function for const void*
- Line 159-162: We define a variant of  `PrintTo()` to print a unsigned char* pointer to string using the print function for const void*
- Line 169: We declare a function `PrintStringTo()` to print a string to a string
- Line 175-178: We define a variant of  `PrintTo()` to print a string to a string, which uses `PrintStringTo()`
- Line 184-187: We define a variant of  `PrintTo()` to print a nullptr to a string, which uses `PrintStringTo()`
- Line 220-240: We declare a specialization of `UniversalPrinter` for type `T&` that has a single method `Print()` which calls any defined `PrintTo()` function for the value passed to a string, with a prefix to display the address of the string
- Line 295-315: We declare a template class `UniversalTersePrinter` specialization for type `const char*` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string
- Line 321-333: We declare a template class `UniversalTersePrinter` specialization for type `char*` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string

### PrintValue.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_PRINTVALUECPP}

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

### AssertMacros.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_ASSERTMACROSH}

We need to extend the assertion macros a bit.

Update the file `code/libraries/unittest/include/unittest/AssertMacros.h`

```cpp
File: code/libraries/unittest/include/unittest/AssertMacros.h
...
64: #ifdef ASSERT_NEAR
65:     #error unittest redefines ASSERT_NEAR
66: #endif
67: 
...
84: #ifdef EXPECT_NEAR
85:     #error unittest redefines EXPECT_NEAR
86: #endif
87: 
88: namespace unittest
89: {
90: 
91: namespace internal
92: {
93: 
94: // Two overloaded helpers for checking at compile time whether an
95: // expression is a null pointer literal (i.e. nullptr or any 0-valued
96: // compile-time integral constant).  Their return values have
97: // different sizes, so we can use sizeof() to test which version is
98: // picked by the compiler.  These helpers have no implementations, as
99: // we only need their signatures.
100: //
101: // Given IsNullLiteralHelper(x), the compiler will pick the first
102: // version if x can be implicitly converted to Secret*, and pick the
103: // second version otherwise.  Since Secret is a secret and incomplete
104: // type, the only expression a user can write that has type Secret* is
105: // a null pointer literal.  Therefore, we know that x is a null
106: // pointer literal if and only if the first version is picked by the
107: // compiler.
108: class Secret;
109: /// <summary>
110: /// Conversion check function to check whether argument is a pointer
111: /// 
112: /// Not implemented, never called, only declared for return type size
113: /// </summary>
114: /// <param name="p">Argument</param>
115: /// <returns>Unused</returns>
116: char IsNullLiteralHelper(Secret* p);
117: /// <summary>
118: /// Conversion check function to check whether argument is not a pointer
119: /// 
120: /// Not implemented, never called, only declared for return type size
121: /// </summary>
122: /// <returns>Unused</returns>
123: char (&IsNullLiteralHelper(...))[2];
124: 
125: } // namespace internal
126: 
127: } // namespace unittest
128: 
129: 
130: /// @brief Boolean expression to check whether the argument is a null literal. Returns true if the argument is nullptr, false otherwise
131: #define IS_NULL_LITERAL(x) \
132:      (sizeof(::unittest::internal::IsNullLiteralHelper(x)) == 1)
...
166: /// @brief Expect predicate function with three parameters (CheckClose), generates a failure using UT_EXPECT_RESULT if predicate function returns false
167: #define EXPECT_PRED_FORMAT3(pred_format, v1, v2, v3) \
168:   UT_EXPECT_RESULT(pred_format(baremetal::string(#v1), baremetal::string(#v2), baremetal::string(#v3), v1, v2, v3))
169: /// @brief Expect predicate function with three parameters (CheckClose), generates a failure using UT_ASSERT_RESULT if predicate function returns false
170: #define ASSERT_PRED_FORMAT3(pred_format, v1, v2, v3) \
171:   UT_ASSERT_RESULT(pred_format(baremetal::string(#v1), baremetal::string(#v2), baremetal::string(#v3), v1, v2, v3))
172: 
...
201: /// @brief Assert that actual value is equal to expected value
202: #define ASSERT_EQ(expected, actual) \
203:     do \
204:     { \
205:         ASSERT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckEqual, expected, actual); \
206:     } while (0)
207: /// @brief Expect that actual value is equal to expected value
208: #define EXPECT_EQ(expected, actual) \
209:     do \
210:     { \
211:         EXPECT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckEqual, expected, actual); \
212:     } while (0)
213: 
214: /// @brief Assert that actual value is not equal to expected value
215: #define ASSERT_NE(expected, actual) \
216:     do \
217:     { \
218:         ASSERT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckNotEqual, expected, actual); \
219:     } while (0)
220: /// @brief Expect that actual value is not equal to expected value
221: #define EXPECT_NE(expected, actual) \
222:     do \
223:     { \
224:         EXPECT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckNotEqual, expected, actual); \
225:     } while (0)
226: 
227: /// @brief Assert that actual value is equal to expected value ignoring case
228: #define ASSERT_EQ_IGNORE_CASE(expected, actual) \
229:     do \
230:     { \
231:         ASSERT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckEqualIgnoreCase, expected, actual); \
232:     } while (0)
233: /// @brief Expect that actual value is equal to expected value ignoring case
234: #define EXPECT_EQ_IGNORE_CASE(expected, actual) \
235:     do \
236:     { \
237:         EXPECT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckEqualIgnoreCase, expected, actual); \
238:     } while (0)
239: 
240: /// @brief Assert that actual value is not equal to expected value ignoring case
241: #define ASSERT_NE_IGNORE_CASE(expected, actual) \
242:     do \
243:     { \
244:         ASSERT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase, expected, actual); \
245:     } while (0)
246: /// @brief Expect that actual value is not equal to expected value ignoring case
247: #define EXPECT_NE_IGNORE_CASE(expected, actual) \
248:     do \
249:     { \
250:         EXPECT_PRED_FORMAT2(::unittest::EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase, expected, actual); \
251:     } while (0)
252: 
253: /// @brief Assert that actual value is equal to expected value within tolerance (for floating point comparison)
254: #define ASSERT_NEAR(expected, actual, tolerance) \
255:     do \
256:     { \
257:         ASSERT_PRED_FORMAT3(::unittest::CheckClose, expected, actual, tolerance); \
258:     } while (0)
259: /// @brief Expect that actual value is equal to expected value within tolerance (for floating point comparison)
260: #define EXPECT_NEAR(expected, actual, tolerance) \
261:     do \
262:     { \
263:         EXPECT_PRED_FORMAT3(::unittest::CheckClose, expected, actual, tolerance); \
264:     } while (0)
265: 
266: /// @brief Assert that value is nullptr
267: #define ASSERT_NULL(value) ASSERT_EQ(nullptr, value)
268: /// @brief Expect that value is nullptr
269: #define EXPECT_NULL(value) EXPECT_EQ(nullptr, value)
270: /// @brief Assert that value is not nullptr
271: #define ASSERT_NOT_NULL(value) ASSERT_NE(nullptr, value)
272: /// @brief Expect that value is not nullptr
273: #define EXPECT_NOT_NULL(value) EXPECT_NE(nullptr, value)
```

- Line 64-66: We check whether the define `ASSERT_NEAR` is already defined. If so compilation ends with an error
- Line 84-86: We check whether the define `EXPECT_NEAR` is already defined. If so compilation ends with an error
- Line 116-123: We declare two functions `IsNullLiteralHelper()`, of which one takes a pointer, and the other takes a variable argument list.
This is some trickery, to determine if the parameter passed is a nullptr. The first returns a single character, the other a pointer to a char array, resulting in different return types and thus sizes
- Line 131-132: We define a macro `IS_NULL_LITERAL` which is used to check if a pointer is a null pointer
- Line 167-168: We define a macro `EXPECT_PRED_FORMAT3` which is passed a check function and three parameters.
The check function is called, and the result is passed to `UT_EXPECT_RESULT`.
The three parameter version is used with `CheckClose()` to check the result of comparison with a tolerance
- Line 170-171: We define a macro `ASSERT_PRED_FORMAT3` which is passed a check function and three parameters.
The check function is called, and the result is passed to `UT_ASSERT_RESULT`.
The three parameter version is used with `CheckClose()` to check the result of comparison with a tolerance
- Line 202-206: We change the macro `ASSERT_EQ` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 208-212: We change the macro `EXPECT_EQ` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 215-219: We change the macro `ASSERT_NE` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 221-225: We change the macro `EXPECT_NE` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 228-232: We define a macro `ASSERT_EQ_IGNORE_CASE` which uses `ASSERT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckEqualIgnoreCase` to check if the parameters are not equal, and generate a `EqFailure` if the check fails
- Line 234-238: We define a macro `EXPECT_EQ_IGNORE_CASE` which uses `EXPECT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckEqualIgnoreCase` to check if the parameters are not equal, and generate a `EqFailure` if the check fails
- Line 241-245: We define a macro `ASSERT_NE_IGNORE_CASE` which uses `ASSERT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase` to check if the parameters are not equal, and generate a `InEqFailure` if the check fails
- Line 247-251: We define a macro `EXPECT_NE_IGNORE_CASE` which uses `EXPECT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase` to check if the parameters are not equal, and generate a `InEqFailure` if the check fails
- Line 254-258: We define a macro `ASSERT_NEAR` which uses `ASSERT_PRED_FORMAT3` with `CheckClose` to check if the absolute difference of compared values is within tolerance, and generate a `CloseFailure` if the check fails
- Line 260-264: We define a macro `EXPECT_NEAR` which uses `EXPECT_PRED_FORMAT3` with `CheckClose` to check if the absolute difference of compared values is within tolerance, and generate a `CloseFailure` if the check fails
- Line 267: We define a macro `ASSERT_NULL` to check if the parameter is a null pointer, and generate a `EqFailure` if the check fails
- Line 269: We define a macro `EXPECT_NULL` to check if the parameter is a null pointer, and generate a `EqFailure` if the check fails
- Line 271: We define a macro `ASSERT_NOT_NULL` to check if the parameter is not a null pointer, and generate a `InEqFailure` if the check fails
- Line 273: We define a macro `EXPECT_NOT_NULL` to check if the parameter is not a null pointer, and generate a `InEqFailure` if the check fails

### Update project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
25: set(PROJECT_LIBS
26:     ${LINKER_LIBRARIES}
27:     ${PROJECT_DEPENDENCIES}
28:     )
29: 
30:     set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Checks.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleTestReporter.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CurrentTest.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/DeferredTestReporter.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PrintValue.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Test.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestDetails.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestFixtureInfo.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestInfo.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRegistry.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResult.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestResults.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestRunner.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/TestSuiteInfo.cpp
45:     )
46: 
47: set(PROJECT_INCLUDES_PUBLIC
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/AssertMacros.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Checks.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/ConsoleTestReporter.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/CurrentTest.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/DeferredTestReporter.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/PrintValue.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/Test.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestDetails.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixture.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestFixtureInfo.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/unittest/TestInfo.h
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

### Application code {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_APPLICATION_CODE}

We'll use `Test` to use the new macros defined.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
...
90: TEST(Test4)
91: {
92:    int* p = nullptr;
93:    int dd = 123;
94:    int* q = &dd;
95:    int* r = &dd;
96:    ASSERT_NULL(p);
97:    EXPECT_NULL(p);
98:    ASSERT_NULL(q);
99:    EXPECT_NULL(q);
100:    ASSERT_NOT_NULL(p);
101:    EXPECT_NOT_NULL(p);
102:    ASSERT_NOT_NULL(q);
103:    EXPECT_NOT_NULL(q);
104:    baremetal::string s1 = "A";
105:    baremetal::string s2 = "B";
106:    baremetal::string s3 = "B";
107:    baremetal::string s4 = "b";
108:    ASSERT_EQ(s1, s2);
109:    EXPECT_EQ(s1, s2);
110:    ASSERT_EQ(s2, s3);
111:    EXPECT_EQ(s2, s3);
112:    ASSERT_NE(s1, s2);
113:    EXPECT_NE(s1, s2);
114:    ASSERT_NE(s2, s3);
115:    EXPECT_NE(s2, s3);
116:    ASSERT_EQ_IGNORE_CASE(s1, s2);
117:    EXPECT_EQ_IGNORE_CASE(s1, s2);
118:    ASSERT_EQ_IGNORE_CASE(s2, s3);
119:    EXPECT_EQ_IGNORE_CASE(s2, s3);
120:    ASSERT_NE_IGNORE_CASE(s1, s2);
121:    EXPECT_NE_IGNORE_CASE(s1, s2);
122:    ASSERT_NE_IGNORE_CASE(s2, s3);
123:    EXPECT_NE_IGNORE_CASE(s2, s3);
124:    ASSERT_EQ_IGNORE_CASE(s2, s4);
125:    EXPECT_EQ_IGNORE_CASE(s2, s4);
126:    ASSERT_NE_IGNORE_CASE(s2, s4);
127:    EXPECT_NE_IGNORE_CASE(s2, s4);
128:    char t[] = { 'A', '\0' };
129:    char u[] = { 'B', '\0' };
130:    char v[] = { 'B', '\0' };
131:    char w[] = { 'b', '\0' };
132:    const char* tC = "A";
133:    const char* uC = "B";
134:    const char* vC = "B";
135:    const char* wC = "b";
136:    ASSERT_EQ(t, u);
137:    EXPECT_EQ(t, u);
138:    ASSERT_EQ(u, v);
139:    EXPECT_EQ(u, v);
140:    ASSERT_EQ(t, u);
141:    EXPECT_EQ(t, uC);
142:    ASSERT_EQ(uC, v);
143:    EXPECT_EQ(uC, vC);
144:    ASSERT_EQ(t, w);
145:    EXPECT_EQ(t, wC);
146:    ASSERT_EQ(uC, w);
147:    EXPECT_EQ(uC, wC);
148:    ASSERT_NE(t, u);
149:    EXPECT_NE(t, u);
150:    ASSERT_NE(u, v);
151:    EXPECT_NE(u, v);
152:    ASSERT_NE(t, u);
153:    EXPECT_NE(t, uC);
154:    ASSERT_NE(uC, v);
155:    EXPECT_NE(uC, vC);
156:    ASSERT_NE(t, w);
157:    EXPECT_NE(t, wC);
158:    ASSERT_NE(uC, w);
159:    EXPECT_NE(uC, wC);
160:    ASSERT_EQ_IGNORE_CASE(t, u);
161:    EXPECT_EQ_IGNORE_CASE(t, u);
162:    ASSERT_EQ_IGNORE_CASE(u, v);
163:    EXPECT_EQ_IGNORE_CASE(u, v);
164:    ASSERT_EQ_IGNORE_CASE(t, u);
165:    EXPECT_EQ_IGNORE_CASE(t, uC);
166:    ASSERT_EQ_IGNORE_CASE(uC, v);
167:    EXPECT_EQ_IGNORE_CASE(uC, vC);
168:    ASSERT_EQ_IGNORE_CASE(t, w);
169:    EXPECT_EQ_IGNORE_CASE(t, wC);
170:    ASSERT_EQ_IGNORE_CASE(uC, w);
171:    EXPECT_EQ_IGNORE_CASE(uC, wC);
172:    ASSERT_NE_IGNORE_CASE(t, u);
173:    EXPECT_NE_IGNORE_CASE(t, u);
174:    ASSERT_NE_IGNORE_CASE(u, v);
175:    EXPECT_NE_IGNORE_CASE(u, v);
176:    ASSERT_NE_IGNORE_CASE(t, u);
177:    EXPECT_NE_IGNORE_CASE(t, uC);
178:    ASSERT_NE_IGNORE_CASE(uC, v);
179:    EXPECT_NE_IGNORE_CASE(uC, vC);
180:    ASSERT_NE_IGNORE_CASE(t, w);
181:    EXPECT_NE_IGNORE_CASE(t, wC);
182:    ASSERT_NE_IGNORE_CASE(uC, w);
183:    EXPECT_NE_IGNORE_CASE(uC, wC);
184: 
185:    double a = 0.123;
186:    double b = 0.122;
187:    ASSERT_EQ(a, b);
188:    EXPECT_EQ(a, b);
189:    ASSERT_NEAR(a, b, 0.0001);
190:    EXPECT_NEAR(a, b, 0.0001);
191:    ASSERT_NEAR(a, b, 0.001);
192:    EXPECT_NEAR(a, b, 0.001);
193: }
...
```

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:201)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
[  FAILED   ] Suite1::FixtureMyTest1::MyTest1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
[  FAILED   ] Suite2::FixtureMyTest2::MyTest2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
[  FAILED   ] DefaultSuite::FixtureMyTest3::MyTest3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
[  FAILED   ] DefaultSuite::DefaultFixture::MyTest
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 4 out of 4 tests failed (47 failures).

Failures:
../code/applications/demo/src/main.cpp:33 : Failure in Suite1::FixtureMyTest1::MyTest1: For some reason
../code/applications/demo/src/main.cpp:59 : Failure in Suite2::FixtureMyTest2::MyTest2: Value of: false
  Expected: true

../code/applications/demo/src/main.cpp:60 : Failure in Suite2::FixtureMyTest2::MyTest2: Value of: true
  Expected: false

../code/applications/demo/src/main.cpp:84 : Failure in DefaultSuite::FixtureMyTest3::MyTest3: Value of: y
  Actual: 1
  Expected: x
  Which is: 0

../code/applications/demo/src/main.cpp:87 : Failure in DefaultSuite::FixtureMyTest3::MyTest3: Value of: z
  Actual: 1
  Expected not equal to: y
  Which is: 1

../code/applications/demo/src/main.cpp:98 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: q
  Actual: 0x000000000029CC54
  Expected: nullptr
  Which is: null

../code/applications/demo/src/main.cpp:99 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: q
  Actual: 0x000000000029CC54
  Expected: nullptr
  Which is: null

../code/applications/demo/src/main.cpp:100 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: p
  Actual: null
  Expected not equal to: nullptr
  Which is: null

../code/applications/demo/src/main.cpp:101 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: p
  Actual: null
  Expected not equal to: nullptr
  Which is: null

../code/applications/demo/src/main.cpp:108 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s2
  Actual: B
  Expected: s1
  Which is: A

../code/applications/demo/src/main.cpp:109 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s2
  Actual: B
  Expected: s1
  Which is: A

../code/applications/demo/src/main.cpp:114 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s3
  Actual: B
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:115 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s3
  Actual: B
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:116 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s2
  Actual: B
  Expected: s1
  Which is: A

../code/applications/demo/src/main.cpp:117 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s2
  Actual: B
  Expected: s1
  Which is: A

../code/applications/demo/src/main.cpp:122 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s3
  Actual: B
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:123 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s3
  Actual: B
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:126 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s4
  Actual: b
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:127 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: s4
  Actual: b
  Expected not equal to: s2
  Which is: B

../code/applications/demo/src/main.cpp:136 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:137 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:140 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:141 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: uC
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:144 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: w
  Actual: b
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:145 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: wC
  Actual: b
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:146 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: w
  Actual: b
  Expected: uC
  Which is: B

../code/applications/demo/src/main.cpp:147 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: wC
  Actual: b
  Expected: uC
  Which is: B

../code/applications/demo/src/main.cpp:150 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: v
  Actual: B
  Expected not equal to: u
  Which is: B

../code/applications/demo/src/main.cpp:151 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: v
  Actual: B
  Expected not equal to: u
  Which is: B

../code/applications/demo/src/main.cpp:154 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: v
  Actual: B
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:155 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: vC
  Actual: B
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:160 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:161 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:164 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: u
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:165 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: uC
  Actual: B
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:168 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: w
  Actual: b
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:169 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: wC
  Actual: b
  Expected: t
  Which is: A

../code/applications/demo/src/main.cpp:174 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: v
  Actual: B
  Expected not equal to: u
  Which is: B

../code/applications/demo/src/main.cpp:175 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: v
  Actual: B
  Expected not equal to: u
  Which is: B

../code/applications/demo/src/main.cpp:178 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: v
  Actual: B
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:179 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: vC
  Actual: B
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:182 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: w
  Actual: b
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:183 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: wC
  Actual: b
  Expected not equal to: uC
  Which is: B

../code/applications/demo/src/main.cpp:187 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: b
  Actual: 0.12200000000000
  Expected: a
  Which is: 0.12300000000000

../code/applications/demo/src/main.cpp:188 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: b
  Actual: 0.12200000000000
  Expected: a
  Which is: 0.12300000000000

../code/applications/demo/src/main.cpp:189 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: b
  Actual: 0.12200000000000
  Expected: a
  Which is: 0.12300000000000
  Tolerance: 0.0001
  (+/-) 0.10000000000000
../code/applications/demo/src/main.cpp:190 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: b
  Actual: 0.12200000000000
  Expected: a
  Which is: 0.12300000000000
  Tolerance: 0.0001
  (+/-) 0.10000000000000

[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   Wait 5 seconds (main:202)
Press r to reboot, h to halt, p to fail assertion and panic
```

## Writing class tests for string - Step 4 {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4}

Now that our unit test framework is complete, let's reimplement the tests we created in [15-string](#TUTORIAL_15_STRING).
First we'll create a folder `test` underneath `code/libraries/baremetal`, and underneath a folder `src`.
We'll also be creating a CMake file for the tests, as well as the structure for the main application to create a kernel image.

<img src="images/unittest-add-test-project.png" alt="Tree view" width="400"/>

### main.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_MAINCPP}

First we'll create the main source file that will run the tests.

Create the file `code\libraries\baremetal\test\main.cpp`

```cpp
File: code\libraries\baremetal\test\main.cpp
1: #include <baremetal/System.h>
2: #include <unittest/unittest.h>
3: 
4: using namespace baremetal;
5: using namespace unittest;
6: 
7: int main()
8: {
9:     ConsoleTestReporter reporter;
10:     RunAllTests(&reporter);
11: 
12:     return static_cast<int>(ReturnCode::ExitHalt);
13: }
14: 
```

### StringTest.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_STRINGTESTCPP}

We'll add the source file containing the string tests.

As this file is quite sizeable, we'll not repeat the full source here, just an excerpt.

Create the file `code\libraries\baremetal\test\StringTest.cpp`

```cpp
File: code\libraries\baremetal\test\StringTest.cpp
40: #include "unittest/unittest.h"
41: 
42: #include "baremetal/String.h"
43: #include "baremetal/Util.h"
44: 
45: using namespace unittest;
46: 
47: namespace baremetal {
48: namespace test {
49: 
50: TEST_SUITE(Baremetal)
51: {
52: 
53: class StringTest
54:     : public TestFixture
55: {
56: public:
57:     const char* otherText = "abcdefghijklmnopqrstuvwxyz";
58:     string other;
59:     void SetUp() override
60:     {
61:         other = otherText;
62:     }
63:     void TearDown() override
64:     {
65:     }
66: };
67: 
68: TEST_FIXTURE(StringTest, ConstructDefault)
69: {
70:     string s;
71:     EXPECT_TRUE(s.empty());
72:     ASSERT_NOT_NULL(s.data());
73:     ASSERT_NOT_NULL(s.c_str());
74:     EXPECT_EQ('\0', s.data()[0]);
75:     EXPECT_EQ(size_t{ 0 }, s.size());
76:     EXPECT_EQ(size_t{ 0 }, s.length());
77:     EXPECT_EQ(size_t{ 0 }, s.capacity());
78: }
79: 
80: TEST_FIXTURE(StringTest, ConstructConstCharPtr)
81: {
82:     const char* text = otherText;
83:     const char* expected = otherText;
84:     size_t expectedLength = strlen(expected);
85:     size_t length = strlen(expected);
86: 
87:     string s(text);
88: 
89:     EXPECT_FALSE(s.empty());
90:     ASSERT_NOT_NULL(s.data());
91:     ASSERT_NOT_NULL(s.c_str());
92:     EXPECT_NE('\0', s.data()[0]);
93:     EXPECT_EQ(expectedLength, s.size());
94:     EXPECT_EQ(expectedLength, s.length());
95:     EXPECT_NE(size_t{ 0 }, s.capacity());
96:     EXPECT_TRUE(expected == s);
97:     EXPECT_TRUE(s == expected);
98:     EXPECT_EQ(expected, s);
99:     EXPECT_EQ(s, expected);
100: }
101: 
102: TEST_FIXTURE(StringTest, ConstructConstCharPtrEmpty)
103: {
104:     const char* text = "";
105:     const char* expected = "";
106:     size_t expectedLength = strlen(expected);
107: 
108:     string s(text);
109: 
110:     EXPECT_TRUE(s.empty());
111:     ASSERT_NOT_NULL(s.data());
112:     ASSERT_NOT_NULL(s.c_str());
113:     EXPECT_EQ('\0', s.data()[0]);
114:     EXPECT_EQ(expectedLength, s.size());
115:     EXPECT_EQ(expectedLength, s.length());
116:     EXPECT_NE(size_t{ 0 }, s.capacity());
117:     EXPECT_TRUE(expected == s);
118:     EXPECT_TRUE(s == expected);
119:     EXPECT_EQ(expected, s);
120:     EXPECT_EQ(s, expected);
121: }
122: 
123: TEST_FIXTURE(StringTest, ConstructNullPtr)
124: {
125:     const char* expected = "";
126:     size_t expectedLength = strlen(expected);
127: 
128:     string s(nullptr);
129: 
130:     EXPECT_TRUE(s.empty());
131:     ASSERT_NOT_NULL(s.data());
132:     ASSERT_NOT_NULL(s.c_str());
133:     EXPECT_EQ('\0', s.data()[0]);
134:     EXPECT_EQ(expectedLength, s.size());
135:     EXPECT_EQ(expectedLength, s.length());
136:     EXPECT_EQ(size_t{ 0 }, s.capacity());
137:     EXPECT_TRUE(expected == s);
138:     EXPECT_TRUE(s == expected);
139:     EXPECT_EQ(expected, s);
140:     EXPECT_EQ(s, expected);
141: }
```

- Line 50: We define the suite for these class tests, which is named after the library, so `Baremetal`
- Line 53-66: We define the class for the test fixture. Notice that we collect some often used variables in the class, and initialize using the `SetUp()` method
- Line 68-78: As an example, we test the default constructor
- Line 80-100: We test the constructor taking const char*
- Line 102-121: We test the constructor taking const char* for the corner case that an empty string is passed
- Line 123-141: We test the constructor taking const char* for the corner case that a nullptr is passed

### Set up test project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_SET_UP_TEST_PROJECT_CONFIGURATION}

We'll fill in the CMake file for the test project. This will be very similar to the CMake file for the main application project.

Create the file `code/libraries/baremetal/test/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/test/CMakeLists.txt
1: project(baremetal-test
2:     DESCRIPTION "Baremetal test application"
3:     LANGUAGES CXX)
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
27:     unittest
28:     )
29: 
30: set(PROJECT_LIBS
31:     ${LINKER_LIBRARIES}
32:     ${PROJECT_DEPENDENCIES}
33:     )
34: 
35: set(PROJECT_SOURCES
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/StringTest.cpp
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
66: if (PLATFORM_BAREMETAL)
67:     set(START_GROUP -Wl,--start-group)
68:     set(END_GROUP -Wl,--end-group)
69: endif()
70: 
71: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
72: 
73: target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
74: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
75: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
76: target_compile_definitions(${PROJECT_NAME} PRIVATE
77:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
78:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
79:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
80:     )
81: target_compile_definitions(${PROJECT_NAME} PUBLIC
82:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
83:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
84:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
85:     )
86: target_compile_options(${PROJECT_NAME} PRIVATE
87:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
88:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
89:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
90:     )
91: target_compile_options(${PROJECT_NAME} PUBLIC
92:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
93:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
94:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
95:     )
96: 
97: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
98: 
99: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
100: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
101:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
102: endif()
103: 
104: link_directories(${LINK_DIRECTORIES})
105: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
106: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
107: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
108: 
109: show_target_properties(${PROJECT_NAME})
110: 
111: add_subdirectory(create-image)
```

### Add image creation project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_ADD_IMAGE_CREATION_PROJECT_CONFIGURATION}

We'll add the CMake file for the test project kernel image. This will be very similar to the CMake file for the main application project kernel image.

Create the file `code/libraries/baremetal/test/create-image/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/test/create-image/CMakeLists.txt
1: project(baremetal-test-image
2:     DESCRIPTION "Kernel image for baremetal test")
3: 
4: message(STATUS "\n**********************************************************************************\n")
5: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
6: 
7: message("\n** Setting up ${PROJECT_NAME} **\n")
8: 
9: set(DEPENDENCY baremetal-test)
10: set(IMAGE_NAME ${BAREMETAL_TARGET_KERNEL}.img)
11: 
12: create_image(${PROJECT_NAME} ${IMAGE_NAME} ${DEPENDENCY})
```

### Update baremetal project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_UPDATE_BAREMETAL_PROJECT_CONFIGURATION}

We need to add the test project CMake file to the baremetal project CMake file in order to build the test project.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/test/CMakeLists.txt
150: show_target_properties(${PROJECT_NAME})
151: 
152: add_subdirectory(test)
```

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. All tests should succeed.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:201)
[===========] Running 156 tests from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (1 fixture)
[  FIXTURE  ] StringTest (156 tests)
[ SUCCEEDED ] Baremetal::StringTest::ConstructDefault
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::ConstructNullPtr
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtrAndSize
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtrAndSizeTooLarge
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtrAndSizeNpos
[ SUCCEEDED ] Baremetal::StringTest::ConstructCountAndChar
[ SUCCEEDED ] Baremetal::StringTest::ConstructCountNposAndChar
[ SUCCEEDED ] Baremetal::StringTest::ConstructCopy
[ SUCCEEDED ] Baremetal::StringTest::ConstructCopyEmpty
[ SUCCEEDED ] Baremetal::StringTest::ConstructMove
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPos
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPosNpos
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPosWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::ConstCharPtrCastOperator
[ SUCCEEDED ] Baremetal::StringTest::AssignmentOperatorConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AssignmentOperatorCopy
[ SUCCEEDED ] Baremetal::StringTest::AssignmentOperatorMove
[ SUCCEEDED ] Baremetal::StringTest::Begin
[ SUCCEEDED ] Baremetal::StringTest::End
[ SUCCEEDED ] Baremetal::StringTest::BeginConst
[ SUCCEEDED ] Baremetal::StringTest::EndConst
[ SUCCEEDED ] Baremetal::StringTest::Iterate
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::AssignNullPtr
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtrAndSize
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtrAndSizeTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtrAndSizeNpos
[ SUCCEEDED ] Baremetal::StringTest::AssignCountAndChar
[ SUCCEEDED ] Baremetal::StringTest::AssignCountNposAndChar
[ SUCCEEDED ] Baremetal::StringTest::AssignString
[ SUCCEEDED ] Baremetal::StringTest::AssignStringEmpty
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPos
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPosNPos
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPosWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::At
[ SUCCEEDED ] Baremetal::StringTest::AtConst
[ SUCCEEDED ] Baremetal::StringTest::AtOutsideString
[ SUCCEEDED ] Baremetal::StringTest::AtConstOutsideString
[ SUCCEEDED ] Baremetal::StringTest::AtEmptyString
[ SUCCEEDED ] Baremetal::StringTest::AtConstEmptyString
[ SUCCEEDED ] Baremetal::StringTest::Front
[ SUCCEEDED ] Baremetal::StringTest::FrontConst
[ SUCCEEDED ] Baremetal::StringTest::FrontEmptyString
[ SUCCEEDED ] Baremetal::StringTest::FrontConstEmptyString
[ SUCCEEDED ] Baremetal::StringTest::Back
[ SUCCEEDED ] Baremetal::StringTest::BackConst
[ SUCCEEDED ] Baremetal::StringTest::BackEmptyString
[ SUCCEEDED ] Baremetal::StringTest::BackConstEmptyString
[ SUCCEEDED ] Baremetal::StringTest::IndexOperator
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorConst
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorOutsideString
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorConstOutsideString
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorEmptyString
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorConstEmptyString
[ SUCCEEDED ] Baremetal::StringTest::Data
[ SUCCEEDED ] Baremetal::StringTest::DataConst
[ SUCCEEDED ] Baremetal::StringTest::CString
[ SUCCEEDED ] Baremetal::StringTest::Reserve
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentChar
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentString
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentStringEmpty
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentNullPtr
[ SUCCEEDED ] Baremetal::StringTest::AppendCountAndChar
[ SUCCEEDED ] Baremetal::StringTest::AppendCountNposAndChar
[ SUCCEEDED ] Baremetal::StringTest::AppendString
[ SUCCEEDED ] Baremetal::StringTest::AppendStringEmpty
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPos
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPosNpos
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPosWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::AppendNullPtr
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtrWithCount
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtrWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtrWithCountNpos
[ SUCCEEDED ] Baremetal::StringTest::Clear
[ SUCCEEDED ] Baremetal::StringTest::FindString
[ SUCCEEDED ] Baremetal::StringTest::FindStringEndOfString
[ SUCCEEDED ] Baremetal::StringTest::FindStringNoMatch
[ SUCCEEDED ] Baremetal::StringTest::FindStringEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPos
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosEndOfString
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosNpos
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrNoMatch
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrNullPtr
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPos
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosEndOfString
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosNpos
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosTooLargeEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosNullPtr
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountNoMatch
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountMatchPart
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountTooLargeEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountNullPtr
[ SUCCEEDED ] Baremetal::StringTest::FindChar
[ SUCCEEDED ] Baremetal::StringTest::FindCharNoMatch
[ SUCCEEDED ] Baremetal::StringTest::FindCharAtPos
[ SUCCEEDED ] Baremetal::StringTest::FindCharAtPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::FindCharAtPosNpos
[ SUCCEEDED ] Baremetal::StringTest::StartsWithString
[ SUCCEEDED ] Baremetal::StringTest::StartsWithConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::StartsWithChar
[ SUCCEEDED ] Baremetal::StringTest::EndsWithString
[ SUCCEEDED ] Baremetal::StringTest::EndsWithConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::EndsWithChar
[ SUCCEEDED ] Baremetal::StringTest::ContainsString
[ SUCCEEDED ] Baremetal::StringTest::ContainsConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::ContainsChar
[ SUCCEEDED ] Baremetal::StringTest::SubStr
[ SUCCEEDED ] Baremetal::StringTest::Equals
[ SUCCEEDED ] Baremetal::StringTest::EqualsCaseInsensitive
[ SUCCEEDED ] Baremetal::StringTest::Compare
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountString
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountStringPos
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountStringPosCount
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountConstCharPtrCount
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosChar
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCharCount
[ SUCCEEDED ] Baremetal::StringTest::ReplaceSubstringString
[ SUCCEEDED ] Baremetal::StringTest::ReplaceSubstringStringMultiple
[ SUCCEEDED ] Baremetal::StringTest::ReplaceSubstringConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::ReplaceSubstringConstCharPtrMultiple
[ SUCCEEDED ] Baremetal::StringTest::Align
[ SUCCEEDED ] Baremetal::StringTest::EqualityOperator
[ SUCCEEDED ] Baremetal::StringTest::InEqualityOperator
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndString
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndNullPtr
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorConstCharPtrAndString
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorConstCharPtrEmptyAndString
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorNullPtrAndString
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndChar
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorCharAndString
[  FIXTURE  ] 156 tests from StringTest
[   SUITE   ] 1 fixture from Baremetal
Success: 156 tests passed.

No failures
[===========] 156 tests from 1 fixture in 1 suite ran.
Info   Halt (System:122)
```

As you can see, we have 156 test for the `string` class, which actually have a multitude of this in test cases.

## Writing class tests for serialization - Step 5 {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_SERIALIZATION___STEP_5}

Let's also reimplement the tests for [16-serializing-and-formatting](#TUTORIAL_16_SERIALIZATION_AND_FORMATTING).

### SerializationTest.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_SERIALIZATION___STEP_5_SERIALIZATIONTESTCPP}

We'll add the source file containing the serialization tests.

Create the file `code\libraries\baremetal\test\SerializationTest.cpp`

```cpp
File: code\libraries\baremetal\test\SerializationTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : SerializationTest.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : SerializationTest
9: //
10: // Description : Serialization tests
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
15: //
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
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
40: #include "unittest/unittest.h"
41: 
42: #include "baremetal/Serialization.h"
43: 
44: using namespace unittest;
45: 
46: namespace baremetal {
47: namespace test {
48: 
49: TEST_SUITE(Baremetal)
50: {
51: 
52: class SerializationTest
53:     : public TestFixture
54: {
55: public:
56:     void SetUp() override
57:     {
58:     }
59:     void TearDown() override
60:     {
61:     }
62: };
63: 
64: TEST_FIXTURE(SerializationTest, SerializeChar)
65: {
66:     char c = 'A';
67:     EXPECT_EQ("65", Serialize(c));
68:     EXPECT_EQ("  65", Serialize(c, 4));
69:     EXPECT_EQ("65  ", Serialize(c, -4));
70: }
71: 
72: TEST_FIXTURE(SerializationTest, SerializeIntegerWithPrefix)
73: {
74:     int32 i32 = 1234567890l;
75:     EXPECT_EQ("0b1001001100101100000001011010010", Serialize(i32, 0, 2, true));
76:     EXPECT_EQ("011145401322", Serialize(i32, 0, 8, true));
77:     EXPECT_EQ("1234567890", Serialize(i32, 0, 10, true));
78:     EXPECT_EQ("0x499602D2", Serialize(i32, 0, 16, true));
79: }
80: 
81: TEST_FIXTURE(SerializationTest, SerializeIntegerWithPrefixAndLeadingZeros)
82: {
83:     int32 i32 = 1234567890l;
84:     EXPECT_EQ("0b01001001100101100000001011010010", Serialize(i32, 32, 2, true, true));
85:     EXPECT_EQ("00000011145401322", Serialize(i32, 16, 8, true, true));
86:     EXPECT_EQ("0000001234567890", Serialize(i32, 16, 10, true, true));
87:     EXPECT_EQ("0x0000499602D2", Serialize(i32, 12, 16, true, true));
88: }
89: 
90: TEST_FIXTURE(SerializationTest, SerializeIntegerWithoutPrefixWithLeadingZeros)
91: {
92:     int32 i32 = 1234567890l;
93:     EXPECT_EQ("01001001100101100000001011010010", Serialize(i32, 32, 2, false, true));
94:     EXPECT_EQ("0000011145401322", Serialize(i32, 16, 8, false, true));
95:     EXPECT_EQ("0000001234567890", Serialize(i32, 16, 10, false, true));
96:     EXPECT_EQ("0000499602D2", Serialize(i32, 12, 16, false, true));
97: }
98: 
99: TEST_FIXTURE(SerializationTest, SerializeInt8)
100: {
101:     int8 i8 = 123;
102:     EXPECT_EQ("123", Serialize(i8));
103:     EXPECT_EQ("     123", Serialize(i8, 8));
104:     EXPECT_EQ("123     ", Serialize(i8, -8));
105:     EXPECT_EQ(" 1111011", Serialize(i8, 8, 2));
106:     EXPECT_EQ("     173", Serialize(i8, 8, 8));
107:     EXPECT_EQ("     123", Serialize(i8, 8, 10));
108:     EXPECT_EQ("      7B", Serialize(i8, 8, 16));
109:     EXPECT_EQ("    0x7B", Serialize(i8, 8, 16, true));
110:     EXPECT_EQ("0x0000007B", Serialize(i8, 8, 16, true, true));
111:     EXPECT_EQ("0000007B", Serialize(i8, 8, 16, false, true));
112: }
113: 
114: TEST_FIXTURE(SerializationTest, SerializeUInt8)
115: {
116:     uint8 u8 = 234;
117:     EXPECT_EQ("234", Serialize(u8));
118:     EXPECT_EQ("     234", Serialize(u8, 8));
119:     EXPECT_EQ("234     ", Serialize(u8, -8));
120:     EXPECT_EQ("11101010", Serialize(u8, 8, 2));
121:     EXPECT_EQ("     352", Serialize(u8, 8, 8));
122:     EXPECT_EQ("     234", Serialize(u8, 8, 10));
123:     EXPECT_EQ("      EA", Serialize(u8, 8, 16));
124:     EXPECT_EQ("    0xEA", Serialize(u8, 8, 16, true));
125:     EXPECT_EQ("0x000000EA", Serialize(u8, 8, 16, true, true));
126:     EXPECT_EQ("000000EA", Serialize(u8, 8, 16, false, true));
127: }
128: 
129: TEST_FIXTURE(SerializationTest, SerializeInt16)
130: {
131:     int16 i16 = 12345;
132:     EXPECT_EQ("12345", Serialize(i16));
133:     EXPECT_EQ("   12345", Serialize(i16, 8));
134:     EXPECT_EQ("12345   ", Serialize(i16, -8));
135:     EXPECT_EQ("  11000000111001", Serialize(i16, 16, 2));
136:     EXPECT_EQ("   30071", Serialize(i16, 8, 8));
137:     EXPECT_EQ("   12345", Serialize(i16, 8, 10));
138:     EXPECT_EQ("    3039", Serialize(i16, 8, 16));
139:     EXPECT_EQ("  0x3039", Serialize(i16, 8, 16, true));
140:     EXPECT_EQ("0x00003039", Serialize(i16, 8, 16, true, true));
141:     EXPECT_EQ("00003039", Serialize(i16, 8, 16, false, true));
142: }
143: 
144: TEST_FIXTURE(SerializationTest, SerializeUInt16)
145: {
146:     uint16 u16 = 34567;
147:     EXPECT_EQ("34567", Serialize(u16));
148:     EXPECT_EQ("   34567", Serialize(u16, 8));
149:     EXPECT_EQ("34567   ", Serialize(u16, -8));
150:     EXPECT_EQ("1000011100000111", Serialize(u16, 16, 2));
151:     EXPECT_EQ("  103407", Serialize(u16, 8, 8));
152:     EXPECT_EQ("   34567", Serialize(u16, 8, 10));
153:     EXPECT_EQ("    8707", Serialize(u16, 8, 16));
154:     EXPECT_EQ("  0x8707", Serialize(u16, 8, 16, true));
155:     EXPECT_EQ("0x00008707", Serialize(u16, 8, 16, true, true));
156:     EXPECT_EQ("00008707", Serialize(u16, 8, 16, false, true));
157: }
158: 
159: TEST_FIXTURE(SerializationTest, SerializeInt32)
160: {
161:     int32 i32 = 1234567890l;
162:     EXPECT_EQ("1234567890", Serialize(i32));
163:     EXPECT_EQ("  1234567890", Serialize(i32, 12));
164:     EXPECT_EQ("1234567890  ", Serialize(i32, -12));
165:     EXPECT_EQ(" 1001001100101100000001011010010", Serialize(i32, 32, 2));
166:     EXPECT_EQ(" 11145401322", Serialize(i32, 12, 8));
167:     EXPECT_EQ("  1234567890", Serialize(i32, 12, 10));
168:     EXPECT_EQ("    499602D2", Serialize(i32, 12, 16));
169:     EXPECT_EQ("  0x499602D2", Serialize(i32, 12, 16, true));
170:     EXPECT_EQ("0x0000499602D2", Serialize(i32, 12, 16, true, true));
171:     EXPECT_EQ("0000499602D2", Serialize(i32, 12, 16, false, true));
172: }
173: 
174: TEST_FIXTURE(SerializationTest, SerializeUInt32)
175: {
176:     uint32 u32 = 2345678900ul;
177:     EXPECT_EQ("2345678900", Serialize(u32));
178:     EXPECT_EQ("  2345678900", Serialize(u32, 12));
179:     EXPECT_EQ("2345678900  ", Serialize(u32, -12));
180:     EXPECT_EQ("10001011110100000011100000110100", Serialize(u32, 32, 2));
181:     EXPECT_EQ(" 21364034064", Serialize(u32, 12, 8));
182:     EXPECT_EQ("  2345678900", Serialize(u32, 12, 10));
183:     EXPECT_EQ("    8BD03834", Serialize(u32, 12, 16));
184:     EXPECT_EQ("  0x8BD03834", Serialize(u32, 12, 16, true));
185:     EXPECT_EQ("0x00008BD03834", Serialize(u32, 12, 16, true, true));
186:     EXPECT_EQ("00008BD03834", Serialize(u32, 12, 16, false, true));
187: }
188: 
189: TEST_FIXTURE(SerializationTest, SerializeInt64)
190: {
191:     int64 i64 = 9223372036854775807ll;
192:     EXPECT_EQ("9223372036854775807", Serialize(i64));
193:     EXPECT_EQ("     9223372036854775807", Serialize(i64, 24));
194:     EXPECT_EQ("9223372036854775807     ", Serialize(i64, -24));
195:     EXPECT_EQ(" 111111111111111111111111111111111111111111111111111111111111111", Serialize(i64, 64, 2));
196:     EXPECT_EQ("   777777777777777777777", Serialize(i64, 24, 8));
197:     EXPECT_EQ("     9223372036854775807", Serialize(i64, 24, 10));
198:     EXPECT_EQ("        7FFFFFFFFFFFFFFF", Serialize(i64, 24, 16));
199:     EXPECT_EQ("      0x7FFFFFFFFFFFFFFF", Serialize(i64, 24, 16, true));
200:     EXPECT_EQ("0x000000007FFFFFFFFFFFFFFF", Serialize(i64, 24, 16, true, true));
201:     EXPECT_EQ("000000007FFFFFFFFFFFFFFF", Serialize(i64, 24, 16, false, true));
202: }
203: 
204: TEST_FIXTURE(SerializationTest, SerializeUInt64)
205: {
206:     uint64 u64 = 9223372036854775808ull;
207:     EXPECT_EQ("9223372036854775808", Serialize(u64));
208:     EXPECT_EQ("     9223372036854775808", Serialize(u64, 24));
209:     EXPECT_EQ("9223372036854775808     ", Serialize(u64, -24));
210:     EXPECT_EQ("1000000000000000000000000000000000000000000000000000000000000000", Serialize(u64, 64, 2));
211:     EXPECT_EQ("  1000000000000000000000", Serialize(u64, 24, 8));
212:     EXPECT_EQ("     9223372036854775808", Serialize(u64, 24, 10));
213:     EXPECT_EQ("        8000000000000000", Serialize(u64, 24, 16));
214:     EXPECT_EQ("      0x8000000000000000", Serialize(u64, 24, 16, true));
215:     EXPECT_EQ("0x000000008000000000000000", Serialize(u64, 24, 16, true, true));
216:     EXPECT_EQ("000000008000000000000000", Serialize(u64, 24, 16, false, true));
217: }
218: 
219: TEST_FIXTURE(SerializationTest, SerializeFloat)
220: {
221:     float f = 1.23456789F;
222:     EXPECT_EQ("1.2345679", Serialize(f));
223:     EXPECT_EQ("   1.2345679", Serialize(f, 12));
224:     EXPECT_EQ("1.2345679   ", Serialize(f, -12));
225:     EXPECT_EQ("        1.23", Serialize(f, 12, 2));
226:     EXPECT_EQ("   1.2345679", Serialize(f, 12, 7));
227:     EXPECT_EQ("   1.2345679", Serialize(f, 12, 8));
228: }
229: 
230: TEST_FIXTURE(SerializationTest, SerializeDouble)
231: {
232:     double d = 1.234567890123456;
233:     EXPECT_EQ("1.23456789012346", Serialize(d));
234:     EXPECT_EQ("  1.23456789012346", Serialize(d, 18));
235:     EXPECT_EQ("1.23456789012346  ", Serialize(d, -18));
236:     EXPECT_EQ("           1.23457", Serialize(d, 18, 5));
237:     EXPECT_EQ("         1.2345679", Serialize(d, 18, 7));
238:     EXPECT_EQ("    1.234567890123", Serialize(d, 18, 12));
239: }
240: 
241: TEST_FIXTURE(SerializationTest, SerializeString)
242: {
243:     string s = "hello world";
244:     EXPECT_EQ("hello world", Serialize(s));
245:     EXPECT_EQ("    hello world", Serialize(s, 15));
246:     EXPECT_EQ("hello world    ", Serialize(s, -15));
247:     EXPECT_EQ("  \"hello world\"", Serialize(s, 15, true));
248: }
249: 
250: TEST_FIXTURE(SerializationTest, SerializeConstCharPtr)
251: {
252:     const char* s = "hello world";
253:     EXPECT_EQ("hello world", Serialize(s));
254:     EXPECT_EQ("    hello world", Serialize(s, 15));
255:     EXPECT_EQ("hello world    ", Serialize(s, -15));
256:     EXPECT_EQ("  \"hello world\"", Serialize(s, 15, true));
257: }
258: 
259: TEST_FIXTURE(SerializationTest, SerializeConstVoidPtr)
260: {
261:     const void* pvc = reinterpret_cast<const void*>(0x0123456789ABCDEF);
262:     EXPECT_EQ("0x0123456789ABCDEF", Serialize(pvc));
263:     EXPECT_EQ("  0x0123456789ABCDEF", Serialize(pvc, 20));
264:     EXPECT_EQ("0x0123456789ABCDEF  ", Serialize(pvc, -20));
265: }
266: 
267: TEST_FIXTURE(SerializationTest, SerializeVoidPtr)
268: {
269:     void* pv = reinterpret_cast<void*>(0x0123456789ABCDEF);
270:     EXPECT_EQ("0x0123456789ABCDEF", Serialize(pv));
271:     EXPECT_EQ("  0x0123456789ABCDEF", Serialize(pv, 20));
272:     EXPECT_EQ("0x0123456789ABCDEF  ", Serialize(pv, -20));
273: }
274: 
275: } // suite Baremetal
276: 
277: } // namespace test
278: } // namespace baremetal
```

The tests should speak for themselves.

### Update test project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_SERIALIZATION___STEP_5_UPDATE_TEST_PROJECT_CONFIGURATION}

As we added a new source file, we'll update the test project CMake file.

Update the file `code/libraries/baremetal/test/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/test/CMakeLists.txt
35: set(PROJECT_SOURCES
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/SerializationTest.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/StringTest.cpp
39:     )
40: 
41: set(PROJECT_INCLUDES_PUBLIC )
42: set(PROJECT_INCLUDES_PRIVATE )
```

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_SERIALIZATION___STEP_5_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run all tests, so both stirng test and serialization tests. All tests should succeed.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Info   Starting up (System:201)
[===========] Running 174 tests from 2 fixtures in 1 suite.
[   SUITE   ] Baremetal (2 fixtures)
[  FIXTURE  ] SerializationTest (18 tests)
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeChar
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeIntegerWithPrefix
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeIntegerWithPrefixAndLeadingZeros
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeIntegerWithoutPrefixWithLeadingZeros
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeInt8
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeUInt8
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeInt16
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeUInt16
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeInt32
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeUInt32
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeInt64
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeUInt64
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeFloat
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeDouble
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeString
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeConstCharPtr
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeConstVoidPtr
[ SUCCEEDED ] Baremetal::SerializationTest::SerializeVoidPtr
[  FIXTURE  ] 18 tests from SerializationTest
[  FIXTURE  ] StringTest (156 tests)
[ SUCCEEDED ] Baremetal::StringTest::ConstructDefault
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::ConstructNullPtr
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtrAndSize
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtrAndSizeTooLarge
[ SUCCEEDED ] Baremetal::StringTest::ConstructConstCharPtrAndSizeNpos
[ SUCCEEDED ] Baremetal::StringTest::ConstructCountAndChar
[ SUCCEEDED ] Baremetal::StringTest::ConstructCountNposAndChar
[ SUCCEEDED ] Baremetal::StringTest::ConstructCopy
[ SUCCEEDED ] Baremetal::StringTest::ConstructCopyEmpty
[ SUCCEEDED ] Baremetal::StringTest::ConstructMove
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPos
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPosNpos
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::ConstructStringFromPosWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::ConstCharPtrCastOperator
[ SUCCEEDED ] Baremetal::StringTest::AssignmentOperatorConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AssignmentOperatorCopy
[ SUCCEEDED ] Baremetal::StringTest::AssignmentOperatorMove
[ SUCCEEDED ] Baremetal::StringTest::Begin
[ SUCCEEDED ] Baremetal::StringTest::End
[ SUCCEEDED ] Baremetal::StringTest::BeginConst
[ SUCCEEDED ] Baremetal::StringTest::EndConst
[ SUCCEEDED ] Baremetal::StringTest::Iterate
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::AssignNullPtr
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtrAndSize
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtrAndSizeTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AssignConstCharPtrAndSizeNpos
[ SUCCEEDED ] Baremetal::StringTest::AssignCountAndChar
[ SUCCEEDED ] Baremetal::StringTest::AssignCountNposAndChar
[ SUCCEEDED ] Baremetal::StringTest::AssignString
[ SUCCEEDED ] Baremetal::StringTest::AssignStringEmpty
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPos
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPosNPos
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::AssignStringFromPosWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::At
[ SUCCEEDED ] Baremetal::StringTest::AtConst
[ SUCCEEDED ] Baremetal::StringTest::AtOutsideString
[ SUCCEEDED ] Baremetal::StringTest::AtConstOutsideString
[ SUCCEEDED ] Baremetal::StringTest::AtEmptyString
[ SUCCEEDED ] Baremetal::StringTest::AtConstEmptyString
[ SUCCEEDED ] Baremetal::StringTest::Front
[ SUCCEEDED ] Baremetal::StringTest::FrontConst
[ SUCCEEDED ] Baremetal::StringTest::FrontEmptyString
[ SUCCEEDED ] Baremetal::StringTest::FrontConstEmptyString
[ SUCCEEDED ] Baremetal::StringTest::Back
[ SUCCEEDED ] Baremetal::StringTest::BackConst
[ SUCCEEDED ] Baremetal::StringTest::BackEmptyString
[ SUCCEEDED ] Baremetal::StringTest::BackConstEmptyString
[ SUCCEEDED ] Baremetal::StringTest::IndexOperator
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorConst
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorOutsideString
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorConstOutsideString
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorEmptyString
[ SUCCEEDED ] Baremetal::StringTest::IndexOperatorConstEmptyString
[ SUCCEEDED ] Baremetal::StringTest::Data
[ SUCCEEDED ] Baremetal::StringTest::DataConst
[ SUCCEEDED ] Baremetal::StringTest::CString
[ SUCCEEDED ] Baremetal::StringTest::Reserve
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentChar
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentString
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentStringEmpty
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::AddAssignmentNullPtr
[ SUCCEEDED ] Baremetal::StringTest::AppendCountAndChar
[ SUCCEEDED ] Baremetal::StringTest::AppendCountNposAndChar
[ SUCCEEDED ] Baremetal::StringTest::AppendString
[ SUCCEEDED ] Baremetal::StringTest::AppendStringEmpty
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPos
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPosNpos
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::AppendStringAtPosWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::AppendNullPtr
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtrWithCount
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtrWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::AppendConstCharPtrWithCountNpos
[ SUCCEEDED ] Baremetal::StringTest::Clear
[ SUCCEEDED ] Baremetal::StringTest::FindString
[ SUCCEEDED ] Baremetal::StringTest::FindStringEndOfString
[ SUCCEEDED ] Baremetal::StringTest::FindStringNoMatch
[ SUCCEEDED ] Baremetal::StringTest::FindStringEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPos
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosEndOfString
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosNpos
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindStringAtPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrNoMatch
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrNullPtr
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPos
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosEndOfString
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosNpos
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosTooLargeEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosNullPtr
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCount
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountNoMatch
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountMatchPart
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountTooLarge
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountTooLargeEmpty
[ SUCCEEDED ] Baremetal::StringTest::FindConstCharPtrAtPosWithCountNullPtr
[ SUCCEEDED ] Baremetal::StringTest::FindChar
[ SUCCEEDED ] Baremetal::StringTest::FindCharNoMatch
[ SUCCEEDED ] Baremetal::StringTest::FindCharAtPos
[ SUCCEEDED ] Baremetal::StringTest::FindCharAtPosTooLarge
[ SUCCEEDED ] Baremetal::StringTest::FindCharAtPosNpos
[ SUCCEEDED ] Baremetal::StringTest::StartsWithString
[ SUCCEEDED ] Baremetal::StringTest::StartsWithConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::StartsWithChar
[ SUCCEEDED ] Baremetal::StringTest::EndsWithString
[ SUCCEEDED ] Baremetal::StringTest::EndsWithConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::EndsWithChar
[ SUCCEEDED ] Baremetal::StringTest::ContainsString
[ SUCCEEDED ] Baremetal::StringTest::ContainsConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::ContainsChar
[ SUCCEEDED ] Baremetal::StringTest::SubStr
[ SUCCEEDED ] Baremetal::StringTest::Equals
[ SUCCEEDED ] Baremetal::StringTest::EqualsCaseInsensitive
[ SUCCEEDED ] Baremetal::StringTest::Compare
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountString
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountStringPos
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountStringPosCount
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCountConstCharPtrCount
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosChar
[ SUCCEEDED ] Baremetal::StringTest::ReplacePosCharCount
[ SUCCEEDED ] Baremetal::StringTest::ReplaceSubstringString
[ SUCCEEDED ] Baremetal::StringTest::ReplaceSubstringStringMultiple
[ SUCCEEDED ] Baremetal::StringTest::ReplaceSubstringConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::ReplaceSubstringConstCharPtrMultiple
[ SUCCEEDED ] Baremetal::StringTest::Align
[ SUCCEEDED ] Baremetal::StringTest::EqualityOperator
[ SUCCEEDED ] Baremetal::StringTest::InEqualityOperator
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndString
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndConstCharPtr
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndConstCharPtrEmpty
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndNullPtr
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorConstCharPtrAndString
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorConstCharPtrEmptyAndString
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorNullPtrAndString
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorStringAndChar
[ SUCCEEDED ] Baremetal::StringTest::AddOperatorCharAndString
[  FIXTURE  ] 156 tests from StringTest
[   SUITE   ] 2 fixtures from Baremetal
Success: 174 tests passed.

No failures
[===========] 174 tests from 2 fixtures in 1 suite ran.
Info   Halt (System:122)
```

Next: [19-exceptions](19-exceptions.md)
