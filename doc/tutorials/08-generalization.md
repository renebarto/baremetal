# Tutorial 08: Generalization {#TUTORIAL_08_GENERALIZATION}

@tableofcontents

## Tutorial information {#TUTORIAL_08_GENERALIZATION_TUTORIAL_INFORMATION}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/08-generalization`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_08_GENERALIZATION_TUTORIAL_INFORMATION_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-08.a`
- a library `output/Debug/lib/stdlib-08.a`
- an application `output/Debug/bin/08-generalization.elf`
- an image in `deploy/Debug/08-generalization-image`

Notice that we add a new library here.

## Generalizing code {#TUTORIAL_08_GENERALIZATION_GENERALIZING_CODE}

So far, we've added some code for UART, GPIO, and for system startup.

You may have noticed some things:
- The code for UART1 involves configuring GPIO pins. GPIO is however functionality in itself, so it would be better to separate it out.
- Also, we will want to support UART0 next to UART1 for output, so it may be wise to create a common interface.
- Writing to registers means writing to memory. It might be practical to abstract memory access, so we can later plug in a fake for testing.

## Generic memory access - Step 1 {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1}

We'll add an abstract interface `IMemoryAccess` that is implemented by the class `MemoryAccess`.
We will pass the interface to classes that need to write to memory, such as `UART1` and `System`.

First, let's create the interface.

### IMemoryAccess.h {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_IMEMORYACCESSH}

Let's create the abstract interface first.

Create the file `code/libraries/baremetal/include/baremetal/IMemoryAccess.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/IMemoryAccess.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : IMemoryAccess.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : IMemoryAccess
9: //
10: // Description : Memory read/write abstract interface
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
42: #include "baremetal/Types.h"
43: 
44: /// @file
45: /// Abstract memory access interface
46: 
47: namespace baremetal {
48: 
49: /// <summary>
50: /// Abstract memory access interface
51: /// </summary>
52: class IMemoryAccess
53: {
54: public:
55:     /// <summary>
56:     /// Default destructor needed for abstract interface
57:     /// </summary>
58:     virtual ~IMemoryAccess() = default;
59: 
60:     /// <summary>
61:     /// Read a 8 bit value from register at address
62:     /// </summary>
63:     /// <param name="address">Address of register</param>
64:     /// <returns>8 bit register value</returns>
65:     virtual uint8 Read8(regaddr address) = 0;
66:     /// <summary>
67:     /// Write a 8 bit value to register at address
68:     /// </summary>
69:     /// <param name="address">Address of register</param>
70:     /// <param name="data">Data to write</param>
71:     virtual void Write8(regaddr address, uint8 data) = 0;
72:     /// <summary>
73:     /// Read, modify and write a 8 bit value to register at address
74:     ///
75:     /// The operation will read the value from the specified register, then AND it with the inverse of the mask (8 bits) provided
76:     /// The data provided (8 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
77:     /// The result will then be written back to the register
78:     /// </summary>
79:     /// <param name="address">Address of register</param>
80:     /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
81:     /// <param name="data">Data to write (after shifting left by shift bits)</param>
82:     /// <param name="shift">Shift to apply to the data to write (shift left)</param>
83:     virtual void ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift) = 0;
84:     /// <summary>
85:     /// Read a 16 bit value from register at address
86:     /// </summary>
87:     /// <param name="address">Address of register</param>
88:     /// <returns>16 bit register value</returns>
89:     virtual uint16 Read16(regaddr address) = 0;
90:     /// <summary>
91:     /// Write a 16 bit value to register at address
92:     /// </summary>
93:     /// <param name="address">Address of register</param>
94:     /// <param name="data">Data to write</param>
95:     virtual void Write16(regaddr address, uint16 data) = 0;
96:     /// <summary>
97:     /// Read, modify and write a 16 bit value to register at address
98:     ///
99:     /// The operation will read the value from the specified register, then AND it with the inverse of the mask (16 bits) provided
100:     /// The data provided (16 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
101:     /// The result will then be written back to the register
102:     /// </summary>
103:     /// <param name="address">Address of register</param>
104:     /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
105:     /// <param name="data">Data to write (after shifting left by shift bits)</param>
106:     /// <param name="shift">Shift to apply to the data to write (shift left)</param>
107:     virtual void ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift) = 0;
108:     /// <summary>
109:     /// Read a 32 bit value from register at address
110:     /// </summary>
111:     /// <param name="address">Address of register</param>
112:     /// <returns>32 bit register value</returns>
113:     virtual uint32 Read32(regaddr address) = 0;
114:     /// <summary>
115:     /// Write a 32 bit value to register at address
116:     /// </summary>
117:     /// <param name="address">Address of register</param>
118:     /// <param name="data">Data to write</param>
119:     virtual void Write32(regaddr address, uint32 data) = 0;
120:     /// <summary>
121:     /// Read, modify and write a 32 bit value to register at address
122:     ///
123:     /// The operation will read the value from the specified register, then AND it with the inverse of the mask (32 bits) provided
124:     /// The data provided (32 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
125:     /// The result will then be written back to the register
126:     /// </summary>
127:     /// <param name="address">Address of register</param>
128:     /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
129:     /// <param name="data">Data to write (after shifting left by shift bits)</param>
130:     /// <param name="shift">Shift to apply to the data to write (shift left)</param>
131:     virtual void ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift) = 0;
132: };
133: 
134: } // namespace baremetal
```

As you can see, we have an interface that provides sets of three methods:
- one to read
- one to write
- one to perform a read-modify-write cycle using a mask and a shift.

The idea behind the read-modify-write method is that we often need to read a value, and then replace some bits, and write back.
So what this method does:
- read memory into x
- calculate x = x & ~mask (keep everything but the bits in mask)
- calculate x = x | ((data << shift) & mask) (add the data bits shifted left, with mask)
- write x to memory

We have three sets of these methods:
- one for 8 bit access
- one for 16 bit access
- one for 32 bit access

So altogether this is a bit more extensive than what we did so far in the `MemoryAccess` class.

### MemoryAccess.h {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_MEMORYACCESSH}

We'll have to update the `MemoryAccess` class to derive from the interface `IMemoryAccess`.
This also means adding some methodes,  as well as making methods virtual instead of static.

Update the file `code/libraries/baremetal/include/baremetal/MemoryAccess.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryAccess.h
...
42: #include "baremetal/IMemoryAccess.h"
43: 
44: /// @file
45: /// Memory access class
46: 
47: namespace baremetal {
48: 
49: /// <summary>
50: /// Memory access interface
51: /// </summary>
52: class MemoryAccess : public IMemoryAccess
53: {
54: public:
55:     uint8  Read8(regaddr address) override;
56:     void   Write8(regaddr address, uint8 data) override;
57:     void   ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift) override;
58:     uint16 Read16(regaddr address) override;
59:     void   Write16(regaddr address, uint16 data) override;
60:     void   ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift) override;
61:     uint32 Read32(regaddr address) override;
62:     void   Write32(regaddr address, uint32 data) override;
63:     void   ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift) override;
64: };
65: 
66: MemoryAccess &GetMemoryAccess();
67: 
68: } // namespace baremetal
```

So we create make class `MemoryAccess` derive from `IMemoryAccess`.
Next to this, we create an accessor function for the singleton `MemoryAccess` instance.

### MemoryAccess.cpp {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_MEMORYACCESSCPP}

Next we update the implementation for `MemoryAccess`.

Update the file `code/libraries/baremetal/src/MemoryAccess.cpp`.

```cpp
File: code/libraries/baremetal/src/MemoryAccess.cpp
...
47: /// <summary>
48: /// Read a 8 bit value from register at address
49: /// </summary>
50: /// <param name="address">Address of register</param>
51: /// <returns>8 bit register value</returns>
52: uint8 MemoryAccess::Read8(regaddr address)
53: {
54:     return *reinterpret_cast<uint8 volatile*>(address);
55: }
56: 
57: /// <summary>
58: /// Write a 8 bit value to register at address
59: /// </summary>
60: /// <param name="address">Address of register</param>
61: /// <param name="data">Data to write</param>
62: void MemoryAccess::Write8(regaddr address, uint8 data)
63: {
64:     *reinterpret_cast<uint8 volatile*>(address) = data;
65: }
66: 
67: /// <summary>
68: /// Read, modify and write a 8 bit value to register at address
69: ///
70: /// The operation will read the value from the specified register, then AND it with the inverse of the mask (8 bits) provided
71: /// The data provided (8 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
72: /// The result will then be written back to the register
73: /// </summary>
74: /// <param name="address">Address of register</param>
75: /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
76: /// <param name="data">Data to write (after shifting left by shift bits)</param>
77: /// <param name="shift">Shift to apply to the data to write (shift left)</param>
78: void MemoryAccess::ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift)
79: {
80:     auto value = Read8(address);
81:     value &= ~mask;
82:     value |= ((data << shift) & mask);
83:     Write8(address, value);
84: }
85: 
86: /// <summary>
87: /// Read a 16 bit value from register at address
88: /// </summary>
89: /// <param name="address">Address of register</param>
90: /// <returns>16 bit register value</returns>
91: uint16 MemoryAccess::Read16(regaddr address)
92: {
93:     return *reinterpret_cast<uint16 volatile*>(address);
94: }
95: 
96: /// <summary>
97: /// Write a 16 bit value to register at address
98: /// </summary>
99: /// <param name="address">Address of register</param>
100: /// <param name="data">Data to write</param>
101: void MemoryAccess::Write16(regaddr address, uint16 data)
102: {
103:     *reinterpret_cast<uint16 volatile*>(address) = data;
104: }
105: 
106: /// <summary>
107: /// Read, modify and write a 16 bit value to register at address
108: ///
109: /// The operation will read the value from the specified register, then AND it with the inverse of the mask (16 bits) provided
110: /// The data provided (16 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
111: /// The result will then be written back to the register
112: /// </summary>
113: /// <param name="address">Address of register</param>
114: /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
115: /// <param name="data">Data to write (after shifting left by shift bits)</param>
116: /// <param name="shift">Shift to apply to the data to write (shift left)</param>
117: void MemoryAccess::ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift)
118: {
119:     auto value = Read16(address);
120:     value &= ~mask;
121:     value |= ((data << shift) & mask);
122:     Write16(address, value);
123: }
124: 
125: /// <summary>
126: /// Read a 32 bit value from register at address
127: /// </summary>
128: /// <param name="address">Address of register</param>
129: /// <returns>32 bit register value</returns>
130: uint32 MemoryAccess::Read32(regaddr address)
131: {
132:     return *reinterpret_cast<uint32 volatile*>(address);
133: }
134: 
135: /// <summary>
136: /// Write a 32 bit value to register at address
137: /// </summary>
138: /// <param name="address">Address of register</param>
139: /// <param name="data">Data to write</param>
140: void MemoryAccess::Write32(regaddr address, uint32 data)
141: {
142:     *reinterpret_cast<uint32 volatile*>(address) = data;
143: }
144: 
145: /// <summary>
146: /// Read, modify and write a 32 bit value to register at address
147: ///
148: /// The operation will read the value from the specified register, then AND it with the inverse of the mask (32 bits) provided
149: /// The data provided (32 bits) will be masked with the mask provided, shifted left bit shift bits, and then OR'ed with the mask read data
150: /// The result will then be written back to the register
151: /// </summary>
152: /// <param name="address">Address of register</param>
153: /// <param name="mask">Mask to apply. Value read will be masked with the inverse of mask, then the data (after shift) will be masked with mask before OR'ing with the value read</param>
154: /// <param name="data">Data to write (after shifting left by shift bits)</param>
155: /// <param name="shift">Shift to apply to the data to write (shift left)</param>
156: void MemoryAccess::ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift)
157: {
158:     auto value = Read32(address);
159:     value &= ~mask;
160:     value |= ((data << shift) & mask);
161:     Write32(address, value);
162: }
163: 
164: /// <summary>
165: /// Construct the singleton memory access interface if needed, and return a reference to the instance
166: /// </summary>
167: /// <returns>Reference to the singleton memory access interface</returns>
168: MemoryAccess& baremetal::GetMemoryAccess()
169: {
170:     static MemoryAccess value;
171:     return value;
172: }
```

