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
The image below shows the class diagram for 23017 and related classes.
As you can see we introduced quite some interface classes:
- IMemoryAccess
- IGPIOPin
- II2CMaster

<img src="images/MCP23017.svg"  alt="MCP 23017 class diagram" width="800"/>

We start making unit tests for each of the classes, starting with the GPIO pin, and ultimately arriving at MCP23017.

We'll start with a test for `PhysicalGPIOPin`.
See the image below.

<img src="images/PhysicalGPIOPinTest.svg"  alt="PhysicalGPIOPinTest class diagram" width="800"/>

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

### PhysicalGPIOPinTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_1_PHYSICALGPIOPINTESTCPP}

Let's create a first test.

Create the file `code\libraries\baremetal\test\src\PhysicalGPIOPinTest.cpp`

```cpp
File: code\libraries\baremetal\test\src\PhysicalGPIOPinTest.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : PhysicalGPIOPinTest.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : PhysicalGPIOPinTest
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
55:     class PhysicalGPIOPinTest : public TestFixture
56:     {
57:     public:
58:         void SetUp() override
59:         {
60:         }
61:         void TearDown() override
62:         {
63:         }
64:     };
65: 
66:     TEST_FIXTURE(PhysicalGPIOPinTest, SimpleTest)
67:     {
68:         MemoryAccessMock memoryAccess;
69:         PhysicalGPIOPin pin(memoryAccess);
70: 
71:         pin.AssignPin(0);
72:         pin.SetMode(GPIOMode::AlternateFunction0);
73:     }
74: 
75: } // suite Baremetal
76: 
77: } // namespace test
78: } // namespace baremetal
```

- Line 66-73: We create a test `SimpleTest`
  - Line 68: We create a `MemoryAccessMock` instance
  - Line 69: We create a `PhysicalGPIOPin` instance with the `MemoryAccessMock` instance injected
  - Line 71-72: We initialize the GPIO pin

### Run selected tests {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_1_RUN_SELECTED_TESTS}

As the total amount of tests in the `Baremetal` test suite is growing quite large, for now let's limit the tests run.

Update the file `code\libraries\baremetal\test\src\main.cpp`

```cpp
File: code\libraries\baremetal\test\src\main.cpp
File: d:\Projects\RaspberryPi\baremetal.github\code\libraries\baremetal\test\src\main.cpp
1: #include "baremetal/System.h"
2: #include "unittest/unittest.h"
3: 
4: using namespace baremetal;
5: using namespace unittest;
6: 
7: int main()
8: {
9:     ConsoleTestReporter reporter;
10:     RunSelectedTests(&reporter, InSelection("Baremetal", "PhysicalGPIOPinTest", nullptr));
11: //    RunAllTests(&reporter);
12: 
13:     return static_cast<int>(ReturnCode::ExitHalt);
14: }
15: 
```

- Line 10: Instead of `RunAllTests` we use `RunSelectedTests`, where we use the predicate `InSelection` using the test suite `Baremetal` and the test fixture `PhysicalGPIOPinTest`

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
[  FIXTURE  ] PhysicalGPIOPinTest (1 test)
Info   0.00:00:00.050 Read32(FE2000E4) (MemoryAccessMock.cpp:100)
Info   0.00:00:00.060 Write32(FE2000E4, 00000000) (MemoryAccessMock.cpp:111)
Info   0.00:00:00.060 Read32(FE200000) (MemoryAccessMock.cpp:100)
Info   0.00:00:00.060 Write32(FE200000, 00000004) (MemoryAccessMock.cpp:111)
[ SUCCEEDED ] Baremetal::PhysicalGPIOPinTest::SerializeChar
[  FIXTURE  ] 1 test from PhysicalGPIOPinTest
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

