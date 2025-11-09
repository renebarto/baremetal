# Tutorial 24: Unit test macros {#TUTORIAL_24_UNIT_TEST_MACROS}

@tableofcontents

## New tutorial setup {#TUTORIAL_24_UNIT_TEST_MACROS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/24-unit-test-macros`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_24_UNIT_TEST_MACROS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-24.a`
- a library `output/Debug/lib/device-24.a`
- a library `output/Debug/lib/stdlib-24.a`
- a library `output/Debug/lib/unittest-24.a`
- an application `output/Debug/bin/24-unit-test-macros.elf`
- an image in `deploy/Debug/24-unit-test-macros-image`

## Creating actual unit tests {#TUTORIAL_24_UNIT_TEST_MACROS_CREATING_ACTUAL_UNIT_TESTS}

As said in the previous tutorial, the tests we wrote so far are quite verbose, and it would be nice to make them more compact and easier to use.

We'll now focus on creating macros to create test suites, test fixtures and test, and for creating actual test cases.
Meanwhile we'll convert the tests we wrote in the last tutorial using these macros, so you can see how things gradually improve.

## Adding macros for creating tests, fixtures and suites - Step 1 {#TUTORIAL_24_UNIT_TEST_MACROS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1}

### TestMacros.h {#TUTORIAL_24_UNIT_TEST_MACROS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_TESTMACROSH}

The most cumbersome about the current implementation is that we need to be very verbose when writing tests, test fixture and test suites.
We can easily solve this by creating some macros.

Create the file `code/libraries/unittest/include/unittest/TestMacros.h`

```cpp
File: code/libraries/unittest/include/unittest/TestMacros.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
47: /// Register the test named TestName with the registry Registry. This will declare a test class Test[TestName] and instantiate it as
48: /// test[TestName]Instance. It will also instantiate a TestRegistrar which will register the test instance with the test registry. It will then define
49: /// the RunImpl() method of the Test[TestName] class, which is formed by the following code between curly brackets
50: #define TEST_EX(TestName, Registry)                                                                                                                  \
51:     class Test##TestName : public unittest::Test                                                                                                     \
52:     {                                                                                                                                                \
53:     private:                                                                                                                                         \
54:         virtual void RunImpl() const override;                                                                                                       \
55:     } test##TestName##Instance;                                                                                                                      \
56:                                                                                                                                                      \
57:     static unittest::TestRegistrar registrar##TestName(Registry, &test##TestName##Instance,                                                          \
58:                                                        unittest::TestDetails(baremetal::String(#TestName), baremetal::String(""),                    \
59:                                                                              baremetal::String(GetSuiteName()), baremetal::String(__FILE__),         \
60:                                                                              __LINE__));                                                             \
61:                                                                                                                                                      \
62:     void Test##TestName::RunImpl() const
63:
64: /// @brief Register test
65: ///
66: /// Register the test named TestName with the singleton test registry. This will use the TEST_EX macro to do the actual test registration
67: #define TEST(TestName) TEST_EX(TestName, unittest::TestRegistry::GetTestRegistry())
68:
69: /// @brief Register test inside test fixture
70: ///
71: /// Register the test inside a test fixture named FixtureClass for the test named TestName with the registry Registry.
72: /// This will declare a class [FixtureClass][TestName]Helper, which calls the Setup() method in the constructor, and the TearDown() method in the
73: /// destructor. It also declares a test class Test[FixtureClass][TestName] and instantiate it as test[FixtureClass][TestName]Instance. It will also
74: /// instantiate a TestRegistrar which will register the test  with the test registry. The RunImpl() method of the Test[FixtureClass][TestName] is
75: /// implemented as creating an instance of the test fixture, and invoking its RunImpl() method. It will then define the RunImpl() method of the
76: /// Test[FixtureClass][TestName] class, which is formed by the following code between curly brackets
77: #define TEST_FIXTURE_EX(FixtureClass, TestName, Registry)                                                                                            \
78:     class FixtureClass##TestName##Helper : public FixtureClass                                                                                       \
79:     {                                                                                                                                                \
80:     public:                                                                                                                                          \
81:         FixtureClass##TestName##Helper(const FixtureClass##TestName##Helper&) = delete;                                                              \
82:         explicit FixtureClass##TestName##Helper(unittest::TestDetails const& details)                                                                \
83:             : m_details{details}                                                                                                                     \
84:         {                                                                                                                                            \
85:             SetUp();                                                                                                                                 \
86:         }                                                                                                                                            \
87:         virtual ~FixtureClass##TestName##Helper()                                                                                                    \
88:         {                                                                                                                                            \
89:             TearDown();                                                                                                                              \
90:         }                                                                                                                                            \
91:         FixtureClass##TestName##Helper& operator=(const FixtureClass##TestName##Helper&) = delete;                                                   \
92:         void RunImpl() const;                                                                                                                        \
93:         unittest::TestDetails const& m_details;                                                                                                      \
94:     };                                                                                                                                               \
95:                                                                                                                                                      \
96:     class Test##FixtureClass##TestName : public unittest::Test                                                                                       \
97:     {                                                                                                                                                \
98:     private:                                                                                                                                         \
99:         void RunImpl() const override;                                                                                                               \
100:     } test##FixtureClass##TestName##Instance;                                                                                                        \
101:                                                                                                                                                      \
102:     unittest::TestRegistrar registrar##FixtureClass##TestName(Registry, &test##FixtureClass##TestName##Instance,                                     \
103:                                                               TestDetails(baremetal::String(#TestName), baremetal::String(#FixtureClass),            \
104:                                                                           baremetal::String(GetSuiteName()), baremetal::String(__FILE__),            \
105:                                                                           __LINE__));                                                                \
106:                                                                                                                                                      \
107:     void Test##FixtureClass##TestName::RunImpl() const                                                                                               \
108:     {                                                                                                                                                \
109:         FixtureClass##TestName##Helper fixtureHelper(*CurrentTest::Details());                                                                       \
110:         fixtureHelper.RunImpl();                                                                                                                     \
111:     }                                                                                                                                                \
112:     void FixtureClass##TestName##Helper::RunImpl() const
113:
114: /// @brief Register test inside test fixture
115: ///
116: /// Register the test named TestName inside a test fixture class named Fixture with the singleton test registry. This will use the TEST_FIXTURE_EX
117: /// macro to do the actual test registration
118: #define TEST_FIXTURE(FixtureClass, TestName) TEST_FIXTURE_EX(FixtureClass, TestName, unittest::TestRegistry::GetTestRegistry())
119:
120: /// @brief Create test suite
121: ///
122: /// Creates a test suite named SuiteName. This simply creates a namespace inside which tests and test fixtures are placed
123: #define TEST_SUITE(SuiteName)                                                                                                                        \
124:     namespace Suite##SuiteName                                                                                                                       \
125:     {                                                                                                                                                \
126:         inline char const* GetSuiteName()                                                                                                            \
127:         {                                                                                                                                            \
128:             return baremetal::String(#SuiteName);                                                                                                    \
129:         }                                                                                                                                            \
130:     }                                                                                                                                                \
131:     namespace Suite##SuiteName
```

Below, when we show `[TEXT]` this means the replacement of the macro parameter.

- Line 45-62: We define the macro `TEST_EX`. This is used by macro `TEST`.
The parameters are the name of the test `TestName` and the reference to the singleton `TestRegistry` instance `Registry`.
- It declares the class `Test[TestName]`, which inherits from `Test`, and defines an instance named `test[TestName]Instance`
- It then defines a `TestRegistrar` instance named `registrar[TestName]` which registers the instance `test[TestName]Instance`.
- It then starts the definition of the `RunImpl()` which is expected to be followed by the actual implementation of the test.
Compare this to the application code we wrote before:

```cpp
File: code/applications/demo/src/main.cpp
178: class MyTest : public Test
179: {
180: public:
181:     void RunImpl() const override;
182: } myTest;
183:
184: TestRegistrar registrarMyTest(TestRegistry::GetTestRegistry(), &myTest, TestDetails("MyTest", "", "", __FILE__, __LINE__));
185:
186: void MyTest::RunImpl() const
187: {
188:     LOG_DEBUG("Running test");
189:     CurrentTest::Results()->OnTestFailure(*CurrentTest::Details(), "Failure message example");
190: }
```

- Here:
    - `MyTest` is replaced by `Test##TestName`
    - `myTest` is replaced by `test##TestName##Instance`
    - `registrarMyTest` is replaced by `registrar##TestName`

- Line 64-67: We define the macro `TEST` which uses `TEST_EX` to register the test named `TestName`.
The only parameter for `TEST` is `TestName`, the name of the test.
The test registry is the singleton `TestRegistry` instance.
The `TEST` macro is intended for tests that do not belong to a test fixture.
The way this macro can be used is as follows:

```cpp
TEST(MyTest)
{
    // Test implementation
}
```

which will then translate to

```cpp
class TestMyTest : public unittest::Test
{
private:
    virtual void RunImpl() const override;
} testMyTestInstance;

static unittest::TestRegistrar registrarMyTest(unittest::TestRegistry::GetTestRegistry(),
                                               &testMyTestInstance,
                                               unittest::TestDetails(baremetal::String(MyTest),
                                                                     baremetal::String(""),
                                                                     baremetal::String(GetSuiteName()),
                                                                     baremetal::String(__FILE__),
                                                                     __LINE__));

void TestMyTest::RunImpl() const
{
    // Test implementation
}
```

- Line 69-112: We define the macro `TEST_FIXTURE_EX`. This is used by macro `TEST_FIXTURE`.
The parameters are the name of the fixture class `FixtureClass`, the name of the test `TestName` and the reference to the singleton `TestRegistry` instance `Registry`.
- It declares the class `[FixtureClass][TestName]Helper`, which inherits from the `FixtureClass`, the class we defined for the fixture
- It then declares a class `Test[FixtureClass][TestName]`, which inherits from `Test`, and defines an instance named `test[FixtureClass][TestName]Instance`
- Next it defines a `TestRegistrar` instance named `registrar[FixtureClass][TestName]` which registers the instance `test[FixtureClass][TestName]Instance`
- Next it defines the implement of `RunImpl()` for the class `Test[FixtureClass][TestName]`.
This creates and instance of class `[FixtureClass][TestName]Helper`.
Through the constructor, the `SetUp()` method is called
- Finally, it then starts the definition of the `RunImpl()` for the class `[FixtureClass][TestName]Helper` which is expected to be followed by the actual implementation of the test
- When the `RunImpl()` method for the class `Test[FixtureClass][TestName]` exits, the desctructor for `[FixtureClass][TestName]Helper` is called, which runs the `TearDown()` method

Compare this to the application code we wrote before:

```cpp
File: code/applications/demo/src/main.cpp
20: class FixtureMyTest1 : public TestFixture
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
33: class FixtureMyTest1Helper : public FixtureMyTest1
34: {
35: public:
36:     FixtureMyTest1Helper(const FixtureMyTest1Helper&) = delete;
37:     explicit FixtureMyTest1Helper(const TestDetails& details)
38:         : m_details{details}
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
51:     LOG_DEBUG(m_details.QualifiedTestName() + "MyTestHelper 1");
52: }
53:
54: class MyTest1 : public Test
55: {
56:     void RunImpl() const override;
57: } myTest1;
58:
59: TestRegistrar registrarFixtureMyTest1(TestRegistry::GetTestRegistry(), &myTest1,
60:                                       TestDetails("MyTest1", "FixtureMyTest1", GetSuiteName(), __FILE__, __LINE__));
61:
62: void MyTest1::RunImpl() const
63: {
64:     LOG_DEBUG("Test 1");
65:     FixtureMyTest1Helper fixtureHelper(*CurrentTest::Details());
66:     fixtureHelper.RunImpl();
67: }
```

- Here
  - `Fixture` is the `FixtureClass` we pass into the macro `TEST_FIXTURE_EX`
  - `MyTest1` is the `TestName` we pass into the macro `TEST_FIXTURE_EX`
  - `FixtureMyTest1Helper` is then replaced by `FixtureMyTest1Helper`
  - `MyTest1` is `TestFixtureMyTest1`
  - `myTest1` is `testFixtureMyTest1Instance`
  - `registrarFixtureMyTest1` is `registrarFixtureMyTest1`

- Line 114-118: We define the macro `TEST_FIXTURE` which uses `TEST_FIXTURE_EX` to register the test named `TestName` for test fixture class `FixtureClass`.
The parameters are the name of the fixture class `FixtureClass`, the name of the test `TestName`.
The test registry is the singleton `TestRegistry` instance.
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

which will then translate to

```cpp
class MyFixtureMyTestHelper : public FixtureClass
{
public:
    MyFixtureMyTestHelper(const MyFixtureMyTestHelper&) = delete;
    explicit MyFixtureMyTestHelper(unittest::TestDetails const& details)
        : m_details{details}
    {
        SetUp();
    }
    virtual ~MyFixtureMyTestHelper()
    {
        TearDown();
    }
    MyFixtureMyTestHelper& operator=(const MyFixtureMyTestHelper&) = delete;
    void RunImpl() const;
    unittest::TestDetails const& m_details;
};

class TestMyFixtureTestName : public unittest::Test
{
private:
    void RunImpl() const override;
} testMyFixtureMyTestInstance;

unittest::TestRegistrar registrarMyFixtureTestName(Registry,
                                                     &testMyFixtureMyTestInstance,
                                                     TestDetails(baremetal::String("MyTest"),
                                                                 baremetal::String("MyFixture"),
                                                                 baremetal::String(GetSuiteName()),
                                                                 baremetal::String(__FILE__),
                                                                 __LINE__));

void TestMyFixtureTestName::RunImpl() const
{
    MyFixtureMyTestHelper fixtureHelper(*CurrentTest::Details());
    fixtureHelper.RunImpl();
}
void MyFixtureMyTestHelper::RunImpl() const
{
    // Test implementation
}
```

Of course there can be multiple tests in a fixture, so the `TEST_FIXTURE` macro can be repeated.
Notice that we use the name of the test, the test fixture, and for the suite name we use `GetSuiteName()`.

- Line 120-131: We define the macro `TEST_SUITE` which defined a test suite.
It creates a namespace for the test suite named `SuiteName`, as wel the `GetSuiteName()` function inside the namespace.
The only parameter is the name of the test suite `SuiteName`.
The `TEST_SUITE` macro is intended for create a test suite around tests and test fixtures, which effectively wraps a test or test fixture inside a namespace, with its specific version of `GetSuiteName()`.
Compare the definition of this macro to the application code we wrote before:

```cpp
File: code/applications/demo/src/main.cpp
File: d:\Projects\RaspberryPi\baremetal.github\code\applications\demo\src\main.cpp
13: namespace Suite1 {
14: 
15: inline char const* GetSuiteName()
16: {
17:     return "Suite1";
18: }
...
69: } // namespace Suite1
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

which will then translate to

```cpp
namespace SuiteMySuite
{
    inline char const* GetSuiteName()
    {
        return baremetal::String("MySuite"");
    }
}
namespace Suite##SuiteName
{
...
}
```

### unittest.h {#TUTORIAL_24_UNIT_TEST_MACROS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_UNITTESTH}

We added a header file, so we'll update the `unittest.h` header

Update the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
...
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
56: #include "unittest/TestMacros.h"
57: #include "unittest/TestRegistry.h"
58: #include "unittest/TestResults.h"
59: #include "unittest/TestRunner.h"
60: #include "unittest/TestSuiteInfo.h"
```

### Application code {#TUTORIAL_24_UNIT_TEST_MACROS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_APPLICATION_CODE}

Now that we have added a test runner and a test report, we can use these to make running the tests even simpler.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include "baremetal/Assert.h"
2: #include "baremetal/Console.h"
3: #include "baremetal/Logger.h"
4: #include "baremetal/System.h"
5: #include "baremetal/Timer.h"
6:
7: #include "unittest/unittest.h"
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

- Line 13: We replace the namespace Suite1 and its `GetSuiteName()` function by the macro `TEST_SUITE`
- Line 30: We replace the declaration and definition of classes `FixtureMyTest1Helper` and `MyTest1` by the macro `TEST_FIXTURE`.
Note that we still need to defined the class `FixtureMyTest1` as it defines common behaviour for all tests in the test fixture
- Line 37: We replace the namespace Suite2 and its `GetSuiteName()` function by the macro `TEST_SUITE`
- Line 54: We replace the declaration and definition of classes `FixtureMyTest2Helper` and `MyTest2` by the macro `TEST_FIXTURE`.
Note that we still need to defined the class `FixtureMyTest2` as it defines common behaviour for all tests in the test fixture
- Line 75: We replace the declaration and definition of classes `FixtureMyTest3Helper` and `MyTest3` by the macro `TEST_FIXTURE`.
As we did not use `TEST_SUITE`, this fixture will be in the global namespace.
Note that we still need to defined the class `FixtureMyTest3` as it defines common behaviour for all tests in the test fixture
- Line 80: We replace the declaration and definition of class `MyTest` by the macro `TEST`.

Note that we do not enable debug level tracing anymore.

### Configuring, building and debugging {#TUTORIAL_24_UNIT_TEST_MACROS_ADDING_MACROS_FOR_CREATING_TESTS_FIXTURES_AND_SUITES___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. As we have one failure in `Test3`, and two in `Test4`, we expect to see two failing tests, and three failures in total.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 4 tests from 4 fixtures in 3 suites.
[   SUITE   ] Suite1 (1 fixture)
[  FIXTURE  ] FixtureMyTest1 (1 test)
[ SUCCEEDED ] Suite1::FixtureMyTest1::MyTest1
[  FIXTURE  ] 1 test from FixtureMyTest1
[   SUITE   ] 1 fixture from Suite1
[   SUITE   ] Suite2 (1 fixture)
[  FIXTURE  ] FixtureMyTest2 (1 test)
[ SUCCEEDED ] Suite2::FixtureMyTest2::MyTest2
[  FIXTURE  ] 1 test from FixtureMyTest2
[   SUITE   ] 1 fixture from Suite2
[   SUITE   ] DefaultSuite (2 fixtures)
[  FIXTURE  ] FixtureMyTest3 (1 test)
[ SUCCEEDED ] DefaultSuite::FixtureMyTest3::MyTest3
[  FIXTURE  ] 1 test from FixtureMyTest3
[  FIXTURE  ] DefaultFixture (1 test)
[  FAILED   ] DefaultSuite::DefaultFixture::MyTest
[  FIXTURE  ] 1 test from DefaultFixture
[   SUITE   ] 2 fixtures from DefaultSuite
FAILURE: 1 out of 4 tests failed (1 failure).

Failures:
D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:80 : Failure in DefaultSuite::DefaultFixture::MyTest: Failure

[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   0.00:00:00.080 Wait 5 seconds (main:93)
Press r to reboot, h to halt
```

or in color:

<img src="images/unittest-output.png"  alt="Unit test output in color" width="1000"/>

## Creating test cases - Step 2 {#TUTORIAL_24_UNIT_TEST_MACROS_CREATING_TEST_CASES___STEP_2}

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

Let's create macros to perform these checks, and and a mechanism to trace back the reason for a failure, e.g. when we expect to have a certain value, what the expected and actual values were.

We'll define a set of macros, and we'll need to do some trickery to get it all to work well. So bare with me.

We'll start with simple boolean tests, and equality / inequality of integral types. The rest will follow in the next and final step.

**Note**: There is a difference between `EXPECT_` and `ASSERT_` macros in GoogleTest, however we cannot currently make that distinction. So in both cases, the tests will continue on failure.

### Checks.h {#TUTORIAL_24_UNIT_TEST_MACROS_CREATING_TEST_CASES___STEP_2_CHECKSH}

We'll start with some utility functions and classes.

Create the file `code/libraries/unittest/include/unittest/Checks.h`

```cpp
File: code/libraries/unittest/include/unittest/Checks.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "baremetal/String.h"
43: 
44: #include "unittest/PrintValue.h"
45: 
46: /// @file
47: /// Assertion checks
48: 
49: namespace unittest {
50: 
51: class TestResults;
52: class TestDetails;
53: 
54: /// <summary>
55: /// Assertion result
56: ///
57: /// Contains the result of an assertion, flags whether it failed, and what the message was
58: /// </summary>
59: struct AssertionResult
60: {
61:     /// <summary>
62:     /// Constructor
63:     /// </summary>
64:     /// <param name="failed">If true, the assertion failed, if false the assertion was successful</param>
65:     /// <param name="message">Message for the assertion</param>
66:     AssertionResult(bool failed, const baremetal::String& message)
67:         : failed(failed)
68:         , message(message)
69:     {
70:     }
71:     /// @brief If true, the assertion failed, if false the assertion was successful
72:     const bool failed;
73:     /// @brief Message for the assertion
74:     const baremetal::String message;
75:     /// @brief bool case operator.
76:     /// @return true, if the assertion failed, false if the assertion was successful
77:     operator bool() const
78:     {
79:         return failed;
80:     }
81: };
82: 
83: extern AssertionResult AssertionSuccess();
84: extern AssertionResult GenericFailure(const baremetal::String& message);
85: extern AssertionResult BooleanFailure(const baremetal::String& valueExpression, const baremetal::String& expectedValue,
86:                                       const baremetal::String& actualValue);
87: extern AssertionResult EqFailure(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
88:                                  const baremetal::String& expectedValue, const baremetal::String& actualValue);
89: extern AssertionResult InEqFailure(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
90:                                    const baremetal::String& expectedValue, const baremetal::String& actualValue);
91: 
92: /// <summary>
93: /// Check whether argument is evaluated as true
94: /// </summary>
95: /// <typeparam name="Value">Type of argument</typeparam>
96: /// <param name="value">Argument</param>
97: /// <returns>Returns true if the argument can be evaluated as true, false otherwise</returns>
98: template <typename Value>
99: bool CheckTrue(const Value value)
100: {
101:     return !!value;
102: }
103: 
104: /// <summary>
105: /// Check whether argument is evaluated as false
106: /// </summary>
107: /// <typeparam name="Value">Type of argument</typeparam>
108: /// <param name="value">Argument</param>
109: /// <returns>Returns true if the argument can be evaluated as false, false otherwise</returns>
110: template <typename Value>
111: bool CheckFalse(const Value value)
112: {
113:     return !value;
114: }
115: 
116: /// <summary>
117: /// Utility class to convert a value to a string for comparison
118: /// </summary>
119: /// <typeparam name="ToPrint">Type of value to be converted to string</typeparam>
120: template <typename ToPrint>
121: class FormatForComparison
122: {
123: public:
124:     /// <summary>
125:     /// Convert value to a string
126:     /// </summary>
127:     /// <param name="value">Value to be converted to string</param>
128:     /// <returns>Resulting string</returns>
129:     static baremetal::String Format(const ToPrint& value)
130:     {
131:         return PrintToString(value);
132:     }
133: };
134: 
135: /// <summary>
136: /// Format a value for a failure message
137: /// </summary>
138: /// <typeparam name="T1">Type of value to be formatted</typeparam>
139: /// <param name="value">Value to be formatted</param>
140: /// <returns>Resulting string</returns>
141: template <typename T1>
142: baremetal::String FormatForComparisonFailureMessage(const T1& value)
143: {
144:     return FormatForComparison<T1>::Format(value);
145: }
146: 
147: /// <summary>
148: /// Evaluate whether a value can be evaluated as true, generate a success object if successful, otherwise a failure object
149: /// </summary>
150: /// <typeparam name="Value">Type of value to be evaluated</typeparam>
151: /// <param name="valueName">String representation of value for failure message</param>
152: /// <param name="value">Value to be evaluated</param>
153: /// <returns>Result object</returns>
154: template <typename Value>
155: AssertionResult CheckTrue(const baremetal::String& valueName, const Value& value)
156: {
157:     if (!CheckTrue(value))
158:     {
159:         return BooleanFailure(valueName, baremetal::String("true"), PrintToString(value));
160:     }
161:     return AssertionSuccess();
162: }
163: 
164: /// <summary>
165: /// Evaluate whether a value can be evaluated as false, generate a success object if successful, otherwise a failure object
166: /// </summary>
167: /// <typeparam name="Value">Type of value to be evaluated</typeparam>
168: /// <param name="valueName">String representation of value for failure message</param>
169: /// <param name="value">Value to be evaluated</param>
170: /// <returns>Result object</returns>
171: template <typename Value>
172: AssertionResult CheckFalse(const baremetal::String& valueName, const Value& value)
173: {
174:     if (!CheckFalse(value))
175:     {
176:         return BooleanFailure(valueName, baremetal::String("false"), PrintToString(value));
177:     }
178:     return AssertionSuccess();
179: }
180: 
181: /// <summary>
182: /// Compare an expected and actual value
183: /// </summary>
184: /// <typeparam name="Expected">Type of expected value</typeparam>
185: /// <typeparam name="Actual">Type of actual value</typeparam>
186: /// <param name="expected">Expected value</param>
187: /// <param name="actual">Actual value</param>
188: /// <returns>True if the values are considered equal, false otherwise</returns>
189: template <typename Expected, typename Actual>
190: bool AreEqual(const Expected& expected, const Actual& actual)
191: {
192:     return (expected == actual);
193: }
194: 
195: /// <summary>
196: /// Evaluate whether an expected value is equal to an actual value, generate a success object if successful, otherwise a failure object
197: /// </summary>
198: /// <typeparam name="Expected">Type of the expected value</typeparam>
199: /// <typeparam name="Actual">Type of the actual value</typeparam>
200: /// <param name="expectedExpression">String representation of the expected value</param>
201: /// <param name="actualExpression">String representation of the actual value</param>
202: /// <param name="expected">Expected value</param>
203: /// <param name="actual">Actual value</param>
204: /// <returns>Result object</returns>
205: template <typename Expected, typename Actual>
206: AssertionResult CheckEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, const Expected& expected,
207:                                    const Actual& actual)
208: {
209:     if (!AreEqual(expected, actual))
210:     {
211:         return EqFailure(expectedExpression, actualExpression, FormatForComparisonFailureMessage(expected),
212:                          FormatForComparisonFailureMessage(actual));
213:     }
214:     return AssertionSuccess();
215: }
216: 
217: /// <summary>
218: /// Evaluate whether an expected value is not equal to an actual value, generate a success object if successful, otherwise a failure object
219: /// </summary>
220: /// <typeparam name="Expected">Type of the expected value</typeparam>
221: /// <typeparam name="Actual">Type of the actual value</typeparam>
222: /// <param name="expectedExpression">String representation of the not expected value</param>
223: /// <param name="actualExpression">String representation of the actual value</param>
224: /// <param name="expected">Expected value</param>
225: /// <param name="actual">Actual value</param>
226: /// <returns>Result object</returns>
227: template <typename Expected, typename Actual>
228: AssertionResult CheckNotEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
229:                                       const Expected& expected, const Actual& actual)
230: {
231:     if (AreEqual(expected, actual))
232:     {
233:         return InEqFailure(expectedExpression, actualExpression, FormatForComparisonFailureMessage(expected),
234:                            FormatForComparisonFailureMessage(actual));
235:     }
236:     return AssertionSuccess();
237: }
238: 
239: /// <summary>
240: /// Helper class for {ASSERT|EXPECT}_EQ/NE
241: ///
242: /// Forms generalized mechanism for calling polymorphic check functions.
243: /// The template argument lhs_is_null_literal is true iff the first argument to ASSERT_EQ()
244: /// is a null pointer literal.  The following default implementation is
245: /// for lhs_is_null_literal being false.
246: /// </summary>
247: template <bool lhs_is_null_literal>
248: class EqHelper
249: {
250: public:
251:     /// <summary>
252:     /// Evaluate whether an expected value is equal to an actual value, generate a success object if successful, otherwise a failure object
253:     /// </summary>
254:     /// <typeparam name="Expected">Type of the expected value</typeparam>
255:     /// <typeparam name="Actual">Type of the actual value</typeparam>
256:     /// <param name="expectedExpression">String representation of the expected value</param>
257:     /// <param name="actualExpression">String representation of the actual value</param>
258:     /// <param name="expected">Expected value</param>
259:     /// <param name="actual">Actual value</param>
260:     /// <returns>Result object</returns>
261:     template <typename Expected, typename Actual>
262:     static AssertionResult CheckEqual(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
263:                                       const Expected& expected, const Actual& actual)
264:     {
265:         return CheckEqualInternal(expectedExpression, actualExpression, expected, actual);
266:     }
267: 
268:     /// <summary>
269:     /// Evaluate whether an expected value is not equal to an actual value, generate a success object if successful, otherwise a failure object
270:     /// </summary>
271:     /// <typeparam name="Expected">Type of the expected value</typeparam>
272:     /// <typeparam name="Actual">Type of the actual value</typeparam>
273:     /// <param name="expectedExpression">String representation of the not expected value</param>
274:     /// <param name="actualExpression">String representation of the actual value</param>
275:     /// <param name="expected">Expected value</param>
276:     /// <param name="actual">Actual value</param>
277:     /// <returns>Result object</returns>
278:     template <typename Expected, typename Actual>
279:     static AssertionResult CheckNotEqual(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
280:                                          const Expected& expected, const Actual& actual)
281:     {
282:         return CheckNotEqualInternal(expectedExpression, actualExpression, expected, actual);
283:     }
284: };
285: 
286: } // namespace unittest
```

- Line 54-81: We define a struct `AssertionResult` which holds the status for a single assertion.
  - Line 61-70: We define the constructor
  - Line 71-72: The member variable `failed` flags whether the assertion failed
  - Line 73-74: The member vairbale `message` holds the failure message, if any
  - Line 75-80: The `bool()` operator returns the value of `failed`
- Line 83: We declare a function `AssertionSuccess()` to signify a successful assert
- Line 84: We declare a function `GenericFailure()` to signify a generic assertion failure (e.g. `FAIL`)
- Line 85-86: We declare a function `BooleanFailure()` to signify a assertion failure on a boolean expression (e.g. `ASSERT_FALSE`, `EXPECT_TRUE`)
- Line 87-88: We declare a function `EqFailure()` to signify a assertion failure on a equality expression (e.g. `EXPECT_EQ`)
- Line 89-90: We declare a function `InEqFailure()` to signify a assertion failure on a inequality expression (e.g. `EXPECT_NE`)
- Line 92-102: We declare a template function `CheckTrue()` to convert a value to a boolean, returning true if the value is true, or not equal to 0
- Line 104-114: We define a template function `CheckFalse()` to convert a value to a boolean, returning true if the value is false, or equal to 0
- Line 116-133: We define a template class `FormatForComparison` which has a single method `Format`, which prints the value passed to a string.
The printing is done using functionality in the header `PrintValue.h` which we'll get to in a minute
- Line 135-145: We define a template function `FormatForComparisonFailureMessage()` which uses the `FormatForComparison` class to print the value passed
- Line 147-162: We define a template function `CheckTrue()` which takes a value and its stringified version, and uses the `CheckTrue()` defined before to check whether the value is seen as true.
If so, `AssertionSuccess()` is returned, otherwise, `BooleanFailure()` is returned
- Line 164-179: We define a template function `CheckFalse()` which takes a value and its stringified version, and uses the `CheckFalse()` defined before to check whether the value is seen as false.
If so, `AssertionSuccess()` is returned, otherwise, `BooleanFailure()` is returned
- Line 181-193: We define a template function `AreEqual()` to compare two values `expected` and `actual`, which can have different types.
The function uses the equality operator to determine if the two values are equal
- Line 195-215: We define a template function `CheckEqualInternal()`, which uses `AreEqual()` to compare two values `expected` and `actual`. Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 217-237: We define a template function `CheckNotEqualInternal()`, which uses `AreEqual()` to compare two values `expected` and `actual`. Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered inequal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 239-284: We declare a class `EqHelper` which has two methods `CheckEqual()` (which uses `CheckEqualInternal()`) and `CheckNotEqual()` (which uses `CheckNotEqualInternal()`)

### Checks.cpp {#TUTORIAL_24_UNIT_TEST_MACROS_CREATING_TEST_CASES___STEP_2_CHECKSCPP}

Let's implement the functions for the checks.

Create the file `code/libraries/unittest/src/Checks.cpp`

```cpp
File: code/libraries/unittest/src/Checks.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "baremetal/Format.h"
43: #include "stdlib/Util.h"
44: 
45: /// @file
46: /// Assertion checks implementation
47: 
48: using namespace baremetal;
49: 
50: namespace unittest {
51: 
52: /// <summary>
53: /// Create a success object
54: /// </summary>
55: /// <returns>Result object</returns>
56: AssertionResult AssertionSuccess()
57: {
58:     return AssertionResult(false, String());
59: }
60: 
61: /// <summary>
62: /// Create a generic failure object with the provided message
63: /// </summary>
64: /// <param name="message">Message to be included</param>
65: /// <returns>Result object</returns>
66: AssertionResult GenericFailure(const baremetal::String& message)
67: {
68:     return AssertionResult(true, message);
69: }
70: 
71: /// <summary>
72: /// Create a boolean failure object
73: /// </summary>
74: /// <param name="valueExpression">String representation of the actual value</param>
75: /// <param name="expectedValue">Expected value</param>
76: /// <param name="actualValue">Actual value</param>
77: /// <returns>Result object</returns>
78: AssertionResult BooleanFailure(const baremetal::String& valueExpression, const baremetal::String& expectedValue, const baremetal::String& actualValue)
79: {
80:     String result = Format("Value of: %s", valueExpression.c_str());
81:     if (actualValue != valueExpression)
82:     {
83:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
84:     }
85: 
86:     result.append(Format("\n  Expected: %s\n", expectedValue.c_str()));
87: 
88:     return AssertionResult(true, result);
89: }
90: 
91: /// <summary>
92: /// Create a equality comparison failure object
93: /// </summary>
94: /// <param name="expectedExpression">String representation of the expected value</param>
95: /// <param name="actualExpression">String representation of the actual value</param>
96: /// <param name="expectedValue">Expected value</param>
97: /// <param name="actualValue">Actual value</param>
98: /// <returns>Result object</returns>
99: AssertionResult EqFailure(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
100:                           const baremetal::String& expectedValue, const baremetal::String& actualValue)
101: {
102:     String result = Format("Value of: %s", actualExpression.c_str());
103:     if (actualValue != actualExpression)
104:     {
105:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
106:     }
107: 
108:     result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
109:     if (expectedValue != expectedExpression)
110:     {
111:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
112:     }
113:     result.append("\n");
114: 
115:     return AssertionResult(true, result);
116: }
117: 
118: /// <summary>
119: /// Create a inequality comparison failure object
120: /// </summary>
121: /// <param name="expectedExpression">String representation of the not expected value</param>
122: /// <param name="actualExpression">String representation of the actual value</param>
123: /// <param name="expectedValue">Expected value</param>
124: /// <param name="actualValue">Actual value</param>
125: /// <returns>Result object</returns>
126: AssertionResult InEqFailure(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
127:                             const baremetal::String& expectedValue, const baremetal::String& actualValue)
128: {
129:     String result = Format("Value of: %s", actualExpression.c_str());
130:     if (actualValue != actualExpression)
131:     {
132:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
133:     }
134: 
135:     result.append(Format("\n  Expected not equal to: %s", expectedExpression.c_str()));
136:     if (expectedValue != expectedExpression)
137:     {
138:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
139:     }
140:     result.append("\n");
141: 
142:     return AssertionResult(true, result);
143: }
144: 
145: } // namespace unittest
```

- Line 52-59: We implement the function `AssertionSuccess()`.
This will return an assertion result flagging no failure
- Line 61-69: We implement the function `GenericFailure()`.
This will return an assertion result flagging a failure, with a string explaining the reason
- Line 71-89: We implement the function `BooleanFailure()`.
This will return an assertion result flagging a failure, with a string explaining that the actual value does not match the expected value
- Line 91-116: We implement the function `EqFailure()`.
This will return an assertion result flagging a failure, with a string explaining that the actual value is not equal to the expected value
- Line 118-143: We implement the function `InEqFailure()`.
This will return an assertion result flagging a failure, with a string explaining that the actual value is equal to the expected value

### PrintValue.h {#TUTORIAL_24_UNIT_TEST_MACROS_CREATING_TEST_CASES___STEP_2_PRINTVALUEH}

The `PrintValue` header contains a lot of template magic to print a value of any type. It is simplified for now, but will be extended in the next step.
I'll try to explain as much as possible. Part of this code is based on how Google Test prints values.

Create the file `code/libraries/unittest/include/unittest/PrintValue.h`

```cpp
File: code/libraries/unittest/include/unittest/PrintValue.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include "baremetal/Serialization.h"
43: #include "baremetal/String.h"
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
55: void PrintTo(const T& value, baremetal::String& s)
56: {
57:     s = baremetal::Serialize(value);
58: }
59: 
60: /// <summary>
61: /// Print a boolean value to string
62: /// </summary>
63: /// <param name="x">Value to print</param>
64: /// <param name="s">Resulting string</param>
65: inline void PrintTo(bool x, baremetal::String& s)
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
83:     static void Print(const T& value, baremetal::String& s)
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
104: void UniversalPrint(const T& value, baremetal::String& s)
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
123:     static void Print(const T& value, baremetal::String& s)
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
141:     static void Print(const T& value, baremetal::String& s)
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
154: inline baremetal::String PrintToString(const T& value)
155: {
156:     baremetal::String s;
157:     UniversalTersePrinter<T>::Print(value, s);
158:     return s;
159: }
```

- Line 48-58: We define a template function `PrintTo()` which is a fallback to print any value to a string using a serializer. This uses the polymorphism of the `Serialize` functions.
- Line 60-68: We define a specialization to `PrintTo()` for boolean values
- Line 70-95: We declare a template class `UniversalPrinter` for type `const T&` that has a single method `Print()` which calls any defined `PrintTo()` function for the value passed to a string
- Line 97-108: We define a template function `UniversalPrint` for type `const T&` which uses the `Print()` method in the `UniversalPrinter` class
- Line 110-127: We declare a template class `UniversalTersePrinter` for type `T` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string
- Line 128-145: We declare a specialization of `UniversalTersePrinter` for type `T&` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string
- Line 147-159: We define a template function `PrintToString()` for type `const T&` which uses the `Print()` method of any matching class `UniversalTersePrinter`

### AssertMacros.h {#TUTORIAL_24_UNIT_TEST_MACROS_CREATING_TEST_CASES___STEP_2_ASSERTMACROSH}

Let's define the test macros.

Create the file `code/libraries/unittest/include/unittest/AssertMacros.h`

```cpp
File: code/libraries/unittest/include/unittest/AssertMacros.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
49: #error unittest redefines ASSERT_TRUE
50: #endif
51: 
52: #ifdef ASSERT_FALSE
53: #error unittest redefines ASSERT_FALSE
54: #endif
55: 
56: #ifdef ASSERT_EQ
57: #error unittest redefines ASSERT_EQ
58: #endif
59: 
60: #ifdef ASSERT_NE
61: #error unittest redefines ASSERT_NE
62: #endif
63: 
64: #ifdef EXPECT_TRUE
65: #error unittest redefines EXPECT_TRUE
66: #endif
67: 
68: #ifdef EXPECT_FALSE
69: #error unittest redefines EXPECT_FALSE
70: #endif
71: 
72: #ifdef EXPECT_EQ
73: #error unittest redefines EXPECT_EQ
74: #endif
75: 
76: #ifdef EXPECT_NE
77: #error unittest redefines EXPECT_NE
78: #endif
79: 
80: /// @brief Generic expect macro. Checks if the argument is true, if so generates a failure
81: #define UT_EXPECT_RESULT(value)                                                                                                                      \
82:     do                                                                                                                                               \
83:     {                                                                                                                                                \
84:         if (const ::unittest::AssertionResult UT_AssertionResult = (value))                                                                          \
85:             ::unittest::CurrentTest::Results()->OnTestFailure(::unittest::TestDetails(*::unittest::CurrentTest::Details(), __LINE__),                \
86:                                                               UT_AssertionResult.message);                                                           \
87:     } while (0)
88: /// @brief Generic assert macro. Checks if the argument is true, if so generates a failure
89: #define UT_ASSERT_RESULT(value)                                                                                                                      \
90:     do                                                                                                                                               \
91:     {                                                                                                                                                \
92:         if (const ::unittest::AssertionResult UT_AssertionResult = (value))                                                                          \
93:         {                                                                                                                                            \
94:             ::unittest::CurrentTest::Results()->OnTestFailure(::unittest::TestDetails(*::unittest::CurrentTest::Details(), __LINE__),                \
95:                                                               UT_AssertionResult.message);                                                           \
96:         }                                                                                                                                            \
97:     } while (0)
98: 
99: /// @brief Expect predicate function with one parameter (CheckTrue, CheckFalse), generates a failure using UT_EXPECT_RESULT if predicate function
100: /// returns false
101: #define EXPECT_PRED_FORMAT1(pred_format, v1)     UT_EXPECT_RESULT(pred_format(baremetal::String(#v1), v1))
102: /// @brief Assert predicate function with one parameter (CheckTrue, CheckFalse), generates a failure using UT_ASSERT_RESULT if predicate function
103: /// returns false
104: #define ASSERT_PRED_FORMAT1(pred_format, v1)     UT_ASSERT_RESULT(pred_format(baremetal::String(#v1), v1))
105: 
106: /// @brief Expect predicate function with two parameters (CheckEqual(IgnoreCase), CheckNotEqual(IgnoreCase)), generates a failure using
107: /// UT_EXPECT_RESULT if predicate function returns false
108: #define EXPECT_PRED_FORMAT2(pred_format, v1, v2) UT_EXPECT_RESULT(pred_format(baremetal::String(#v1), baremetal::String(#v2), v1, v2))
109: /// @brief Assert predicate function with two parameters (CheckEqual(IgnoreCase), CheckNotEqual(IgnoreCase)), generates a failure using
110: /// UT_ASSERT_RESULT if predicate function returns false
111: #define ASSERT_PRED_FORMAT2(pred_format, v1, v2) UT_ASSERT_RESULT(pred_format(baremetal::String(#v1), baremetal::String(#v2), v1, v2))
112: 
113: /// @brief Force failure with message
114: #define FAIL(message)                            UT_EXPECT_RESULT(GenericFailure(message))
115: /// @brief Assert that value is true
116: #define ASSERT_TRUE(value)                                                                                                                           \
117:     do                                                                                                                                               \
118:     {                                                                                                                                                \
119:         ASSERT_PRED_FORMAT1(::unittest::CheckTrue, value);                                                                                           \
120:     } while (0)
121: /// @brief Expect that value is true
122: #define EXPECT_TRUE(value)                                                                                                                           \
123:     do                                                                                                                                               \
124:     {                                                                                                                                                \
125:         EXPECT_PRED_FORMAT1(::unittest::CheckTrue, value);                                                                                           \
126:     } while (0)
127: 
128: /// @brief Assert that value is false
129: #define ASSERT_FALSE(value)                                                                                                                          \
130:     do                                                                                                                                               \
131:     {                                                                                                                                                \
132:         ASSERT_PRED_FORMAT1(::unittest::CheckFalse, value);                                                                                          \
133:     } while (0)
134: /// @brief Expect that value is false
135: #define EXPECT_FALSE(value)                                                                                                                          \
136:     do                                                                                                                                               \
137:     {                                                                                                                                                \
138:         EXPECT_PRED_FORMAT1(::unittest::CheckFalse, value);                                                                                          \
139:     } while (0)
140: 
141: /// @brief Assert that actual value is equal to expected value
142: #define ASSERT_EQ(expected, actual)                                                                                                                  \
143:     do                                                                                                                                               \
144:     {                                                                                                                                                \
145:         ASSERT_PRED_FORMAT2(::unittest::EqHelper<false>::CheckEqual, expected, actual);                                          \
146:     } while (0)
147: /// @brief Expect that actual value is equal to expected value
148: #define EXPECT_EQ(expected, actual)                                                                                                                  \
149:     do                                                                                                                                               \
150:     {                                                                                                                                                \
151:         EXPECT_PRED_FORMAT2(::unittest::EqHelper<false>::CheckEqual, expected, actual);                                          \
152:     } while (0)
153: 
154: /// @brief Assert that actual value is not equal to expected value
155: #define ASSERT_NE(expected, actual)                                                                                                                  \
156:     do                                                                                                                                               \
157:     {                                                                                                                                                \
158:         ASSERT_PRED_FORMAT2(::unittest::EqHelper<false>::CheckNotEqual, expected, actual);                                       \
159:     } while (0)
160: /// @brief Expect that actual value is not equal to expected value
161: #define EXPECT_NE(expected, actual)                                                                                                                  \
162:     do                                                                                                                                               \
163:     {                                                                                                                                                \
164:         EXPECT_PRED_FORMAT2(::unittest::EqHelper<false>::CheckNotEqual, expected, actual);                                       \
165:     } while (0)
```

- Line 48-78: We check whether any of the defines defined in this header are already defined. If so compilation ends with an error
- Line 80-87: We define a macro `UT_EXPECT_RESULT` which checks if the parameter passed (which is of type `AssertionResult`) has a failure (using the `bool()` method).
If there is a failure, a test failure is added to the current result
- Line 88-97: We define a macro `UT_ASSERT_RESULT` which checks if the parameter passed (which is of type `AssertionResult`) has a failure (using the `bool()` method).
If there is a failure, a test failure is added to the current result. Normally an exception would be thrown, however we don't have exceptions enabled yet
- Line 99-101: We define a macro `EXPECT_PRED_FORMAT1` which is passed a check function and a single parameter.
The check function is called, and the result is passed to `UT_EXPECT_RESULT`.
The single parameter version is used with `CheckTrue()` or `CheckFalse()` to check the result of a boolean expression
- Line 102-104: We define a macro `ASSERT_PRED_FORMAT1` which is passed a check function and a single parameter.
The check function is called, and the result is passed to `UT_ASSERT_RESULT`.
The single parameter version is used with `CheckTrue()` or `CheckFalse(` to check the result of a boolean expression
- Line 106-108: We define a macro `EXPECT_PRED_FORMAT2` which is passed a check function and two parameters.
The check function is called, and the result is passed to `UT_EXPECT_RESULT`.
The two parameter version is used with `EqHelper::CheckEqual()` or `EqHelper::CheckNotEqual()` to check the result of a comparison expression
- Line 109-111: We define a macro `ASSERT_PRED_FORMAT2` which is passed a check function and two parameters.
The check function is called, and the result is passed to `UT_ASSERT_RESULT`.
The two parameter version is used with `EqHelper::CheckEqual()` or `EqHelper::CheckNotEqual()` to check the result of a comparison expression
- Line 113-114: We define a macro `FAIL` which uses `UT_EXPECT_RESULT` to return a `GenericFailure`
- Line 115-120: We define a macro `ASSERT_TRUE` which uses `ASSERT_PRED_FORMAT1` with `CheckTrue()` to check if the parameter is true, and generate a `BooleanFailure` if the check fails
- Line 121-126: We define a macro `EXPECT_TRUE` which uses `EXPECT_PRED_FORMAT1` with `CheckTrue()` to check if the parameter is true, and generate a `BooleanFailure` if the check fails
- Line 128-133: We define a macro `ASSERT_FALSE` which uses `ASSERT_PRED_FORMAT1` with `CheckFalse()` to check if the parameter is false, and generate a `BooleanFailure` if the check fails
- Line 134-139: We define a macro `EXPECT_FALSE` which uses `EXPECT_PRED_FORMAT1` with `CheckFalse()` to check if the parameter is false, and generate a `BooleanFailure` if the check fails
- Line 141-146: We define a macro `ASSERT_EQ` which uses `ASSERT_PRED_FORMAT2` with `EqHelper::CheckEqual()` to check if the parameters are equal, and generate a `EqFailure` if the check fails
- Line 147-152: We define a macro `EXPECT_EQ` which uses `EXPECT_PRED_FORMAT2` with `EqHelper::CheckEqual()` to check if the parameters are equal, and generate a `EqFailure` if the check fails
- Line 154-159: We define a macro `ASSERT_NE` which uses `ASSERT_PRED_FORMAT2` with `EqHelper::CheckNotEqual()` to check if the parameters are not equal, and generate a `InEqFailure` if the check fails
- Line 160-165: We define a macro `EXPECT_NE` which uses `EXPECT_PRED_FORMAT2` with `EqHelper::CheckNotEqual()` to check if the parameters are not equal, and generate a `InEqFailure` if the check fails

### unittest.h {#TUTORIAL_24_UNIT_TEST_MACROS_CREATING_TEST_CASES___STEP_2_UNITTESTH}

We added a header file, so we'll update the `unittest.h` header

Update the file `code/libraries/unittest/include/unittest/unittest.h`

```cpp
File: code/libraries/unittest/include/unittest/unittest.h
...
45: #include "unittest/TestFixture.h"
46: #include "unittest/TestSuite.h"
47:
48: #include "unittest/ITestReporter.h"
49: #include "unittest/AssertMacros.h"
50: #include "unittest/ConsoleTestReporter.h"
51: #include "unittest/CurrentTest.h"
52: #include "unittest/DeferredTestReporter.h"
53: #include "unittest/Test.h"
54: #include "unittest/TestDetails.h"
55: #include "unittest/TestFixtureInfo.h"
56: #include "unittest/TestInfo.h"
57: #include "unittest/TestMacros.h"
58: #include "unittest/TestRegistry.h"
59: #include "unittest/TestResults.h"
60: #include "unittest/TestRunner.h"
61: #include "unittest/TestSuiteInfo.h"
```

### Application code {#TUTORIAL_24_UNIT_TEST_MACROS_CREATING_TEST_CASES___STEP_2_APPLICATION_CODE}

Now let's start using the macros we defined.

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
13: TEST_SUITE(Suite1)
14: {
15: 
16:     class FixtureMyTest1 : public TestFixture
17:     {
18:     public:
19:         void SetUp() override
20:         {
21:             LOG_DEBUG("MyTest SetUp");
22:         }
23:         void TearDown() override
24:         {
25:             LOG_DEBUG("MyTest TearDown");
26:         }
27:     };
28: 
29:     TEST_FIXTURE(FixtureMyTest1, MyTest1)
30:     {
31:         FAIL("For some reason");
32:     }
33: 
34: } // Suite1
35: 
36: TEST_SUITE(Suite2)
37: {
38: 
39:     class FixtureMyTest2 : public TestFixture
40:     {
41:     public:
42:         void SetUp() override
43:         {
44:             LOG_DEBUG("FixtureMyTest2 SetUp");
45:         }
46:         void TearDown() override
47:         {
48:             LOG_DEBUG("FixtureMyTest2 TearDown");
49:         }
50:     };
51: 
52:     TEST_FIXTURE(FixtureMyTest2, MyTest2)
53:     {
54:         EXPECT_TRUE(true);
55:         EXPECT_FALSE(false);
56:         EXPECT_TRUE(false);
57:         EXPECT_FALSE(true);
58:     }
59: 
60: } // Suite2
61: 
62: class FixtureMyTest3 : public TestFixture
63: {
64: public:
65:     void SetUp() override
66:     {
67:         LOG_DEBUG("FixtureMyTest3 SetUp");
68:     }
69:     void TearDown() override
70:     {
71:         LOG_DEBUG("FixtureMyTest3 TearDown");
72:     }
73: };
74: 
75: TEST_FIXTURE(FixtureMyTest3, MyTest3)
76: {
77:     int x = 0;
78:     int y = 1;
79:     int z = 1;
80:     EXPECT_EQ(x, y);
81:     EXPECT_EQ(y, z);
82:     EXPECT_NE(x, y);
83:     EXPECT_NE(y, z);
84: }
85: 
86: TEST(MyTest4)
87: {
88:     ASSERT_TRUE(false);
89: }
90: 
91: int main()
92: {
93:     auto& console = GetConsole();
94: 
95:     ConsoleTestReporter reporter;
96:     RunAllTests(&reporter);
97: 
98:     LOG_INFO("Wait 5 seconds");
99:     Timer::WaitMilliSeconds(5000);
100: 
101:     console.Write("Press r to reboot, h to halt\n");
102:     char ch{};
103:     while ((ch != 'r') && (ch != 'h'))
104:     {
105:         ch = console.ReadChar();
106:         console.WriteChar(ch);
107:     }
108: 
109:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
110: }
```

- Line 30-33: We change the `MyTest1` function to a call to `FAIL()`, this will generate a simple failure
- Line 54-60: We change the `MyTest2` function to boolean checks. Obviously, `EXPECT_TRUE(false)` and `EXPECT_FALSE(true)` will fail
- Line 78-87: We change the `MyTest3` function to equality checks. Obviously, `EXPECT_EQ(x, y)` and ` EXPECT_NE(y, z)` will fail
- Line 89-92: We change the `Test4` function to a failed assertion.
As said before, this should throw an exception, but for now this is commented out, so it will result in a normal failure, and the test run will continue

### Configuring, building and debugging {#TUTORIAL_24_UNIT_TEST_MACROS_CREATING_TEST_CASES___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.000 Starting up (System:213)
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
D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:32 : Failure in Suite1::FixtureMyTest1::MyTest1: For some reason
D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:58 : Failure in Suite2::FixtureMyTest2::MyTest2: Value of: false
  Expected: true

D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:59 : Failure in Suite2::FixtureMyTest2::MyTest2: Value of: true
  Expected: false

D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:83 : Failure in DefaultSuite::FixtureMyTest3::MyTest3: Value of: y
  Actual: 1
  Expected: x
  Which is: 0

D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:86 : Failure in DefaultSuite::FixtureMyTest3::MyTest3: Value of: z
  Actual: 1
  Expected not equal to: y
  Which is: 1

D:/Projects/RaspberryPi/baremetal.github/code/applications/demo/src/main.cpp:91 : Failure in DefaultSuite::DefaultFixture::MyTest: Value of: false
  Expected: true


[===========] 4 tests from 4 fixtures in 3 suites ran.
Info   0.00:00:00.100 Wait 5 seconds (main:101)
Press r to reboot, h to halt
```

## Test assert macro extension - Step 3 {#TUTORIAL_24_UNIT_TEST_MACROS_TEST_ASSERT_MACRO_EXTENSION___STEP_3}

We can now perform boolean checks and compare integers, but we would also like to be able to compare pointers and strings.
So we'll extend the macros a bit.

### Checks.h {#TUTORIAL_24_UNIT_TEST_MACROS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_CHECKSH}

We need to extend the utility functions a bit.
We'll add a comparison for floating point value, when need to be checked with a tolerance.
Also, we'll add some strings comparisons, both case sensitive and case insensitive.

Update the file `code/libraries/unittest/include/unittest/Checks.h`

```cpp
File: code/libraries/unittest/include/unittest/Checks.h
...
91: extern AssertionResult CloseFailure(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
92:                                     const baremetal::String& toleranceExpression, const baremetal::String& expectedValue,
93:                                     const baremetal::String& actualValue, const baremetal::String& toleranceValue);
...
242: AssertionResult CheckEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char const* expected,
243:                                    char const* actual);
244: 
245: AssertionResult CheckEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char* expected,
246:                                    char* actual);
247: 
248: AssertionResult CheckEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char* expected,
249:                                    char const* actual);
250: 
251: AssertionResult CheckEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char const* expected,
252:                                    char* actual);
253: 
254: AssertionResult CheckNotEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char const* expected,
255:                                       char const* actual);
256: 
257: AssertionResult CheckNotEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char* expected,
258:                                       char* actual);
259: 
260: AssertionResult CheckNotEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char* expected,
261:                                       char const* actual);
262: 
263: AssertionResult CheckNotEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char const* expected,
264:                                       char* actual);
265: 
266: AssertionResult CheckEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
267:                                    const baremetal::String& expected, const baremetal::String& actual);
268: 
269: AssertionResult CheckEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
270:                                    const baremetal::String& expected, const char* actual);
271: 
272: AssertionResult CheckEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, const char* expected,
273:                                    const baremetal::String& actual);
274: 
275: AssertionResult CheckNotEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
276:                                       const baremetal::String& expected, const baremetal::String& actual);
277: 
278: AssertionResult CheckNotEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
279:                                       const baremetal::String& expected, const char* actual);
280: 
281: AssertionResult CheckNotEqualInternal(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, const char* expected,
282:                                       const baremetal::String& actual);
283: 
284: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
285:                                              char const* expected, char const* actual);
286: 
287: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char* expected,
288:                                              char* actual);
289: 
290: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression, char* expected,
291:                                              char const* actual);
292: 
293: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
294:                                              char const* expected, char* actual);
295: 
296: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
297:                                                 char const* expected, char const* actual);
298: 
299: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
300:                                                 char* expected, char* actual);
301: 
302: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
303:                                                 char* expected, char const* actual);
304: 
305: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
306:                                                 char const* expected, char* actual);
307: 
308: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
309:                                              const baremetal::String& expected, const baremetal::String& actual);
310: 
311: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
312:                                              const baremetal::String& expected, const char* actual);
313: 
314: AssertionResult CheckEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
315:                                              const char* expected, const baremetal::String& actual);
316: 
317: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
318:                                                 const baremetal::String& expected, const baremetal::String& actual);
319: 
320: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
321:                                                 const baremetal::String& expected, const char* actual);
322: 
323: AssertionResult CheckNotEqualInternalIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
324:                                                 const char* expected, const baremetal::String& actual);
325: 
...
373: /// <summary>
374: /// Helper class for {ASSERT|EXPECT}_EQ/NE_IGNORE_CASE
375: ///
376: /// Forms generalized mechanism for calling polymorphic check functions for string comparisons ignoring case.
377: /// </summary>
378: class EqHelperStringCaseInsensitive
379: {
380: public:
381:     /// <summary>
382:     /// Evaluate whether an expected value is equal to an actual value, ignoring case, generate a success object if successful, otherwise a failure
383:     /// object
384:     /// </summary>
385:     /// <typeparam name="Expected">Type of the expected value</typeparam>
386:     /// <typeparam name="Actual">Type of the actual value</typeparam>
387:     /// <param name="expectedExpression">String representation of the expected value</param>
388:     /// <param name="actualExpression">String representation of the actual value</param>
389:     /// <param name="expected">Expected value</param>
390:     /// <param name="actual">Actual value</param>
391:     /// <returns>Result object</returns>
392:     template <typename Expected, typename Actual>
393:     static AssertionResult CheckEqualIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
394:                                                 const Expected& expected, const Actual& actual)
395:     {
396:         return CheckEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
397:     }
398: 
399:     /// <summary>
400:     /// Evaluate whether an expected value is not equal to an actual value, ignoring case, generate a success object if successful, otherwise a
401:     /// failure object
402:     /// </summary>
403:     /// <typeparam name="Expected">Type of the expected value</typeparam>
404:     /// <typeparam name="Actual">Type of the actual value</typeparam>
405:     /// <param name="expectedExpression">String representation of the not expected value</param>
406:     /// <param name="actualExpression">String representation of the actual value</param>
407:     /// <param name="expected">Expected value</param>
408:     /// <param name="actual">Actual value</param>
409:     /// <returns>Result object</returns>
410:     template <typename Expected, typename Actual>
411:     static AssertionResult CheckNotEqualIgnoreCase(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
412:                                                    const Expected& expected, const Actual& actual)
413:     {
414:         return CheckNotEqualInternalIgnoreCase(expectedExpression, actualExpression, expected, actual);
415:     }
416: };
417: 
418: /// <summary>
419: /// Compare an expected and actual value, with tolerance
420: ///
421: /// Compares two floating point values, and returns true if the absolute difference is within tolerance
422: /// </summary>
423: /// <typeparam name="Expected">Type of expected value</typeparam>
424: /// <typeparam name="Actual">Type of actual value</typeparam>
425: /// <typeparam name="Tolerance">Type of tolerance value</typeparam>
426: /// <param name="expected">Expected value</param>
427: /// <param name="actual">Actual value</param>
428: /// <param name="tolerance">Tolerance value</param>
429: /// <returns>True if the values are considered equal, false otherwise</returns>
430: template <typename Expected, typename Actual, typename Tolerance>
431: bool AreClose(const Expected& expected, const Actual& actual, Tolerance const& tolerance)
432: {
433:     return (actual >= (expected - tolerance)) && (actual <= (expected + tolerance));
434: }
435: 
436: /// <summary>
437: /// Evaluate whether an expected value is equal to an actual value within tolerance, generate a success object if successful, otherwise a failure
438: /// object
439: /// </summary>
440: /// <typeparam name="Expected">Type of the expected value</typeparam>
441: /// <typeparam name="Actual">Type of the actual value</typeparam>
442: /// <typeparam name="Tolerance">Type of the tolerance value</typeparam>
443: /// <param name="expectedExpression">String representation of the expected value</param>
444: /// <param name="actualExpression">String representation of the actual value</param>
445: /// <param name="toleranceExpression">String representation of the tolerance value</param>
446: /// <param name="expected">Expected value</param>
447: /// <param name="actual">Actual value</param>
448: /// <param name="tolerance">Tolerance value</param>
449: /// <returns>Result object</returns>
450: template <typename Expected, typename Actual, typename Tolerance>
451: AssertionResult CheckClose(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
452:                            const baremetal::String& toleranceExpression, const Expected& expected, const Actual& actual, Tolerance const& tolerance)
453: {
454:     if (!AreClose(expected, actual, tolerance))
455:     {
456:         return CloseFailure(expectedExpression, actualExpression, toleranceExpression, FormatForComparisonFailureMessage(expected),
457:                             FormatForComparisonFailureMessage(actual), FormatForComparisonFailureMessage(tolerance));
458:     }
459:     return AssertionSuccess();
460: }
461: 
462: } // namespace unittest
```

- Line 91-93: We declare a function `CloseFailure()` to signify a assertion failure on comparison between numbers with a fault tolerance
- Line 242-243: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 245-246: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 248-249: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 251-252: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 254-255: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 257-258: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 260-261: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 263-264: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 266-267: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 269-270: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 272-273: We declare a new variant of the function `CheckEqualInternal()`, which compares two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 275-276: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 278-279: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 281-282: We declare a new variant of the function `CheckNotEqualInternal()`, which compares two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 284-285: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 287-288: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 290-291: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 293-294: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 296-297: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 299-300: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 302-303: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type char * and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 305-306: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char * and `actual` of type char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 308-309: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 311-312: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 314-315: We declare a new variant of the function `CheckEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 317-318: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 320-321: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type string and `actual` of type const char*.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 323-324: We declare a new variant of the function `CheckNotEqualInternalIgnoreCase()`, which compares ignoring case two values `expected` of type const char* and `actual` of type string.
Their stringified versions are passed as `expectedExpression` and `actualExpression`.
If the values are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 373-416: We declare a class `EqHelperCaseInsensitive` which has two methods `CheckEqualIgnoreCase()` (which uses `CheckEqualInternalIgnoreCase()`) and `CheckNotEqualIgnoreCase()` (which uses `CheckNotEqualInternalIgnoreCase()`)
- Line 418-434: We define a template function `AreClose()` to compare two values `expected` and `actual`, with a tolerance `tolerance`, which can have different types.
The function compares the absolute difference between `expected` and `actual` with the given tolerance, It returns true of the difference is smaller or equal to the tolerance, false otherwise
- Line 436-460: We define a template function `CheckClose()`, which uses `AreClose()` to compare two values `expected` and `actual`. Their stringified versions are passed as `expectedExpression` and `actualExpression` with tolerance `tolerance`.
If the absolute difference between `expected` and `actual` is smaller or equal to `tolerance`, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned

### Checks.cpp {#TUTORIAL_24_UNIT_TEST_MACROS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_CHECKSCPP}

Let's implement the new functions for the checks.

Update the file `code/libraries/unittest/src/Checks.cpp`

```cpp
File: code/libraries/unittest/src/Checks.cpp
...
49: using namespace unittest;
50: 
51: /// <summary>
52: /// Compare two strings ignoring case
53: /// </summary>
54: /// <param name="a">Left hand side of comparison</param>
55: /// <param name="b">Right hand side of comparison</param>
56: /// <returns>True if the strings are equal ignoring case, false otherwise</returns>
57: static bool EqualCaseInsensitive(const String& a, const String& b)
58: {
59:     if (a.length() != b.length())
60:         return false;
61:     return strcasecmp(a.data(), b.data()) == 0;
62: }
63: 
64: /// <summary>
65: /// Create a success object
66: /// </summary>
67: /// <returns>Result object</returns>
68: AssertionResult unittest::AssertionSuccess()
69: {
70:     return AssertionResult(false, String());
71: }
72: 
73: /// <summary>
74: /// Create a generic failure object with the provided message
75: /// </summary>
76: /// <param name="message">Message to be included</param>
77: /// <returns>Result object</returns>
78: AssertionResult unittest::GenericFailure(const baremetal::String& message)
79: {
80:     return AssertionResult(true, message);
81: }
82: 
83: /// <summary>
84: /// Create a boolean failure object
85: /// </summary>
86: /// <param name="valueExpression">String representation of the actual value</param>
87: /// <param name="expectedValue">Expected value</param>
88: /// <param name="actualValue">Actual value</param>
89: /// <returns>Result object</returns>
90: AssertionResult unittest::BooleanFailure(const baremetal::String& valueExpression, const baremetal::String& expectedValue,
91:                                          const baremetal::String& actualValue)
92: {
93:     String result = Format("Value of: %s", valueExpression.c_str());
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
112: AssertionResult unittest::EqFailure(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
113:                                     const baremetal::String& expectedValue, const baremetal::String& actualValue)
114: {
115:     String result = Format("Value of: %s", actualExpression.c_str());
116:     if (actualValue != actualExpression)
117:     {
118:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
119:     }
120: 
121:     result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
122:     if (expectedValue != expectedExpression)
123:     {
124:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
125:     }
126:     result.append("\n");
127: 
128:     return AssertionResult(true, result);
129: }
130: 
131: /// <summary>
132: /// Create a inequality comparison failure object
133: /// </summary>
134: /// <param name="expectedExpression">String representation of the not expected value</param>
135: /// <param name="actualExpression">String representation of the actual value</param>
136: /// <param name="expectedValue">Expected value</param>
137: /// <param name="actualValue">Actual value</param>
138: /// <returns>Result object</returns>
139: AssertionResult unittest::InEqFailure(const baremetal::String& expectedExpression, const baremetal::String& actualExpression,
140:                                       const baremetal::String& expectedValue, const baremetal::String& actualValue)
141: {
142:     String result = Format("Value of: %s", actualExpression.c_str());
143:     if (actualValue != actualExpression)
144:     {
145:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
146:     }
147: 
148:     result.append(Format("\n  Expected not equal to: %s", expectedExpression.c_str()));
149:     if (expectedValue != expectedExpression)
150:     {
151:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
152:     }
153:     result.append("\n");
154: 
155:     return AssertionResult(true, result);
156: }
157: 
158: /// <summary>
159: /// Create a comparison with tolerance failure object
160: /// </summary>
161: /// <param name="expectedExpression">String representation of the expected value</param>
162: /// <param name="actualExpression">String representation of the actual value</param>
163: /// <param name="toleranceExpression">String representation of the tolerance value</param>
164: /// <param name="expectedValue">Expected value</param>
165: /// <param name="actualValue">Actual value</param>
166: /// <param name="toleranceValue">Tolerance value</param>
167: /// <returns>Result object</returns>
168: AssertionResult unittest::CloseFailure(const String& expectedExpression, const String& actualExpression, const String& toleranceExpression,
169:                                        const String& expectedValue, const String& actualValue, const String& toleranceValue)
170: {
171:     String result = Format("Value of: %s", actualExpression.c_str());
172:     if (actualValue != actualExpression)
173:     {
174:         result.append(Format("\n  Actual: %s", actualValue.c_str()));
175:     }
176: 
177:     result.append(Format("\n  Expected: %s", expectedExpression.c_str()));
178:     if (expectedValue != expectedExpression)
179:     {
180:         result.append(Format("\n  Which is: %s", expectedValue.c_str()));
181:     }
182:     result.append(Format("\n  Tolerance: %s", toleranceExpression.c_str()));
183:     if (toleranceValue != toleranceExpression)
184:     {
185:         result.append(Format("\n  (+/-) %s", toleranceValue.c_str()));
186:     }
187: 
188:     return AssertionResult(true, result);
189: }
190: 
191: namespace internal {
192: 
193: /// <summary>
194: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
195: /// </summary>
196: /// <param name="expectedExpression">String representation of expected value</param>
197: /// <param name="actualExpression">String representation of actual value</param>
198: /// <param name="expected">Expected value</param>
199: /// <param name="actual">Actual value</param>
200: /// <returns>Result object</returns>
201: AssertionResult CheckStringsEqual(const String& expectedExpression, const String& actualExpression, char const* expected, char const* actual)
202: {
203:     if (expected == actual)
204:         return AssertionSuccess();
205: 
206:     if (strcmp(expected, actual))
207:     {
208:         return EqFailure(expectedExpression, actualExpression, String(expected), String(actual));
209:     }
210:     return AssertionSuccess();
211: }
212: 
213: /// <summary>
214: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
215: /// </summary>
216: /// <param name="expectedExpression">String representation of expected value</param>
217: /// <param name="actualExpression">String representation of actual value</param>
218: /// <param name="expected">Expected value</param>
219: /// <param name="actual">Actual value</param>
220: /// <returns>Result object</returns>
221: AssertionResult CheckStringsNotEqual(const String& expectedExpression, const String& actualExpression, char const* expected, char const* actual)
222: {
223:     if (expected == actual)
224:         return InEqFailure(expectedExpression, actualExpression, String(expected), String(actual));
225: 
226:     if (!strcmp(expected, actual))
227:     {
228:         return InEqFailure(expectedExpression, actualExpression, String(expected), String(actual));
229:     }
230:     return AssertionSuccess();
231: }
232: 
233: /// <summary>
234: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
235: /// </summary>
236: /// <param name="expectedExpression">String representation of expected value</param>
237: /// <param name="actualExpression">String representation of actual value</param>
238: /// <param name="expected">Expected value</param>
239: /// <param name="actual">Actual value</param>
240: /// <returns>Result object</returns>
241: AssertionResult CheckStringsEqualIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
242:                                             char const* actual)
243: {
244:     if (expected == actual)
245:         return AssertionSuccess();
246: 
247:     if (!EqualCaseInsensitive(String(expected), String(actual)))
248:     {
249:         return EqFailure(expectedExpression, actualExpression, String(expected), String(actual));
250:     }
251:     return AssertionSuccess();
252: }
253: 
254: /// <summary>
255: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
256: /// </summary>
257: /// <param name="expectedExpression">String representation of expected value</param>
258: /// <param name="actualExpression">String representation of actual value</param>
259: /// <param name="expected">Expected value</param>
260: /// <param name="actual">Actual value</param>
261: /// <returns>Result object</returns>
262: AssertionResult CheckStringsNotEqualIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
263:                                                char const* actual)
264: {
265:     if (expected == actual)
266:         return InEqFailure(expectedExpression, actualExpression, String(expected), String(actual));
267: 
268:     if (EqualCaseInsensitive(String(expected), String(actual)))
269:     {
270:         return InEqFailure(expectedExpression, actualExpression, String(expected), String(actual));
271:     }
272:     return AssertionSuccess();
273: }
274: 
275: } // namespace internal
276: 
277: /// <summary>
278: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
279: /// </summary>
280: /// <param name="expectedExpression">String representation of expected value</param>
281: /// <param name="actualExpression">String representation of actual value</param>
282: /// <param name="expected">Expected value</param>
283: /// <param name="actual">Actual value</param>
284: /// <returns>Result object</returns>
285: AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, char const* expected,
286:                                              char const* actual)
287: {
288:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
289: }
290: 
291: /// <summary>
292: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
293: /// </summary>
294: /// <param name="expectedExpression">String representation of expected value</param>
295: /// <param name="actualExpression">String representation of actual value</param>
296: /// <param name="expected">Expected value</param>
297: /// <param name="actual">Actual value</param>
298: /// <returns>Result object</returns>
299: AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, char* expected,
300:                                              char* actual) // cppcheck-suppress constParameterPointer
301: {
302:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
303: }
304: 
305: /// <summary>
306: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
307: /// </summary>
308: /// <param name="expectedExpression">String representation of expected value</param>
309: /// <param name="actualExpression">String representation of actual value</param>
310: /// <param name="expected">Expected value</param>
311: /// <param name="actual">Actual value</param>
312: /// <returns>Result object</returns>
313: AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, char* expected,
314:                                              char const* actual) // cppcheck-suppress constParameterPointer
315: {
316:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
317: }
318: 
319: /// <summary>
320: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
321: /// </summary>
322: /// <param name="expectedExpression">String representation of expected value</param>
323: /// <param name="actualExpression">String representation of actual value</param>
324: /// <param name="expected">Expected value</param>
325: /// <param name="actual">Actual value</param>
326: /// <returns>Result object</returns>
327: AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, char const* expected,
328:                                              char* actual) // cppcheck-suppress constParameterPointer
329: {
330:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
331: }
332: 
333: /// <summary>
334: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
335: /// </summary>
336: /// <param name="expectedExpression">String representation of expected value</param>
337: /// <param name="actualExpression">String representation of actual value</param>
338: /// <param name="expected">Expected value</param>
339: /// <param name="actual">Actual value</param>
340: /// <returns>Result object</returns>
341: AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, char const* expected,
342:                                                 char const* actual)
343: {
344:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
345: }
346: 
347: /// <summary>
348: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
349: /// </summary>
350: /// <param name="expectedExpression">String representation of expected value</param>
351: /// <param name="actualExpression">String representation of actual value</param>
352: /// <param name="expected">Expected value</param>
353: /// <param name="actual">Actual value</param>
354: /// <returns>Result object</returns>
355: AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, char* expected,
356:                                                 char* actual) // cppcheck-suppress constParameterPointer
357: {
358:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
359: }
360: 
361: /// <summary>
362: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
363: /// </summary>
364: /// <param name="expectedExpression">String representation of expected value</param>
365: /// <param name="actualExpression">String representation of actual value</param>
366: /// <param name="expected">Expected value</param>
367: /// <param name="actual">Actual value</param>
368: /// <returns>Result object</returns>
369: AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, char* expected,
370:                                                 char const* actual) // cppcheck-suppress constParameterPointer
371: {
372:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
373: }
374: 
375: /// <summary>
376: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
377: /// </summary>
378: /// <param name="expectedExpression">String representation of expected value</param>
379: /// <param name="actualExpression">String representation of actual value</param>
380: /// <param name="expected">Expected value</param>
381: /// <param name="actual">Actual value</param>
382: /// <returns>Result object</returns>
383: AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, char const* expected,
384:                                                 char* actual) // cppcheck-suppress constParameterPointer
385: {
386:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
387: }
388: 
389: /// <summary>
390: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
391: /// </summary>
392: /// <param name="expectedExpression">String representation of expected value</param>
393: /// <param name="actualExpression">String representation of actual value</param>
394: /// <param name="expected">Expected value</param>
395: /// <param name="actual">Actual value</param>
396: /// <returns>Result object</returns>
397: AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, const String& expected,
398:                                              const String& actual)
399: {
400:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
401: }
402: 
403: /// <summary>
404: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
405: /// </summary>
406: /// <param name="expectedExpression">String representation of expected value</param>
407: /// <param name="actualExpression">String representation of actual value</param>
408: /// <param name="expected">Expected value</param>
409: /// <param name="actual">Actual value</param>
410: /// <returns>Result object</returns>
411: AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, const String& expected,
412:                                              const char* actual)
413: {
414:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
415: }
416: 
417: /// <summary>
418: /// Check that strings are equal, generate a success object if successful, otherwise a failure object
419: /// </summary>
420: /// <param name="expectedExpression">String representation of expected value</param>
421: /// <param name="actualExpression">String representation of actual value</param>
422: /// <param name="expected">Expected value</param>
423: /// <param name="actual">Actual value</param>
424: /// <returns>Result object</returns>
425: AssertionResult unittest::CheckEqualInternal(const String& expectedExpression, const String& actualExpression, const char* expected,
426:                                              const String& actual)
427: {
428:     return internal::CheckStringsEqual(expectedExpression, actualExpression, expected, actual);
429: }
430: 
431: /// <summary>
432: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
433: /// </summary>
434: /// <param name="expectedExpression">String representation of expected value</param>
435: /// <param name="actualExpression">String representation of actual value</param>
436: /// <param name="expected">Expected value</param>
437: /// <param name="actual">Actual value</param>
438: /// <returns>Result object</returns>
439: AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, const String& expected,
440:                                                 const String& actual)
441: {
442:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
443: }
444: 
445: /// <summary>
446: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
447: /// </summary>
448: /// <param name="expectedExpression">String representation of expected value</param>
449: /// <param name="actualExpression">String representation of actual value</param>
450: /// <param name="expected">Expected value</param>
451: /// <param name="actual">Actual value</param>
452: /// <returns>Result object</returns>
453: AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, const String& expected,
454:                                                 const char* actual)
455: {
456:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
457: }
458: 
459: /// <summary>
460: /// Check that strings are not equal, generate a success object if successful, otherwise a failure object
461: /// </summary>
462: /// <param name="expectedExpression">String representation of expected value</param>
463: /// <param name="actualExpression">String representation of actual value</param>
464: /// <param name="expected">Expected value</param>
465: /// <param name="actual">Actual value</param>
466: /// <returns>Result object</returns>
467: AssertionResult unittest::CheckNotEqualInternal(const String& expectedExpression, const String& actualExpression, const char* expected,
468:                                                 const String& actual)
469: {
470:     return internal::CheckStringsNotEqual(expectedExpression, actualExpression, expected, actual);
471: }
472: 
473: /// <summary>
474: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
475: /// </summary>
476: /// <param name="expectedExpression">String representation of expected value</param>
477: /// <param name="actualExpression">String representation of actual value</param>
478: /// <param name="expected">Expected value</param>
479: /// <param name="actual">Actual value</param>
480: /// <returns>Result object</returns>
481: AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
482:                                                        char const* actual)
483: {
484:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
485: }
486: 
487: /// <summary>
488: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
489: /// </summary>
490: /// <param name="expectedExpression">String representation of expected value</param>
491: /// <param name="actualExpression">String representation of actual value</param>
492: /// <param name="expected">Expected value</param>
493: /// <param name="actual">Actual value</param>
494: /// <returns>Result object</returns>
495: AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char* expected, char* actual)
496: {
497:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
498: }
499: 
500: /// <summary>
501: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
502: /// </summary>
503: /// <param name="expectedExpression">String representation of expected value</param>
504: /// <param name="actualExpression">String representation of actual value</param>
505: /// <param name="expected">Expected value</param>
506: /// <param name="actual">Actual value</param>
507: /// <returns>Result object</returns>
508: AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char* expected,
509:                                                        char const* actual)
510: {
511:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
512: }
513: 
514: /// <summary>
515: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
516: /// </summary>
517: /// <param name="expectedExpression">String representation of expected value</param>
518: /// <param name="actualExpression">String representation of actual value</param>
519: /// <param name="expected">Expected value</param>
520: /// <param name="actual">Actual value</param>
521: /// <returns>Result object</returns>
522: AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
523:                                                        char* actual)
524: {
525:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
526: }
527: 
528: /// <summary>
529: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
530: /// </summary>
531: /// <param name="expectedExpression">String representation of expected value</param>
532: /// <param name="actualExpression">String representation of actual value</param>
533: /// <param name="expected">Expected value</param>
534: /// <param name="actual">Actual value</param>
535: /// <returns>Result object</returns>
536: AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
537:                                                           char const* actual)
538: {
539:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
540: }
541: 
542: /// <summary>
543: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
544: /// </summary>
545: /// <param name="expectedExpression">String representation of expected value</param>
546: /// <param name="actualExpression">String representation of actual value</param>
547: /// <param name="expected">Expected value</param>
548: /// <param name="actual">Actual value</param>
549: /// <returns>Result object</returns>
550: AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char* expected,
551:                                                           char* actual)
552: {
553:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
554: }
555: 
556: /// <summary>
557: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
558: /// </summary>
559: /// <param name="expectedExpression">String representation of expected value</param>
560: /// <param name="actualExpression">String representation of actual value</param>
561: /// <param name="expected">Expected value</param>
562: /// <param name="actual">Actual value</param>
563: /// <returns>Result object</returns>
564: AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char* expected,
565:                                                           char const* actual)
566: {
567:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
568: }
569: 
570: /// <summary>
571: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
572: /// </summary>
573: /// <param name="expectedExpression">String representation of expected value</param>
574: /// <param name="actualExpression">String representation of actual value</param>
575: /// <param name="expected">Expected value</param>
576: /// <param name="actual">Actual value</param>
577: /// <returns>Result object</returns>
578: AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, char const* expected,
579:                                                           char* actual)
580: {
581:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
582: }
583: 
584: /// <summary>
585: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
586: /// </summary>
587: /// <param name="expectedExpression">String representation of expected value</param>
588: /// <param name="actualExpression">String representation of actual value</param>
589: /// <param name="expected">Expected value</param>
590: /// <param name="actual">Actual value</param>
591: /// <returns>Result object</returns>
592: AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const String& expected,
593:                                                        const String& actual)
594: {
595:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
596: }
597: 
598: /// <summary>
599: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
600: /// </summary>
601: /// <param name="expectedExpression">String representation of expected value</param>
602: /// <param name="actualExpression">String representation of actual value</param>
603: /// <param name="expected">Expected value</param>
604: /// <param name="actual">Actual value</param>
605: /// <returns>Result object</returns>
606: AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const String& expected,
607:                                                        const char* actual)
608: {
609:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
610: }
611: 
612: /// <summary>
613: /// Check that strings are equal ignoring case, generate a success object if successful, otherwise a failure object
614: /// </summary>
615: /// <param name="expectedExpression">String representation of expected value</param>
616: /// <param name="actualExpression">String representation of actual value</param>
617: /// <param name="expected">Expected value</param>
618: /// <param name="actual">Actual value</param>
619: /// <returns>Result object</returns>
620: AssertionResult unittest::CheckEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const char* expected,
621:                                                        const String& actual)
622: {
623:     return internal::CheckStringsEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
624: }
625: 
626: /// <summary>
627: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
628: /// </summary>
629: /// <param name="expectedExpression">String representation of expected value</param>
630: /// <param name="actualExpression">String representation of actual value</param>
631: /// <param name="expected">Expected value</param>
632: /// <param name="actual">Actual value</param>
633: /// <returns>Result object</returns>
634: AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const String& expected,
635:                                                           const String& actual)
636: {
637:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
638: }
639: 
640: /// <summary>
641: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
642: /// </summary>
643: /// <param name="expectedExpression">String representation of expected value</param>
644: /// <param name="actualExpression">String representation of actual value</param>
645: /// <param name="expected">Expected value</param>
646: /// <param name="actual">Actual value</param>
647: /// <returns>Result object</returns>
648: AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const String& expected,
649:                                                           const char* actual)
650: {
651:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
652: }
653: 
654: /// <summary>
655: /// Check that strings are not equal ignoring case, generate a success object if successful, otherwise a failure object
656: /// </summary>
657: /// <param name="expectedExpression">String representation of expected value</param>
658: /// <param name="actualExpression">String representation of actual value</param>
659: /// <param name="expected">Expected value</param>
660: /// <param name="actual">Actual value</param>
661: /// <returns>Result object</returns>
662: AssertionResult unittest::CheckNotEqualInternalIgnoreCase(const String& expectedExpression, const String& actualExpression, const char* expected,
663:                                                           const String& actual)
664: {
665:     return internal::CheckStringsNotEqualIgnoreCase(expectedExpression, actualExpression, expected, actual);
666: }
667: 
668: } // namespace unittest
```