As you can see, the implementation is using a reinterpration cast to a uint16 / uint32 pointer for the 16 bit and 32 bit variants, and using the pointer itself for the 8 bit variant.

We can now start replacing the direct memory access with calls to the methods of IMemoryAccess.

### Update UART1 code {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_UPDATE_UART1_CODE}

#### UART1.h {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_UPDATE_UART1_CODE_UART1H}

So, we update the class `UART1` to use IMemoryAccess calls.
The class `UART1` will have two constructors:
- A private default constructor, which is used by friend function `GetUART1()`
- A public constructor taking a `IMemoryAccess` reference. This can be used to inject a non-default `IMemoryAccess` instance

Update the file `code/libraries/baremetal/include/baremetal/UART1.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
...
113: class IMemoryAccess;
114: 
115: /// <summary>
116: /// Encapsulation for the UART1 device.
117: ///
118: /// This is a pseudo singleton, in that it is not possible to create a default instance (GetUART1() needs to be used for this),
119: /// but it is possible to create an instance with a custom IMemoryAccess instance for testing.
120: /// </summary>
121: class UART1
122: {
123:     /// <summary>
124:     /// Construct the singleton UART1 instance if needed, and return a reference to the instance. This is a friend function of class UART1
125:     /// </summary>
126:     /// <returns>Reference to the singleton UART1 instance</returns>
127:     friend UART1& GetUART1();
128: 
129: private:
130:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
131:     bool m_isInitialized;
132:     /// @brief Memory access interface reference for accessing registers.
133:     IMemoryAccess& m_memoryAccess;
134:     /// @brief Baudrate set for device
135:     unsigned m_baudrate;
136: 
137:     UART1();
138: 
139: public:
140:     UART1(IMemoryAccess& memoryAccess);
141: 
142:     void Initialize(unsigned baudrate);
143:     unsigned GetBaudrate() const;
144:     char Read();
145:     void Write(char c);
146:     void WriteString(const char* str);
147: 
148: private:
149:     // Set GPIO pin mode
150:     bool SetMode(MCP23017Pin pinNumber, GPIOMode mode);
151:     // Set GPIO pin function
152:     bool SetFunction(MCP23017Pin pinNumber, GPIOFunction function);
153:     // Set GPIO pin pull mode
154:     bool SetPullMode(MCP23017Pin pinNumber, GPIOPullMode pullMode);
155:     // Switch GPIO off
156:     bool Off(MCP23017Pin pinNumber, GPIOMode mode);
157: };
158: 
159: UART1& GetUART1();
160: 
161: } // namespace baremetal
```

- Line 112: We forward declare the class IMemoryAccess
- Line 133: We now add a member variable to contain a reference to a IMemoryAccess instantiation
- Line 137: We still keep a default contructor, but make it private.
This means only our accessor function can create an default instance of UART1, making it a true singleton.
- Line 140: We add a method to create an instance with a specific memory access instance injected.
This can later be used for testing.

#### UART1.cpp {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_UPDATE_UART1_CODE_UART1CPP}

Let's update the implementation:

Update the file `code/libraries/baremetal/src/UART1.cpp`.

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...
74: /// <summary>
75: /// Constructs a default UART1 instance.
76: ///
77: /// Note that the constructor is private, so GetUART1() is needed to instantiate the UART1.
78: /// </summary>
79: UART1::UART1()
80:     : m_isInitialized{}
81:     , m_memoryAccess{GetMemoryAccess()}
82: {
83: }
84: 
85: /// <summary>
86: /// Constructs a specialized UART1 instance with a custom IMemoryAccess instance. This is intended for testing.
87: /// </summary>
88: /// <param name="memoryAccess">Memory access interface</param>
89: UART1::UART1(IMemoryAccess& memoryAccess)
90:     : m_isInitialized{}
91:     , m_memoryAccess{memoryAccess}
92: {
93: }
...
```

- Line 74-83: The default constructor implementation initializes `m_memoryAccess` with the `MemoryAccess` singleton using `GetMemoryAccess()`
- Line 85-93: The other constructor takes initializes `m_memoryAccess` with the `IMemoryAccess` instance passed as a parameter.

We also need to update the code actually reading and writing registers:

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...
95: /// <summary>
96: /// Initialize the UART1 device. Only performed once, guarded by m_isInitialized.
97: ///
98: ///  Set baud rate and characteristics (8N1) and map to GPIO. Baud rate maximum is 115200
99: /// </summary>
100: /// <param name="baudrate">Baud rate to set</param>
101: void UART1::Initialize(unsigned baudrate)
102: {
103:     if (m_isInitialized)
104:         return;
105: 
106:     // initialize UART
107:     auto value = m_memoryAccess.Read32(RPI_AUX_ENABLES);
108:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value & ~RPI_AUX_ENABLES_UART1); // Disable UART1, AUX mini uart
109: 
110:     SetMode(14, GPIOMode::AlternateFunction5);
111: 
112:     SetMode(15, GPIOMode::AlternateFunction5);
113: 
114:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value | RPI_AUX_ENABLES_UART1);                                                                                              // enable UART1, AUX mini uart
115:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, 0);                                                                                                                          // Disable Tx, Rx
116:     m_memoryAccess.Write32(RPI_AUX_MU_LCR, RPI_AUX_MU_LCR_DATA_SIZE_8);                                                                                                  // 8 bit mode
117:     m_memoryAccess.Write32(RPI_AUX_MU_MCR, RPI_AUX_MU_MCR_RTS_HIGH);                                                                                                     // RTS high
118:     m_memoryAccess.Write32(RPI_AUX_MU_IER, 0);                                                                                                                           // Disable interrupts
119:     m_memoryAccess.Write32(RPI_AUX_MU_IIR, RPI_AUX_MU_IIR_TX_FIFO_ENABLE | RPI_AUX_MU_IIR_RX_FIFO_ENABLE | RPI_AUX_MU_IIR_TX_FIFO_CLEAR | RPI_AUX_MU_IIR_RX_FIFO_CLEAR); // Clear FIFO
120:     m_memoryAccess.Write32(RPI_AUX_MU_BAUD, RPI_AUX_MU_BAUD_VALUE(baudrate));                                                                                            // Set baudrate
121:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, RPI_AUX_MU_CNTL_ENABLE_RX | RPI_AUX_MU_CNTL_ENABLE_TX);                                                                      // Enable Tx, Rx
122: 
123:     m_baudrate = baudrate;
124:     m_isInitialized = true;
125: }
126: 
127: /// <summary>
128: /// Return set baudrate
129: /// </summary>
130: /// <returns>Baudrate set for device</returns>
131: unsigned UART1::GetBaudrate() const
132: {
133:     return m_baudrate;
134: }
135: 
136: /// <summary>
137: /// Receive a character
138: /// </summary>
139: /// <returns>Character received</returns>
140: char UART1::Read()
141: {
142:     // wait until something is in the buffer
143:     // Check Rx FIFO holds data
144:     while (!(m_memoryAccess.Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LSR_RX_READY))
145:     {
146:         NOP();
147:     }
148:     // Read it and return
149:     return static_cast<char>(m_memoryAccess.Read32(RPI_AUX_MU_IO));
150: }
151: 
152: /// <summary>
153: /// Send a character
154: /// </summary>
155: /// <param name="c">Character to be sent</param>
156: void UART1::Write(char c)
157: {
158:     // wait until we can send
159:     // Check Tx FIFO empty
160:     while (!(m_memoryAccess.Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LSR_TX_EMPTY))
161:     {
162:         NOP();
163:     }
164:     // Write the character to the buffer
165:     m_memoryAccess.Write32(RPI_AUX_MU_IO, static_cast<uint32>(c));
166: }
167: 
168: /// <summary>
169: /// Write a string
170: /// </summary>
171: /// <param name="str">String to be written</param>
172: void UART1::WriteString(const char* str)
173: {
174:     while (*str)
175:     {
176:         // convert newline to carriage return + newline
177:         if (*str == '\n')
178:             Write('\r');
179:         Write(*str++);
180:     }
181: }
182: 
183: bool UART1::SetMode(MCP23017Pin pinNumber, GPIOMode mode)
184: {
185:     if (pinNumber >= NUM_GPIO)
186:         return false;
187:     if (mode >= GPIOMode::Unknown)
188:         return false;
189:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
190:     {
191:         if (!SetPullMode(pinNumber, GPIOPullMode::Off))
192:             return false;
193: 
194:         if (!SetFunction(pinNumber,
195:                          static_cast<GPIOFunction>(static_cast<unsigned>(mode) - static_cast<unsigned>(GPIOMode::AlternateFunction0) + static_cast<unsigned>(GPIOFunction::AlternateFunction0))))
196:             return false;
197:     }
198:     else if (GPIOMode::Output == mode)
199:     {
200:         if (!SetPullMode(pinNumber, GPIOPullMode::Off))
201:             return false;
202: 
203:         if (!SetFunction(pinNumber, GPIOFunction::Output))
204:             return false;
205:     }
206:     else
207:     {
208:         if (!SetPullMode(pinNumber, (mode == GPIOMode::InputPullUp) ? GPIOPullMode::PullUp : (mode == GPIOMode::InputPullDown) ? GPIOPullMode::PullDown : GPIOPullMode::Off))
209:             return false;
210:         if (!SetFunction(pinNumber, GPIOFunction::Input))
211:             return false;
212:     }
213:     if (mode == GPIOMode::Output)
214:         Off(pinNumber, mode);
215:     return true;
216: }
217: 
218: bool UART1::SetFunction(MCP23017Pin pinNumber, GPIOFunction function)
219: {
220:     if (pinNumber >= NUM_GPIO)
221:         return false;
222:     if (function >= GPIOFunction::Unknown)
223:         return false;
224: 
225:     regaddr selectRegister = RPI_GPIO_GPFSEL0 + (pinNumber / 10) * 4;
226:     uint32 shift = (pinNumber % 10) * 3;
227: 
228:     static const unsigned FunctionMap[] = {0, 1, 4, 5, 6, 7, 3, 2};
229: 
230:     uint32 value = m_memoryAccess.Read32(selectRegister);
231:     value &= ~(7 << shift);
232:     value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
233:     m_memoryAccess.Write32(selectRegister, value);
234:     return true;
235: }
236: 
237: bool UART1::SetPullMode(MCP23017Pin pinNumber, GPIOPullMode pullMode)
238: {
239:     if (pullMode >= GPIOPullMode::Unknown)
240:         return false;
241:     if (pinNumber >= NUM_GPIO)
242:         return false;
243: 
244: #if BAREMETAL_RPI_TARGET == 3
245:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (pinNumber / 32) * 4;
246:     uint32 shift = pinNumber % 32;
247: 
248:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
249:     WaitCycles(NumWaitCycles);
250:     m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
251:     WaitCycles(NumWaitCycles);
252:     m_memoryAccess.Write32(clkRegister, 0);
253: #else
254:     regaddr modeReg = RPI_GPIO_GPPUPPDN0 + (pinNumber / 16) * 4;
255:     unsigned shift = (pinNumber % 16) * 2;
256: 
257:     static const unsigned ModeMap[3] = {0, 2, 1};
258: 
259:     uint32 value = m_memoryAccess.Read32(modeReg);
260:     value &= ~(3 << shift);
261:     value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
262:     m_memoryAccess.Write32(modeReg, value);
263: #endif
264: 
265:     return true;
266: }
267: 
268: bool UART1::Off(MCP23017Pin pinNumber, GPIOMode mode)
269: {
270:     if (pinNumber >= NUM_GPIO)
271:         return false;
272: 
273:     // Output level can be set in input mode for subsequent switch to output
274:     if (mode >= GPIOMode::Unknown)
275:         return false;
276: 
277:     unsigned regOffset = (pinNumber / 32) * 4;
278:     uint32 regMask = 1 << (pinNumber % 32);
279: 
280:     bool value = false;
281: 
282:     regaddr setClrReg = (value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset;
283: 
284:     m_memoryAccess.Write32(setClrReg, regMask);
285: 
286:     return true;
287: }
...
```

