# Tutorial 10: UART0 {#TUTORIAL_10_UART0}

@tableofcontents

## New tutorial setup {#TUTORIAL_10_UART0_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/10-uart0`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_10_UART0_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-10.a`
- an application `output/Debug/bin/10-mailbox.elf`
- an image in `deploy/Debug/10-mailbox-image`

## UART0 {#TUTORIAL_10_UART0_UART0}

Now that we have the mailbox implemented, we can start to add UART0.
The reason for this is that UART0 needs a clock frequency set, that has to be done through the mailbox.
We'll first start by defining a common interface for UART0 and UART1, so that we can use them interchangeably.
We will then add functionality for setting the UART clock.
Finally we will add and implement UART0.

### Defining a common interface - Step 1 {#TUTORIAL_10_UART0_UART0_DEFINING_A_COMMON_INTERFACE__STEP_1}

In order to be able to use any of UART0 and UART1 for e.g. logging, we need to define a common abstract interface.

### CharDevice.h {#TUTORIAL_10_UART0_UART0_CHARDEVICEH}

Create the file `code/libraries/baremetal/include/baremetal/CharDevice.h`

```cpp
File: code/libraries/baremetal/include/baremetal/CharDevice.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : CharDevice.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : CharDevice
9: //
10: // Description : Generic character read / write device interface
11: //
12: //------------------------------------------------------------------------------
13: //
14: // Baremetal - A C++ bare metal environment for embedded 64 bit ARM CharDevices
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
42: namespace baremetal {
43: 
44: // Abstract character CharDevice
45: // Abstraction of a CharDevice that can read and write characters
46: class CharDevice
47: {
48: public:
49:     virtual ~CharDevice() = default;
50: 
51:     // Read a character
52:     virtual char Read() = 0;
53:     // Write a character
54:     virtual void Write(char c) = 0;
55: };
56: 
57: } // namespace baremetal
```

- Line 46: We declare the class `CharDevice`
- Line 49: As this is an abstract interface, we need to declare a virtual destructor. The default implementation is sufficient
- Line 52: We declare a pure virtual method `Read()` like we did for UART1
- Line 54: We declare a pure virtual method `Write()` like we did for UART1

### UART1.h {#TUTORIAL_10_UART0_UART0_UART1H}

We will now derive `UART1` from our new `CharDevice` interface.
Update the file `code/libraries/baremetal/include/baremetal/UART1.h`

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
...
40: #pragma once
41: 
42: #include <baremetal/CharDevice.h>
43: 
44: namespace baremetal {
45: 
46: class IMemoryAccess;
47: 
48: // Encapsulation for the UART1 device.
49: // This is a pseudo singleton, in that it is not possible to create a default instance (GetUART1() needs to be used for this),
50: // but it is possible to create an instance with a custom IMemoryAccess instance for testing.
51: class UART1 : public CharDevice
52: {
53:     friend UART1& GetUART1();
54: 
55: private:
56:     bool            m_initialized;
57:     IMemoryAccess  &m_memoryAccess;
58: 
59:     // Constructs a default UART1 instance. Note that the constructor is private, so GetUART1() is needed to instantiate the UART1.
60:     UART1();
61: 
62: public:
63:     // Constructs a specialized UART1 instance with a custom IMemoryAccess instance. This is intended for testing.
64:     UART1(IMemoryAccess &memoryAccess);
65:     // Initialize the UART1 device. Only performed once, guarded by m_initialized.
66:     //
67:     // Set baud rate and characteristics (115200 8N1) and map to GPIO
68:     void Initialize();
69:     // Read a character
70:     char Read() override;
71:     // Write a character
72:     void Write(char c) override;
73:     // Write a string
74:     void WriteString(const char* str);
75: };
76: 
77: // Constructs the singleton UART1 instance, if needed.
78: UART1 &GetUART1();
79: 
80: } // namespace baremetal
```

- Line 42: We need to include `CharDevice.h`. This indirectly includes `Types.h`.
- Line 51: We derive from `CharDevice`
- Line 69: We declare the `Read()` method as overriding the method on the interface
- Line 72: We declare the `Write()` method as overriding the method on the interface

The code for UART1 does not need any changes.

### Update project configuration {#TUTORIAL_10_UART0_UART0_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.
Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
46: set(PROJECT_INCLUDES_PUBLIC
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
69:     )
...
```