Now this is all nice, but this way we cannot test whether the results is what we expect.
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
73:     /// Constructor for read operations, without data
74:     /// </summary>
75:     /// <param name="aIsWriteOperation">True if this is a write operation, false for a read operation. Should be false.</param>
76:     /// <param name="aAddress">Addres to read from / write to</param>
77:     MemoryAccessOperation(bool aIsWriteOperation, regaddr aAddress)
78:         : isWriteOperation{aIsWriteOperation}
79:         , address{aAddress}
80:         , data{}
81:     {
82:     }
83:     /// <summary>
84:     /// Constructor for write operations, with data
85:     /// </summary>
86:     /// <param name="aIsWriteOperation">True if this is a write operation, false for a read operation. Should be true.</param>
87:     /// <param name="aAddress">Addres to read from / write to</param>
88:     /// <param name="aData">Data to write</param>
89:     MemoryAccessOperation(bool aIsWriteOperation, regaddr aAddress, uint32 aData)
90:         : isWriteOperation{aIsWriteOperation}
91:         , address{aAddress}
92:         , data{aData}
93:     {
94:     }
95:     /// <summary>
96:     /// Check memory access operations for equality
97:     /// </summary>
98:     /// <param name="other">Value to compare to</param>
99:     /// <returns>True if equal, false otherwise</returns>
100:     bool operator == (const MemoryAccessOperation &other) const
101:     {
102:         return (other.isWriteOperation == isWriteOperation) &&
103:             (other.address == address) &&
104:             (!isWriteOperation || (other.data == data));
105:     }
106:     /// <summary>
107:     /// Check memory access operations for inequality
108:     /// </summary>
109:     /// <param name="other">Value to compare to</param>
110:     /// <returns>True if unequal, false otherwise</returns>
111:     bool operator != (const MemoryAccessOperation &other) const
112:     {
113:         return !operator==(other);
114:     }
115: };
116: 
117: String Serialize(const MemoryAccessOperation& value);
118: 
119: /// <summary>
120: /// Memory access mock class
121: /// </summary>
122: class MemoryAccessMock : public IMemoryAccess
123: {
124: private:
125:     /// @brief Size of memory access operation array
126:     static constexpr size_t BufferSize = 1000;
127:     /// List op memory access operations
128:     MemoryAccessOperation m_accessOps[BufferSize] ALIGN(8);
129:     /// @brief Number of registered memory acess operations
130:     size_t m_numAccessOps;
131: 
132: public:
133:     MemoryAccessMock();
134: 
135:     size_t GetNumOperations() const;
136:     const MemoryAccessOperation& GetOperation(size_t index) const;
137: 
138:     uint8 Read8(regaddr address) override;
139:     void Write8(regaddr address, uint8 data) override;
140: 
141:     uint16 Read16(regaddr address) override;
142:     void Write16(regaddr address, uint16 data) override;
143: 
144:     uint32 Read32(regaddr address) override;
145:     void Write32(regaddr address, uint32 data) override;
146: };
147: 
148: } // namespace baremetal
```

- Line 51-115: We declare and implement a struct `MemoryAccessOperation` which contains data on a memory access operation
  - Line 56-57: The member variable `isWriteOperation` is used to distinguish between write operations (with data) and read operations (without data)
  - Line 58-59: The member variable `address` contains the address read from or written to
  - Line 60-61: The member variable `data` contains the data written for write operations
  - Line 63-71: We declare and implement the default constructor
  - Line 72-82: We declare and implement the constructor intended for read operations
  - Line 83-94: We declare and implement the constructor intended for write operations
  - Line 95-105: We declare and implement the equality operator.
  This will check whether the values are equal, for write operations also taking into account the data, for read operations the data is don't care
  - Line 106-114: We declare and implement the inequality operator
- Line 117: We declare the serialization function for `MemoryAccessOperation`, which is needed when comparing in a test
- Line 119-146: We update the `MemoryAccessMock` class
  - Line 125-126: We add a constant to indicate the size of the array holding the memory access operations
  - Line 127-128: We declare the array `m_accessOps` holding the memory access operations
  - Line 129-130: We declare the member variable `m_numAccessOps` which keeps track of the actual number of registered operations
  - Line 133: We declare the default constructor
  - Line 135: We declare a method `GetNumOperations()` to retrieve the actual number of registered operations
  - Line 136: We declare a method `GetOperation()` to retrieve a specific operation by index

### MemoryAccessMock.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_2_MEMORYACCESSMOCKCPP}

We'll implement the `MemoryAccessMock` class.

Update the file `code\libraries\baremetal\test\src\MemoryAccessMock.cpp`

```cpp
File: code\libraries\baremetal\test\src\MemoryAccessMock.cpp
...
40: #include "baremetal/Assert.h"
41: #include "baremetal/Format.h"
42: #include "baremetal/Logger.h"
43: #include "MemoryAccessMock.h"
44: 
45: /// @file
46: /// Memory access mock class implementation
47: 
48: /// @brief Define log name
49: LOG_MODULE("MemoryAccessMock");
50: 
51: using namespace baremetal;
52: 
53: /// <summary>
54: /// Serialize a memory access operation to string
55: /// </summary>
56: /// <param name="value">Value to be serialized</param>
57: /// <returns>Resulting string</returns>
58: String baremetal::Serialize(const MemoryAccessOperation &value)
59: {
60:     if (value.isWriteOperation)
61:         return Format("W: %p %08lx", value.address, value.data);
62:     return Format("R: %p", value.address);
63: }
64: 
65: /// <summary>
66: /// Default constructor
67: /// </summary>
68: MemoryAccessMock::MemoryAccessMock()
69:     : m_accessOps{}
70:     , m_numAccessOps{}
71: {
72: }
73: 
74: /// <summary>
75: /// Return number of registered memory access operations
76: /// </summary>
77: /// <returns>Number of registered memory access operations</returns>
78: size_t MemoryAccessMock::GetNumOperations() const
79: {
80:     return m_numAccessOps;
81: }
82: 
83: /// <summary>
84: /// Retrieve a registered memory access operation from the list
85: /// </summary>
86: /// <param name="index">Index of operation</param>
87: /// <returns>Requested memory access operation</returns>
88: const MemoryAccessOperation &MemoryAccessMock::GetOperation(size_t index) const
89: {
90:     assert(index < m_numAccessOps);
91:     return m_accessOps[index];
92: }
93: 
94: /// <summary>
95: /// Read a 8 bit value from register at address
96: /// </summary>
97: /// <param name="address">Address of register</param>
98: /// <returns>8 bit register value</returns>
99: uint8 MemoryAccessMock::Read8(regaddr address)
100: {
101:     assert(m_numAccessOps < BufferSize);
102:     m_accessOps[m_numAccessOps++] = { false, address };
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
114:     assert(m_numAccessOps < BufferSize);
115:     m_accessOps[m_numAccessOps++] = { true, address, static_cast<uint32>(data) };
116:     TRACE_INFO("Write8(%p, %02x)", address, data);
117: }
118: 
119: /// <summary>
120: /// Read a 16 bit value from register at address
121: /// </summary>
122: /// <param name="address">Address of register</param>
123: /// <returns>16 bit register value</returns>
124: uint16 MemoryAccessMock::Read16(regaddr address)
125: {
126:     assert(m_numAccessOps < BufferSize);
127:     m_accessOps[m_numAccessOps++] = { false, address };
128:     TRACE_INFO("Read16(%p)", address);
129:     return {};
130: }
131: 
132: /// <summary>
133: /// Write a 16 bit value to register at address
134: /// </summary>
135: /// <param name="address">Address of register</param>
136: /// <param name="data">Data to write</param>
137: void MemoryAccessMock::Write16(regaddr address, uint16 data)
138: {
139:     assert(m_numAccessOps < BufferSize);
140:     m_accessOps[m_numAccessOps++] = { true, address, static_cast<uint32>(data) };
141:     TRACE_INFO("Write16(%p, %04x)", address, data);
142: }
143: 
144: /// <summary>
145: /// Read a 32 bit value from register at address
146: /// </summary>
147: /// <param name="address">Address of register</param>
148: /// <returns>32 bit register value</returns>
149: uint32 MemoryAccessMock::Read32(regaddr address)
150: {
151:     assert(m_numAccessOps < BufferSize);
152:     m_accessOps[m_numAccessOps++] = { false, address };
153:     TRACE_INFO("Read32(%p)", address);
154:     return {};
155: }
156: 
157: /// <summary>
158: /// Write a 32 bit value to register at address
159: /// </summary>
160: /// <param name="address">Address of register</param>
161: /// <param name="data">Data to write</param>
162: void MemoryAccessMock::Write32(regaddr address, uint32 data)
163: {
164:     assert(m_numAccessOps < BufferSize);
165:     m_accessOps[m_numAccessOps++] = { true, address, data };
166:     TRACE_INFO("Write32(%p, %08x)", address, data);
167: }
```

- Line 53-63: We implement the serialization of `MemoryAccessOperation`
- Line 65-72: We implement the `MemoryAccessMock` constructor
- Line 74-81: We implement the method `GetNumOperations()`
- Line 83-92: We implement the method `GetOperation()`
- Line 94-105: We change the method `Read8()` to register a read call
- Line 107-117: We change the method `Write8()` to register a write call
- Line 119-130: We change the method `Read16()` to register a read call
- Line 132-142: We change the method `Write16()` to register a write call
- Line 144-155: We change the method `Read32()` to register a read call
- Line 157-167: We change the method `Write32()` to register a write call

### PhysicalGPIOPinTest.cpp {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_2_PHYSICALGPIOPINTESTCPP}

We'll update the test, and take care of both Raspberry Pi 3 and 4 platforms.

Update the file `code\libraries\baremetal\test\src\PhysicalGPIOPinTest.cpp`

```cpp
File: code\libraries\baremetal\test\src\PhysicalGPIOPinTest.cpp
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
56:     class PhysicalGPIOPinTest : public TestFixture
57:     {
58:     public:
59:         void SetUp() override
60:         {
61:         }
62:         void TearDown() override
63:         {
64:         }
65:     };
66: 
67:     TEST_FIXTURE(PhysicalGPIOPinTest, SimpleTest)
68:     {
69:         MemoryAccessMock memoryAccess;
70:         PhysicalGPIOPin pin(memoryAccess);
71: 
72:         pin.AssignPin(0);
73:         pin.SetMode(GPIOMode::AlternateFunction0);
74: 
75:         size_t index{};
76: #if BAREMETAL_RPI_TARGET == 3
77:         EXPECT_EQ(size_t{5}, memoryAccess.GetNumOperations());
78:         EXPECT_EQ((MemoryAccessOperation{ true, RPI_GPIO_GPPUD }), memoryAccess.GetOperation(index++));
79:         EXPECT_EQ((MemoryAccessOperation{ true, RPI_GPIO_GPPUDCLK0, 0x00000001 }), memoryAccess.GetOperation(index++));
80:         EXPECT_EQ((MemoryAccessOperation{ true, RPI_GPIO_GPPUDCLK0, 0x00000000 }), memoryAccess.GetOperation(index++));
81: #else
82:         EXPECT_EQ(size_t{4}, memoryAccess.GetNumOperations());
83:         EXPECT_EQ((MemoryAccessOperation{ false, RPI_GPIO_GPPUPPDN0 }), memoryAccess.GetOperation(index++));
84:         EXPECT_EQ((MemoryAccessOperation{ true, RPI_GPIO_GPPUPPDN0, 0x00000000 }), memoryAccess.GetOperation(index++));
85: #endif
86:         EXPECT_EQ((MemoryAccessOperation{ false, RPI_GPIO_GPFSEL0 }), memoryAccess.GetOperation(index++));
87:         EXPECT_EQ((MemoryAccessOperation{ true, RPI_GPIO_GPFSEL0, 0x00000004 }), memoryAccess.GetOperation(index++));
88:     }
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
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (3 fixtures)
[  FIXTURE  ] PhysicalGPIOPinTest (1 test)
Info   0.00:00:00.030 Write32(3F200094, 00) (MemoryAccessMock.cpp:149)
Info   0.00:00:00.040 Write32(3F200098, 01) (MemoryAccessMock.cpp:149)
Info   0.00:00:00.050 Write32(3F200098, 00) (MemoryAccessMock.cpp:149)
Info   0.00:00:00.050 Read32(3F200000) (MemoryAccessMock.cpp:136)
Info   0.00:00:00.050 Write32(3F200000, 04) (MemoryAccessMock.cpp:149)
[ SUCCEEDED ] Baremetal::PhysicalGPIOPinTest::SimpleTest
[  FIXTURE  ] 1 test from PhysicalGPIOPinTest
[   SUITE   ] 3 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.070 Halt (System:121)
```

For Raspberry Pi 4:
```text
Setting up UART0
Info   0.00:00:00.000 Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:93)
Info   0.00:00:00.010 Starting up (System:213)
[===========] Running 1 test from 1 fixture in 1 suite.
[   SUITE   ] Baremetal (3 fixtures)
[  FIXTURE  ] PhysicalGPIOPinTest (1 test)
Info   0.00:00:00.030 Read32(FE2000E4) (MemoryAccessMock.cpp:136)
Info   0.00:00:00.040 Write32(FE2000E4, 00) (MemoryAccessMock.cpp:149)
Info   0.00:00:00.040 Read32(FE200000) (MemoryAccessMock.cpp:136)
Info   0.00:00:00.040 Write32(FE200000, 04) (MemoryAccessMock.cpp:149)
[ SUCCEEDED ] Baremetal::PhysicalGPIOPinTest::SimpleTest
[  FIXTURE  ] 1 test from PhysicalGPIOPinTest
[   SUITE   ] 3 fixtures from Baremetal
Success: 1 test passed.

No failures
[===========] 1 test from 1 fixture in 1 suite ran.
Info   0.00:00:00.070 Halt (System:121)
```

## Creating actual unit tests - Step 3 {#TUTORIAL_25_WRITING_UNIT_TESTS_CREATING_ACTUAL_UNIT_TESTS___STEP_2}

Now let's put some intelligence into all this. We already write a stub for the GPIO in class `MemoryAccessStubGPIO`, which interprets the read and written, let's make the registration of memory access operations smarter for GPIO.

### MemoryAccessGPIOMock.h

Create the file `code\libraries\baremetal\test\include\MemoryAccessGPIOMock.h`

```cpp
File: code\libraries\baremetal\test\include\MemoryAccessGPIOMock.h
...
```

Next: [25-lcd](25-lcd.md)
