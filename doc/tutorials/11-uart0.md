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
As we'll see later, we can also retrieve the clock frequency used for UART1, so that we don't have to create defines for this, and always use the correct clock frequency.

We'll start by defining a common interface for UART0 and UART1, so that we can use them interchangeably.
We will then add functionality for setting the UART clock.
Finally we will add and implement UART0.

## Defining a common interface - Step 1 {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1}

In order to be able to use any of UART0 and UART1 for e.g. logging, we need to define a common abstract interface.

We'll start with a generic device. This allows reading and writing blocks of data, in a block by block fashion (also called block device) or one character at a time (a character device).
Files are also block devices, and also support e.g. setting position, and retrieving size.

### Device.h {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_DEVICEH}

Create the file `code/libraries/baremetal/include/baremetal/Device.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Device.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Device.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Device
9: //
10: // Description : Generic device interface
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
42: #include "stdlib/Types.h"
43: 
44: /// @file
45: /// Abstract device
46: 
47: namespace baremetal {
48: 
49: /// <summary>
50: /// Generic device interface
51: /// </summary>
52: class Device
53: {
54: public:
55:     virtual ~Device() = default;
56: 
57:     virtual bool IsBlockDevice() = 0;
58:     virtual ssize_t Read(void* buffer, size_t count);
59:     virtual ssize_t Write(const void* buffer, size_t count);
60:     virtual void Flush();
61: 
62:     virtual ssize_t Seek(size_t offset);
63:     virtual ssize_t GetSize() const;
64: };
65: 
66: } // namespace baremetal
```

- Line 49-66: We declare the class `Device`
  - Line 55: As this is an abstract interface, we need to declare a virtual destructor. The default implementation is sufficient
  - Line 57: We declare a pure virtual method `IsBlockDevice()` to distinguish between block devices and character devices
  - Line 58: We declare a virtual method `Read()` which can read one or more characters / bytes
  - Line 59: We declare a virtual method `Write()` which can write one or more characters / bytes
  - Line 60: We declare a virtual method `Flush()` to write any outstanding data and wait until this is done
  - Line 62: We declare a virtual method `Seek()` to go to a certain position.
This is only supported for block devices
  - Line 63: We declare a virtual method `GetSize()` to retrieve the size of the device data.
This is only supported for block devices

### Device.cpp {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_DEVICECPP}

Let's implement the `Device` class. This will have a trivial default implementation.

Create the file `code/libraries/baremetal/src/Device.cpp`

```cpp
File: code/libraries/baremetal/src/Device.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Device.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Device
9: //
10: // Description : Generic device interface
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
40: #include "baremetal/Device.h"
41: 
42: using namespace baremetal;
43: 
44: /// @file
45: /// Generic device
46: 
47: /// <summary>
48: /// Read a specified number of bytes from the device into a buffer
49: /// </summary>
50: /// <param name="buffer">Buffer, where read data will be placed</param>
51: /// <param name="count">Maximum number of bytes to be read</param>
52: /// <returns>Number of read bytes or < 0 on failure</returns>
53: ssize_t Device::Read(void* buffer, size_t count)
54: {
55:     return static_cast<ssize_t>(-1);
56: }
57: 
58: /// <summary>
59: /// Write a specified number of bytes to the device
60: /// </summary>
61: /// <param name="buffer">Buffer, from which data will be fetched for write</param>
62: /// <param name="count">Number of bytes to be written</param>
63: /// <returns>Number of written bytes or < 0 on failure</returns>
64: ssize_t Device::Write(const void* buffer, size_t count)
65: {
66:     return static_cast<ssize_t>(-1);
67: }
68: 
69: /// <summary>
70: /// Flush any buffers for device
71: /// </summary>
72: void Device::Flush()
73: {
74:     // Do nothing
75: }
76: 
77: /// <summary>
78: /// Seek to a specified offset in the device file.
79: ///
80: /// This is only supported by block devices.
81: /// </summary>
82: /// <param name="offset">Byte offset from start</param>
83: /// <returns>The resulting offset, (ssize_t) -1 on error</returns>
84: ssize_t Device::Seek(uint64 offset)
85: {
86:     return static_cast<ssize_t>(-1);
87: }
88: 
89: /// <summary>
90: /// Get size for a device file
91: ///
92: /// This is only supported by block devices.
93: /// </summary>
94: /// <returns>Total byte size of a block device, (ssize_t) -1 on error</returns>
95: ssize_t Device::GetSize() const
96: {
97:     return static_cast<ssize_t>(-1);
98: }
```

