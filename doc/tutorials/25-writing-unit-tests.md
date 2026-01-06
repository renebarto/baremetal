# Tutorial 25: Writing unit tests {#TUTORIAL_25_WRITING_UNIT_TESTS}

@tableofcontents

## New tutorial setup {#TUTORIAL_25_WRITING_UNIT_TESTS_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/25-writing-unit-tests`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_25_WRITING_UNIT_TESTS_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-25.a`
- a library `output/Debug/lib/device-25.a`
- a library `output/Debug/lib/stdlib-25.a`
- a library `output/Debug/lib/unittest-25.a`
- an application `output/Debug/bin/25-writing-unit-tests.elf`
- an image in `deploy/Debug/25-writing-unit-tests-image`

## Creating actual unit tests - Step 1 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_1}

In the previous tutorial we added tests for the `String` class and for serialization.
Now let's move towards testing the MCP23017 device we introduced in [Using MCP23017 - Step 2](#TUTORIAL_22_I2C_USING_MCP23017___STEP_2).
The image below shows the class diagram for MCP23017 and related classes.
As you can see we introduced quite some interface classes:
- IMemoryAccess
- IGPIOPin
- II2CMaster

<img src="images/MCP23017.svg"  alt="MCP 23017 class diagram" width="800"/>

We'll start making unit tests for each of the classes, starting with the GPIO pin, and ultimately arriving at MCP23017.

We'll start with a test for `PhysicalGPIOPin`.
See the image below.

<img src="images/MemoryAccessMockTest.svg"  alt="MemoryAccessMockTest class diagram" width="800"/>

### MemoryAccessMock.h {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_1_MEMORYACCESSMOCKH}

We'll first create the `MemoryAccessMock` class which will simply print on read and write to console operations.
This is a temporary class, which will be replaced with a more dedicated mock class later on.

Create the file `code\libraries\baremetal\test\include\MemoryAccessMock.h`

```cpp
File: code\libraries\baremetal\test\include\MemoryAccessMock.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessMock.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessMock
9: //
10: // Description : Memory read/write mock class
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
42: #include "baremetal/IMemoryAccess.h"
43: 
44: /// @file
45: /// Memory access mock class
46: 
47: namespace baremetal {
48: 
49: /// <summary>
50: /// Memory access mock class
51: /// </summary>
52: class MemoryAccessMock : public IMemoryAccess
53: {
54: public:
55:     uint8 Read8(regaddr address) override;
56:     void Write8(regaddr address, uint8 data) override;
57: 
58:     uint16 Read16(regaddr address) override;
59:     void Write16(regaddr address, uint16 data) override;
60: 
61:     uint32 Read32(regaddr address) override;
62:     void Write32(regaddr address, uint32 data) override;
63: };
64: 
65: } // namespace baremetal
```

### MemoryAccessMock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_1_MEMORYACCESSMOCKCPP}

We'll implement the `MemoryAccessMock` class.

Create the file `code\libraries\baremetal\test\src\MemoryAccessMock.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessMock.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessMock.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessMock
9: //
10: // Description : Memory read/write mock class
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
40: #include "baremetal/Logger.h"
41: #include "MemoryAccessMock.h"
42: 
43: /// @file
44: /// Memory access mock class implementation
45: 
46: /// @brief Define log name
47: LOG_MODULE("MemoryAccessMock");
48: 
49: using namespace baremetal;
50: 
51: /// <summary>
52: /// Read a 8 bit value from register at address
53: /// </summary>
54: /// <param name="address">Address of register</param>
55: /// <returns>8 bit register value</returns>
56: uint8 MemoryAccessMock::Read8(regaddr address)
57: {
58:     TRACE_INFO("Read8(%p)", address);
59:     return {};
60: }
61: 
62: /// <summary>
63: /// Write a 8 bit value to register at address
64: /// </summary>
65: /// <param name="address">Address of register</param>
66: /// <param name="data">Data to write</param>
67: void MemoryAccessMock::Write8(regaddr address, uint8 data)
68: {
69:     TRACE_INFO("Write8(%p, %02x)", address, data);
70: }
71: 
72: /// <summary>
73: /// Read a 16 bit value from register at address
74: /// </summary>
75: /// <param name="address">Address of register</param>
76: /// <returns>16 bit register value</returns>
77: uint16 MemoryAccessMock::Read16(regaddr address)
78: {
79:     TRACE_INFO("Read16(%p)", address);
80:     return {};
81: }
82: 
83: /// <summary>
84: /// Write a 16 bit value to register at address
85: /// </summary>
86: /// <param name="address">Address of register</param>
87: /// <param name="data">Data to write</param>
88: void MemoryAccessMock::Write16(regaddr address, uint16 data)
89: {
90:     TRACE_INFO("Write16(%p, %04x)", address, data);
91: }
92: 
93: /// <summary>
94: /// Read a 32 bit value from register at address
95: /// </summary>
96: /// <param name="address">Address of register</param>
97: /// <returns>32 bit register value</returns>
98: uint32 MemoryAccessMock::Read32(regaddr address)
99: {
100:     TRACE_INFO("Read32(%p)", address);
101:     return {};
102: }
103: 
104: /// <summary>
105: /// Write a 32 bit value to register at address
106: /// </summary>
107: /// <param name="address">Address of register</param>
108: /// <param name="data">Data to write</param>
109: void MemoryAccessMock::Write32(regaddr address, uint32 data)
110: {
111:     TRACE_INFO("Write32(%p, %08x)", address, data);
112: }
```

As you can see, we simply trace the calls for now.

### MemoryAccessMockTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_1_MEMORYACCESSMOCKTESTCPP}

Let's create a first test.

Create the file `code\libraries\baremetal\test\src\MemoryAccessMockTest.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessMockTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessMockTest.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessMockTest
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
40: #include "baremetal/PhysicalGPIOPin.h"
41: 
42: #include "MemoryAccessMock.h"
43: 
44: #include "unittest/unittest.h"
45: 
46: using namespace unittest;
47: 
48: namespace baremetal {
49: namespace test {
50: 
51: /// @brief Baremetal test suite
52: TEST_SUITE(Baremetal)
53: {
54: 
55: class MemoryAccessMockTest : public TestFixture
56: {
57: public:
58:     void SetUp() override
59:     {
60:     }
61:     void TearDown() override
62:     {
63:     }
64: };
65: 
66: TEST_FIXTURE(MemoryAccessMockTest, SimpleTest)
67: {
68:     MemoryAccessMock memoryAccess;
69:     PhysicalGPIOPin pin(memoryAccess);
70: 
71:     pin.AssignPin(0);
72:     pin.SetMode(GPIOMode::AlternateFunction0);
73: }
74: 
75: } // suite Baremetal
76: 
77: } // namespace test
78: } // namespace baremetal
```

- Line 66-73: We create a test `SimpleTest`
  - Line 68: We create a `MemoryAccessMock` instance
  - Line 69: We create a `PhysicalGPIOPin` instance and inject the `MemoryAccessMock` instance
  - Line 71-72: We initialize the GPIO pin

### Run selected tests {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_1_RUN_SELECTED_TESTS}

As the total amount of tests in the `Baremetal` test suite is growing quite large, for now let's limit the tests run.

Update the file `code\libraries\baremetal\test\src\main.cpp`

```cpp
File: code\libraries\baremetal\test\src\main.cpp
1: #include "baremetal/System.h"
2: #include "unittest/unittest.h"
3: 
4: using namespace baremetal;
5: using namespace unittest;
6: 
7: int main()
8: {
9:     ConsoleTestReporter reporter;
10:     RunSelectedTests(&reporter, InSelection("Baremetal", "MemoryAccessMockTest", nullptr));
11: //    RunAllTests(&reporter);
12: 
13:     return static_cast<int>(ReturnCode::ExitHalt);
14: }
```

- Line 10: Instead of `RunAllTests` we use `RunSelectedTests`, where we use the predicate `InSelection` using the test suite `Baremetal` and the test fixture `MemoryAccessMockTest`

### Update the CMake file {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_1_UPDATE_THE_CMAKE_FILE}

We'll update the CMake file for the test project, so that we can include the `MemoryAccessMock` header easily.

Create the file `code/libraries/baremetal/test/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/test/CMakeLists.txt
14: set(PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE ${COMPILE_DEFINITIONS_C})
15: set(PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC )
16: set(PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE ${COMPILE_DEFINITIONS_ASM})
17: set(PROJECT_COMPILE_OPTIONS_CXX_PRIVATE ${COMPILE_OPTIONS_CXX})
18: set(PROJECT_COMPILE_OPTIONS_CXX_PUBLIC )
19: set(PROJECT_COMPILE_OPTIONS_ASM_PRIVATE ${COMPILE_OPTIONS_ASM})
20: set(PROJECT_INCLUDE_DIRS_PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
21: set(PROJECT_INCLUDE_DIRS_PUBLIC )
...
```

- Line 20: We set the private include directories to include the `include` directory of the test project.