- Line 49: We change the namespace usage, to not put the functions inside the namespace directly.
This has to do with doxygen warnings, the `baremetal` namespace is added in the signature of the all functions function, although it is not needed for C++.
- Line 51-62: We define a local function `EqualCaseInsensitive` which compares two strings in a case insensitive way, and returns true if they are considered equal, false otherwise
- Ling 68: We add the namespace qualification to `AssertionSuccess()`
- Ling 78: We add the namespace qualification to `GenericFailure()`
- Ling 90: We add the namespace qualification to `BooleanFailure()`
- Ling 112: We add the namespace qualification to `EqFailure()`
- Ling 139: We add the namespace qualification to `InEqFailure()`
- Line 158-189: We implement the function `CloseFailure()`.
This will return an assertion result flagging a failure, with a string explaining that the absolute difference between the actual value and the expected value is larger than the tolerance.
- Line 193-211: We define a function `CheckStringsEqual()` in a local namespace `internal`.
If the two values of type const char* are considered equal, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 213-231: We define a function `CheckStringsNotEqual()` in a local namespace `internal`.
If the two values of type const char* are considered not equal, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 233-252: We define a function `CheckStringsEqualIgnoreCase()` in a local namespace `internal`.
If the two values of type const char* are considered equal ignoring case, `AssertionSuccess()` is returned, otherwise, `EqFailure()` is returned
- Line 254-273: We define a function `CheckStringsNotEqualIgnoreCase()` in a local namespace `internal`.
If the two values of type const char* are considered not equal ignoring case, `AssertionSuccess()` is returned, otherwise, `InEqFailure()` is returned
- Line 277-289: We implement the function `CheckEqualInternal()` for two value of type const char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 291-303: We implement the function `CheckEqualInternal()` for two value of type char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 305-317: We implement the function `CheckEqualInternal()` for two value of type char* and const char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 319-331: We implement the function `CheckEqualInternal()` for two value of type const char* and char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 333-345: We implement the function `CheckNotEqualInternal()` for two value of type const char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 347-359: We implement the function `CheckNotEqualInternal()` for two value of type char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 361-373: We implement the function `CheckNotEqualInternal()` for two value of type char* and const char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 375-387: We implement the function `CheckNotEqualInternal()` for two value of type const char* and char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 389-401: We implement the function `CheckEqualInternal()` for two value of type string.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 403-415: We implement the function `CheckEqualInternal()` for two value of type string and const char*.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 417-429: We implement the function `CheckEqualInternal()` for two value of type const char* and string.
This uses the function `CheckStringsEqual()` to compare the strings
- Line 431-443: We implement the function `CheckNotEqualInternal()` for two value of type string.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 445-457: We implement the function `CheckNotEqualInternal()` for two value of type string and const char*.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 459-471: We implement the function `CheckNotEqualInternal()` for two value of type const char* and string.
This uses the function `CheckStringsNotEqual()` to compare the strings
- Line 473-485: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type const char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 487-498: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 500-512: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type char* and const char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 514-526: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type const char* and char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 528-540: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type const char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 542-554: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 556-568: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type char* and const char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 570-582: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type const char* and char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 584-596: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type string.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 598-610: We implement the function `CheckEqualInternalIgnoreCase()` for two value of string and const char*.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 612-624: We implement the function `CheckEqualInternalIgnoreCase()` for two value of type const char* and string.
This uses the function `CheckStringsEqualIgnoreCase()` to compare the strings
- Line 626-638: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type string.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 640-652: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type string and const char*.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings
- Line 654-666: We implement the function `CheckNotEqualInternalIgnoreCase()` for two value of type const char* and string.
This uses the function `CheckStringsNotEqualIgnoreCase()` to compare the strings