So you can see that wherever we were using the static members of `MemoryAccess`, we now call methods on the `IMemoryAccess` interface.

### Update System code {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_UPDATE_SYSTEM_CODE}

#### System.h {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_UPDATE_SYSTEM_CODE_SYSTEMH}

We update System to use IMemoryAccess calls. For this, we will also pass in the memory access reference to the constructor.

Update the file `code/libraries/baremetal/include/baremetal/System.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/System.h
...
49: class IMemoryAccess;
50: 
51: /// <summary>
52: /// System startup / shutdown handling class
53: /// </summary>
54: class System
55: {
56:     /// <summary>
57:     /// Construct the singleton System instance if needed, and return a reference to the instance. This is a friend function of class System
58:     /// </summary>
59:     /// <returns>Reference to the singleton system instance</returns>
60:     friend System& GetSystem();
61: 
62: private:
63:     /// @brief Memory access interface reference for accessing registers.
64:     IMemoryAccess& m_memoryAccess;
65: 
66:     System();
67: 
68: public:
69:     System(IMemoryAccess& memoryAccess);
70: 
71:     [[noreturn]] void Halt();
72:     [[noreturn]] void Reboot();
73: };
...
```

- Line 49: We forward declare the class IMemoryAccess
- Line 64: We now add a member variable to contain a reference to a IMemoryAccess instantiation
- Line 66: We still keep a default contructor, but make it private.
This means only our accessor function can create an default instance of System, making it a true singleton.
- Line 69: We add a method to create an instance with a specific memory access instance injected.
This can later be used for testing.

#### System.cpp {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_UPDATE_SYSTEM_CODE_SYSTEMCPP}

Let's update the implementation:

Update the file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
...
54: /// <summary>
55: /// Constructs a default System instance. Note that the constructor is private, so GetSystem() is needed to instantiate the System.
56: /// </summary>
57: System::System()
58:     : m_memoryAccess{GetMemoryAccess()}
59: {
60: }
61: 
62: /// <summary>
63: /// Constructs a specialized System instance with a custom IMemoryAccess instance. This is intended for testing.
64: /// </summary>
65: /// <param name="memoryAccess">Memory access interface</param>
66: System::System(IMemoryAccess& memoryAccess)
67:     : m_memoryAccess{memoryAccess}
68: {
69: }
...
```

- Line 54-60: The default constructor implementation initializes `m_memoryAccess` with the `MemoryAccess` singleton using `GetMemoryAccess()`
- Line 62-69: The other constructor takes initializes `m_memoryAccess` with the `IMemoryAccess` instance passed as a parameter.

We also need to update the code actually reading and writing registers:

Update the file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
71: /// <summary>
72: /// Halts the system. This function will not return
73: /// </summary>
74: void System::Halt()
75: {
76:     GetUART1().WriteString("Halt\n");
77: 
78:     for (int i = 0; i < 1000000; ++i)
79:         NOP();
80: 
81:     // power off the SoC (GPU + CPU)
82:     auto r = m_memoryAccess.Read32(RPI_PWRMGT_RSTS);
83:     r &= ~RPI_PWRMGT_RSTS_PARTITION_CLEAR;
84:     r |= RPI_PARTITIONVALUE(63); // Partition 63 used to indicate halt
85:     m_memoryAccess.Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r);
86:     m_memoryAccess.Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | 10);
87:     m_memoryAccess.Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);
88: 
89:     for (;;) // Satisfy [[noreturn]]
90:     {
91:         DataSyncBarrier();
92:         WaitForInterrupt();
93:     }
94: }
95: 
96: /// <summary>
97: /// Reboots the system. This function will not return
98: /// </summary>
99: void System::Reboot()
100: {
101:     GetUART1().WriteString("Reboot\n");
102: 
103:     DisableIRQs();
104:     DisableFIQs();
105: 
106:     for (int i = 0; i < 1000000; ++i)
107:         NOP();
108: 
109:     // power off the SoC (GPU + CPU)
110:     auto r = m_memoryAccess.Read32(RPI_PWRMGT_RSTS);
111:     r &= ~RPI_PWRMGT_RSTS_PARTITION_CLEAR;
112:     m_memoryAccess.Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r); // boot from partition 0
113:     m_memoryAccess.Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | 10);
114:     m_memoryAccess.Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_REBOOT);
115: 
116:     for (;;) // Satisfy [[noreturn]]
117:     {
118:         DataSyncBarrier();
119:         WaitForInterrupt();
120:     }
121: }
```

Building now will result in linker errors:

```text
>------ Build All started: Project: baremetal, Configuration: BareMetal-RPI3-Debug ------
  [1/10] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\Startup.S.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj -c ../code/libraries/baremetal/src/Startup.S
  [2/10] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -fanalyzer -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [3/10] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Util.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\Util.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Util.cpp.obj -c ../code/libraries/baremetal/src/Util.cpp
  [4/10] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/CXAGuard.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\CXAGuard.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/CXAGuard.cpp.obj -c ../code/libraries/baremetal/src/CXAGuard.cpp
  [5/10] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\MemoryAccess.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj -c ../code/libraries/baremetal/src/MemoryAccess.cpp
  [6/10] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/System.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\System.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/System.cpp.obj -c ../code/libraries/baremetal/src/System.cpp
  [7/10] D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_RPI_TARGET=3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\UART1.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj -c ../code/libraries/baremetal/src/UART1.cpp
  [8/10] cmd.exe /C "cd . && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E rm -f ..\output\RPI3\Debug\lib\libbaremetal.a && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ar.exe qc ..\output\RPI3\Debug\lib\libbaremetal.a  code/libraries/baremetal/CMakeFiles/baremetal.dir/src/CXAGuard.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/System.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Util.cpp.obj && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ranlib.exe ..\output\RPI3\Debug\lib\libbaremetal.a && cd ."
  [9/10] cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -o ..\output\RPI3\Debug\bin\demo.elf  -Wl,--start-group  ../output/RPI3/Debug/lib/libbaremetal.a  -Wl,--end-group && cd ."
  FAILED: ../output/RPI3/Debug/bin/demo.elf 
  cmd.exe /C "cd . && D:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\Toolchains\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.3.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -o ..\output\RPI3\Debug\bin\demo.elf  -Wl,--start-group  ../output/RPI3/Debug/lib/libbaremetal.a  -Wl,--end-group && cd ."
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/RPI3/Debug/lib/libbaremetal.a(MemoryAccess.cpp.obj): in function `baremetal::GetMemoryAccess()':
  D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug/../code/libraries/baremetal/src/MemoryAccess.cpp:170:(.text+0x324): undefined reference to `__dso_handle'
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug/../code/libraries/baremetal/src/MemoryAccess.cpp:170:(.text+0x328): undefined reference to `__dso_handle'
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug/../code/libraries/baremetal/src/MemoryAccess.cpp:170:(.text+0x33c): undefined reference to `__cxa_atexit'
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/RPI3/Debug/lib/libbaremetal.a(MemoryAccess.cpp.obj): in function `baremetal::IMemoryAccess::~IMemoryAccess()':
  D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug/../code/libraries/baremetal/include/baremetal/IMemoryAccess.h:58:(.text._ZN9baremetal13IMemoryAccessD0Ev[_ZN9baremetal13IMemoryAccessD5Ev]+0x1c): undefined reference to `operator delete(void*, unsigned long)'
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/RPI3/Debug/lib/libbaremetal.a(MemoryAccess.cpp.obj): in function `baremetal::MemoryAccess::~MemoryAccess()':
  D:\Projects\baremetal\cmake-Baremetal-RPI3-Debug/../code/libraries/baremetal/include/baremetal/MemoryAccess.h:52:(.text._ZN9baremetal12MemoryAccessD0Ev[_ZN9baremetal12MemoryAccessD5Ev]+0x1c): undefined reference to `operator delete(void*, unsigned long)'
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: ..\output\RPI3\Debug\bin\demo.elf: hidden symbol `__dso_handle' isn't defined
  D:/Toolchains/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.3.1/../../../../aarch64-none-elf/bin/ld.exe: final link failed: bad value
D:\Projects\baremetal\cmake-BareMetal-RPI3-Debug\collect2.exe : error : ld returned 1 exit status
  ninja: build stopped: subcommand failed.

Build All failed.
```

A number of symbols cannot be found:
- `__dso_handle`
- `__cxa_atexit`
- `operator delete(void*, unsigned long)`

All of these are again caused by variables or functions we should implement.
The details are not really worthwhile discussing here, we'll get to that later when dealing with memory management.

- `__dso_handle` is a "guard" that is used to identify DSO (Dynamic Shared Objects) during global destruction. It is a variable of type `void*`.
- `__cxa_atexit()` is a C function in that registers a function to be called by exit() or when a shared library is unloaded.
It is used to register a function to be called when the program terminates normally, there can be multiple functions registered.
The functions are called in reverse order of registration, i.e., the function registered last will be executed first.
- `operator delete(void*, unsigned long)` is an operator to deallocate a block of memory. It requires that another operator is defined: `operator delete(void*)`
This operator is needed because we now inherit from an abstract class `IMemoryAccess`, which however does have a (virtual) desctructor.
Calling the destructor will de-allocate memory

In short, all this has to do with application shutdown (which we will not need for now), and memory management (which we will deal with later).
So for now, simple stub implementations are sufficient.

The first two will be implemented in `System.cpp`.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
54: #ifdef __cplusplus
55: extern "C" {
56: #endif
57: 
58: /// <summary>
59: /// __dso_handle is a "guard" that is used to identify dynamic shared objects during global destruction. It is only known to the compiler / linker
60: /// </summary>
61: void* __dso_handle WEAK;
62: 
63: /// @brief WEAK version of __cxa_atexit
64: void __cxa_atexit(void (*func)(void* param), void* param, void* handle) WEAK;
65: 
66: /// <summary>
67: /// __cxa_atexit() will call the destructor of the static of a dynamic library when this dynamic library is unloaded before the program exits.
68: /// </summary>
69: /// <param name="func">Registered function to be called</param>
70: /// <param name="param">Parameter to be passed to registered function</param>
71: /// <param name="handle">Handle of the shared library to be unloaded (its __dso_handle)</param>
72: void __cxa_atexit([[maybe_unused]] void (*func)(void* param), [[maybe_unused]] void* param, [[maybe_unused]] void* handle)
73: {
74: }
75: 
76: #ifdef __cplusplus
77: }
78: #endif
79: 
...
```

The term WEAK needs to be defined still. This has to do with fallbacks in case the linker cannot find a symbol. GCC has a special attribute for this.
As you can see, `__cxa_atexit` is defined as a stub (empty)implementation.

We will put the definition for `WEAK` in `Macros.h`.

Update the file `code/libraries/baremetal/include/baremetal/Macros.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Macros.h
...
45: /// @brief Make a variable a weak instance (GCC compiler only)
46: #define WEAK    __attribute__((weak))
47: 
48: /// @brief Convert bit index into integer with zero bit
49: /// @param n Bit index
50: #define BIT0(n) (0)
51: /// @brief Convert bit index into integer with one bit
52: /// @param n Bit index
53: #define BIT1(n) (1UL << (n))
```

For the delete operators, we will create a new header and source file, to start preparing for memory management.

Create the file `code/libraries/baremetal/include/baremetal/New.h`

```cpp
File: code/libraries/baremetal/include/baremetal/New.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : New.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Generic memory allocation functions
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
42: /// @file
43: /// Basic memory allocation functions
44: 
45: #include "baremetal/Types.h"
46: 
47: void operator delete(void* address) noexcept;
48: void operator delete(void* address, size_t size) noexcept;
```

Create the file `code/libraries/baremetal/src/New.cpp`

```cpp
File: code/libraries/baremetal/src/New.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : New.cpp
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Generic memory allocation functions
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
40: #include "baremetal/New.h"
41: 
42: /// @file
43: /// Basic memory allocation functions implementation
44: 
45: /// <summary>
46: /// Standard de-allocation for single value.
47: /// </summary>
48: /// <param name="address">Address to memory block to free</param>
49: void operator delete(void* address) noexcept
50: {
51: }
52: 
53: /// <summary>
54: /// Standard de-allocation with size for single value.
55: /// </summary>
56: /// <param name="address">Address to memory block to free</param>
57: /// <param name="size">Size of memory block to free</param>
58: void operator delete(void* address, size_t size) noexcept
59: {
60: }
```

Again, the delete operators are for now implemented as stubs.

### Configuring, building and debugging {#TUTORIAL_08_GENERALIZATION_GENERIC_MEMORY_ACCESS___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code.

The code will not behave differently, but will be more well-structured.

## Separating out GPIO code - Step 2 {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_GPIO_CODE___STEP_2}

Now let's separate out the GPIO functionality. We'll create a new class PhysicalGPIOPin which will implement the functionality.

Raspberry Pi also has virtual GPIO pins, hence the name. We'll cover these later.

Again, to enable testing later on, we'll create an abstract interface IGPIOPin, which we'll derive from.

### IGPIOPin.h {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_GPIO_CODE___STEP_2_IGPIOPINH}

Create the file `code/libraries/baremetal/include/baremetal/IGPIOPin.h`

```cpp
File: code/libraries/baremetal/include/baremetal/IGPIOPin.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : IGPIOPin.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : IGPIOPin
9: //
10: // Description : GPIO pin abstraction
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
42: /// @file
43: /// Abstract GPIO pin. Could be either a virtual or physical pin
44: 
45: #include "baremetal/Types.h"
46: 
47: namespace baremetal {
48: 
49: /// @brief GPIO mode
50: enum class GPIOMode
51: {
52:     /// @brief GPIO used as input
53:     Input,
54:     /// @brief GPIO used as output
55:     Output,
56:     /// @brief GPIO used as input, using pull-up
57:     InputPullUp,
58:     /// @brief GPIO used as input, using pull-down
59:     InputPullDown,
60:     /// @brief GPIO used as Alternate Function 0. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
61:     AlternateFunction0,
62:     /// @brief GPIO used as Alternate Function 1. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
63:     AlternateFunction1,
64:     /// @brief GPIO used as Alternate Function 2. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
65:     AlternateFunction2,
66:     /// @brief GPIO used as Alternate Function 3. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
67:     AlternateFunction3,
68:     /// @brief GPIO used as Alternate Function 4. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
69:     AlternateFunction4,
70:     /// @brief GPIO used as Alternate Function 5. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
71:     AlternateFunction5,
72:     /// @brief GPIO mode unknown / not set / invalid
73:     Unknown,
74: };
75: 
76: /// <summary>
77: /// Abstraction of a GPIO pin
78: /// </summary>
79: class IGPIOPin
80: {
81: public:
82:     /// <summary>
83:     /// Default destructor needed for abstract interface
84:     /// </summary>
85:     virtual ~IGPIOPin() = default;
86: 
87:     /// <summary>
88:     /// Return pin number (high bit = 0 for a phsical pin, 1 for a virtual pin)
89:     /// </summary>
90:     /// <returns>Pin number</returns>
91:     virtual uint8 GetPinNumber() const = 0;
92:     /// <summary>
93:     /// Assign a GPIO pin
94:     /// </summary>
95:     /// <param name="pinNumber">Pin number</param>
96:     /// <returns>true if successful, false otherwise</returns>
97:     virtual bool AssignPin(MCP23017Pin pinNumber) = 0;
98: 
99:     /// <summary>
100:     /// Switch GPIO on
101:     /// </summary>
102:     virtual void On() = 0;
103:     /// <summary>
104:     /// Switch GPIO off
105:     /// </summary>
106:     virtual void Off() = 0;
107:     /// <summary>
108:     /// Get GPIO value
109:     /// </summary>
110:     /// <returns>GPIO value, true if on, false if off</returns>
111:     virtual bool Get() = 0;
112:     /// <summary>
113:     /// Set GPIO on (true) or off (false)
114:     /// </summary>
115:     /// <param name="on">Value to set, on (true) or off (false)</param>
116:     virtual void Set(bool on) = 0;
117:     /// <summary>
118:     /// Invert GPIO value on->off off->on
119:     /// </summary>
120:     virtual void Invert() = 0;
121: };
122: 
123: } // namespace baremetal
```