### Configuring, building and debugging {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. All tests should succeed.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.000 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (3 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
Info   0.00:00:00.050 Read32(FE2000E4) (MemoryAccessMock.cpp:100)
Info   0.00:00:00.060 Write32(FE2000E4, 00000000) (MemoryAccessMock.cpp:111)
Info   0.00:00:00.060 Read32(FE200000) (MemoryAccessMock.cpp:100)
Info   0.00:00:00.060 Write32(FE200000, 00000004) (MemoryAccessMock.cpp:111)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SerializeChar
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 3 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.080 Halt (System:121)
```

As you can see, we now have only 1 test to run.

The test shows a read for address 0xFE2000E4 (`RPI_GPIO_GPPUPPDN0_OFFSET`), followed by a write to the same adress with value 0x00000000 (meaning no pull up/down for pin 0).
Next, there is a read for address 0xFE200000 (`RPI_GPIO_GPFSEL0`), followed by a write to the same address with value 0x00000004 (meaning setting pin 0 to alternate function 0, which makes it the SDA pin for I2C0).
This all is specific for Raspberry Pi 4, for Raspberry Pi 3 this is slightly different.

Now this is all nice, but this way we cannot test whether the results is what we expect. We could check for console output, but that is quite cumbersome.
A simple first step is keeping an array of accesses to memory, so we can check that the correct steps happen in the correct order.

## Creating actual unit tests - Step 2 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_2}

Let's update `MemoryAccessMock` to add an array of access operations, which is then updated when memory is accessed.

### MemoryAccessMock.h {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_2_MEMORYACCESSMOCKH}

Update the file `code\libraries\baremetal\test\include\MemoryAccessMock.h`

```cpp
File: code\libraries\baremetal\test\include\MemoryAccessMock.h
...
40: #pragma once
41: 
42: #include "stdlib/Macros.h"
43: #include "baremetal/IMemoryAccess.h"
44: #include "baremetal/String.h"
45: 
46: /// @file
47: /// Memory access mock class
48: 
49: namespace baremetal {
50: 
51: /// <summary>
52: /// Data structure to contain a memory access operation
53: /// </summary>
54: struct MemoryAccessOperation
55: {
56:     /// @brief True if write operation, false if read
57:     bool isWriteOperation;
58:     /// @brief Address read from or written to
59:     regaddr address;
60:     /// @brief On read, the data returned, on write the data written
61:     uint32 data;
62: 
63:     /// <summary>
64:     /// Default constructor
65:     /// </summary>
66:     MemoryAccessOperation()
67:         : isWriteOperation{}
68:         , address{}
69:         , data{}
70:     {
71:     }
72:     /// <summary>
73:     /// Constructor for write operations, with data
74:     /// </summary>
75:     /// <param name="theAddress">Addres to read from / write to</param>
76:     /// <param name="theData">Data to write</param>
77:     /// <param name="theIsWriteOperation">True for write operation, false for read</param>
78:     MemoryAccessOperation(regaddr theAddress, uint32 theData = {}, bool theIsWriteOperation = false)
79:         : isWriteOperation{theIsWriteOperation}
80:         , address{theAddress}
81:         , data{theData}
82:     {
83:     }
84:     /// <summary>
85:     /// Check memory access operations for equality
86:     /// </summary>
87:     /// <param name="other">Value to compare to</param>
88:     /// <returns>True if equal, false otherwise</returns>
89:     bool operator == (const MemoryAccessOperation &other) const
90:     {
91:         return (other.isWriteOperation == isWriteOperation) &&
92:             (other.address == address) &&
93:             (other.data == data);
94:     }
95:     /// <summary>
96:     /// Check memory access operations for inequality
97:     /// </summary>
98:     /// <param name="other">Value to compare to</param>
99:     /// <returns>True if unequal, false otherwise</returns>
100:     bool operator != (const MemoryAccessOperation &other) const
101:     {
102:         return !operator==(other);
103:     }
104: } ALIGN(8);
105: 
106: String Serialize(const MemoryAccessOperation& value);
107: 
108: /// <summary>
109: /// Memory access mock class
110: /// </summary>
111: class MemoryAccessMock : public IMemoryAccess
112: {
113: private:
114:     /// @brief Size of memory access operation array
115:     static constexpr size_t BufferSize = 1000;
116:     /// @brief List of memory access operations
117:     MemoryAccessOperation m_accessOps[BufferSize] ALIGN(8);
118:     /// @brief Number of registered memory access operations
119:     size_t m_numAccessOps;
120: 
121: public:
122:     MemoryAccessMock();
123: 
124:     size_t GetNumOperations() const;
125:     const MemoryAccessOperation& GetMemoryOperation(size_t index) const;
126: 
127:     uint8 Read8(regaddr address) override;
128:     void Write8(regaddr address, uint8 data) override;
129: 
130:     uint16 Read16(regaddr address) override;
131:     void Write16(regaddr address, uint16 data) override;
132: 
133:     uint32 Read32(regaddr address) override;
134:     void Write32(regaddr address, uint32 data) override;
135: 
136: private:
137:     void AddOperation(const MemoryAccessOperation& operation);
138: };
139: 
140: } // namespace baremetal
```

- Line 51-104: We declare and implement a struct `MemoryAccessOperation` which contains data on a memory access operation. Note we align to 8 bytes
  - Line 56-57: The member variable `isWriteOperation` is used to distinguish between write and read operations
  - Line 58-59: The member variable `address` contains the address read from or written to
  - Line 60-61: The member variable `data` contains the data written for write operations, the data read for read operations
  - Line 63-71: We declare and implement the default constructor
  - Line 72-83: We declare and implement the non-default constructor (which defaults to a read operation)
  - Line 84-94: We declare and implement the equality operator
  - Line 95-103: We declare and implement the inequality operator
- Line 106: We declare the serialization function for `MemoryAccessOperation`, which is needed when comparing in a test
- Line 108-138: We update the `MemoryAccessMock` class
  - Line 114-115: We add a constant to indicate the size of the array holding the memory access operations
  - Line 116-117: We declare the array `m_accessOps` holding the memory access operations. Note we align to 8 bytes
  - Line 118-119: We declare the member variable `m_numAccessOps` which keeps track of the actual number of registered operations
  - Line 122: We declare the default constructor
  - Line 124: We declare a method `GetNumOperations()` to retrieve the actual number of registered operations
  - Line 125: We declare a method `GetMemoryOperation()` to retrieve a specific operation by index
  - Line 137: We declare a method to add an operation to the array (if it fits)

### MemoryAccessMock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_2_MEMORYACCESSMOCKCPP}

We'll update the `MemoryAccessMock` class.

Update the file `code\libraries\baremetal\test\src\MemoryAccessMock.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessMock.cpp
...
40: #include "MemoryAccessMock.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Format.h"
44: #include "baremetal/Logger.h"
45: 
46: /// @file
47: /// Memory access mock class implementation
48: 
49: /// @brief Define log name
50: LOG_MODULE("MemoryAccessMock");
51: 
52: using namespace baremetal;
53: 
54: /// <summary>
55: /// Serialize a memory access operation to string
56: /// </summary>
57: /// <param name="value">Value to be serialized</param>
58: /// <returns>Resulting string</returns>
59: String baremetal::Serialize(const MemoryAccessOperation &value)
60: {
61:     if (value.isWriteOperation)
62:         return Format("W: %p %08lx", value.address, value.data);
63:     return Format("R: %p", value.address);
64: }
65: 
66: /// <summary>
67: /// Default constructor
68: /// </summary>
69: MemoryAccessMock::MemoryAccessMock()
70:     : m_accessOps{}
71:     , m_numAccessOps{}
72: {
73: }
74: 
75: /// <summary>
76: /// Return number of registered memory access operations
77: /// </summary>
78: /// <returns>Number of registered memory access operations</returns>
79: size_t MemoryAccessMock::GetNumOperations() const
80: {
81:     return m_numAccessOps;
82: }
83: 
84: /// <summary>
85: /// Retrieve a registered memory access operation from the list
86: /// </summary>
87: /// <param name="index">Index of operation</param>
88: /// <returns>Requested memory access operation</returns>
89: const MemoryAccessOperation &MemoryAccessMock::GetMemoryOperation(size_t index) const
90: {
91:     assert(index < m_numAccessOps);
92:     return m_accessOps[index];
93: }
94: 
95: /// <summary>
96: /// Read a 8 bit value from register at address
97: /// </summary>
98: /// <param name="address">Address of register</param>
99: /// <returns>8 bit register value</returns>
100: uint8 MemoryAccessMock::Read8(regaddr address)
101: {
102:     AddOperation({ address });
103:     TRACE_INFO("Read8(%p)", address);
104:     return {};
105: }
106: 
107: /// <summary>
108: /// Write a 8 bit value to register at address
109: /// </summary>
110: /// <param name="address">Address of register</param>
111: /// <param name="data">Data to write</param>
112: void MemoryAccessMock::Write8(regaddr address, uint8 data)
113: {
114:     AddOperation({ address, static_cast<uint32>(data), true });
115:     TRACE_INFO("Write8(%p, %02x)", address, data);
116: }
117: 
118: /// <summary>
119: /// Read a 16 bit value from register at address
120: /// </summary>
121: /// <param name="address">Address of register</param>
122: /// <returns>16 bit register value</returns>
123: uint16 MemoryAccessMock::Read16(regaddr address)
124: {
125:     AddOperation({ address });
126:     TRACE_INFO("Read16(%p)", address);
127:     return {};
128: }
129: 
130: /// <summary>
131: /// Write a 16 bit value to register at address
132: /// </summary>
133: /// <param name="address">Address of register</param>
134: /// <param name="data">Data to write</param>
135: void MemoryAccessMock::Write16(regaddr address, uint16 data)
136: {
137:     AddOperation({ address, static_cast<uint32>(data), true });
138:     TRACE_INFO("Write16(%p, %04x)", address, data);
139: }
140: 
141: /// <summary>
142: /// Read a 32 bit value from register at address
143: /// </summary>
144: /// <param name="address">Address of register</param>
145: /// <returns>32 bit register value</returns>
146: uint32 MemoryAccessMock::Read32(regaddr address)
147: {
148:     AddOperation({ address });
149:     TRACE_INFO("Read32(%p)", address);
150:     return {};
151: }
152: 
153: /// <summary>
154: /// Write a 32 bit value to register at address
155: /// </summary>
156: /// <param name="address">Address of register</param>
157: /// <param name="data">Data to write</param>
158: void MemoryAccessMock::Write32(regaddr address, uint32 data)
159: {
160:     AddOperation({ address, data, true });
161:     TRACE_INFO("Write32(%p, %08x)", address, data);
162: }
163: 
164: /// <summary>
165: /// Add a memory access operation to the list
166: /// </summary>
167: /// <param name="operation">Operation to add</param>
168: void MemoryAccessMock::AddOperation(const MemoryAccessOperation& operation)
169: {
170:     assert(m_numAccessOps < BufferSize);
171:     m_accessOps[m_numAccessOps++] = operation;
172: }
```

- Line 54-64: We implement the serialization of `MemoryAccessOperation`
- Line 66-73: We implement the `MemoryAccessMock` constructor
- Line 75-82: We implement the method `GetNumOperations()`
- Line 84-93: We implement the method `GetMemoryOperation()`
- Line 95-105: We change the method `Read8()` to register a 8 bit read call
- Line 107-116: We change the method `Write8()` to register a 8 bit write call
- Line 118-128: We change the method `Read16()` to register a 16 bit read call
- Line 130-139: We change the method `Write16()` to register a 16 bit write call
- Line 141-151: We change the method `Read32()` to register a 32 bit read call
- Line 153-162: We change the method `Write32()` to register a 32 bit write call
- Line 164-172: We implement the method `AddOperation()`

### MemoryAccessMockTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_2_MEMORYACCESSMOCKTESTCPP}

We'll update the test, and take care of both Raspberry Pi 3 and 4 platforms.

Update the file `code\libraries\baremetal\test\src\MemoryAccessMockTest.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessMockTest.cpp
...
40: #include "baremetal/BCMRegisters.h"
41: #include "baremetal/PhysicalGPIOPin.h"
42: 
43: #include "MemoryAccessMock.h"
44: 
45: #include "unittest/unittest.h"
46: 
47: using namespace unittest;
48: 
49: namespace baremetal {
50: namespace test {
51: 
52: /// @brief Baremetal test suite
53: TEST_SUITE(Baremetal)
54: {
55: 
56: class MemoryAccessMockTest : public TestFixture
57: {
58: public:
59:     void SetUp() override
60:     {
61:     }
62:     void TearDown() override
63:     {
64:     }
65: };
66: 
67: TEST_FIXTURE(MemoryAccessMockTest, SimpleTest)
68: {
69:     MemoryAccessMock memoryAccess;
70:     PhysicalGPIOPin pin(memoryAccess);
71: 
72:     pin.AssignPin(0);
73:     pin.SetMode(GPIOMode::AlternateFunction0);
74: 
75:     size_t index{};
76: #if BAREMETAL_RPI_TARGET == 3
77:     EXPECT_EQ(size_t{5}, memoryAccess.GetNumOperations());
78:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUD, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
79:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000001, true }), memoryAccess.GetMemoryOperation(index++));
80:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
81: #else
82:     EXPECT_EQ(size_t{4}, memoryAccess.GetNumOperations());
83:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUPPDN0 }), memoryAccess.GetMemoryOperation(index++));
84:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUPPDN0, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
85: #endif
86:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPFSEL0 }), memoryAccess.GetMemoryOperation(index++));
87:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPFSEL0, 0x00000004, true }), memoryAccess.GetMemoryOperation(index++));
88: }
89: 
90: } // suite Baremetal
91: 
92: } // namespace test
93: } // namespace baremetal
```

- Line 77-80: We check that the number of registered operations is correct, and that the pull up/down operations are performed for Raspberry Pi 3
- Line 82-84: We check that the number of registered operations is correct, and that the pull up/down operations are performed for Raspberry Pi 4 and later
- Line 86-87: We check that the pin mode operations are performed

### Configuring, building and debugging {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. All tests should succeed.

For Raspberry Pi 3:

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.000 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (3 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
Info   0.00:00:00.020 Write32(3F200094, 00000000) (MemoryAccessMock.cpp:161)
Info   0.00:00:00.030 Write32(3F200098, 00000001) (MemoryAccessMock.cpp:161)
Info   0.00:00:00.030 Write32(3F200098, 00000000) (MemoryAccessMock.cpp:161)
Info   0.00:00:00.030 Read32(3F200000) (MemoryAccessMock.cpp:149)
Info   0.00:00:00.030 Write32(3F200000, 00000004) (MemoryAccessMock.cpp:161)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 3 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.040 Halt (System:121)
```

For Raspberry Pi 4:
```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.000 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (3 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
Info   0.00:00:00.010 Read32(FE2000E4) (MemoryAccessMock.cpp:149)
Info   0.00:00:00.020 Write32(FE2000E4, 00000000) (MemoryAccessMock.cpp:161)
Info   0.00:00:00.020 Read32(FE200000) (MemoryAccessMock.cpp:149)
Info   0.00:00:00.020 Write32(FE200000, 00000004) (MemoryAccessMock.cpp:161)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 3 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.030 Halt (System:121)
```

## Creating actual unit tests - Step 3 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_3}

So far we've been able to check that memory operations are performed, and we track the values written, however when reading we simply return 0.
It would be better if we could simulate actual behavior of the GPIO peripheral, so we'll introduce a virtual method for reading and writing a uint32 value.
This can then be overridden in a derived class to simulate actual behavior.

### MemoryAccessMock.h {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_3_MEMORYACCESSMOCKH}

Update the file `code\libraries\baremetal\test\include\MemoryAccessMock.h`

```cpp
File: code\libraries\baremetal\test\include\MemoryAccessMock.h
...
File: d:\Projects\Private\RaspberryPi\baremetal.github\code\libraries\baremetal\test\include\MemoryAccessMock.h
108: /// <summary>
109: /// Memory access mock class
110: /// </summary>
111: class MemoryAccessMock : public IMemoryAccess
112: {
113: private:
114:     /// @brief Size of memory access operation array
115:     static constexpr size_t BufferSize = 1000;
116:     /// @brief List of memory access operations
117:     MemoryAccessOperation m_accessOps[BufferSize] ALIGN(8);
118:     /// @brief Number of registered memory access operations
119:     size_t m_numAccessOps;
120: 
121: public:
122:     MemoryAccessMock();
123: 
124:     size_t GetNumOperations() const;
125:     const MemoryAccessOperation& GetMemoryOperation(size_t index) const;
126:     /// <summary>
127:     /// Callback on read operation
128:     /// </summary>
129:     /// <param name="address">Address to read from</param>
130:     /// <returns>32 bit unsigned integer read</returns>
131:     virtual uint32 OnRead(regaddr address) { return {}; }
132:     /// <summary>
133:     /// Callback on write operation
134:     /// </summary>
135:     /// <param name="address">Address to write to</param>
136:     /// <param name="data">32 bit unsigned integer value to write</param>
137:     virtual void OnWrite(regaddr address, uint32 data) {}
138: 
139:     uint8 Read8(regaddr address) override;
140:     void Write8(regaddr address, uint8 data) override;
141: 
142:     uint16 Read16(regaddr address) override;
143:     void Write16(regaddr address, uint16 data) override;
144: 
145:     uint32 Read32(regaddr address) override;
146:     void Write32(regaddr address, uint32 data) override;
147: 
148: private:
149:     void AddOperation(const MemoryAccessOperation& operation);
150: };
...
```

- Line 126-137: We add two virtual methods `OnRead()` and `OnWrite()`, which are called on read and write operations respectively. By default they do nothing (return 0 for read).

### MemoryAccessMockTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_3_MEMORYACCESSMOCKTESTCPP}

We'll update the test, and create an inheriting class which implements the two methods `OnRead()` and `OnWrite()`.

Update the file `code\libraries\baremetal\test\src\MemoryAccessMockTest.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessMockTest.cpp
...
File: d:\Projects\Private\RaspberryPi\baremetal.github\code\libraries\baremetal\test\src\MemoryAccessMockTest.cpp
52: /// @brief Baremetal test suite
53: TEST_SUITE(Baremetal)
54: {
55: 
56: class MemoryAccessMockTest : public TestFixture
57: {
58: public:
59:     void SetUp() override
60:     {
61:     }
62:     void TearDown() override
63:     {
64:     }
65: };
66: 
67: class MemoryAccessTestMock : public MemoryAccessMock
68: {
69: public:
70:     uint32 m_GPIO_GPPUD{};
71:     uint32 m_GPIO_GPPUDCLK0{};
72:     uint32 m_GPIO_GPPUPPDN0{};
73:     uint32 m_GPIO_GPFSEL0{};
74:     uint32 OnRead(regaddr address) override
75:     {
76: #if BAREMETAL_RPI_TARGET == 3
77:         if (address == RPI_GPIO_GPPUD)
78:             return m_GPIO_GPPUD;
79:         else if (address == RPI_GPIO_GPPUDCLK0)
80:             return m_GPIO_GPPUDCLK0;
81: #else
82:         if (address == RPI_GPIO_GPPUPPDN0)
83:             return m_GPIO_GPPUPPDN0;
84: #endif
85:         if (address == RPI_GPIO_GPFSEL0)
86:             return m_GPIO_GPFSEL0;
87:         return {0};
88:     }
89:     void OnWrite(regaddr address, uint32 value) override
90:     {
91: #if BAREMETAL_RPI_TARGET == 3
92:         if (address == RPI_GPIO_GPPUD)
93:             m_GPIO_GPPUD = value;
94:         else if (address == RPI_GPIO_GPPUDCLK0)
95:             m_GPIO_GPPUDCLK0 = value;
96: #else
97:         if (address == RPI_GPIO_GPPUPPDN0)
98:             m_GPIO_GPPUPPDN0 = value;
99: #endif
100:         if (address == RPI_GPIO_GPFSEL0)
101:             m_GPIO_GPFSEL0 = value;
102: 
103:         return;
104:     }
105: };
106: 
107: TEST_FIXTURE(MemoryAccessMockTest, SimpleTest)
108: {
109:     MemoryAccessTestMock memoryAccess;
110:     PhysicalGPIOPin pin(memoryAccess);
111: 
112:     pin.AssignPin(0);
113:     pin.SetMode(GPIOMode::AlternateFunction0);
114: 
115:     size_t index{};
116: #if BAREMETAL_RPI_TARGET == 3
117:     EXPECT_EQ(size_t{5}, memoryAccess.GetNumOperations());
118:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUD, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
119:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000001, true }), memoryAccess.GetMemoryOperation(index++));
120:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
121:     EXPECT_EQ(uint32{0x00000000}, memoryAccess.m_GPIO_GPPUD);
122:     EXPECT_EQ(uint32{0x00000000}, memoryAccess.m_GPIO_GPPUDCLK0);
123: #else
124:     EXPECT_EQ(size_t{4}, memoryAccess.GetNumOperations());
125:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUPPDN0 }), memoryAccess.GetMemoryOperation(index++));
126:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUPPDN0, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
127:     EXPECT_EQ(uint32{0x00000000}, memoryAccess.m_GPIO_GPPUPPDN0);
128: #endif
129:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPFSEL0 }), memoryAccess.GetMemoryOperation(index++));
130:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPFSEL0, 0x00000004, true }), memoryAccess.GetMemoryOperation(index++));
131:     EXPECT_EQ(uint32{0x00000004}, memoryAccess.m_GPIO_GPFSEL0);
132: }
133: 
134: } // suite Baremetal
...
```

- Line 67-105: We create a class `MemoryAccessTestMock` inheriting from `MemoryAccessMock`, which implements the two methods `OnRead()` and `OnWrite()`
  - Line 70-73: We add member variables to hold the state of the relevant GPIO registers
  - Line 74-88: We implement the method `OnRead()`, which returns the value of the relevant GPIO register
  - Line 89-104: We implement the method `OnWrite()`, which updates the value of the relevant GPIO register
- Line 109: We create an instance of `MemoryAccessTestMock` instead of `MemoryAccessMock`
- Line 121-122: We check that the GPIO pull up/down registers have the expected values for Raspberry Pi 3
- Line 124-127: We check that the GPIO pull up/down registers have the expected values for Raspberry Pi 4 and later
- Line 129-131: We check that the GPIO function registers have the expected values

### Configuring, building and debugging {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. All tests should succeed.

