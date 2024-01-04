# Improving startup and static initialization

Contents:
- [Tutorial information](##Tutorial-information)
  - [Tutorial results](###Tutorial-results)
- [Generalizing code](##Generalizing-code)
- [Generic memory access - Step 1](##Generic-memory-access-Step-1)
  - [IMemoryAccess.h - Step 1](###IMemoryAccess.h-Step-1)
  - [MemoryAccess.h - Step 1](###MemoryAccess.h-Step-1)
  - [MemoryAccess.cpp - Step 1](###MemoryAccess.cpp-Step-1)
  - [Update UART1 code - Step 1](###Update-UART1-code-Step-1)
  - [Update System code - Step 1](###Update-System-code-Step-1)
  - [Update project configuration - Step 1](###Update-project-configuration-Step-1)
  - [Configuring, building and debugging - Step 1](###Configuring-building-and-debugging-Step-1)
- [Separating out GPIO code - Step 2](##Separating-out-GPIO-code-Step-2)
  - [IGPIOPin.h - Step 2](###IGPIOPin.h-Step-2)
  - [PhysicalGPIOPin.h - Step 2](###PhysicalGPIOPin.h-Step-2)
  - [PhysicalGPIOPin.cpp - Step 2](###PhysicalGPIOPin.cpp-Step-2)
  - [Update UART1 code - Step 2](###Update-UART1-code-Step-2)
  - [Update project configuration - Step 2](###Update-project-configuration-Step-2)
  - [Configuring, building and debugging - Step 2](###Configuring-building-and-debugging-Step-2)
- [Separating out delay code - Step 3](##Separating-out-delay-code-Step-3)
  - [Timer.h - Step 3](###Timer.h-Step-3)
  - [Timer.cpp - Step 3](###Timer.cpp-Step-3)
  - [Update PhysicalGPIOPin code - Step 3](###Update-PhysicalGPIOPin-code-Step-3)
  - [Adding a wait before system halt or reboot - Step 3](###Adding-a-wait-before-system-halt-or-reboot-Step-3)
  - [Update project configuration - Step 3](###Update-project-configuration-Step-3)
  - [Configuring, building and debugging - Step 3](###Configuring-building-and-debugging-Step-3)

## Tutorial information

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/07-generalization`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-07.a`
- an application `output/Debug/bin/07-generalization.elf`
- an image in `deploy/Debug/07-generalization-image`

## Generalizing code

So far, we've added some code for UART, GPIO, and for system startup.

You may have noticed some things:
- The code for UART1 involves configuring GPIO pins. GPIO is however functionality in itself, so it would be better to separate it out.
- Also, we will want to support UART0 next to UART1 for output, so it may be wise to create a common interface.
- Writing to registers means writing to memory. It might be practical to separate memory access out, so we can later plug in a fake for testing.

If you're curious to see how this works, or just want to dive directly into the code,
in `tutorial/07-generalization` there is a complete copy of what we work towards in this section.
Its root will clearly be `tutorial/07-generalization`.
Please be aware of this when e.g. debugging, the paths in vs.launch.json may not match your specific case.

## Generic memory access - Step 1

We'll add an abstract interface `IMemoryAccess` that is implemented by the class `MemoryAccess`.
We will pass the interface to classes that need to write to memory, such as `UART1` and `System`.

First, let's create the interface.

### IMemoryAccess.h - Step 1

Let's create the abstract interface first.
Create the file `code/libraries/baremetal/include/baremetal/IMemoryAccess.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/IMemoryAccess.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
42: #include <baremetal/Types.h>
43: 
44: namespace baremetal {
45: 
46: class IMemoryAccess
47: {
48: public:
49:     virtual ~IMemoryAccess() = default;
50: 
51:     virtual uint8  Read8(regaddr address)                                                    = 0;
52:     virtual void   Write8(regaddr address, uint8 data)                                       = 0;
53:     virtual void   ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift)    = 0;
54:     virtual uint16 Read16(regaddr address)                                                   = 0;
55:     virtual void   Write16(regaddr address, uint16 data)                                     = 0;
56:     virtual void   ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift) = 0;
57:     virtual uint32 Read32(regaddr address)                                                   = 0;
58:     virtual void   Write32(regaddr address, uint32 data)                                     = 0;
59:     virtual void   ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift) = 0;
60: };
61: 
62: } // namespace baremetal
```

As you can see, we have an interface that provides three methods:
- one to read
- one to write
- one to perform a read-modify-write cycle using a mask and a shift.

The idea behind the last is that we often need to read a value, and then replace some bits, and write back.
So what this method does:
- read memory into x
- calculate x = x & ~mask (keep everything but the bits in mask)
- calculate x = x | ((data << shift) & mask) (add the data bits shifted, with mask)
- write x to memory

We have three sets of these methods:
- one for 8 bit access
- one for 16 bit access
- one for 32 bit access

### MemoryAccess.h - Step 1

Next we derive from the abstract `IMemoryAccess` interface.
Create the file `code/libraries/baremetal/include/baremetal/MemoryAccess.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryAccess.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : MemoryAccess.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccess
9: //
10: // Description : Memory read/write
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
42: #include <baremetal/IMemoryAccess.h>
43: 
44: namespace baremetal {
45: 
46: class MemoryAccess : public IMemoryAccess
47: {
48: public:
49:     uint8  Read8(regaddr address) override;
50:     void   Write8(regaddr address, uint8 data) override;
51:     void   ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift) override;
52:     uint16 Read16(regaddr address) override;
53:     void   Write16(regaddr address, uint16 data) override;
54:     void   ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift) override;
55:     uint32 Read32(regaddr address) override;
56:     void   Write32(regaddr address, uint32 data) override;
57:     void   ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift) override;
58: };
59: 
60: MemoryAccess &GetMemoryAccess();
61: 
62: } // namespace baremetal
```

So we create a class `MemoryAccess`, derived from `IMemoryAccess`. Next to this, we create an accessor function for the singleton `MemoryAccess` instance.

### MemoryAccess.cpp - Step 1

Next we implement `MemoryAccess`.
Create the file `code/libraries/baremetal/src/MemoryAccess.cpp`.

```cpp
File: code/libraries/baremetal/src/MemoryAccess.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : MemoryAccess.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MemoryAccess
9: //
10: // Description : Memory read/write
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
40: #include <baremetal/MemoryAccess.h>
41: 
42: using namespace baremetal;
43: 
44: uint8 MemoryAccess::Read8(regaddr address)
45: {
46:     return *reinterpret_cast<uint8 volatile*>(address);
47: }
48: 
49: void MemoryAccess::Write8(regaddr address, uint8 data)
50: {
51:     *reinterpret_cast<uint8 volatile*>(address) = data;
52: }
53: 
54: void MemoryAccess::ReadModifyWrite8(regaddr address, uint8 mask, uint8 data, uint8 shift)
55: {
56:     auto value = Read8(address);
57:     value &= ~mask;
58:     value |= ((data << shift) & mask);
59:     Write8(address, value);
60: }
61: 
62: uint16 MemoryAccess::Read16(regaddr address)
63: {
64:     return *reinterpret_cast<uint16 volatile*>(address);
65: }
66: 
67: void MemoryAccess::Write16(regaddr address, uint16 data)
68: {
69:     *reinterpret_cast<uint16 volatile*>(address) = data;
70: }
71: 
72: void MemoryAccess::ReadModifyWrite16(regaddr address, uint16 mask, uint16 data, uint8 shift)
73: {
74:     auto value = Read16(address);
75:     value &= ~mask;
76:     value |= ((data << shift) & mask);
77:     Write16(address, value);
78: }
79: 
80: uint32 MemoryAccess::Read32(regaddr address)
81: {
82:     return *address;
83: }
84: 
85: void MemoryAccess::Write32(regaddr address, uint32 data)
86: {
87:     *address = data;
88: }
89: 
90: void MemoryAccess::ReadModifyWrite32(regaddr address, uint32 mask, uint32 data, uint8 shift)
91: {
92:     auto value = Read32(address);
93:     value &= ~mask;
94:     value |= ((data << shift) & mask);
95:     Write32(address, value);
96: }
97: 
98: MemoryAccess &baremetal::GetMemoryAccess()
99: {
100:     static MemoryAccess value;
101:     return value;
102: }
```

As you can see, the implementation is using a reinterpration cast to a uint8 / uint16 pointer, or as before using the pointer itself for uint32.

We can now start replacing the direct memory access with calls to the methods of IMemoryAccess.

### Update UART1 code - Step 1

#### UART1.h

So, we update UART1 to use IMemoryAccess calls. For this, we will also need to pass in the memory access reference to the constructor:
Update the file `code/libraries/baremetal/include/baremetal/UART1.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
...

106: class IMemoryAccess;
107: 
108: /// @brief Encapsulation for the UART1 device.
109: ///
110: class UART1
111: {
112:     friend UART1& GetUART1();
113: 
114: private:
115:     bool            m_initialized;
116:     IMemoryAccess  &m_memoryAccess;
117: 
118:     /// @brief Constructs a default UART1 instance. Note that the constructor is private, so GetUART1() is needed to instantiate the UART1.
119:     UART1();
120: 
121: public:
122:     /// @brief Constructs a specialized UART1 instance with a custom IMemoryAccess instance. This is intended for testing.
123:     UART1(IMemoryAccess &memoryAccess);
124: 

...
```

- Line 106: We forward declare the class IMemoryAccess
- Line 112: We now add a reference to a IMemoryAccess instantiation
- Line 118-119: We still keep a default contructor, but make it private.
This means only our accessor function can create an default instance of UART1, making it a true singleton.
- Line 122-123: We add a method to create an instance with a specific memory access instance injected.
This can later be used for testing.

#### UART1.cpp

Let's update the implementation:
Update the file `code/libraries/baremetal/src/UART1.cpp`.

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...

40: #include <baremetal/UART1.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: 
46: 

...

66: UART1::UART1()
67:     : m_initialized{}
68:     , m_memoryAccess{GetMemoryAccess()}
69: {
70: }
71: 
72: UART1::UART1(IMemoryAccess &memoryAccess)
73:     : m_initialized{}
74:     , m_memoryAccess{memoryAccess}
75: {
76: }

...
```

- Line 44: We add an include statement for `MemoryAccess.h` to declare `IMemoryAccess`, as well as the `GetMemoryAccess()` getter function
- Line 66-70: The default constructor implementation initializes `m_memoryAccess` with the `MemoryAccess` singleton using `GetMemoryAccess()`
- Line 72-76: The other constructor takes initializes `m_memoryAccess` with the `IMemoryAccess` instance passed as a parameter.

We also need to update the code actually reading and writing registers:

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...

78: // Set baud rate and characteristics (115200 8N1) and map to GPIO
79: void UART1::Initialize()
80: {
81:     if (m_initialized)
82:         return;
83: 
84:     // initialize UART
85:     auto value = m_memoryAccess.Read32(RPI_AUX_ENABLES);
86:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value & ~RPI_AUX_ENABLES_UART1);    // Disable UART1, AUX mini uart
87: 
88:     SetMode(14, GPIOMode::AlternateFunction5);
89: 
90:     SetMode(15, GPIOMode::AlternateFunction5);
91: 
92:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value | RPI_AUX_ENABLES_UART1);     // enable UART1, AUX mini uart
93:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, 0);                                 // Disable Tx, Rx
94:     m_memoryAccess.Write32(RPI_AUX_MU_LCR, RPI_AUX_MU_LCR_DATA_SIZE_8);         // 8 bit mode
95:     m_memoryAccess.Write32(RPI_AUX_MU_MCR, RPI_AUX_MU_MCR_RTS_HIGH);            // RTS high
96:     m_memoryAccess.Write32(RPI_AUX_MU_IER, 0);                                  // Disable interrupts
97:     m_memoryAccess.Write32(RPI_AUX_MU_IIR, RPI_AUX_MU_IIR_TX_FIFO_ENABLE | RPI_AUX_MU_IIR_RX_FIFO_ENABLE | RPI_AUX_MU_IIR_TX_FIFO_CLEAR | RPI_AUX_MU_IIR_RX_FIFO_CLEAR);
98:                                                                                 // Clear FIFO
99: #if BAREMETAL_TARGET == RPI3
100:     m_memoryAccess.Write32(RPI_AUX_MU_BAUD, 270);                               // 250 MHz / (8 * (baud + 1)) = 250000000 / (8 * 271) =  115313 -> 115200 baud
101: #else
102:     m_memoryAccess.Write32(RPI_AUX_MU_BAUD, 541);                               // 500 MHz / (8 * (baud + 1)) = 500000000 / (8 * 542) =  115313 -> 115200 baud
103: #endif
104: 
105:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, RPI_AUX_MU_CNTL_ENABLE_RX | RPI_AUX_MU_CNTL_ENABLE_TX);
106:                                                                                 // Enable Tx, Rx
107:     m_initialized = true;
108: }
109: 
110: // Write a character
111: void UART1::Write(char c)
112: {
113:     // wait until we can send
114:     // Check Tx FIFO empty
115:     while (!(m_memoryAccess.Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LST_TX_EMPTY))
116:     {
117:         NOP();
118:     }
119:     // Write the character to the buffer
120:     m_memoryAccess.Write32(RPI_AUX_MU_IO, static_cast<unsigned int>(c));
121: }
122: 
123: // Receive a character
124: char UART1::Read()
125: {
126:     // wait until something is in the buffer
127:     // Check Rx FIFO holds data
128:     while (!(m_memoryAccess.Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LST_RX_READY))
129:     {
130:         NOP();
131:     }
132:     // Read it and return
133:     return static_cast<char>(m_memoryAccess.Read32(RPI_AUX_MU_IO));
134: }

...

182: bool UART1::SetFunction(uint8 pinNumber, GPIOFunction function)
183: {
184:     if (pinNumber >= NUM_GPIO)
185:         return false;
186:     if (function >= GPIOFunction::Unknown)
187:         return false;
188: 
189:     regaddr selectRegister = RPI_GPIO_GPFSEL0 + (pinNumber / 10);
190:     uint32  shift = (pinNumber % 10) * 3;
191: 
192:     static const unsigned FunctionMap[] = { 0, 1, 4, 5, 6, 7, 3, 2 };
193: 
194:     uint32 value = m_memoryAccess.Read32(selectRegister);
195:     value &= ~(7 << shift);
196:     value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
197:     m_memoryAccess.Write32(selectRegister, value);
198:     return true;
199: }
200: 
201: bool UART1::SetPullMode(uint8 pinNumber, GPIOPullMode pullMode)
202: {
203:     if (pullMode >= GPIOPullMode::Unknown)
204:         return false;
205: 
206:     if (pinNumber >= NUM_GPIO)
207:         return false;
208: #if BAREMETAL_TARGET == RPI3
209:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (pinNumber / 32);
210:     uint32  shift = pinNumber % 32;
211: 
212:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
213:     WaitCycles(NumWaitCycles);
214:     m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
215:     WaitCycles(NumWaitCycles);
216:     m_memoryAccess.Write32(clkRegister, 0);
217: #else
218:     regaddr               modeReg = RPI_GPIO_GPPUPPDN0 + (pinNumber / 16);
219:     unsigned              shift = (pinNumber % 16) * 2;
220: 
221:     static const unsigned ModeMap[3] = { 0, 2, 1 };
222: 
223:     uint32                value = *(modeReg);
224:     value &= ~(3 << shift);
225:     value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
226:     *(modeReg) = value;
227: #endif
228: 
229:     return true;
230: }
231: 
232: bool UART1::Off(uint8 pinNumber, GPIOMode mode)
233: {
234:     if (pinNumber >= NUM_GPIO)
235:         return false;
236: 
237:     // Output level can be set in input mode for subsequent switch to output
238:     if (mode >= GPIOMode::AlternateFunction0)
239:         return false;
240: 
241:     unsigned regOffset = (pinNumber / 32);
242:     uint32 regMask = 1 << (pinNumber % 32);
243: 
244:     bool value = false;
245: 
246:     regaddr regAddress = (value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset;
247: 
248:     m_memoryAccess.Write32(regAddress, regMask);
249: 
250:     return true;
251: }

...
```

### Update System code - Step 1

#### System.h

We update System to use IMemoryAccess calls. For this, we will also need to pass in the memory access reference to the constructor:
Update the file `code/libraries/baremetal/include/baremetal/System.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/System.h
...

46: class IMemoryAccess;
47: 
48: class System
49: {
50:     friend System& GetSystem();
51: 
52: private:
53:     IMemoryAccess  &m_memoryAccess;
54: 
55:     /// @brief Constructs a default System instance. Note that the constructor is private, so GetSystem() is needed to instantiate the System.
56:     System();
57: 
58: public:
59:     /// @brief Constructs a specialized System instance with a custom IMemoryAccess instance. This is intended for testing.
60:     System(IMemoryAccess &memoryAccess);

...
```

- Line 46: We forward declare the class IMemoryAccess
- Line 53: We now add a reference to a IMemoryAccess instantiation
- Line 55-56: We still keep a default contructor, but make it private.
This means only our accessor function can create an default instance of System, making it a true singleton.
- Line 59-60: We add a method to create an instance with a specific memory access instance injected.
This can later be used for testing.

#### System.cpp

Let's update the implementation:
Update the file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
...

42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: #include <baremetal/SysConfig.h>
46: #include <baremetal/UART1.h>
47: #include <baremetal/Util.h>

...

57: System::System()
58:     : m_memoryAccess{GetMemoryAccess()}
59: {
60: }
61: 
62: System::System(IMemoryAccess &memoryAccess)
63:     : m_memoryAccess{memoryAccess}
64: {
65: }

...
```

- Line 44: We add an include statement for `MemoryAccess.h` to declare `IMemoryAccess`, as well as the `GetMemoryAccess()` getter function
- Line 57-60: The default constructor implementation initializes `m_memoryAccess` with the `MemoryAccess` singleton using `GetMemoryAccess()`
- Line 62-65: The other constructor takes initializes `m_memoryAccess` with the `IMemoryAccess` instance passed as a parameter.

We also need to update the code actually reading and writing registers:
Update the file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
67: void System::Halt()
68: {
69:     GetUART1().WriteString("Halt\n");
70: 
71:     // power off the SoC (GPU + CPU)
72:     auto r = m_memoryAccess.Read32(RPI_PWRMGT_RSTS);
73:     r &= ~0xFFFFFAAA;
74:     r |= 0x555; // partition 63 used to indicate halt
75:     m_memoryAccess.Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r);
76:     m_memoryAccess.Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | 10);
77:     m_memoryAccess.Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_FULLRST);
78: 
79:     for (;;) // Satisfy [[noreturn]]
80:     {
81:         DataSyncBarrier();
82:         WaitForInterrupt();
83:     }
84: }
85: 
86: void System::Reboot()
87: {
88:     GetUART1().WriteString("Reboot\n");
89: 
90:     DisableIRQs();
91:     DisableFIQs();
92: 
93:     // power off the SoC (GPU + CPU)
94:     auto r = m_memoryAccess.Read32(RPI_PWRMGT_RSTS);
95:     r &= ~0xFFFFFAAA;
96:     m_memoryAccess.Write32(RPI_PWRMGT_RSTS, RPI_PWRMGT_WDOG_MAGIC | r); // boot from partition 0
97:     m_memoryAccess.Write32(RPI_PWRMGT_WDOG, RPI_PWRMGT_WDOG_MAGIC | 10);
98:     m_memoryAccess.Write32(RPI_PWRMGT_RSTC, RPI_PWRMGT_WDOG_MAGIC | RPI_PWRMGT_RSTC_FULLRST);
99: 
100:     for (;;) // Satisfy [[noreturn]]
101:     {
102:         DataSyncBarrier();
103:         WaitForInterrupt();
104:     }
105: }
```

### Update project configuration - Step 1

We need to update the CMake file for baremetal.
Update the file `code/libraries/baremetal/CMakeLists.txt`.

```cpp
File: code/libraries/baremetal/CMakeLists.txt
...

29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
36:     )
37: 
38: set(PROJECT_INCLUDES_PUBLIC
39:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/iMemoryAccess.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
50:     )
51: set(PROJECT_INCLUDES_PRIVATE )

...
```

Building now will result in linker errors:

```text
>------ Build All started: Project: 07-generalization, Configuration: BareMetal-Debug ------
  [1/4] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_TARGET=RPI3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\MemoryAccess.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj -c ../code/libraries/baremetal/src/MemoryAccess.cpp
  [2/4] cmd.exe /C "cd . && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E rm -f ..\output\Debug\lib\libbaremetal.a && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ar.exe qc ..\output\Debug\lib\libbaremetal.a  code/libraries/baremetal/CMakeFiles/baremetal.dir/src/CXAGuard.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/System.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Util.cpp.obj && D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ranlib.exe ..\output\Debug\lib\libbaremetal.a && cd ."
  [3/4] cmd.exe /C "cd . && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -o ..\output\Debug\bin\demo.elf  -Wl,--start-group  ../output/Debug/lib/libbaremetal.a  -Wl,--end-group && cd ."
  FAILED: ../output/Debug/bin/demo.elf 
  cmd.exe /C "cd . && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -o ..\output\Debug\bin\demo.elf  -Wl,--start-group  ../output/Debug/lib/libbaremetal.a  -Wl,--end-group && cd ."
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/Debug/lib/libbaremetal.a(MemoryAccess.cpp.obj): in function `baremetal::GetMemoryAccess()':
  D:\Projects\baremetal.github\cmake-BareMetal-Debug/../code/libraries/baremetal/src/MemoryAccess.cpp:100:(.text+0x324): undefined reference to `__dso_handle'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal.github\cmake-BareMetal-Debug/../code/libraries/baremetal/src/MemoryAccess.cpp:100:(.text+0x328): undefined reference to `__dso_handle'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: D:\Projects\baremetal.github\cmake-BareMetal-Debug/../code/libraries/baremetal/src/MemoryAccess.cpp:100:(.text+0x33c): undefined reference to `__cxa_atexit'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/Debug/lib/libbaremetal.a(MemoryAccess.cpp.obj): in function `baremetal::IMemoryAccess::~IMemoryAccess()':
  D:\Projects\baremetal.github\cmake-BareMetal-Debug/../code/libraries/baremetal/include/baremetal/IMemoryAccess.h:49:(.text._ZN9baremetal13IMemoryAccessD0Ev[_ZN9baremetal13IMemoryAccessD5Ev]+0x1c): undefined reference to `operator delete(void*, unsigned long)'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: ../output/Debug/lib/libbaremetal.a(MemoryAccess.cpp.obj): in function `baremetal::MemoryAccess::~MemoryAccess()':
  D:\Projects\baremetal.github\cmake-BareMetal-Debug/../code/libraries/baremetal/include/baremetal/MemoryAccess.h:46:(.text._ZN9baremetal12MemoryAccessD0Ev[_ZN9baremetal12MemoryAccessD5Ev]+0x1c): undefined reference to `operator delete(void*, unsigned long)'
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: ..\output\Debug\bin\demo.elf: hidden symbol `__dso_handle' isn't defined
  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/13.2.1/../../../../aarch64-none-elf/bin/ld.exe: final link failed: bad value
D:\Projects\baremetal.github\cmake-BareMetal-Debug\collect2.exe : error : ld returned 1 exit status
  ninja: build stopped: subcommand failed.

Build All failed.
```

A number of symbols cannot be found:
- __dso_handle
- __cxa_atexit
- operator delete(void*, unsigned long)

All of these are again caused by variables or functions we should implement.
The details are not really worthwhile discussing here, we'll get to that later when dealing with memory management.

- __dso_handle is a "guard" that is used to identify DSO (Dynamic Shared Objects) during global destruction. It is a variable of type void*.
- __cxa_atexit() is a function in C++ that registers a function to be called by exit() or when a shared library is unloaded. 
It is used to register a function to be called when the program terminates normally.
The function is called in reverse order of registration, i.e., the function registered last will be executed first.
- operator delete(void*, unsigned long) is an operator to deallocate a block of memory. It requires that another operator is defined: `operator delete(void*)`

In short, all this has to do with application shutdown (which we will not need for now), and memory management (which we will deal with later).
So for now, simple stub implementations are sufficient.

The first two will be implemented in `System.cpp`.
Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...

49: using namespace baremetal;
50: 
51: #ifdef __cplusplus
52: extern "C"
53: {
54: #endif
55: 
56: void *__dso_handle WEAK;
57: 
58: void __cxa_atexit(void* pThis, void (*func)(void* pThis), void* pHandle) WEAK;
59: 
60: void __cxa_atexit(void* /*pThis*/, void (* /*func*/)(void* pThis), void* /*pHandle*/)
61: {
62:     /// \todo Complete
63: }
64: 
65: #ifdef __cplusplus
66: }
67: #endif
68: 
69: System& baremetal::GetSystem()
70: {
71:     static System value;
72:     return value;
73: }

...
```

The term WEAK needs to be defined still. This has to do with fallbacks in case the linker cannot find a symbol. GCC has a special attribute for this.
As you can see, `__cxa_atexit` is defined as a stub implementation.

We will put this definition in `Macros.h`.
Update the file `code/libraries/baremetal/include/baremetal/Macros.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Macros.h
...

45: /// @defgroup Macros
46: /// @{
47: 
48: /// @brief Make a variable a weak instance (GCC compiler only)
49: #define WEAK                __attribute__ ((weak))
50: 
51: /// @brief Convert bit index into integer
52: /// @param n Bit index
53: #define BIT(n)              (1U << (n))
54: 
55: /// @}
56: 
```

For the delete operators, we will create a new header and source file, to start preparing for memory management.
Create the file `code/libraries/baremetal/include/baremetal/New.h`

```cpp
File: code/libraries/baremetal/include/baremetal/New.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
42: #include <baremetal/Types.h>
43: 
44: void operator delete(void* block) noexcept;
45: void operator delete(void* block, size_t size) noexcept;
```

Create the file `code/libraries/baremetal/src/New.cpp`

```cpp
File: code/libraries/baremetal/src/New.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
40: #include <baremetal/New.h>
41: 
42: void operator delete(void* /*block*/) noexcept
43: {
44: }
45: 
46: void operator delete(void* /*block*/, size_t /*size*/) noexcept
47: {
48: }
```

Again, the delete operators are for now implemented as stubs.
Update the file `code/libraries/baremetal/CMakeLists.txt`.

```cpp
File: code/libraries/baremetal/CMakeLists.txt
...

29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
37:     )
38: 
39: set(PROJECT_INCLUDES_PUBLIC
40:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/iMemoryAccess.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
52:     )
53: set(PROJECT_INCLUDES_PRIVATE )

...
```

### Configuring, building and debugging - Step 1

We can now configure and build our code.

The code will not behave differently, but will be more well-structured.

## Separating out GPIO code - Step 2

Now let's separate out the GPIO functionality. We'll create a new class PhysicalGPIOPin which will implement the functionality.

Raspberry Pi also has virtual GPIO pins, hence the name. We'll cover these later.

Again, to enable testing later on, we'll create an abstract interface IGPIOPin, which we'll derive from.

### IGPIOPin.h - Step 2

Create the file `code/libraries/baremetal/include/baremetal/IGPIOPin.h`

```cpp
File: code/libraries/baremetal/include/baremetal/IGPIOPin.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
42: #include <baremetal/Types.h>
43: 
44: namespace baremetal {
45: 
46: /// @brief GPIO mode
47: enum class GPIOMode
48: {
49:     /// @brief GPIO used as input
50:     Input,
51:     /// @brief GPIO used as output
52:     Output,
53:     /// @brief GPIO used as input, using pull-up
54:     InputPullUp,
55:     /// @brief GPIO used as input, using pull-down
56:     InputPullDown,
57:     /// @brief GPIO used as Alternate Function 0.
58:     AlternateFunction0,
59:     /// @brief GPIO used as Alternate Function 1.
60:     AlternateFunction1,
61:     /// @brief GPIO used as Alternate Function 2.
62:     AlternateFunction2,
63:     /// @brief GPIO used as Alternate Function 3.
64:     AlternateFunction3,
65:     /// @brief GPIO used as Alternate Function 4.
66:     AlternateFunction4,
67:     /// @brief GPIO used as Alternate Function 5.
68:     AlternateFunction5,
69:     Unknown,
70: };
71: 
72: /// @brief GPIO function
73: enum class GPIOFunction
74: {
75:     /// @brief GPIO used as input
76:     Input,
77:     /// @brief GPIO used as output
78:     Output,
79:     /// @brief GPIO used as Alternate Function 0.
80:     AlternateFunction0,
81:     /// @brief GPIO used as Alternate Function 1.
82:     AlternateFunction1,
83:     /// @brief GPIO used as Alternate Function 2.
84:     AlternateFunction2,
85:     /// @brief GPIO used as Alternate Function 3.
86:     AlternateFunction3,
87:     /// @brief GPIO used as Alternate Function 4.
88:     AlternateFunction4,
89:     /// @brief GPIO used as Alternate Function 5.
90:     AlternateFunction5,
91:     Unknown,
92: };
93: 
94: /// @brief GPIO pull mode
95: enum class GPIOPullMode
96: {
97:     /// @brief GPIO pull mode off (no pull-up or pull-down)
98:     Off,
99:     /// @brief GPIO pull mode pull-down
100:     PullDown,
101:     /// @brief GPIO pull mode pull-up
102:     PullUp,
103:     Unknown,
104: };
105: 
106: /// @brief Abstraction of a GPIO pin
107: class IGPIOPin
108: {
109: public:
110:     virtual ~IGPIOPin() = default;
111: 
112:     /// @brief Return pin number (high bit = 0 for a phsical pin, 1 for a virtual pin)
113:     /// @return Pin number
114:     virtual uint8 GetPinNumber() const = 0;
115:     /// @brief Assign a GPIO pin
116:     /// @param pin      Pin number
117:     /// @return true if successful, false otherwise
118:     virtual bool AssignPin(uint8 pinNumber) = 0;
119: 
120:     /// @brief Switch GPIO on
121:     virtual void On() = 0;
122:     /// @brief Switch GPIO off
123:     virtual void Off() = 0;
124:     /// @brief Get GPIO value
125:     virtual bool Get() = 0;
126:     /// @brief Set GPIO on (true) or off (false)
127:     virtual void Set(bool on) = 0;
128:     /// @brief Invert GPIO value on->off off->on
129:     virtual void Invert() = 0;
130: };
131: 
132: } // namespace baremetal
```

### PhysicalGPIOPin.h - Step 2

Create the file `code/libraries/baremetal/include/baremetal/IGPIOPin.h`

```cpp
File: code/libraries/baremetal/include/baremetal/PhysicalGPIOPin.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
42: #include <baremetal/IGPIOPin.h>
43: #include <baremetal/MemoryAccess.h>
44: 
45: /// @file
46: /// Physical GPIO pin
47: 
48: namespace baremetal {
49: 
50: /// @brief Physical GPIO pin (i.e. available on GPIO header)
51: class PhysicalGPIOPin : public IGPIOPin
52: {
53: private:
54:     uint8                 m_pinNumber;
55:     GPIOMode              m_mode;
56:     GPIOFunction          m_function;
57:     GPIOPullMode          m_pullMode;
58:     bool                  m_value;
59:     IMemoryAccess& m_memoryAccess;
60: 
61: public:
62:     /// @brief Creates a virtual GPIO pin
63:     PhysicalGPIOPin(IMemoryAccess& memoryAccess = GetMemoryAccess());
64:     // PhysicalGPIOPin(const PhysicalGPIOPin &other);
65: 
66:     /// @brief Creates a virtual GPIO pin
67:     PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess = GetMemoryAccess());
68: 
69:     uint8 GetPinNumber() const override;
70:     /// @brief Assign a GPIO pin
71:     /// @param pin      Pin number
72:     /// @return true if successful, false otherwise
73:     bool AssignPin(uint8 pinNumber) override;
74: 
75:     /// @brief Switch GPIO on
76:     void On() override;
77:     /// @brief Switch GPIO off
78:     void Off() override;
79:     /// @brief Get GPIO value
80:     bool Get() override;
81:     /// @brief Set GPIO on (true) or off (false)
82:     void Set(bool on) override;
83:     /// @brief Invert GPIO value on->off off->on
84:     void Invert() override;
85: 
86:     /// @brief Get the mode for the GPIO pin
87:     /// @return mode GPIO mode. See \ref GPIOMode
88:     GPIOMode GetMode();
89:     /// @brief Set the mode for the GPIO pin
90:     /// @param mode GPIO mode to be selected. See \ref GPIOMode
91:     /// @return true if successful, false otherwise
92:     bool SetMode(GPIOMode mode);
93:     /// @brief Get GPIO pin function
94:     /// @return GPIO pin function used. See \ref GPIOFunction
95:     GPIOFunction GetFunction();
96:     /// @brief Get GPIO pin pull mode
97:     /// @return GPIO pull mode used. See \ref GPIOPullMode
98:     GPIOPullMode GetPullMode();
99:     /// @brief Set GPIO pin pull mode
100:     /// @param pullMode GPIO pull mode to be used. See \ref GPIOPullMode
101:     void SetPullMode(GPIOPullMode pullMode);
102: 
103: private:
104:     /// @brief Set GPIO pin function
105:     /// @param function GPIO function to be selected. See \ref GPIOFunction, \ref BCM_GPIO_ALTERNATIVE_FUNCTIONS
106:     void SetFunction(GPIOFunction function);
107: };
108: 
109: } // namespace baremetal
```

### PhysicalGPIOPin.cpp - Step 2

Create the file `code/libraries/baremetal/include/baremetal/IGPIOPin.h`

```cpp
File: code/libraries/baremetal/src.PhysicalGPIOPin.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
40: #include <baremetal/PhysicalGPIOPin.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: 
46: /// @file
47: /// Physical GPIO pin implementation
48: 
49: /// @brief Total count of GPIO pins, numbered from 0 through 53
50: #define NUM_GPIO 54
51: 
52: namespace baremetal {
53: 
54: #if BAREMETAL_RPI_TARGET == 3
55: static const int NumWaitCycles = 150;
56: 
57: static void WaitCycles(uint32 numCycles)
58: {
59:     if (numCycles)
60:     {
61:         while (numCycles--)
62:         {
63:             NOP();
64:         }
65:     }
66: }
67: #endif // BAREMETAL_RPI_TARGET == 3
68: 
69: PhysicalGPIOPin::PhysicalGPIOPin(IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
70:     : m_pinNumber{ NUM_GPIO }
71:     , m_mode{ GPIOMode::Unknown }
72:     , m_function{ GPIOFunction::Unknown }
73:     , m_pullMode{ GPIOPullMode::Unknown }
74:     , m_value{}
75:     , m_memoryAccess{ memoryAccess }
76: {
77: }
78: 
79: PhysicalGPIOPin::PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess /*= m_memoryAccess*/)
80:     : m_pinNumber{ NUM_GPIO }
81:     , m_mode{ GPIOMode::Unknown }
82:     , m_value{}
83:     , m_memoryAccess{ memoryAccess }
84: {
85:     AssignPin(pinNumber);
86:     SetMode(mode);
87: }
88: 
89: uint8 PhysicalGPIOPin::GetPinNumber() const
90: {
91:     return m_pinNumber;
92: }
93: 
94: bool PhysicalGPIOPin::AssignPin(uint8 pinNumber)
95: {
96:     // Check if pin already assigned
97:     if (m_pinNumber != NUM_GPIO)
98:         return false;
99:     m_pinNumber = pinNumber;
100: 
101:     return true;
102: }
103: 
104: void PhysicalGPIOPin::On()
105: {
106:     Set(true);
107: }
108: 
109: void PhysicalGPIOPin::Off()
110: {
111:     Set(false);
112: }
113: 
114: bool PhysicalGPIOPin::Get()
115: {
116:     // Check if pin is assigned
117:     if (m_pinNumber >= NUM_GPIO)
118:         return false;
119: 
120:     if ((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown))
121:     {
122:         uint32 regOffset = (m_pinNumber / 32);
123:         uint32 regMask = 1 << (m_pinNumber % 32);
124:         return (m_memoryAccess.Read32(RPI_GPIO_GPLEV0 + regOffset) & regMask) ? true : false;
125:     }
126:     return m_value;
127: }
128: 
129: void PhysicalGPIOPin::Set(bool on)
130: {
131:     // Check if pin is assigned
132:     if (m_pinNumber >= NUM_GPIO)
133:         return;
134: 
135:     // Check if mode is output
136:     if (m_mode == GPIOMode::Output)
137:         return;
138: 
139:     m_value = on;
140: 
141:     uint32 regOffset = (m_pinNumber / 32);
142:     uint32 regMask = 1 << (m_pinNumber % 32);
143:     regaddr regAddress = (m_value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset;
144: 
145:     m_memoryAccess.Write32(regAddress, regMask);
146: }
147: 
148: void PhysicalGPIOPin::Invert()
149: {
150:     Set(!Get());
151: }
152: 
153: GPIOMode PhysicalGPIOPin::GetMode()
154: {
155:     return m_mode;
156: }
157: 
158: static GPIOFunction ConvertGPIOModeToFunction(GPIOMode mode)
159: {
160:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
161:     {
162:         unsigned alternateFunctionIndex = static_cast<unsigned>(mode) - static_cast<unsigned>(GPIOMode::AlternateFunction0);
163:         return static_cast<GPIOFunction>(static_cast<unsigned>(GPIOFunction::AlternateFunction0) + alternateFunctionIndex);
164:     }
165:     else if (GPIOMode::Output == mode)
166:     {
167:         return GPIOFunction::Output;
168:     }
169:     return GPIOFunction::Input;
170: }
171: 
172: bool PhysicalGPIOPin::SetMode(GPIOMode mode)
173: {
174:     // Check if pin is assigned
175:     if (m_pinNumber >= NUM_GPIO)
176:         return false;
177: 
178:     // Check if mode is valid
179:     if (mode >= GPIOMode::Unknown)
180:         return false;
181: 
182:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
183:     {
184:         SetPullMode(GPIOPullMode::Off);
185: 
186:         SetFunction(ConvertGPIOModeToFunction(mode));
187:     }
188:     else if (GPIOMode::Output == mode)
189:     {
190:         SetPullMode(GPIOPullMode::Off);
191: 
192:         SetFunction(ConvertGPIOModeToFunction(mode));
193:     }
194:     else
195:     {
196:         SetPullMode((mode == GPIOMode::InputPullUp) ? GPIOPullMode::PullUp : (mode == GPIOMode::InputPullDown) ? GPIOPullMode::PullDown : GPIOPullMode::Off);
197:         SetFunction(ConvertGPIOModeToFunction(mode));
198:     }
199:     m_mode = mode;
200:     if (m_mode == GPIOMode::Output)
201:         Off();
202:     return true;
203: }
204: 
205: GPIOFunction PhysicalGPIOPin::GetFunction()
206: {
207:     return m_function;
208: }
209: 
210: GPIOPullMode PhysicalGPIOPin::GetPullMode()
211: {
212:     return m_pullMode;
213: }
214: 
215: void PhysicalGPIOPin::SetFunction(GPIOFunction function)
216: {
217:     // Check if pin is assigned
218:     if (m_pinNumber >= NUM_GPIO)
219:         return;
220: 
221:     // Check if mode is valid
222:     if (function >= GPIOFunction::Unknown)
223:         return;
224: 
225:     regaddr selectRegister = RPI_GPIO_GPFSEL0 + (m_pinNumber / 10);
226:     uint32  shift = (m_pinNumber % 10) * 3;
227: 
228:     static const unsigned FunctionMap[] = { 0, 1, 4, 5, 6, 7, 3, 2 };
229: 
230:     uint32 value = m_memoryAccess.Read32(selectRegister);
231:     value &= ~(7 << shift);
232:     value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
233:     m_memoryAccess.Write32(selectRegister, value);
234:     m_function = function;
235: }
236: 
237: void PhysicalGPIOPin::SetPullMode(GPIOPullMode pullMode)
238: {
239:     // Check if pin is assigned
240:     if (m_pinNumber >= NUM_GPIO)
241:         return;
242: 
243:     // Check if mode is valid
244:     if (pullMode >= GPIOPullMode::Unknown)
245:         return;
246: 
247: #if BAREMETAL_RPI_TARGET == 3
248:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (m_pinNumber / 32);
249:     uint32  shift = m_pinNumber % 32;
250: 
251:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
252:     WaitCycles(NumWaitCycles);
253:     m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
254:     WaitCycles(NumWaitCycles);
255:     m_memoryAccess.Write32(clkRegister, 0);
256: #else
257:     regaddr               modeReg = RPI_GPIO_GPPUPPDN0 + (m_pinNumber / 16);
258:     unsigned              shift = (m_pinNumber % 16) * 2;
259: 
260:     static const unsigned ModeMap[3] = { 0, 2, 1 };
261: 
262:     uint32                value = m_memoryAccess.Read32(modeReg);
263:     value &= ~(3 << shift);
264:     value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
265:     m_memoryAccess.Write32(modeReg, value);
266: #endif
267: 
268:     m_pullMode = pullMode;
269: }
270: 
271: } // namespace baremetal
```

### Update UART1 code - Step 2

We can now clean up the code for `UART1`, and make use of the `PhysicalGPIOPin` we just implemented.

#### UART1.h

Update the file `code/libraries/baremetal/include/baremetal/UART1.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : UART1.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : UART1
9: //
10: // Description : RPI UART1 class
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
42: #include <baremetal/Types.h>
43: 
44: namespace baremetal {
45: 
46: class IMemoryAccess;
47: 
48: /// @brief Encapsulation for the UART1 device.
49: ///
50: class UART1
51: {
52:     friend UART1& GetUART1();
53: 
54: private:
55:     bool            m_initialized;
56:     IMemoryAccess  &m_memoryAccess;
57: 
58:     /// @brief Constructs a default UART1 instance. Note that the constructor is private, so GetUART1() is needed to instantiate the UART1.
59:     UART1();
60: 
61: public:
62:     /// @brief Constructs a specialized UART1 instance with a custom IMemoryAccess instance. This is intended for testing.
63:     UART1(IMemoryAccess &memoryAccess);
64:     /// @brief Initialize the UART1 device. Only performed once, guarded by m_initialized.
65:     ///
66:     ///  Set baud rate and characteristics (115200 8N1) and map to GPIO
67:     void Initialize();
68:     /// @brief Read a character
69:     /// @return Character read
70:     char Read();
71:     /// @brief Write a character
72:     /// @param c Character to be written
73:     void Write(char c);
74:     /// @brief Write a string
75:     /// @param str String to be written
76:     void WriteString(const char* str);
77: };
78: 
79: UART1& GetUART1();
80: 
81: } // namespace baremetal
82: 
```

- Line 46-106: We remove the enum definitions which are now in `IGPIOPin.h`
- Line 137-149: We remove the methods for GPIO which are noew in 'PhysicalGPIOPin.h`

#### UART1.cpp

Update the file `code/libraries/baremetal/src/UART1.cpp`.

```cpp
File: code/libraries/baremetal/src/UART1.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
3: //
4: // File        : UART1.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : UART1
9: //
10: // Description : RPI UART1 class
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
40: #include <baremetal/UART1.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: #include <baremetal/PhysicalGPIOPin.h>
46: 
47: namespace baremetal {
48: 
49: UART1::UART1()
50:     : m_initialized{}
51:     , m_memoryAccess{GetMemoryAccess()}
52: {
53: }
54: 
55: UART1::UART1(IMemoryAccess &memoryAccess)
56:     : m_initialized{}
57:     , m_memoryAccess{memoryAccess}
58: {
59: }
60: 
61: // Set baud rate and characteristics (115200 8N1) and map to GPIO
62: void UART1::Initialize()
63: {
64:     if (m_initialized)
65:         return;
66: 
67:     // initialize UART
68:     auto value = m_memoryAccess.Read32(RPI_AUX_ENABLES);
69:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value & ~RPI_AUX_ENABLES_UART1);    // Disable UART1, AUX mini uart
70: 
71:     PhysicalGPIOPin rxdPin(15, GPIOMode::AlternateFunction5, m_memoryAccess);
72:     PhysicalGPIOPin txdPin(14, GPIOMode::AlternateFunction5, m_memoryAccess);
73: 
74:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value | RPI_AUX_ENABLES_UART1);     // enable UART1, AUX mini uart
75:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, 0);                                 // Disable Tx, Rx
76:     m_memoryAccess.Write32(RPI_AUX_MU_LCR, RPI_AUX_MU_LCR_DATA_SIZE_8);         // 8 bit mode
77:     m_memoryAccess.Write32(RPI_AUX_MU_MCR, RPI_AUX_MU_MCR_RTS_HIGH);            // RTS high
78:     m_memoryAccess.Write32(RPI_AUX_MU_IER, 0);                                  // Disable interrupts
79:     m_memoryAccess.Write32(RPI_AUX_MU_IIR, RPI_AUX_MU_IIR_TX_FIFO_ENABLE | RPI_AUX_MU_IIR_RX_FIFO_ENABLE | RPI_AUX_MU_IIR_TX_FIFO_CLEAR | RPI_AUX_MU_IIR_RX_FIFO_CLEAR);
80:                                                                                 // Clear FIFO
81: #if BAREMETAL_RPI_TARGET == 3
82:     m_memoryAccess.Write32(RPI_AUX_MU_BAUD, 270);                               // 250 MHz / (8 * (baud + 1)) = 250000000 / (8 * 271) =  115313 -> 115200 baud
83: #else
84:     m_memoryAccess.Write32(RPI_AUX_MU_BAUD, 541);                               // 500 MHz / (8 * (baud + 1)) = 500000000 / (8 * 542) =  115313 -> 115200 baud
85: #endif
86: 
87:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, RPI_AUX_MU_CNTL_ENABLE_RX | RPI_AUX_MU_CNTL_ENABLE_TX);
88:                                                                                 // Enable Tx, Rx
89:     m_initialized = true;
90: }
91: 
92: // Write a character
93: void UART1::Write(char c)
94: {
95:     // wait until we can send
96:     // Check Tx FIFO empty
97:     while (!(m_memoryAccess.Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LST_TX_EMPTY))
98:     {
99:         NOP();
100:     }
101:     // Write the character to the buffer
102:     m_memoryAccess.Write32(RPI_AUX_MU_IO, static_cast<unsigned int>(c));
103: }
104: 
105: // Receive a character
106: char UART1::Read()
107: {
108:     // wait until something is in the buffer
109:     // Check Rx FIFO holds data
110:     while (!(m_memoryAccess.Read32(RPI_AUX_MU_LSR) & RPI_AUX_MU_LST_RX_READY))
111:     {
112:         NOP();
113:     }
114:     // Read it and return
115:     return static_cast<char>(m_memoryAccess.Read32(RPI_AUX_MU_IO));
116: }
117: 
118: void UART1::WriteString(const char* str)
119: {
120:     while (*str)
121:     {
122:         // convert newline to carriage return + newline
123:         if (*str == '\n')Line 88
124:             Write('\r');
125:         Write(*str++);
126:     }
127: }
128: 
129: UART1& GetUART1()
130: {
131:     static UART1 value;
132:     value.Initialize();
133: 
134:     return value;
135: }
136: 
137: } // namespace baremetal
```

- Line 45: We need to include `PhysicalGPIOPin.h`
- Line 46-48: We can remove the `NUM_GPIO` definition
- Line 51-66: We can remove the implementation for `WaitCycles()`
- Line 88: We now instantiate a local PhysicialGPIOPin instance (we will no longer need it once the GPIO pin is configured) to set up the TxD pin (14)
- Line 90: We now instantiate a local PhysicialGPIOPin instance (we will no longer need it once the GPIO pin is configured) to set up the RxD pin (15)
- Line 147-253: We can remove the implementation of methods now moved to `PhysicalGPIOPin.cpp`

### Update project configuration - Step 2

As we added some files to the baremetal project, we need to update its CMake file.
Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
38:     )
39: 
40: set(PROJECT_INCLUDES_PUBLIC
41:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
55:     )
56: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging - Step 2

We can now configure and build our code, and start debugging.

The code will not behave differently, it's just cleaner in its structure.

## Separating out delay code - Step 3

You may have notices that we still have a delay loop in `PhysicalGPIOPin.cpp`.
This code is also generic, and can be used to perform a busy form of waiting also when rebooting or haldting the system.
It therefore seems wise to separate out this code as well.

We will create a new class Timer, which fornow will only have one method, but we will be using different forms of waiting using the system timer later on, as well as an interrupt based timer.

### Timer.h - Step 3

Let's create the `Timer` class.
Create the file `code/libraries/baremetal/include/baremetal/Timer.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
42: #include <baremetal/Types.h>
43: 
44: namespace baremetal {
45:     
46: /// <summary>
47: /// For now holds only busy waiting
48: /// </summary>
49: class Timer
50: {
51: public:
52:     /// @brief Wait for specified number of NOP statements. Busy wait
53:     /// @param numCycles    Wait time in cycles
54:     static void WaitCycles(uint32 numCycles);
55: };
56: 
57: } // namespace baremetal
```

### Timer.cpp - Step 3

Next we implement `Timer`.
Create the file `code/libraries/baremetal/src/Timer.cpp`.

```cpp
File: code/libraries/baremetal/src/Timer.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
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
40: #include "baremetal/Timer.h"
41: 
42: #include <baremetal/ARMInstructions.h>
43: 
44: using namespace baremetal;
45: 
46: void Timer::WaitCycles(uint32 numCycles)
47: {
48:     if (numCycles)
49:     {
50:         while (numCycles--)
51:         {
52:             NOP();
53:         }
54:     }
55: }
```

### Update PhysicalGPIOPin code - Step 3

We can now remove the implmentation of the delay loop from `PhysicalGPIOPin.cpp`.

Update the file `code/libraries/baremetal/src/PhysicialGPIOPin.cpp`.

```cpp
File: code/libraries/baremetal/src/PhysicialGPIOPin.cpp
...
49: /// @brief Total count of GPIO pins, numbered from 0 through 53
50: #define NUM_GPIO 54
51: 
52: namespace baremetal {
53: 
54: #if BAREMETAL_RPI_TARGET == 3
55:     static const int NumWaitCycles = 150;
56: #endif // BAREMETAL_RPI_TARGET == 3
57: 
...
```

- Line 56-66: We can remove the `WaitCycles()` function.

### Adding a wait before system halt or reboot

As we have now isolated the delay loop into the Timer class, we can also use it to implement short delays when halting or rebooting the system, giving the UART1 some time to complete its output.

Update the file `code/libraries/baremetal/src/System.cpp`.

```cpp
File: code/libraries/baremetal/src/System.cpp
...
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: #include <baremetal/SysConfig.h>
46: #include <baremetal/Timer.h>
47: #include <baremetal/UART1.h>
48: #include <baremetal/Util.h>
...
70: static const uint32 NumWaitCycles = 100000000;
71: 
72: System& baremetal::GetSystem()
73: {
74:     static System value;
75:     return value;
76: }
...
88: void System::Halt()
89: {
90:     GetUART1().WriteString("Halt\n");
91:     Timer::WaitCycles(NumWaitCycles);
...
108: void System::Reboot()
109: {
110:     GetUART1().WriteString("Reboot\n");
111:     Timer::WaitCycles(NumWaitCycles);
...
```

- Line 46: We need to include `Timer.h`
- Line 70: We defined a constant to indicated the number of NOP instructions to wait
- Line 91: We wait the specified number of cycles before halting the system
- Line 111: We wait the specified number of cycles before rebooting the system

### Update project configuration - Step 3

As we added some files to the baremetal project, we need to update its CMake file.
Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
39:     )
40: 
41: set(PROJECT_INCLUDES_PUBLIC
42:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
43:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
44:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
45:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
57:     )
58: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging - Step 3

We can now configure and build our code, and start debugging.

The code will not behave differently, it's just cleaner in its structure.
However, when running on a physical board, we will see a difference:

```text
Starting up
Hello World!
Press r to reboot, h to halt
rReboot
```

As you can see, now the full text is displayed, also the strange characters are no longer shown.

Next: [08-timer](08-timer.md)

