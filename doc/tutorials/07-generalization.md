# Tutorial 07: Generalization {#TUTORIAL_07_GENERALIZATION}

@tableofcontents

## Tutorial information {#TUTORIAL_07_GENERALIZATION_TUTORIAL_INFORMATION}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/07-generalization`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_07_GENERALIZATION_TUTORIAL_INFORMATION_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-07.a`
- an application `output/Debug/bin/07-generalization.elf`
- an image in `deploy/Debug/07-generalization-image`

## Generalizing code {#TUTORIAL_07_GENERALIZATION_GENERALIZING_CODE}

So far, we've added some code for UART, GPIO, and for system startup.

You may have noticed some things:
- The code for UART1 involves configuring GPIO pins. GPIO is however functionality in itself, so it would be better to separate it out.
- Also, we will want to support UART0 next to UART1 for output, so it may be wise to create a common interface.
- Writing to registers means writing to memory. It might be practical to separate memory access out, so we can later plug in a fake for testing.

If you're curious to see how this works, or just want to dive directly into the code,
in `tutorial/07-generalization` there is a complete copy of what we work towards in this section.
Its root will clearly be `tutorial/07-generalization`.
Please be aware of this when e.g. debugging, the paths in vs.launch.json may not match your specific case.

## Generic memory access - Step 1 {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1}

We'll add an abstract interface `IMemoryAccess` that is implemented by the class `MemoryAccess`.
We will pass the interface to classes that need to write to memory, such as `UART1` and `System`.

First, let's create the interface.

### IMemoryAccess.h {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_IMEMORYACCESSH}

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

### MemoryAccess.h {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_MEMORYACCESSH}

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

### MemoryAccess.cpp {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_MEMORYACCESSCPP}

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

### Update UART1 code {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_UPDATE_UART1_CODE}

#### UART1.h {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_UPDATE_UART1_CODE_UART1H}

So, we update UART1 to use IMemoryAccess calls. For this, we will also need to pass in the memory access reference to the constructor:
Update the file `code/libraries/baremetal/include/baremetal/UART1.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
...

106: class IMemoryAccess;
107: 
108: // Encapsulation for the UART1 device.
109: class UART1
110: {
111:     friend UART1& GetUART1();
112: 
113: private:
114:     bool            m_initialized;
115:     IMemoryAccess  &m_memoryAccess;
116: 
117:     // Constructs a default UART1 instance. Note that the constructor is private, so GetUART1() is needed to instantiate the UART1.
118:     UART1();
119: 
120: public:
121:     // Constructs a specialized UART1 instance with a custom IMemoryAccess instance. This is intended for testing.
122:     UART1(IMemoryAccess &memoryAccess);

...
```

- Line 106: We forward declare the class IMemoryAccess
- Line 112: We now add a reference to a IMemoryAccess instantiation
- Line 118-119: We still keep a default contructor, but make it private.
This means only our accessor function can create an default instance of UART1, making it a true singleton.
- Line 122-123: We add a method to create an instance with a specific memory access instance injected.
This can later be used for testing.

#### UART1.cpp {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_UPDATE_UART1_CODE_UART1CPP}

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
99: #if BAREMETAL_RPI_TARGET == 3
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
208: #if BAREMETAL_RPI_TARGET == 3
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

### Update System code {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_UPDATE_SYSTEM_CODE}

#### System.h {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_UPDATE_SYSTEM_CODE_SYSTEMH}

We update System to use IMemoryAccess calls. For this, we will also need to pass in the memory access reference to the constructor.
Also, we will need to make `Halt()` and `Reboot()` non-static members, as they will need to use the memory access reference.
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
55:     // Constructs a default System instance. Note that the constructor is private, so GetSystem() is needed to instantiate the System.
56:     System();
57: 
58: public:
59:     // Constructs a specialized System instance with a custom IMemoryAccess instance. This is intended for testing.
60:     System(IMemoryAccess &memoryAccess);
61: 
62:     [[noreturn]] static void Halt();
63:     [[noreturn]] static void Reboot();
...
```

- Line 46: We forward declare the class IMemoryAccess
- Line 53: We now add a reference to a IMemoryAccess instantiation
- Line 55-56: We still keep a default contructor, but make it private.
This means only our accessor function can create an default instance of System, making it a true singleton.
- Line 59-60: We add a method to create an instance with a specific memory access instance injected.
This can later be used for testing.

#### System.cpp {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_UPDATE_SYSTEM_CODE_SYSTEMCPP}

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

### Update project configuration {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_UPDATE_PROJECT_CONFIGURATION}

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
62: }
63: 
64: #ifdef __cplusplus
65: }
66: #endif
67: 
68: System& baremetal::GetSystem()
69: {
70:     static System value;
71:     return value;
72: }

...
```