For Raspberry Pi 3:

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (3 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
Info   0.00:00:00.020 Write32(3F200094, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.020 Write32(3F200098, 00000001) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.020 Write32(3F200098, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.030 Read32(3F200000, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.030 Write32(3F200000, 00000004) (MemoryAccessMock.cpp:169)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 3 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.040 Halt (System:121)
```

For Raspberry Pi 4:
```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.000 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (3 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
Info   0.00:00:00.020 Read32(FE2000E4, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.020 Write32(FE2000E4, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.020 Read32(FE200000, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.020 Write32(FE200000, 00000004) (MemoryAccessMock.cpp:169)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 3 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.040 Halt (System:121)
```

## Creating actual unit tests - Step 4 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_4}

Now let's put some intelligence into all this. We already wrote a stub for the GPIO in class `MemoryAccessStubGPIO`, which interprets the data read and written to each register, let's make the registration of memory access operations smarter for GPIO.

### MemoryAccessGPIOMock.h {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_4_MEMORYACCESSGPIOMOCKH}

Create the file `code\libraries\baremetal\test\include\MemoryAccessGPIOMock.h`

```cpp
File: code\libraries\baremetal\test\include\MemoryAccessGPIOMock.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessGPIOMock.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessGPIOMock
9: //
10: // Description : GPIO memory access mock class
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
42: #include "MemoryAccessMock.h"
43: #include "baremetal/PhysicalGPIOPin.h"
44: #include "baremetal/String.h"
45: #include "stdlib/Macros.h"
46: 
47: /// @file
48: /// Memory access mock class
49: 
50: namespace baremetal {
51: 
52: /// @brief GPIO registers storage
53: struct GPIORegisters
54: {
55:     /// @brief RPI_GPIO_GPFSEL0 value
56:     uint32 FunctionSelect0; // 0x00
57:     /// @brief RPI_GPIO_GPFSEL1 value
58:     uint32 FunctionSelect1; // 0x04
59:     /// @brief RPI_GPIO_GPFSEL2 value
60:     uint32 FunctionSelect2; // 0x08
61:     /// @brief RPI_GPIO_GPFSEL3 value
62:     uint32 FunctionSelect3; // 0x0C
63:     /// @brief RPI_GPIO_GPFSEL4 value
64:     uint32 FunctionSelect4; // 0x10
65:     /// @brief RPI_GPIO_GPFSEL5 value
66:     uint32 FunctionSelect5; // 0x14
67:     /// @brief Reserved value
68:     uint32 Reserved_1; // 0x18
69:     /// @brief RPI_GPIO_GPSET0 value
70:     uint32 Set0; // 0x1C
71:     /// @brief RPI_GPIO_GPSET1 value
72:     uint32 Set1; // 0x20
73:     /// @brief Reserved value
74:     uint32 Reserved_2; // 0x24
75:     /// @brief RPI_GPIO_GPCLR0 value
76:     uint32 Clear0; // 0x28
77:     /// @brief RPI_GPIO_GPCLR1 value
78:     uint32 Clear1; // 0x2C
79:     /// @brief Reserved value
80:     uint32 Reserved_3; // 0x30
81:     /// @brief RPI_GPIO_GPLEV0 value
82:     uint32 PinLevel0; // 0x34
83:     /// @brief RPI_GPIO_GPLEV1 value
84:     uint32 PinLevel1; // 0x38
85:     /// @brief Reserved value
86:     uint32 Reserved_4; // 0x3C
87:     /// @brief RPI_GPIO_GPEDS0 value
88:     uint32 EventDetectStatus0; // 0x40
89:     /// @brief RPI_GPIO_GPEDS1 value
90:     uint32 EventDetectStatus1; // 0x44
91:     /// @brief Reserved value
92:     uint32 Reserved_5; // 0x48
93:     /// @brief RPI_GPIO_GPREN0 value
94:     uint32 RisingEdgeDetectEn0; // 0x4C
95:     /// @brief RPI_GPIO_GPREN1 value
96:     uint32 RisingEdgeDetectEn1; // 0x50
97:     /// @brief Reserved value
98:     uint32 Reserved_6; // 0x54
99:     /// @brief RPI_GPIO_GPFEN0 value
100:     uint32 FallingEdgeDetectEn0; // 0x58
101:     /// @brief RPI_GPIO_GPFEN1 value
102:     uint32 FallingEdgeDetectEn1; // 0x5C
103:     /// @brief Reserved value
104:     uint32 Reserved_7; // 0x60
105:     /// @brief RPI_GPIO_GPHEN0 value
106:     uint32 HighDetectEn0; // 0x64
107:     /// @brief RPI_GPIO_GPHEN1 value
108:     uint32 HighDetectEn1; // 0x68
109:     /// @brief Reserved value
110:     uint32 Reserved_8; // 0x6C
111:     /// @brief RPI_GPIO_GPLEN0 value
112:     uint32 LowDetectEn0; // 0x70
113:     /// @brief RPI_GPIO_GPLEN1 value
114:     uint32 LowDetectEn1; // 0x74
115:     /// @brief Reserved value
116:     uint32 Reserved_9; // 0x78
117:     /// @brief RPI_GPIO_GPAREN0 value
118:     uint32 AsyncRisingEdgeDetectEn0; // 0x7C
119:     /// @brief RPI_GPIO_GPAREN1 value
120:     uint32 AsyncRisingEdgeDetectEn1; // 0x80
121:     /// @brief Reserved value
122:     uint32 Reserved_10; // 0x84
123:     /// @brief RPI_GPIO_GPAFEN0 value
124:     uint32 AsyncFallingEdgeDetectEn0; // 0x88
125:     /// @brief RPI_GPIO_GPAFEN1 value
126:     uint32 AsyncFallingEdgeDetectEn1; // 0x8C
127:     /// @brief Reserved value
128:     uint32 Reserved_11; // 0x90
129:     /// @brief RPI_GPIO_GPPUD value
130:     uint32 PullUpDownEnable; // 0x94
131:     /// @brief RPI_GPIO_GPPUDCLK0 value
132:     uint32 PullUpDownEnableClock0; // 0x98
133:     /// @brief RPI_GPIO_GPPUDCLK1 value
134:     uint32 PullUpDownEnableClock1; // 0x9C
135:     /// @brief Reserved value
136:     uint32 Reserved_12[4]; // 0xA0-AC
137:     /// @brief Test register value
138:     uint32 Test; // 0xB0
139: #if BAREMETAL_RPI_TARGET == 4
140:     /// @brief Reserved value
141:     uint32 Reserved_13;     // 0xB4
142:     uint32 Reserved_14[10]; // 0xB8-DC
143:     uint32 Reserved_15;     // 0xE0
144:     /// @brief RPI_GPIO_GPPUPPDN0 value
145:     uint32 PullUpDown0; // 0xE4
146:     /// @brief RPI_GPIO_GPPUPPDN1 value
147:     uint32 PullUpDown1; // 0xE8
148:     /// @brief RPI_GPIO_GPPUPPDN2 value
149:     uint32 PullUpDown2; // 0xEC
150:     /// @brief RPI_GPIO_GPPUPPDN3 value
151:     uint32 PullUpDown3; // 0xF0
152:     /// @brief Reserved value
153:     uint32 Reserved_16;    // 0xF4: Alignment
154:     uint32 Reserved_17[2]; // 0xF8-FC: Alignment
155: #endif
156: 
157:     /// <summary>
158:     /// Constructor for GPIORegisters
159:     ///
160:     /// Sets default register values
161:     /// </summary>
162:     GPIORegisters()
163:         : FunctionSelect0{}
164:         , FunctionSelect1{}
165:         , FunctionSelect2{}
166:         , FunctionSelect3{}
167:         , FunctionSelect4{}
168:         , FunctionSelect5{}
169:         , Reserved_1{}
170:         , Set0{}
171:         , Set1{}
172:         , Reserved_2{}
173:         , Clear0{}
174:         , Clear1{}
175:         , Reserved_3{}
176:         , PinLevel0{}
177:         , PinLevel1{}
178:         , Reserved_4{}
179:         , EventDetectStatus0{}
180:         , EventDetectStatus1{}
181:         , Reserved_5{}
182:         , RisingEdgeDetectEn0{}
183:         , RisingEdgeDetectEn1{}
184:         , Reserved_6{}
185:         , FallingEdgeDetectEn0{}
186:         , FallingEdgeDetectEn1{}
187:         , Reserved_7{}
188:         , HighDetectEn0{}
189:         , HighDetectEn1{}
190:         , Reserved_8{}
191:         , LowDetectEn0{}
192:         , LowDetectEn1{}
193:         , Reserved_9{}
194:         , AsyncRisingEdgeDetectEn0{}
195:         , AsyncRisingEdgeDetectEn1{}
196:         , Reserved_10{}
197:         , AsyncFallingEdgeDetectEn0{}
198:         , AsyncFallingEdgeDetectEn1{}
199:         , Reserved_11{}
200:         , PullUpDownEnable{}
201:         , PullUpDownEnableClock0{}
202:         , PullUpDownEnableClock1{}
203:         , Reserved_12{}
204:         , Test{}
205: #if BAREMETAL_RPI_TARGET == 4
206:         , Reserved_13{}
207:         , Reserved_14{}
208:         , Reserved_15{}
209:         , PullUpDown0{0xAAA95555}
210:         , PullUpDown1{0xA0AAAAAA}
211:         , PullUpDown2{0x50AAA95A}
212:         , PullUpDown3{0x00055555}
213:         , Reserved_16{}
214:         , Reserved_17{}
215: #endif
216:     {
217:     }
218: } PACKED;
219: 
220: enum PhysicalGPIOPinOperationCode
221: {
222:     /// @brief Get pin value
223:     GetPinValue,
224:     /// @brief Get pin event status
225:     GetPinEventStatus,
226:     /// @brief Set pin function
227:     SetPinMode,
228:     /// @brief Set pin value
229:     SetPinValue,
230:     /// @brief Clear pin event detect status
231:     ClearPinEventStatus,
232:     /// @brief Set pin rising edge interrupt enable
233:     SetPinRisingEdgeInterruptEnable,
234:     /// @brief Set pin falling edge interrupt enable
235:     SetPinFallingEdgeInterruptEnable,
236:     /// @brief Set pin high level interrupt enable
237:     SetPinHighLevelInterruptEnable,
238:     /// @brief Set pin low level interrupt enable
239:     SetPinLowLevelInterruptEnable,
240:     /// @brief Set pin async rising edge interrupt enable
241:     SetPinAsyncRisingEdgeInterruptEnable,
242:     /// @brief Set pin async falling edge interrupt enable
243:     SetPinAsyncFallingEdgeInterruptEnable,
244:     /// @brief Set pull up/down mode
245:     SetPullUpDownMode,
246:     /// @brief Set pin pull up/down clock
247:     SetPinPullUpDownClock,
248:     /// @brief Set pin mux mode
249:     SetPinMuxMode,
250:     /// @brief Set pin pull up/down mode
251:     SetPinPullUpDownMode,
252: };
253: 
254: /// <summary>
255: /// Data structure to contain a memory access operation
256: /// </summary>
257: struct PhysicalGPIOPinOperation
258: {
259:     // GPIO pin operation code
260:     PhysicalGPIOPinOperationCode operation; // Size: 4 bytes
261:     // GPIO pin number
262:     uint8 pin;                              // Size: 1 byte (padded with 3 bytes)
263:     // GPIO pin mode
264:     GPIOFunction function;                  // Size: 4 bytes
265:     // GPIO pin argument (if any)
266:     uint32 argument;                        // Size: 4 bytes
267: 
268:     /// <summary>
269:     /// Default constructor
270:     /// </summary>
271:     PhysicalGPIOPinOperation()
272:         : operation{}
273:         , pin{}
274:         , function{}
275:         , argument{}
276:     {
277:     }
278:      /// <summary>
279:     /// Constructor for read or 2write operation concerning pin function
280:     /// </summary>
281:     /// <param name="theOperation">Operation code</param>
282:     /// <param name="thePin">Pin number</param>
283:     /// <param name="theFunction">Pin function to be set</param>
284:     PhysicalGPIOPinOperation(PhysicalGPIOPinOperationCode theOperation, uint8 thePin, GPIOFunction theFunction)
285:         : operation{theOperation}
286:         , pin{thePin}
287:         , function{theFunction}
288:         , argument{}
289:     {
290:     }
291:     /// <summary>
292:     /// Constructor for read or write operation with other data
293:     /// </summary>
294:     /// <param name="theOperation">Operation code</param>
295:     /// <param name="thePin">Pin number</param>
296:     /// <param name="theArgument">Value read</param>
297:     PhysicalGPIOPinOperation(PhysicalGPIOPinOperationCode theOperation, uint8 thePin, uint32 theArgument)
298:         : operation{theOperation}
299:         , pin{thePin}
300:         , function{}
301:         , argument{theArgument}
302:     {
303:     }
304:     /// <summary>
305:     /// Check memory access operations for equality
306:     /// </summary>
307:     /// <param name="other">Value to compare to</param>
308:     /// <returns>True if equal, false otherwise</returns>
309:     bool operator==(const PhysicalGPIOPinOperation& other) const
310:     {
311:         return (other.operation == operation) &&
312:             (other.pin == pin) &&
313:             (other.function == function) &&
314:             (other.argument == argument);
315:     }
316:     /// <summary>
317:     /// Check memory access operations for inequality
318:     /// </summary>
319:     /// <param name="other">Value to compare to</param>
320:     /// <returns>True if unequal, false otherwise</returns>
321:     bool operator!=(const PhysicalGPIOPinOperation& other) const
322:     {
323:         return !operator==(other);
324:     }
325: };
326: 
327: String Serialize(const PhysicalGPIOPinOperation& value);
328: 
329: /// <summary>
330: /// Memory access mock class
331: /// </summary>
332: class MemoryAccessGPIOMock : public MemoryAccessMock
333: {
334: private:
335:     /// @brief Saved GPIO register values
336:     GPIORegisters m_registers;
337:     /// @brief Size of memory access operation array
338:     static constexpr size_t BufferSize = 1000;
339:     /// List op memory access operations
340:     PhysicalGPIOPinOperation m_accessOps[BufferSize] ALIGN(8);
341:     /// @brief Number of registered memory access operations
342:     size_t m_numAccessOps;
343: 
344: public:
345:     MemoryAccessGPIOMock();
346: 
347:     size_t GetNumOperations() const;
348:     const PhysicalGPIOPinOperation& GetGPIOOperation(size_t index) const;
349: 
350:     uint32 OnRead(regaddr address) override;
351:     void OnWrite(regaddr address, uint32 data) override;
352: 
353: private:
354:     uint32 GetRegisterOffset(regaddr address);
355:     void AddOperation(const PhysicalGPIOPinOperation& operation);
356: };
357: 
358: } // namespace baremetal
```

- Line 52-218: We define a structure `GPIORegisters` to hold the state of all GPIO registers (much like we did in the stub class)
- Line 220-252: We define an enumeration `PhysicalGPIOPinOperationCode` to hold the various GPIO pin operation types
- Line 254-325: We define a structure `PhysicalGPIOPinOperation` to hold the details of a GPIO pin operation
- Line 327: We declare a function `Serialize()` to convert a `PhysicalGPIOPinOperation` to a string
- Line 329-356: We define the class `MemoryAccessGPIOMock` inheriting from `MemoryAccessMock`, which implements the GPIO specific behavior
  - Line 335-336: We add a member variable `m_registers` to hold the GPIO register state
  - Line 337-338: We add a constant `BufferSize` to hold the size of the array op operations we store
  - Line 339-340: We add a member variable `m_accessOps` to hold the list of GPIO operations performed
  - Line 341-342: We add a member variable `m_numAccessOps` to hold the number of GPIO operations performed
  - Line 345: We declare the default constructor
  - Line 347: We declare the method `GetNumOperations()` to retrieve the number of GPIO operations performed
  - Line 348: We declare the method `GetGPIOOperation()` to retrieve a specific operation
  - Line 350-351: Override methods for read and write operations
  - Line 354: We declare the private method `GetRegisterOffset()` to get the register offset from an address
  - Line 355: We declare the private method `AddOperation()` to add a GPIO operation to the list

### MemoryAccessGPIOMock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_4_MEMORYACCESSGPIOMOCKCPP}

We'll now implement the methods of the `MemoryAccessGPIOMock` class.

Create the file `code\libraries\baremetal\test\src\MemoryAccessGPIOMock.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessGPIOMock.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessGPIOMock.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessGPIOMock
9: //
10: // Description : GPIO memory access mock class
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
40: #include "MemoryAccessGPIOMock.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/Format.h"
45: #include "baremetal/Logger.h"
46: 
47: /// @file
48: /// Memory access mock class implementation
49: 
50: /// @brief Define log name
51: LOG_MODULE("MemoryAccessGPIOMock");
52: 
53: using namespace baremetal;
54: 
55: /// <summary>
56: /// Convert pin function to string
57: /// </summary>
58: /// <param name="mode">Pin function</param>
59: /// <returns>String representing pin function</returns>
60: static String PinFunctionToString(GPIOFunction function)
61: {
62:     String result{};
63:     switch (function)
64:     {
65:     case GPIOFunction::Input:
66:         result = "Input";
67:         break;
68:     case GPIOFunction::Output:
69:         result = "Output";
70:         break;
71:     case GPIOFunction::AlternateFunction0:
72:         result = "Alt0";
73:         break;
74:     case GPIOFunction::AlternateFunction1:
75:         result = "Alt1";
76:         break;
77:     case GPIOFunction::AlternateFunction2:
78:         result = "Alt2";
79:         break;
80:     case GPIOFunction::AlternateFunction3:
81:         result = "Alt3";
82:         break;
83:     case GPIOFunction::AlternateFunction4:
84:         result = "Alt4";
85:         break;
86:     case GPIOFunction::AlternateFunction5:
87:         result = "Alt5";
88:         break;
89:     case GPIOFunction::Unknown:
90:         result = "Unknown";
91:         break;
92:     }
93:     return result;
94: }
95: 
96: /// <summary>
97: /// Convert pull up/down mode to string
98: /// </summary>
99: /// <param name="mode">Pull up/down mode</param>
100: /// <returns>String representing pull up/down mode</returns>
101: static String PullModeToString(uint8 mode)
102: {
103:     String result{};
104:     switch (mode & 0x03)
105:     {
106:     case 0:
107:         result = "None";
108:         break;
109:     case 1:
110: #if BAREMETAL_RPI_TARGET == 3
111:         result = "PullDown";
112: #else
113:         result = "PullUp";
114: #endif
115:         break;
116:     case 2:
117: #if BAREMETAL_RPI_TARGET == 3
118:         result = "PullUp";
119: #else
120:         result = "PullDown";
121: #endif
122:         break;
123:     case 3:
124:         result = "Reserved";
125:         break;
126:     }
127:     return result;
128: }
129: 
130: /// <summary>
131: /// Convert operation code to string
132: /// </summary>
133: /// <param name="code">Operation cpde</param>
134: /// <returns>String representing operator code</returns>
135: static String OperationCodeToString(PhysicalGPIOPinOperationCode code)
136: {
137:     String result{};
138:     switch (code)
139:     {
140:     case PhysicalGPIOPinOperationCode::GetPinValue:
141:         result = "GetPinValue";
142:         break;
143:     case PhysicalGPIOPinOperationCode::GetPinEventStatus:
144:         result = "GetPinEventStatus";
145:         break;
146:     case PhysicalGPIOPinOperationCode::SetPinMode:
147:         result = "SetPinMode";
148:         break;
149:     case PhysicalGPIOPinOperationCode::SetPinValue:
150:         result = "SetPinValue";
151:         break;
152:     case PhysicalGPIOPinOperationCode::ClearPinEventStatus:
153:         result = "ClearPinEventStatus";
154:         break;
155:     case PhysicalGPIOPinOperationCode::SetPinRisingEdgeInterruptEnable:
156:         result = "SetPinRisingEdgeInterruptEnable";
157:         break;
158:     case PhysicalGPIOPinOperationCode::SetPinFallingEdgeInterruptEnable:
159:         result = "SetPinFallingEdgeInterruptEnable";
160:         break;
161:     case PhysicalGPIOPinOperationCode::SetPinHighLevelInterruptEnable:
162:         result = "SetPinHighLevelInterruptEnable";
163:         break;
164:     case PhysicalGPIOPinOperationCode::SetPinLowLevelInterruptEnable:
165:         result = "SetPinLowLevelInterruptEnable";
166:         break;
167:     case PhysicalGPIOPinOperationCode::SetPinAsyncRisingEdgeInterruptEnable:
168:         result = "SetPinAsyncRisingEdgeInterruptEnable";
169:         break;
170:     case PhysicalGPIOPinOperationCode::SetPinAsyncFallingEdgeInterruptEnable:
171:         result = "SetPinAsyncFallingEdgeInterruptEnable";
172:         break;
173:     case PhysicalGPIOPinOperationCode::SetPullUpDownMode:
174:         result = "SetPullUpDownMode";
175:         break;
176:     case PhysicalGPIOPinOperationCode::SetPinPullUpDownClock:
177:         result = "SetPinPullUpDownClock";
178:         break;
179:     case PhysicalGPIOPinOperationCode::SetPinMuxMode:
180:         result = "SetPinMuxMode";
181:         break;
182:     case PhysicalGPIOPinOperationCode::SetPinPullUpDownMode:
183:         result = "SetPinPullUpDownMode";
184:         break;
185: 
186:     }
187:     return result;
188: }
189: 
190: /// <summary>
191: /// Serialize a GPIO memory access operation to string
192: /// </summary>
193: /// <param name="value">Value to be serialized</param>
194: /// <returns>Resulting string</returns>
195: String baremetal::Serialize(const PhysicalGPIOPinOperation &value)
196: {
197:     String result = Format("Operation=%s, Pin=%s, ", OperationCodeToString(value.operation).c_str(), (value.pin == 0xFF ? "None" : Format("%d", value.pin).c_str()));
198:     switch (value.operation)
199:     {
200:     case PhysicalGPIOPinOperationCode::SetPinMode:
201:         result += Format("Function=%s", PinFunctionToString(value.function).c_str());
202:         break;
203:     case PhysicalGPIOPinOperationCode::SetPullUpDownMode:
204:     case PhysicalGPIOPinOperationCode::SetPinPullUpDownMode:
205:         result += Format("PullMode=%s", PullModeToString(static_cast<uint8>(value.argument)).c_str());
206:         break;
207:     default:
208:         result += Format("Argument=%d", value.argument);
209:         break;
210:     }
211:     return result;
212: }
213: 
214: /// @brief GPIO base address
215: static uintptr GPIOBaseAddress{RPI_GPIO_BASE};
216: /// @brief Mask used to check whether an address is in the GPIO register range
217: static uintptr GPIOBaseAddressMask{0xFFFFFFFFFFFFFF00};
218: 
219: /// <summary>
220: /// MemoryAccessGPIOMock constructor
221: /// </summary>
222: MemoryAccessGPIOMock::MemoryAccessGPIOMock()
223:     : m_registers{}
224:     , m_accessOps{}
225:     , m_numAccessOps{}
226: {
227: }
228: 
229: /// <summary>
230: /// Return number of registered memory access operations
231: /// </summary>
232: /// <returns>Number of registered memory access operations</returns>
233: size_t MemoryAccessGPIOMock::GetNumOperations() const
234: {
235:     return m_numAccessOps;
236: }
237: 
238: /// <summary>
239: /// Retrieve a registered memory access operation from the list
240: /// </summary>
241: /// <param name="index">Index of operation</param>
242: /// <returns>Requested memory access operation</returns>
243: const PhysicalGPIOPinOperation &MemoryAccessGPIOMock::GetGPIOOperation(size_t index) const
244: {
245:     assert(index < m_numAccessOps);
246:     return m_accessOps[index];
247: }
248: 
249: /// <summary>
250: /// Read a 32 bit value from register at address
251: /// </summary>
252: /// <param name="address">Address of register</param>
253: /// <returns>32 bit register value</returns>
254: uint32 MemoryAccessGPIOMock::OnRead(regaddr address)
255: {
256:     if ((reinterpret_cast<uintptr>(address) & GPIOBaseAddressMask) != GPIOBaseAddress)
257:         return MemoryAccessMock::OnRead(address);
258: 
259:     uintptr offset = GetRegisterOffset(address);
260:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
261:     switch (offset)
262:     {
263:     case RPI_GPIO_GPFSEL0_OFFSET:
264:     case RPI_GPIO_GPFSEL1_OFFSET:
265:     case RPI_GPIO_GPFSEL2_OFFSET:
266:     case RPI_GPIO_GPFSEL3_OFFSET:
267:     case RPI_GPIO_GPFSEL4_OFFSET:
268:     case RPI_GPIO_GPFSEL5_OFFSET:
269:         break;
270:     case RPI_GPIO_GPLEV0_OFFSET:
271:     case RPI_GPIO_GPLEV1_OFFSET:
272:         {
273:             uint8 pinBase = (offset - RPI_GPIO_GPLEV0_OFFSET) / 4 * 32;
274:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
275:             {
276:                 int shift = pinIndex;
277:                 uint8 pin = pinBase + pinIndex;
278:                 uint8 value = (*registerField >> shift) & 0x00000001;
279:                 AddOperation({ PhysicalGPIOPinOperationCode::GetPinValue, pin, value });
280:             }
281:             break;
282:         }
283:     case RPI_GPIO_GPEDS0_OFFSET:
284:     case RPI_GPIO_GPEDS1_OFFSET:
285:         {
286:             uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET) / 4 * 32;
287:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
288:             {
289:                 int shift = pinIndex;
290:                 uint8 pin = pinBase + pinIndex;
291:                 uint8 value = (*registerField >> shift) & 0x00000001;
292:                 AddOperation({ PhysicalGPIOPinOperationCode::GetPinEventStatus, pin, value });
293:             }
294:             break;
295:         }
296:     case RPI_GPIO_GPREN0_OFFSET:
297:     case RPI_GPIO_GPREN1_OFFSET:
298:     case RPI_GPIO_GPFEN0_OFFSET:
299:     case RPI_GPIO_GPFEN1_OFFSET:
300:     case RPI_GPIO_GPHEN0_OFFSET:
301:     case RPI_GPIO_GPHEN1_OFFSET:
302:     case RPI_GPIO_GPLEN0_OFFSET:
303:     case RPI_GPIO_GPLEN1_OFFSET:
304:     case RPI_GPIO_GPAREN0_OFFSET:
305:     case RPI_GPIO_GPAREN1_OFFSET:
306:     case RPI_GPIO_GPAFEN0_OFFSET:
307:     case RPI_GPIO_GPAFEN1_OFFSET:
308:         break;
309: #if BAREMETAL_RPI_TARGET == 3
310:     case RPI_GPIO_GPPUD_OFFSET:
311:     case RPI_GPIO_GPPUDCLK0_OFFSET:
312:     case RPI_GPIO_GPPUDCLK1_OFFSET:
313:         break;
314: #elif BAREMETAL_RPI_TARGET == 4
315:     case RPI_GPIO_GPPINMUXSD_OFFSET:
316:     case RPI_GPIO_GPPUPPDN0_OFFSET:
317:     case RPI_GPIO_GPPUPPDN1_OFFSET:
318:     case RPI_GPIO_GPPUPPDN2_OFFSET:
319:     case RPI_GPIO_GPPUPPDN3_OFFSET:
320:         break;
321: #endif
322:     default:
323:         LOG_ERROR("Invalid register access for reading: offset %d", offset);
324:         break;
325:     }
326:     return *registerField;
327: }
328: 
329: /// <summary>
330: /// Write a 32 bit value to register at address
331: /// </summary>
332: /// <param name="address">Address of register</param>
333: /// <param name="data">Data to write</param>
334: void MemoryAccessGPIOMock::OnWrite(regaddr address, uint32 data)
335: {
336:     if ((reinterpret_cast<uintptr>(address) & GPIOBaseAddressMask) != GPIOBaseAddress)
337:     {
338:         MemoryAccessMock::OnWrite(address, data);
339:         return;
340:     }
341: 
342:     uintptr offset = GetRegisterOffset(address);
343:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
344:     switch (offset)
345:     {
346:     case RPI_GPIO_GPFSEL0_OFFSET:
347:     case RPI_GPIO_GPFSEL1_OFFSET:
348:     case RPI_GPIO_GPFSEL2_OFFSET:
349:     case RPI_GPIO_GPFSEL3_OFFSET:
350:     case RPI_GPIO_GPFSEL4_OFFSET:
351:     case RPI_GPIO_GPFSEL5_OFFSET:
352:         {
353:             uint8 pinBase = (offset - RPI_GPIO_GPFSEL0_OFFSET) / 4 * 10;
354:             uint32 diff = data ^ *registerField;
355:             for (uint8 pinIndex = 0; pinIndex < 10; ++pinIndex)
356:             {
357:                 int shift = pinIndex * 3;
358:                 if (((diff >> shift) & 0x00000007) != 0)
359:                 {
360:                     uint8 pin = pinBase + pinIndex;
361:                     uint8 value = (data >> shift) & 0x00000007;
362:                     uint8 lookup[]{0, 1, 7, 6, 2, 3, 4, 5};
363:                     auto pinMode = static_cast<GPIOFunction>(lookup[value]);
364:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinMode, pin, pinMode});
365:                 }
366:             }
367:             break;
368:         }
369:     case RPI_GPIO_GPSET0_OFFSET:
370:     case RPI_GPIO_GPSET1_OFFSET:
371:         {
372:             uint8 pinBase = (offset - RPI_GPIO_GPSET0_OFFSET) / 4 * 32;
373:             uint32 diff = data ^ *registerField;
374:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
375:             {
376:                 int shift = pinIndex;
377:                 if (((diff >> shift) & 0x00000001) != 0)
378:                 {
379:                     uint8 pin = pinBase + pinIndex;
380:                     uint8 value = (data >> shift) & 0x00000001;
File: d:\Projects\Private\RaspberryPi\baremetal.github\code\libraries\baremetal\test\src\MemoryAccessGPIOMock.cpp
381:                     if (value)
382:                         AddOperation({PhysicalGPIOPinOperationCode::SetPinValue, pin, 1});
383:                 }
384:             }
385:             break;
386:         }
387:     case RPI_GPIO_GPCLR0_OFFSET:
388:     case RPI_GPIO_GPCLR1_OFFSET:
389:         {
390:             uint8 pinBase = (offset - RPI_GPIO_GPCLR0_OFFSET) / 4 * 32;
391:             uint32 diff = data ^ *registerField;
392:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
393:             {
394:                 int shift = pinIndex;
395:                 if (((diff >> shift) & 0x00000001) != 0)
396:                 {
397:                     uint8 pin = pinBase + pinIndex;
398:                     uint8 value = (data >> shift) & 0x00000001;
399:                     if (value)
400:                         AddOperation({PhysicalGPIOPinOperationCode::SetPinValue, pin, 0});
401:                 }
402:             }
403:             break;
404:         }
405:     case RPI_GPIO_GPEDS0_OFFSET:
406:     case RPI_GPIO_GPEDS1_OFFSET:
407:         {
408:             uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET) / 4 * 32;
409:             uint32 diff = data ^ *registerField;
410:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
411:             {
412:                 int shift = pinIndex;
413:                 if (((diff >> shift) & 0x00000001) != 0)
414:                 {
415:                     uint8 pin = pinBase + pinIndex;
416:                     uint8 value = (data >> shift) & 0x00000001;
417:                     AddOperation({PhysicalGPIOPinOperationCode::ClearPinEventStatus, pin, value});
418:                 }
419:             }
420:             break;
421:         }
422:     case RPI_GPIO_GPREN0_OFFSET:
423:     case RPI_GPIO_GPREN1_OFFSET:
424:         {
425:             uint8 pinBase = (offset - RPI_GPIO_GPREN0_OFFSET) / 4 * 32;
426:             uint32 diff = data ^ *registerField;
427:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
428:             {
429:                 int shift = pinIndex;
430:                 if (((diff >> shift) & 0x00000001) != 0)
431:                 {
432:                     uint8 pin = pinBase + pinIndex;
433:                     uint8 value = (data >> shift) & 0x00000001;
434:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinRisingEdgeInterruptEnable, pin, value});
435:                 }
436:             }
437:             break;
438:         }
439:     case RPI_GPIO_GPFEN0_OFFSET:
440:     case RPI_GPIO_GPFEN1_OFFSET:
441:         {
442:             uint8 pinBase = (offset - RPI_GPIO_GPFEN0_OFFSET) / 4 * 32;
443:             uint32 diff = data ^ *registerField;
444:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
445:             {
446:                 int shift = pinIndex;
447:                 if (((diff >> shift) & 0x00000001) != 0)
448:                 {
449:                     uint8 pin = pinBase + pinIndex;
450:                     uint8 value = (data >> shift) & 0x00000001;
451:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinFallingEdgeInterruptEnable, pin, value});
452:                 }
453:             }
454:             break;
455:         }
456:     case RPI_GPIO_GPHEN0_OFFSET:
457:     case RPI_GPIO_GPHEN1_OFFSET:
458:         {
459:             uint8 pinBase = (offset - RPI_GPIO_GPHEN0_OFFSET) / 4 * 32;
460:             uint32 diff = data ^ *registerField;
461:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
462:             {
463:                 int shift = pinIndex;
464:                 if (((diff >> shift) & 0x00000001) != 0)
465:                 {
466:                     uint8 pin = pinBase + pinIndex;
467:                     uint8 value = (data >> shift) & 0x00000001;
468:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinHighLevelInterruptEnable, pin, value});
469:                 }
470:             }
471:             break;
472:         }
473:     case RPI_GPIO_GPLEN0_OFFSET:
474:     case RPI_GPIO_GPLEN1_OFFSET:
475:         {
476:             uint8 pinBase = (offset - RPI_GPIO_GPLEN0_OFFSET) / 4 * 32;
477:             uint32 diff = data ^ *registerField;
478:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
479:             {
480:                 int shift = pinIndex;
481:                 if (((diff >> shift) & 0x00000001) != 0)
482:                 {
483:                     uint8 pin = pinBase + pinIndex;
484:                     uint8 value = (data >> shift) & 0x00000001;
485:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinLowLevelInterruptEnable, pin, value});
486:                 }
487:             }
488:             break;
489:         }
490:     case RPI_GPIO_GPAREN0_OFFSET:
491:     case RPI_GPIO_GPAREN1_OFFSET:
492:         {
493:             uint8 pinBase = (offset - RPI_GPIO_GPAREN0_OFFSET) / 4 * 32;
494:             uint32 diff = data ^ *registerField;
495:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
496:             {
497:                 int shift = pinIndex;
498:                 if (((diff >> shift) & 0x00000001) != 0)
499:                 {
500:                     uint8 pin = pinBase + pinIndex;
501:                     uint8 value = (data >> shift) & 0x00000001;
502:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinAsyncRisingEdgeInterruptEnable, pin, value});
503:                 }
504:             }
505:             break;
506:         }
507:     case RPI_GPIO_GPAFEN0_OFFSET:
508:     case RPI_GPIO_GPAFEN1_OFFSET:
509:         {
510:             uint8 pinBase = (offset - RPI_GPIO_GPAFEN0_OFFSET) / 4 * 32;
511:             uint32 diff = data ^ *registerField;
512:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
513:             {
514:                 int shift = pinIndex;
515:                 if (((diff >> shift) & 0x00000001) != 0)
516:                 {
517:                     uint8 pin = pinBase + pinIndex;
518:                     uint8 value = (data >> shift) & 0x00000001;
519:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinAsyncFallingEdgeInterruptEnable, pin, value});
520:                 }
521:             }
522:             break;
523:         }
524: #if BAREMETAL_RPI_TARGET == 3
525:     case RPI_GPIO_GPPUD_OFFSET:
526:         {
527:             uint8 value = data & 0x00000003;
528:             AddOperation({PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, value});
529:             break;
530:         }
531:     case RPI_GPIO_GPPUDCLK0_OFFSET:
532:     case RPI_GPIO_GPPUDCLK1_OFFSET:
533:         {
534:             uint8 pinBase = (offset - RPI_GPIO_GPPUDCLK0_OFFSET) / 4 * 32;
535:             uint32 diff = data ^ *registerField;
536:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
537:             {
538:                 int shift = pinIndex;
539:                 if (((diff >> shift) & 0x00000001) != 0)
540:                 {
541:                     uint8 pin = pinBase + pinIndex;
542:                     uint8 value = (data >> shift) & 0x00000001;
543:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, pin, value});
544:                 }
545:             }
546:             break;
547:         }
548: #elif BAREMETAL_RPI_TARGET == 4
549:     case RPI_GPIO_GPPINMUXSD_OFFSET:
550:         {
551:             uint32 value = *registerField;
552:             AddOperation({PhysicalGPIOPinOperationCode::SetPinMuxMode, 0xFF, value});
553:             break;
554:         }
555:     case RPI_GPIO_GPPUPPDN0_OFFSET:
556:     case RPI_GPIO_GPPUPPDN1_OFFSET:
557:     case RPI_GPIO_GPPUPPDN2_OFFSET:
558:     case RPI_GPIO_GPPUPPDN3_OFFSET:
559:         {
560:             uint8 pinBase = (offset - RPI_GPIO_GPPUPPDN0_OFFSET) / 4 * 16;
561:             uint32 diff = data ^ *registerField;
562:             for (uint8 pinIndex = 0; pinIndex < 16; ++pinIndex)
563:             {
564:                 int shift = pinIndex * 2;
565:                 if (((diff >> shift) & 0x00000003) != 0)
566:                 {
567:                     uint8 pin = pinBase + pinIndex;
568:                     uint8 value = (data >> shift) & 0x00000003;
569:                     String modeName = PullModeToString(value);
570:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, pin, value});
571:                 }
572:             }
573:             break;
574:         }
575: #endif
576:     default:
577:         LOG_ERROR("Invalid GPIO register access for writing: offset %d", offset);
578:         break;
579:     }
580:     *registerField = data;
581: }
582: 
583: /// <summary>
584: /// Determine register address offset relative to GPIO base address
585: ///
586: /// If the address is not in the correct range, an assert is fired
587: /// </summary>
588: /// <param name="address">Address to check</param>
589: /// <returns>Offset relative to GPIO base address</returns>
590: uint32 MemoryAccessGPIOMock::GetRegisterOffset(regaddr address)
591: {
592:     return reinterpret_cast<uintptr>(address) - GPIOBaseAddress;
593: }
594: 
595: /// <summary>
596: /// Add a memory access operation to the list
597: /// </summary>
598: /// <param name="operation">Operation to add</param>
599: void MemoryAccessGPIOMock::AddOperation(const PhysicalGPIOPinOperation& operation)
600: {
601:     assert(m_numAccessOps < BufferSize);
602:     m_accessOps[m_numAccessOps++] = operation;
603: }
```

- Line 55-94: We define a helper function `PinFunctionToString()` to convert a `GPIOFunction` enum value to a string
- Line 96-128: We define a helper function `PullModeToString()` to convert a pull up/down mode value to a string
- Line 130-188: We define a helper function `OperationCodeToString()` to convert a `PhysicalGPIOPinOperationCode` enum value to a string
- Line 190-212: We implement the `Serialize()` function to convert a `PhysicalGPIOPinOperation` to a string
- Line 214-217: We define constants for the GPIO base address and mask
- Line 219-227: We implement the constructor for `MemoryAccessGPIOMock`
- Line 229-236: We implement the `GetNumOperations()` method
- Line 238-247: We implement the `GetGPIOOperation()` method
- Line 249-327: We implement the `OnRead()` method to handle read operations
  - Line 256-257: We check if the address is in the GPIO register range, if not we call the base class method
  - Line 259-260: We calculate the register offset and get a pointer to the corresponding register field
  - Line 272-282: For level registers, we store a `GetPinValue` operation for each pin
  - Line 285-295: For event status registers, we store a `GetPinEventStatus` operation for each pin
  - Other registers are simply read, no operations are stored
- Line 329-570: We implement the `OnWrite()` method to handle write operations
  - Line 336-339: We check if the address is in the GPIO register range, if not we call the base class method and return
  - Line 342-343: We calculate the register offset and get a pointer to the corresponding register field
  - Line 352-368: For function select registers, we store a `SetPinMode` operation for each pin whose mode changed
  - Line 371-386: For set registers, we store a `SetPinValue` operation for each pin being set
  - Line 389-404: For clear registers, we store a `SetPinValue` operation for each pin being cleared
  - Line 407-421: For event status registers, we store a `ClearPinEventStatus` operation for each pin being cleared
  - Line 422-523: Similar logic is applied for the interrupt enable GPIO registers to store the appropriate operations
  - Line 525-547: For RPi 3 pull up/down registers, we store `SetPullUpDownMode` and `SetPinPullUpDownClock` operations
  - Line 549-574: For RPi 4 pull up/down registers, we store `SetPinMuxMode` and `SetPinPullUpDownMode` operations
- Line 583-593: We implement the `GetRegisterOffset()` method to calculate the register offset and assert if the address is invalid
- Line 595-603: We implement the `AddOperation()` method to add a GPIO operation to the list

### MemoryAccessGPIOMockTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_4_MEMORYACCESSGPIOMOCKTESTCPP}

Let's create a test.

Create the file `code\libraries\baremetal\test\src\MemoryAccessGPIOMockTest.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessMockGPIOTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessGPIOMockTest.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessGPIOMockTest
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
40: #include "baremetal/BCMRegisters.h"
41: #include "baremetal/PhysicalGPIOPin.h"
42: 
43: #include "MemoryAccessGPIOMock.h"
44: 
45: #include "unittest/unittest.h"
46: 
47: using namespace unittest;
48: 
49: namespace baremetal {
50: namespace test {
51: 
52: /// @brief Baremetal test suite
53: TEST_SUITE(Baremetal)
54: {
55: 
56: class MemoryAccessGPIOMockTest : public TestFixture
57: {
58: public:
59:     void SetUp() override
60:     {
61:     }
62:     void TearDown() override
63:     {
64:     }
65: };
66: 
67: TEST_FIXTURE(MemoryAccessGPIOMockTest, SimpleTest)
68: {
69:     MemoryAccessGPIOMock memoryAccess;
70:     PhysicalGPIOPin pin(memoryAccess);
71: 
72:     pin.AssignPin(0);
73:     pin.SetMode(GPIOMode::AlternateFunction0);
74: 
75:     size_t index{};
76: #if BAREMETAL_RPI_TARGET == 3
77:     EXPECT_EQ(size_t{4}, memoryAccess.GetNumOperations());
78:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(index++));
79:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000000, 0x1 }), memoryAccess.GetGPIOOperation(index++));
80:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000000, 0x0 }), memoryAccess.GetGPIOOperation(index++));
81: #else
82:     EXPECT_EQ(size_t{2}, memoryAccess.GetNumOperations());
83:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x0, 0x0 }), memoryAccess.GetGPIOOperation(index++));
84: #endif
85:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x0, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(index++));
86: }
87: 
88: } // suite Baremetal
89: 
90: } // namespace test
91: } // namespace baremetal
```

- Line 69: We create an instance of the `MemoryAccessGPIOMock`
- Line 70: We create an instance of the `PhysicalGPIOPin`, passing the mock memory access instance
- Line 72-73: We assign pin 0 and set its mode to `AlternateFunction0` as before
- Line 77-80: For RPi 3, we expect 4 operations to be recorded: setting pull up/down mode, setting pull up/down clock twice, and setting pin mode
- Line 82-83: For RPi 4, we expect 2 operations to be recorded: setting pull up/down mode and setting pin mode
- Line 85: We check that the pin mode operation is as expected

### Extend selected tests {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_4_EXTEND_SELECTED_TESTS}

As we are being selective about which tests to run, we need to update the main test application to add the tests for the GPIO mock.

Update the file `code\libraries\baremetal\test\src\main.cpp`

```cpp
File: code\libraries\baremetal\test\src\main.cpp
File: d:\Projects\Private\RaspberryPi\baremetal.github\code\libraries\baremetal\test\src\main.cpp
1: #include "baremetal/System.h"
2: #include "unittest/unittest.h"
3: 
4: using namespace baremetal;
5: using namespace unittest;
6: 
7: int main()
8: {
9:     ConsoleTestReporter reporter;
10:     RunSelectedTests(&reporter, InSelection("Baremetal", "MemoryAccessMockTest", nullptr));
11:     RunSelectedTests(&reporter, InSelection("Baremetal", "MemoryAccessGPIOMockTest", nullptr));
12: //    RunAllTests(&reporter);
13: 
14:     return static_cast<int>(ReturnCode::ExitHalt);
15: }
```

- Line 11: We add the line to run the `MemoryAccessGPIOMockTest` tests

### Configuring, building and debugging {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_4_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. All tests should succeed.

For Raspberry Pi 3:

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (4 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
Info   0.00:00:00.030 Write32(3F200094, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.030 Write32(3F200098, 00000001) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.030 Write32(3F200098, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.030 Read32(3F200000, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.040 Write32(3F200000, 00000004) (MemoryAccessMock.cpp:169)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 4 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (4 fixtures)
[  FIXTURE  ] MemoryAccessGPIOMockTest (1 test)
Info   0.00:00:00.050 Write32(3F200094, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.050 Write32(3F200098, 00000001) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.050 Write32(3F200098, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.050 Read32(3F200000, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.060 Write32(3F200000, 00000004) (MemoryAccessMock.cpp:169)
[ SUCCEEDED ] Baremetal::MemoryAccessGPIOMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessGPIOMockTest
[   SUITE   ] 4 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.060 Halt (System:121)
```