### PrintValue.h {#TUTORIAL_24_UNIT_TEST_MACROS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_PRINTVALUEH}

The `PrintValue` header will now be extended to support printing pointers and strings.

Update the file `code/libraries/unittest/include/unittest/PrintValue.h`

```cpp
File: code/libraries/unittest/include/unittest/PrintValue.h
...
48: /// @brief Used to denote a nullptr type
49: using nullptr_t = decltype(nullptr);
50: 
51: /// <summary>
52: /// Direct cast to another type
53: /// </summary>
54: /// <typeparam name="To">Type to cast to</typeparam>
55: /// <param name="x">Value to be casted</param>
56: /// <returns>Casted value</returns>
57: template <typename To>
58: inline To ImplicitCast_(To x)
59: {
60:     return x;
61: }
62: 
63: /// <summary>
64: /// Print a value to string using a serializer
65: /// </summary>
66: /// <typeparam name="T">Type of value to print</typeparam>
67: /// <param name="value">Value to print</param>
68: /// <param name="s">Resulting string</param>
69: template <typename T>
70: void PrintTo(const T& value, baremetal::String& s)
71: {
72:     s = baremetal::Serialize(value);
73: }
74: /// <summary>
75: /// Print a unsigned character value to string
76: /// </summary>
77: /// <param name="c">Value to print</param>
78: /// <param name="s">Resulting string</param>
79: void PrintTo(unsigned char c, baremetal::String& s);
80: /// <summary>
81: /// Print a signed character value to string
82: /// </summary>
83: /// <param name="c">Value to print</param>
84: /// <param name="s">Resulting string</param>
85: void PrintTo(signed char c, baremetal::String& s);
86: /// <summary>
87: /// Print a character value to string
88: /// </summary>
89: /// <param name="c">Value to print</param>
90: /// <param name="s">Resulting string</param>
91: inline void PrintTo(char c, baremetal::String& s)
92: {
93:     PrintTo(static_cast<unsigned char>(c), s);
94: }
95: 
96: /// <summary>
97: /// Print a boolean value to string
98: /// </summary>
99: /// <param name="x">Value to print</param>
100: /// <param name="s">Resulting string</param>
101: inline void PrintTo(bool x, baremetal::String& s)
102: {
103:     s = (x ? "true" : "false");
104: }
105: 
106: /// <summary>
107: /// Print a const char* to string
108: /// </summary>
109: /// <param name="str">Value to print</param>
110: /// <param name="s">Resulting string</param>
111: void PrintTo(const char* str, baremetal::String& s);
112: /// <summary>
113: /// Print a char* to string
114: /// </summary>
115: /// <param name="str">Value to print</param>
116: /// <param name="s">Resulting string</param>
117: inline void PrintTo(char* str, baremetal::String& s)
118: {
119:     PrintTo(ImplicitCast_<const char*>(str), s);
120: }
121: 
122: /// <summary>
123: /// Print a signed char* to string
124: ///
125: /// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
126: /// </summary>
127: /// <param name="str">Value to print</param>
128: /// <param name="s">Resulting string</param>
129: inline void PrintTo(const signed char* str, baremetal::String& s)
130: {
131:     PrintTo(ImplicitCast_<const void*>(str), s);
132: }
133: /// <summary>
134: /// Print a signed char* to string
135: ///
136: /// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
137: /// </summary>
138: /// <param name="str">Value to print</param>
139: /// <param name="s">Resulting string</param>
140: inline void PrintTo(signed char* str, baremetal::String& s)
141: {
142:     PrintTo(ImplicitCast_<const void*>(str), s);
143: }
144: /// <summary>
145: /// Print a signed char* to string
146: ///
147: /// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
148: /// </summary>
149: /// <param name="str">Value to print</param>
150: /// <param name="s">Resulting string</param>
151: inline void PrintTo(const unsigned char* str, baremetal::String& s)
152: {
153:     PrintTo(ImplicitCast_<const void*>(str), s);
154: }
155: /// <summary>
156: /// Print a signed char* to string
157: ///
158: /// Signed/unsigned char is often used for representing binary data, so we print pointers to it as void* to be safe.
159: /// </summary>
160: /// <param name="str">Value to print</param>
161: /// <param name="s">Resulting string</param>
162: inline void PrintTo(unsigned char* str, baremetal::String& s)
163: {
164:     PrintTo(ImplicitCast_<const void*>(str), s);
165: }
166: 
167: /// <summary>
168: /// Print a string to string
169: /// </summary>
170: /// <param name="str">Value to print</param>
171: /// <param name="s">Resulting string</param>
172: void PrintStringTo(const baremetal::String& str, baremetal::String& s);
173: /// <summary>
174: /// Print a string to string
175: /// </summary>
176: /// <param name="str">Value to print</param>
177: /// <param name="s">Resulting string</param>
178: inline void PrintTo(const baremetal::String& str, baremetal::String& s)
179: {
180:     PrintStringTo(str, s);
181: }
182: 
183: /// <summary>
184: /// Print a nullptr to string
185: /// </summary>
186: /// <param name="s">Resulting string</param>
187: inline void PrintTo(nullptr_t /*p*/, baremetal::String& s)
188: {
189:     PrintStringTo(baremetal::String("null"), s);
190: }
...
219: /// <summary>
220: /// Universal printer class for reference type, using PrintTo()
221: /// </summary>
222: /// <typeparam name="T">Type value to print</typeparam>
223: template <typename T>
224: class UniversalPrinter<T&>
225: {
226: public:
227:     /// <summary>
228:     /// Print a reference to string
229:     /// </summary>
230:     /// <param name="value">Value to print</param>
231:     /// <param name="s">Resulting string</param>
232:     static void Print(const T& value, baremetal::String& s)
233:     {
234:         // Prints the address of the value.  We use reinterpret_cast here
235:         // as static_cast doesn't compile when T is a function type.
236:         s = "@";
237:         s.append(baremetal::Serialize(reinterpret_cast<const void*>(&value)));
238:         s.append(" ");
239: 
240:         // Then prints the value itself.
241:         PrintTo(value, s);
242:     }
243: };
244: 
245: /// <summary>
246: /// Universal print to string function, uses UniversalPrinter
247: /// </summary>
248: /// <typeparam name="T">Type value to print</typeparam>
249: /// <param name="value">Value to print</param>
250: /// <param name="s">Resulting string</param>
251: template <typename T>
252: void UniversalPrint(const T& value, baremetal::String& s)
253: {
254:     typedef T T1;
255:     UniversalPrinter<T1>::Print(value, s);
256: }
257: 
258: /// <summary>
259: /// Universal terse printer class, uses UniversalPrint
260: /// </summary>
261: /// <typeparam name="T">Type value to print</typeparam>
262: template <typename T>
263: class UniversalTersePrinter
264: {
265: public:
266:     /// <summary>
267:     /// Print a type T to string
268:     /// </summary>
269:     /// <param name="value">Value to print</param>
270:     /// <param name="s">Resulting string</param>
271:     static void Print(const T& value, baremetal::String& s)
272:     {
273:         UniversalPrint(value, s);
274:     }
275: };
276: /// <summary>
277: /// Universal terse printer class for reference, uses UniversalPrint
278: /// </summary>
279: /// <typeparam name="T">Type value to print</typeparam>
280: template <typename T>
281: class UniversalTersePrinter<T&>
282: {
283: public:
284:     /// <summary>
285:     /// Print a reference to string
286:     /// </summary>
287:     /// <param name="value">Value to print</param>
288:     /// <param name="s">Resulting string</param>
289:     static void Print(const T& value, baremetal::String& s)
290:     {
291:         UniversalPrint(value, s);
292:     }
293: };
294: /// <summary>
295: /// Universal terse printer class for const char*, uses UniversalPrint
296: /// </summary>
297: /// <typeparam name="T">Type value to print</typeparam>
298: template <>
299: class UniversalTersePrinter<const char*>
300: {
301: public:
302:     /// <summary>
303:     /// Print a reference to string
304:     /// </summary>
305:     /// <param name="str">Value to print</param>
306:     /// <param name="s">Resulting string</param>
307:     static void Print(const char* str, baremetal::String& s)
308:     {
309:         if (str == nullptr)
310:         {
311:             s = "null";
312:         }
313:         else
314:         {
315:             UniversalPrint(baremetal::String(str), s);
316:         }
317:     }
318: };
319: /// <summary>
320: /// Universal terse printer class for char*, uses UniversalPrint
321: /// </summary>
322: /// <typeparam name="T">Type value to print</typeparam>
323: template <>
324: class UniversalTersePrinter<char*>
325: {
326: public:
327:     /// <summary>
328:     /// Print a reference to string
329:     /// </summary>
330:     /// <param name="str">Value to print</param>
331:     /// <param name="s">Resulting string</param>
332:     static void Print(char* str, baremetal::String& s)
333:     {
334:         UniversalTersePrinter<const char*>::Print(str, s);
335:     }
336: };
337: 
338: /// <summary>
339: /// String print, uses UniversalPrinter
340: /// </summary>
341: /// <typeparam name="T">Type value to print</typeparam>
342: /// <param name="value">Value to print</param>
343: /// <returns>Resulting string</returns>
344: template <typename T>
345: inline baremetal::String PrintToString(const T& value)
346: {
347:     baremetal::String s;
348:     UniversalTersePrinter<T>::Print(value, s);
349:     return s;
350: }
```