- Line 47-56: We implement the method `Read()`, which will simple return -1
- Line 58-67: We implement the method `Write()`, which will simple return -1
- Line 69-75: We implement the method `Flush()`, which will do nothing
- Line 77-87: We implement the method `Seek()`, which will simple return -1
- Line 89-98: We implement the method `GetSize()`, which will simple return -1

### CharDevice.h {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_CHARDEVICEH}

We'll chreate the character device, which will derive from `Device`. We will later also implement block devices, but for now only character devices.

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
10: // Description : Abstract character read / write device interface
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
43: /// Abstract character device
44: 
45: #include "baremetal/Device.h"
46: 
47: namespace baremetal {
48: 
49: /// <summary>
50: /// Abstract character device
51: ///
52: /// Abstraction of a CharDevice that can read and write characters
53: /// </summary>
54: class CharDevice : public Device
55: {
56: public:
57:     virtual ~CharDevice() = default;
58: 
59:     bool IsBlockDevice() override
60:     {
61:         return false;
62:     }
63:     ssize_t Read(void* buffer, size_t count) override;
64:     ssize_t Write(const void* buffer, size_t count) override;
65: 
66:     /// <summary>
67:     /// Read a character
68:     /// </summary>
69:     /// <returns>Character read</returns>
70:     virtual char Read() = 0;
71:     /// <summary>
72:     /// Write a character
73:     /// </summary>
74:     /// <param name="ch">Character to be written</param>
75:     virtual void Write(char ch) = 0;
76: };
77: 
78: } // namespace baremetal
```

- Line 49-78: We declare the class `CharDevice`
  - Line 57: As this is still an abstract interface, we need to declare a virtual destructor.
The default implementation is sufficient
  - Line 59-62: We define the override for method `IsBlockDevice()`.
It will return false to signal this is a character device
  - Line 63: We declare the override for method `Read()`
  - Line 64: We declare the override for method `Write()`
  - Line 66-70: We declare a pure virtual method `Read()` for a single character.
This is similar to what we did for UART1
  - Line 71-75: We declare a pure virtual method `Write()` for a single character.
This is similar to what we did for UART1

Notice we do not override the methods `Seek()` and `GetSize()` as the default implementations are fine.

### CharDevice.cpp {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_CHARDEVICECPP}

We will now implement the overrides for `CharDevice`.

Update the file `code/libraries/baremetal/src/CharDevice.cpp`

```
File: code/libraries/baremetal/src/CharDevice.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : CharDevice.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : CharDevice
9: //
10: // Description : Abstract character read / write device
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
40: #include "baremetal/CharDevice.h"
41: 
42: using namespace baremetal;
43: 
44: /// @file
45: /// Abstract character device
46: 
47: /// <summary>
48: /// Read a specified number of bytes from the device into a buffer
49: /// </summary>
50: /// <param name="buffer">Buffer, where read data will be placed</param>
51: /// <param name="count">Maximum number of bytes to be read</param>
52: /// <returns>Number of read bytes or < 0 on failure</returns>
53: ssize_t CharDevice::Read(void* buffer, size_t count)
54: {
55:     if (buffer == nullptr)
56:         return static_cast<ssize_t>(-1);
57:     char* bufferPtr = reinterpret_cast<char*>(buffer);
58:     for (size_t i = 0; i < count; ++i)
59:         *bufferPtr++ = Read();
60:     return count;
61: }
62: 
63: /// <summary>
64: /// Write a specified number of bytes to the device
65: /// </summary>
66: /// <param name="buffer">Buffer, from which data will be fetched for write</param>
67: /// <param name="count">Number of bytes to be written</param>
68: /// <returns>Number of written bytes or < 0 on failure</returns>
69: ssize_t CharDevice::Write(const void* buffer, size_t count)
70: {
71:     if (buffer == nullptr)
72:         return static_cast<ssize_t>(-1);
73:     const char* bufferPtr = reinterpret_cast<const char*>(buffer);
74:     for (size_t i = 0; i < count; ++i)
75:     {
76:         // convert newline to carriage return + newline
77:         if (*bufferPtr == '\n')
78:             Write('\r');
79:         Write(*bufferPtr++);
80:     }
81:     return count;
82: }
```

- Line 47-61: We implement the method `Read()`
  - Line 55-56: We change whether the pointer is a null pointer, and return -1 if so
  - Line 57: We cast the pointer to a character pointer
  - Line 58-59: We read the requested number of characters by calling the `Read()` method for a single character
- Line 63-82: We implement the method `Write()`
  - Line 71-72: We change whether the pointer is a null pointer, and return -1 if so
  - Line 73: We cast the pointer to a const character pointer
  - Line 74-80: We write the requested number of characters by calling the `Write()` method for a single character.
Note that we insert an addition carriage return ('\\r') whenever a line feed character ('\\n') is found.
This is the same as we did for UART1

### UART1.h {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_UART1H}

We will now derive `UART1` from our new `CharDevice` class.

Update the file `code/libraries/baremetal/include/baremetal/UART1.h`

```cpp
File: code/libraries/baremetal/include/baremetal/UART1.h
...
42: #include "baremetal/CharDevice.h"
43: 
44: /// @file
45: /// Raspberry Pi UART1 serial device declaration
46: 
47: /// @brief baremetal namespace
48: namespace baremetal {
49: 
50: class IMemoryAccess;
51: 
52: /// <summary>
53: /// Encapsulation for the UART1 device.
54: ///
55: /// This is a pseudo singleton, in that it is not possible to create a default instance (GetUART1() needs to be used for this),
56: /// but it is possible to create an instance with a custom IMemoryAccess instance for testing.
57: /// </summary>
58: class UART1 : public CharDevice
59: {
60:     /// <summary>
61:     /// Construct the singleton UART1 instance if needed, and return a reference to the instance. This is a friend function of class UART1
62:     /// </summary>
63:     /// <returns>Reference to the singleton UART1 instance</returns>
64:     friend UART1& GetUART1();
65: 
66: private:
67:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
68:     bool m_isInitialized;
69:     /// @brief Memory access interface reference for accessing registers.
70:     IMemoryAccess& m_memoryAccess;
71:     /// @brief Baudrate set for device
72:     unsigned m_baudrate;
73: 
74:     UART1();
75: 
76: public:
77:     UART1(IMemoryAccess& memoryAccess);
78: 
79:     void Initialize(unsigned baudrate);
80:     unsigned GetBaudrate() const;
81:     char Read() override;
82:     void Write(char ch) override;
83:     void WriteString(const char* str);
84: };
85: 
86: UART1& GetUART1();
87: 
88: } // namespace baremetal
```

- Line 42: We need to include `CharDevice.h`. This indirectly includes `Types.h`.
- Line 58: We derive from `CharDevice` publicly
- Line 81: We declare the `Read()` method as overriding the method on the parent class
- Line 82: We declare the `Write()` method as overriding the method on the parent class

The code for UART1 does not need any changes.

### Configuring, building and debugging {#TUTORIAL_11_UART0_DEFINING_A_COMMON_INTERFACE___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application does not behave differently.

## Extending mailbox interface - Step 2 {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2}

In order to get or set the clock, we need to extend `RPIProperties`.

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
56:     EMMC = 1,
57:     /// @brief UART0 clock
58:     UART = 2,
59:     /// @brief ARM processor clock
60:     ARM = 3,
61:     /// @brief Core SoC clock
62:     CORE = 4,
63:     /// @brief V3D clock
64:     V3D = 5,
65:     /// @brief H264 clock
66:     H264 = 6,
67:     /// @brief ISP clock
68:     ISP = 7,
69:     /// @brief SDRAM clock
70:     SDRAM = 8,
71:     /// @brief PIXEL clock
72:     PIXEL = 9,
73:     /// @brief PWM clock
74:     PWM = 10,
75:     /// @brief HEVC clock
76:     HEVC = 11,
77:     /// @brief EMMC2 clock 2
78:     EMMC2 = 12,
79:     /// @brief M2MC clock
80:     M2MC = 14,
81:     /// @brief Pixel clock
82:     PIXEL_BVB = 14,
83: };
84: 
85: /// <summary>
86: /// Top level functionality for requests on Mailbox interface
87: /// </summary>
88: class RPIProperties
89: {
90: private:
91:     /// @brief Reference to mailbox for functions requested
92:     IMailbox& m_mailbox;
93: 
94: public:
95:     explicit RPIProperties(IMailbox& mailbox);
96: 
97:     bool GetBoardSerial(uint64& serial);
98:     bool GetClockRate(ClockID clockID, uint32& freqHz);
99:     bool GetMeasuredClockRate(ClockID clockID, uint32& freqHz);
100:     bool SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo);
101: };
102: 
103: } // namespace baremetal
```