For Raspberry Pi 4:
```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (4 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
Info   0.00:00:00.020 Read32(FE2000E4, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.020 Write32(FE2000E4, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.030 Read32(FE200000, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.030 Write32(FE200000, 00000004) (MemoryAccessMock.cpp:169)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 4 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (4 fixtures)
[  FIXTURE  ] MemoryAccessGPIOMockTest (1 test)
Info   0.00:00:00.050 Read32(FE2000E4, AAA95555)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.050 Write32(FE2000E4, AAA95554) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.050 Read32(FE200000, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.050 Write32(FE200000, 00000004) (MemoryAccessMock.cpp:169)
[ SUCCEEDED ] Baremetal::MemoryAccessGPIOMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessGPIOMockTest
[   SUITE   ] 4 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.070 Halt (System:121)
```

## Creating actual unit tests - Step 5 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_5}

We see for for runs that the memory operations are shown, let's move the tracing of memory access to the `OnRead()` and `OnWrite()` methods of the mock.
For the GPIO mock, let's trace more specific operations.

### MemoryAccessMock.h {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_5_MEMORYACCESSMOCKH}

Update the file `code\libraries\baremetal\test\include\MemoryAccessMock.h`

```cpp
File: code\libraries\baremetal\test\include\MemoryAccessMock.h
...
108: /// <summary>
109: /// Memory access mock class
110: /// </summary>
111: class MemoryAccessMock : public IMemoryAccess
112: {
113: private:
114:     /// @brief Size of memory access operation array
115:     static constexpr size_t BufferSize = 1000;
116:     /// @brief List of memory access operations
117:     MemoryAccessOperation m_accessOps[BufferSize] ALIGN(8);
118:     /// @brief Number of registered memory access operations
119:     size_t m_numAccessOps;
120: 
121: public:
122:     MemoryAccessMock();
123: 
124:     size_t GetNumOperations() const;
125:     const MemoryAccessOperation& GetMemoryOperation(size_t index) const;
126:     virtual uint32 OnRead(regaddr address);
127:     virtual void OnWrite(regaddr address, uint32 data);
128: 
129:     uint8 Read8(regaddr address) override;
130:     void Write8(regaddr address, uint8 data) override;
131: 
132:     uint16 Read16(regaddr address) override;
133:     void Write16(regaddr address, uint16 data) override;
134: 
135:     uint32 Read32(regaddr address) override;
136:     void Write32(regaddr address, uint32 data) override;
137: 
138: private:
139:     void AddOperation(const MemoryAccessOperation& operation);
140: };
141: 
142: } // namespace baremetal
```