- Line 49-74: We declare an enum class `GPIOMode`, which is the combined GPIO pin mode.
- Line 76-121: We declare an abstract class IGPIOPin, which is the interface of a GPIO pin.
  - Line 82-85: We declare a virtual destructor, needed for an abstract class, an give it a default implementation
  - Line 87-91: We declare a pure virtual method `GetPinNumber()` to return the number of a GPIO pin
  - Line 92-97: We declare a pure virtual method `AssignPin()` to assign a pin number to a GPIO pin
  - Line 99-102: We declare a pure virtual method `On()` to switch an output pin on (this has no effect for an input pin)
  - Line 103-106: We declare a pure virtual method `Off()` to switch an output pin off (this has no effect for an input pin)
  - Line 107-111: We declare a pure virtual method `Get()` to return the status of an input or output pin (true is on, false is off)
  - Line 112-116: We declare a pure virtual method `Set()` to switch an output pin on (true) or off (false) (this has no effect for an input pin)
  - Line 117-120: We declare a pure virtual method `Invert()` to toggle an output pin (on->off and off->on) (this has no effect for an input pin)

### PhysicalGPIOPin.h {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_GPIO_CODE___STEP_2_PHYSICALGPIOPINH}

Create the file `code/libraries/baremetal/include/baremetal/PhysicalGPIOPin.h`

```cpp
File: code/libraries/baremetal/include/baremetal/PhysicalGPIOPin.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : PhysicalGPIOPin.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : PhysicalGPIOPin
9: //
10: // Description : Physical GPIO pin
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
42: #include "baremetal/IGPIOPin.h"
43: #include "baremetal/MemoryAccess.h"
44: 
45: /// @file
46: /// Physical GPIO pin
47: 
48: namespace baremetal {
49: 
50: /// @brief GPIO function
51: enum class GPIOFunction;
52: 
53: /// @brief GPIO pull mode
54: enum class GPIOPullMode;
55: 
56: /// @brief Total count of GPIO pins, numbered from 0 through 53
57: #define NUM_GPIO 54
58: 
59: /// <summary>
60: /// Physical GPIO pin (i.e. available on GPIO header)
61: /// </summary>
62: class PhysicalGPIOPin : public IGPIOPin
63: {
64: private:
65:     /// @brief Configured GPIO pin number (0..53)
66:     uint8 m_pinNumber;
67:     /// @brief Configured GPIO mode. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
68:     GPIOMode m_mode;
69:     /// @brief Configured GPIO function.
70:     GPIOFunction m_function;
71:     /// @brief Configured GPIO pull mode (only for input function).
72:     GPIOPullMode m_pullMode;
73:     /// @brief Current value of the GPIO pin (true for on, false for off).
74:     bool m_value;
75:     /// @brief Memory access interface reference for accessing registers.
76:     IMemoryAccess& m_memoryAccess;
77: 
78: public:
79:     PhysicalGPIOPin(IMemoryAccess& memoryAccess = GetMemoryAccess());
80: 
81:     PhysicalGPIOPin(MCP23017Pin pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess = GetMemoryAccess());
82: 
83:     uint8 GetPinNumber() const override;
84:     bool AssignPin(MCP23017Pin pinNumber) override;
85: 
86:     void On() override;
87:     void Off() override;
88:     bool Get() override;
89:     void Set(bool on) override;
90:     void Invert() override;
91: 
92:     GPIOMode GetMode();
93:     bool SetMode(GPIOMode mode);
94:     GPIOFunction GetFunction();
95:     GPIOPullMode GetPullMode();
96:     void SetPullMode(GPIOPullMode pullMode);
97: 
98: private:
99:     void SetFunction(GPIOFunction function);
100: };
101: 
102: } // namespace baremetal
```

- Line 50-51: We forward declare an enum class `GPIOFunction`, which defines the function of a GPIO pin, so input, output, or an alternate function as shown before
- Line 53-54: We forward declare an enum class `GPIOPullMode`, which defines the pull up / down mode for a GPIO pin. This can be off (for an output pin or alternate function), pull up or down (for an input pin)
- Line 56-57: We define the constant `NUM_GPIO` as before
- Line 59-100: We delcare the class `PhysicalGPIOPin` which inherits from `IGPIOPin`
  - Line 65-66: We declare a member variable `m_pinNumber` to hold the assigned pin number (0..53)
  - Line 67-68: We declare a member variable `m_mode` to hold GPIO pin mode
  - Line 69-70: We declare a member variable `m_function` to hold GPIO pin function
  - Line 71-72: We declare a member variable `m_pullMode` to hold GPIO pin pull up / down mode
  - Line 73-74: We declare a member variable `m_value` to hold the current GPIO pin state for reading back the value of an output pin
  - Line 75-76: We declare a member variable `m_memoryAccess` to hold the memory access interface reference
  - Line 79: We declare a constructor which uses take a `IMemoryAccess` instance, by default the singleton instance
  - Line 81: We declare a constructor which takes a pin number, a mode, and a `IMemoryAccess` instance, by default the singleton instance
  - Line 83: We override the `GetPinNumber()` method
  - Line 84: We override the `AssignPin()` method
  - Line 86: We override the `On()` method
  - Line 87: We override the `Off()` method
  - Line 88: We override the `Get()` method
  - Line 89: We override the `Set()` method
  - Line 90: We override the `Invert()` method
  - Line 92: We declare a method `GetMode()` to retrieve the GPIO pin mode
  - Line 93: We declare a method `SetMode()` to set the GPIO pin mode
  - Line 94: We declare a method `GetFunction()` to retrieve the GPIO pin function
  - Line 95: We declare a method `GetPullMode()` to retrieve the GPIO pin pull mode
  - Line 96: We declare a method `SetPullMode()` to set the GPIO pin pull mode mode
  - Line 99: We declare a private method `SetFunction()` to set the GPIO pin function

### PhysicalGPIOPin.cpp {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_GPIO_CODE___STEP_2_PHYSICALGPIOPINCPP}

Create the file `code/libraries/baremetal/src/PhysicalGPIOPin.cpp`

