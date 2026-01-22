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
We'll place the headers for a mock under include/baremetal/mocks, alongside the stubs we wrote before.
Similarly, we'll place the implementation sources under src/mocks, alongside the stubs.

Create the file `code\libraries\baremetal\include\baremetal\mocks\MemoryAccessMock.h`

```cpp
File: code\libraries\baremetal\include\baremetal\mocks\MemoryAccessMock.h
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

Create the file `code\libraries\baremetal\src\mocks\MemoryAccessMock.cpp`

```cpp
File: code\libraries\baremetal\src\mocks\MemoryAccessMock.cpp
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
40: #include "baremetal/mocks/MemoryAccessMock.h"
41: #include "baremetal/Logger.h"
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
10: // Description : Memory access mock tests
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
40: #include "baremetal/mocks/MemoryAccessMock.h"
41: 
42: #include "baremetal/PhysicalGPIOPin.h"
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

Update the file `code\libraries\baremetal\include\baremetal\mocks\MemoryAccessMock.h`

```cpp
File: code\libraries\baremetal\include\baremetal\mocks\MemoryAccessMock.h
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
124:     size_t GetNumMemoryOperations() const;
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
  - Line 124: We declare a method `GetNumMemoryOperations()` to retrieve the actual number of registered operations
  - Line 125: We declare a method `GetMemoryOperation()` to retrieve a specific operation by index
  - Line 137: We declare a method to add an operation to the array (if it fits)

### MemoryAccessMock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_2_MEMORYACCESSMOCKCPP}

We'll update the `MemoryAccessMock` class.

Update the file `code\libraries\baremetal\src\mocks\MemoryAccessMock.cpp`

```cpp
File: code\libraries\baremetal\src\mocks\MemoryAccessMock.cpp
...
40: #include "baremetal/mocks/MemoryAccessMock.h"
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
79: size_t MemoryAccessMock::GetNumMemoryOperations() const
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
- Line 75-82: We implement the method `GetNumMemoryOperations()`
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
40: #include "baremetal/mocks/MemoryAccessMock.h"
41: 
42: #include "baremetal/BCMRegisters.h"
43: #include "baremetal/PhysicalGPIOPin.h"
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
77:     EXPECT_EQ(size_t{5}, memoryAccess.GetNumMemoryOperations());
78:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUD, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
79:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000001, true }), memoryAccess.GetMemoryOperation(index++));
80:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
81: #else
82:     EXPECT_EQ(size_t{4}, memoryAccess.GetNumMemoryOperations());
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

Update the file `code\libraries\baremetal\include\baremetal\mocks\MemoryAccessMock.h`

```cpp
File: code\libraries\baremetal\include\baremetal\mocks\MemoryAccessMock.h
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
124:     size_t GetNumMemoryOperations() const;
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
117:     EXPECT_EQ(size_t{5}, memoryAccess.GetNumMemoryOperations());
118:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUD, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
119:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000001, true }), memoryAccess.GetMemoryOperation(index++));
120:     EXPECT_EQ((MemoryAccessOperation{ RPI_GPIO_GPPUDCLK0, 0x00000000, true }), memoryAccess.GetMemoryOperation(index++));
121:     EXPECT_EQ(uint32{0x00000000}, memoryAccess.m_GPIO_GPPUD);
122:     EXPECT_EQ(uint32{0x00000000}, memoryAccess.m_GPIO_GPPUDCLK0);
123: #else
124:     EXPECT_EQ(size_t{4}, memoryAccess.GetNumMemoryOperations());
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

Create the file `code\libraries\baremetal\include\baremetal\mocks\MemoryAccessGPIOMock.h`

```cpp
File: code\libraries\baremetal\include\baremetal\mocks\MemoryAccessGPIOMock.h
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
42: #include "baremetal/mocks/MemoryAccessMock.h"
43: 
44: #include "baremetal/PhysicalGPIOPin.h"
45: #include "baremetal/String.h"
46: #include "stdlib/Macros.h"
47: 
48: /// @file
49: /// Memory access mock class
50: 
51: namespace baremetal {
52: 
53: /// @brief GPIO registers storage
54: struct GPIORegisters
55: {
56:     /// @brief RPI_GPIO_GPFSEL0 value
57:     uint32 FunctionSelect0; // 0x00
58:     /// @brief RPI_GPIO_GPFSEL1 value
59:     uint32 FunctionSelect1; // 0x04
60:     /// @brief RPI_GPIO_GPFSEL2 value
61:     uint32 FunctionSelect2; // 0x08
62:     /// @brief RPI_GPIO_GPFSEL3 value
63:     uint32 FunctionSelect3; // 0x0C
64:     /// @brief RPI_GPIO_GPFSEL4 value
65:     uint32 FunctionSelect4; // 0x10
66:     /// @brief RPI_GPIO_GPFSEL5 value
67:     uint32 FunctionSelect5; // 0x14
68:     /// @brief Reserved value
69:     uint32 Reserved_1; // 0x18
70:     /// @brief RPI_GPIO_GPSET0 value
71:     uint32 Set0; // 0x1C
72:     /// @brief RPI_GPIO_GPSET1 value
73:     uint32 Set1; // 0x20
74:     /// @brief Reserved value
75:     uint32 Reserved_2; // 0x24
76:     /// @brief RPI_GPIO_GPCLR0 value
77:     uint32 Clear0; // 0x28
78:     /// @brief RPI_GPIO_GPCLR1 value
79:     uint32 Clear1; // 0x2C
80:     /// @brief Reserved value
81:     uint32 Reserved_3; // 0x30
82:     /// @brief RPI_GPIO_GPLEV0 value
83:     uint32 PinLevel0; // 0x34
84:     /// @brief RPI_GPIO_GPLEV1 value
85:     uint32 PinLevel1; // 0x38
86:     /// @brief Reserved value
87:     uint32 Reserved_4; // 0x3C
88:     /// @brief RPI_GPIO_GPEDS0 value
89:     uint32 EventDetectStatus0; // 0x40
90:     /// @brief RPI_GPIO_GPEDS1 value
91:     uint32 EventDetectStatus1; // 0x44
92:     /// @brief Reserved value
93:     uint32 Reserved_5; // 0x48
94:     /// @brief RPI_GPIO_GPREN0 value
95:     uint32 RisingEdgeDetectEn0; // 0x4C
96:     /// @brief RPI_GPIO_GPREN1 value
97:     uint32 RisingEdgeDetectEn1; // 0x50
98:     /// @brief Reserved value
99:     uint32 Reserved_6; // 0x54
100:     /// @brief RPI_GPIO_GPFEN0 value
101:     uint32 FallingEdgeDetectEn0; // 0x58
102:     /// @brief RPI_GPIO_GPFEN1 value
103:     uint32 FallingEdgeDetectEn1; // 0x5C
104:     /// @brief Reserved value
105:     uint32 Reserved_7; // 0x60
106:     /// @brief RPI_GPIO_GPHEN0 value
107:     uint32 HighDetectEn0; // 0x64
108:     /// @brief RPI_GPIO_GPHEN1 value
109:     uint32 HighDetectEn1; // 0x68
110:     /// @brief Reserved value
111:     uint32 Reserved_8; // 0x6C
112:     /// @brief RPI_GPIO_GPLEN0 value
113:     uint32 LowDetectEn0; // 0x70
114:     /// @brief RPI_GPIO_GPLEN1 value
115:     uint32 LowDetectEn1; // 0x74
116:     /// @brief Reserved value
117:     uint32 Reserved_9; // 0x78
118:     /// @brief RPI_GPIO_GPAREN0 value
119:     uint32 AsyncRisingEdgeDetectEn0; // 0x7C
120:     /// @brief RPI_GPIO_GPAREN1 value
121:     uint32 AsyncRisingEdgeDetectEn1; // 0x80
122:     /// @brief Reserved value
123:     uint32 Reserved_10; // 0x84
124:     /// @brief RPI_GPIO_GPAFEN0 value
125:     uint32 AsyncFallingEdgeDetectEn0; // 0x88
126:     /// @brief RPI_GPIO_GPAFEN1 value
127:     uint32 AsyncFallingEdgeDetectEn1; // 0x8C
128:     /// @brief Reserved value
129:     uint32 Reserved_11; // 0x90
130:     /// @brief RPI_GPIO_GPPUD value
131:     uint32 PullUpDownEnable; // 0x94
132:     /// @brief RPI_GPIO_GPPUDCLK0 value
133:     uint32 PullUpDownEnableClock0; // 0x98
134:     /// @brief RPI_GPIO_GPPUDCLK1 value
135:     uint32 PullUpDownEnableClock1; // 0x9C
136:     /// @brief Reserved value
137:     uint32 Reserved_12[4]; // 0xA0-AC
138:     /// @brief Test register value
139:     uint32 Test; // 0xB0
140: #if BAREMETAL_RPI_TARGET == 4
141:     /// @brief Reserved value
142:     uint32 Reserved_13;     // 0xB4
143:     uint32 Reserved_14[10]; // 0xB8-DC
144:     uint32 Reserved_15;     // 0xE0
145:     /// @brief RPI_GPIO_GPPUPPDN0 value
146:     uint32 PullUpDown0; // 0xE4
147:     /// @brief RPI_GPIO_GPPUPPDN1 value
148:     uint32 PullUpDown1; // 0xE8
149:     /// @brief RPI_GPIO_GPPUPPDN2 value
150:     uint32 PullUpDown2; // 0xEC
151:     /// @brief RPI_GPIO_GPPUPPDN3 value
152:     uint32 PullUpDown3; // 0xF0
153:     /// @brief Reserved value
154:     uint32 Reserved_16;    // 0xF4: Alignment
155:     uint32 Reserved_17[2]; // 0xF8-FC: Alignment
156: #endif
157: 
158:     /// <summary>
159:     /// Constructor for GPIORegisters
160:     ///
161:     /// Sets default register values
162:     /// </summary>
163:     GPIORegisters()
164:         : FunctionSelect0{}
165:         , FunctionSelect1{}
166:         , FunctionSelect2{}
167:         , FunctionSelect3{}
168:         , FunctionSelect4{}
169:         , FunctionSelect5{}
170:         , Reserved_1{}
171:         , Set0{}
172:         , Set1{}
173:         , Reserved_2{}
174:         , Clear0{}
175:         , Clear1{}
176:         , Reserved_3{}
177:         , PinLevel0{}
178:         , PinLevel1{}
179:         , Reserved_4{}
180:         , EventDetectStatus0{}
181:         , EventDetectStatus1{}
182:         , Reserved_5{}
183:         , RisingEdgeDetectEn0{}
184:         , RisingEdgeDetectEn1{}
185:         , Reserved_6{}
186:         , FallingEdgeDetectEn0{}
187:         , FallingEdgeDetectEn1{}
188:         , Reserved_7{}
189:         , HighDetectEn0{}
190:         , HighDetectEn1{}
191:         , Reserved_8{}
192:         , LowDetectEn0{}
193:         , LowDetectEn1{}
194:         , Reserved_9{}
195:         , AsyncRisingEdgeDetectEn0{}
196:         , AsyncRisingEdgeDetectEn1{}
197:         , Reserved_10{}
198:         , AsyncFallingEdgeDetectEn0{}
199:         , AsyncFallingEdgeDetectEn1{}
200:         , Reserved_11{}
201:         , PullUpDownEnable{}
202:         , PullUpDownEnableClock0{}
203:         , PullUpDownEnableClock1{}
204:         , Reserved_12{}
205:         , Test{}
206: #if BAREMETAL_RPI_TARGET == 4
207:         , Reserved_13{}
208:         , Reserved_14{}
209:         , Reserved_15{}
210:         , PullUpDown0{0xAAA95555}
211:         , PullUpDown1{0xA0AAAAAA}
212:         , PullUpDown2{0x50AAA95A}
213:         , PullUpDown3{0x00055555}
214:         , Reserved_16{}
215:         , Reserved_17{}
216: #endif
217:     {
218:     }
219: } PACKED;
220: 
221: /// @brief GPIO pin operation codes
222: enum PhysicalGPIOPinOperationCode
223: {
224:     /// @brief Get pin value
225:     GetPinValue,
226:     /// @brief Get pin event status
227:     GetPinEventStatus,
228:     /// @brief Set pin function
229:     SetPinMode,
230:     /// @brief Set pin value
231:     SetPinValue,
232:     /// @brief Clear pin event detect status
233:     ClearPinEventStatus,
234:     /// @brief Set pin rising edge interrupt enable
235:     SetPinRisingEdgeInterruptEnable,
236:     /// @brief Set pin falling edge interrupt enable
237:     SetPinFallingEdgeInterruptEnable,
238:     /// @brief Set pin high level interrupt enable
239:     SetPinHighLevelInterruptEnable,
240:     /// @brief Set pin low level interrupt enable
241:     SetPinLowLevelInterruptEnable,
242:     /// @brief Set pin async rising edge interrupt enable
243:     SetPinAsyncRisingEdgeInterruptEnable,
244:     /// @brief Set pin async falling edge interrupt enable
245:     SetPinAsyncFallingEdgeInterruptEnable,
246:     /// @brief Set pull up/down mode
247:     SetPullUpDownMode,
248:     /// @brief Set pin pull up/down clock
249:     SetPinPullUpDownClock,
250:     /// @brief Set pin mux mode
251:     SetPinMuxMode,
252:     /// @brief Set pin pull up/down mode
253:     SetPinPullUpDownMode,
254: };
255: 
256: /// <summary>
257: /// Data structure to contain a memory access operation
258: /// </summary>
259: struct PhysicalGPIOPinOperation
260: {
261:     /// @brief GPIO pin operation code
262:     PhysicalGPIOPinOperationCode operation; // Size: 4 bytes
263:     /// @brief GPIO pin number
264:     uint8 pin;                              // Size: 1 byte (padded with 3 bytes)
265:     /// @brief GPIO pin mode
266:     GPIOFunction function;                  // Size: 4 bytes
267:     /// @brief GPIO pin argument (if any)
268:     uint32 argument;                        // Size: 4 bytes
269: 
270:     /// <summary>
271:     /// Default constructor
272:     /// </summary>
273:     PhysicalGPIOPinOperation()
274:         : operation{}
275:         , pin{}
276:         , function{}
277:         , argument{}
278:     {
279:     }
280:      /// <summary>
281:     /// Constructor for read or 2write operation concerning pin function
282:     /// </summary>
283:     /// <param name="theOperation">Operation code</param>
284:     /// <param name="thePin">Pin number</param>
285:     /// <param name="theFunction">Pin function to be set</param>
286:     PhysicalGPIOPinOperation(PhysicalGPIOPinOperationCode theOperation, uint8 thePin, GPIOFunction theFunction)
287:         : operation{theOperation}
288:         , pin{thePin}
289:         , function{theFunction}
290:         , argument{}
291:     {
292:     }
293:     /// <summary>
294:     /// Constructor for read or write operation with other data
295:     /// </summary>
296:     /// <param name="theOperation">Operation code</param>
297:     /// <param name="thePin">Pin number</param>
298:     /// <param name="theArgument">Value read</param>
299:     PhysicalGPIOPinOperation(PhysicalGPIOPinOperationCode theOperation, uint8 thePin, uint32 theArgument)
300:         : operation{theOperation}
301:         , pin{thePin}
302:         , function{}
303:         , argument{theArgument}
304:     {
305:     }
306:     /// <summary>
307:     /// Check memory access operations for equality
308:     /// </summary>
309:     /// <param name="other">Value to compare to</param>
310:     /// <returns>True if equal, false otherwise</returns>
311:     bool operator==(const PhysicalGPIOPinOperation& other) const
312:     {
313:         return (other.operation == operation) &&
314:             (other.pin == pin) &&
315:             (other.function == function) &&
316:             (other.argument == argument);
317:     }
318:     /// <summary>
319:     /// Check memory access operations for inequality
320:     /// </summary>
321:     /// <param name="other">Value to compare to</param>
322:     /// <returns>True if unequal, false otherwise</returns>
323:     bool operator!=(const PhysicalGPIOPinOperation& other) const
324:     {
325:         return !operator==(other);
326:     }
327: };
328: 
329: String Serialize(const PhysicalGPIOPinOperation& value);
330: 
331: /// <summary>
332: /// Memory access mock class
333: /// </summary>
334: class MemoryAccessGPIOMock : public MemoryAccessMock
335: {
336: private:
337:     /// @brief Saved GPIO register values
338:     GPIORegisters m_registers;
339:     /// @brief Size of memory access operation array
340:     static constexpr size_t BufferSize = 1000;
341:     /// List op memory access operations
342:     PhysicalGPIOPinOperation m_accessOps[BufferSize] ALIGN(8);
343:     /// @brief Number of registered memory access operations
344:     size_t m_numAccessOps;
345: 
346: public:
347:     MemoryAccessGPIOMock();
348: 
349:     size_t GetNumGPIOOperations() const;
350:     const PhysicalGPIOPinOperation& GetGPIOOperation(size_t index) const;
351: 
352:     uint32 OnRead(regaddr address) override;
353:     void OnWrite(regaddr address, uint32 data) override;
354: 
355: private:
356:     uint32 GetRegisterOffset(regaddr address);
357:     void AddOperation(const PhysicalGPIOPinOperation& operation);
358: };
359: 
360: } // namespace baremetal
```

- Line 53-219: We define a structure `GPIORegisters` to hold the state of all GPIO registers (much like we did in the stub class)
- Line 221-254: We define an enumeration `PhysicalGPIOPinOperationCode` to hold the various GPIO pin operation types
- Line 256-327: We define a structure `PhysicalGPIOPinOperation` to hold the details of a GPIO pin operation
- Line 329: We declare a function `Serialize()` to convert a `PhysicalGPIOPinOperation` to a string
- Line 331-358: We define the class `MemoryAccessGPIOMock` inheriting from `MemoryAccessMock`, which implements the GPIO specific behavior
  - Line 337-338: We add a member variable `m_registers` to hold the GPIO register state
  - Line 339-340: We add a constant `BufferSize` to hold the size of the array op operations we store
  - Line 341-342: We add a member variable `m_accessOps` to hold the list of GPIO operations performed
  - Line 343-344: We add a member variable `m_numAccessOps` to hold the number of GPIO operations performed
  - Line 347: We declare the default constructor
  - Line 349: We declare the method `GetNumGPIOOperations()` to retrieve the number of GPIO operations performed
  - Line 350: We declare the method `GetGPIOOperation()` to retrieve a specific operation
  - Line 352-353: Override methods for read and write operations
  - Line 356: We declare the private method `GetRegisterOffset()` to get the register offset from an address
  - Line 357: We declare the private method `AddOperation()` to add a GPIO operation to the list

### MemoryAccessGPIOMock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_4_MEMORYACCESSGPIOMOCKCPP}

We'll now implement the methods of the `MemoryAccessGPIOMock` class.

Create the file `code\libraries\baremetal\src\mocks\MemoryAccessGPIOMock.cpp`

```cpp
File: code\libraries\baremetal\src\mocks\MemoryAccessGPIOMock.cpp
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
40: #include "baremetal/mocks/MemoryAccessGPIOMock.h"
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
58: /// <param name="function">Pin function</param>
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
233: size_t MemoryAccessGPIOMock::GetNumGPIOOperations() const
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
- Line 229-236: We implement the `GetNumGPIOOperations()` method
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

### PhysicalGPIOPinTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_4_PHYSICALGPIOPINTESTCPP}

Let's create a test.

Create the file `code\libraries\baremetal\test\src\PhysicalGPIOPinTest.cpp`

```cpp
File: code\libraries\baremetal\src\mocks\MemoryAccessMockGPIOTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : PhysicalGPIOPinTest.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : PhysicalGPIOPinTest
9: //
10: // Description : GPIO memory access mock tests
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
40: #include "baremetal/mocks/MemoryAccessGPIOMock.h"
41: 
42: #include "baremetal/BCMRegisters.h"
43: #include "baremetal/PhysicalGPIOPin.h"
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
56: class PhysicalGPIOPinTest : public TestFixture
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
67: TEST_FIXTURE(PhysicalGPIOPinTest, SimpleTest)
68: {
69:     MemoryAccessGPIOMock memoryAccess;
70:     PhysicalGPIOPin pin(memoryAccess);
71: 
72:     pin.AssignPin(0);
73:     pin.SetMode(GPIOMode::AlternateFunction0);
74: 
75:     size_t index{};
76: #if BAREMETAL_RPI_TARGET == 3
77:     EXPECT_EQ(size_t{4}, memoryAccess.GetNumGPIOOperations());
78:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(index++));
79:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000000, 0x1 }), memoryAccess.GetGPIOOperation(index++));
80:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000000, 0x0 }), memoryAccess.GetGPIOOperation(index++));
81: #else
82:     EXPECT_EQ(size_t{2}, memoryAccess.GetNumGPIOOperations());
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

As we are being selective about which tests to run, we need to update the main test application to add the tests for the `PhysicalGPIOPin`.

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
11:     RunSelectedTests(&reporter, InSelection("Baremetal", "PhysicalGPIOPinTest", nullptr));
12: //    RunAllTests(&reporter);
13: 
14:     return static_cast<int>(ReturnCode::ExitHalt);
15: }
```