- Line 126-127: We move the implementation of the methods `OnRead()` and `OnWrite()` to the source file

### MemoryAccessMock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_5_MEMORYACCESSMOCKCPP}

Update the file `code\libraries\baremetal\test\src\MemoryAccessMock.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessMock.cpp
...
95: /// <summary>
96: /// Callback on read operation
97: /// </summary>
98: /// <param name="address">Address to read from</param>
99: /// <returns>32 bit unsigned integer read</returns>
100: uint32 MemoryAccessMock::OnRead(regaddr address)
101: {
102:     uint32 value{};
103:     TRACE_INFO("Read(%p, %02x))", address, value);
104:     return value;
105: }
106: 
107: /// <summary>
108: /// Callback on write operation
109: /// </summary>
110: /// <param name="address">Address to write to</param>
111: /// <param name="data">32 bit unsigned integer value to write</param>
112: void MemoryAccessMock::OnWrite(regaddr address, uint32 data)
113: {
114:     TRACE_INFO("Write(%p, %02x)", address, data);
115: }
116: 
117: /// <summary>
118: /// Read a 8 bit value from register at address
119: /// </summary>
120: /// <param name="address">Address of register</param>
121: /// <returns>8 bit register value</returns>
122: uint8 MemoryAccessMock::Read8(regaddr address)
123: {
124:     uint8 value = OnRead(address) & 0xFF;
125:     AddOperation({address, value, false});
126:     return value;
127: }
128: 
129: /// <summary>
130: /// Write a 8 bit value to register at address
131: /// </summary>
132: /// <param name="address">Address of register</param>
133: /// <param name="data">Data to write</param>
134: void MemoryAccessMock::Write8(regaddr address, uint8 data)
135: {
136:     uint32 value = static_cast<uint32>(data);
137:     AddOperation({ address, value, true });
138:     OnWrite(address, value);
139: }
140: 
141: /// <summary>
142: /// Read a 16 bit value from register at address
143: /// </summary>
144: /// <param name="address">Address of register</param>
145: /// <returns>16 bit register value</returns>
146: uint16 MemoryAccessMock::Read16(regaddr address)
147: {
148:     uint16 value = OnRead(address) & 0xFFFF;
149:     AddOperation({address, value, false});
150:     return value;
151: }
152: 
153: /// <summary>
154: /// Write a 16 bit value to register at address
155: /// </summary>
156: /// <param name="address">Address of register</param>
157: /// <param name="data">Data to write</param>
158: void MemoryAccessMock::Write16(regaddr address, uint16 data)
159: {
160:     uint32 value = static_cast<uint32>(data);
161:     AddOperation({ address, value, true });
162:     OnWrite(address, value);
163: }
164: 
165: /// <summary>
166: /// Read a 32 bit value from register at address
167: /// </summary>
168: /// <param name="address">Address of register</param>
169: /// <returns>32 bit register value</returns>
170: uint32 MemoryAccessMock::Read32(regaddr address)
171: {
172:     uint32 value = OnRead(address);
173:     AddOperation({address, value, false});
174:     return value;
175: }
176: 
177: /// <summary>
178: /// Write a 32 bit value to register at address
179: /// </summary>
180: /// <param name="address">Address of register</param>
181: /// <param name="data">Data to write</param>
182: void MemoryAccessMock::Write32(regaddr address, uint32 data)
183: {
184:     AddOperation({ address, data, true });
185:     OnWrite(address, data);
186: }
...
```