```cpp
File: code/libraries/baremetal/src/PhysicalGPIOPin.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : PhysicalGPIOPin.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : PhysicalGPIOPin
9: //
10: // Description : Physical GPIO pin
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
40: #include "baremetal/PhysicalGPIOPin.h"
41: 
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/MemoryAccess.h"
45: 
46: /// @file
47: /// Physical GPIO pin implementation
48: 
49: namespace baremetal {
50: 
51: /// @brief GPIO function
52: enum class GPIOFunction
53: {
54:     /// @brief GPIO used as input
55:     Input,
56:     /// @brief GPIO used as output
57:     Output,
58:     /// @brief GPIO used as Alternate Function 0. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
59:     AlternateFunction0,
60:     /// @brief GPIO used as Alternate Function 1. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
61:     AlternateFunction1,
62:     /// @brief GPIO used as Alternate Function 2. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
63:     AlternateFunction2,
64:     /// @brief GPIO used as Alternate Function 3. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
65:     AlternateFunction3,
66:     /// @brief GPIO used as Alternate Function 4. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
67:     AlternateFunction4,
68:     /// @brief GPIO used as Alternate Function 5. See \ref RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO
69:     AlternateFunction5,
70:     /// @brief GPIO function unknown / not set / invalid
71:     Unknown,
72: };
73: 
74: /// @brief GPIO pull mode
75: enum class GPIOPullMode
76: {
77:     /// @brief GPIO pull mode off (no pull-up or pull-down)
78:     Off,
79:     /// @brief GPIO pull mode pull-down
80:     PullDown,
81:     /// @brief GPIO pull mode pull-up
82:     PullUp,
83:     /// @brief GPIO pull mode unknown / not set / invalid
84:     Unknown,
85: };
86: 
87: #if BAREMETAL_RPI_TARGET == 3
88: /// @brief Number of cycles to wait when setting pull mode for GPIO pin (Raspberry Pi 3 only)
89: static const int NumWaitCycles = 150;
90: 
91: static void WaitCycles(uint32 numCycles)
92: {
93:     if (numCycles)
94:     {
95:         while (numCycles--)
96:         {
97:             NOP();
98:         }
99:     }
100: }
101: #endif // BAREMETAL_RPI_TARGET == 3
102: 
103: /// <summary>
104: /// Creates a virtual GPIO pin
105: /// </summary>
106: /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
107: PhysicalGPIOPin::PhysicalGPIOPin(IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
108:     : m_pinNumber{NUM_GPIO}
109:     , m_mode{GPIOMode::Unknown}
110:     , m_function{GPIOFunction::Unknown}
111:     , m_pullMode{GPIOPullMode::Unknown}
112:     , m_value{}
113:     , m_memoryAccess{memoryAccess}
114: {
115: }
116: 
117: /// <summary>
118: /// Creates a virtual GPIO pin
119: /// </summary>
120: /// <param name="pinNumber">GPIO pin number (0..53)</param>
121: /// <param name="mode">Mode for the pin. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
122: /// <param name="memoryAccess">Memory access interface. Default is the Memory Access interface singleton</param>
123: PhysicalGPIOPin::PhysicalGPIOPin(MCP23017Pin pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess /*= m_memoryAccess*/)
124:     : m_pinNumber{NUM_GPIO}
125:     , m_mode{GPIOMode::Unknown}
126:     , m_value{}
127:     , m_memoryAccess{memoryAccess}
128: {
129:     AssignPin(pinNumber);
130:     SetMode(mode);
131: }
132: 
133: /// <summary>
134: /// Return the configured GPIO pin number
135: /// </summary>
136: /// <returns>GPIO pin number (0..53)</returns>
137: uint8 PhysicalGPIOPin::GetPinNumber() const
138: {
139:     return m_pinNumber;
140: }
141: 
142: /// <summary>
143: /// Assign a GPIO pin
144: /// </summary>
145: /// <param name="pinNumber">GPIO pin number to set (0..53)</param>
146: /// <returns>Return true on success, false on failure</returns>
147: bool PhysicalGPIOPin::AssignPin(MCP23017Pin pinNumber)
148: {
149:     // Check if pin already assigned
150:     if (m_pinNumber != NUM_GPIO)
151:         return false;
152:     m_pinNumber = pinNumber;
153: 
154:     return true;
155: }
156: 
157: /// <summary>
158: /// Switch GPIO on
159: /// </summary>
160: void PhysicalGPIOPin::On()
161: {
162:     Set(true);
163: }
164: 
165: /// <summary>
166: /// Switch GPIO off
167: /// </summary>
168: void PhysicalGPIOPin::Off()
169: {
170:     Set(false);
171: }
172: 
173: /// <summary>
174: /// Get GPIO value
175: /// </summary>
176: /// <returns>The status of the configured GPIO pin. Returns true if on, false if off</returns>
177: bool PhysicalGPIOPin::Get()
178: {
179:     // Check if pin is assigned
180:     if (m_pinNumber >= NUM_GPIO)
181:         return false;
182: 
183:     if ((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown))
184:     {
185:         uint32 regOffset = (m_pinNumber / 32);
186:         uint32 regMask = 1 << (m_pinNumber % 32);
187:         return (m_memoryAccess.Read32(RPI_GPIO_GPLEV0 + regOffset * 4) & regMask) ? true : false;
188:     }
189:     return m_value;
190: }
191: 
192: /// <summary>
193: /// Set GPIO on (true) or off (false)
194: /// </summary>
195: /// <param name="on">Value to set GPIO pin to (true for on, false for off).</param>
196: void PhysicalGPIOPin::Set(bool on)
197: {
198:     // Check if pin is assigned
199:     if (m_pinNumber >= NUM_GPIO)
200:         return;
201: 
202:     // Check if mode is output
203:     if (m_mode != GPIOMode::Output)
204:         return;
205: 
206:     m_value = on;
207: 
208:     uint32 regOffset = (m_pinNumber / 32);
209:     uint32 regMask = 1 << (m_pinNumber % 32);
210:     regaddr regAddress = (m_value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset * 4;
211: 
212:     m_memoryAccess.Write32(regAddress, regMask);
213: }
214: 
215: /// <summary>
216: /// Invert GPIO value on->off off->on
217: /// </summary>
218: void PhysicalGPIOPin::Invert()
219: {
220:     Set(!Get());
221: }
222: 
223: /// <summary>
224: /// Get the mode for the GPIO pin
225: /// </summary>
226: /// <returns>Currently set mode for the configured GPIO pin</returns>
227: GPIOMode PhysicalGPIOPin::GetMode()
228: {
229:     return m_mode;
230: }
231: 
232: /// <summary>
233: /// Convert GPIO mode to GPIO function. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.
234: /// </summary>
235: /// <param name="mode">GPIO mode</param>
236: /// <returns>GPIO function</returns>
237: static GPIOFunction ConvertGPIOModeToFunction(GPIOMode mode)
238: {
239:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
240:     {
241:         unsigned alternateFunctionIndex = static_cast<unsigned>(mode) - static_cast<unsigned>(GPIOMode::AlternateFunction0);
242:         return static_cast<GPIOFunction>(static_cast<unsigned>(GPIOFunction::AlternateFunction0) + alternateFunctionIndex);
243:     }
244:     else if (GPIOMode::Output == mode)
245:     {
246:         return GPIOFunction::Output;
247:     }
248:     return GPIOFunction::Input;
249: }
250: 
251: /// <summary>
252: /// Set the mode for the GPIO pin
253: /// </summary>
254: /// <param name="mode">Mode to be set for the configured GPIO. The mode is valid combination of the function and the pull mode. Only the input function has valid pull modes.</param>
255: /// <returns>Return true on success, false on failure</returns>
256: bool PhysicalGPIOPin::SetMode(GPIOMode mode)
257: {
258:     // Check if pin is assigned
259:     if (m_pinNumber >= NUM_GPIO)
260:         return false;
261: 
262:     // Check if mode is valid
263:     if (mode >= GPIOMode::Unknown)
264:         return false;
265: 
266:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
267:     {
268:         SetPullMode(GPIOPullMode::Off);
269: 
270:         SetFunction(ConvertGPIOModeToFunction(mode));
271:     }
272:     else if (GPIOMode::Output == mode)
273:     {
274:         SetPullMode(GPIOPullMode::Off);
275: 
276:         SetFunction(ConvertGPIOModeToFunction(mode));
277:     }
278:     else
279:     {
280:         SetPullMode((mode == GPIOMode::InputPullUp) ? GPIOPullMode::PullUp : (mode == GPIOMode::InputPullDown) ? GPIOPullMode::PullDown : GPIOPullMode::Off);
281:         SetFunction(ConvertGPIOModeToFunction(mode));
282:     }
283:     m_mode = mode;
284:     if (m_mode == GPIOMode::Output)
285:         Off();
286:     return true;
287: }
288: 
289: /// <summary>
290: /// Get GPIO pin function
291: /// </summary>
292: /// <returns>Function set for the configured GPIO pin</returns>
293: GPIOFunction PhysicalGPIOPin::GetFunction()
294: {
295:     return m_function;
296: }
297: 
298: /// <summary>
299: /// Get GPIO pin pull mode
300: /// </summary>
301: /// <returns>Pull mode set for the configured GPIO pin</returns>
302: GPIOPullMode PhysicalGPIOPin::GetPullMode()
303: {
304:     return m_pullMode;
305: }
306: 
307: /// <summary>
308: /// Set GPIO pin pull mode
309: /// </summary>
310: /// <param name="pullMode">Pull mode to be set for the configured GPIO pin</param>
311: void PhysicalGPIOPin::SetPullMode(GPIOPullMode pullMode)
312: {
313:     // Check if pin is assigned
314:     if (m_pinNumber >= NUM_GPIO)
315:         return;
316: 
317:     // Check if mode is valid
318:     if (pullMode >= GPIOPullMode::Unknown)
319:         return;
320: 
321: #if BAREMETAL_RPI_TARGET == 3
322:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (m_pinNumber / 32) * 4;
323:     uint32 shift = m_pinNumber % 32;
324: 
325:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
326:     WaitCycles(NumWaitCycles);
327:     m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
328:     WaitCycles(NumWaitCycles);
329:     m_memoryAccess.Write32(clkRegister, 0);
330: #else
331:     regaddr modeReg = RPI_GPIO_GPPUPPDN0 + (m_pinNumber / 16) * 4;
332:     unsigned shift = (m_pinNumber % 16) * 2;
333: 
334:     static const unsigned ModeMap[3] = {0, 2, 1};
335: 
336:     uint32 value = m_memoryAccess.Read32(modeReg);
337:     value &= ~(3 << shift);
338:     value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
339:     m_memoryAccess.Write32(modeReg, value);
340: #endif
341: 
342:     m_pullMode = pullMode;
343: }
344: 
345: /// <summary>
346: /// Set GPIO pin function
347: /// </summary>
348: /// <param name="function">Function to be set for the configured GPIO pin</param>
349: void PhysicalGPIOPin::SetFunction(GPIOFunction function)
350: {
351:     // Check if pin is assigned
352:     if (m_pinNumber >= NUM_GPIO)
353:         return;
354: 
355:     // Check if mode is valid
356:     if (function >= GPIOFunction::Unknown)
357:         return;
358: 
359:     regaddr selectRegister = RPI_GPIO_GPFSEL0 + (m_pinNumber / 10) * 4;
360:     uint32 shift = (m_pinNumber % 10) * 3;
361: 
362:     static const unsigned FunctionMap[] = {0, 1, 4, 5, 6, 7, 3, 2};
363: 
364:     uint32 value = m_memoryAccess.Read32(selectRegister);
365:     value &= ~(7 << shift);
366:     value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
367:     m_memoryAccess.Write32(selectRegister, value);
368:     m_function = function;
369: }
370: 
371: } // namespace baremetal
```

- Line 51-73: We define the enum class `GPIOFunction`, which was forward declared in the header file
- Line 74-86: We define the enum class `GPIOPullMode`, which was forward declared in the header file
- Line 91-100: We define the local function WaitCycles as before
- Line 103-115: We implement the first constructor taking a `IMemoryAccess` instance.
This sets the mode, function, and pull mode to invalid, and the pin number to `NUM_GPIO` also signalling an invalid value.
In other words, this constructor creates an unassigned pin
- Line 117-131: We implement the second constructor taking a pin number, a pin mode, and a `IMemoryAccess` instance.
This sets the pin number, mode, function, and pull mode to the values specified.
In other words, this constructor creates an assigned pin
- Line 133-140: We implement the `GetPinNumber()` method
- Line 142-155: We implement the `AssignPin()` method. This sets the pin number
- Line 157-163: We implement the `On()` method
- Line 165-171: We implement the `Off()` method, as before, but now using the `Set()` method
- Line 173-190: We implement the `Get()` method.
Note that we again calculate a register offset here as well as a mask, this time we read one of the registers `RPI_GPIO_GPLEV0`..`RPI_GPIO_GPLEV1`.
As we use one bit per GPIO, there are two registers, and there are 32 pins in one register
- Line 192-213: We implement the `Set()` method. As before we write to either the registers `RPI_GPIO_GPSET0`..`RPI_GPIO_GPSET1` or `RPI_GPIO_GPCLR0`..`RPI_GPIO_GPSET1` depending on whether we switch the output on or off
- Line 215-221: We implement the `Invert()` method
- Line 223-230: We implement the `GetMode()` method
- Line 232-249: We implement a local helper function to convert a `GPIOMode` to a `GPIOFUnction`
- Line 251-287: We implement the `SetMode()` method. This extracts the pin function and pull mode, and sets these.
As you can see, for an alternate function or an output we always set the pull mode to off
- Line 289-296: We implement the `GetFunction()` method
- Line 298-305: We implement the `GetPullMode()` method
- Line 307-343: We implement the `SetPullMode()` method.
This is similar to what we did before in the UART1 class
- Line 345-369: We implement the `SetFunction()` method.
This is similar to what we did before in the UART1 class
 