- Line 48-49: We define a type `nullptr_t` to check for nullptr
- Line 51-61: We define a template function `ImplicitCast_` that casts any type to itself
- Line 74-79: We declare a variant of `PrintTo()` to print an unsigned char to string
- Line 80-85: We declare a variant of `PrintTo()` to print a signed char to string
- Line 86-94: We define a variant of `PrintTo()` to print a character to string using the function for unsigned char
- Line 106-111: We declare a variant of  `PrintTo()` to print a const char* string to string
- Line 112-120: We define a variant of  `PrintTo()` to print a char* string to string using the print function for const char*
- Line 122-132: We define a variant of  `PrintTo()` to print a const signed char* pointer to string using the print function for const void*
- Line 133-143: We define a variant of  `PrintTo()` to print a signed char* pointer to string using the print function for const void*
- Line 144-154: We define a variant of  `PrintTo()` to print a const unsigned char* pointer to string using the print function for const void*
- Line 155-165: We define a variant of  `PrintTo()` to print a unsigned char* pointer to string using the print function for const void*
- Line 167-172: We declare a function `PrintStringTo()` to print a string to a string
- Line 173-181: We define a variant of  `PrintTo()` to print a string to a string, which uses `PrintStringTo()`
- Line 183-190: We define a variant of  `PrintTo()` to print a nullptr to a string, which uses `PrintStringTo()`
- Line 219-243: We declare a specialization of `UniversalPrinter` for type `T&` that has a single method `Print()` which calls any defined `PrintTo()` function for the value passed to a string, with a prefix to display the address of the string
- Line 294-318: We declare a template class `UniversalTersePrinter` specialization for type `const char*` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string
- Line 319-336: We declare a template class `UniversalTersePrinter` specialization for type `char*` that has a single method `Print()` which calls any defined `UniversalPrint()` function for the value passed to a string

