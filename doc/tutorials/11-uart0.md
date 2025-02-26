# Tutorial 11: UART0 {#TUTORIAL_11_UART0}

@tableofcontents

## New tutorial setup {#TUTORIAL_11_UART0_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/11-uart0`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_11_UART0_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-11.a`
- a library `output/Debug/lib/stdlib-11.a`
- an application `output/Debug/bin/11-uart0.elf`
- an image in `deploy/Debug/11-uart0-image`

## UART0 {#TUTORIAL_11_UART0_UART0}

Now that we have the mailbox implemented, we can start to add UART0.
The reason for this is that UART0 needs a clock frequency set, which has to be done through the mailbox.
We'll first start by defining a common interface for UART0 and UART1, so that we can use them interchangeably.
We will then add functionality for setting the UART clock.
Finally we will add and implement UART0.

## Defining a common interface - Step 1 {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1}

In order to be able to use any of UART0 and UART1 for e.g. logging, we need to define a common abstract interface.

### CharDevice.h {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_CHARDEVICEH}

Create the file `code/libraries/baremetal/include/baremetal/CharDevice.h`

```cpp
File: code/libraries/baremetal/include/baremetal/CharDevice.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
43: /// Abstract character device
44: 
45: namespace baremetal {
46: 
47:     /// <summary>
48:     /// Abstract character CharDevice
49:     ///
50:     /// Abstraction of a CharDevice that can read and write characters
51:     /// </summary>
52:     class CharDevice
53:     {
54:     public:
55:         virtual ~CharDevice() = default;
56: 
57:         /// <summary>
58:         /// Read a character
59:         /// </summary>
60:         /// <returns>Character read</returns>
61:         virtual char Read() = 0;
62:         /// <summary>
63:         /// Write a character
64:         /// </summary>
65:         /// <param name="c">Character to be written</param>
66:         virtual void Write(char c) = 0;
67:     };
68: 
69:     } // namespace baremetal
70: 
```

- Line 52: We declare the class `CharDevice`
- Line 55: As this is an abstract interface, we need to declare a virtual destructor. The default implementation is sufficient
- Line 61: We declare a pure virtual method `Read()` like we did for UART1
- Line 66: We declare a pure virtual method `Write()` like we did for UART1

### UART1.h {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_UART1H}

We will now derive `UART1` from our new `CharDevice` interface.

Update the file `code/libraries/baremetal/include/baremetal/UART1.h`

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
...
42: #include <baremetal/CharDevice.h>
43: 
44: /// @file
45: /// Raspberry Pi UART1 serial device
46: 
47: namespace baremetal {
48: 
49: class IMemoryAccess;
50: 
51: /// <summary>
52: /// Encapsulation for the UART1 device.
53: ///
54: /// This is a pseudo singleton, in that it is not possible to create a default instance (GetUART1() needs to be used for this),
55: /// but it is possible to create an instance with a custom IMemoryAccess instance for testing.
56: /// </summary>
57: class UART1 : public CharDevice
58: {
59:     /// <summary>
60:     /// Construct the singleton UART1 instance if needed, and return a reference to the instance. This is a friend function of class UART1
61:     /// </summary>
62:     /// <returns>Reference to the singleton UART1 instance</returns>
63:     friend UART1& GetUART1();
64: 
65: private:
66:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
67:     bool            m_isInitialized;
68:     /// @brief Memory access interface reference for accessing registers.
69:     IMemoryAccess  &m_memoryAccess;
70: 
71:     UART1();
72: 
73: public:
74:     UART1(IMemoryAccess &memoryAccess);
75: 
76:     void Initialize();
77:     char Read() override;
78:     void Write(char c) override;
79:     void WriteString(const char* str);
80: };
81: 
82: UART1 &GetUART1();
83: 
84: } // namespace baremetal
```

- Line 42: We need to include `CharDevice.h`. This indirectly includes `Types.h`.
- Line 57: We derive from `CharDevice`
- Line 77: We declare the `Read()` method as overriding the method on the interface
- Line 78: We declare the `Write()` method as overriding the method on the interface

The code for UART1 does not need any changes.

### Update project configuration {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
File: d:\Projects\RaspberryPi\baremetal.github.shadow\tutorial\11-uart0\code\libraries\baremetal\CMakeLists.txt
45: set(PROJECT_INCLUDES_PUBLIC
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
64:     )
65: 
...
```

### Configuring, building and debugging {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application does not behave differently.

## Extending mailbox interface - Step 2 {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2}

In order to set the clock, we need to extend `RPIProperties`.

### RPIProperties.h {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_RPIPROPERTIESH}