### Update UART1 code {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_GPIO_CODE___STEP_2_UPDATE_UART1_CODE}

We can now clean up the code for `UART1`, and make use of the `PhysicalGPIOPin` we just implemented.

#### UART1.h {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_GPIO_CODE___STEP_2_UPDATE_UART1_CODE_UART1H}

Update the file `code/libraries/baremetal/include/baremetal/UART1.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
...
40: #pragma once
41: 
42: /// @file
43: /// Raspberry Pi UART1 serial device declaration
44: 
45: /// @brief baremetal namespace
46: namespace baremetal {
47: 
48: class IMemoryAccess;
49: 
50: /// <summary>
51: /// Encapsulation for the UART1 device.
52: ///
53: /// This is a pseudo singleton, in that it is not possible to create a default instance (GetUART1() needs to be used for this),
54: /// but it is possible to create an instance with a custom IMemoryAccess instance for testing.
55: /// </summary>
56: class UART1
57: {
58:     /// <summary>
59:     /// Construct the singleton UART1 instance if needed, and return a reference to the instance. This is a friend function of class UART1
60:     /// </summary>
61:     /// <returns>Reference to the singleton UART1 instance</returns>
62:     friend UART1& GetUART1();
63: 
64: private:
65:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
66:     bool m_isInitialized;
67:     /// @brief Memory access interface reference for accessing registers.
68:     IMemoryAccess& m_memoryAccess;
69:     /// @brief Baudrate set for device
70:     unsigned m_baudrate;
71: 
72:     UART1();
73: 
74: public:
75:     UART1(IMemoryAccess& memoryAccess);
76: 
77:     void Initialize(unsigned baudrate);
78:     unsigned GetBaudrate() const;
79:     char Read();
80:     void Write(char c);
81:     void WriteString(const char* str);
82: };
83: 
84: UART1& GetUART1();
85: 
86: } // namespace baremetal
```

- Line 42: We remove the include for `Types.h` which is no longer needed
- Line 50-75: We remove the enum class GPIOMOde which is now in `IGPIOPin.h`
- Line 77-111: We remove the enum class definitions for GPIOFunction and GPIOPullMode which are now in `PhysicalIGPIOPin.cpp`
- Line 147-156: We remove the methods for GPIO which are noew in 'PhysicalGPIOPin.h`

#### UART1.cpp {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_GPIO_CODE___STEP_2_UPDATE_UART1_CODE_UART1CPP}