### PrintValue.cpp {#TUTORIAL_24_UNIT_TEST_MACROS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_PRINTVALUECPP}

Some print functions need to be implemented.

Create the file `code/libraries/unittest/src/PrintValue.cpp`

```cpp
File: code/libraries/unittest/src/PrintValue.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
39: #include "unittest/PrintValue.h"
40: 
41: using namespace baremetal;
42: 
43: void PrintStringTo(const baremetal::String& str, baremetal::String& s)
44: {
45:     s = str;
46: }
47: 
48: void PrintTo(unsigned char ch, baremetal::String& s)
49: {
50:     s += ch;
51: }
```

The functions speak for themselves.

### AssertMacros.h {#TUTORIAL_24_UNIT_TEST_MACROS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_ASSERTMACROSH}

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
91: /// @brief Internal namespace
92: namespace internal
93: {
94: 
95: // Two overloaded helpers for checking at compile time whether an
96: // expression is a null pointer literal (i.e. nullptr or any 0-valued
97: // compile-time integral constant).  Their return values have
98: // different sizes, so we can use sizeof() to test which version is
99: // picked by the compiler.  These helpers have no implementations, as
100: // we only need their signatures.
101: //
102: // Given IsNullLiteralHelper(x), the compiler will pick the first
103: // version if x can be implicitly converted to Secret*, and pick the
104: // second version otherwise.  Since Secret is a secret and incomplete
105: // type, the only expression a user can write that has type Secret* is
106: // a null pointer literal.  Therefore, we know that x is a null
107: // pointer literal if and only if the first version is picked by the
108: // compiler.
109: class Secret;
110: /// <summary>
111: /// Conversion check function to check whether argument is a pointer
112: ///
113: /// Not implemented, never called, only declared for return type size
114: /// </summary>
115: /// <param name="p">Argument</param>
116: /// <returns>Unused</returns>
117: char IsNullLiteralHelper(Secret* p);
118: /// <summary>
119: /// Conversion check function to check whether argument is not a pointer
120: ///
121: /// Not implemented, never called, only declared for return type size
122: /// </summary>
123: /// <returns>Unused</returns>
124: char (&IsNullLiteralHelper(...))[2];
125: 
126: } // namespace internal
127: 
128: } // namespace unittest
129: 
130: /// @brief Boolean expression to check whether the argument is a null literal. Returns true if the argument is nullptr, false otherwise
131: #define IS_NULL_LITERAL(x) \
132:      (sizeof(::unittest::internal::IsNullLiteralHelper(x)) == 1)
...
166: /// @brief Expect predicate function with three parameters (CheckClose), generates a failure using UT_EXPECT_RESULT if predicate function returns false
167: #define EXPECT_PRED_FORMAT3(pred_format, v1, v2, v3) \
168:   UT_EXPECT_RESULT(pred_format(baremetal::String(#v1), baremetal::String(#v2), baremetal::String(#v3), v1, v2, v3))
169: /// @brief Expect predicate function with three parameters (CheckClose), generates a failure using UT_ASSERT_RESULT if predicate function returns false
170: #define ASSERT_PRED_FORMAT3(pred_format, v1, v2, v3) \
171:   UT_ASSERT_RESULT(pred_format(baremetal::String(#v1), baremetal::String(#v2), baremetal::String(#v3), v1, v2, v3))
172:
...
201: /// @brief Assert that actual value is equal to expected value
202: #define ASSERT_EQ(expected, actual)                                                                                                                  \
203:     do                                                                                                                                               \
204:     {                                                                                                                                                \
205:         ASSERT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckEqual, expected, actual);                                          \
206:     } while (0)
207: /// @brief Expect that actual value is equal to expected value
208: #define EXPECT_EQ(expected, actual)                                                                                                                  \
209:     do                                                                                                                                               \
210:     {                                                                                                                                                \
211:         EXPECT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckEqual, expected, actual);                                          \
212:     } while (0)
213: 
214: /// @brief Assert that actual value is not equal to expected value
215: #define ASSERT_NE(expected, actual)                                                                                                                  \
216:     do                                                                                                                                               \
217:     {                                                                                                                                                \
218:         ASSERT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckNotEqual, expected, actual);                                       \
219:     } while (0)
220: /// @brief Expect that actual value is not equal to expected value
221: #define EXPECT_NE(expected, actual)                                                                                                                  \
222:     do                                                                                                                                               \
223:     {                                                                                                                                                \
224:         EXPECT_PRED_FORMAT2(::unittest::EqHelper<IS_NULL_LITERAL(expected)>::CheckNotEqual, expected, actual);                                       \
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
- Line 110-124: We declare two functions `IsNullLiteralHelper()`, of which one takes a pointer, and the other takes a variable argument list.
This is some trickery, to determine if the parameter passed is a nullptr. The first returns a single character, the other a pointer to a char array, resulting in different return types and thus sizes
- Line 130-132: We define a macro `IS_NULL_LITERAL` which is used to check if a pointer is a null pointer.
This uses the functions just defined
- Line 166-168: We define a macro `EXPECT_PRED_FORMAT3` which is passed a check function and three parameters.
The check function is called, and the result is passed to `UT_EXPECT_RESULT`.
The three parameter version is used with `CheckClose()` to check the result of comparison with a tolerance
- Line 169-171: We define a macro `ASSERT_PRED_FORMAT3` which is passed a check function and three parameters.
The check function is called, and the result is passed to `UT_ASSERT_RESULT`.
The three parameter version is used with `CheckClose()` to check the result of comparison with a tolerance
- Line 201-206: We change the macro `ASSERT_EQ` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 207-212: We change the macro `EXPECT_EQ` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 214-219: We change the macro `ASSERT_NE` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 220-225: We change the macro `EXPECT_NE` to use the `EqHelper` class depending on whether the argument is a nullptr or not
- Line 227-232: We define a macro `ASSERT_EQ_IGNORE_CASE` which uses `ASSERT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckEqualIgnoreCase` to check if the parameters are not equal, and generate a `EqFailure` if the check fails
- Line 233-238: We define a macro `EXPECT_EQ_IGNORE_CASE` which uses `EXPECT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckEqualIgnoreCase` to check if the parameters are not equal, and generate a `EqFailure` if the check fails
- Line 240-245: We define a macro `ASSERT_NE_IGNORE_CASE` which uses `ASSERT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase` to check if the parameters are not equal, and generate a `InEqFailure` if the check fails
- Line 246-251: We define a macro `EXPECT_NE_IGNORE_CASE` which uses `EXPECT_PRED_FORMAT2` with `EqHelperStringCaseInsensitive::CheckNotEqualIgnoreCase` to check if the parameters are not equal, and generate a `InEqFailure` if the check fails
- Line 253-258: We define a macro `ASSERT_NEAR` which uses `ASSERT_PRED_FORMAT3` with `CheckClose` to check if the absolute difference of compared values is within tolerance, and generate a `CloseFailure` if the check fails
- Line 259-264: We define a macro `EXPECT_NEAR` which uses `EXPECT_PRED_FORMAT3` with `CheckClose` to check if the absolute difference of compared values is within tolerance, and generate a `CloseFailure` if the check fails
- Line 266-267: We define a macro `ASSERT_NULL` to check if the parameter is a null pointer, and generate a `EqFailure` if the check fails
- Line 268-269: We define a macro `EXPECT_NULL` to check if the parameter is a null pointer, and generate a `EqFailure` if the check fails
- Line 270-271: We define a macro `ASSERT_NOT_NULL` to check if the parameter is not a null pointer, and generate a `InEqFailure` if the check fails
- Line 272-273: We define a macro `EXPECT_NOT_NULL` to check if the parameter is not a null pointer, and generate a `InEqFailure` if the check fails

### Application code {#TUTORIAL_24_UNIT_TEST_MACROS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_APPLICATION_CODE}

We'll use `Test` to use the new macros defined.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
...
86: TEST(MyTest4)
87: {
88:     int* p = nullptr;
89:     int dd = 123;
90:     int* q = &dd;
91:     ASSERT_NULL(p);
92:     EXPECT_NULL(p);
93:     ASSERT_NULL(q);
94:     EXPECT_NULL(q);
95:     ASSERT_NOT_NULL(p);
96:     EXPECT_NOT_NULL(p);
97:     ASSERT_NOT_NULL(q);
98:     EXPECT_NOT_NULL(q);
99:     baremetal::String s1 = "A";
100:     baremetal::String s2 = "B";
101:     baremetal::String s3 = "B";
102:     baremetal::String s4 = "b";
103:     ASSERT_EQ(s1, s2);
104:     EXPECT_EQ(s1, s2);
105:     ASSERT_EQ(s2, s3);
106:     EXPECT_EQ(s2, s3);
107:     ASSERT_NE(s1, s2);
108:     EXPECT_NE(s1, s2);
109:     ASSERT_NE(s2, s3);
110:     EXPECT_NE(s2, s3);
111:     ASSERT_EQ_IGNORE_CASE(s1, s2);
112:     EXPECT_EQ_IGNORE_CASE(s1, s2);
113:     ASSERT_EQ_IGNORE_CASE(s2, s3);
114:     EXPECT_EQ_IGNORE_CASE(s2, s3);
115:     ASSERT_NE_IGNORE_CASE(s1, s2);
116:     EXPECT_NE_IGNORE_CASE(s1, s2);
117:     ASSERT_NE_IGNORE_CASE(s2, s3);
118:     EXPECT_NE_IGNORE_CASE(s2, s3);
119:     ASSERT_EQ_IGNORE_CASE(s2, s4);
120:     EXPECT_EQ_IGNORE_CASE(s2, s4);
121:     ASSERT_NE_IGNORE_CASE(s2, s4);
122:     EXPECT_NE_IGNORE_CASE(s2, s4);
123:     char t[] = {'A', '\0'};
124:     char u[] = {'B', '\0'};
125:     char v[] = {'B', '\0'};
126:     char w[] = {'b', '\0'};
127:     const char* uC = "B";
128:     const char* vC = "B";
129:     const char* wC = "b";
130:     ASSERT_EQ(t, u);
131:     EXPECT_EQ(t, u);
132:     ASSERT_EQ(u, v);
133:     EXPECT_EQ(u, v);
134:     ASSERT_EQ(t, u);
135:     EXPECT_EQ(t, uC);
136:     ASSERT_EQ(uC, v);
137:     EXPECT_EQ(uC, vC);
138:     ASSERT_EQ(t, w);
139:     EXPECT_EQ(t, wC);
140:     ASSERT_EQ(uC, w);
141:     EXPECT_EQ(uC, wC);
142:     ASSERT_NE(t, u);
143:     EXPECT_NE(t, u);
144:     ASSERT_NE(u, v);
145:     EXPECT_NE(u, v);
146:     ASSERT_NE(t, u);
147:     EXPECT_NE(t, uC);
148:     ASSERT_NE(uC, v);
149:     EXPECT_NE(uC, vC);
150:     ASSERT_NE(t, w);
151:     EXPECT_NE(t, wC);
152:     ASSERT_NE(uC, w);
153:     EXPECT_NE(uC, wC);
154:     ASSERT_EQ_IGNORE_CASE(t, u);
155:     EXPECT_EQ_IGNORE_CASE(t, u);
156:     ASSERT_EQ_IGNORE_CASE(u, v);
157:     EXPECT_EQ_IGNORE_CASE(u, v);
158:     ASSERT_EQ_IGNORE_CASE(t, u);
159:     EXPECT_EQ_IGNORE_CASE(t, uC);
160:     ASSERT_EQ_IGNORE_CASE(uC, v);
161:     EXPECT_EQ_IGNORE_CASE(uC, vC);
162:     ASSERT_EQ_IGNORE_CASE(t, w);
163:     EXPECT_EQ_IGNORE_CASE(t, wC);
164:     ASSERT_EQ_IGNORE_CASE(uC, w);
165:     EXPECT_EQ_IGNORE_CASE(uC, wC);
166:     ASSERT_NE_IGNORE_CASE(t, u);
167:     EXPECT_NE_IGNORE_CASE(t, u);
168:     ASSERT_NE_IGNORE_CASE(u, v);
169:     EXPECT_NE_IGNORE_CASE(u, v);
170:     ASSERT_NE_IGNORE_CASE(t, u);
171:     EXPECT_NE_IGNORE_CASE(t, uC);
172:     ASSERT_NE_IGNORE_CASE(uC, v);
173:     EXPECT_NE_IGNORE_CASE(uC, vC);
174:     ASSERT_NE_IGNORE_CASE(t, w);
175:     EXPECT_NE_IGNORE_CASE(t, wC);
176:     ASSERT_NE_IGNORE_CASE(uC, w);
177:     EXPECT_NE_IGNORE_CASE(uC, wC);
178: 
179:     double a = 0.123;
180:     double b = 0.122;
181:     ASSERT_EQ(a, b);
182:     EXPECT_EQ(a, b);
183:     ASSERT_NEAR(a, b, 0.0001);
184:     EXPECT_NEAR(a, b, 0.0001);
185:     ASSERT_NEAR(a, b, 0.001);
186:     EXPECT_NEAR(a, b, 0.001);
187: }
...
```

You may recognize some of these tests from the `String` we wrote before in [Tutorial 16: String](#TUTORIAL_16_STRING).
However here we purposely write some failing tests to test the unit test code.

### Configuring, building and debugging {#TUTORIAL_24_UNIT_TEST_MACROS_TEST_ASSERT_MACRO_EXTENSION___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

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

## Writing class tests for string - Step 4 {#TUTORIAL_24_UNIT_TEST_MACROS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4}

Now that our unit test framework is complete, let's reimplement the tests we created in [Tutorial 16: String](#TUTORIAL_16_STRING).
First we'll create a folder `test` underneath `code/libraries/baremetal`, and underneath it a folder `src`.
We'll also be creating a CMake file for the tests, as well as the structure for the main application to create a kernel image.

<img src="images/unittest-add-test-project.png" alt="Tree view" width="400"/>

### main.cpp {#TUTORIAL_24_UNIT_TEST_MACROS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_MAINCPP}

First we'll create the main source file that will run the tests.

Create the file `code\libraries\baremetal\test\main.cpp`

```cpp
File: code\libraries\baremetal\test\main.cpp
1: #include "baremetal/System.h"
2: #include "unittest/unittest.h"
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

### StringTest.cpp {#TUTORIAL_24_UNIT_TEST_MACROS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_STRINGTESTCPP}

We'll add the source file containing the string tests.

As this file is quite sizeable, we'll not repeat the full source here, just an excerpt.

Create the file `code\libraries\baremetal\test\StringTest.cpp`

```cpp
File: code\libraries\baremetal\test\StringTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : StringTest.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : StringTest
9: //
10: // Description : String class tests
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
42: #include "baremetal/String.h"
43: #include "stdlib/Util.h"
44: 
45: using namespace unittest;
46: 
47: namespace baremetal {
48: namespace test {
49: 
50: /// @brief Minimum string allocation size
51: static constexpr size_t MinimumAllocationSize = 256;
52: 
53: /// @brief Baremetal test suite
54: TEST_SUITE(Baremetal)
55: {
56: 
57:     class StringTest : public TestFixture
58:     {
59:     public:
60:         const char* otherText = "abcdefghijklmnopqrstuvwxyz";
61:         String other;
62:         void SetUp() override
63:         {
64:             other = otherText;
65:         }
66:         void TearDown() override
67:         {
68:         }
69:     };
70: 
71:     TEST_FIXTURE(StringTest, ConstructDefault)
72:     {
73:         String s;
74:         EXPECT_TRUE(s.empty());
75:         ASSERT_NOT_NULL(s.data());
76:         ASSERT_NOT_NULL(s.c_str());
77:         EXPECT_EQ('\0', s.data()[0]);
78:         EXPECT_EQ(size_t{0}, s.size());
79:         EXPECT_EQ(size_t{0}, s.length());
80:         EXPECT_EQ(size_t{0}, s.capacity());
81:     }
82: 
83:     TEST_FIXTURE(StringTest, ConstructConstCharPtr)
84:     {
85:         const char* text = otherText;
86:         const char* expected = otherText;
87:         size_t expectedLength = strlen(expected);
88:         size_t length = strlen(expected);
89: 
90:         String s(text);
91: 
92:         EXPECT_FALSE(s.empty());
93:         ASSERT_NOT_NULL(s.data());
94:         ASSERT_NOT_NULL(s.c_str());
95:         EXPECT_NE('\0', s.data()[0]);
96:         EXPECT_EQ(expectedLength, s.size());
97:         EXPECT_EQ(expectedLength, s.length());
98:         EXPECT_NE(size_t{0}, s.capacity());
99:         EXPECT_TRUE(expected == s);
100:         EXPECT_TRUE(s == expected);
101:         EXPECT_EQ(expected, s);
102:         EXPECT_EQ(s, expected);
103:     }
104: 
105:     TEST_FIXTURE(StringTest, ConstructConstCharPtrEmpty)
106:     {
107:         const char* text = "";
108:         const char* expected = "";
109:         size_t expectedLength = strlen(expected);
110: 
111:         String s(text);
112: 
113:         EXPECT_TRUE(s.empty());
114:         ASSERT_NOT_NULL(s.data());
115:         ASSERT_NOT_NULL(s.c_str());
116:         EXPECT_EQ('\0', s.data()[0]);
117:         EXPECT_EQ(expectedLength, s.size());
118:         EXPECT_EQ(expectedLength, s.length());
119:         EXPECT_NE(size_t{0}, s.capacity());
120:         EXPECT_TRUE(expected == s);
121:         EXPECT_TRUE(s == expected);
122:         EXPECT_EQ(expected, s);
123:         EXPECT_EQ(s, expected);
124:     }
125: 
126:     TEST_FIXTURE(StringTest, ConstructNullPtr)
127:     {
128:         const char* expected = "";
129:         size_t expectedLength = strlen(expected);
130: 
131:         String s(nullptr);
132: 
133:         EXPECT_TRUE(s.empty());
134:         ASSERT_NOT_NULL(s.data());
135:         ASSERT_NOT_NULL(s.c_str());
136:         EXPECT_EQ('\0', s.data()[0]);
137:         EXPECT_EQ(expectedLength, s.size());
138:         EXPECT_EQ(expectedLength, s.length());
139:         EXPECT_EQ(size_t{0}, s.capacity());
140:         EXPECT_TRUE(expected == s);
141:         EXPECT_TRUE(s == expected);
142:         EXPECT_EQ(expected, s);
143:         EXPECT_EQ(s, expected);
144:     }
...
```

- Line 50-51: We define the suite for these class tests, which is named after the library, so `Baremetal` (using the same casing `baremetal` would result in compiler errors, as the compiler will confuse the two namespaces)
- Line 53-69: We define the class for the test fixture. Notice that we collect some often used variables in the class, and initialize using the `SetUp()` method
- Line 71-81: As an example, we test the default constructor
- Line 83-103: We test the constructor taking const char*
- Line 105-124: We test the constructor taking const char* for the corner case that an empty string is passed
- Line 126-144: We test the constructor taking const char* for the corner case that a nullptr is passed

The rest of the tests are included in the tutorial code, but we won't dive deeper into this.
Tests for Serialization (see [Tutorial 17: Serialization and formatting](#TUTORIAL_17_SERIALIZATION_AND_FORMATTING)) are also included, again not discussed here.

### Set up test project configuration {#TUTORIAL_24_UNIT_TEST_MACROS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_SET_UP_TEST_PROJECT_CONFIGURATION}

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
35: file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp src/*.S)
36: set(GLOB_RECURSE PROJECT_INCLUDES_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/*.h)
37: set(PROJECT_INCLUDES_PRIVATE )
38: 
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
109: set(BAREMETAL_EXECUTABLE_TARGET ${PROJECT_NAME})
110: setup_image(${PROJECT_NAME})
```

### Update baremetal project configuration {#TUTORIAL_24_UNIT_TEST_MACROS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_UPDATE_BAREMETAL_PROJECT_CONFIGURATION}

We need to add the test project CMake file to the baremetal project CMake file in order to build the test project.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/test/CMakeLists.txt
95: show_target_properties(${PROJECT_NAME})
96: 
97: add_subdirectory(test)
```

### Configuring, building and debugging {#TUTORIAL_24_UNIT_TEST_MACROS_WRITING_CLASS_TESTS_FOR_STRING___STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. All tests should succeed.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.000 Starting up (System:213)
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
Info   0.00:00:00.630 Halt (System:121)
```

As you can see, we have 156 test for the `String` class and 18 for serialization, which actually have a multitude of this in test cases.

Next: [25-writing-unit-tests](25-writing-unit-tests.md)
