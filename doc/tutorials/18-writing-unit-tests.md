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

As said in the previous tutorial, in tutorials `15-string` and `16-serializing-and-formatting` we have been adding quite some code, and added assertion to verify correctness.

We'll now focus on creating macros to create test suites, test fixtures and test, and for creating actual test cases.
When that is done, we'll convert the tests in the two mentioned tutorials to actual class tests.

## Adding macros for creating tests, fixtures and suites - Step 1 {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES__STEP_1}

### TestMacros.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES__STEP_1_TESTMACROSH}

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

### unittest.h {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES__STEP_1_UNITTESTH}

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

### Update project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES__STEP_1_UPDATE_PROJECT_CONFIGURATION}

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

### Application code {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES__STEP_1_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES__STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

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

## Creating test cases - Step 2 {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES__STEP_2}

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

We'll define a set of macros, and we'll need to do some trickery to get it all to work well. So bare with me.

We'll start with simple boolean tests, and equality / inequality of integral types. The rest will follow in the next and final step.

### Checks.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES__STEP_2_CHECKSH}

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

### Checks.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES__STEP_2_CHECKSCPP}

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

### PrintValue.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES__STEP_2_PRINTVALUEH}

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

### AssertMacros.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES__STEP_2_ASSERTMACROSH}

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

### unittest.h {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES__STEP_2_UNITTESTH}

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

### Update project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES__STEP_2_UPDATE_PROJECT_CONFIGURATION}

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

### Application code {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES__STEP_2_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_CREATING_TEST_CASES__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

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

## Test assert macro extension - Step 3 {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION__STEP_3}

We can now perform boolean checks and compare integers, but we would also like to be able to compare pointers and strings.
So we'll extend the macros a bit.

### Checks.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION__STEP_3_CHECKSH}

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

### Checks.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION__STEP_3_CHECKSCPP}

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

### PrintValue.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION__STEP_3_PRINTVALUEH}

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

### PrintValue.cpp {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION__STEP_3_PRINTVALUECPP}

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

### AssertMacros.h {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION__STEP_3_ASSERTMACROSH}

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

### Update project configuration {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION__STEP_3_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/unittest/CMakeLists.txt`

```cmake
File: code/libraries/unittest/CMakeLists.txt
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

### Application code {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION__STEP_3_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_18_WRITING_UNIT_TESTS_TEST_ASSERT_MACRO_EXTENSION__STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

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

## Writing class tests for string - Step 4 {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_STRING__STEP_4}

## Writing class tests for serialization - Step 5 {#TUTORIAL_18_WRITING_UNIT_TESTS_WRITING_CLASS_TESTS_FOR_SERIALIZATION__STEP_5}


Next: [19-exceptions](19-exceptions.md)