The term WEAK needs to be defined still. This has to do with fallbacks in case the linker cannot find a symbol. GCC has a special attribute for this.
As you can see, `__cxa_atexit` is defined as a stub implementation.

We will put this definition in `Macros.h`.
Update the file `code/libraries/baremetal/include/baremetal/Macros.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Macros.h
...

42: // Make a variable a weak instance (GCC compiler only)
43: #define WEAK                __attribute__ ((weak))
44: 
45: // Convert bit index into integer
46: #define BIT(n)              (1U << (n))
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

### Configuring, building and debugging {#TUTORIAL_07_GENERALIZATION_GENERIC_MEMORY_ACCESS__STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code.

The code will not behave differently, but will be more well-structured.

## Separating out GPIO code - Step 2 {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_GPIO_CODE__STEP_2}

Now let's separate out the GPIO functionality. We'll create a new class PhysicalGPIOPin which will implement the functionality.

Raspberry Pi also has virtual GPIO pins, hence the name. We'll cover these later.

Again, to enable testing later on, we'll create an abstract interface IGPIOPin, which we'll derive from.

### IGPIOPin.h {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_GPIO_CODE__STEP_2_IGPIOPINH}

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
40: #pragma once
41: 
42: #include <baremetal/Types.h>
43: 
44: namespace baremetal {
45: 
46: // GPIO mode
47: enum class GPIOMode
48: {
49:     // GPIO used as input
50:     Input,
51:     // GPIO used as output
52:     Output,
53:     // GPIO used as input, using pull-up
54:     InputPullUp,
55:     // GPIO used as input, using pull-down
56:     InputPullDown,
57:     // GPIO used as Alternate Function 0.
58:     AlternateFunction0,
59:     // GPIO used as Alternate Function 1.
60:     AlternateFunction1,
61:     // GPIO used as Alternate Function 2.
62:     AlternateFunction2,
63:     // GPIO used as Alternate Function 3.
64:     AlternateFunction3,
65:     // GPIO used as Alternate Function 4.
66:     AlternateFunction4,
67:     // GPIO used as Alternate Function 5.
68:     AlternateFunction5,
69:     Unknown,
70: };
71: 
72: // GPIO function
73: enum class GPIOFunction
74: {
75:     // GPIO used as input
76:     Input,
77:     // GPIO used as output
78:     Output,
79:     // GPIO used as Alternate Function 0.
80:     AlternateFunction0,
81:     // GPIO used as Alternate Function 1.
82:     AlternateFunction1,
83:     // GPIO used as Alternate Function 2.
84:     AlternateFunction2,
85:     // GPIO used as Alternate Function 3.
86:     AlternateFunction3,
87:     // GPIO used as Alternate Function 4.
88:     AlternateFunction4,
89:     // GPIO used as Alternate Function 5.
90:     AlternateFunction5,
91:     Unknown,
92: };
93: 
94: // GPIO pull mode
95: enum class GPIOPullMode
96: {
97:     // GPIO pull mode off (no pull-up or pull-down)
98:     Off,
99:     // GPIO pull mode pull-down
100:     PullDown,
101:     // GPIO pull mode pull-up
102:     PullUp,
103:     Unknown,
104: };
105: 
106: // Abstraction of a GPIO pin
107: class IGPIOPin
108: {
109: public:
110:     virtual ~IGPIOPin() = default;
111: 
112:     // Return pin number (high bit = 0 for a phsical pin, 1 for a virtual pin)
113:     virtual uint8 GetPinNumber() const = 0;
114:     // Assign a GPIO pin
115:     virtual bool AssignPin(uint8 pinNumber) = 0;
116: 
117:     // Switch GPIO on
118:     virtual void On() = 0;
119:     // Switch GPIO off
120:     virtual void Off() = 0;
121:     // Get GPIO value
122:     virtual bool Get() = 0;
123:     // Set GPIO on (true) or off (false)
124:     virtual void Set(bool on) = 0;
125:     // Invert GPIO value on->off off->on
126:     virtual void Invert() = 0;
127: };
128: 
129: } // namespace baremetal
```