This code speaks for itself.

### MemoryAccessGPIOMock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_5_MEMORYACCESSGPIOMOCKCPP}

Update the file `code\libraries\baremetal\test\src\MemoryAccessGPIOMock.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessGPIOMock.cpp
...
249: /// <summary>
250: /// Read a 32 bit value from register at address
251: /// </summary>
252: /// <param name="address">Address of register</param>
253: /// <returns>32 bit register value</returns>
254: uint32 MemoryAccessGPIOMock::OnRead(regaddr address)
255: {
256:     if ((reinterpret_cast<uintptr>(address) & GPIOBaseAddressMask) != GPIOBaseAddress)
257:         return MemoryAccessMock::OnRead(address);
258: 
259:     uintptr offset = GetRegisterOffset(address);
260:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
261:     switch (offset)
262:     {
263:     case RPI_GPIO_GPFSEL0_OFFSET:
264:     case RPI_GPIO_GPFSEL1_OFFSET:
265:     case RPI_GPIO_GPFSEL2_OFFSET:
266:     case RPI_GPIO_GPFSEL3_OFFSET:
267:     case RPI_GPIO_GPFSEL4_OFFSET:
268:     case RPI_GPIO_GPFSEL5_OFFSET:
269:         break;
270:     case RPI_GPIO_GPLEV0_OFFSET:
271:     case RPI_GPIO_GPLEV1_OFFSET:
272:         {
273:             uint8 pinBase = (offset - RPI_GPIO_GPLEV0_OFFSET) / 4 * 32;
274:             String line{"GPIO Read Pin Level "};
275:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
276:             {
277:                 int shift = pinIndex;
278:                 uint8 pin = pinBase + pinIndex;
279:                 uint8 value = (*registerField >> shift) & 0x00000001;
280:                 AddOperation({ PhysicalGPIOPinOperationCode::GetPinValue, pin, value });
281:                 if (value)
282:                     line += Format(" - Pin %d ON ", pin);
283:                 else
284:                     line += Format(" - Pin %d OFF", pin);
285:             }
286:             TRACE_DEBUG(line.c_str());
287:             break;
288:         }
289:     case RPI_GPIO_GPEDS0_OFFSET:
290:     case RPI_GPIO_GPEDS1_OFFSET:
291:         {
292:             uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET) / 4 * 32;
293:             String line{"GPIO Read Pin Event Detect Status "};
294:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
295:             {
296:                 int shift = pinIndex;
297:                 uint8 pin = pinBase + pinIndex;
298:                 uint8 value = (*registerField >> shift) & 0x00000001;
299:                 AddOperation({ PhysicalGPIOPinOperationCode::GetPinEventStatus, pin, value });
300:                 if (value)
301:                     line += Format(" - Pin %d ON ", pin);
302:                 else
303:                     line += Format(" - Pin %d OFF", pin);
304:             }
305:             TRACE_DEBUG(line.c_str());
306:             break;
307:         }
308:     case RPI_GPIO_GPREN0_OFFSET:
309:     case RPI_GPIO_GPREN1_OFFSET:
310:     case RPI_GPIO_GPFEN0_OFFSET:
311:     case RPI_GPIO_GPFEN1_OFFSET:
312:     case RPI_GPIO_GPHEN0_OFFSET:
313:     case RPI_GPIO_GPHEN1_OFFSET:
314:     case RPI_GPIO_GPLEN0_OFFSET:
315:     case RPI_GPIO_GPLEN1_OFFSET:
316:     case RPI_GPIO_GPAREN0_OFFSET:
317:     case RPI_GPIO_GPAREN1_OFFSET:
318:     case RPI_GPIO_GPAFEN0_OFFSET:
319:     case RPI_GPIO_GPAFEN1_OFFSET:
320:         break;
321: #if BAREMETAL_RPI_TARGET == 3
322:     case RPI_GPIO_GPPUD_OFFSET:
323:     case RPI_GPIO_GPPUDCLK0_OFFSET:
324:     case RPI_GPIO_GPPUDCLK1_OFFSET:
325:         break;
326: #elif BAREMETAL_RPI_TARGET == 4
327:     case RPI_GPIO_GPPINMUXSD_OFFSET:
328:     case RPI_GPIO_GPPUPPDN0_OFFSET:
329:     case RPI_GPIO_GPPUPPDN1_OFFSET:
330:     case RPI_GPIO_GPPUPPDN2_OFFSET:
331:     case RPI_GPIO_GPPUPPDN3_OFFSET:
332:         break;
333: #endif
334:     default:
335:         LOG_ERROR("Invalid register access for reading: offset %d", offset);
336:         break;
337:     }
338:     return *registerField;
339: }
340: 
341: /// <summary>
342: /// Write a 32 bit value to register at address
343: /// </summary>
344: /// <param name="address">Address of register</param>
345: /// <param name="data">Data to write</param>
346: void MemoryAccessGPIOMock::OnWrite(regaddr address, uint32 data)
347: {
348:     if ((reinterpret_cast<uintptr>(address) & GPIOBaseAddressMask) != GPIOBaseAddress)
349:     {
350:         MemoryAccessMock::OnWrite(address, data);
351:         return;
352:     }
353: 
354:     uintptr offset = GetRegisterOffset(address);
355:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers) + offset);
356:     switch (offset)
357:     {
358:     case RPI_GPIO_GPFSEL0_OFFSET:
359:     case RPI_GPIO_GPFSEL1_OFFSET:
360:     case RPI_GPIO_GPFSEL2_OFFSET:
361:     case RPI_GPIO_GPFSEL3_OFFSET:
362:     case RPI_GPIO_GPFSEL4_OFFSET:
363:     case RPI_GPIO_GPFSEL5_OFFSET:
364:         {
365:             uint8 pinBase = (offset - RPI_GPIO_GPFSEL0_OFFSET) / 4 * 10;
366:             uint32 diff = data ^ *registerField;
367:             for (uint8 pinIndex = 0; pinIndex < 10; ++pinIndex)
368:             {
369:                 int shift = pinIndex * 3;
370:                 if (((diff >> shift) & 0x00000007) != 0)
371:                 {
372:                     uint8 pin = pinBase + pinIndex;
373:                     uint8 value = (data >> shift) & 0x00000007;
374:                     uint8 lookup[]{0, 1, 7, 6, 2, 3, 4, 5};
375:                     auto pinMode = static_cast<GPIOFunction>(lookup[value]);
376:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinMode, pin, pinMode});
377:                     String modeName = PinFunctionToString(pinMode);
378:                     TRACE_DEBUG("GPIO Set Pin %d Mode %s", pin, modeName.c_str());
379:                 }
380:             }
381:             break;
382:         }
383:     case RPI_GPIO_GPSET0_OFFSET:
384:     case RPI_GPIO_GPSET1_OFFSET:
385:         {
386:             uint8 pinBase = (offset - RPI_GPIO_GPSET0_OFFSET) / 4 * 32;
387:             uint32 diff = data ^ *registerField;
388:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
389:             {
390:                 int shift = pinIndex;
391:                 if (((diff >> shift) & 0x00000001) != 0)
392:                 {
393:                     uint8 pin = pinBase + pinIndex;
394:                     uint8 value = (data >> shift) & 0x00000001;
395:                     if (value)
396:                     {
397:                         AddOperation({PhysicalGPIOPinOperationCode::SetPinValue, pin, 1});
398:                         TRACE_DEBUG("GPIO Set Pin %d ON", pin);
399:                     }
400:                 }
401:             }
402:             break;
403:         }
404:     case RPI_GPIO_GPCLR0_OFFSET:
405:     case RPI_GPIO_GPCLR1_OFFSET:
406:         {
407:             uint8 pinBase = (offset - RPI_GPIO_GPCLR0_OFFSET) / 4 * 32;
408:             uint32 diff = data ^ *registerField;
409:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
410:             {
411:                 int shift = pinIndex;
412:                 if (((diff >> shift) & 0x00000001) != 0)
413:                 {
414:                     uint8 pin = pinBase + pinIndex;
415:                     uint8 value = (data >> shift) & 0x00000001;
416:                     if (value)
417:                     {
418:                         AddOperation({PhysicalGPIOPinOperationCode::SetPinValue, pin, 0});
419:                         TRACE_DEBUG("GPIO Set Pin %d OFF", pin);
420:                     }
421:                 }
422:             }
423:             break;
424:         }
425:     case RPI_GPIO_GPEDS0_OFFSET:
426:     case RPI_GPIO_GPEDS1_OFFSET:
427:         {
428:             uint8 pinBase = (offset - RPI_GPIO_GPEDS0_OFFSET) / 4 * 32;
429:             uint32 diff = data ^ *registerField;
430:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
431:             {
432:                 int shift = pinIndex;
433:                 if (((diff >> shift) & 0x00000001) != 0)
434:                 {
435:                     uint8 pin = pinBase + pinIndex;
436:                     uint8 value = (data >> shift) & 0x00000001;
437:                     if (value != 0)
438:                     {
439:                         AddOperation({PhysicalGPIOPinOperationCode::ClearPinEventStatus, pin, value});
440:                         TRACE_DEBUG("GPIO Clear Pin %d Event Status", pin);
441:                     }
442:                 }
443:             }
444:             break;
445:         }
446:     case RPI_GPIO_GPREN0_OFFSET:
447:     case RPI_GPIO_GPREN1_OFFSET:
448:         {
449:             uint8 pinBase = (offset - RPI_GPIO_GPREN0_OFFSET) / 4 * 32;
450:             uint32 diff = data ^ *registerField;
451:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
452:             {
453:                 int shift = pinIndex;
454:                 if (((diff >> shift) & 0x00000001) != 0)
455:                 {
456:                     uint8 pin = pinBase + pinIndex;
457:                     uint8 value = (data >> shift) & 0x00000001;
458:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinRisingEdgeInterruptEnable, pin, value});
459:                     if (value != 0)
460:                         TRACE_DEBUG("GPIO Set Pin %d Rising Edge Detect ON", pin);
461:                     else
462:                         TRACE_DEBUG("GPIO Set Pin %d Rising Edge Detect OFF", pin);
463:                 }
464:             }
465:             break;
466:         }
467:     case RPI_GPIO_GPFEN0_OFFSET:
468:     case RPI_GPIO_GPFEN1_OFFSET:
469:         {
470:             uint8 pinBase = (offset - RPI_GPIO_GPFEN0_OFFSET) / 4 * 32;
471:             uint32 diff = data ^ *registerField;
472:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
473:             {
474:                 int shift = pinIndex;
475:                 if (((diff >> shift) & 0x00000001) != 0)
476:                 {
477:                     uint8 pin = pinBase + pinIndex;
478:                     uint8 value = (data >> shift) & 0x00000001;
479:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinFallingEdgeInterruptEnable, pin, value});
480:                     if (value != 0)
481:                         TRACE_DEBUG("GPIO Set Pin %d Falling Edge Detect ON", pin);
482:                     else
483:                         TRACE_DEBUG("GPIO Set Pin %d Falling Edge Detect OFF", pin);
484:                  }
485:             }
486:             break;
487:         }
488:     case RPI_GPIO_GPHEN0_OFFSET:
489:     case RPI_GPIO_GPHEN1_OFFSET:
490:         {
491:             uint8 pinBase = (offset - RPI_GPIO_GPHEN0_OFFSET) / 4 * 32;
492:             uint32 diff = data ^ *registerField;
493:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
494:             {
495:                 int shift = pinIndex;
496:                 if (((diff >> shift) & 0x00000001) != 0)
497:                 {
498:                     uint8 pin = pinBase + pinIndex;
499:                     uint8 value = (data >> shift) & 0x00000001;
500:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinHighLevelInterruptEnable, pin, value});
501:                     if (value != 0)
502:                         TRACE_DEBUG("GPIO Set Pin %d High Level Detect ON", pin);
503:                     else
504:                         TRACE_DEBUG("GPIO Set Pin %d High Level Detect OFF", pin);
505:                 }
506:             }
507:             break;
508:         }
509:     case RPI_GPIO_GPLEN0_OFFSET:
510:     case RPI_GPIO_GPLEN1_OFFSET:
511:         {
512:             uint8 pinBase = (offset - RPI_GPIO_GPLEN0_OFFSET) / 4 * 32;
513:             uint32 diff = data ^ *registerField;
514:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
515:             {
516:                 int shift = pinIndex;
517:                 if (((diff >> shift) & 0x00000001) != 0)
518:                 {
519:                     uint8 pin = pinBase + pinIndex;
520:                     uint8 value = (data >> shift) & 0x00000001;
521:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinLowLevelInterruptEnable, pin, value});
522:                     if (value != 0)
523:                         TRACE_DEBUG("GPIO Set Pin %d Low Level Detect ON", pin);
524:                     else
525:                         TRACE_DEBUG("GPIO Set Pin %d Low Level Detect OFF", pin);
526:                 }
527:             }
528:             break;
529:         }
530:     case RPI_GPIO_GPAREN0_OFFSET:
531:     case RPI_GPIO_GPAREN1_OFFSET:
532:         {
533:             uint8 pinBase = (offset - RPI_GPIO_GPAREN0_OFFSET) / 4 * 32;
534:             uint32 diff = data ^ *registerField;
535:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
536:             {
537:                 int shift = pinIndex;
538:                 if (((diff >> shift) & 0x00000001) != 0)
539:                 {
540:                     uint8 pin = pinBase + pinIndex;
541:                     uint8 value = (data >> shift) & 0x00000001;
542:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinAsyncRisingEdgeInterruptEnable, pin, value});
543:                     if (value != 0)
544:                         TRACE_DEBUG("GPIO Set Pin %d Async Rising Edge Detect ON", pin);
545:                     else
546:                         TRACE_DEBUG("GPIO Set Pin %d Async Rising Edge Detect OFF", pin);
547:                 }
548:             }
549:             break;
550:         }
551:     case RPI_GPIO_GPAFEN0_OFFSET:
552:     case RPI_GPIO_GPAFEN1_OFFSET:
553:         {
554:             uint8 pinBase = (offset - RPI_GPIO_GPAFEN0_OFFSET) / 4 * 32;
555:             uint32 diff = data ^ *registerField;
556:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
557:             {
558:                 int shift = pinIndex;
559:                 if (((diff >> shift) & 0x00000001) != 0)
560:                 {
561:                     uint8 pin = pinBase + pinIndex;
562:                     uint8 value = (data >> shift) & 0x00000001;
563:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinAsyncFallingEdgeInterruptEnable, pin, value});
564:                     if (value != 0)
565:                         TRACE_DEBUG("GPIO Set Pin %d Async Falling Edge Detect ON", pin);
566:                     else
567:                         TRACE_DEBUG("GPIO Set Pin %d Async Falling Edge Detect OFF", pin);
568:                 }
569:             }
570:             break;
571:         }
572: #if BAREMETAL_RPI_TARGET == 3
573:     case RPI_GPIO_GPPUD_OFFSET:
574:         {
575:             uint8 value = data & 0x00000003;
576:             AddOperation({PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, value});
577:             String modeName = PullModeToString(value);
578:             TRACE_DEBUG("GPIO Set Pin Pull Up/Down Mode %s", modeName.c_str());
579:             break;
580:         }
581:     case RPI_GPIO_GPPUDCLK0_OFFSET:
582:     case RPI_GPIO_GPPUDCLK1_OFFSET:
583:         {
584:             uint8 pinBase = (offset - RPI_GPIO_GPPUDCLK0_OFFSET) / 4 * 32;
585:             uint32 diff = data ^ *registerField;
586:             for (uint8 pinIndex = 0; pinIndex < 32; ++pinIndex)
587:             {
588:                 int shift = pinIndex;
589:                 if (((diff >> shift) & 0x00000001) != 0)
590:                 {
591:                     uint8 pin = pinBase + pinIndex;
592:                     uint8 value = (data >> shift) & 0x00000001;
593:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, pin, value});
594:                     if (value != 0)
595:                         TRACE_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock ON", pin);
596:                     else
597:                         TRACE_DEBUG("GPIO Set Pin %d Pull Up/Down Enable Clock OFF", pin);
598:                 }
599:             }
600:             break;
601:         }
602: #elif BAREMETAL_RPI_TARGET == 4
603:     case RPI_GPIO_GPPINMUXSD_OFFSET:
604:         {
605:             uint32 value = *registerField;
606:             TRACE_DEBUG("GPIO Set Pin Mux Mode %x", value);
607:             AddOperation({PhysicalGPIOPinOperationCode::SetPinMuxMode, 0xFF, value});
608:             break;
609:         }
610:     case RPI_GPIO_GPPUPPDN0_OFFSET:
611:     case RPI_GPIO_GPPUPPDN1_OFFSET:
612:     case RPI_GPIO_GPPUPPDN2_OFFSET:
613:     case RPI_GPIO_GPPUPPDN3_OFFSET:
614:         {
615:             uint8 pinBase = (offset - RPI_GPIO_GPPUPPDN0_OFFSET) / 4 * 16;
616:             uint32 diff = data ^ *registerField;
617:             for (uint8 pinIndex = 0; pinIndex < 16; ++pinIndex)
618:             {
619:                 int shift = pinIndex * 2;
620:                 if (((diff >> shift) & 0x00000003) != 0)
621:                 {
622:                     uint8 pin = pinBase + pinIndex;
623:                     uint8 value = (data >> shift) & 0x00000003;
624:                     String modeName = PullModeToString(value);
625:                     AddOperation({PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, pin, value});
626:                     TRACE_DEBUG("GPIO Set Pin %d Pull Up/Down Mode %s", pin, modeName.c_str());
627:                 }
628:             }
629:             break;
630:         }
631: #endif
632:     default:
633:         LOG_ERROR("Invalid GPIO register access for writing: offset %d", offset);
634:         break;
635:     }
636:     *registerField = data;
637: }
...
```