- Line 11: We add the line to run the `PhysicalGPIOPinTest` tests

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
[  FIXTURE  ] PhysicalGPIOPinTest (1 test)
Info   0.00:00:00.050 Write32(3F200094, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.050 Write32(3F200098, 00000001) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.050 Write32(3F200098, 00000000) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.050 Read32(3F200000, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.060 Write32(3F200000, 00000004) (MemoryAccessMock.cpp:169)
[ SUCCEEDED ] Baremetal::PhysicalGPIOPinTest::SimpleTest
[  FIXTURE  ] 1 test from PhysicalGPIOPinTest
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
[  FIXTURE  ] PhysicalGPIOPinTest (1 test)
Info   0.00:00:00.050 Read32(FE2000E4, AAA95555)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.050 Write32(FE2000E4, AAA95554) (MemoryAccessMock.cpp:169)
Info   0.00:00:00.050 Read32(FE200000, 00000000)) (MemoryAccessMock.cpp:156)
Info   0.00:00:00.050 Write32(FE200000, 00000004) (MemoryAccessMock.cpp:169)
[ SUCCEEDED ] Baremetal::PhysicalGPIOPinTest::SimpleTest
[  FIXTURE  ] 1 test from PhysicalGPIOPinTest
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

Update the file `code\libraries\baremetal\include\baremetal\mocks\MemoryAccessMock.h`

```cpp
File: code\libraries\baremetal\include\baremetal\mocks\MemoryAccessMock.h
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
124:     size_t GetNumMemoryOperations() const;
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

Update the file `code\libraries\baremetal\src\mocks\MemoryAccessMock.cpp`

```cpp
File: code\libraries\baremetal\src\mocks\MemoryAccessMock.cpp
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

Update the file `code\libraries\baremetal\src\mocks\MemoryAccessGPIOMock.cpp`

```cpp
File: code\libraries\baremetal\src\mocks\MemoryAccessGPIOMock.cpp
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
40: #include "baremetal/mocks/MemoryAccessMock.h"
41: 
42: #include "baremetal/BCMRegisters.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/PhysicalGPIOPin.h"
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
[  FIXTURE  ] PhysicalGPIOPinTest (1 test)
Debug  0.00:00:00.040 GPIO Set Pin Pull Up/Down Mode None (MemoryAccessGPIOMock.cpp:578)
Debug  0.00:00:00.040 GPIO Set Pin 0 Pull Up/Down Enable Clock ON (MemoryAccessGPIOMock.cpp:595)
Debug  0.00:00:00.040 GPIO Set Pin 0 Pull Up/Down Enable Clock OFF (MemoryAccessGPIOMock.cpp:597)
Debug  0.00:00:00.040 GPIO Set Pin 0 Mode Alt0 (MemoryAccessGPIOMock.cpp:378)
[ SUCCEEDED ] Baremetal::PhysicalGPIOPinTest::SimpleTest
[  FIXTURE  ] 1 test from PhysicalGPIOPinTest
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
[  FIXTURE  ] PhysicalGPIOPinTest (1 test)
Debug  0.00:00:00.050 GPIO Set Pin 0 Pull Up/Down Mode None (MemoryAccessGPIOMock.cpp:626)
Debug  0.00:00:00.050 GPIO Set Pin 0 Mode Alt0 (MemoryAccessGPIOMock.cpp:378)
[ SUCCEEDED ] Baremetal::PhysicalGPIOPinTest::SimpleTest
[  FIXTURE  ] 1 test from PhysicalGPIOPinTest
[   SUITE   ] 4 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.070 Halt (System:121)
```

## Creating actual unit tests - Step 6 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_6}

Now we'll move on to the I2C unit tests.

As things move on, we'll notice that the unit tests will get more complex.

We'll take a close look at how we set up the stub for I2C master, and how we can use it in the unit tests.

Let first create a mock for the I2C master.

### MemoryAccessI2CMasterMock.h {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_6_MEMORYACCESSI2CMASTERMOCKH}

Create the file `code\libraries\baremetal\include\baremetal\mocks\MemoryAccessI2CMasterMock.h`

```cpp
File: code\libraries\baremetal\include\baremetal\mocks\MemoryAccessI2CMasterMock.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessI2CMasterMock.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessI2CMasterMock
9: //
10: // Description : I2C master memory access mock class
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
42: #include "baremetal/mocks/MemoryAccessGPIOMock.h"
43: 
44: #include "baremetal/I2CMaster.h"
45: #include "baremetal/String.h"
46: #include "stdlib/Macros.h"
47: 
48: /// @file
49: /// Memory access mock class
50: 
51: namespace baremetal {
52: 
53: /// @brief I2C master registers storage
54: struct I2CMasterRegisters
55: {
56:     /// @brief RPI_I2C_C value
57:     uint32 Control; // 0x00
58:     /// @brief RPI_I2C_S value
59:     uint32 Status; // 0x04
60:     /// @brief RPI_I2C_DLEN value
61:     uint32 DataLength; // 0x08
62:     /// @brief RPI_I2C_A value
63:     uint32 Address; // 0x0C
64:     /// @brief RPI_I2C_FIFO value on write
65:     uint32 FIFOWrite; // 0x10
66:     /// @brief RPI_I2C_DIV value
67:     uint32 ClockDivider; // 0x14
68:     /// @brief RPI_I2C_DEL value
69:     uint32 DataDelay; // 0x18
70:     /// @brief RPI_I2C_CLKT value
71:     uint32 ClockStretchTimeout; // 0x1C
72: 
73:     /// <summary>
74:     /// Constructor for I2CMasterRegisters
75:     ///
76:     /// Sets default register values
77:     /// </summary>
78:     I2CMasterRegisters()
79:         : Control{}
80:         , Status{0x00000050}
81:         , DataLength{}
82:         , Address{}
83:         , FIFOWrite{}
84:         , ClockDivider{0x000005DC}
85:         , DataDelay{0x00300030}
86:         , ClockStretchTimeout{0x00000040}
87:     {
88:     }
89: } PACKED;
90: 
91: /// @brief I2C master operation codes
92: enum I2CMasterOperationCode
93: {
94:     /// @brief Enable or disable I2C controller
95:     EnableController,
96:     /// @brief Enable RX interrupt
97:     EnableRXInterrupt,
98:     /// @brief Enable TX interrupt
99:     EnableTXInterrupt,
100:     /// @brief Enable Done interrupt
101:     EnableDoneInterrupt,
102:     /// @brief Set read mode
103:     SetReadMode,
104:     /// @brief Set write mode
105:     SetWriteMode,
106:     /// @brief Clear FIFO
107:     ClearFIFO,
108:     /// @brief Start transfer
109:     StartTransfer,
110:     /// @brief Reset clock stretch timeout
111:     ResetClockStretchTimeout,
112:     /// @brief Reset acknowledge error
113:     ResetAckError,
114:     /// @brief Reset done
115:     ResetDone,
116:     /// @brief Set data length
117:     SetDataLength,
118:     /// @brief Set address
119:     SetAddress,
120:     /// @brief Write to FIFO
121:     WriteFIFO,
122:     /// @brief Set clock divider
123:     SetClockDivider,
124:     /// @brief Set falling edge delay
125:     SetFallingEdgeDelay,
126:     /// @brief Set rising edge delay
127:     SetRisingEdgeDelay,
128:     /// @brief Set clock stretch timeout
129:     SetClockStretchTimeout,
130:     /// @brief Read from FIFO
131:     ReadFIFO,
132:     /// @brief Set acknowledge error
133:     SetAckError,
134: };
135: 
136: /// <summary>
137: /// Data structure to contain a memory access operation
138: /// </summary>
139: struct I2CMasterOperation
140: {
141:     /// @brief I2C master operation code
142:     I2CMasterOperationCode operation; // Size: 4 bytes
143:     /// @brief I2C bus
144:     uint8 bus; // Size: 1 bytes
145:     /// @brief Argument (if any)
146:     uint32 argument; // Size: 4 bytes
147: 
148:     /// <summary>
149:     /// Default constructor
150:     /// </summary>
151:     I2CMasterOperation()
152:         : operation{}
153:         , bus{}
154:         , argument{}
155:     {
156:     }
157:      /// <summary>
158:     /// Constructor for read or 2write operation concerning pin function
159:     /// </summary>
160:     /// <param name="theOperation">Operation code</param>
161:     /// <param name="theBus">I2C bus</param>
162:     /// <param name="theArgument">Argument value</param>
163:     I2CMasterOperation(I2CMasterOperationCode theOperation, uint8 theBus, uint32 theArgument = 0)
164:         : operation{theOperation}
165:         , bus{theBus}
166:         , argument{theArgument}
167:     {
168:     }
169:     /// <summary>
170:     /// Check memory access operations for equality
171:     /// </summary>
172:     /// <param name="other">Value to compare to</param>
173:     /// <returns>True if equal, false otherwise</returns>
174:     bool operator==(const I2CMasterOperation& other) const
175:     {
176:         return (other.operation == operation) &&
177:             (other.bus == bus) &&
178:             (other.argument == argument);
179:     }
180:     /// <summary>
181:     /// Check memory access operations for inequality
182:     /// </summary>
183:     /// <param name="other">Value to compare to</param>
184:     /// <returns>True if unequal, false otherwise</returns>
185:     bool operator!=(const I2CMasterOperation& other) const
186:     {
187:         return !operator==(other);
188:     }
189: } ALIGN(8);
190: 
191: String Serialize(const I2CMasterOperation& value);
192: 
193: #if BAREMETAL_RPI_TARGET == 3
194: /// @brief Number of I2C buses for RPI 3
195: #define I2C_BUSES 2
196: #else
197: /// @brief Number of I2C buses for RPI 4
198: #define I2C_BUSES 7
199: #endif
200: 
201: /// @brief FIFO size (both read and write)
202: #define I2C_FIFO_SIZE 16
203: 
204: /// @brief FIFO template class
205: template <int N>
206: class FIFO
207: {
208: private:
209:     /// @brief Data in FIFO (max 16 bytes)
210:     uint8 m_data[N]; // Size 16
211:     /// @brief Read index. If m_readIndex == m_writeIndex, the FIFO is either empty or full, depending on the full flag
212:     int m_readIndex; // Size 4
213:     /// @brief Write index. If m_readIndex == m_writeIndex, the FIFO is either empty or full, depending on the full flag
214:     int m_writeIndex; // Size 4
215:     /// @brief Flag whether the FIFO is full
216:     bool m_isFull; // Size 1
217:     /// @brief Force alignment to 16 bytes
218:     uint8 align[7]; // Size 7
219: 
220: public:
221:     /// <summary>
222:     /// FIFO constructor
223:     /// </summary>
224:     FIFO()
225:         : m_data{}
226:         , m_readIndex{}
227:         , m_writeIndex{}
228:         , m_isFull{}
229:     {
230:     }
231:     /// <summary>
232:     /// Read a byte from the FIFO
233:     /// </summary>
234:     /// <returns>Byte read. If nothing can be read, returns 0</returns>
235:     uint8 Read()
236:     {
237:         uint8 result{};
238:         if (!IsEmpty())
239:         {
240:             result = m_data[m_readIndex];
241:             m_readIndex = (m_readIndex + 1) % N;
242:             m_isFull = false;
243:         }
244:         return result;
245:     }
246:     /// <summary>
247:     /// Write a byte to the FIFO
248:     /// </summary>
249:     /// <param name="data">Data to write</param>
250:     void Write(uint8 data)
251:     {
252:         if (!IsFull())
253:         {
254:             m_data[m_writeIndex] = data;
255:             m_writeIndex = (m_writeIndex + 1) % N;
256:             if (m_readIndex == m_writeIndex)
257:                 m_isFull = true;
258:         }
259:     }
260:     /// <summary>
261:     /// Check if FIFO is empty
262:     /// </summary>
263:     /// <returns>true if FIFO is empty, false otherwise</returns>
264:     bool IsEmpty()
265:     {
266:         return (m_readIndex == m_writeIndex) && !m_isFull;
267:     }
268:     /// <summary>
269:     /// Check if FIFO is full
270:     /// </summary>
271:     /// <returns>true if FIFO is full, false otherwise</returns>
272:     bool IsFull()
273:     {
274:         return (m_readIndex == m_writeIndex) && m_isFull;
275:     }
276:     /// <summary>
277:     /// Get number of bytes used in FIFO
278:     /// </summary>
279:     /// <returns>Number of bytes used in FIFO</returns>
280:     size_t UsedSpace()
281:     {
282:         if (m_isFull)
283:             return N;
284:         return (m_writeIndex - m_readIndex + N) % N;
285:     }
286:     /// <summary>
287:     /// Get number of bytes free in FIFO
288:     /// </summary>
289:     /// <returns>Number of bytes free used in FIFO</returns>
290:     size_t FreeSpace()
291:     {
292:         return N - UsedSpace();
293:     }
294:     /// <summary>
295:     /// Check if FIFO is at most 25% full
296:     /// </summary>
297:     /// <returns>true if FIFO is at most 25% full, false otherwise</returns>
298:     bool IsOneQuarterOrLessFull()
299:     {
300:         return UsedSpace() <= (N / 4);
301:     }
302:     /// <summary>
303:     /// Check if FIFO is at least 75% full (at most 25% empty)
304:     /// </summary>
305:     /// <returns>true if FIFO is at least 75% full, false otherwise</returns>
306:     bool IsThreeQuartersOrMoreFull()
307:     {
308:         return FreeSpace() <= (N / 4);
309:     }
310:     /// <summary>
311:     /// Flush the FIFO
312:     /// </summary>
313:     void Flush()
314:     {
315:         m_readIndex = m_writeIndex = 0;
316:         m_isFull = false;
317:     }
318: } ALIGN(8);
319: 
320: /// @brief Callback for sending address
321: using SendAddressByteCallback = bool(I2CMasterRegisters& registers, uint8 address);
322: /// @brief Callback for receiving data
323: using RecvDataByteCallback = bool(I2CMasterRegisters& registers, uint8& data);
324: /// @brief Callback for sending data
325: using SendDataByteCallback = bool(I2CMasterRegisters& registers, uint8 data);
326: 
327: /// <summary>
328: /// Memory access mock class
329: /// </summary>
330: class MemoryAccessI2CMasterMock : public MemoryAccessGPIOMock
331: {
332: private:
333:     /// @brief Saved GPIO register values
334:     I2CMasterRegisters m_registers[I2C_BUSES] ALIGN(8);
335:     /// @brief Receive FIFO
336:     FIFO<I2C_FIFO_SIZE> m_rxFifo[I2C_BUSES] ALIGN(8);
337:     /// @brief Send FIFO
338:     FIFO<I2C_FIFO_SIZE> m_txFifo[I2C_BUSES] ALIGN(8);
339:     /// @brief Pointer to send address callback
340:     SendAddressByteCallback* m_sendAddressByteCallback;
341:     /// @brief Pointer to receive data callback
342:     RecvDataByteCallback* m_recvDataByteCallback;
343:     /// @brief Pointer to send data callback
344:     SendDataByteCallback* m_sendDataByteCallback;
345:     /// @brief Number of data bytes received
346:     uint8 m_numBytesReceived;
347:     /// @brief Number of data bytes sent
348:     uint8 m_numBytesSent;
349:     /// @brief Size of memory access operation array
350:     static constexpr size_t BufferSize = 1000;
351:     /// List op memory access operations
352:     I2CMasterOperation m_ops[BufferSize] ALIGN(8);
353:     /// @brief Number of registered memory access operations
354:     size_t m_numOps;
355: 
356: public:
357:     MemoryAccessI2CMasterMock();
358: 
359:     size_t GetNumI2CMasterOperations() const;
360:     const I2CMasterOperation& GetI2CMasterOperation(size_t index) const;
361: 
362:     uint32 OnRead(regaddr address) override;
363:     void OnWrite(regaddr address, uint32 data) override;
364: 
365:     void SetSendAddressByteCallback(SendAddressByteCallback callback);
366:     void SetRecvDataByteCallback(RecvDataByteCallback callback);
367:     void SetSendDataByteCallback(SendDataByteCallback callback);
368: 
369: private:
370:     bool InRangeForI2CMaster(regaddr address, uint8& bus);
371:     uint32 GetRegisterOffset(regaddr address, regaddr baseAddress);
372:     void AddOperation(const I2CMasterOperation& operation);
373:     void HandleWriteControlRegister(uint8 bus, uint32 data);
374:     void HandleWriteStatusRegister(uint8 bus, uint32 data);
375:     void HandleWriteFIFORegister(uint8 bus, uint8 data);
376:     uint8 HandleReadFIFORegister(uint8 bus);
377:     void HandleSendData(uint8 bus);
378:     void HandleRecvData(uint8 bus);
379:     void UpdateFIFOStatus(uint8 bus);
380:     void CancelTransfer(uint8 bus);
381:     void EndTransfer(uint8 bus);
382: };
383: 
384: } // namespace baremetal
```

- Line 53-89: We define a class `I2CMasterRegisters` to hold the register of a I2C master
- Line 91-134: We define an enumeration `I2CMasterOperationCode` to hold the operation codes for I2C master operations
- Line 136-189: We define a structure `I2CMasterOperation` to hold a memory access operation for the I2C master. It will hold the operation, the bus concerned, and an optional argument
- Line 191: We declare a function to serialize an I2C master operation for logging
- Line 193-199: We define the number of I2C buses depending on the target Raspberry Pi version
- Line 201-318: We define a template class `FIFO` to implement a simple FIFO buffer. The template parameter is the size of the FIFO
    - Line 211-216: Notice that we implement the FIFO as a circular buffer, which means we cannot distinguish between empty and full
    (in both cases the read and write index are equal), so we need to add a boolean to flag whether the FIFO is full
    - Line 217-218: We add a member variable `align` to make sure the class size is a multiple of 8 bytes
- Line 320-325: We define callback types for sending address bytes, receiving data bytes, and sending data bytes
- Line 327-382: We define the class `MemoryAccessI2CMasterMock`, derived from `MemoryAccessGPIOMock`, to implement the I2C master memory access mock

### MemoryAccessI2CMasterMock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_6_MEMORYACCESSI2CMASTERMOCKCPP}

Let's implement the `MemoryAccessI2CMasterMock` class.

Create the file `code\libraries\baremetal\src\mocks\MemoryAccessI2CMasterMock.cpp`

```cpp
File: code\libraries\baremetal\src\mocks\MemoryAccessI2CMasterMock.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessI2CMasterMock.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessI2CMasterMock
9: //
10: // Description : I2C master memory access mock class
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
40: #include "baremetal/mocks/MemoryAccessI2CMasterMock.h"
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
51: LOG_MODULE("MemoryAccessI2CMasterMock");
52: 
53: using namespace baremetal;
54: 
55: /// <summary>
56: /// Convert operation code to string
57: /// </summary>
58: /// <param name="code">Operation cpde</param>
59: /// <returns>String representing operator code</returns>
60: static String OperationCodeToString(I2CMasterOperationCode code)
61: {
62:     String result{};
63:     switch (code)
64:     {
65:     case I2CMasterOperationCode::EnableController:
66:         result = "EnableController";
67:         break;
68:     case I2CMasterOperationCode::EnableRXInterrupt:
69:         result = "EnableRXInterrupt";
70:         break;
71:     case I2CMasterOperationCode::EnableTXInterrupt:
72:         result = "EnableTXInterrupt";
73:         break;
74:     case I2CMasterOperationCode::EnableDoneInterrupt:
75:         result = "EnableDoneInterrupt";
76:         break;
77:     case I2CMasterOperationCode::SetReadMode:
78:         result = "SetReadMode";
79:         break;
80:     case I2CMasterOperationCode::SetWriteMode:
81:         result = "SetWriteMode";
82:         break;
83:     case I2CMasterOperationCode::ClearFIFO:
84:         result = "ClearFIFO";
85:         break;
86:     case I2CMasterOperationCode::StartTransfer:
87:         result = "StartTransfer";
88:         break;
89:     case I2CMasterOperationCode::ResetClockStretchTimeout:
90:         result = "ResetClockStretchTimeout";
91:         break;
92:     case I2CMasterOperationCode::ResetAckError:
93:         result = "ResetAckError";
94:         break;
95:     case I2CMasterOperationCode::ResetDone:
96:         result = "ResetDone";
97:         break;
98:     case I2CMasterOperationCode::SetDataLength:
99:         result = "SetDataLength";
100:         break;
101:     case I2CMasterOperationCode::SetAddress:
102:         result = "SetAddress";
103:         break;
104:     case I2CMasterOperationCode::WriteFIFO:
105:         result = "WriteFIFO";
106:         break;
107:     case I2CMasterOperationCode::SetClockDivider:
108:         result = "SetClockDivider";
109:         break;
110:     case I2CMasterOperationCode::SetFallingEdgeDelay:
111:         result = "SetFallingEdgeDelay";
112:         break;
113:     case I2CMasterOperationCode::SetRisingEdgeDelay:
114:         result = "SetRisingEdgeDelay";
115:         break;
116:     case I2CMasterOperationCode::SetClockStretchTimeout:
117:         result = "SetClockStretchTimeout";
118:         break;
119:     case I2CMasterOperationCode::ReadFIFO:
120:         result = "ReadFIFO";
121:         break;
122:     case I2CMasterOperationCode::SetAckError:
123:         result = "SetAckError";
124:         break;
125:     }
126:     return result;
127: }
128: 
129: /// <summary>
130: /// Serialize a I2C master memory access operation to string
131: /// </summary>
132: /// <param name="value">Value to be serialized</param>
133: /// <returns>Resulting string</returns>
134: String baremetal::Serialize(const I2CMasterOperation &value)
135: {
136:     String result = Format("Operation=%s, Bus=%s, ", OperationCodeToString(value.operation).c_str(), Format("%d", value.bus).c_str());
137:     switch (value.operation)
138:     {
139:     case I2CMasterOperationCode::EnableController:
140:     case I2CMasterOperationCode::EnableRXInterrupt:
141:     case I2CMasterOperationCode::EnableTXInterrupt:
142:     case I2CMasterOperationCode::EnableDoneInterrupt:
143:         result += Format("Enable=%s", (value.argument != 0 ? "ON" : "OFF"));
144:         break;
145:     case I2CMasterOperationCode::SetReadMode:
146:     case I2CMasterOperationCode::SetWriteMode:
147:     case I2CMasterOperationCode::ClearFIFO:
148:     case I2CMasterOperationCode::StartTransfer:
149:     case I2CMasterOperationCode::ResetClockStretchTimeout:
150:     case I2CMasterOperationCode::ResetAckError:
151:     case I2CMasterOperationCode::ResetDone:
152:         break;
153:     case I2CMasterOperationCode::SetDataLength:
154:         result += Format("Length=%d", value.argument);
155:         break;
156:     case I2CMasterOperationCode::SetAddress:
157:         result += Format("Address=%d", value.argument);
158:         break;
159:     case I2CMasterOperationCode::WriteFIFO:
160:         result += Format("Data=%d", value.argument);
161:         break;
162:     case I2CMasterOperationCode::SetClockDivider:
163:         result += Format("ClockDivider=%d", value.argument);
164:         break;
165:     case I2CMasterOperationCode::SetFallingEdgeDelay:
166:         result += Format("FallingEdgeDelay=%d", value.argument);
167:         break;
168:     case I2CMasterOperationCode::SetRisingEdgeDelay:
169:         result += Format("RisingEdgeDelay=%d", value.argument);
170:         break;
171:     case I2CMasterOperationCode::SetClockStretchTimeout:
172:         result += Format("ClockStretchTimeout=%d", value.argument);
173:         break;
174:     case I2CMasterOperationCode::ReadFIFO:
175:         result += Format("Data=%d", value.argument);
176:         break;
177:     case I2CMasterOperationCode::SetAckError:
178:         break;
179:     default:
180:         result += Format("Argument=%d", value.argument);
181:         break;
182:     }
183:     return result;
184: }
185: 
186: /// @brief GPIO base address
187: static regaddr I2CMasterBaseAddress[I2C_BUSES] = {
188:     RPI_I2C0_BASE,
189:     RPI_I2C1_BASE,
190: #if BAREMETAL_RPI_TARGET == 4
191:     0,
192:     RPI_I2C3_BASE,
193:     RPI_I2C4_BASE,
194:     RPI_I2C5_BASE,
195:     RPI_I2C6_BASE,
196: #endif
197: };
198: /// @brief Mask used to check whether an address is in the GPIO register range
199: static uintptr I2CMasterBaseAddressMask{0xFFFFFFFFFFFFFFE0};
200: 
201: /// <summary>
202: /// MemoryAccessI2CMasterMock constructor
203: /// </summary>
204: MemoryAccessI2CMasterMock::MemoryAccessI2CMasterMock()
205:     : m_registers{}
206:     , m_ops{}
207:     , m_numOps{}
208: {
209: }
210: 
211: /// <summary>
212: /// Return number of registered memory access operations
213: /// </summary>
214: /// <returns>Number of registered memory access operations</returns>
215: size_t MemoryAccessI2CMasterMock::GetNumI2CMasterOperations() const
216: {
217:     return m_numOps;
218: }
219: 
220: /// <summary>
221: /// Retrieve a registered memory access operation from the list
222: /// </summary>
223: /// <param name="index">Index of operation</param>
224: /// <returns>Requested memory access operation</returns>
225: const I2CMasterOperation &MemoryAccessI2CMasterMock::GetI2CMasterOperation(size_t index) const
226: {
227:     assert(index < m_numOps);
228:     return m_ops[index];
229: }
230: 
231: /// <summary>
232: /// Read a 32 bit value from register at address
233: /// </summary>
234: /// <param name="address">Address of register</param>
235: /// <returns>32 bit register value</returns>
236: uint32 MemoryAccessI2CMasterMock::OnRead(regaddr address)
237: {
238:     uint8 bus = 0xFF;
239:     if (!InRangeForI2CMaster(address, bus))
240:         return MemoryAccessGPIOMock::OnRead(address);
241: 
242:     uintptr offset = GetRegisterOffset(address, I2CMasterBaseAddress[bus]);
243:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers[bus]) + offset);
244:     TRACE_DEBUG("I2C Read register %016x = %08x", offset, *registerField);
245:     switch (offset)
246:     {
247:     case RPI_I2C_C_OFFSET:
248:         {
249:             String line{"I2C Read Control Register "};
250:             if (*registerField & RPI_I2C_C_ENABLE)
251:                 line += "Enable ON  ";
252:             else
253:                 line += "Enable OFF ";
254:             if (*registerField & RPI_I2C_C_INTR_ENABLE)
255:                 line += "RX Interrupt ON  ";
256:             else
257:                 line += "RX Interrupt OFF ";
258:             if (*registerField & RPI_I2C_C_INTT_ENABLE)
259:                 line += "TX Interrupt ON  ";
260:             else
261:                 line += "TX Interrupt OFF ";
262:             if (*registerField & RPI_I2C_C_INTD_ENABLE)
263:                 line += "DONE Interrupt ON  ";
264:             else
265:                 line += "DONE Interrupt OFF ";
266:             if (*registerField & RPI_I2C_C_READ)
267:                 line += "Read ";
268:             else
269:                 line += "Write ";
270:             TRACE_DEBUG(line.c_str());
271:             break;
272:         }
273:     case RPI_I2C_S_OFFSET:
274:         {
275:             String line{"I2C Read Status Register "};
276:             if (*registerField & RPI_I2C_S_CLKT)
277:                 line += "CLKT ";
278:             else
279:                 line += "     ";
280:             if (*registerField & RPI_I2C_S_ERR)
281:                 line += "NACK ";
282:             else
283:                 line += "ACK  ";
284:             if (*registerField & RPI_I2C_S_RXF)
285:                 line += "RXF ";
286:             else
287:                 line += "    ";
288:             if (*registerField & RPI_I2C_S_TXE)
289:                 line += "TXE ";
290:             else
291:                 line += "    ";
292:             if (*registerField & RPI_I2C_S_RXD)
293:                 line += "RXD ";
294:             else
295:                 line += "    ";
296:             if (*registerField & RPI_I2C_S_TXD)
297:                 line += "TXD ";
298:             else
299:                 line += "    ";
300:             if (*registerField & RPI_I2C_S_RXR)
301:                 line += "RXR ";
302:             else
303:                 line += "    ";
304:             if (*registerField & RPI_I2C_S_TXW)
305:                 line += "TXW ";
306:             else
307:                 line += "    ";
308:             if (*registerField & RPI_I2C_S_DONE)
309:                 line += "DONE ";
310:             else
311:                 line += "     ";
312:             if (*registerField & RPI_I2C_S_TA)
313:                 line += "TA ";
314:             else
315:                 line += "   ";
316:             TRACE_DEBUG(line.c_str());
317:             break;
318:         }
319:     case RPI_I2C_DLEN_OFFSET:
320:         {
321:             TRACE_DEBUG("I2C Get Data Length %d", *registerField);
322:             break;
323:         }
324:     case RPI_I2C_A_OFFSET:
325:         {
326:             TRACE_DEBUG("I2C Get Address %02x", *registerField);
327:             break;
328:         }
329:     case RPI_I2C_FIFO_OFFSET:
330:         {
331:             TRACE_DEBUG("I2C Read FIFO %02x", *registerField);
332:             AddOperation({I2CMasterOperationCode::ReadFIFO, bus, *registerField});
333:             *registerField = HandleReadFIFORegister(bus);
334:             break;
335:         }
336:     case RPI_I2C_DIV_OFFSET:
337:         {
338:             TRACE_DEBUG("I2C Set Clock Divider %d", *registerField);
339:             break;
340:         }
341:     case RPI_I2C_DEL_OFFSET:
342:         {
343:             if (*registerField & 0xFFFF0000)
344:             {
345:                 TRACE_DEBUG("I2C Get Falling Edge Delay %d", (*registerField >> 16));
346:             }
347:             if (*registerField & 0x0000FFFF)
348:             {
349:                 TRACE_DEBUG("I2C Get Rising Edge Delay %d", (*registerField & 0x0000FFFF));
350:             }
351:             break;
352:         }
353:     case RPI_I2C_CLKT_OFFSET:
354:         {
355:             TRACE_DEBUG("I2C Get Clock Stretch Timeout %d", (*registerField & 0x0000FFFF));
356:             break;
357:         }
358:     default:
359:         LOG_ERROR("Invalid I2C register access for reading: offset %d", offset);
360:         break;
361:     }
362:     return *registerField;
363: }
364: 
365: /// <summary>
366: /// Write a 32 bit value to register at address
367: /// </summary>
368: /// <param name="address">Address of register</param>
369: /// <param name="data">Data to write</param>
370: void MemoryAccessI2CMasterMock::OnWrite(regaddr address, uint32 data)
371: {
372:     uint8 bus = 0xFF;
373:     if (!InRangeForI2CMaster(address, bus))
374:     {
375:         MemoryAccessGPIOMock::OnWrite(address, data);
376:         return;
377:     }
378: 
379:     uintptr offset = GetRegisterOffset(address, I2CMasterBaseAddress[bus]);
380:     uint32* registerField = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_registers[bus]) + offset);
381:     TRACE_DEBUG("I2C Write register %016x = %08x", offset, data);
382:     switch (offset)
383:     {
384:     case RPI_I2C_C_OFFSET:
385:         {
386:             uint32 diff = (data ^ *registerField) | 0x00B0;
387:             if (diff & RPI_I2C_C_ENABLE)
388:             {
389:                 AddOperation({I2CMasterOperationCode::EnableController, bus, (data & RPI_I2C_C_ENABLE) ? 1u : 0u});
390:             }
391:             if (diff & RPI_I2C_C_INTR_ENABLE)
392:             {
393:                 AddOperation({I2CMasterOperationCode::EnableRXInterrupt, bus, (data & RPI_I2C_C_INTR_ENABLE) ? 1u : 0u});
394:             }
395:             if (diff & RPI_I2C_C_INTT_ENABLE)
396:             {
397:                 AddOperation({I2CMasterOperationCode::EnableTXInterrupt, bus, (data & RPI_I2C_C_INTT_ENABLE) ? 1u : 0u});
398:             }
399:             if (diff & RPI_I2C_C_INTD_ENABLE)
400:             {
401:                 AddOperation({I2CMasterOperationCode::EnableDoneInterrupt, bus, (data & RPI_I2C_C_INTD_ENABLE) ? 1u : 0u});
402:             }
403:             if (diff & RPI_I2C_C_READ)
404:             {
405:                 if (data & RPI_I2C_C_READ)
406:                     AddOperation({I2CMasterOperationCode::SetReadMode, bus});
407:                 else
408:                     AddOperation({I2CMasterOperationCode::SetWriteMode, bus});
409:             }
410:             if (data & RPI_I2C_C_CLEAR)
411:                 AddOperation({I2CMasterOperationCode::ClearFIFO, bus});
412:             if (data & RPI_I2C_C_ST)
413:                 AddOperation({I2CMasterOperationCode::StartTransfer, bus});
414:             HandleWriteControlRegister(bus, data);
415:             break;
416:         }
417:     case RPI_I2C_S_OFFSET:
418:         {
419:             if (data & RPI_I2C_S_CLKT)
420:                 AddOperation({I2CMasterOperationCode::ResetClockStretchTimeout, bus});
421:             if (data & RPI_I2C_S_ERR)
422:                 AddOperation({I2CMasterOperationCode::ResetAckError, bus});
423:             if (data & RPI_I2C_S_DONE)
424:                 AddOperation({I2CMasterOperationCode::ResetDone, bus});
425:             HandleWriteStatusRegister(bus, data);
426:             break;
427:         }
428:     case RPI_I2C_DLEN_OFFSET:
429:         {
430:             AddOperation({I2CMasterOperationCode::SetDataLength, bus, data});
431:             *registerField = data;
432:             break;
433:         }
434:     case RPI_I2C_A_OFFSET:
435:         {
436:             AddOperation({I2CMasterOperationCode::SetAddress, bus, data});
437:             *registerField = data;
438:             break;
439:         }
440:     case RPI_I2C_FIFO_OFFSET:
441:         {
442:             AddOperation({I2CMasterOperationCode::WriteFIFO, bus, data});
443:             HandleWriteFIFORegister(bus, data);
444:             break;
445:         }
446:     case RPI_I2C_DIV_OFFSET:
447:         {
448:             if (data != *registerField)
449:             {
450:                 AddOperation({I2CMasterOperationCode::SetClockDivider, bus, data});
451:             }
452:             *registerField = data;
453:             break;
454:         }
455:     case RPI_I2C_DEL_OFFSET:
456:         {
457:             uint32 diff = data & *registerField;
458:             if (diff & 0xFFFF0000)
459:             {
460:                 AddOperation({I2CMasterOperationCode::SetFallingEdgeDelay, bus, (data >> 16)});
461:             }
462:             if (diff & 0x0000FFFF)
463:             {
464:                 AddOperation({I2CMasterOperationCode::SetRisingEdgeDelay, bus, (data & 0xFFFF)});
465:             }
466:             *registerField = data;
467:             break;
468:         }
469:     case RPI_I2C_CLKT_OFFSET:
470:         {
471:             uint32 diff = data & *registerField;
472:             if (diff & 0x0000FFFF)
473:             {
474:                 AddOperation({I2CMasterOperationCode::SetClockStretchTimeout, bus, (data & 0xFFFF)});
475:             }
476:             *registerField = data;
477:             break;
478:         }
479:     default:
480:         LOG_ERROR("Invalid I2C register access for writing: offset %d", offset);
481:         break;
482:     }
483: }
484: 
485: /// <summary>
486: /// Set callback function for address send
487: /// </summary>
488: /// <param name="callback">Address of callback function</param>
489: void MemoryAccessI2CMasterMock::SetSendAddressByteCallback(SendAddressByteCallback callback)
490: {
491:     m_sendAddressByteCallback = callback;
492: }
493: 
494: /// <summary>
495: /// Set callback function for data receive
496: /// </summary>
497: /// <param name="callback">Address of callback function</param>
498: void MemoryAccessI2CMasterMock::SetRecvDataByteCallback(RecvDataByteCallback callback)
499: {
500:     m_recvDataByteCallback = callback;
501: }
502: 
503: /// <summary>
504: /// Set callback function for data send
505: /// </summary>
506: /// <param name="callback">Address of callback function</param>
507: void MemoryAccessI2CMasterMock::SetSendDataByteCallback(SendDataByteCallback callback)
508: {
509:     m_sendDataByteCallback = callback;
510: }
511: 
512: /// <summary>
513: /// Determined if address is in range for I2C master registers for any bus, return the bus index if so
514: /// </summary>
515: /// <param name="address">Address to check</param>
516: /// <param name="bus">I2C bus if address is in range for this bus</param>
517: /// <returns>True if address is in range for registers of a I2C bus master, false otherwise</returns>
518: bool MemoryAccessI2CMasterMock::InRangeForI2CMaster(regaddr address, uint8 &bus)
519: {
520:     for (uint8 i = 0; i < I2C_BUSES; ++i)
521:     {
522:         uintptr baseAddr = reinterpret_cast<uintptr>(I2CMasterBaseAddress[i]);
523:         if ((reinterpret_cast<uintptr>(address) & I2CMasterBaseAddressMask) == baseAddr)
524:         {
525:             bus = i;
526:             return true;
527:         }
528:     }
529:     return false;
530: }
531: 
532: /// <summary>
533: /// Determine register address offset relative to GPIO base address
534: ///
535: /// If the address is not in the correct range, an assert is fired
536: /// </summary>
537: /// <param name="address">Address to check</param>
538: /// <param name="baseAddress">Base address for I2C bus master</param>
539: /// <returns>Offset relative to I2C master base address</returns>
540: uint32 MemoryAccessI2CMasterMock::GetRegisterOffset(regaddr address, regaddr baseAddress)
541: {
542:     return reinterpret_cast<uintptr>(address) - reinterpret_cast<uintptr>(baseAddress);
543: }
544: 
545: /// <summary>
546: /// Add a memory access operation to the list
547: /// </summary>
548: /// <param name="operation">Operation to add</param>
549: void MemoryAccessI2CMasterMock::AddOperation(const I2CMasterOperation& operation)
550: {
551:     assert(m_numOps < BufferSize);
552:     m_ops[m_numOps++] = operation;
553: }
554: 
555: /// <summary>
556: /// Handle writing to I2C Control Register
557: /// </summary>
558: /// <param name="bus">I2C bus</param>
559: /// <param name="data">Value to write to the register</param>
560: void MemoryAccessI2CMasterMock::HandleWriteControlRegister(uint8 bus, uint32 data)
561: {
562:     if ((data & RPI_I2C_C_CLEAR) != 0)
563:     {
564:         m_rxFifo[bus].Flush();
565:         m_txFifo[bus].Flush();
566:         m_numBytesReceived = 0;
567:         m_numBytesSent = 0;
568:     }
569:     m_registers[bus].Control = data & ~(RPI_I2C_C_CLEAR | RPI_I2C_C_ST);
570:     if (((data & RPI_I2C_C_ENABLE) != 0) && ((data & RPI_I2C_C_ST) != 0))
571:     {
572:         // We started a (new) transaction
573:         m_registers[bus].Status = m_registers[bus].Status | RPI_I2C_S_TA;
574:         if (m_sendAddressByteCallback)
575:         {
576:             if (!(*m_sendAddressByteCallback)(m_registers[bus], m_registers[bus].Address))
577:             {
578:                 m_registers[bus].Status |= RPI_I2C_S_ERR;
579:                 AddOperation({I2CMasterOperationCode::SetAckError, bus});
580:             }
581:         }
582:         if ((m_registers[bus].Control & RPI_I2C_C_READ) != 0)
583:         {
584:             HandleRecvData(bus);
585:         }
586:         else
587:         {
588:             HandleSendData(bus);
589:         }
590:     }
591: }
592: 
593: /// <summary>
594: /// Handle writing to I2C Status Register
595: /// </summary>
596: /// <param name="bus">I2C bus</param>
597: /// <param name="data">Value to write to the register</param>
598: void MemoryAccessI2CMasterMock::HandleWriteStatusRegister(uint8 bus, uint32 data)
599: {
600:     if (data & RPI_I2C_S_ERR)
601:     {
602:         m_registers[bus].Status = m_registers[bus].Status & ~RPI_I2C_S_ERR;
603:     }
604:     if (data & RPI_I2C_S_DONE)
605:     {
606:         m_registers[bus].Status = m_registers[bus].Status & ~(RPI_I2C_S_DONE | RPI_I2C_S_TA);
607:     }
608:     if (data & RPI_I2C_S_CLKT)
609:     {
610:         m_registers[bus].Status = m_registers[bus].Status & ~RPI_I2C_S_CLKT;
611:     }
612: }
613: 
614: /// <summary>
615: /// Handle writing to I2C Transmit FIFO Register
616: /// </summary>
617: /// <param name="bus">I2C bus</param>
618: /// <param name="data">Value to write to the register</param>
619: void MemoryAccessI2CMasterMock::HandleWriteFIFORegister(uint8 bus, uint8 data)
620: {
621:     if (!m_txFifo[bus].IsFull())
622:     {
623:         m_txFifo[bus].Write(data);
624:     }
625:     UpdateFIFOStatus(bus);
626:     HandleSendData(bus);
627: }
628: 
629: /// <summary>
630: /// Handle reading to I2C Receive FIFO Register
631: /// </summary>
632: /// <param name="bus">I2C bus</param>
633: /// <returns>Value read from the FIFO</returns>
634: uint8 MemoryAccessI2CMasterMock::HandleReadFIFORegister(uint8 bus)
635: {
636:     uint8 result{};
637:     HandleRecvData(bus);
638:     if (!m_rxFifo[bus].IsEmpty())
639:     {
640:         result = m_rxFifo[bus].Read();
641:     }
642:     UpdateFIFOStatus(bus);
643:     return result;
644: }
645: 
646: /// <summary>
647: /// Handle transmitting data
648: /// </summary>
649: /// <param name="bus">I2C bus</param>
650: void MemoryAccessI2CMasterMock::HandleSendData(uint8 bus)
651: {
652:     if (((m_registers[bus].Status & RPI_I2C_S_TA) != 0) && ((m_registers[bus].Control & RPI_I2C_C_READ) == 0))
653:     {
654:         while (!m_txFifo[bus].IsEmpty() && (m_numBytesSent < m_registers[bus].DataLength))
655:         {
656:             auto data = m_txFifo[bus].Read();
657:             UpdateFIFOStatus(bus);
658:             bool sentData{};
659:             if (m_sendDataByteCallback)
660:                 sentData = (*m_sendDataByteCallback)(m_registers[bus], data);
661:             if (sentData)
662:                 ++m_numBytesSent;
663:             else
664:             {
665:                 CancelTransfer(bus);
666:                 break;
667:             }
668:         }
669:     }
670:     if (m_numBytesSent >= m_registers[bus].DataLength)
671:         EndTransfer(bus);
672: }
673: 
674: /// <summary>
675: /// Handle receiving data
676: /// </summary>
677: /// <param name="bus">I2C bus</param>
678: void MemoryAccessI2CMasterMock::HandleRecvData(uint8 bus)
679: {
680:     if (((m_registers[bus].Control & RPI_I2C_C_ENABLE) != 0) && ((m_registers[bus].Control & RPI_I2C_C_READ) != 0))
681:     {
682:         while (!m_rxFifo[bus].IsFull() && (m_numBytesReceived < m_registers[bus].DataLength))
683:         {
684:             uint8 data{};
685:             bool receivedData{};
686:             if (m_recvDataByteCallback)
687:             {
688:                 receivedData = (*m_recvDataByteCallback)(m_registers[bus], data);
689:             }
690:             if (receivedData)
691:             {
692:                 m_rxFifo[bus].Write(data);
693:                 UpdateFIFOStatus(bus);
694:                 ++m_numBytesReceived;
695:             }
696:             else
697:             {
698:                 CancelTransfer(bus);
699:                 break;
700:             }
701:         }
702:     }
703:     if (m_numBytesReceived >= m_registers[bus].DataLength)
704:         EndTransfer(bus);
705: }
706: 
707: /// <summary>
708: /// Update the FIFO status bits in the I2C Status Register
709: /// </summary>
710: /// <param name="bus">I2C bus</param>
711: void MemoryAccessI2CMasterMock::UpdateFIFOStatus(uint8 bus)
712: {
713:     auto& txFifo = m_txFifo[bus];
714:     auto& rxFifo = m_rxFifo[bus];
715:     if (txFifo.IsEmpty())
716:         m_registers[bus].Status |= RPI_I2C_S_TXE;
717:     else
718:         m_registers[bus].Status &= (~RPI_I2C_S_TXE);
719:     if (txFifo.IsFull())
720:         m_registers[bus].Status |= RPI_I2C_S_TXD;
721:     else
722:         m_registers[bus].Status &= (~RPI_I2C_S_TXD);
723:     if (txFifo.IsOneQuarterOrLessFull())
724:         m_registers[bus].Status |= RPI_I2C_S_TXW;
725:     else
726:         m_registers[bus].Status &= (~RPI_I2C_S_TXW);
727:     if (rxFifo.IsEmpty())
728:         m_registers[bus].Status &= (~RPI_I2C_S_RXD);
729:     else
730:         m_registers[bus].Status |= RPI_I2C_S_RXD;
731:     if (rxFifo.IsFull())
732:         m_registers[bus].Status |= RPI_I2C_S_RXF;
733:     else
734:         m_registers[bus].Status &= (~RPI_I2C_S_RXF);
735:     if (rxFifo.IsThreeQuartersOrMoreFull())
736:         m_registers[bus].Status |= RPI_I2C_S_RXR;
737:     else
738:         m_registers[bus].Status &= (~RPI_I2C_S_RXR);
739: }
740: 
741: /// <summary>
742: /// Cancel a transfer, i.e. set error flag to denote NACK, and set done flag
743: /// </summary>
744: /// <param name="bus">I2C bus</param>
745: void MemoryAccessI2CMasterMock::CancelTransfer(uint8 bus)
746: {
747:     m_registers[bus].Status |= (RPI_I2C_S_ERR | RPI_I2C_S_DONE);
748:     m_registers[bus].Status &= (~RPI_I2C_S_TA);
749: }
750: 
751: /// <summary>
752: /// Finalize a transfer, i.e. set done flag
753: /// </summary>
754: /// <param name="bus">I2C bus</param>
755: void MemoryAccessI2CMasterMock::EndTransfer(uint8 bus)
756: {
757:     m_registers[bus].Status |= (RPI_I2C_S_DONE);
758:     m_registers[bus].Status &= (~RPI_I2C_S_TA);
759: }
```

- Line 55-127: We add a static function `OperationCodeToString` that converts an `I2CMasterOperationCode` enum value to its corresponding string representation
- Line 129-184: We implement the `Serialize` function for `I2CMasterOperation`, which converts an operation into a human-readable string format
- Line 186-197: We define the base addresses for the I2C master registers for each bus
- Line 199: We define a mask to check if an address is in the I2C master register range
- Line 201-209: We implement the constructor for `MemoryAccessI2CMasterMock`, initializing member variables
- Line 211-218: We implement `GetNumI2CMasterOperations()` to return the number of recorded I2C operations
- Line 220-229: We implement `GetI2CMasterOperation()` to retrieve a specific recorded I2C operation
- Line 231-363: We implement `OnRead()` to handle reading from I2C master registers, updating internal state and recording operations as needed
- Line 365-483: We implement `OnWrite()` to handle writing to I2C master registers, updating internal state and recording operations as needed
- Line 485-492: We implement `SetSendAddressByteCallback()` to set a callback for sending address bytes
- Line 494-501: We implement `SetRecvDataByteCallback()` to set a callback for receiving data bytes
- Line 503-510: We implement `SetSendDataByteCallback()` to set a callback for sending data bytes
- Line 512-530: We implement `InRangeForI2CMaster()` to check if an address belongs to any I2C master bus and return the bus index if any
- Line 532-543: We implement `GetRegisterOffset()` to calculate the offset of a register address relative to the I2C master base address
- Line 545-553: We implement `AddOperation()` to record an I2C operation in the internal buffer
- Line 555-591: We implement `HandleWriteControlRegister()` to process writes to the I2C control register
- Line 593-612: We implement `HandleWriteStatusRegister()` to process writes to the I2C status register
- Line 614-627: We implement `HandleWriteFIFORegister()` to process writes to the I2C FIFO register
- Line 629-644: We implement `HandleReadFIFORegister()` to process reads from the I2C FIFO register
- Line 646-672: We implement `HandleSendData()` to manage sending data over I2C
- Line 674-705: We implement `HandleRecvData()` to manage receiving data over I2C
- Line 707-739: We implement `UpdateFIFOStatus()` to update the status bits related to the FIFOs in the I2C status register
- Line 741-749: We implement `CancelTransfer()` to handle the cancellation of an I2C transfer
- Line 751-759: We implement `EndTransfer()` to finalize an I2C transfer

This code is quite similar to the I2C stub implementation we creater earlier, but it adds detailed logging and operation recording for testing purposes.

### FIFOTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_6_FIFOTESTCPP}

Let's write some tests for the FIFO first.

Create the file `code\libraries\baremetal\test\src\FIFOTest.cpp`

```cpp
File: code\libraries\baremetal\test\src\FIFOTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : FIFOTest.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : FIFOTest
9: //
10: // Description : FIFO tests
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
40: #include "baremetal/mocks/MemoryAccessI2CMasterMock.h"
41: 
42: #include "baremetal/Logger.h"
43: 
44: #include "unittest/unittest.h"
45: 
46: /// @brief Define log name
47: LOG_MODULE("FIFOTest");
48: 
49: using namespace unittest;
50: 
51: namespace baremetal {
52: namespace test {
53: 
54: /// @brief Baremetal test suite
55: TEST_SUITE(Baremetal)
56: {
57: 
58: class FIFOTest : public TestFixture
59: {
60: public:
61:     void SetUp() override
62:     {
63:     }
64:     void TearDown() override
65:     {
66:     }
67: };
68: 
69: TEST_FIXTURE(FIFOTest, FIFOIsInitiallyEmpty)
70: {
71:     constexpr size_t N = 4;
72:     FIFO<N> fifo;
73:     EXPECT_TRUE(fifo.IsEmpty());
74:     EXPECT_FALSE(fifo.IsFull());
75:     EXPECT_TRUE(fifo.IsOneQuarterOrLessFull());
76:     EXPECT_FALSE(fifo.IsThreeQuartersOrMoreFull());
77:     EXPECT_EQ(size_t{0}, fifo.UsedSpace());
78:     EXPECT_EQ(size_t{4}, fifo.FreeSpace());
79: }
80: 
81: TEST_FIXTURE(FIFOTest, FIFOOneQuarterFull)
82: {
83:     constexpr size_t N = 4;
84:     FIFO<N> fifo;
85:     fifo.Write(0x12);
86:     EXPECT_FALSE(fifo.IsEmpty());
87:     EXPECT_FALSE(fifo.IsFull());
88:     EXPECT_TRUE(fifo.IsOneQuarterOrLessFull());
89:     EXPECT_FALSE(fifo.IsThreeQuartersOrMoreFull());
90:     EXPECT_EQ(size_t{1}, fifo.UsedSpace());
91:     EXPECT_EQ(size_t{3}, fifo.FreeSpace());
92: }
93: 
94: TEST_FIXTURE(FIFOTest, FIFOHalfFull)
95: {
96:     constexpr size_t N = 4;
97:     FIFO<N> fifo;
98:     fifo.Write(0x12);
99:     fifo.Write(0x34);
100:     EXPECT_FALSE(fifo.IsEmpty());
101:     EXPECT_FALSE(fifo.IsFull());
102:     EXPECT_FALSE(fifo.IsOneQuarterOrLessFull());
103:     EXPECT_FALSE(fifo.IsThreeQuartersOrMoreFull());
104:     EXPECT_EQ(size_t{2}, fifo.UsedSpace());
105:     EXPECT_EQ(size_t{2}, fifo.FreeSpace());
106: }
107: 
108: TEST_FIXTURE(FIFOTest, FIFOThreeQuartersFull)
109: {
110:     constexpr size_t N = 4;
111:     FIFO<N> fifo;
112:     fifo.Write(0x12);
113:     fifo.Write(0x34);
114:     fifo.Write(0x56);
115:     EXPECT_FALSE(fifo.IsEmpty());
116:     EXPECT_FALSE(fifo.IsFull());
117:     EXPECT_FALSE(fifo.IsOneQuarterOrLessFull());
118:     EXPECT_TRUE(fifo.IsThreeQuartersOrMoreFull());
119:     EXPECT_EQ(size_t{3}, fifo.UsedSpace());
120:     EXPECT_EQ(size_t{1}, fifo.FreeSpace());
121: }
122: 
123: TEST_FIXTURE(FIFOTest, FIFOFull)
124: {
125:     constexpr size_t N = 4;
126:     FIFO<N> fifo;
127:     fifo.Write(0x12);
128:     fifo.Write(0x34);
129:     fifo.Write(0x56);
130:     fifo.Write(0x78);
131:     EXPECT_FALSE(fifo.IsEmpty());
132:     EXPECT_TRUE(fifo.IsFull());
133:     EXPECT_FALSE(fifo.IsOneQuarterOrLessFull());
134:     EXPECT_TRUE(fifo.IsThreeQuartersOrMoreFull());
135:     EXPECT_EQ(size_t{4}, fifo.UsedSpace());
136:     EXPECT_EQ(size_t{0}, fifo.FreeSpace());
137: }
138: 
139: TEST_FIXTURE(FIFOTest, FIFOFullThenFlushed)
140: {
141:     constexpr size_t N = 4;
142:     FIFO<N> fifo;
143:     fifo.Write(0x12);
144:     fifo.Write(0x34);
145:     fifo.Write(0x56);
146:     fifo.Write(0x78);
147:     fifo.Flush();
148:     EXPECT_TRUE(fifo.IsEmpty());
149:     EXPECT_EQ(size_t{0}, fifo.UsedSpace());
150:     EXPECT_EQ(size_t{4}, fifo.FreeSpace());
151: }
152: 
153: } // suite Baremetal
154: 
155: } // namespace test
156: } // namespace baremetal
```

- Line 69-79: Test that a newly created FIFO is empty
- Line 81-92: Test that a FIFO with one element is one quarter full
- Line 94-106: Test that a FIFO with two elements is half full
- LIne 108-121: Test that a FIFO with three elements is three quarters full
- LIne 123-137: Test that a FIFO with four elements is full
- Line 139-151: Test that flushing a full FIFO makes it empty again

### Extend selected tests {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_6_EXTEND_SELECTED_TESTS}

We need to update the main test application to add the tests for the `FIFO`.

Update the file `code\libraries\baremetal\test\src\main.cpp`

```cpp
File: code\libraries\baremetal\test\src\main.cpp
1: #include "baremetal/System.h"
2: #include "baremetal/Logger.h"
3: #include "unittest/unittest.h"
4: 
5: using namespace baremetal;
6: using namespace unittest;
7: 
8: int main()
9: {
10:     ConsoleTestReporter reporter;
11:     GetLogger().SetLogLevel(LogSeverity::Info);
12:     RunSelectedTests(&reporter, InSelection("Baremetal", "MemoryAccessMockTest", nullptr));
13:     RunSelectedTests(&reporter, InSelection("Baremetal", "PhysicalGPIOPinTest", nullptr));
14:     RunSelectedTests(&reporter, InSelection("Baremetal", "FIFOTest", nullptr));
15: //    RunAllTests(&reporter);
16: 
17:     return static_cast<int>(ReturnCode::ExitHalt);
18: }
```

Note that we also changed the log level to Info, so we have less cluttering in the console output.

### Configuring, building and debugging {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_6_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. All tests should succeed.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.000 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (6 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 6 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (6 fixtures)
[  FIXTURE  ] PhysicalGPIOPinTest (1 test)
[ SUCCEEDED ] Baremetal::PhysicalGPIOPinTest::SimpleTest
[  FIXTURE  ] 1 test from PhysicalGPIOPinTest
[   SUITE   ] 6 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
[===========] Running 6 tests from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (6 fixtures)
[  FIXTURE  ] FIFOTest (6 tests)
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOIsInitiallyEmpty
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOOneQuarterFull
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOHalfFull
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOThreeQuartersFull
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOFull
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOFullThenFlushed
[  FIXTURE  ] 6 tests from FIFOTest
[   SUITE   ] 6 fixtures from Baremetal
Success: 6 tests passed.

No failures
[===========] 6 tests from 1 fixture in 1 suite ran.
Info   0.00:00:00.060 Halt (System:121)
```

## Creating actual unit tests - Step 7 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_7}

### I2CMasterTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_7_I2CMASTERTESTCPP}

Let's write tests for the `I2CMaster` class.

We'll write a test where the is no I2C device at the given address, and we expect a NACK error.
We'll also write a test where a device is present and responds correctly.

Create the file `code\libraries\baremetal\test\src\I2CMasterTest.cpp`

```cpp
File: code\libraries\baremetal\test\src\I2CMasterTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : I2CMasterTest.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : I2CMasterTest
9: //
10: // Description : I2C master tests
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
40: #include "baremetal/mocks/MemoryAccessI2CMasterMock.h"
41: 
42: #include "baremetal/BCMRegisters.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/I2CMaster.h"
45: 
46: #include "unittest/unittest.h"
47: 
48: /// @brief Define log name
49: LOG_MODULE("I2CMasterTest");
50: 
51: using namespace unittest;
52: 
53: namespace baremetal {
54: namespace test {
55: 
56: /// @brief Baremetal test suite
57: TEST_SUITE(Baremetal)
58: {
59: 
60: class I2CMasterTest : public TestFixture
61: {
62: public:
63:     void SetUp() override
64:     {
65:     }
66:     void TearDown() override
67:     {
68:     }
69: };
70: 
71: class MemoryAccessI2CMasterMockNoReply : public MemoryAccessI2CMasterMock
72: {
73: public:
74:     MemoryAccessI2CMasterMockNoReply()
75:         : MemoryAccessI2CMasterMock()
76:     {
77:         SetSendAddressByteCallback(OnSendAddress);
78:         SetRecvDataByteCallback(OnRecvData);
79:         SetSendDataByteCallback(OnSendData);
80:     }
81:     static bool OnSendAddress(I2CMasterRegisters &registers, uint8 address)
82:     {
83:         return false;
84:     }
85:     static bool OnRecvData(I2CMasterRegisters& registers, uint8& data)
86:     {
87:         return false;
88:     }
89:     static bool OnSendData(I2CMasterRegisters& registers, uint8 data)
90:     {
91:         return false;
92:     }
93: };
94: 
95: class MemoryAccessI2CMasterMockDevicePresent : public MemoryAccessI2CMasterMock
96: {
97: public:
98:     static bool bytesWritten;
99:     MemoryAccessI2CMasterMockDevicePresent()
100:         : MemoryAccessI2CMasterMock()
101:     {
102:         SetSendAddressByteCallback(OnSendAddress);
103:         SetRecvDataByteCallback(OnRecvData);
104:         SetSendDataByteCallback(OnSendData);
105:     }
106:     static bool OnSendAddress(I2CMasterRegisters &registers, uint8 address)
107:     {
108:         return true;
109:     }
110:     static bool OnRecvData(I2CMasterRegisters& registers, uint8& data)
111:     {
112:         data = 0x00;
113:         if (bytesWritten)
114:         {
115:             data = 0x01;
116:             bytesWritten = false;
117:         }
118:         return true;
119:     }
120:     static bool OnSendData(I2CMasterRegisters& registers, uint8 data)
121:     {
122:         bytesWritten = true;
123:         return true;
124:     }
125: };
126: bool MemoryAccessI2CMasterMockDevicePresent::bytesWritten = false;
127: 
128: TEST_FIXTURE(I2CMasterTest, NoDevice)
129: {
130:     uint8 busIndex = 1;
131:     uint8 address{ 0x20 };
132:     MemoryAccessI2CMasterMockNoReply memoryAccess;
133:     {
134:         I2CMaster master(memoryAccess);
135: 
136:         master.Initialize(busIndex);
137: 
138:         EXPECT_FALSE(master.Scan(address));
139: 
140:     }
141:     size_t indexGPIOOps{};
142:     size_t indexI2COps{};
143: #if BAREMETAL_RPI_TARGET == 3
144:     EXPECT_EQ(size_t{16}, memoryAccess.GetNumGPIOOperations());
145:     
146:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
147:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
148:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
149:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
150:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
151:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
152:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
153:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
154: 
155:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
156:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
157:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
158:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
159:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
160:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
161:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
162:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
163: #else
164:     EXPECT_EQ(size_t{8}, memoryAccess.GetNumGPIOOperations());
165:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
166:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
167:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
168:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
169:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
170:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
171:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
172:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
173: #endif
174: 
175:     EXPECT_EQ(size_t{13}, memoryAccess.GetNumI2CMasterOperations());
176:     
177:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetClockDivider, busIndex, 3500 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
178:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
179:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
180:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
181:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
182:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
183:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
184:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
185:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::EnableController, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
186:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
187:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
188:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
189:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
190: }
191: 
192: TEST_FIXTURE(I2CMasterTest, FoundDevice)
193: {
194:     uint8 busIndex = 1;
195:     uint8 address{ 0x20 };
196:     MemoryAccessI2CMasterMockDevicePresent memoryAccess;
197:     {
198:         I2CMaster master(memoryAccess);
199: 
200:         master.Initialize(busIndex);
201: 
202:         EXPECT_TRUE(master.Scan(address));
203:         uint8 dataRead{};
204:         EXPECT_EQ(size_t{1}, master.Read(address, dataRead));
205:         EXPECT_EQ(uint8{0x01}, dataRead);
206:     }
207:     size_t indexGPIOOps{};
208:     size_t indexI2COps{};
209: #if BAREMETAL_RPI_TARGET == 3
210:     EXPECT_EQ(size_t{16}, memoryAccess.GetNumGPIOOperations());
211: 
212:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
213:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
214:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
215:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
216:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
217:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
218:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
219:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
220: 
221:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
222:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
223:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
224:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
225:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPullUpDownMode, 0xFF, 0x2 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
226:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
227:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownClock, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
228:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
229: #else
230:     EXPECT_EQ(size_t{8}, memoryAccess.GetNumGPIOOperations());
231:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000002, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
232:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
233:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000003, 0x0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
234:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::AlternateFunction0 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
235:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000002, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
236:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000002, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
237:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinPullUpDownMode, 0x00000003, 0x1 }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
238:     EXPECT_EQ((PhysicalGPIOPinOperation{ PhysicalGPIOPinOperationCode::SetPinMode, 0x00000003, GPIOFunction::Input }), memoryAccess.GetGPIOOperation(indexGPIOOps++));
239: #endif
240: 
241:     EXPECT_EQ(size_t{21}, memoryAccess.GetNumI2CMasterOperations());
242:     
243:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetClockDivider, busIndex, 3500 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
244:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
245:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
246:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
247:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
248:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
249:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
250:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
251:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::EnableController, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
252:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
253:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
254: 
255:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
256:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
257:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
258:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
259:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
260:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
261:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetReadMode, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
262:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
263:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ReadFIFO, busIndex, 0 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
264:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
265: }
266: 
267: } // suite Baremetal
268: 
269: } // namespace test
270: } // namespace baremetal
```

- Line 71-93: We declare a mock class `MemoryAccessI2CMasterMockNoReply` for no device present
- Line 95-125: We declare a mock class `MemoryAccessI2CMasterMockDevicePresent` for device present
- Line 128-189: Test that scanning for a non-existing device returns false
  - Line 144-162: Expectations for GPIO for Raspberry Pi 3. Note that we dropped the memory access expectations here
  - Line 164-172: Expectations for GPIO for other targets. Note that we dropped the memory access expectations here
  - Line 175-189: Expectations for I2C operations
- Line 191-265: Test that scanning for an existing device returns true and reading data works
  - Line 210-228: Expectations for GPIO for Raspberry Pi 3. Note that we dropped the memory access expectations here
  - Line 230-238: Expectations for GPIO for other targets. Note that we dropped the memory access expectations here
  - Line 241-253: Expectations for I2C operations during scan
  - Line 255-264: Expectations for I2C operations during read

### Extend selected tests {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_7_EXTEND_SELECTED_TESTS}

We need to update the main test application to add the tests for the `I2CMaster`.

Update the file `code\libraries\baremetal\test\src\main.cpp`

```cpp
File: code\libraries\baremetal\test\src\main.cpp
1: #include "baremetal/System.h"
2: #include "baremetal/Logger.h"
3: #include "unittest/unittest.h"
4: 
5: using namespace baremetal;
6: using namespace unittest;
7: 
8: int main()
9: {
10:     ConsoleTestReporter reporter;
11:     GetLogger().SetLogLevel(LogSeverity::Info);
12:     RunSelectedTests(&reporter, InSelection("Baremetal", "MemoryAccessMockTest", nullptr));
13:     RunSelectedTests(&reporter, InSelection("Baremetal", "PhysicalGPIOPinTest", nullptr));
14:     RunSelectedTests(&reporter, InSelection("Baremetal", "FIFOTest", nullptr));
15:     RunSelectedTests(&reporter, InSelection("Baremetal", "I2CMasterTest", nullptr));
16: //    RunAllTests(&reporter);
17: 
18:     return static_cast<int>(ReturnCode::ExitHalt);
19: }
```

### Configuring, building and debugging {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_7_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. All tests should succeed.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (6 fixtures)
[  FIXTURE  ] MemoryAccessMockTest (1 test)
[ SUCCEEDED ] Baremetal::MemoryAccessMockTest::SimpleTest
[  FIXTURE  ] 1 test from MemoryAccessMockTest
[   SUITE   ] 6 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (6 fixtures)
[  FIXTURE  ] PhysicalGPIOPinTest (1 test)
[ SUCCEEDED ] Baremetal::PhysicalGPIOPinTest::SimpleTest
[  FIXTURE  ] 1 test from PhysicalGPIOPinTest
[   SUITE   ] 6 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
[===========] Running 6 tests from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (6 fixtures)
[  FIXTURE  ] FIFOTest (6 tests)
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOIsInitiallyEmpty
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOOneQuarterFull
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOHalfFull
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOThreeQuartersFull
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOFull
[ SUCCEEDED ] Baremetal::FIFOTest::FIFOFullThenFlushed
[  FIXTURE  ] 6 tests from FIFOTest
[   SUITE   ] 6 fixtures from Baremetal
Success: 6 tests passed.

No failures
[===========] 6 tests from 1 fixture in 1 suite ran.
[===========] Running 2 tests from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (6 fixtures)
[  FIXTURE  ] I2CMasterTest (2 tests)
Info   0.00:00:00.070 Initialize bus 1, mode 0, config 0 (I2CMaster:166)
Info   0.00:00:00.070 Set clock 100000 (I2CMaster:207)
Info   0.00:00:00.070 Set up bus 1, config 0, base address FE804000 (I2CMaster:190)
[ SUCCEEDED ] Baremetal::I2CMasterTest::NoDevice
Info   0.00:00:00.080 Initialize bus 1, mode 0, config 0 (I2CMaster:166)
Info   0.00:00:00.080 Set clock 100000 (I2CMaster:207)
Info   0.00:00:00.080 Set up bus 1, config 0, base address FE804000 (I2CMaster:190)
[ SUCCEEDED ] Baremetal::I2CMasterTest::FoundDevice
[  FIXTURE  ] 2 tests from I2CMasterTest
[   SUITE   ] 6 fixtures from Baremetal
Success: 2 tests passed.

No failures
[===========] 2 tests from 1 fixture in 1 suite ran.
Info   0.00:00:00.090 Halt (System:121)
```