Update the file `code/libraries/baremetal/src/UART1.cpp`.

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...
40: #include "baremetal/UART1.h"
41: 
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/MemoryAccess.h"
45: #include "baremetal/PhysicalGPIOPin.h"
46: 
47: /// @file
48: /// Raspberry Pi UART1 serial device implementation
49: 
50: namespace baremetal {
51: 
52: /// <summary>
53: /// Constructs a default UART1 instance.
54: ///
55: /// Note that the constructor is private, so GetUART1() is needed to instantiate the UART1.
56: /// </summary>
57: UART1::UART1()
...
73: /// <summary>
74: /// Initialize the UART1 device. Only performed once, guarded by m_isInitialized.
75: ///
76: ///  Set baud rate and characteristics (8N1) and map to GPIO. Baud rate maximum is 115200
77: /// </summary>
78: /// <param name="baudrate">Baud rate to set</param>
79: void UART1::Initialize(unsigned baudrate)
80: {
81:     if (m_isInitialized)
82:         return;
83: 
84:     // initialize UART
85:     auto value = m_memoryAccess.Read32(RPI_AUX_ENABLES);
86:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value & ~RPI_AUX_ENABLES_UART1); // Disable UART1, AUX mini uart
87: 
88:     SetMode(14, GPIOMode::AlternateFunction5);
89: 
90:     SetMode(15, GPIOMode::AlternateFunction5);
91: 
92:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value | RPI_AUX_ENABLES_UART1);                                                                                              // enable UART1, AUX mini uart
93:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, 0);                                                                                                                          // Disable Tx, Rx
94:     m_memoryAccess.Write32(RPI_AUX_MU_LCR, RPI_AUX_MU_LCR_DATA_SIZE_8);                                                                                                  // 8 bit mode
95:     m_memoryAccess.Write32(RPI_AUX_MU_MCR, RPI_AUX_MU_MCR_RTS_HIGH);                                                                                                     // RTS high
96:     m_memoryAccess.Write32(RPI_AUX_MU_IER, 0);                                                                                                                           // Disable interrupts
97:     m_memoryAccess.Write32(RPI_AUX_MU_IIR, RPI_AUX_MU_IIR_TX_FIFO_ENABLE | RPI_AUX_MU_IIR_RX_FIFO_ENABLE | RPI_AUX_MU_IIR_TX_FIFO_CLEAR | RPI_AUX_MU_IIR_RX_FIFO_CLEAR); // Clear FIFO
98:     m_memoryAccess.Write32(RPI_AUX_MU_BAUD, RPI_AUX_MU_BAUD_VALUE(baudrate));                                                                                            // Set baudrate
99:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, RPI_AUX_MU_CNTL_ENABLE_RX | RPI_AUX_MU_CNTL_ENABLE_TX);                                                                      // Enable Tx, Rx
100: 
101:     m_baudrate = baudrate;
102:     m_isInitialized = true;
103: }
...
161: /// <summary>
162: /// Construct the singleton UART1 device if needed, and return a reference to the instance
163: /// </summary>
164: /// <returns>Reference to the singleton UART1 device</returns>
165: UART1& GetUART1()
166: {
167:     static UART1 value;
168: 
169:     return value;
170: }
171: 
172: } // namespace baremetal
```

- Line 45: We need to include `PhysicalGPIOPin.h`
- Line 50-51: We can remove the `NUM_GPIO` definition
- Line 55-73: We can remove the implementation for `WaitCycles()`
- Line 88: We now instantiate a local PhysicialGPIOPin instance (we will no longer need it once the GPIO pin is configured) to set up the TxD pin (14)
- Line 89: We now instantiate a local PhysicialGPIOPin instance (we will no longer need it once the GPIO pin is configured) to set up the RxD pin (15)
- Line 184-288: We can remove the implementation of methods now moved to `PhysicalGPIOPin.cpp`

### Configuring, building and debugging {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_GPIO_CODE___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The code will not behave differently, it's just cleaner in its structure.

## Separating out delay code - Step 3 {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_DELAY_CODE___STEP_3}

You will have noticed that we still have a delay loop in `PhysicalGPIOPin.cpp`.
This code is also generic, and can also be used to perform a busy form of waiting when rebooting or halting the system.
It therefore seems wise to separate out this code as well.

We will create a new class Timer, which for now will only have one method, but we will be using different forms of waiting using the system timer later on, as well as an interrupt based timer.

### Timer.h {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_DELAY_CODE___STEP_3_TIMERH}

Let's create the `Timer` class.

Create the file `code/libraries/baremetal/include/baremetal/Timer.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Timer.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Timer
9: //
10: // Description : Timer class
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
42: /// @file
43: /// Raspberry Pi Timer
44: 
45: #include "baremetal/Types.h"
46: 
47: namespace baremetal {
48: 
49: class IMemoryAccess;
50: 
51: /// <summary>
52: /// Timer class. For now only contains busy waiting methods
53: ///
54: /// Note that this class is created as a singleton, using the GetTimer() function.
55: /// </summary>
56: class Timer
57: {
58: public:
59:     static void WaitCycles(uint32 numCycles);
60: };
61: 
62: } // namespace baremetal
```

### Timer.cpp {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_DELAY_CODE___STEP_3_TIMERCPP}

Next we implement `Timer`.

Create the file `code/libraries/baremetal/src/Timer.cpp`.

```cpp
File: code/libraries/baremetal/src/Timer.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Timer.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Timer
9: //
10: // Description : Timer class
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
40: #include "baremetal/Timer.h"
41: 
42: #include "baremetal/ARMInstructions.h"
43: 
44: /// @file
45: /// Raspberry Pi Timer implementation
46: 
47: using namespace baremetal;
48: 
49: /// <summary>
50: /// Wait for specified number of NOP statements. Busy wait
51: /// </summary>
52: /// <param name="numCycles">Number of cycles to wait</param>
53: void Timer::WaitCycles(uint32 numCycles)
54: {
55:     if (numCycles)
56:     {
57:         while (numCycles--)
58:         {
59:             NOP();
60:         }
61:     }
62: }
```

### Update PhysicalGPIOPin code {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_DELAY_CODE___STEP_3_UPDATE_PHYSICALGPIOPIN_CODE}

We can now remove the implementation of the delay loop from `PhysicalGPIOPin.cpp`.

Update the file `code/libraries/baremetal/src/PhysicialGPIOPin.cpp`.

```cpp
File: code/libraries/baremetal/src/PhysicialGPIOPin.cpp
...
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/MemoryAccess.h"
45: #include "baremetal/Timer.h"
...
88: #if BAREMETAL_RPI_TARGET == 3
89: /// @brief Number of cycles to wait when setting pull mode for GPIO pin (Raspberry Pi 3 only)
90: static const int NumWaitCycles = 150;
91: #endif // BAREMETAL_RPI_TARGET == 3
...
301: void PhysicalGPIOPin::SetPullMode(GPIOPullMode pullMode)
302: {
303:     // Check if pin is assigned
304:     if (m_pinNumber >= NUM_GPIO)
305:         return;
306: 
307:     // Check if mode is valid
308:     if (pullMode >= GPIOPullMode::Unknown)
309:         return;
310: 
311: #if BAREMETAL_RPI_TARGET == 3
312:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (m_pinNumber / 32) * 4;
313:     uint32  shift = m_pinNumber % 32;
314: 
315:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
316:     Timer::WaitCycles(NumWaitCycles);
317:     m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
318:     Timer::WaitCycles(NumWaitCycles);
319:     m_memoryAccess.Write32(clkRegister, 0);
320: #else
321: 
...
```

- Line 45: We need to include the header file for the `Timer` class
- Line 91-100: We can remove the `WaitCycles()` function.
- Line 326, 328: We replace the call to `WaitCycles()` with a call to the `Timer` method `WaitCycles()`.

### Adding a wait before system halt or reboot {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_DELAY_CODE___STEP_3_ADDING_A_WAIT_BEFORE_SYSTEM_HALT_OR_REBOOT}

As we have now isolated the delay loop into the Timer class, we can also use it to perform the short delays when halting or rebooting the system, giving the UART1 some time to complete its output.

Update the file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
...
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/MemoryAccess.h"
45: #include "baremetal/SysConfig.h"
46: #include "baremetal/Timer.h"
47: #include "baremetal/UART1.h"
48: #include "baremetal/Util.h"
...
81: /// @brief Wait time in cycles to ensure that UART info is written before system halt or reboot/ Each cycles is a NOP instruction
82: static const uint32 NumWaitCycles = 1000000;
83: 
84: /// <summary>
85: /// Constructs a default System instance. Note that the constructor is private, so GetSystem() is needed to instantiate the System.
86: /// </summary>
87: System::System()
...
101: /// <summary>
102: /// Halts the system. This function will not return
103: /// </summary>
104: void System::Halt()
105: {
106:     GetUART1().WriteString("Halt\n");
107:     Timer::WaitCycles(NumWaitCycles);
...
124: /// <summary>
125: /// Reboots the system. This function will not return
126: /// </summary>
127: void System::Reboot()
128: {
129:     GetUART1().WriteString("Reboot\n");
130:     Timer::WaitCycles(NumWaitCycles);
...
```

- Line 46: We need to include `Timer.h`
- Line 81-82: We define a constant to indicated the number of NOP instructions to wait (we will just take a large number to be sure)
- Line 107: We wait the specified number of cycles before halting the system using the Timer method `WaitCycles()`
- Line 130: We wait the specified number of cycles before rebooting the system using the Timer method `WaitCycles()`

### Configuring, building and debugging {#TUTORIAL_08_GENERALIZATION_SEPARATING_OUT_DELAY_CODE___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The code will not behave differently, it's just cleaner in its structure.

```text
Starting up
Hello World!
Press r to reboot, h to halt
rReboot
```

As you can see, now the full text is displayed, also the strange characters are no longer shown.

## Separating standard functionality - Step 4 {#TUTORIAL_08_GENERALIZATION_SEPARATING_STANDARD_FUNCTIONALITY___STEP_4}

So far, we've been adding quite a few utility functions, that would normally be part of a standard C or C++ library.
In order to prepare for such a library, let's move this code to a new library.
We'll create a new library `stdlib` and move the following code there:
- All functions in baremetal/include/baremetal/New.h and baremetal/src/New.cpp
- All functions in baremetal/include/baremetal/Util.h and baremetal/src/Util.cpp
- All definitions in baremetal/include/baremetal/Macros.h
- All type definitions in baremetal/include/baremetal/Types.h
- All functions in baremetal/src/CXAGuard.cpp

The new library wil be next to `baremetal`, and have the following structure:

<img src="images/treeview-stdlib-library.png" alt="Initial project structure" width="300"/>

### CMake file for stdlib library {#TUTORIAL_08_GENERALIZATION_SEPARATING_STANDARD_FUNCTIONALITY___STEP_4_CMAKE_FILE_FOR_STDLIB_LIBRARY}

We'll have to set up the CMake file for the new library.

Create the file `code/libraries/stdlib/CMakeLists.txt`

```cmake
File: code/libraries/stdlib/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3: 
4: project(stdlib
5:     DESCRIPTION "Standard utility library"
6:     LANGUAGES CXX ASM)
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
22:     )
23: 
24: set(PROJECT_LIBS
25:     ${LINKER_LIBRARIES}
26:     ${PROJECT_DEPENDENCIES}
27:     )
28: 
29: file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp src/*.S)
30: set(GLOB_RECURSE PROJECT_INCLUDES_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/stdlib/*.h)
31: set(PROJECT_INCLUDES_PRIVATE )
32: 
33: set(PROJECT_INCLUDES_PRIVATE )
34: 
35: if (CMAKE_VERBOSE_MAKEFILE)
36:     display_list("Package                           : " ${PROJECT_NAME} )
37:     display_list("Package description               : " ${PROJECT_DESCRIPTION} )
38:     display_list("Defines C - public                : " ${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC} )
39:     display_list("Defines C - private               : " ${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE} )
40:     display_list("Defines C++ - public              : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC} )
41:     display_list("Defines C++ - private             : " ${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE} )
42:     display_list("Defines ASM - private             : " ${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE} )
43:     display_list("Compiler options C - public       : " ${PROJECT_COMPILE_OPTIONS_C_PUBLIC} )
44:     display_list("Compiler options C - private      : " ${PROJECT_COMPILE_OPTIONS_C_PRIVATE} )
45:     display_list("Compiler options C++ - public     : " ${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC} )
46:     display_list("Compiler options C++ - private    : " ${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE} )
47:     display_list("Compiler options ASM - private    : " ${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE} )
48:     display_list("Include dirs - public             : " ${PROJECT_INCLUDE_DIRS_PUBLIC} )
49:     display_list("Include dirs - private            : " ${PROJECT_INCLUDE_DIRS_PRIVATE} )
50:     display_list("Linker options                    : " ${PROJECT_LINK_OPTIONS} )
51:     display_list("Dependencies                      : " ${PROJECT_DEPENDENCIES} )
52:     display_list("Link libs                         : " ${PROJECT_LIBS} )
53:     display_list("Source files                      : " ${PROJECT_SOURCES} )
54:     display_list("Include files - public            : " ${PROJECT_INCLUDES_PUBLIC} )
55:     display_list("Include files - private           : " ${PROJECT_INCLUDES_PRIVATE} )
56: endif()
57: 
58: add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_INCLUDES_PUBLIC} ${PROJECT_INCLUDES_PRIVATE})
59: target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
60: target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS_PRIVATE})
61: target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_DIRS_PUBLIC})
62: target_compile_definitions(${PROJECT_NAME} PRIVATE
63:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PRIVATE}>
64:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PRIVATE}>
65:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PRIVATE}>
66:     )
67: target_compile_definitions(${PROJECT_NAME} PUBLIC
68:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_DEFINITIONS_C_PUBLIC}>
69:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_DEFINITIONS_CXX_PUBLIC}>
70:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_DEFINITIONS_ASM_PUBLIC}>
71:     )
72: target_compile_options(${PROJECT_NAME} PRIVATE
73:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PRIVATE}>
74:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PRIVATE}>
75:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PRIVATE}>
76:     )
77: target_compile_options(${PROJECT_NAME} PUBLIC
78:     $<$<COMPILE_LANGUAGE:C>:${PROJECT_COMPILE_OPTIONS_C_PUBLIC}>
79:     $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_COMPILE_OPTIONS_CXX_PUBLIC}>
80:     $<$<COMPILE_LANGUAGE:ASM>:${PROJECT_COMPILE_OPTIONS_ASM_PUBLIC}>
81:     )
82: 
83: set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${SUPPORTED_CPP_STANDARD})
84: 
85: list_to_string(PROJECT_LINK_OPTIONS PROJECT_LINK_OPTIONS_STRING)
86: if (NOT "${PROJECT_LINK_OPTIONS_STRING}" STREQUAL "")
87:     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${PROJECT_LINK_OPTIONS_STRING}")
88: endif()
89: 
90: link_directories(${LINK_DIRECTORIES})
91: set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_TARGET_NAME})
92: set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIB_DIR})
93: 
94: show_target_properties(${PROJECT_NAME})
```

### Update CMake file for baremetal library {#TUTORIAL_08_GENERALIZATION_SEPARATING_STANDARD_FUNCTIONALITY___STEP_4_UPDATE_CMAKE_FILE_FOR_BAREMETAL_LIBRARY}

We can now remove the files we moved to the new library from the `baremetal` CMake file.
Also, we need to add a dependency on the new `stdlib` library in `baremetal`.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
21: set(PROJECT_DEPENDENCIES
22:     stdlib
23:     )
24: 
25: set(PROJECT_LIBS
26:     ${LINKER_LIBRARIES}
27:     ${PROJECT_DEPENDENCIES}
28:     )
```

### Add stdlib to libraries CMake file {#TUTORIAL_08_GENERALIZATION_SEPARATING_STANDARD_FUNCTIONALITY___STEP_4_ADD_STDLIB_TO_LIBRARIES_CMAKE_FILE}

We still need to add the `stdlib` library.

Update the file `code/libraries/CMakeLists.txt`

```cmake
File: code/libraries/CMakeLists.txt
1: message(STATUS "\n**********************************************************************************\n")
2: message(STATUS "\n## In directory: ${CMAKE_CURRENT_SOURCE_DIR}")
3: 
4: add_subdirectory(baremetal)
5: add_subdirectory(stdlib)
```

### Update includes {#TUTORIAL_08_GENERALIZATION_SEPARATING_STANDARD_FUNCTIONALITY___STEP_4_UPDATE_INCLUDES}

The last step is to change all inclusions from `baremetal/` to `stdlib/` for all files moved.
I will not add the source changes for this here, as it should be trivial.

Next: [09-timer](09-timer.md)