- Line 50-83: We define an enum class `ClockID` which defines all the different clocks that can be retrieved or set
- Line 98: We add the method `GetClockRate()` to retrieve the clock rate for a selected clock
- Line 99: We add the method `GetMeasuredClockRate()` to retrieve the actually measured clock rate for a selected clock.
For some clocks the set clock rate is not retrievable, so we can fall back to the measured clock
- Line 100: We add the method `SetClockRate()` to set the clock rate for a selected clock

### RPIProperties.cpp {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_RPIPROPERTIESCPP}

We implement the new method `SetClockRate()`.

Update the file `code/libraries/baremetal/src/RPIProperties.cpp`

```cpp
File: code/libraries/baremetal/src/RPIProperties.cpp
...
62: //// <summary>
63: /// Mailbox property tag structure for requesting board serial number.
64: /// </summary>
65: struct PropertyTagClockRate
66: {
67:     /// @brief Tag ID, must be equal to PROPTAG_GET_CLOCK_RATE, PROPTAG_GET_MAX_CLOCK_RATE, PROPTAG_GET_MIN_CLOCK_RATE, PROPTAG_GET_CLOCK_RATE_MEASURED or PROPTAG_SET_CLOCK_RATE.
68:     PropertyTag tag;
69:     /// @brief Clock ID, selected the clock for which information is requested or set
70:     uint32 clockID;
71:     /// @brief Requested or set clock frequency, in Hz
72:     uint32 rate;
73:     /// @brief If 1, do not switch to turbo setting if ARM clock is above default.
74:     /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.
75:     uint32 skipTurbo;
76: } PACKED;
77: 
...
107: /// <summary>
108: /// Get clock rate for specified clock
109: /// </summary>
110: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
111: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
112: /// <returns>Return true on success, false on failure</returns>
113: bool RPIProperties::GetClockRate(ClockID clockID, uint32& freqHz)
114: {
115:     PropertyTagClockRate tag{};
116:     RPIPropertiesInterface interface(m_mailbox);
117: 
118:     tag.clockID = static_cast<uint32>(clockID);
119:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE, &tag, sizeof(tag));
120: 
121:     if (result)
122:     {
123:         freqHz = tag.rate;
124:     }
125: 
126:     return result;
127: }
128: 
129: /// <summary>
130: /// Get measured clock rate for specified clock
131: /// </summary>
132: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
133: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
134: /// <returns>Return true on success, false on failure</returns>
135: bool RPIProperties::GetMeasuredClockRate(ClockID clockID, uint32& freqHz)
136: {
137:     PropertyTagClockRate tag{};
138:     RPIPropertiesInterface interface(m_mailbox);
139: 
140:     tag.clockID = static_cast<uint32>(clockID);
141:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE_MEASURED, &tag, sizeof(tag));
142: 
143:     if (result)
144:     {
145:         freqHz = tag.rate;
146:     }
147: 
148:     return result;
149: }
150: 
151: /// <summary>
152: /// Set clock rate for specified clock
153: /// </summary>
154: /// <param name="clockID">ID of clock to be set</param>
155: /// <param name="freqHz">Clock frequencyy in Hz</param>
156: /// <param name="skipTurbo">When true, do not switch to turbo setting if ARM clock is above default.
157: /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.</param>
158: /// <returns>Return true on success, false on failure</returns>
159: bool RPIProperties::SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo)
160: {
161:     PropertyTagClockRate tag{};
162:     RPIPropertiesInterface interface(m_mailbox);
163: 
164:     tag.clockID = static_cast<uint32>(clockID);
165:     tag.rate = freqHz;
166:     tag.skipTurbo = skipTurbo;
167:     auto result = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));
168: 
169:     // Do not write to console here, as this call is needed to set up the console
170: 
171:     return result;
172: }
173: 
174: } // namespace baremetal
```