We add the method `SetClockRate()` as well as the type for the clock to set.

Update the file `code/libraries/baremetal/include/baremetal/RPIProperties.h`

```cpp
File: code/libraries/baremetal/include/baremetal/RPIProperties.h
...
48: namespace baremetal {
49: 
50: /// <summary>
51: /// Clock ID number. Used to retrieve and set the clock frequency for several clocks
52: /// </summary>
53: enum class ClockID : uint32
54: {
55:     /// @brief EMMC clock
56:     EMMC      = 1,
57:     /// @brief UART0 clock
58:     UART      = 2,
59:     /// @brief ARM processor clock
60:     ARM       = 3,
61:     /// @brief Core SoC clock
62:     CORE      = 4,
63:     /// @brief EMMC clock 2
64:     EMMC2     = 12,
65:     /// @brief Pixel clock
66:     PIXEL_BVB = 14,
67: };
68: 
69: /// <summary>
70: /// Top level functionality for requests on Mailbox interface
71: /// </summary>
72: class RPIProperties
73: {
74: private:
75:     /// @brief Reference to mailbox for functions requested
76:     IMailbox &m_mailbox;
77: 
78: public:
79:     explicit RPIProperties(IMailbox &mailbox);
80: 
81:     bool GetBoardSerial(uint64 &serial);
82:     bool SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo);
83: };
84: 
85: } // namespace baremetal
```

### RPIProperties.cpp {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_RPIPROPERTIESCPP}

We implement the new method `SetClockRate()`.

Update the file `code/libraries/baremetal/src/RPIProperties.cpp`

```cpp
File: code/libraries/baremetal/src/RPIProperties.cpp
...
51: /// <summary>
52: /// Mailbox property tag structure for requesting board serial number.
53: /// </summary>
54: struct PropertyTagSerial
55: {
56:     /// Tag ID, must be equal to PROPTAG_GET_BOARD_REVISION.
57:     PropertyTag tag;
58:     /// The requested serial number/ This is a 64 bit unsigned number, divided up into two times a 32 bit number
59:     uint32   serial[2];
60: } PACKED;
61: 
62: /// <summary>
63: /// Mailbox property tag structure for requesting board serial number.
64: /// </summary>
65: struct PropertyTagClockRate
66: {
67:     /// @brief Tag ID, must be equal to PROPTAG_GET_CLOCK_RATE, PROPTAG_GET_MAX_CLOCK_RATE, PROPTAG_GET_MIN_CLOCK_RATE, PROPTAG_GET_CLOCK_RATE_MEASURED or PROPTAG_SET_CLOCK_RATE.
68:     PropertyTag tag;
69:     /// @brief Clock ID, selected the clock for which information is requested or set
70:     uint32   clockID;
71:     /// @brief Requested or set clock frequency, in Hz
72:     uint32   rate;
73:     /// @brief If 1, do not switch to turbo setting if ARM clock is above default.
74:     /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.
75:     uint32   skipTurbo;
76: } PACKED;
...
107: /// <summary>
108: /// Set clock rate for specified clock
109: /// </summary>
110: /// <param name="clockID">ID of clock to be set</param>
111: /// <param name="freqHz">Clock frequencyy in Hz</param>
112: /// <param name="skipTurbo">When true, do not switch to turbo setting if ARM clock is above default.
113: /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.</param>
114: /// <returns>Return true on success, false on failure</returns>
115: bool RPIProperties::SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo)
116: {
117:     PropertyTagClockRate      tag{};
118:     RPIPropertiesInterface interface(m_mailbox);
119: 
120:     tag.clockID   = static_cast<uint32>(clockID);
121:     tag.rate      = freqHz;
122:     tag.skipTurbo = skipTurbo;
123:     auto result   = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));
124: 
125:     // Do not write to console here, as this call is needed to set up the console
126: 
127:     return result;
128: }
129: 
130: } // namespace baremetal
```

- Line 65-76: We declare a struct `PropertyTagClockRate` for `SetClockRate()` to set the clock. This holds, next to the property header:
  - clockID: The id of the clock to be set. This is direct conversion of the `ClockID` enum
  - rate: The frequency for the clock to set, in Hz
  - skipTurbo: By default when setting an ARM frequency above default, other turbo settings will be enabled.
You can disable this effect by setting skipTurbo to 1
- Line 115-128: We implement the method `SetClockRate()`.
The implementation is comparable to that of `GetBoardSerial()`, we simple create an instance of the struct, fill its fields, and call `GetTag()` on the properties interface

### BCMRegisters.h {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_BCMREGISTERSH}