### PhysicalGPIOPin.h {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_GPIO_CODE__STEP_2_PHYSICALGPIOPINH}

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
40: #pragma once
41: 
42: #include <baremetal/IGPIOPin.h>
43: #include <baremetal/MemoryAccess.h>
44: 
45: namespace baremetal {
46: 
47: // Physical GPIO pin (i.e. available on GPIO header)
48: class PhysicalGPIOPin : public IGPIOPin
49: {
50: private:
51:     uint8                 m_pinNumber;
52:     GPIOMode              m_mode;
53:     GPIOFunction          m_function;
54:     GPIOPullMode          m_pullMode;
55:     bool                  m_value;
56:     IMemoryAccess& m_memoryAccess;
57: 
58: public:
59:     // Creates a virtual GPIO pin
60:     PhysicalGPIOPin(IMemoryAccess& memoryAccess = GetMemoryAccess());
61:     // PhysicalGPIOPin(const PhysicalGPIOPin &other);
62: 
63:     // Creates a virtual GPIO pin
64:     PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess = GetMemoryAccess());
65: 
66:     uint8 GetPinNumber() const override;
67:     // Assign a GPIO pin
68:     bool AssignPin(uint8 pinNumber) override;
69: 
70:     // Switch GPIO on
71:     void On() override;
72:     // Switch GPIO off
73:     void Off() override;
74:     // Get GPIO value
75:     bool Get() override;
76:     // Set GPIO on (true) or off (false)
77:     void Set(bool on) override;
78:     // Invert GPIO value on->off off->on
79:     void Invert() override;
80: 
81:     // Get the mode for the GPIO pin
82:     GPIOMode GetMode();
83:     // Set the mode for the GPIO pin
84:     bool SetMode(GPIOMode mode);
85:     // Get GPIO pin function
86:     GPIOFunction GetFunction();
87:     // Get GPIO pin pull mode
88:     GPIOPullMode GetPullMode();
89:     // Set GPIO pin pull mode
90:     void SetPullMode(GPIOPullMode pullMode);
91: 
92: private:
93:     // Set GPIO pin function
94:     void SetFunction(GPIOFunction function);
95: };
96: 
97: } // namespace baremetal
```

### PhysicalGPIOPin.cpp {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_GPIO_CODE__STEP_2_PHYSICALGPIOPINCPP}

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
40: #include <baremetal/PhysicalGPIOPin.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: 
46: // Total count of GPIO pins, numbered from 0 through 53
47: #define NUM_GPIO 54
48: 
49: namespace baremetal {
50: 
51: #if BAREMETAL_RPI_TARGET == 3
52: static const int NumWaitCycles = 150;
53: 
54: static void WaitCycles(uint32 numCycles)
55: {
56:     if (numCycles)
57:     {
58:         while (numCycles--)
59:         {
60:             NOP();
61:         }
62:     }
63: }
64: #endif // BAREMETAL_RPI_TARGET == 3
65: 
66: PhysicalGPIOPin::PhysicalGPIOPin(IMemoryAccess& memoryAccess /*= GetMemoryAccess()*/)
67:     : m_pinNumber{ NUM_GPIO }
68:     , m_mode{ GPIOMode::Unknown }
69:     , m_function{ GPIOFunction::Unknown }
70:     , m_pullMode{ GPIOPullMode::Unknown }
71:     , m_value{}
72:     , m_memoryAccess{ memoryAccess }
73: {
74: }
75: 
76: PhysicalGPIOPin::PhysicalGPIOPin(uint8 pinNumber, GPIOMode mode, IMemoryAccess& memoryAccess /*= m_memoryAccess*/)
77:     : m_pinNumber{ NUM_GPIO }
78:     , m_mode{ GPIOMode::Unknown }
79:     , m_value{}
80:     , m_memoryAccess{ memoryAccess }
81: {
82:     AssignPin(pinNumber);
83:     SetMode(mode);
84: }
85: 
86: uint8 PhysicalGPIOPin::GetPinNumber() const
87: {
88:     return m_pinNumber;
89: }
90: 
91: bool PhysicalGPIOPin::AssignPin(uint8 pinNumber)
92: {
93:     // Check if pin already assigned
94:     if (m_pinNumber != NUM_GPIO)
95:         return false;
96:     m_pinNumber = pinNumber;
97: 
98:     return true;
99: }
100: 
101: void PhysicalGPIOPin::On()
102: {
103:     Set(true);
104: }
105: 
106: void PhysicalGPIOPin::Off()
107: {
108:     Set(false);
109: }
110: 
111: bool PhysicalGPIOPin::Get()
112: {
113:     // Check if pin is assigned
114:     if (m_pinNumber >= NUM_GPIO)
115:         return false;
116: 
117:     if ((m_mode == GPIOMode::Input) || (m_mode == GPIOMode::InputPullUp) || (m_mode == GPIOMode::InputPullDown))
118:     {
119:         uint32 regOffset = (m_pinNumber / 32);
120:         uint32 regMask = 1 << (m_pinNumber % 32);
121:         return (m_memoryAccess.Read32(RPI_GPIO_GPLEV0 + regOffset) & regMask) ? true : false;
122:     }
123:     return m_value;
124: }
125: 
126: void PhysicalGPIOPin::Set(bool on)
127: {
128:     // Check if pin is assigned
129:     if (m_pinNumber >= NUM_GPIO)
130:         return;
131: 
132:     // Check if mode is output
133:     if (m_mode == GPIOMode::Output)
134:         return;
135: 
136:     m_value = on;
137: 
138:     uint32 regOffset = (m_pinNumber / 32);
139:     uint32 regMask = 1 << (m_pinNumber % 32);
140:     regaddr regAddress = (m_value ? RPI_GPIO_GPSET0 : RPI_GPIO_GPCLR0) + regOffset;
141: 
142:     m_memoryAccess.Write32(regAddress, regMask);
143: }
144: 
145: void PhysicalGPIOPin::Invert()
146: {
147:     Set(!Get());
148: }
149: 
150: GPIOMode PhysicalGPIOPin::GetMode()
151: {
152:     return m_mode;
153: }
154: 
155: static GPIOFunction ConvertGPIOModeToFunction(GPIOMode mode)
156: {
157:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
158:     {
159:         unsigned alternateFunctionIndex = static_cast<unsigned>(mode) - static_cast<unsigned>(GPIOMode::AlternateFunction0);
160:         return static_cast<GPIOFunction>(static_cast<unsigned>(GPIOFunction::AlternateFunction0) + alternateFunctionIndex);
161:     }
162:     else if (GPIOMode::Output == mode)
163:     {
164:         return GPIOFunction::Output;
165:     }
166:     return GPIOFunction::Input;
167: }
168: 
169: bool PhysicalGPIOPin::SetMode(GPIOMode mode)
170: {
171:     // Check if pin is assigned
172:     if (m_pinNumber >= NUM_GPIO)
173:         return false;
174: 
175:     // Check if mode is valid
176:     if (mode >= GPIOMode::Unknown)
177:         return false;
178: 
179:     if ((GPIOMode::AlternateFunction0 <= mode) && (mode <= GPIOMode::AlternateFunction5))
180:     {
181:         SetPullMode(GPIOPullMode::Off);
182: 
183:         SetFunction(ConvertGPIOModeToFunction(mode));
184:     }
185:     else if (GPIOMode::Output == mode)
186:     {
187:         SetPullMode(GPIOPullMode::Off);
188: 
189:         SetFunction(ConvertGPIOModeToFunction(mode));
190:     }
191:     else
192:     {
193:         SetPullMode((mode == GPIOMode::InputPullUp) ? GPIOPullMode::PullUp : (mode == GPIOMode::InputPullDown) ? GPIOPullMode::PullDown : GPIOPullMode::Off);
194:         SetFunction(ConvertGPIOModeToFunction(mode));
195:     }
196:     m_mode = mode;
197:     if (m_mode == GPIOMode::Output)
198:         Off();
199:     return true;
200: }
201: 
202: GPIOFunction PhysicalGPIOPin::GetFunction()
203: {
204:     return m_function;
205: }
206: 
207: GPIOPullMode PhysicalGPIOPin::GetPullMode()
208: {
209:     return m_pullMode;
210: }
211: 
212: void PhysicalGPIOPin::SetFunction(GPIOFunction function)
213: {
214:     // Check if pin is assigned
215:     if (m_pinNumber >= NUM_GPIO)
216:         return;
217: 
218:     // Check if mode is valid
219:     if (function >= GPIOFunction::Unknown)
220:         return;
221: 
222:     regaddr selectRegister = RPI_GPIO_GPFSEL0 + (m_pinNumber / 10);
223:     uint32  shift = (m_pinNumber % 10) * 3;
224: 
225:     static const unsigned FunctionMap[] = { 0, 1, 4, 5, 6, 7, 3, 2 };
226: 
227:     uint32 value = m_memoryAccess.Read32(selectRegister);
228:     value &= ~(7 << shift);
229:     value |= static_cast<uint32>(FunctionMap[static_cast<size_t>(function)]) << shift;
230:     m_memoryAccess.Write32(selectRegister, value);
231:     m_function = function;
232: }
233: 
234: void PhysicalGPIOPin::SetPullMode(GPIOPullMode pullMode)
235: {
236:     // Check if pin is assigned
237:     if (m_pinNumber >= NUM_GPIO)
238:         return;
239: 
240:     // Check if mode is valid
241:     if (pullMode >= GPIOPullMode::Unknown)
242:         return;
243: 
244: #if BAREMETAL_RPI_TARGET == 3
245:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (m_pinNumber / 32);
246:     uint32  shift = m_pinNumber % 32;
247: 
248:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
249:     WaitCycles(NumWaitCycles);
250:     m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
251:     WaitCycles(NumWaitCycles);
252:     m_memoryAccess.Write32(clkRegister, 0);
253: #else
254:     regaddr               modeReg = RPI_GPIO_GPPUPPDN0 + (m_pinNumber / 16);
255:     unsigned              shift = (m_pinNumber % 16) * 2;
256: 
257:     static const unsigned ModeMap[3] = { 0, 2, 1 };
258: 
259:     uint32                value = m_memoryAccess.Read32(modeReg);
260:     value &= ~(3 << shift);
261:     value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
262:     m_memoryAccess.Write32(modeReg, value);
263: #endif
264: 
265:     m_pullMode = pullMode;
266: }
267: 
268: } // namespace baremetal
```