This code speaks for itself.

### MemoryAccessMockTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_5_MEMORYACCESSMOCKTESTCPP}

As we override the `OnRead()` and `OnWrite()` methods, we also need to trace in the test code.

Update the file `code\libraries\baremetal\test\src\MemoryAccessMockTest.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessMockTest.cpp
...
40: #include "baremetal/BCMRegisters.h"
41: #include "baremetal/Logger.h"
42: #include "baremetal/PhysicalGPIOPin.h"
43: 
44: #include "MemoryAccessMock.h"
45: 
46: #include "unittest/unittest.h"
47: 
48: /// @brief Define log name
49: LOG_MODULE("MemoryAccessMockTest");
...
71: class MemoryAccessTestMock : public MemoryAccessMock
72: {
73: public:
74:     uint32 m_GPIO_GPPUD{};
75:     uint32 m_GPIO_GPPUDCLK0{};
76:     uint32 m_GPIO_GPPUPPDN0{};
77:     uint32 m_GPIO_GPFSEL0{};
78:     uint32 OnRead(regaddr address) override
79:     {
80:         uint32 value{};
81: #if BAREMETAL_RPI_TARGET == 3
82:         if (address == RPI_GPIO_GPPUD)
83:             value = m_GPIO_GPPUD;
84:         else if (address == RPI_GPIO_GPPUDCLK0)
85:             value = m_GPIO_GPPUDCLK0;
86: #else
87:         if (address == RPI_GPIO_GPPUPPDN0)
88:             value = m_GPIO_GPPUPPDN0;
89: #endif
90:         if (address == RPI_GPIO_GPFSEL0)
91:             value = m_GPIO_GPFSEL0;
92:         TRACE_DEBUG("Read(%p, %02x))", address, value);
93:         return value;
94:     }
95:     void OnWrite(regaddr address, uint32 value) override
96:     {
97:         TRACE_DEBUG("Write(%p, %02x))", address, value);
98: #if BAREMETAL_RPI_TARGET == 3
99:         if (address == RPI_GPIO_GPPUD)
100:             m_GPIO_GPPUD = value;
101:         else if (address == RPI_GPIO_GPPUDCLK0)
102:             m_GPIO_GPPUDCLK0 = value;
103: #else
104:         if (address == RPI_GPIO_GPPUPPDN0)
105:             m_GPIO_GPPUPPDN0 = value;
106: #endif
107:         if (address == RPI_GPIO_GPFSEL0)
108:             m_GPIO_GPFSEL0 = value;
109: 
110:         return;
111:     }
112: };
...
```

### Configuring, building and debugging {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_5_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. All tests should succeed.

For Raspberry Pi 3:

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:93)
Info   0.00:00:00.000 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (4 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
Debug  0.00:00:00.010 Write(3F200094, 00)) (MemoryAccessMockTest.cpp:97)
Debug  0.00:00:00.020 Write(3F200098, 01)) (MemoryAccessMockTest.cpp:97)
Debug  0.00:00:00.020 Write(3F200098, 00)) (MemoryAccessMockTest.cpp:97)
Debug  0.00:00:00.020 Read(3F200000, 00)) (MemoryAccessMockTest.cpp:92)
Debug  0.00:00:00.020 Write(3F200000, 04)) (MemoryAccessMockTest.cpp:97)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 4 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (4 fixtures)
[  FIXTURE  ] MemoryAccessGPIOMockTest (1 test)
Debug  0.00:00:00.040 GPIO Set Pin Pull Up/Down Mode None (MemoryAccessGPIOMock.cpp:578)
Debug  0.00:00:00.040 GPIO Set Pin 0 Pull Up/Down Enable Clock ON (MemoryAccessGPIOMock.cpp:595)
Debug  0.00:00:00.040 GPIO Set Pin 0 Pull Up/Down Enable Clock OFF (MemoryAccessGPIOMock.cpp:597)
Debug  0.00:00:00.040 GPIO Set Pin 0 Mode Alt0 (MemoryAccessGPIOMock.cpp:378)
[ SUCCEEDED ] Baremetal::MemoryAccessGPIOMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessGPIOMockTest
[   SUITE   ] 4 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.050 Halt (System:121)
```

For Raspberry Pi 4:
```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (4 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
Debug  0.00:00:00.020 Read(FE2000E4, 00)) (MemoryAccessMockTest.cpp:92)
Debug  0.00:00:00.020 Write(FE2000E4, 00)) (MemoryAccessMockTest.cpp:97)
Debug  0.00:00:00.030 Read(FE200000, 00)) (MemoryAccessMockTest.cpp:92)
Debug  0.00:00:00.030 Write(FE200000, 04)) (MemoryAccessMockTest.cpp:97)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 4 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (4 fixtures)
[  FIXTURE  ] MemoryAccessGPIOMockTest (1 test)
Debug  0.00:00:00.050 GPIO Set Pin 0 Pull Up/Down Mode None (MemoryAccessGPIOMock.cpp:626)
Debug  0.00:00:00.050 GPIO Set Pin 0 Mode Alt0 (MemoryAccessGPIOMock.cpp:378)
[ SUCCEEDED ] Baremetal::MemoryAccessGPIOMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessGPIOMockTest
[   SUITE   ] 4 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.070 Halt (System:121)
```

## Creating actual unit tests - Step 6 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_6}

Now we'll move on to the I2C unit tests.


Next: [25-lcd](25-lcd.md)