We need to add some registers of the Broadcom SoC in the Raspberry Pi for UART0 (or PL011 UART).

Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
251: //---------------------------------------------
252: // Raspberry Pi UART0
253: //---------------------------------------------
254: 
255: /// @brief Raspberry Pi UART0 registers base address. See @ref RASPBERRY_PI_UART0
256: #define RPI_UART0_BASE                RPI_BCM_IO_BASE + 0x00201000
257: /// @brief Raspberry Pi UART0 data register (R/W). See @ref RASPBERRY_PI_UART0
258: #define RPI_UART0_DR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000000)
259: /// @brief Raspberry Pi UART0 flag register (R/W). See @ref RASPBERRY_PI_UART0
260: #define RPI_UART0_FR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000018)
261: /// @brief Raspberry Pi UART0 integer baud rate divisor register (R/W). See @ref RASPBERRY_PI_UART0
262: #define RPI_UART0_IBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000024)
263: /// @brief Raspberry Pi UART0 factional baud rate divisor register (R/W). See @ref RASPBERRY_PI_UART0
264: #define RPI_UART0_FBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000028)
265: /// @brief Raspberry Pi UART0 line control register (R/W). See @ref RASPBERRY_PI_UART0
266: #define RPI_UART0_LCRH                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000002C)
267: /// @brief Raspberry Pi UART0 control register register (R/W). See @ref RASPBERRY_PI_UART0
268: #define RPI_UART0_CR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000030)
269: /// @brief Raspberry Pi UART0 interrupt FIFO level select register (R/W). See @ref RASPBERRY_PI_UART0
270: #define RPI_UART0_IFLS                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000034)
271: /// @brief Raspberry Pi UART0 interrupt mask set/clear register (R/W). See @ref RASPBERRY_PI_UART0
272: #define RPI_UART0_IMSC                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000038)
273: /// @brief Raspberry Pi UART0 raw interrupt status register (R/W). See @ref RASPBERRY_PI_UART0
274: #define RPI_UART0_RIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000003C)
275: /// @brief Raspberry Pi UART0 masked interrupt status  register (R/W). See @ref RASPBERRY_PI_UART0
276: #define RPI_UART0_MIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000040)
277: /// @brief Raspberry Pi UART0 interrupt clear register (R/W). See @ref RASPBERRY_PI_UART0
278: #define RPI_UART0_ICR                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000044)
279: /// @brief Raspberry Pi UART0 DMA control register (R/W). See @ref RASPBERRY_PI_UART0
280: #define RPI_UART0_DMACR               reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000048)
281: 
282: /// @brief Raspberry Pi UART0 flag register values
283: /// @brief Raspberry Pi UART0 flag register Receive data ready bit. See @ref RASPBERRY_PI_UART0
284: #define RPI_UART0_FR_RX_READY         BIT1(4)
285: /// @brief Raspberry Pi UART0 flag register Transmit data empty bit. See @ref RASPBERRY_PI_UART0
286: #define RPI_UART0_FR_TX_EMPTY         BIT1(5)
287: 
...
```

We will not go into details here, we'll cover this when we use the registers.
More information on the PL011 UARTs (UART0 and others on Raspberry PI 4 and 5) registers can be found [here](#RASPBERRY_PI_UART0) as well as in the official 
[Broadcom documentation BCM2837 (Raspberry Pi 3)](pdf/bcm2837-peripherals.pdf) (page 175),
[Broadcom documentation BCM2711 (Raspberry Pi 4)](pdf/bcm2711-peripherals.pdf) (page 144) and
[Broadcom documentation RP1 (Raspberry Pi 5)](pdf/rp1-peripherals.pdf) (page 34)

The UART0 register addresses are all prefixed with `RPI_UART0_`.

### UART0.h {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_UART0H}

We declare the class `UART0` which derives from `CharDevice`.

Create the file `code/libraries/baremetal/include/baremetal/UART0.h`

```cpp
File: code/libraries/baremetal/include/baremetal/UART0.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
42: #include <baremetal/CharDevice.h>
43: 
44: /// @file
45: /// Raspberry Pi UART0 serial device
46: 
47: namespace baremetal {
48: 
49: class IMemoryAccess;
50: 
51: /// <summary>
52: /// Encapsulation for the UART0 device.
53: ///
54: /// This is a pseudo singleton, in that it is not possible to create a default instance (GetUART0() needs to be used for this),
55: /// but it is possible to create an instance with a custom IMemoryAccess instance for testing.
56: /// </summary>
57: class UART0 : public CharDevice
58: {
59:     /// <summary>
60:     /// Construct the singleton UART0 instance if needed, and return a reference to the instance. This is a friend function of class UART0
61:     /// </summary>
62:     /// <returns>Reference to the singleton UART0 instance</returns>
63:     friend UART0 &GetUART0();
64: 
65: private:
66:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
67:     bool            m_isInitialized;
68:     /// @brief Memory access interface reference for accessing registers.
69:     IMemoryAccess  &m_memoryAccess;
70: 
71:     UART0();
72: 
73: public:
74:     UART0(IMemoryAccess &memoryAccess);
75: 
76:     void Initialize();
77:     char Read() override;
78:     void Write(char c) override;
79:     void WriteString(const char* str);
80: };
81: 
82: UART0 &GetUART0();
83: 
84: } // namespace baremetal
```

The `UART0` class declaration is almost identical to the `UART1` class.

### UART0.cpp {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_UART0CPP}

We implement the class `UART0`.

Create the file `code/libraries/baremetal/src/UART0.cpp`

```cpp
File: code/libraries/baremetal/src/UART0.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include <baremetal/UART0.h>
41: 
42: #include <baremetal/ARMInstructions.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/Mailbox.h>
45: #include <baremetal/MemoryAccess.h>
46: #include <baremetal/PhysicalGPIOPin.h>
47: #include <baremetal/RPIProperties.h>
48: 
49: /// @file
50: /// Raspberry Pi UART0 serial device implementation
51: 
52: namespace baremetal {
53: 
54: /// <summary>
55: /// Constructs a default UART0 instance.
56: ///
57: /// Note that the constructor is private, so GetUART0() is needed to instantiate the UART0.
58: /// </summary>
59: UART0::UART0()
60:     : m_isInitialized{}
61:     , m_memoryAccess{GetMemoryAccess()}
62: {
63: }
64: 
65: /// <summary>
66: /// Constructs a specialized UART1 instance with a custom IMemoryAccess instance. This is intended for testing.
67: /// </summary>
68: /// <param name="memoryAccess">Memory access interface</param>
69: UART0::UART0(IMemoryAccess &memoryAccess)
70:     : m_isInitialized{}
71:     , m_memoryAccess{memoryAccess}
72: {
73: }
74: 
75: /// <summary>
76: /// Initialize the UART0 device. Only performed once, guarded by m_isInitialized.
77: ///
78: ///  Set baud rate and characteristics (115200 8N1) and map to GPIO
79: /// </summary>
80: void UART0::Initialize()
81: {
82:     if (m_isInitialized)
83:         return;
84:     // initialize UART
85:     m_memoryAccess.Write32(RPI_UART0_CR, 0); // turn off UART0
86: 
87:     Mailbox       mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT, m_memoryAccess);
88:     RPIProperties properties(mailbox);
89:     if (!properties.SetClockRate(ClockID::UART, 4000000, false))
90:         return;
91: 
92:     // map UART0 to GPIO pins
93:     PhysicalGPIOPin txdPin(14, GPIOMode::AlternateFunction0, m_memoryAccess);
94:     PhysicalGPIOPin rxdPin(15, GPIOMode::AlternateFunction0, m_memoryAccess);
95:     m_memoryAccess.Write32(RPI_UART0_ICR, 0x7FF); // clear interrupts
96:     m_memoryAccess.Write32(RPI_UART0_IBRD, 2);    // 115200 baud
97:     m_memoryAccess.Write32(RPI_UART0_FBRD, 0xB);
98:     m_memoryAccess.Write32(RPI_UART0_LCRH, 0x7 << 4); // 8n1, enable FIFOs
99:     m_memoryAccess.Write32(RPI_UART0_CR, 0x301);      // enable Tx, Rx, UART
100:     m_isInitialized = true;
101: }
102: 
103: /// <summary>
104: /// Send a character
105: /// </summary>
106: /// <param name="c">Character to be sent</param>
107: void UART0::Write(char c)
108: {
109:     // wait until we can send
110:     // Check Tx FIFO empty
111:     while (m_memoryAccess.Read32(RPI_UART0_FR) & RPI_UART0_FR_TX_EMPTY)
112:     {
113:         NOP();
114:     }
115:     // Write the character to the buffer
116:     m_memoryAccess.Write32(RPI_UART0_DR, static_cast<unsigned int>(c));
117: }
118: 
119: /// <summary>
120: /// Receive a character
121: /// </summary>
122: /// <returns>Character received</returns>
123: char UART0::Read()
124: {
125:     // wait until something is in the buffer
126:     // Check Rx FIFO holds data
127:     while (m_memoryAccess.Read32(RPI_UART0_FR) & RPI_UART0_FR_RX_READY)
128:     {
129:         NOP();
130:     }
131:     // Read it and return
132:     return static_cast<char>(m_memoryAccess.Read32(RPI_UART0_DR));
133: }
134: 
135: /// <summary>
136: /// Write a string
137: /// </summary>
138: /// <param name="str">String to be written</param>
139: void UART0::WriteString(const char *str)
140: {
141:     while (*str)
142:     {
143:         // convert newline to carriage return + newline
144:         if (*str == '\n')
145:             Write('\r');
146:         Write(*str++);
147:     }
148: }
149: 
150: /// <summary>
151: /// Construct the singleton UART0 device if needed, and return a reference to the instance
152: /// </summary>
153: /// <returns>Reference to the singleton UART0 device</returns>
154: UART0 &GetUART0()
155: {
156:     static UART0 value;
157:     value.Initialize();
158:     return value;
159: }
160: 
161: } // namespace baremetal
```

The implementation is very similar to that for `UART1`

- Line 42-47: We also need to include `Mailbox.h` and `RPIProperties.h`
- Line 59-63: The default constructor is identical to the one for `UART1`
- Line 69-73: The constructor taking a MemoryAccess instance is identical to the one for `UART1`
- Line 80-101: The `Initialize()` method is similar to the one for `UART1`
  - Line 85: Disabling the UART uses a different register and value
  - Line 87-90: We set the clock rate for the UART clock to 4 MHz, so that we can set up the baud rate correctly. If this fails, we return without setitng the UART to initialized
  - Line 93: As can be seen in [GPIO functions](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO), we need to set the TxD pin GPIO 14 to alternate function 0 to get the UART0 TxD signal
  - Line 94: As can be seen in [GPIO functions](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO), we need to set the RxD pin GPIO 15 to alternate function 0 to get the UART0 RxD signal
  - Line 95: Switching off interrupts uses a different register and value
  - Line 96-97: Setting the baudrate works differently.
The `RPI_UART0_IBRD` register holds the integral part of a divisor, `RPI_UART0_FBRD` the fractional part. We calculate these part as follows
```text
divisor = UART clock rate / (16 * baudrate) = 4000000 / (16 * 115200) = 2.170184
integer part = 2
fractional part = 0.170184 * 64 = 10.89 -> 11 = 0xB
```

- Line 98: Setting the mode 8N1 enabling the FIFO uses a different register and value.
- Line 99: Enabling the UART uses a different register and value
- Line 107-117: The `Write()` method is similar to the one for `UART1`
  - Line 127: The Tx FIFO empty check uses a different register and value
  - Line 116: The Tx data buffer uses a different register
- Line 123-133: The `Read()` method is similar to the one for `UART1`
  - Line 127: The Rx data ready check uses a different register and value
  - Line 132: The Rx data buffer uses a different register
- Line 139-148: The `WriteString()` method is identical to the one for `UART1`
- Line 154-159: The `GetUART0()` method is almost identical to the one for `UART1`

Setting the baudrate results in a baudrate of (4000000 / (16 * 2.171875)) = 115108 baud, which is close enough to 115200 baud.

### Update the application code {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_UPDATE_THE_APPLICATION_CODE}

#### main.cpp {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_UPDATE_THE_APPLICATION_CODE_MAINCPP}

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

#### System.cpp {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_UPDATE_THE_APPLICATION_CODE_SYSTEMCPP}

As we switch the main application to UART0, we should also switch the code in `System.cpp` to UART0, otherwise we will be suddenly changing the port over, with strange effects.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
48: #include <baremetal/UART0.h>
...
115: void System::Halt()
116: {
117:     GetUART0().WriteString("Halt\n");
118:     Timer::WaitMilliSeconds(WaitTime);
...
138: void System::Reboot()
139: {
140:     GetUART0().WriteString("Reboot\n");
141:     Timer::WaitMilliSeconds(WaitTime);
...
190:     GetUART0().WriteString("Starting up\n");
191: 
192:     extern int main();
...
```

### Update project configuration {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
43:     )
44: 
45: set(PROJECT_INCLUDES_PUBLIC
46:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
47:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
48:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
49:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
50:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
51:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
65:     )
66: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.
The code will now write to UART0, which is connected to GPIO pins 14 (Txd) and 15 (RxD) instead of UART1.
In the end, the visible behaviour does not change.

```text
Starting up
Hello World!
Mailbox call succeeded
Serial: C3D6D0CB
Wait 5 seconds
Press r to reboot, h to halt
```

Next: [12-logger](12-logger.md)