### Configuring, building and debugging {#TUTORIAL_10_UART0_UART0_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application does not behave differently.

## Extending mailbox interface - Step 2 {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2}

In order to set the clock, we need to extend `RPIProperties`.

### RPIProperties.h {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_RPIPROPERTIESH}

We add the method `SetClockRate()` as well as the type for the clock to set.
Update the file `code/libraries/baremetal/include/baremetal/RPIProperties.h`

```cpp
File: code/libraries/baremetal/include/baremetal/RPIProperties.h
...
45: namespace baremetal {
46: 
47: enum class ClockID : uint32
48: {
49:     EMMC      = 1,
50:     UART      = 2,
51:     ARM       = 3,
52:     CORE      = 4,
53:     EMMC2     = 12,
54:     PIXEL_BVB = 14,
55: };
56: 
57: class RPIProperties
58: {
59: private:
60:     IMailbox &m_mailbox;
61: 
62: public:
63:     explicit RPIProperties(IMailbox &mailbox);
64: 
65:     bool GetBoardSerial(uint64 &serial);
66:     bool SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo);
67: };
68: 
69: } // namespace baremetal
```

### RPIProperties.cpp {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_RPIPROPERTIESCPP}

We implement the new method `SetClockRate()`. 
Update the file `code/libraries/baremetal/src/RPIProperties.cpp`

```cpp
File: code/libraries/baremetal/src/RPIProperties.cpp
...
48: struct PropertySerial
49: {
50:     Property tag;
51:     uint32   serial[2];
52: } PACKED;
53: 
54: struct PropertyClockRate
55: {
56:     Property tag;
57:     uint32   clockID;
58:     uint32   rate;      // Hz
59:     uint32   skipTurbo; // If 1, do not set turbo mode, if 0, set turbo mode if necessary
60: } PACKED;
...
82: bool RPIProperties::SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo)
83: {
84:     PropertyClockRate      tag{};
85:     RPIPropertiesInterface interface(m_mailbox);
86: 
87:     tag.clockID   = static_cast<uint32>(clockID);
88:     tag.rate      = freqHz;
89:     tag.skipTurbo = skipTurbo;
90:     auto result   = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));
91: 
92:     return result;
93: }
94: 
95: } // namespace baremetal
```

- Line 54-60: We declare a struct like we did for `GetBoardSerial()` to set the clock. This holds, next to the property header:
  - clockID: The id of the clock to be set. This is direct conversion of the `ClockID` enum
  - rate: The frequency for the clock to set, in Hz
  - skipTurbo: By default when setting an ARM frequency above default, other turbo settings will be enabled.
You can disable this effect by setting skipTurbo to 1
- Line 82-93: We implement the method `SetClockRate()`.
The implementation is comparable to that of `GetBoardSerial()`, we simple create an instance of the struct, fill its fields, and call `GetTag()` on the properties interface

### BCMRegisters.h {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_BCMREGISTERSH}