As you can see, we now have only 1 test to run.

The test shows a read for address 0xFE2000E4 (`RPI_GPIO_GPPUPPDN0_OFFSET`), followed by a write to the same adress with value 0x00000000 (meaning no pull up/down for pin 0).
Next, there is a read for address 0xFE200000 (`RPI_GPIO_GPFSEL0`), followed by a write to the same address with value 0x00000004 (meaning setting pin 0 to alternate function 0, which makes it the SDA pin for I2C0).
This all is specific for Raspberry Pi 4, for Raspberry Pi 3 this is slightly different.

Now this is all nice, but this way we cannot test whether the results is what we expect. We could check for console output, but that is quite cumbersome.
A simple first step is keeping an array of accesses to memory, so we can check that the correct steps happen in the correct order.

## Creating actual unit tests - Step 8 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_8}

Now we come to the final stage, the MCP23017 I2C port expander.
We'll create unit tests for this device.

We'll take a close look at how we set up the stub for MCP23017 device, and how we can use it in the unit tests.

Let's first create a mock for the MCP23017 device. This will be part of the device library, so we need to add test infrastructure to the device library, similar to the baremetal library.

### Reverting baremetal test main.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_8_REVERTING_BAREMETAL_TEST_MAINCPP}

To start with, let's revert the changes in the baremetal library's `main.cpp` file. This means from now on all baremetal tests will run again.

