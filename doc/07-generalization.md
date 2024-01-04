# Improving startup and static initialization

Contents:
 - [Generalizing code](##Generalizing-code)
 - [Generic memory access - Step 1](##Generic-memory-access-Step-1)
   - [IMemoryAccess.h](###IMemoryAccess.h)
   - [MemoryAccess.h](###MemoryAccess.h)
   - [MemoryAccess.cpp](###MemoryAccess.cpp)
   - [Update UART1 code](###Update-UART1-code)
   - [Update System code](###Update-System-code)
   - [Update project configuration](###Update-project-configuration)
   - [Configuring, building and debugging](###Configuring-building-and-debugging)

## Generalizing code

So far, we've added some code for UART, GPIO, and for system startup.

You may have noticed some things:
- The code for UART1 involves configuring GPIO pins. GPIO is however functionality in itself, so it would be better to separate it out.
- Also, we will want to support UART0 next to UART1 for output, so it may be wise to create a common interface.
- Writing to registers means writing to memory. It might be practical to separate memory access out, so we can later plug in a fake for testing.

If you're curious to see how this works, or just want to dive directly into the code,
in `tutorials/07-generalization` there is a complete copy of what we work towards in this section.
Its root will clearly be `tutorial/07-generalization`.
Please be aware of this when e.g. debugging, the paths in vs.launch.json may not match your specific case.

## Generic memory access - Step 1

We'll add an abstract interface `IMemoryAccess` that is implemented by the class `MemoryAccess`.
We will pass the interface to classes that need to write to memory, such as `UART1` and `System`.

First, let's create the interface.

### IMemoryAccess.h

Let's create the abstract interface first.
Create the file `code/libraries/baremetal/include/baremetal/IMemoryAccess.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/IMemoryAccess.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
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

### MemoryAccess.h

Next we derive from the abstract `IMemoryAccess` interface.
Create the file `code/libraries/baremetal/include/baremetal/MemoryAccess.h`.

```cpp
File: code/libraries/baremetal/include/baremetal/MemoryAccess.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
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

### MemoryAccess.cpp

Next we implement `MemoryAccess`.
Create the file `code/libraries/baremetal/src/MemoryAccess.cpp`.

```cpp
File: code/libraries/baremetal/src/MemoryAccess.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2023 Rene Barto
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

### Update UART1 code

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

### Update System code

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
72:     auto r = m_memoryAccess.Read32(ARM_PWRMGT_RSTS);
73:     r &= ~0xFFFFFAAA;
74:     r |= 0x555; // partition 63 used to indicate halt
75:     m_memoryAccess.Write32(ARM_PWRMGT_RSTS, ARM_PWRMGT_WDOG_MAGIC | r);
76:     m_memoryAccess.Write32(ARM_PWRMGT_WDOG, ARM_PWRMGT_WDOG_MAGIC | 10);
77:     m_memoryAccess.Write32(ARM_PWRMGT_RSTC, ARM_PWRMGT_WDOG_MAGIC | ARM_PWRMGT_RSTC_FULLRST);
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
94:     auto r = m_memoryAccess.Read32(ARM_PWRMGT_RSTS);
95:     r &= ~0xFFFFFAAA;
96:     m_memoryAccess.Write32(ARM_PWRMGT_RSTS, ARM_PWRMGT_WDOG_MAGIC | r); // boot from partition 0
97:     m_memoryAccess.Write32(ARM_PWRMGT_WDOG, ARM_PWRMGT_WDOG_MAGIC | 10);
98:     m_memoryAccess.Write32(ARM_PWRMGT_RSTC, ARM_PWRMGT_WDOG_MAGIC | ARM_PWRMGT_RSTC_FULLRST);
99: 
100:     for (;;) // Satisfy [[noreturn]]
101:     {
102:         DataSyncBarrier();
103:         WaitForInterrupt();
104:     }
105: }
```

### Update project configuration

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
2: // Copyright   : Copyright(c) 2023 Rene Barto
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
2: // Copyright   : Copyright(c) 2023 Rene Barto
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

### Configuring, building and debugging

We can now configure and build our code.

```text
1> CMake generation started for configuration: 'BareMetal-Debug'.
1> Command line: "C:\Windows\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="D:\Projects\baremetal.github\output\install\BareMetal-Debug" -DCMAKE_TOOLCHAIN_FILE:FILEPATH="D:\Projects\baremetal.github\baremetal.toolchain" -DVERBOSE_BUILD=ON -DBAREMETAL_TARGET=RPI3 -DCMAKE_MAKE_PROGRAM="C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2019\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" "D:\Projects\baremetal.github" 2>&1"
1> Working directory: D:\Projects\baremetal.github\cmake-BareMetal-Debug
1> [CMake] -- CMake 3.20.21032501-MSVC_2
1> [CMake] -- Building for Raspberry Pi 3
1> [CMake] -- 
1> [CMake] ** Setting up project **
1> [CMake] --
1> [CMake] -- 
1> [CMake] ##################################################################################
1> [CMake] -- 
1> [CMake] ** Setting up toolchain **
1> [CMake] --
1> [CMake] -- TOOLCHAIN_ROOT           D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf
1> [CMake] -- Processor                aarch64
1> [CMake] -- Platform tuple           aarch64-none-elf
1> [CMake] -- Assembler                D:/Toolchains/arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
1> [CMake] -- C compiler               D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-gcc.exe
1> [CMake] -- C++ compiler             D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-g++.exe
1> [CMake] -- Archiver                 D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ar.exe
1> [CMake] -- Linker                   D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-ld.exe
1> [CMake] -- ObjCopy                  D:/toolchains/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/bin/aarch64-none-elf-objcopy.exe
1> [CMake] -- Std include path         D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1/include
1> [CMake] -- CMAKE_EXE_LINKER_FLAGS=   -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1
1> [CMake] -- C++ compiler version:    13.2.1
1> [CMake] -- C compiler version:      13.2.1
1> [CMake] -- C++ supported standard:  17
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/applications
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/applications/demo
1> [CMake] 
1> [CMake] ** Setting up demo **
1> [CMake] 
1> [CMake] -- Package                           :  demo
1> [CMake] -- Package description               :  Demo application
1> [CMake] -- Defines C - public                : 
1> [CMake] -- Defines C - private               : 
1> [CMake] -- Defines C++ - public              : 
1> [CMake] -- Defines C++ - private             :  PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3 _DEBUG
1> [CMake] -- Defines ASM - private             :  PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3
1> [CMake] -- Compiler options C - public       : 
1> [CMake] -- Compiler options C - private      : 
1> [CMake] -- Compiler options C++ - public     : 
1> [CMake] -- Compiler options C++ - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter
1> [CMake] -- Compiler options ASM - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2
1> [CMake] -- Include dirs - public             : 
1> [CMake] -- Include dirs - private            : 
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      :  baremetal
1> [CMake] -- Link libs                         :  baremetal
1> [CMake] -- Source files                      :  D:/Projects/baremetal.github/code/applications/demo/src/main.cpp
1> [CMake] -- Include files - public            : 
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] -- Properties for demo
1> [CMake] -- Target type                       :  EXECUTABLE
1> [CMake] -- Target defines                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3 _DEBUG> $<$<COMPILE_LANGUAGE:ASM>:PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target options                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target include dirs public        :  INCLUDES-NOTFOUND
1> [CMake] -- Target include dirs private       :  INCLUDES-NOTFOUND
1> [CMake] -- Target link libraries             :  -Wl,--start-group baremetal -Wl,--end-group
1> [CMake] -- Target link options               :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target exported defines           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported options           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported include dirs      :  INCLUDE_DIRS_EXPORTS-NOTFOUND
1> [CMake] -- Target exported link libraries    :  -Wl,--start-group baremetal -Wl,--end-group
1> [CMake] -- Target imported dependencies      : 
1> [CMake] -- Target imported link libraries    : 
1> [CMake] -- Target link dependencies          :  LINK_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target manual dependencies        :  EXPLICIT_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target static library location    :  D:/Projects/baremetal.github/output/Debug/lib
1> [CMake] -- Target dynamic library location   :  LIBRARY_LOCATION-NOTFOUND
1> [CMake] -- Target binary location            :  D:/Projects/baremetal.github/output/Debug/bin
1> [CMake] -- Target link flags                 :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target version                    :  TARGET_VERSION-NOTFOUND
1> [CMake] -- Target so-version                 :  TARGET_SOVERSION-NOTFOUND
1> [CMake] -- Target output name                :  demo.elf
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/applications/demo/create-image
1> [CMake] 
1> [CMake] ** Setting up demo-image **
1> [CMake] 
1> [CMake] -- create_image demo-image kernel8.img demo
1> [CMake] -- TARGET_NAME demo.elf
1> [CMake] -- generate D:/Projects/baremetal.github/deploy/Debug/demo-image/kernel8.img from D:/Projects/baremetal.github/output/Debug/bin/demo
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/libraries
1> [CMake] -- 
1> [CMake] **********************************************************************************
1> [CMake] 
1> [CMake] -- 
1> [CMake] ## In directory: D:/Projects/baremetal.github/code/libraries/baremetal
1> [CMake] -- Package                           :  baremetal
1> [CMake] -- Package description               :  Bare metal library
1> [CMake] -- Defines C - public                : 
1> [CMake] -- Defines C - private               : 
1> [CMake] -- Defines C++ - public              : 
1> [CMake] -- Defines C++ - private             :  PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3 _DEBUG
1> [CMake] -- Defines ASM - private             :  PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3
1> [CMake] -- Compiler options C - public       : 
1> [CMake] -- Compiler options C - private      : 
1> [CMake] -- Compiler options C++ - public     : 
1> [CMake] -- Compiler options C++ - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter
1> [CMake] -- Compiler options ASM - private    :  -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2
1> [CMake] -- Include dirs - public             :  D:/Projects/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Include dirs - private            : 
1> [CMake] -- Linker options                    :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles
1> [CMake] -- Dependencies                      : 
1> [CMake] -- Link libs                         : 
1> [CMake] -- Source files                      :  D:/Projects/baremetal.github/code/libraries/baremetal/src/CXAGuard.cpp D:/Projects/baremetal.github/code/libraries/baremetal/src/MemoryAccess.cpp D:/Projects/baremetal.github/code/libraries/baremetal/src/New.cpp D:/Projects/baremetal.github/code/libraries/baremetal/src/Startup.S D:/Projects/baremetal.github/code/libraries/baremetal/src/System.cpp D:/Projects/baremetal.github/code/libraries/baremetal/src/UART1.cpp D:/Projects/baremetal.github/code/libraries/baremetal/src/Util.cpp
1> [CMake] -- Include files - public            :  D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/ARMInstructions.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/BCMRegisters.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/iMemoryAccess.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/Macros.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/MemoryAccess.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/MemoryMap.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/New.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/SysConfig.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/System.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/Types.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/UART1.h D:/Projects/baremetal.github/code/libraries/baremetal/include/baremetal/Util.h
1> [CMake] -- Include files - private           : 
1> [CMake] -- 
1> [CMake] -- Properties for baremetal
1> [CMake] -- Target type                       :  STATIC_LIBRARY
1> [CMake] -- Target defines                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3 _DEBUG> $<$<COMPILE_LANGUAGE:ASM>:PLATFORM_BAREMETAL BAREMETAL_TARGET=RPI3> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target options                    :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2> $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target include dirs public        :  D:/Projects/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target include dirs private       :  D:/Projects/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target link libraries             :  LIBRARIES-NOTFOUND
1> [CMake] -- Target link options               :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target exported defines           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported options           :  $<$<COMPILE_LANGUAGE:C>:> $<$<COMPILE_LANGUAGE:CXX>:> $<$<COMPILE_LANGUAGE:ASM>:>
1> [CMake] -- Target exported include dirs      :  D:/Projects/baremetal.github/code/libraries/baremetal/include
1> [CMake] -- Target exported link libraries    :  LIBRARIES_EXPORTS-NOTFOUND
1> [CMake] -- Target imported dependencies      : 
1> [CMake] -- Target imported link libraries    : 
1> [CMake] -- Target link dependencies          :  LINK_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target manual dependencies        :  EXPLICIT_DEPENDENCIES-NOTFOUND
1> [CMake] -- Target static library location    :  D:/Projects/baremetal.github/output/Debug/lib
1> [CMake] -- Target dynamic library location   :  LIBRARY_LOCATION-NOTFOUND
1> [CMake] -- Target binary location            :  RUNTIME_LOCATION-NOTFOUND
1> [CMake] -- Target link flags                 :  -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles 
1> [CMake] -- Target version                    :  TARGET_VERSION-NOTFOUND
1> [CMake] -- Target so-version                 :  TARGET_SOVERSION-NOTFOUND
1> [CMake] -- Target output name                :  baremetal
1> [CMake] -- Configuring done
1> [CMake] -- Generating done
1> [CMake] -- Build files have been written to: D:/Projects/baremetal.github/cmake-BareMetal-Debug
1> Extracted CMake variables.
1> Extracted source files and headers.
1> Extracted code model.
1> Extracted toolchain configurations.
1> Extracted includes paths.
1> CMake generation finished.
```

```text
>------ Build All started: Project: 07-generalization, Configuration: BareMetal-Debug ------
  [1/11] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_TARGET=RPI3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -MF code\applications\demo\CMakeFiles\demo.dir\src\main.cpp.obj.d -o code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -c ../code/applications/demo/src/main.cpp
  [2/11] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_TARGET=RPI3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\MemoryAccess.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj -c ../code/libraries/baremetal/src/MemoryAccess.cpp
  [3/11] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_TARGET=RPI3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/CXAGuard.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\CXAGuard.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/CXAGuard.cpp.obj -c ../code/libraries/baremetal/src/CXAGuard.cpp
  [4/11] D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-gcc.exe -DBAREMETAL_TARGET=RPI3 -DPLATFORM_BAREMETAL -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -O2 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\Startup.S.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj -c ../code/libraries/baremetal/src/Startup.S
  [5/11] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_TARGET=RPI3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/New.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\New.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/New.cpp.obj -c ../code/libraries/baremetal/src/New.cpp
  [6/11] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_TARGET=RPI3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Util.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\Util.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Util.cpp.obj -c ../code/libraries/baremetal/src/Util.cpp
  [7/11] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_TARGET=RPI3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/System.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\System.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/System.cpp.obj -c ../code/libraries/baremetal/src/System.cpp
  [8/11] D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -DBAREMETAL_TARGET=RPI3 -DPLATFORM_BAREMETAL -D_DEBUG -I../code/libraries/baremetal/include -g -mcpu=cortex-a53 -mlittle-endian -mcmodel=small -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-value -Wno-aligned-new -ffreestanding -fsigned-char -nostartfiles -mno-outline-atomics -nostdinc -nostdlib -nostdinc++ -fno-exceptions -fno-rtti -O0 -Wno-unused-variable -Wno-unused-parameter -std=gnu++17 -MD -MT code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj -MF code\libraries\baremetal\CMakeFiles\baremetal.dir\src\UART1.cpp.obj.d -o code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj -c ../code/libraries/baremetal/src/UART1.cpp
  [9/11] cmd.exe /C "cd . && "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E rm -f ..\output\Debug\lib\libbaremetal.a && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ar.exe qc ..\output\Debug\lib\libbaremetal.a  code/libraries/baremetal/CMakeFiles/baremetal.dir/src/CXAGuard.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/MemoryAccess.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/New.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Startup.S.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/System.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/UART1.cpp.obj code/libraries/baremetal/CMakeFiles/baremetal.dir/src/Util.cpp.obj && D:\Toolchains\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-ranlib.exe ..\output\Debug\lib\libbaremetal.a && cd ."
  [10/11] cmd.exe /C "cd . && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-g++.exe -g -LD:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf/lib/gcc/aarch64-none-elf/13.2.1   -Wl,--section-start=.init=0x80000 -T D:/Projects/baremetal.github/baremetal.ld -nostdlib -nostartfiles code/applications/demo/CMakeFiles/demo.dir/src/main.cpp.obj -o ..\output\Debug\bin\demo.elf  -Wl,--start-group  ../output/Debug/lib/libbaremetal.a  -Wl,--end-group && cd ."
  [11/11] cmd.exe /C "cd /D D:\Projects\baremetal.github\cmake-BareMetal-Debug\code\applications\demo\create-image && D:\toolchains\arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-aarch64-none-elf\bin\aarch64-none-elf-objcopy.exe D:/Projects/baremetal.github/output/Debug/bin/demo.elf -O binary D:/Projects/baremetal.github/deploy/Debug/demo-image/kernel8.img"

Build All succeeded.
```

The code will not behave differently, but will be more well-structured.

## Separating GPIO code