We need to add some registers of the Broadcom SoC in the Raspberry Pi for UART0 (or PL011 UART).
Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
183: //---------------------------------------------
184: // UART0 registers
185: //---------------------------------------------
186: 
187: // Raspberry Pi UART0 registers base address
188: #define RPI_UART0_BASE                RPI_BCM_IO_BASE + 0x00201000
189: // Raspberry Pi UART0 data register (R/W)
190: #define RPI_UART0_DR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000000)
191: // Raspberry Pi UART0 flag register (R/W)
192: #define RPI_UART0_FR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000018)
193: // Raspberry Pi UART0 integer baud rate divisor register (R/W)
194: #define RPI_UART0_IBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000024)
195: // Raspberry Pi UART0 factional baud rate divisor register (R/W)
196: #define RPI_UART0_FBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000028)
197: // Raspberry Pi UART0 line control register (R/W)
198: #define RPI_UART0_LCRH                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000002C)
199: // Raspberry Pi UART0 control register register (R/W)
200: #define RPI_UART0_CR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000030)
201: // Raspberry Pi UART0 interrupt FIFO level select register (R/W)
202: #define RPI_UART0_IFLS                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000034)
203: // Raspberry Pi UART0 interrupt mask set/clear register (R/W)
204: #define RPI_UART0_IMSC                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000038)
205: // Raspberry Pi UART0 raw interrupt status register (R/W)
206: #define RPI_UART0_RIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000003C)
207: // Raspberry Pi UART0 masked interrupt status  register (R/W)
208: #define RPI_UART0_MIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000040)
209: // Raspberry Pi UART0 interrupt clear register (R/W)
210: #define RPI_UART0_ICR                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000044)
211: // Raspberry Pi UART0 DMA control register (R/W)
212: #define RPI_UART0_DMACR               reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000048)
213: 
214: #define RPI_UART0_FR_RX_READY         BIT(4)
215: #define RPI_UART0_FR_TX_EMPTY         BIT(5)
216: 
...
```

We will not go into details here, we'll cover this when we use the registers.
More information on the PL011 UARTs (UART0 and others on Raspberry PI 4 and 5) registers can be found [here](#RASPBERRY_PI_UART0) as well as in the official 
[Broadcom documentation BCM2835 (Raspberry Pi 1/2)](pdf/bcm2835-peripherals.pdf) (page 175), 
[Broadcom documentation BCM2837 (Raspberry Pi 3)](pdf/bcm2837-peripherals.pdf) (page 175),  
[Broadcom documentation BCM2711 (Raspberry Pi 4)](pdf/bcm2711-peripherals.pdf) (page 144) and
[Broadcom documentation BCM2711 (Raspberry Pi 4)](pdf/rp1-peripherals.pdf) (page 34)

The Mini UART or UART1 register addresses are all prefixed with `RPI_UART0_`.

### UART0.h {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_UART0H}

We declare the class `UART0` which derives from `CharDevice`.
Create the file `code/libraries/baremetal/include/baremetal/UART0.h`

```cpp
File: code/libraries/baremetal/include/baremetal/UART0.h
File: f:\Projects\Private\baremetal.tmp\code\libraries\baremetal\include\baremetal\UART0.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : UART0.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : UART0
9: //
10: // Description : RPI UART0 class
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
42: #include <baremetal/CharDevice.h>
43: 
44: namespace baremetal {
45: 
46: class IMemoryAccess;
47: 
48: // Encapsulation for the UART0 device.
49: // This is a pseudo singleton, in that it is not possible to create a default instance (GetUART0() needs to be used for this),
50: // but it is possible to create an instance with a custom IMemoryAccess instance for testing.
51: class UART0 : public CharDevice
52: {
53:     friend UART0 &GetUART0();
54: 
55: private:
56:     bool            m_initialized;
57:     IMemoryAccess  &m_memoryAccess;
58: 
59:     // Constructs a default UART0 instance. Note that the constructor is private, so GetUART0() is needed to instantiate the UART0.
60:     UART0();
61: 
62: public:
63:     // Constructs a specialized UART0 instance with a custom IMemoryAccess instance. This is intended for testing.
64:     UART0(IMemoryAccess &memoryAccess);
65:     // Initialize the UART0 device. Only performed once, guarded by m_initialized.
66:     // Set baud rate and characteristics (115200 8N1) and map to GPIO
67:     void Initialize();
68:     // Read a character
69:     char Read() override;
70:     // Write a character
71:     void Write(char c) override;
72:     // Write a string
73:     void WriteString(const char* str);
74: };
75: 
76: // Constructs the singleton UART0 instance, if needed.
77: UART0 &GetUART0();
78: 
79: } // namespace baremetal
```

The `UART0` class declaration is identical to the `UART1` class.

### UART0.cpp {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_UART0CPP}

We implement the class `UART0`. 
Create the file `code/libraries/baremetal/src/UART0.cpp`

```cpp
File: code/libraries/baremetal/src/UART0.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : UART0.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : UART0
9: //
10: // Description : RPI UART0 class
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
40: #include <baremetal/UART0.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/Mailbox.h>
45: #include <baremetal/MemoryAccess.h>
46: #include <baremetal/PhysicalGPIOPin.h>
47: #include <baremetal/RPIProperties.h>
48: 
49: namespace baremetal {
50: 
51: UART0::UART0()
52:     : m_initialized{}
53:     , m_memoryAccess{GetMemoryAccess()}
54: {
55: }
56: 
57: UART0::UART0(IMemoryAccess &memoryAccess)
58:     : m_initialized{}
59:     , m_memoryAccess{memoryAccess}
60: {
61: }
62: 
63: // Set baud rate and characteristics (115200 8N1) and map to GPIO
64: void UART0::Initialize()
65: {
66:     if (m_initialized)
67:         return;
68:     // initialize UART
69:     m_memoryAccess.Write32(RPI_UART0_CR, 0); // turn off UART0
70: 
71:     Mailbox       mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT, m_memoryAccess);
72:     RPIProperties properties(mailbox);
73:     if (!properties.SetClockRate(ClockID::UART, 4000000, false))
74:         return;
75: 
76:     // map UART0 to GPIO pins
77:     PhysicalGPIOPin txdPin(14, GPIOMode::AlternateFunction0, m_memoryAccess);
78:     PhysicalGPIOPin rxdPin(15, GPIOMode::AlternateFunction0, m_memoryAccess);
79:     m_memoryAccess.Write32(RPI_UART0_ICR, 0x7FF); // clear interrupts
80:     m_memoryAccess.Write32(RPI_UART0_IBRD, 2);    // 115200 baud
81:     m_memoryAccess.Write32(RPI_UART0_FBRD, 0xB);
82:     m_memoryAccess.Write32(RPI_UART0_LCRH, 0x7 << 4); // 8n1, enable FIFOs
83:     m_memoryAccess.Write32(RPI_UART0_CR, 0x301);      // enable Tx, Rx, UART
84:     m_initialized = true;
85: }
86: 
87: // Write a character
88: void UART0::Write(char c)
89: {
90:     // wait until we can send
91:     // Check Tx FIFO empty
92:     while (m_memoryAccess.Read32(RPI_UART0_FR) & RPI_UART0_FR_TX_EMPTY)
93:     {
94:         NOP();
95:     }
96:     // Write the character to the buffer
97:     m_memoryAccess.Write32(RPI_UART0_DR, static_cast<unsigned int>(c));
98: }
99: 
100: // Receive a character
101: char UART0::Read()
102: {
103:     // wait until something is in the buffer
104:     // Check Rx FIFO holds data
105:     while (m_memoryAccess.Read32(RPI_UART0_FR) & RPI_UART0_FR_RX_READY)
106:     {
107:         NOP();
108:     }
109:     // Read it and return
110:     return static_cast<char>(m_memoryAccess.Read32(RPI_UART0_DR));
111: }
112: 
113: void UART0::WriteString(const char *str)
114: {
115:     while (*str)
116:     {
117:         // convert newline to carriage return + newline
118:         if (*str == '\n')
119:             Write('\r');
120:         Write(*str++);
121:     }
122: }
123: 
124: UART0 &GetUART0()
125: {
126:     static UART0 value;
127:     value.Initialize();
128:     return value;
129: }
130: 
131: } // namespace baremetal
```

The implementation is very similar to that for `UART1`

- Line 42-47: We also need to include `Mailbox.h` and `RPIProperties.h`
- Line 51-55: The default constructor is identical to the one for `UART1`
- Line 57-61: The non default constructor is identical to the one for `UART1`
- Line 64-85: The `Initialize()` method is similar to the one for `UART1`
  - Line 69: Disabling the UART uses a different register and value
  - Line 71-74: We set the clock rate for the UART clock to 4 MHz, so that we can set up the baud rate correctly
  - Line 77: As can be seen in [GPIO functions](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO), we need to set the TxD pin GPIO 14 to alternate function 0 to get the UART0 TxD signal
  - Line 78: As can be seen in [GPIO functions](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO), we need to set the RxD pin GPIO 15 to alternate function 0 to get the UART0 RxD signal
  - Line 79: Switching off interrupts uses a different register and value
  - Line 80-81: Setting the baudrate works differently. 
The `RPI_UART0_IBRD` register holds the integral part of a divisor, `RPI_UART0_FBRD` the fractional part. We calculate these part as follows

```text
divisor = UART clock rate / (16 * baudrate) = 4000000 / (16 * 115200) = 2.170184
integer part = 2
fractional part = 0.170184 * 64 = 10.89 -> 11 = 0xB
```
- Line 82: Setting the mode 8N1 enabling the FIFO uses a different register and value.
- Line 83: Enabling the UART uses a different register and value
- Line 88-98: The `Write()` method is similar to the one for `UART1`
  - Line 92: The Tx FIFO empty check uses a different register and value
  - Line 97: The Tx data buffer uses a different register and value
- Line 101-111: The `Read()` method is similar to the one for `UART1`
  - Line 105: The Rx data ready check uses a different register and value
  - Line 110: The Rx data buffer uses a different register and value
- Line 113-122: The `WriteString()` method is identical to the one for `UART1`
- Line 124-129: The `GetUART0()` method is almost identical to the one for `UART1`

### Update the application code {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_UPDATE_THE_APPLICATION_CODE}

#### main.cpp {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_UPDATE_THE_APPLICATION_CODE_MAINCPP}

Let's use UART0 now.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/BCMRegisters.h>
3: #include <baremetal/Mailbox.h>
4: #include <baremetal/MemoryManager.h>
5: #include <baremetal/RPIProperties.h>
6: #include <baremetal/SysConfig.h>
7: #include <baremetal/Serialization.h>
8: #include <baremetal/System.h>
9: #include <baremetal/Timer.h>
10: #include <baremetal/UART0.h>
11: 
12: using namespace baremetal;
13: 
14: int main()
15: {
16:     auto& uart = GetUART0();
17:     uart.WriteString("Hello World!\n");
18: 
19:     char buffer[128];
20:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
21:     RPIProperties properties(mailbox);
22: 
23:     uint64 serial;
24:     if (properties.GetBoardSerial(serial))
25:     {
26:         uart.WriteString("Mailbox call succeeded\n");
27:         uart.WriteString("Serial: ");
28:         Serialize(buffer, sizeof(buffer), serial, 8, 16, false, true);
29:         uart.WriteString(buffer);
30:         uart.WriteString("\n");
31:     }
32:     else
33:     {
34:         uart.WriteString("Mailbox call failed\n");
35:     }
36: 
37:     uart.WriteString("Wait 5 seconds\n");
38:     Timer::WaitMilliSeconds(5000);
39: 
40:     uart.WriteString("Press r to reboot, h to halt\n");
41:     char ch{};
42:     while ((ch != 'r') && (ch != 'h'))
43:     {
44:         ch = uart.Read();
45:         uart.Write(ch);
46:     }
47: 
48:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
49: }
```

#### System.cpp {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_UPDATE_THE_APPLICATION_CODE_SYSTEMCPP}

As we switch the main application to UART0, we should also switch the code in `System.cpp` to UART0, otherwise we will be suddenly changing the port over, with strange effects.
Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
47: #include <baremetal/UART0.h>
...
87: void System::Halt()
88: {
89:     GetUART0().WriteString("Halt\n");
...
107: void System::Reboot()
108: {
109:     GetUART0().WriteString("Reboot\n");
...
158:    GetUART0().WriteString("Starting up\n");
159: 
160:     extern int main();
...
```

### Update project configuration {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.
Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
45:     )
46: 
47: set(PROJECT_INCLUDES_PUBLIC
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
71:     )
72: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_10_UART0_EXTENDING_MAILBOX_INTERFACE__STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.
The code will now write to UART0, which is connected to GPIO pins 14 (Txd) and 15 (RxD) instead of UART1.
In the end, the visible behaviour does not change.

```text
Starting up
Hello World!
Mailbox call succeeded
Serial: 10000000D18E8B28
Wait 5 seconds
Press r to reboot, h to halt
```

Next: [11-doxygen](11-doxygen.md)

