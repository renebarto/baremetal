# Tutorial 19: Timer extension {#TUTORIAL_19_TIMER_EXTENSION}

@tableofcontents

## New tutorial setup {#TUTORIAL_19_TIMER_EXTENSION_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/18-timer-extension`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_19_TIMER_EXTENSION_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-19.a`
- a library `output/Debug/lib/stdlib-19.a`
- an application `output/Debug/bin/19-timer-extension.elf`
- an image in `deploy/Debug/19-timer-extension-image`

## Extending the timer {#TUTORIAL_19_TIMER_EXTENSION_EXTENDING_THE_TIMER}

In tutorials `08-timer` we created the `Timer` class. However, the functionality was limited to delays. 
We would like to have timers which are updated automatically on interrupt basis.

So we'll be extending the `Timer` class for this.
We will start by first creating an interrupt handler, which counts ticks.
Then we'll introduce `KernelTimer`, which can be used to trigger after a certain amount of time.

## Adding interrupts to the timer - Step 1 {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1}

We'll add some functionality to the `Timer` class for enabling, handling and disabling interrupts on the timer.

### Timer.h {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_TIMERH}

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

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
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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
59: 
60:     const baremetal::string& SuiteName() const { return m_suiteName; }
61:     const baremetal::string& FixtureName() const { return m_fixtureName; }
62:     const baremetal::string& TestName() const { return m_testName; }
63:     const baremetal::string& SourceFileName() const { return m_fileName; }
64:     int SourceFileLineNumber() const { return m_lineNumber; }
65: };
66: 
67: } // namespace unittest
```

The `TestDetails` class is added to the `unittest` namespace.

- Line 42: We use strings, so we need to include the header for the `string` class
- Line 47-65: We declare the class `TestDetails` which will hold information on a test
  - Line 50: The class member variable `m_suiteName` is the test suite name
  - Line 51: The class member variable `m_fixtureName` is the test fixture name
  - Line 52: The class member variable `m_testName` is the test name
  - Line 53: The class member variable `m_fileName` is the source file in which the actual test is defined
  - Line 54: The class member variable `m_lineNumber` is the source line in which the actual test is defined
  - Line 57: We declare the default constructor
  - Line 58: We declare the normal constructor which specifies all the needed information
  - Line 60: We declare an accessor `SuiteName()` for the test suite name
  - Line 61: We declare an accessor `FixtureName()` for the test fixture name
  - Line 62: We declare an accessor `TestName()` for the test name
  - Line 63: We declare an accessor `SourceFileName()` for the source file name
  - Line 64: We declare an accessor `SourceFileLineNumber()` for the source line number

### TestDetails.cpp {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_TESTDETAILSCPP}

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
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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

### TestBase.h {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_TESTBASEH}

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
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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

### TestBase.cpp {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_TESTBASECPP}

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
16: // Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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

### Update CMake file {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_UPDATE_CMAKE_FILE}

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

### Update application code {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_UPDATE_APPLICATION_CODE}

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

### Configuring, building and debugging {#TUTORIAL_19_TIMER_EXTENSION_ADDING_INTERRUPTS_TO_THE_TIMER___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

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

Next: [19-timer-extension](19-timer-extension.md)