### Update UART1 code {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_GPIO_CODE__STEP_2_UPDATE_UART1_CODE}

We can now clean up the code for `UART1`, and make use of the `PhysicalGPIOPin` we just implemented.

#### UART1.h {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_GPIO_CODE__STEP_2_UPDATE_UART1_CODE_UART1H}

Update the file `code/libraries/baremetal/include/baremetal/UART1.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
40: #pragma once
41: 
42: #include <baremetal/Types.h>
43: 
44: namespace baremetal {
45: 
46: class IMemoryAccess;
47: 
48: // Encapsulation for the UART1 device.
49: class UART1
50: {
51:     friend UART1& GetUART1();
52: 
53: private:
54:     bool            m_initialized;
55:     IMemoryAccess  &m_memoryAccess;
56: 
57:     // Constructs a default UART1 instance. Note that the constructor is private, so GetUART1() is needed to instantiate the UART1.
58:     UART1();
59: 
60: public:
61:     // Constructs a specialized UART1 instance with a custom IMemoryAccess instance. This is intended for testing.
62:     UART1(IMemoryAccess &memoryAccess);
63:     // Initialize the UART1 device. Only performed once, guarded by m_initialized.
64:     //  Set baud rate and characteristics (115200 8N1) and map to GPIO
65:     void Initialize();
66:     // Read a character
67:     char Read();
68:     // Write a character
69:     void Write(char c);
70:     // Write a string
71:     void WriteString(const char* str);
72: };
73: 
74: UART1& GetUART1();
75: 
76: } // namespace baremetal
```