- Line 62-76: We declare a struct `PropertyTagClockRate` for `GetClockRate()`, `GetMeasuredClockRate()`, and `SetClockRate()` to set the clock.
This holds, next to the property header:
  - clockID: The id of the clock to be set. This is a value of the `ClockID` enum
  - rate: The frequency for the clock to set, in Hz
  - skipTurbo: By default when setting an ARM frequency above default, other turbo settings will be enabled.
You can disable this effect by setting skipTurbo to 1
- Line 107-127: We implement the method `GetClockRate()`.
The implementation is comparable to that of `GetBoardSerial()`, we simple create an instance of the struct, fill its fields, and call `GetTag()` on the properties interface
- Line 129-149: We implement the method `GetMeasuredClockRate()`
- Line 151-172: We implement the method `SetClockRate()`

### BCMRegisters.h {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_BCMREGISTERSH}

We can now change the macro to convert the clock for UART1, to take into account the retrieved clock frequency.

We also need to add some registers of the Broadcom SoC in the Raspberry Pi for UART0 (or PL011 UART).
For more information, see [Raspberry Pi PL011 UART](#RASPBERRY_PI_PL011_UART).

Update the file `code/libraries/baremetal/include/baremetal/BCMRegisters.h`:

```cpp
File: code/libraries/baremetal/include/baremetal/BCMRegisters.h
...
255: //---------------------------------------------
256: // Raspberry Pi UART0
257: //---------------------------------------------
258: 
259: /// @brief Raspberry Pi UART0 registers base address. See @ref RASPBERRY_PI_PL011_UART
260: #define RPI_UART0_BASE                RPI_BCM_IO_BASE + 0x00201000
261: /// @brief Raspberry Pi UART0 data register (R/W). See @ref RASPBERRY_PI_PL011_UART
262: #define RPI_UART0_DR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000000)
263: /// @brief Raspberry Pi UART0 flag register (R/W). See @ref RASPBERRY_PI_PL011_UART
264: #define RPI_UART0_FR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000018)
265: /// @brief Raspberry Pi UART0 integer baud rate divisor register (R/W). See @ref RASPBERRY_PI_PL011_UART
266: #define RPI_UART0_IBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000024)
267: /// @brief Raspberry Pi UART0 factional baud rate divisor register (R/W). See @ref RASPBERRY_PI_PL011_UART
268: #define RPI_UART0_FBRD                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000028)
269: /// @brief Raspberry Pi UART0 line control register (R/W). See @ref RASPBERRY_PI_PL011_UART
270: #define RPI_UART0_LCRH                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000002C)
271: /// @brief Raspberry Pi UART0 control register register (R/W). See @ref RASPBERRY_PI_PL011_UART
272: #define RPI_UART0_CR                  reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000030)
273: /// @brief Raspberry Pi UART0 interrupt FIFO level select register (R/W). See @ref RASPBERRY_PI_PL011_UART
274: #define RPI_UART0_IFLS                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000034)
275: /// @brief Raspberry Pi UART0 interrupt mask set/clear register (R/W). See @ref RASPBERRY_PI_PL011_UART
276: #define RPI_UART0_IMSC                reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000038)
277: /// @brief Raspberry Pi UART0 raw interrupt status register (R/W). See @ref RASPBERRY_PI_PL011_UART
278: #define RPI_UART0_RIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x0000003C)
279: /// @brief Raspberry Pi UART0 masked interrupt status  register (R/W). See @ref RASPBERRY_PI_PL011_UART
280: #define RPI_UART0_MIS                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000040)
281: /// @brief Raspberry Pi UART0 interrupt clear register (R/W). See @ref RASPBERRY_PI_PL011_UART
282: #define RPI_UART0_ICR                 reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000044)
283: /// @brief Raspberry Pi UART0 DMA control register (R/W). See @ref RASPBERRY_PI_PL011_UART
284: #define RPI_UART0_DMACR               reinterpret_cast<regaddr>(RPI_UART0_BASE + 0x00000048)
285: 
286: /// @brief Raspberry Pi UART0 flag register values
287: /// @brief Raspberry Pi UART0 flag register Receive data ready bit. See @ref RASPBERRY_PI_PL011_UART
288: #define RPI_UART0_FR_RX_READY         BIT1(4)
289: /// @brief Raspberry Pi UART0 flag register Transmit data empty bit. See @ref RASPBERRY_PI_PL011_UART
290: #define RPI_UART0_FR_TX_EMPTY         BIT1(5)
291: 
...
335: /// @brief Raspberry Pi Mini UART (UART1) Baudrate register. See @ref RASPBERRY_PI_UART1
336: #define RPI_AUX_MU_BAUD       reinterpret_cast<regaddr>(RPI_AUX_BASE + 0x00000068)
337: /// @brief Calculate Raspberry Pi Mini UART (UART1) baud rate value from frequency
338: #define RPI_AUX_MU_BAUD_VALUE(clockRate, baud)   static_cast<uint32>((clockRate / (baud * 8)) - 1)
...
```

We will not go into details here, we'll cover this when we use the registers.
More information on the PL011 UARTs (UART0 and others on Raspberry PI 4 and 5) registers can be found [here](#RASPBERRY_PI_PL011_UART) as well as in the official
[Broadcom documentation BCM2837 (Raspberry Pi 3)](pdf/bcm2837-peripherals.pdf) (page 175),
[Broadcom documentation BCM2711 (Raspberry Pi 4)](pdf/bcm2711-peripherals.pdf) (page 144) and
[Broadcom documentation RP1 (Raspberry Pi 5)](pdf/rp1-peripherals.pdf) (page 34)

The UART0 register addresses are all prefixed with `RPI_UART0_`.

For the UART1 clock, we removed the core clock frequencies as definitions, and changed the macro to calculate the baud rate value to use a specified clock rate.

### UART1.cpp {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_UART1CPP}

We will now request the core clock frequency from the mail box, and use that.

Update the file `code/libraries/baremetal/src/UART1.cpp`

```cpp
File: code/libraries/baremetal/src/UART1.cpp
...
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/Mailbox.h"
45: #include "baremetal/MemoryAccess.h"
46: #include "baremetal/PhysicalGPIOPin.h"
47: #include "baremetal/RPIProperties.h"
...
81: void UART1::Initialize(unsigned baudrate)
82: {
83:     if (m_isInitialized)
84:         return;
85: 
86:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT, m_memoryAccess);
87:     RPIProperties property(mailbox);
88:     uint32 clockFrequency;
89:     property.GetClockRate(ClockID::CORE, clockFrequency);
90: 
91:     // initialize UART
92:     auto value = m_memoryAccess.Read32(RPI_AUX_ENABLES);
93:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value & ~RPI_AUX_ENABLES_UART1); // Disable UART1, AUX mini uart
94: 
95:     PhysicalGPIOPin rxdPin(15, GPIOMode::AlternateFunction5, m_memoryAccess);
96:     PhysicalGPIOPin txdPin(14, GPIOMode::AlternateFunction5, m_memoryAccess);
97: 
98:     m_memoryAccess.Write32(RPI_AUX_ENABLES, value | RPI_AUX_ENABLES_UART1);                                                                                              // enable UART1, AUX mini uart
99:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, 0);                                                                                                                          // Disable Tx, Rx
100:     m_memoryAccess.Write32(RPI_AUX_MU_LCR, RPI_AUX_MU_LCR_DATA_SIZE_8);                                                                                                  // 8 bit mode
101:     m_memoryAccess.Write32(RPI_AUX_MU_MCR, RPI_AUX_MU_MCR_RTS_HIGH);                                                                                                     // RTS high
102:     m_memoryAccess.Write32(RPI_AUX_MU_IER, 0);                                                                                                                           // Disable interrupts
103:     m_memoryAccess.Write32(RPI_AUX_MU_IIR, RPI_AUX_MU_IIR_TX_FIFO_ENABLE | RPI_AUX_MU_IIR_RX_FIFO_ENABLE | RPI_AUX_MU_IIR_TX_FIFO_CLEAR | RPI_AUX_MU_IIR_RX_FIFO_CLEAR); // Clear FIFO
104:     m_memoryAccess.Write32(RPI_AUX_MU_BAUD, RPI_AUX_MU_BAUD_VALUE(clockFrequency, baudrate));                                                                                            // Set baudrate
105:     m_memoryAccess.Write32(RPI_AUX_MU_CNTL, RPI_AUX_MU_CNTL_ENABLE_RX | RPI_AUX_MU_CNTL_ENABLE_TX);                                                                      // Enable Tx, Rx
106: 
107:     m_baudrate = baudrate;
108:     m_isInitialized = true;
109: }

```

- Line 44: We need to include the header for the `Mailbox` class
- Line 47: We need to include the header for the `RPIProperties` class
- Line 86-89: We create a mailbox and an instance of `RPIProperties`, and request the core clock rate.
- Line 104: We use the requested core clock rate to determine the value for programming the requested baud rate.

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
42: #include "baremetal/CharDevice.h"
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
70:     /// @brief Baud rate set for this device
71:     unsigned        m_baudRate;
72: 
73:     UART0();
74: 
75: public:
76:     UART0(IMemoryAccess &memoryAccess);
77: 
78:     void Initialize(unsigned baudrate);
79:     unsigned GetBaudRate() const;
80:     char Read() override;
81:     void Write(char ch) override;
82:     void WriteString(const char* str);
83: };
84: 
85: UART0 &GetUART0();
86: 
87: } // namespace baremetal
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
40: #include "baremetal/UART0.h"
41: 
42: #include "baremetal/ARMInstructions.h"
43: #include "baremetal/BCMRegisters.h"
44: #include "baremetal/Mailbox.h"
45: #include "baremetal/MemoryAccess.h"
46: #include "baremetal/PhysicalGPIOPin.h"
47: #include "baremetal/RPIProperties.h"
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
62:     , m_baudRate{}
63: {
64: }
65: 
66: /// <summary>
67: /// Constructs a specialized UART1 instance with a custom IMemoryAccess instance. This is intended for testing.
68: /// </summary>
69: /// <param name="memoryAccess">Memory access interface</param>
70: UART0::UART0(IMemoryAccess& memoryAccess)
71:     : m_isInitialized{}
72:     , m_memoryAccess{memoryAccess}
73:     , m_baudRate{}
74: {
75: }
76: 
77: /// <summary>
78: /// Convert the requested baudrate to the values for the integer and fractional part of the divisor to be set for UART0
79: /// </summary>
80: /// <param name="baudrate">Requested baud rate</param>
81: /// <param name="baudClock">UART0 clock rate</param>
82: /// <param name="intPart">Integral part of divisor</param>
83: /// <param name="fracPart">Fractional part of divisor</param>
84: static void ConvertBaudrate(unsigned baudrate, unsigned baudClock, uint32& intPart, uint32& fracPart)
85: {
86:     unsigned divisor = 16 * baudrate;
87:     unsigned quotient = baudClock * 128 / divisor;
88:     intPart = quotient / 128;                      // 16 bits
89:     fracPart = (quotient - 128 * intPart + 1) / 2; // 6 bits, round
90: }
91: 
92: /// <summary>
93: /// Initialize the UART0 device. Only performed once, guarded by m_isInitialized.
94: ///
95: ///  Set baud rate and characteristics (8N1) and map to GPIO
96: /// </summary>
97: /// <param name="baudrate">Baud rate to set, maximum is 921600</param>
98: void UART0::Initialize(unsigned baudrate)
99: {
100:     if (m_isInitialized)
101:         return;
102:     // initialize UART
103:     m_memoryAccess.Write32(RPI_UART0_CR, 0); // turn off UART0
104: 
105:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT, m_memoryAccess);
106:     RPIProperties properties(mailbox);
107:     unsigned baudClock = 32000000;
108:     if (!properties.SetClockRate(ClockID::UART, baudClock, false))
109:         return;
110: 
111:     // map UART0 to GPIO pins
112:     PhysicalGPIOPin txdPin(14, GPIOMode::AlternateFunction0, m_memoryAccess);
113:     PhysicalGPIOPin rxdPin(15, GPIOMode::AlternateFunction0, m_memoryAccess);
114:     m_memoryAccess.Write32(RPI_UART0_ICR, 0x7FF); // clear interrupts
115:     uint32 intPart{};
116:     uint32 fracPart{};
117:     ConvertBaudrate(baudrate, baudClock, intPart, fracPart);
118:     m_memoryAccess.Write32(RPI_UART0_IBRD, intPart);
119:     m_memoryAccess.Write32(RPI_UART0_FBRD, fracPart);
120:     m_memoryAccess.Write32(RPI_UART0_LCRH, 0x7 << 4); // 8N1, enable FIFOs
121:     m_memoryAccess.Write32(RPI_UART0_CR, 0x301);      // enable Tx, Rx, UART
122: 
123:     m_baudRate = baudrate;
124:     m_isInitialized = true;
125: }
126: 
127: /// <summary>
128: /// Return set baudrate
129: /// </summary>
130: /// <returns>Baudrate set for device</returns>
131: unsigned UART0::GetBaudRate() const
132: {
133:     return m_baudRate;
134: }
135: 
136: /// <summary>
137: /// Send a character
138: /// </summary>
139: /// <param name="ch">Character to be sent</param>
140: void UART0::Write(char ch)
141: {
142:     // wait until we can send
143:     // Check Tx FIFO empty
144:     while (m_memoryAccess.Read32(RPI_UART0_FR) & RPI_UART0_FR_TX_EMPTY)
145:     {
146:         NOP();
147:     }
148:     // Write the character to the buffer
149:     m_memoryAccess.Write32(RPI_UART0_DR, static_cast<unsigned int>(ch));
150: }
151: 
152: /// <summary>
153: /// Receive a character
154: /// </summary>
155: /// <returns>Character received</returns>
156: char UART0::Read()
157: {
158:     // wait until something is in the buffer
159:     // Check Rx FIFO holds data
160:     while (m_memoryAccess.Read32(RPI_UART0_FR) & RPI_UART0_FR_RX_READY)
161:     {
162:         NOP();
163:     }
164:     // Read it and return
165:     return static_cast<char>(m_memoryAccess.Read32(RPI_UART0_DR));
166: }
167: 
168: /// <summary>
169: /// Write a string
170: /// </summary>
171: /// <param name="str">String to be written</param>
172: void UART0::WriteString(const char* str)
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
183: /// <summary>
184: /// Construct the singleton UART0 device if needed, and return a reference to the instance
185: /// </summary>
186: /// <returns>Reference to the singleton UART0 device</returns>
187: UART0& GetUART0()
188: {
189:     static UART0 value;
190:     return value;
191: }
192: 
193: } // namespace baremetal
194: 
```

The implementation is very similar to that for `UART1`

- Line 42-47: We also need to include `Mailbox.h` and `RPIProperties.h`
- Line 59-63: The default constructor is identical to the one for `UART1`
- Line 69-73: The constructor taking a MemoryAccess instance is identical to the one for `UART1`
- Line 77-90: We add a function `ConvertBaudrate()` to convert the requested baud rate to the correct integral and fractional part of the divisor to be used.
- Line 92-125: The `Initialize()` method is similar to the one for `UART1`
  - Line 103: Disabling the UART uses a different register and value
  - Line 105-109: We set the clock rate for the UART clock to 32 MHz, so that we can set up the baud rate correctly.
If this fails, we return without setitng the UART to initialized.
By using 32 MHz as the clock rate, we can go up to 8 times the standard 115200 baud, so 921600 baud.
Circle uses a clock rate of 4 MHz, limiting the baud rate to 115200
  - Line 112: As can be seen in [GPIO functions](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO), we need to set the TxD pin GPIO 14 to alternate function 0 to get the UART0 TxD signal
  - Line 113: As can be seen in [GPIO functions](#RASPBERRY_PI_GPIO_ALTERNATIVE_FUNCTIONS_FOR_GPIO), we need to set the RxD pin GPIO 15 to alternate function 0 to get the UART0 RxD signal
  - Line 114: Switching off interrupts uses a different register and value
  - Line 115-119: Setting the baudrate works differently.
The `RPI_UART0_IBRD` register holds the integral part of a divisor, `RPI_UART0_FBRD` the fractional part. We calculate these part as follows
```text
divisor = UART clock rate / (16 * baudrate) = 32000000 / (16 * 115200) = 17.3611
integer part = 17 = 0x11
fractional part = 0.3611 * 64 = 23.11 -> 23 = 0x17
```

- Line 120: Setting the mode 8N1 and enabling the FIFO uses a different register and value.
- Line 121: Enabling the UART uses a different register and value
- Line 127-134: The `GetBaudrate()` method is equal to the one for `UART1`
- Line 136-150: The `Write()` method is similar to the one for `UART1`
  - Line 144: The Tx FIFO empty check uses a different register and value
  - Line 149: The Tx data buffer uses a different register
- Line 152-166: The `Read()` method is similar to the one for `UART1`
  - Line 160: The Rx data ready check uses a different register and value
  - Line 165: The Rx data buffer uses a different register
- Line 168-181: The `WriteString()` method is identical to the one for `UART1`
- Line 168-181: The `WriteString()` method is identical to the one for `UART1`
- Line 183-191: The `GetUART0()` method is almost identical to the one for `UART1`

Setting the baudrate results in a baudrate of (32000000 / (16 * 0x17.11)) = 115212 baud, which is close enough to 115200 baud.

### Update the application code {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_UPDATE_THE_APPLICATION_CODE}

#### main.cpp {#TUTORIAL_11_UART0_EXTENDING_MAILBOX_INTERFACE___STEP_2_UPDATE_THE_APPLICATION_CODE_MAINCPP}

Let's use UART0 now.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/BCMRegisters.h"
3: #include "baremetal/Mailbox.h"
4: #include "baremetal/MemoryManager.h"
5: #include "baremetal/RPIProperties.h"
6: #include "baremetal/SysConfig.h"
7: #include "baremetal/Serialization.h"
8: #include "baremetal/System.h"
9: #include "baremetal/Timer.h"
10: #include "baremetal/UART0.h"
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
47: #include "baremetal/UART0.h"
...
104: void System::Halt()
105: {
106:     GetUART0().WriteString("Halt\n");
107:     Timer::WaitMilliSeconds(WaitTime);
...
127: void System::Reboot()
128: {
129:     GetUART0().WriteString("Reboot\n");
130:     Timer::WaitMilliSeconds(WaitTime);
...
178:     GetUART0().Initialize(115200);
179:     GetUART0().WriteString("Starting up\n");
180: 
181:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
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
Serial: 10000000FDA42E87
Wait 5 seconds
Press r to reboot, h to halt
```

Next: [12-console](12-console.md)