Update the file `code/libraries/baremetal/test/src/main.cpp`

```cmake
File: code/libraries/baremetal/test/src/main.cpp
1: #include "baremetal/System.h"
2: #include "baremetal/Logger.h"
3: #include "unittest/unittest.h"
4: 
5: using namespace baremetal;
6: using namespace unittest;
7: 
8: int main()
9: {
10:     ConsoleTestReporter reporter;
11:     GetLogger().SetLogLevel(LogSeverity::Info);
12:     RunAllTests(&reporter);
13: 
14:     return static_cast<int>(ReturnCode::ExitHalt);
15: }
```

### Set up test project configuration {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_8_SET_UP_TEST_PROJECT_CONFIGURATION}

We'll fill in the CMake file for the test project.

Create the file `code/libraries/device/test/CMakeLists.txt`

```cmake
File: code/libraries/device/test/CMakeLists.txt
1: project(device-test
2:     DESCRIPTION "Device test application"
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
26:     device
27:     unittest
28:     )
29: 
30: set(PROJECT_LIBS
31:     ${LINKER_LIBRARIES}
32:     ${PROJECT_DEPENDENCIES}
33:     )
34: 
35: file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
36: file(GLOB_RECURSE PROJECT_INCLUDES_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
37: set(PROJECT_INCLUDES_PRIVATE )
38: 
39: if (CMAKE_VERBOSE_MAKEFILE)
40:     display_list("Package                           : " ${PROJECT_NAME} )
41:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
42:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
43:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
44:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
45:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
46:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
47:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
48:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
49:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
50:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
51:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
52:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
53:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
54:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
55:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
56:     display_list("Link libs                         : " ${PROJECT_LIBS} )
57:     display_list("Source files                      : " ${PROJECT_SOURCES} )
58:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
59:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
60: endif()
61: 
62: if (PLATFORM_BAREMETAL)
63:     set(START_GROUP -Wl,--start-group)
64:     set(END_GROUP -Wl,--end-group)
65: endif()
66: 
67: add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
68: 
69: target_link_libraries(${PROJECT_NAME} ${START_GROUP} ${PROJECT_LIBS} ${END_GROUP})
70: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
71: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
72: target_compile_definitions(${PROJECT_NAME} PRIVATE
73:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
74:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
75:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
76:     )
77: target_compile_definitions(${PROJECT_NAME} PUBLIC
78:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
79:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
80:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
81:     )
82: target_compile_options(${PROJECT_NAME} PRIVATE
83:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
84:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
85:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
86:     )
87: target_compile_options(${PROJECT_NAME} PUBLIC
88:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
89:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
90:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
91:     )
92: 
93: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
94: 
95: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
96: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
97:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
98: endif()
99: 
100: link_directories(${LINK_DIRECTORIES})
101: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
102: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
103: set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
104: 
105: show_target_properties(${PROJECT_NAME})
106: 
107: set(BAREMETAL_EXECUTABLE_TARGET ${PROJECT_NAME})
108: setup_image(${PROJECT_NAME})
```