- Line 46-106: We remove the enum definitions which are now in `IGPIOPin.h`
- Line 137-149: We remove the methods for GPIO which are noew in 'PhysicalGPIOPin.h`

#### UART1.cpp {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_GPIO_CODE__STEP_2_UPDATE_UART1_CODE_UART1CPP}

Update the file `code/libraries/baremetal/src/UART1.cpp`.

```cpp
File: code/libraries/baremetal/src/UART1.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
123:         if (*str == '\n')
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

### Update project configuration {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_GPIO_CODE__STEP_2_UPDATE_PROJECT_CONFIGURATION}

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

### Configuring, building and debugging {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_GPIO_CODE__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The code will not behave differently, it's just cleaner in its structure.

## Separating out delay code - Step 3 {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_DELAY_CODE__STEP_3}

You may have notices that we still have a delay loop in `PhysicalGPIOPin.cpp`.
This code is also generic, and can be used to perform a busy form of waiting also when rebooting or haldting the system.
It therefore seems wise to separate out this code as well.

We will create a new class Timer, which fornow will only have one method, but we will be using different forms of waiting using the system timer later on, as well as an interrupt based timer.

### Timer.h {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_DELAY_CODE__STEP_3_TIMERH}

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
40: #pragma once
41: 
42: #include <baremetal/Types.h>
43: 
44: namespace baremetal {
45:     
46: // For now holds only busy waiting
47: class Timer
48: {
49: public:
50:     // Wait for specified number of NOP statements. Busy wait
51:     static void WaitCycles(uint32 numCycles);
52: };
53: 
54: } // namespace baremetal
```

### Timer.cpp {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_DELAY_CODE__STEP_3_TIMERCPP}

Next we implement `Timer`.
Create the file `code/libraries/baremetal/src/Timer.cpp`.

```cpp
File: code/libraries/baremetal/src/Timer.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
40: #include <baremetal/Timer.h>
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

### Update PhysicalGPIOPin code {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_DELAY_CODE__STEP_3_UPDATE_PHYSICALGPIOPIN_CODE}

We can now remove the implmentation of the delay loop from `PhysicalGPIOPin.cpp`.

Update the file `code/libraries/baremetal/src/PhysicialGPIOPin.cpp`.

```cpp
File: code/libraries/baremetal/src/PhysicialGPIOPin.cpp
...
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/MemoryAccess.h>
45: #include <baremetal/Timer.h>
...
47: // Total count of GPIO pins, numbered from 0 through 53
48: #define NUM_GPIO 54
49: 
50: namespace baremetal {
51: 
52: #if BAREMETAL_RPI_TARGET == 3
53: static const int NumWaitCycles = 150;
54: #endif // BAREMETAL_RPI_TARGET == 3
...
234: #if BAREMETAL_RPI_TARGET == 3
235:     regaddr clkRegister = RPI_GPIO_GPPUDCLK0 + (m_pinNumber / 32);
236:     uint32  shift = m_pinNumber % 32;
237: 
238:     m_memoryAccess.Write32(RPI_GPIO_GPPUD, static_cast<uint32>(pullMode));
239:     Timer::WaitCycles(NumWaitCycles);
240:     m_memoryAccess.Write32(clkRegister, static_cast<uint32>(1 << shift));
241:     Timer::WaitCycles(NumWaitCycles);
242:     m_memoryAccess.Write32(clkRegister, 0);
243: #else
244:     regaddr               modeReg = RPI_GPIO_GPPUPPDN0 + (m_pinNumber / 16);
245:     unsigned              shift = (m_pinNumber % 16) * 2;
246: 
247:     static const unsigned ModeMap[3] = { 0, 2, 1 };
248: 
249:     uint32                value = m_memoryAccess.Read32(modeReg);
250:     value &= ~(3 << shift);
251:     value |= ModeMap[static_cast<size_t>(pullMode)] << shift;
252:     m_memoryAccess.Write32(modeReg, value);
253: #endif
...
```

- Line 56-66: We can remove the `WaitCycles()` function.
- Line 239, 241: We replace the call to `WaitCycles()` with a call to the `Timer` method `WaitCycles()`.

### Adding a wait before system halt or reboot {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_DELAY_CODE__STEP_3_ADDING_A_WAIT_BEFORE_SYSTEM_HALT_OR_REBOOT}

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

### Update project configuration {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_DELAY_CODE__STEP_3_UPDATE_PROJECT_CONFIGURATION}

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

### Configuring, building and debugging {#TUTORIAL_07_GENERALIZATION_SEPARATING_OUT_DELAY_CODE__STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

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