### Update device project configuration {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_8_UPDATE_DEVICE_PROJECT_CONFIGURATION}

We need to add the test project CMake file to the baremetal project CMake file in order to build the test project.

Update the file `code/libraries/device/CMakeLists.txt`

```cmake
File: code/libraries/device/CMakeLists.txt
93: show_target_properties(${PROJECT_NAME})
94: 
95: add_subdirectory(test)
```

### main.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_8_MAINCPP}

Finally we create the main source file that will run the tests.

Create the file `code\libraries\device\test\src\mocks\main.cpp`

```cpp
File: code\libraries\device\test\src\main.cpp
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

### MemoryAccessMCP23017Mock.h {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_8_MEMORYACCESSMCP23017MOCKH}

Create the file `code\libraries\device\include\device\mocks\MemoryAccessMCP23017Mock.h`

```cpp
File: code\libraries\device\include\device\mocks\MemoryAccessMCP23017Mock.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessMCP23017Mock.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessMCP23017Mock
9: //
10: // Description : MCP23017 memory access stub with LEDs on output pins, and controllable inputs on input pins
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
42: #include "baremetal/mocks/MemoryAccessI2CMasterMock.h"
43: #include "stdlib/Macros.h"
44: #include "baremetal/String.h"
45: #include "device/i2c/MCP23017.h"
46: 
47: /// @file
48: /// MemoryAccessMCP23017Mock
49: 
50: namespace device {
51: 
52: /// @brief MCP23017 registers
53: class MCP23017Registers
54: {
55: public:
56:     /// @brief I/O Direction Register Port A
57:     uint8 IODIRA;
58:     /// @brief I/O Direction Register Port B
59:     uint8 IODIRB;
60:     /// @brief Input Polarity Register Port A
61:     uint8 IPOLA;
62:     /// @brief Input Polarity Register Port B
63:     uint8 IPOLB;
64:     /// @brief GPIO Interrupt Enable Register Port A
65:     uint8 GPINTENA;
66:     /// @brief GPIO Interrupt Enable Register Port B
67:     uint8 GPINTENB;
68:     /// @brief Default Compare Register Port A
69:     uint8 DEFVALA;
70:     /// @brief Default Compare Register Port B
71:     uint8 DEFVALB;
72:     /// @brief Interrupt Control Register Port A
73:     uint8 INTCONA;
74:     /// @brief Interrupt Control Register Port B
75:     uint8 INTCONB;
76:     /// @brief I/O Control Register Port A
77:     uint8 IOCONA;
78:     /// @brief I/O Control Register Port B
79:     uint8 IOCONB;
80:     /// @brief GPIO Pull-up Resistor Configuration Register Port A
81:     uint8 GPPUA;
82:     /// @brief GPIO Pull-up Resistor Configuration Register Port B
83:     uint8 GPPUB;
84:     /// @brief Interrupt Flag Register Port A
85:     uint8 INTFA;
86:     /// @brief Interrupt Flag Register Port B
87:     uint8 INTFB;
88:     /// @brief Interrupt Captured Register Port A
89:     uint8 INTCAPA;
90:     /// @brief Interrupt Captured Register Port B
91:     uint8 INTCAPB;
92:     /// @brief GPIO Port Register Port A
93:     uint8 GPIOA;
94:     /// @brief GPIO Port Register Port b
95:     uint8 GPIOB;
96:     /// @brief Output Latch Register Port A
97:     uint8 OLATA;
98:     /// @brief Output Latch Register Port B
99:     uint8 OLATB;
100: 
101:     /// <summary>
102:     /// Constructor for MCP23017 register storage
103:     /// </summary>
104:     MCP23017Registers()
105:         : IODIRA{0xFF}
106:         , IODIRB{0xFF}
107:         , IPOLA{}
108:         , IPOLB{}
109:         , GPINTENA{}
110:         , GPINTENB{}
111:         , DEFVALA{}
112:         , DEFVALB{}
113:         , INTCONA{}
114:         , INTCONB{}
115:         , IOCONA{}
116:         , IOCONB{}
117:         , GPPUA{}
118:         , GPPUB{}
119:         , INTFA{}
120:         , INTFB{}
121:         , INTCAPA{}
122:         , INTCAPB{}
123:         , GPIOA{}
124:         , GPIOB{}
125:         , OLATA{}
126:         , OLATB{}
127:     {
128:     }
129: };
130: 
131: /// @brief I2C master operation codes
132: enum MCP23017OperationCode
133: {
134:     /// @brief Write IO Configuration Register
135:     WriteIOCON,
136:     /// @brief Write Port A IO Direction Register
137:     WriteIODIRA,
138:     /// @brief Write Port B IO Direction Register
139:     WriteIODIRB,
140:     /// @brief Write Port A GPIO Register
141:     WriteGPIOA,
142:     /// @brief Write Port B GPIO Register
143:     WriteGPIOB,
144:     /// @brief Write Port A Interrupt Enable Register
145:     WriteGPINTENA,
146:     /// @brief Write Port B Interrupt Enable Register
147:     WriteGPINTENB,
148:     /// @brief Write Port A Interrupt Control Register
149:     WriteINTCONA,
150:     /// @brief Write Port B Interrupt Control Register
151:     WriteINTCONB,
152:     /// @brief Write Port A Default Value Register
153:     WriteDEFVALA,
154:     /// @brief Write Port B Default Value Register
155:     WriteDEFVALB,
156:     /// @brief Write Port A Input Polarity Register
157:     WriteIPOLA,
158:     /// @brief Write Port B Input Polarity Register
159:     WriteIPOLB,
160:     /// @brief Write Port A GPIO Pull-up Register
161:     WriteGPPUA,
162:     /// @brief Write Port B GPIO Pull-up Register
163:     WriteGPPUB,
164:     /// @brief Write Port A Output Latch Register
165:     WriteOLATA,
166:     /// @brief Write Port B Output Latch Register
167:     WriteOLATB,
168:     /// @brief Read IO Configuration Register
169:     ReadIOCON,
170:     /// @brief Read Port A IO Direction Register
171:     ReadIODIRA,
172:     /// @brief Read Port B IO Direction Register
173:     ReadIODIRB,
174:     /// @brief Read Port A GPIO Register
175:     ReadGPIOA,
176:     /// @brief Read Port B GPIO Register
177:     ReadGPIOB,
178:     /// @brief Read Port A Interrupt Enable Register
179:     ReadGPINTENA,
180:     /// @brief Read Port B Interrupt Enable Register
181:     ReadGPINTENB,
182:     /// @brief Read Port A Interrupt Control Register
183:     ReadINTCONA,
184:     /// @brief Read Port B Interrupt Control Register
185:     ReadINTCONB,
186:     /// @brief Read Port A Default Value Register
187:     ReadDEFVALA,
188:     /// @brief Read Port B Default Value Register
189:     ReadDEFVALB,
190:     /// @brief Read Port A Input Polarity Register
191:     ReadIPOLA,
192:     /// @brief Read Port B Input Polarity Register
193:     ReadIPOLB,
194:     /// @brief Read Port A GPIO Pull-up Register
195:     ReadGPPUA,
196:     /// @brief Read Port B GPIO Pull-up Register
197:     ReadGPPUB,
198:     /// @brief Read Port A Interrupt Flag Register
199:     ReadINTFA,
200:     /// @brief Read Port B Interrupt Flag Register
201:     ReadINTFB,
202:     /// @brief Read Port A Interrupt Captured Register
203:     ReadINTCAPA,
204:     /// @brief Read Port B Interrupt Captured Register
205:     ReadINTCAPB,
206:     /// @brief Read Port A Output Latch Register
207:     ReadOLATA,
208:     /// @brief Read Port B Output Latch Register
209:     ReadOLATB,
210:     /// @brief Read IO Configuration Register
211: };
212: 
213: /// <summary>
214: /// Data structure to contain a memory access operation
215: /// </summary>
216: struct MCP23017Operation
217: {
218:     /// @brief I2C master operation code
219:     MCP23017OperationCode operation; // Size: 4 bytes
220:     /// @brief Argument (if any)
221:     uint32 argument; // Size: 4 bytes
222: 
223:     /// <summary>
224:     /// Default constructor
225:     /// </summary>
226:     MCP23017Operation()
227:         : operation{}
228:         , argument{}
229:     {
230:     }
231: 
232:     /// <summary>
233:     /// Constructor for read or write operation concerning pin function
234:     /// </summary>
235:     /// <param name="theOperation">Operation code</param>
236:     /// <param name="theArgument">Argument value</param>
237:     MCP23017Operation(MCP23017OperationCode theOperation, uint32 theArgument = 0)
238:         : operation{theOperation}
239:         , argument{theArgument}
240:     {
241:     }
242:     /// <summary>
243:     /// Check memory access operations for equality
244:     /// </summary>
245:     /// <param name="other">Value to compare to</param>
246:     /// <returns>True if equal, false otherwise</returns>
247:     bool operator==(const MCP23017Operation& other) const
248:     {
249:         return (other.operation == operation) &&
250:             (other.argument == argument);
251:     }
252:     /// <summary>
253:     /// Check memory access operations for inequality
254:     /// </summary>
255:     /// <param name="other">Value to compare to</param>
256:     /// <returns>True if unequal, false otherwise</returns>
257:     bool operator!=(const MCP23017Operation& other) const
258:     {
259:         return !operator==(other);
260:     }
261: } ALIGN(8);
262: 
263: /// @brief MemoryAccess implementation for I2C stub
264: class MemoryAccessMCP23017Mock : public baremetal::MemoryAccessI2CMasterMock
265: {
266: private:
267:     /// @brief Singleton instance
268:     static MemoryAccessMCP23017Mock* m_pThis;
269:     /// @brief Storage for I2C registers
270:     MCP23017Registers m_registers;
271:     /// @brief A read / write register cycle was started
272:     bool m_cycleStarted;
273:     /// @brief Register selected for current read / write register cycle
274:     uint8 m_selectedRegister;
275:     /// @brief Size of memory access operation array
276:     static constexpr size_t BufferSize = 1000;
277:     /// List op memory access operations
278:     MCP23017Operation m_ops[BufferSize] ALIGN(8);
279:     /// @brief Number of registered memory access operations
280:     size_t m_numOps;
281: 
282: public:
283:     MemoryAccessMCP23017Mock();
284: 
285:     size_t GetNumMCP23017Operations() const;
286:     const MCP23017Operation& GetMCP23017Operation(size_t index) const;
287: 
288:     void ResetCycle();
289:     static bool OnSendAddress(baremetal::I2CMasterRegisters& registers, uint8 data);
290:     static bool OnRecvData(baremetal::I2CMasterRegisters& registers, uint8& data);
291:     static bool OnSendData(baremetal::I2CMasterRegisters& registers, uint8 data);
292: 
293:     virtual bool OnReadRegister(uint8 registerIndex, uint8& data);
294:     virtual bool OnWriteRegister(uint8 registerIndex, uint8 data);
295: 
296: private:
297:     void AddOperation(const MCP23017Operation& operation);
298: };
299: 
300: } // namespace device
301: 
302: namespace baremetal {
303: 
304: baremetal::String Serialize(const device::MCP23017Operation& value);
305: 
306: } // namespace baremetal
```

- Line 52-129: Similar to what we did for the I2C master, we declare a class `MCP23017Registers` to hold the registers of the MCP23017.
- Line 131-211: Again similar to what we did before we declare an enum `MCP23017OperationCode` to hold the different operations performed
- Line 213-261: Again similar we declare a struct `MCP23017Operation` to hold the data for an operation on the MCP23017 registers
- Line 263-300: We declare a class `MemoryAccessMCP23017Mock` deriving from `MemoryAccessI2CMasterMock` which acts as a MCP23107 device
- Line 304: We declare a function `Serialize()` to serialize a `MCP23017Operation`

Notice that we again base the mock class on the stub class defined before.

### MemoryAccessMCP23017Mock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_8_MEMORYACCESSMCP23017MOCKCPP}

Let's implement the `MemoryAccessI2CMasterMock` class.

Create the file `code\libraries\device\src\mocks\MemoryAccessMCP23017Mock.cpp`

```cpp
File: code\libraries\device\src\mocks\MemoryAccessMCP23017Mock.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MemoryAccessMCP23017Mock.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccessMCP23017Mock
9: //
10: // Description : MCP23017 memory access stub with LEDs on output pins, and controllable inputs on input pins
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
40: #include "device/mocks/MemoryAccessMCP23017Mock.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/Format.h"
45: #include "baremetal/Logger.h"
46: #include "baremetal/String.h"
47: #include "device/i2c/MCP23017.h"
48: 
49: /// @file
50: /// MemoryAccessMCP23017Mock
51: 
52: /// @brief Define log name
53: LOG_MODULE("MemoryAccessMCP23017Mock");
54: 
55: using namespace baremetal;
56: using namespace device;
57: 
58: /// <summary>
59: /// Convert operation code to string
60: /// </summary>
61: /// <param name="code">Operation cpde</param>
62: /// <returns>String representing operator code</returns>
63: static String OperationCodeToString(MCP23017OperationCode code)
64: {
65:     String result{};
66:     switch (code)
67:     {
68:     case MCP23017OperationCode::WriteIOCON:
69:         result = "WriteIOCON";
70:         break;
71:     case MCP23017OperationCode::WriteIODIRA:
72:         result = "WriteIODIRA";
73:         break;
74:     case MCP23017OperationCode::WriteIODIRB:
75:         result = "WriteIODIRB";
76:         break;
77:     case MCP23017OperationCode::WriteGPIOA:
78:         result = "WriteGPIOA";
79:         break;
80:     case MCP23017OperationCode::WriteGPIOB:
81:         result = "WriteGPIOB";
82:         break;
83:     case MCP23017OperationCode::WriteGPINTENA:
84:         result = "WriteGPINTENA";
85:         break;
86:     case MCP23017OperationCode::WriteGPINTENB:
87:         result = "WriteGPINTENB";
88:         break;
89:     case MCP23017OperationCode::WriteINTCONA:
90:         result = "WriteINTCONA";
91:         break;
92:     case MCP23017OperationCode::WriteINTCONB:
93:         result = "WriteINTCONB";
94:         break;
95:     case MCP23017OperationCode::WriteDEFVALA:
96:         result = "WriteDEFVALA";
97:         break;
98:     case MCP23017OperationCode::WriteDEFVALB:
99:         result = "WriteDEFVALB";
100:         break;
101:     case MCP23017OperationCode::WriteIPOLA:
102:         result = "WriteIPOLA";
103:         break;
104:     case MCP23017OperationCode::WriteIPOLB:
105:         result = "WriteIPOLB";
106:         break;
107:     case MCP23017OperationCode::WriteGPPUA:
108:         result = "WriteGPPUA";
109:         break;
110:     case MCP23017OperationCode::WriteGPPUB:
111:         result = "WriteGPPUB";
112:         break;
113:     case MCP23017OperationCode::WriteOLATA:
114:         result = "WriteOLATA";
115:         break;
116:     case MCP23017OperationCode::WriteOLATB:
117:         result = "WriteOLATB";
118:         break;
119:     case MCP23017OperationCode::ReadIOCON:
120:         result = "ReadIOCON";
121:         break;
122:     case MCP23017OperationCode::ReadIODIRA:
123:         result = "ReadIODIRA";
124:         break;
125:     case MCP23017OperationCode::ReadIODIRB:
126:         result = "ReadIODIRB";
127:         break;
128:     case MCP23017OperationCode::ReadGPIOA:
129:         result = "ReadGPIOA";
130:         break;
131:     case MCP23017OperationCode::ReadGPIOB:
132:         result = "ReadGPIOB";
133:         break;
134:     case MCP23017OperationCode::ReadGPINTENA:
135:         result = "ReadGPINTENA";
136:         break;
137:     case MCP23017OperationCode::ReadGPINTENB:
138:         result = "ReadGPINTENB";
139:         break;
140:     case MCP23017OperationCode::ReadINTCONA:
141:         result = "ReadINTCONA";
142:         break;
143:     case MCP23017OperationCode::ReadINTCONB:
144:         result = "ReadINTCONB";
145:         break;
146:     case MCP23017OperationCode::ReadDEFVALA:
147:         result = "ReadDEFVALA";
148:         break;
149:     case MCP23017OperationCode::ReadDEFVALB:
150:         result = "ReadDEFVALB";
151:         break;
152:     case MCP23017OperationCode::ReadIPOLA:
153:         result = "ReadIPOLA";
154:         break;
155:     case MCP23017OperationCode::ReadIPOLB:
156:         result = "ReadIPOLB";
157:         break;
158:     case MCP23017OperationCode::ReadGPPUA:
159:         result = "ReadGPPUA";
160:         break;
161:     case MCP23017OperationCode::ReadGPPUB:
162:         result = "ReadGPPUB";
163:         break;
164:     case MCP23017OperationCode::ReadINTFA:
165:         result = "ReadINTFA";
166:         break;
167:     case MCP23017OperationCode::ReadINTFB:
168:         result = "ReadINTFB";
169:         break;
170:     case MCP23017OperationCode::ReadINTCAPA:
171:         result = "ReadINTCAPA";
172:         break;
173:     case MCP23017OperationCode::ReadINTCAPB:
174:         result = "ReadINTCAPB";
175:         break;
176:     case MCP23017OperationCode::ReadOLATA:
177:         result = "ReadOLATA";
178:         break;
179:     case MCP23017OperationCode::ReadOLATB:
180:         result = "ReadOLATB";
181:         break;
182:     }
183:     return result;
184: }
185: 
186: /// <summary>
187: /// Serialize a GPIO memory access operation to string
188: /// </summary>
189: /// <param name="value">Value to be serialized</param>
190: /// <returns>Resulting string</returns>
191: String baremetal::Serialize(const MCP23017Operation &value)
192: {
193:     String result = Format("Operation=%s, ", OperationCodeToString(value.operation).c_str());
194:     switch (value.operation)
195:     {
196:     case MCP23017OperationCode::WriteIOCON:
197:     case MCP23017OperationCode::ReadIOCON:
198:         {
199:             String line = "IO configuation: ";
200:             if (value.argument & IOCON_BANK1)
201:                 line += "Bank 1 ";
202:             else
203:                 line += "Bank 0 ";
204:             if (value.argument & IOCON_MIRROR)
205:                 line += "INT_MIRROR ";
206:             else
207:                 line += "           ";
208:             if (value.argument & IOCON_SEQOP)
209:                 line += "SEQOP ";
210:             else
211:                 line += "      ";
212:             if (value.argument & IOCON_DISSLW)
213:                 line += "DISSLW ";
214:             else
215:                 line += "       ";
216:             if (value.argument & IOCON_HAEN)
217:                 line += "HAEN ";
218:             else
219:                 line += "HAEN ";
220:             if (value.argument & IOCON_ODR)
221:                 line += "ODR ";
222:             else
223:                 line += "    ";
224:             if (value.argument & IOCON_INTPOL)
225:                 line += "INTPOL H";
226:             else
227:                 line += "INTPOL L";
228:             result += line;
229:         }
230:         break;
231:     case MCP23017OperationCode::WriteIODIRA:
232:     case MCP23017OperationCode::ReadIODIRA:
233:         {
234:             String line = "Direction Port A: ";
235:             uint8 bits = value.argument;
236:             for (int i = 0; i < 8; ++i)
237:             {
238:                 line += (bits & 0x80 ? "I" : "O");
239:                 bits <<= 1;
240:             }
241:             result += line;
242:         }
243:         break;
244:     case MCP23017OperationCode::WriteIODIRB:
245:     case MCP23017OperationCode::ReadIODIRB:
246:         {
247:             String line = "Direction Port B: ";
248:             uint8 bits = value.argument;
249:             for (int i = 0; i < 8; ++i)
250:             {
251:                 line += (bits & 0x80 ? "I" : "O");
252:                 bits <<= 1;
253:             }
254:             result += line;
255:         }
256:         break;
257:     case MCP23017OperationCode::WriteGPIOA:
258:     case MCP23017OperationCode::ReadGPIOA:
259:         {
260:             String line = "GPIO Port A: ";
261:             uint8 gpio = value.argument;
262:             for (int i = 0; i < 8; ++i)
263:             {
264:                 line += (gpio & 0x80 ? "X" : " ");
265:                 gpio <<= 1;
266:             }
267:             result += line;
268:         }
269:         break;
270:     case MCP23017OperationCode::WriteGPIOB:
271:     case MCP23017OperationCode::ReadGPIOB:
272:         {
273:             String line = "GPIO Port B: ";
274:             uint8 gpio = value.argument;
275:             for (int i = 0; i < 8; ++i)
276:             {
277:                 line += (gpio & 0x80 ? "X" : " ");
278:                 gpio <<= 1;
279:             }
280:             result += line;
281:         }
282:         break;
283:     case MCP23017OperationCode::WriteGPINTENA:
284:     case MCP23017OperationCode::ReadGPINTENA:
285:         {
286:             String line = "Interrupt enable Port A: ";
287:             uint8 bits = value.argument;
288:             for (int i = 0; i < 8; ++i)
289:             {
290:                 line += Format("Pin %d ", i);
291:                 line += (bits & 0x80 ? "X" : " ");
292:                 bits <<= 1;
293:             }
294:             result += line;
295:         }
296:         break;
297:     case MCP23017OperationCode::WriteGPINTENB:
298:     case MCP23017OperationCode::ReadGPINTENB:
299:         {
300:             String line = "Interrupt enable Port B: ";
301:             uint8 bits = value.argument;
302:             for (int i = 0; i < 8; ++i)
303:             {
304:                 line += Format("Pin %d ", i);
305:                 line += (bits & 0x80 ? "X" : " ");
306:                 bits <<= 1;
307:             }
308:             result += line;
309:         }
310:         break;
311:     case MCP23017OperationCode::WriteINTCONA:
312:     case MCP23017OperationCode::ReadINTCONA:
313:         {
314:             String line = "Interrupt control Port A: ";
315:             uint8 bits = value.argument;
316:             for (int i = 0; i < 8; ++i)
317:             {
318:                 line += Format("Pin %d ", i);
319:                 line += (bits & 0x80 ? "DEF " : "PRV ");
320:                 bits <<= 1;
321:             }
322:             result += line;
323:         }
324:         break;
325:     case MCP23017OperationCode::WriteINTCONB:
326:     case MCP23017OperationCode::ReadINTCONB:
327:         {
328:             String line = "Interrupt control Port B: ";
329:             uint8 bits = value.argument;
330:             for (int i = 0; i < 8; ++i)
331:             {
332:                 line += Format("Pin %d ", i);
333:                 line += (bits & 0x80 ? "DEF " : "PRV ");
334:                 bits <<= 1;
335:             }
336:             result += line;
337:         }
338:         break;
339:     case MCP23017OperationCode::WriteDEFVALA:
340:     case MCP23017OperationCode::ReadDEFVALA:
341:         {
342:             String line = "Default value Port A: ";
343:             uint8 bits = value.argument;
344:             for (int i = 0; i < 8; ++i)
345:             {
346:                 line += Format("Pin %d ", i);
347:                 line += (bits & 0x80 ? "1" : "0");
348:                 bits <<= 1;
349:             }
350:             result += line;
351:         }
352:         break;
353:     case MCP23017OperationCode::WriteDEFVALB:
354:     case MCP23017OperationCode::ReadDEFVALB:
355:         {
356:             String line = "Default value Port B: ";
357:             uint8 bits = value.argument;
358:             for (int i = 0; i < 8; ++i)
359:             {
360:                 line += Format("Pin %d ", i);
361:                 line += (bits & 0x80 ? "1" : "0");
362:                 bits <<= 1;
363:             }
364:             result += line;
365:         }
366:         break;
367:     case MCP23017OperationCode::WriteIPOLA:
368:     case MCP23017OperationCode::ReadIPOLA:
369:         {
370:             String line = "Input polarity Port A: ";
371:             uint8 bits = value.argument;
372:             for (int i = 0; i < 8; ++i)
373:             {
374:                 line += Format("Pin %d ", i);
375:                 line += (bits & 0x80 ? "~" : " ");
376:                 bits <<= 1;
377:             }
378:             result += line;
379:         }
380:         break;
381:     case MCP23017OperationCode::WriteIPOLB:
382:     case MCP23017OperationCode::ReadIPOLB:
383:         {
384:             String line = "Input polarity Port B: ";
385:             uint8 bits = value.argument;
386:             for (int i = 0; i < 8; ++i)
387:             {
388:                 line += Format("Pin %d ", i);
389:                 line += (bits & 0x80 ? "~" : " ");
390:                 bits <<= 1;
391:             }
392:             result += line;
393:         }
394:         break;
395:     case MCP23017OperationCode::WriteGPPUA:
396:     case MCP23017OperationCode::ReadGPPUA:
397:         {
398:             String line = "GPIO Pull-up Port A: ";
399:             uint8 bits = value.argument;
400:             for (int i = 0; i < 8; ++i)
401:             {
402:                 line += Format("Pin %d ", i);
403:                 line += (bits & 0x80 ? "U" : "");
404:                 bits <<= 1;
405:             }
406:             result += line;
407:             break;
408:         }
409:     case MCP23017OperationCode::WriteGPPUB:
410:     case MCP23017OperationCode::ReadGPPUB:
411:         {
412:             String line = "GPIO Pull-up Port B: ";
413:             uint8 bits = value.argument;
414:             for (int i = 0; i < 8; ++i)
415:             {
416:                 line += Format("Pin %d ", i);
417:                 line += (bits & 0x80 ? "U" : "");
418:                 bits <<= 1;
419:             }
420:             result += line;
421:             break;
422:         }
423:     case MCP23017OperationCode::WriteOLATA:
424:     case MCP23017OperationCode::ReadOLATA:
425:         {
426:             String line = "Output latch port A: ";
427:             uint8 bits = value.argument;
428:             for (int i = 0; i < 8; ++i)
429:             {
430:                 line += Format("Pin %d ", i);
431:                 line += (bits & 0x80 ? "1" : "0");
432:                 bits <<= 1;
433:             }
434:             result += line;
435:             break;
436:         }
437:     case MCP23017OperationCode::WriteOLATB:
438:     case MCP23017OperationCode::ReadOLATB:
439:         {
440:             String line = "Output latch port B: ";
441:             uint8 bits = value.argument;
442:             for (int i = 0; i < 8; ++i)
443:             {
444:                 line += Format("Pin %d ", i);
445:                 line += (bits & 0x80 ? "1" : "0");
446:                 bits <<= 1;
447:             }
448:             result += line;
449:             break;
450:         }
451:     case MCP23017OperationCode::ReadINTFA:
452:         {
453:             String line = "Interrupt flags Port A: ";
454:             uint8 bits = value.argument;
455:             for (int i = 0; i < 8; ++i)
456:             {
457:                 line += Format("Pin %d ", i);
458:                 line += (bits & 0x80 ? "X" : "");
459:                 bits <<= 1;
460:             }
461:             result += line;
462:         }
463:         break;
464:     case MCP23017OperationCode::ReadINTFB:
465:         {
466:             String line = "Interrupt flags Port ZB: ";
467:             uint8 bits = value.argument;
468:             for (int i = 0; i < 8; ++i)
469:             {
470:                 line += Format("Pin %d ", i);
471:                 line += (bits & 0x80 ? "X" : "");
472:                 bits <<= 1;
473:             }
474:             result += line;
475:         }
476:         break;
477:     case MCP23017OperationCode::ReadINTCAPA:
478:         {
479:             String line = "Interrupt capture Port A: ";
480:             uint8 bits = value.argument;
481:             for (int i = 0; i < 8; ++i)
482:             {
483:                 line += Format("Pin %d ", i);
484:                 line += (bits & 0x80 ? "1" : "0");
485:                 bits <<= 1;
486:             }
487:             result += line;
488:         }
489:         break;
490:     case MCP23017OperationCode::ReadINTCAPB:
491:         {
492:             String line = "Interrupt capture Port B: ";
493:             uint8 bits = value.argument;
494:             for (int i = 0; i < 8; ++i)
495:             {
496:                 line += Format("Pin %d ", i);
497:                 line += (bits & 0x80 ? "1" : "0");
498:                 bits <<= 1;
499:             }
500:             result += line;
501:         }
502:         break;
503: 
504:     default:
505:         result += Format("Argument=%d", value.argument);
506:         break;
507:     }
508:     return result;
509: }
510: 
511: /// @brief Singleton instance
512: MemoryAccessMCP23017Mock* MemoryAccessMCP23017Mock::m_pThis{};
513: 
514: /// <summary>
515: /// MemoryAccessMCP23017Mock constructor
516: /// </summary>
517: MemoryAccessMCP23017Mock::MemoryAccessMCP23017Mock()
518:     : m_registers{}
519:     , m_cycleStarted{}
520:     , m_selectedRegister{}
521: {
522:     m_pThis = this;
523:     SetSendAddressByteCallback(OnSendAddress);
524:     SetRecvDataByteCallback(OnRecvData);
525:     SetSendDataByteCallback(OnSendData);
526: }
527: 
528: /// <summary>
529: /// Return number of registered memory access operations
530: /// </summary>
531: /// <returns>Number of registered memory access operations</returns>
532: size_t MemoryAccessMCP23017Mock::GetNumMCP23017Operations() const
533: {
534:     return m_numOps;
535: }
536: 
537: /// <summary>
538: /// Retrieve a registered memory access operation from the list
539: /// </summary>
540: /// <param name="index">Index of operation</param>
541: /// <returns>Requested memory access operation</returns>
542: const MCP23017Operation &MemoryAccessMCP23017Mock::GetMCP23017Operation(size_t index) const
543: {
544:     assert(index < m_numOps);
545:     return m_ops[index];
546: }
547: 
548: /// <summary>
549: /// Reset read or write cycle
550: /// </summary>
551: void MemoryAccessMCP23017Mock::ResetCycle()
552: {
553:     m_cycleStarted = false;
554: }
555: 
556: /// <summary>
557: /// Callback when I2C address is sent
558: /// </summary>
559: /// <param name="registers">I2C register storage, unused</param>
560: /// <param name="data">I2C address, unused</param>
561: /// <returns>True always</returns>
562: bool MemoryAccessMCP23017Mock::OnSendAddress(I2CMasterRegisters& /*registers*/, uint8 /*data*/)
563: {
564:     return true;
565: }
566: 
567: /// <summary>
568: /// Callback when I2C byte is to be received
569: /// </summary>
570: /// <param name="registers">I2C Register storage for stub</param>
571: /// <param name="data">Byte requested</param>
572: /// <returns>True always</returns>
573: bool MemoryAccessMCP23017Mock::OnRecvData(I2CMasterRegisters& registers, uint8& data)
574: {
575:     auto result = m_pThis->OnReadRegister(m_pThis->m_selectedRegister, data);
576:     LOG_DEBUG("Read register %02x: %02x", m_pThis->m_selectedRegister, data);
577:     m_pThis->ResetCycle();
578: 
579:     return result;
580: }
581: 
582: /// <summary>
583: /// Callback when I2C byte is sent
584: /// </summary>
585: /// <param name="registers">I2C Register storage for stub</param>
586: /// <param name="data">Byte sent</param>
587: /// <returns>True always</returns>
588: bool MemoryAccessMCP23017Mock::OnSendData(I2CMasterRegisters& registers, uint8 data)
589: {
590:     if (!m_pThis->m_cycleStarted)
591:     {
592:         m_pThis->m_selectedRegister = data;
593:         m_pThis->m_cycleStarted = true;
594:     }
595:     else
596:     {
597:         LOG_DEBUG("Write register %02x: %02x", m_pThis->m_selectedRegister, data);
598:         m_pThis->OnWriteRegister(m_pThis->m_selectedRegister, data);
599:         m_pThis->ResetCycle();
600:     }
601:     return true;
602: }
603: 
604: /// <summary>
605: /// Read MCP23017 register
606: /// </summary>
607: /// <param name="registerIndex">Index of the register</param>
608: /// <param name="data">Data read on return</param>
609: /// <returns>True if successful, false otherwise</returns>
610: bool MemoryAccessMCP23017Mock::OnReadRegister(uint8 registerIndex, uint8& data)
611: {
612:     uint8* registerAddress = &m_pThis->m_registers.IODIRA + m_pThis->m_selectedRegister;
613:     data = *registerAddress;
614:     switch (m_pThis->m_selectedRegister)
615:     {
616:     case IOCONA:
617:     case IOCONB:
618:         AddOperation({ ReadIOCON, data });
619:         break;
620:     case IODIRA:
621:         AddOperation({ ReadIODIRA, data });
622:         break;
623:     case IODIRB:
624:         AddOperation({ ReadIODIRB, data });
625:         break;
626:     case GPIOA:
627:         AddOperation({ ReadGPIOA, data });
628:         break;
629:     case GPIOB:
630:         AddOperation({ ReadGPIOB, data });
631:         break;
632:     case GPINTENA:
633:         AddOperation({ ReadGPINTENA, data });
634:         break;
635:     case GPINTENB:
636:         AddOperation({ ReadGPINTENB, data });
637:         break;
638:     case INTCONA:
639:         AddOperation({ ReadINTCONA, data });
640:         break;
641:     case INTCONB:
642:         AddOperation({ ReadINTCONA, data });
643:         break;
644:     case DEFVALA:
645:         AddOperation({ ReadDEFVALA, data });
646:         break;
647:     case DEFVALB:
648:         AddOperation({ ReadDEFVALB, data });
649:         break;
650:     case IPOLA:
651:         AddOperation({ ReadIPOLA, data });
652:         break;
653:     case IPOLB:
654:         AddOperation({ ReadIPOLB, data });
655:         break;
656:     case GPPUA:
657:         AddOperation({ ReadGPPUA, data });
658:         break;
659:     case GPPUB:
660:         AddOperation({ ReadGPPUB, data });
661:         break;
662:     case INTFA:
663:         AddOperation({ ReadINTFA, data });
664:         break;
665:     case INTFB:
666:         AddOperation({ ReadINTFB, data });
667:         break;
668:     case INTCAPA:
669:         AddOperation({ ReadINTCAPA, data });
670:         break;
671:     case INTCAPB:
672:         AddOperation({ ReadINTCAPB, data });
673:         break;
674:     case OLATA:
675:         AddOperation({ ReadOLATA, data });
676:         break;
677:     case OLATB:
678:         AddOperation({ ReadOLATB, data });
679:         break;
680:     }
681:     return true;
682: }
683: 
684: /// <summary>
685: /// Write MCP23017 register
686: /// </summary>
687: /// <param name="registerIndex">Index of the register</param>
688: /// <param name="data">Data to be written</param>
689: /// <returns>True if successful, false otherwise</returns>
690: bool MemoryAccessMCP23017Mock::OnWriteRegister(uint8 registerIndex, uint8 data)
691: {
692:     uint8* registerAddress = &m_pThis->m_registers.IODIRA + m_pThis->m_selectedRegister;
693:     *registerAddress = data;
694:     switch (m_pThis->m_selectedRegister)
695:     {
696:     case IOCONA:
697:     case IOCONB:
698:         AddOperation({ WriteIOCON, data});
699:         break;
700:     case IODIRA:
701:         AddOperation({ WriteIODIRA, data });
702:         break;
703:     case IODIRB:
704:         AddOperation({ WriteIODIRB, data });
705:         break;
706:     case GPIOA:
707:         AddOperation({ WriteGPIOA, data});
708:         break;
709:     case GPIOB:
710:         AddOperation({ WriteGPIOB, data});
711:         break;
712:     case GPINTENA:
713:         AddOperation({ WriteGPINTENA, data});
714:         break;
715:     case GPINTENB:
716:         AddOperation({ WriteGPINTENB, data});
717:         break;
718:     case INTCONA:
719:         AddOperation({ WriteINTCONA, data});
720:         break;
721:     case INTCONB:
722:         AddOperation({ WriteINTCONB, data});
723:         break;
724:     case DEFVALA:
725:         AddOperation({ WriteDEFVALA, data});
726:         break;
727:     case DEFVALB:
728:         AddOperation({ WriteDEFVALB, data});
729:         break;
730:     case IPOLA:
731:         AddOperation({ WriteIPOLA, data});
732:         break;
733:     case IPOLB:
734:         AddOperation({ WriteIPOLB, data});
735:         break;
736:     case GPPUA:
737:         AddOperation({ WriteGPPUA, data});
738:         break;
739:     case GPPUB:
740:         AddOperation({ WriteGPPUB, data});
741:         break;
742:     case OLATA:
743:         AddOperation({ WriteOLATA, data});
744:         break;
745:     case OLATB:
746:         AddOperation({ WriteOLATB, data});
747:         break;
748:     }
749:     return true;
750: }
751: 
752: /// <summary>
753: /// Add a memory access operation to the list
754: /// </summary>
755: /// <param name="operation">Operation to add</param>
756: void MemoryAccessMCP23017Mock::AddOperation(const MCP23017Operation& operation)
757: {
758:     assert(m_numOps < BufferSize);
759:     m_ops[m_numOps++] = operation;
760: }
```

- Line 58-184: We define a function `OperationCodeToString()` as before to convert a MCP23017 operation code to string
- Line 186-509: We implemnt the `Serialize()` function to serialize a MCP23017 operation to string
- Line 512: We define the static class member `m_pThis` which points to the singleton `MemoryAccessMCP23017Mock` instance
- Line 514- 526: We implement the `MemoryAccessMCP23017Mock` constructor
- Line 528-535: We implement the method `GetNumMCP23017Operations()`
- Line 537-546: We implement the method `GetMCP23017Operation()`
- Line 548-554: We implement the method `ResetCycle()` in the same way as for the stub we created before
- Line 556-565: We implement the method `OnSendAddress()`
- Line 567-580: We implement the method `OnRecvData()`. This method is slightly different than the stub, as we refactored out the method `OnReadRegister()`
- Line 582-602: We implement the method `OnSendData()`. This method is slightly different than the stub, as we refactored out the method `OnWriteRegister()`
- Line 604-682: We implement the new method `OnReadRegister()`. This simply generates operations for each register read
- Line 684-750: We implement the new method `OnWriteRegister()`. This simply generates operations for each register written
- Line 752-760: We implement the method `AddOperation()` like we did before

### MCP23017Test.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_8_MCP23017TESTCPP}

Let's now write a test for the `I2CMaster` class.

We'll write a test where the is no I2C device at the given address, and we expect a NACK error.
We'll also write a test where a device is present and responds correctly.

Create the file `code\libraries\baremetal\test\src\MCP23017Test.cpp`

```cpp
File: code\libraries\baremetal\test\src\MCP23017Test.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MCP23017Test.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MCP23017Test
9: //
10: // Description : MCP23017 tests
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
40: #include "device/mocks/MemoryAccessMCP23017Mock.h"
41: 
42: #include "baremetal/BCMRegisters.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/I2CMaster.h"
45: #include "device/i2c/MCP23017.h"
46: 
47: #include "unittest/unittest.h"
48: 
49: /// @brief Define log name
50: LOG_MODULE("MCP23017Test");
51: 
52: using namespace baremetal;
53: using namespace unittest;
54: 
55: namespace device {
56: namespace test {
57: 
58: /// @brief Baremetal test suite
59: TEST_SUITE(Baremetal)
60: {
61: 
62: class MCP23017Test : public TestFixture
63: {
64: public:
65:     void SetUp() override
66:     {
67:     }
68:     void TearDown() override
69:     {
70:     }
71: };
72: 
73: TEST_FIXTURE(MCP23017Test, SetLEDSOn)
74: {
75:     uint8 busIndex = 1;
76:     uint8 address{0x20};
77:     MemoryAccessMCP23017Mock memoryAccess;
78:     {
79:         MCP23017 expander(memoryAccess);
80: 
81:         if (!expander.Initialize(busIndex, address))
82:         {
83:             LOG_INFO("Cannot initialize expander");
84:         }
85: 
86:         expander.GetPortAValue();
87:         expander.SetPortADirections(MCP23017PinDirection::Out);
88:         expander.SetPortBDirections(MCP23017PinDirection::Out);
89:         expander.SetPortAValue(0x55);
90:         expander.SetPortBValue(0xAA);
91:     }
92:     size_t indexI2COps{};
93: 
94:     EXPECT_EQ(size_t{126}, memoryAccess.GetNumI2CMasterOperations());
95:     
96:     // Initialize
97:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetClockDivider, busIndex, 3500 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
98:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
99:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
100:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
101:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
102:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
103:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
104:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0A }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
105:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x2C }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
106:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::EnableController, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
107:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
108:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
109:     // GetPortAValue()
110:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
111:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
112:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
113:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
114:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
115:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
116:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x12 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
117:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
118:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 1 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
119:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetReadMode, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
120:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
121:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ReadFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
122:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
123: 
124:     // SetPortADirections(MCP23017PinDirection::Out)
125:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
126:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
127:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
128:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
129:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
130:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
131:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0C }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
132:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
133:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetWriteMode, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
134:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
135:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
136: 
137:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
138:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
139:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
140:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
141:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
142:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
143:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
144:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
145:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
146:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
147: 
148:     // SetPortBDirections(MCP23017PinDirection::Out)
149:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
150:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
151:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
152:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
153:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
154:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
155:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0D }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
156:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
157:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
158:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
159: 
160:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
161:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
162:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
163:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
164:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
165:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
166:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x01 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
167:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
168:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
169:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
170: 
171:     // SetPortAValue(0x55)
172:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
173:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
174:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
175:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
176:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
177:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
178:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x12 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
179:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x55 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
180:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
181:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
182: 
183:     // SetPortBValue(0xAA)
184:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
185:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
186:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
187:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
188:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
189:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
190:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x13 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
191:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0xAA }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
192:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
193:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
194: 
195:     // SetPortADirections(MCP23017PinDirection::In)
196:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
197:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
198:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
199:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
200:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
201:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
202:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0C }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
203:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
204:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
205:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
206: 
207:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
208:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
209:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
210:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
211:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
212:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
213:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
214:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0xFF }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
215:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
216:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
217: 
218:     // SetPortBDirections(MCP23017PinDirection::In)
219:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
220:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
221:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
222:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
223:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
224:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
225:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x0D }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
226:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x00 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
227:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
228:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
229: 
230:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ClearFIFO, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
231:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetAddress, busIndex, address }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
232:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetClockStretchTimeout, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
233:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetAckError, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
234:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
235:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::SetDataLength, busIndex, 2 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
236:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0x01 }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
237:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::WriteFIFO, busIndex, 0xFF }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
238:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::StartTransfer, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
239:     EXPECT_EQ((I2CMasterOperation{ I2CMasterOperationCode::ResetDone, busIndex }), memoryAccess.GetI2CMasterOperation(indexI2COps++));
240: 
241:     EXPECT_EQ(memoryAccess.GetNumI2CMasterOperations(), indexI2COps);
242:     size_t indexMCP23017Ops{};
243: 
244:     EXPECT_EQ(size_t{12}, memoryAccess.GetNumMCP23017Operations());
245: 
246:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIOCON, 0x2C }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
247: 
248:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::ReadGPIOA, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
249: 
250:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPPUA, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
251:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIODIRA, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
252:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPPUB, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
253:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIODIRB, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
254: 
255:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPIOA, 0x55 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
256:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPIOB, 0xAA }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
257: 
258:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPPUA, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
259:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIODIRA, 0xFF }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
260:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteGPPUB, 0x00 }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
261:     EXPECT_EQ((MCP23017Operation{ MCP23017OperationCode::WriteIODIRB, 0xFF }), memoryAccess.GetMCP23017Operation(indexMCP23017Ops++));
262: 
263:     EXPECT_EQ(memoryAccess.GetNumMCP23017Operations(), indexMCP23017Ops);
264: }
265: 
266: } // suite Baremetal
267: 
268: } // namespace test
269: } // namespace device
```

- Line 62-71: We define a class `MCP23017Test` as the test fixture
- Line 73-264: We add a test `SetLEDSOn` which initializes MCP23017, sets the pin directions for port A and B, and then writes values to the ports
  - Line 75-91: This is the actual work done in the test. The functionality is very much like the application we wrote in [Scanning for actual device - Step 4](#TUTORIAL_22_I2C_SCANNING_FOR_ACTUAL_DEVICE___STEP_4)
  - Line 94-239: We test the operations from I2C master perspective.
This is quite a list which is therefore commented with the operations performed.
You'll notice that sometimes there is an operation to set read or write mode, when the mode changes (line 119, 133).
You'll also notice that at the end of the block, due to the destructor for `MCP23017` the pins are put back in input mode
  - Line 241-263: We test the operations for the MCP23017 itself.
This list is much shorted, and clearly resembles the operations performed
### Configuring, building and debugging {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_8_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will run the tests. The test should succeed.

```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (1 fixture)
[  FIXTURE  ] MCP23017Test (1 test)
Info   0.00:00:00.030 Initialize 20 (MCP23017:79)
Info   0.00:00:00.030 Initialize bus 1, mode 0, config 0 (I2CMaster:166)
Info   0.00:00:00.040 Set clock 100000 (I2CMaster:207)
Info   0.00:00:00.040 Set up bus 1, config 0, base address FE804000 (I2CMaster:190)
[ SUCCEEDED ] Baremetal::MCP23017Test::SetLEDSOn
[  FIXTURE  ] 1 test from MCP23017Test
[   SUITE   ] 1 fixture from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.060 Halt (System:121)
```

This has been a quite extensive overview of how we can write tests for devices very close to hardware, by using a mock `MemoryAccess` implementation, which is then stepwise extended to support the actual device.
From here on, I will be adding tests, but will not explain the tests in this detail, as the focus is on the actual code interacting with devices.

Next: [26-i2c-display](26-i2c-display.md)
