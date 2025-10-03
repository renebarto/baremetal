# Tutorial 14: Board information {#TUTORIAL_14_BOARD_INFORMATION}

@tableofcontents

## New tutorial setup {#TUTORIAL_14_BOARD_INFORMATION_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/13-board-information`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_14_BOARD_INFORMATION_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-14.a`
- a library `output/Debug/lib/stdlib-14.a`
- an application `output/Debug/bin/14-board-information.elf`
- an image in `deploy/Debug/14-board-information-image`

## Retrieving board information {#TUTORIAL_14_BOARD_INFORMATION_RETRIEVING_BOARD_INFORMATION}

In order to start with memory management, we need to know how much memory is available.
Next to this, we wish to have insight in the actual hardware we're running on.

So let's gather some information, and print it.

## MachineInfo {#TUTORIAL_14_BOARD_INFORMATION_MACHINEINFO}

We will create a class `MachineInfo` to retrieve all machine / board specific information.
Later we will update the `Logger` class to print the board type when starting up.

In order to retrieve board information and other info, we will start by extending the functionality for the class `RPIProperties`.

### RPIProperties.h {#TUTORIAL_14_BOARD_INFORMATION_MACHINEINFO_RPIPROPERTIESH}

We will add a few methods to the `RPIProperties` class, to retrieve information about the hardware.

Update the file `code/libraries/baremetal/include/baremetal/RPIProperties.h`

```cpp
File: code/libraries/baremetal/include/baremetal/RPIProperties.h
...
50: /// @brief Raspberry Pi board model
51: enum class BoardModel : uint32
52: {
53:     /// @brief Raspberry Pi 1 Model A
54:     RaspberryPi_A,
55:     /// @brief Raspberry Pi 1 Model B release 1 with 256 Mb RAM
56:     RaspberryPi_BRelease1MB256,
57:     /// @brief Raspberry Pi 1 Model B release 2 with 256 Mb RAM
58:     RaspberryPi_BRelease2MB256,
59:     /// @brief Raspberry Pi 1 Model B with 512 Mb RAM
60:     RaspberryPi_BRelease2MB512,
61:     /// @brief Raspberry Pi 1 Model A+
62:     RaspberryPi_APlus,
63:     /// @brief Raspberry Pi 1 Model B+
64:     RaspberryPi_BPlus,
65:     /// @brief Raspberry Pi Zero
66:     RaspberryPi_Zero,
67:     /// @brief Raspberry Pi Zero W
68:     RaspberryPi_ZeroW,
69:     /// @brief Raspberry Pi Zero 2 W
70:     RaspberryPi_Zero2W,
71:     /// @brief Raspberry Pi 2 Movel B
72:     RaspberryPi_2B,
73:     /// @brief Raspberry Pi 3 Model B
74:     RaspberryPi_3B,
75:     /// @brief Raspberry Pi 3 Model A+
76:     RaspberryPi_3APlus,
77:     /// @brief Raspberry Pi 3 Model B+
78:     RaspberryPi_3BPlus,
79:     /// @brief Raspberry Pi Compute Module 1
80:     RaspberryPi_CM,
81:     /// @brief Raspberry Pi Compute Module 3
82:     RaspberryPi_CM3,
83:     /// @brief Raspberry Pi Compute Module 3+
84:     RaspberryPi_CM3Plus,
85:     /// @brief Raspberry Pi 4 Model B
86:     RaspberryPi_4B,
87:     /// @brief Raspberry Pi 400
88:     RaspberryPi_400,
89:     /// @brief Raspberry Pi Compute Module 4
90:     RaspberryPi_CM4,
91:     /// @brief Raspberry Pi Compute Module 4S
92:     RaspberryPi_CM4S,
93:     /// @brief Raspberry Pi 5 Model B
94:     RaspberryPi_5B,
95:     /// @brief Model unknown / not set / invalid
96:     Unknown
97: };
98: 
99: /// <summary>
100: /// Raspberry Pi board revision number
101: /// </summary>
102: enum class BoardRevision : uint32
103: {
104:     /// @brief Raspberry Pi 1 Model B Revision 1
105:     RaspberryPi_BRev1 = 0x00000002,
106:     /// @brief Raspberry Pi 1 Model B Revision 1 no fuse
107:     RaspberryPi_BRev1NoFuse = 0x00000003,
108:     /// @brief Raspberry Pi 1 Model B Revision 2 256 Mb RAM
109:     RaspberryPi_BRev2_256_1 = 0x00000004,
110:     /// @brief Raspberry Pi 1 Model B Revision 2 256 Mb RAM
111:     RaspberryPi_BRev2_256_2 = 0x00000005,
112:     /// @brief Raspberry Pi 1 Model B Revision 2 256 Mb RAM
113:     RaspberryPi_BRev2_256_3 = 0x00000006,
114:     /// @brief Raspberry Pi 1 Model A Revision 1
115:     RaspberryPi_A_1 = 0x00000007,
116:     /// @brief Raspberry Pi 1 Model A Revision 1
117:     RaspberryPi_A_2 = 0x00000008,
118:     /// @brief Raspberry Pi 1 Model A Revision 1
119:     RaspberryPi_A_3 = 0x00000009,
120:     /// @brief Raspberry Pi 1 Model B Revision 2 512 Mb RAM
121:     RaspberryPi_BRev2_512_1 = 0x0000000D,
122:     /// @brief Raspberry Pi 1 Model B Revision 2 512 Mb RAM
123:     RaspberryPi_BRev2_512_2 = 0x0000000E,
124:     /// @brief Raspberry Pi 1 Model B Revision 2 512 Mb RAM
125:     RaspberryPi_BRev2_512_3 = 0x0000000F,
126:     /// @brief Raspberry Pi 1 Model B+
127:     RaspberryPi_BPlus_1 = 0x00000010,
128:     /// @brief Raspberry Pi 1 Model B+
129:     RaspberryPi_BPlus_2 = 0x00000013,
130:     /// @brief Raspberry Pi 1 Model B+
131:     RaspberryPi_BPlus_3 = 0x00900032,
132:     /// @brief Raspberry Pi 1 Compute Module 1
133:     RaspberryPi_CM_1 = 0x00000011,
134:     /// @brief Raspberry Pi 1 Compute Module 1
135:     RaspberryPi_CM_2 = 0x00000014,
136:     /// @brief Raspberry Pi 1 Model A+ Revision 1 256 Mb RAM
137:     RaspberryPi_APlus_256 = 0x00000012,
138:     /// @brief Raspberry Pi 1 Model A+ Revision 1 512 Mb RAM
139:     RaspberryPi_APlus_512 = 0x00000015,
140:     /// @brief Raspberry Pi 2 Model B Revision 1.1.1
141:     RaspberryPi_2BRev1_1_1 = 0x00A01041,
142:     /// @brief Raspberry Pi 2 Model B Revision 1.1.2
143:     RaspberryPi_2BRev1_1_2 = 0x00A21041,
144:     /// @brief Raspberry Pi 2 Model B Revision 1.2
145:     RaspberryPi_2BRev1_2 = 0x00A22042,
146:     /// @brief Raspberry Pi Zero Revision 1.2
147:     RaspberryPi_ZeroRev1_2 = 0x00900092,
148:     /// @brief Raspberry Pi Zero Revision 1.4
149:     RaspberryPi_ZeroRev1_4 = 0x00900093,
150:     /// @brief Raspberry Pi Zero W
151:     RaspberryPi_ZeroW = 0x009000C1,
152:     /// @brief Raspberry Pi 3 Model B Revision 1
153:     RaspberryPi_3B_1 = 0x00A02082,
154:     /// @brief Raspberry Pi 3 Model B Revision 2
155:     RaspberryPi_3B_2 = 0x00A22082,
156:     /// @brief Raspberry Pi 3 Model B+ Revision 1
157:     RaspberryPi_3BPlus = 0x00A200D3,
158:     /// @brief Raspberry Pi 4 Model B Revision 1.1 1 Gb RAM
159:     RaspberryPi_4BRev1_1_1Gb = 0x00A03111,
160:     /// @brief Raspberry Pi 4 Model B Revision 1.1 2 Gb RAM
161:     RaspberryPi_4BRev1_1_2Gb = 0x00B03111,
162:     /// @brief Raspberry Pi 4 Model B Revision 1.2 2 Gb RAM
163:     RaspberryPi_4BRev1_2_2Gb = 0x00B03112,
164:     /// @brief Raspberry Pi 4 Model B Revision 1.1 4 Gb RAM
165:     RaspberryPi_4BRev1_1_4Gb = 0x00C03111,
166:     /// @brief Raspberry Pi 4 Model B Revision 1.2 4 Gb RAM
167:     RaspberryPi_4BRev1_2_4Gb = 0x00C03112,
168:     /// @brief Raspberry Pi 4 Model B Revision 1.4 4 Gb RAM
169:     RaspberryPi_4BRev1_4_4Gb = 0x00C03114,
170:     /// @brief Raspberry Pi 4 Model B Revision 1.4 8 Gb RAM
171:     RaspberryPi_4BRev1_4_8Gb = 0x00D03114,
172:     /// @brief Raspberry Pi 400
173:     RaspberryPi_400 = 0x00C03130,
174:     /// @brief Raspberry Pi Zero Model 2 W
175:     RaspberryPi_Zero2W = 0x00902120,
176:     /// @brief Raspberry Pi 5 Model B Revision 1, code TBD
177:     RaspberryPi_5B = 0x00000000,
178: };
179: 
...
215: /// <summary>
216: /// Top level functionality for requests on Mailbox interface
217: /// </summary>
218: class RPIProperties
219: {
220: private:
221:     /// @brief Reference to mailbox for functions requested
222:     IMailbox& m_mailbox;
223: 
224: public:
225:     explicit RPIProperties(IMailbox& mailbox);
226: 
227:     bool GetFirmwareRevision(uint32& revision);
228:     bool GetBoardModel(BoardModel& model);
229:     bool GetBoardRevision(BoardRevision& revision);
230:     bool GetBoardMACAddress(uint8 address[6]);
231:     bool GetBoardSerial(uint64& serial);
232:     bool GetARMMemory(uint32& baseAddress, uint32& size);
233:     bool GetVCMemory(uint32& baseAddress, uint32& size);
234:     bool GetClockRate(ClockID clockID, uint32& freqHz);
235:     bool GetMeasuredClockRate(ClockID clockID, uint32& freqHz);
236:     bool SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo);
237: };
...
```

- Line 50-97: We add the enum `BoardModel` to define all different Raspberry Pi board models
- Line 99-178: We add the enum `BoardRevision` to define all different revision codes for Raspberry Pi boards
- Line 227: We add the method `GetFirmwareRevision()` to retrieve the FW revision of the board
- Line 228: We add the method `GetBoardModel()` to retrieve the board model
- Line 229: We add the method `GetBoardRevision()` to retrieve the board revision
- Line 230: We add the method `GetBoardMACAddress()` to retrieve the network MAC address
- Line 232: We add the method `GetARMMemory()` to retrieve the base address and size of memory allocated to the ARM cores
- Line 233: We add the method `GetVCMemory()` to retrieve the base address and size of memory allocated to the VideoCore

### RPIProperties.cpp {#TUTORIAL_14_BOARD_INFORMATION_MACHINEINFO_RPIPROPERTIESCPP}

Next we'll implement the new methods for class `RPIProperties`.

Update the file `code/libraries/baremetal/src/RPIProperties.cpp`.

```cpp
File: code/libraries/baremetal/src/RPIProperties.cpp
...
40: #include "baremetal/RPIProperties.h"
41: 
42: #include "baremetal/BCMRegisters.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/RPIProperties.h"
45: #include "baremetal/RPIPropertiesInterface.h"
46: #include "stdlib/Util.h"
...
51: /// @brief Define log name
52: LOG_MODULE("RPIProperties");
53: 
...
83: /// <summary>
84: /// Mailbox property tag structure for requesting MAC address.
85: /// </summary>
86: struct PropertyTagMACAddress
87: {
88:     /// Tag ID, must be equal to PROPTAG_GET_MAC_ADDRESS.
89:     PropertyTag tag;
90:     /// MAC Address (6 bytes)
91:     uint8 address[6];
92:     /// Padding to align to 4 bytes
93:     uint8 padding[2];
94: } PACKED;
95: 
96: /// <summary>
97: /// Mailbox property tag structure for requesting memory information.
98: /// </summary>
99: struct PropertyTagMemory
100: {
101:     /// Tag ID, must be equal to PROPTAG_GET_ARM_MEMORY or PROPTAG_GET_VC_MEMORY.
102:     PropertyTag tag;
103:     /// Base address
104:     uint32 baseAddress;
105:     /// Size in bytes
106:     uint32 size;
107: } PACKED;
108: 
...
118: /// <summary>
119: /// Retrieve FW revision number
120: /// </summary>
121: /// <param name="revision">FW revision (out)</param>
122: /// <returns>Return true on success, false on failure</returns>
123: bool RPIProperties::GetFirmwareRevision(uint32& revision)
124: {
125:     PropertyTagSimple tag{};
126:     RPIPropertiesInterface interface(m_mailbox);
127: 
128:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_FIRMWARE_REVISION, &tag, sizeof(tag));
129: 
130:     TRACE_DEBUG("GetFirmwareRevision");
131:     TRACE_DEBUG("Result: %s", result ? "OK" : "Fail");
132: 
133:     if (result)
134:     {
135:         revision = tag.value;
136:         TRACE_DEBUG("Revision: %08lx", tag.value);
137:     }
138: 
139:     return result;
140: }
141: 
142: /// <summary>
143: /// Retrieve Raspberry Pi board model
144: /// </summary>
145: /// <param name="model">Board model (out)</param>
146: /// <returns>Return true on success, false on failure</returns>
147: bool RPIProperties::GetBoardModel(BoardModel& model)
148: {
149:     PropertyTagSimple tag{};
150:     RPIPropertiesInterface interface(m_mailbox);
151: 
152:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_MODEL, &tag, sizeof(tag));
153: 
154:     TRACE_DEBUG("GetBoardModel");
155:     TRACE_DEBUG("Result: %s", result ? "OK" : "Fail");
156: 
157:     if (result)
158:     {
159:         model = static_cast<BoardModel>(tag.value);
160:         TRACE_DEBUG("Model: %08lx", tag.value);
161:     }
162: 
163:     return result;
164: }
165: 
166: /// <summary>
167: /// Retrieve Raspberry Pi board revision
168: /// </summary>
169: /// <param name="revision">Board revision (out)</param>
170: /// <returns>Return true on success, false on failure</returns>
171: bool RPIProperties::GetBoardRevision(BoardRevision& revision)
172: {
173:     PropertyTagSimple tag{};
174:     RPIPropertiesInterface interface(m_mailbox);
175: 
176:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_REVISION, &tag, sizeof(tag));
177: 
178:     TRACE_DEBUG("GetBoardRevision");
179:     TRACE_DEBUG("Result: %s", result ? "OK" : "Fail");
180: 
181:     if (result)
182:     {
183:         revision = static_cast<BoardRevision>(tag.value);
184:         TRACE_DEBUG("Revision: %08lx", tag.value);
185:     }
186: 
187:     return result;
188: }
189: 
190: /// <summary>
191: /// Retrieve network MAC address
192: /// </summary>
193: /// <param name="address">MAC address (out)</param>
194: /// <returns>Return true on success, false on failure</returns>
195: bool RPIProperties::GetBoardMACAddress(uint8 address[6])
196: {
197:     PropertyTagMACAddress tag{};
198:     RPIPropertiesInterface interface(m_mailbox);
199: 
200:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_MAC_ADDRESS, &tag, sizeof(tag));
201: 
202:     TRACE_DEBUG("GetBoardMACAddress");
203:     TRACE_DEBUG("Result: %s", result ? "OK" : "Fail");
204: 
205:     if (result)
206:     {
207:         memcpy(address, tag.address, sizeof(tag.address));
208:         TRACE_DEBUG("Address: %02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4], address[5]);
209:     }
210: 
211:     return result;
212: }
213: 
214: /// <summary>
215: /// Request board serial number
216: /// </summary>
217: /// <param name="serial">On return, set to serial number, if successful</param>
218: /// <returns>Return true on success, false on failure</returns>
219: bool RPIProperties::GetBoardSerial(uint64& serial)
220: {
221:     PropertyTagSerial tag{};
222:     RPIPropertiesInterface interface(m_mailbox);
223: 
224:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_SERIAL, &tag, sizeof(tag));
225: 
226:     TRACE_DEBUG("GetBoardSerial");
227:     TRACE_DEBUG("Result: %s", result ? "OK" : "Fail");
228: 
229:     if (result)
230:     {
231:         serial = (static_cast<uint64>(tag.serial[1]) << 32 | static_cast<uint64>(tag.serial[0]));
232:         TRACE_DEBUG("Serial: %016llx", serial);
233:     }
234: 
235:     return result;
236: }
237: 
238: /// <summary>
239: /// Retrieve ARM assigned memory base address and size
240: /// </summary>
241: /// <param name="baseAddress">ARM assigned base address (out)</param>
242: /// <param name="size">ARM assigned memory size in bytes (out)</param>
243: /// <returns>Return true on success, false on failure</returns>
244: bool RPIProperties::GetARMMemory(uint32& baseAddress, uint32& size)
245: {
246:     PropertyTagMemory tag{};
247:     RPIPropertiesInterface interface(m_mailbox);
248: 
249:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_ARM_MEMORY, &tag, sizeof(tag));
250: 
251:     TRACE_DEBUG("GetARMMemory");
252:     TRACE_DEBUG("Result: %s", result ? "OK" : "Fail");
253: 
254:     if (result)
255:     {
256:         baseAddress = tag.baseAddress;
257:         size = tag.size;
258: 
259:         TRACE_DEBUG("Base address: %08lx", baseAddress);
260:         TRACE_DEBUG("Size:         %08lx", size);
261:     }
262: 
263:     return result;
264: }
265: 
266: /// <summary>
267: /// Retrieve VideoCore assigned memory base address and size
268: /// </summary>
269: /// <param name="baseAddress">VideoCore assigned base address (out)</param>
270: /// <param name="size">VideoCore assigned memory size in bytes (out)</param>
271: /// <returns>Return true on success, false on failure</returns>
272: bool RPIProperties::GetVCMemory(uint32& baseAddress, uint32& size)
273: {
274:     PropertyTagMemory tag{};
275:     RPIPropertiesInterface interface(m_mailbox);
276: 
277:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_VC_MEMORY, &tag, sizeof(tag));
278: 
279:     TRACE_DEBUG("GetARMMemory");
280:     TRACE_DEBUG("Result: %s", result ? "OK" : "Fail");
281: 
282:     if (result)
283:     {
284:         baseAddress = tag.baseAddress;
285:         size = tag.size;
286:         TRACE_DEBUG("Base address: %08lx", baseAddress);
287:         TRACE_DEBUG("Size:         %08lx", size);
288:     }
289: 
290:     return result;
291: }
292: 
293: /// <summary>
294: /// Get clock rate for specified clock
295: /// </summary>
296: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
297: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
298: /// <returns>Return true on success, false on failure</returns>
299: bool RPIProperties::GetClockRate(ClockID clockID, uint32& freqHz)
300: {
301:     PropertyTagClockRate tag{};
302:     RPIPropertiesInterface interface(m_mailbox);
303: 
304:     tag.clockID = static_cast<uint32>(clockID);
305:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE, &tag, sizeof(tag));
306: 
307:     TRACE_DEBUG("GetClockRate");
308:     TRACE_DEBUG("Clock ID:   %08lx", tag.clockID);
309:     TRACE_DEBUG("Result: %s", result ? "OK" : "Fail");
310: 
311:     if (result)
312:     {
313:         freqHz = tag.rate;
314:         TRACE_DEBUG("Rate:       %08lx", tag.rate);
315:     }
316: 
317:     return result;
318: }
319: 
320: /// <summary>
321: /// Get measured clock rate for specified clock
322: /// </summary>
323: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
324: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
325: /// <returns>Return true on success, false on failure</returns>
326: bool RPIProperties::GetMeasuredClockRate(ClockID clockID, uint32& freqHz)
327: {
328:     PropertyTagClockRate tag{};
329:     RPIPropertiesInterface interface(m_mailbox);
330: 
331:     tag.clockID = static_cast<uint32>(clockID);
332:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE_MEASURED, &tag, sizeof(tag));
333: 
334:     TRACE_DEBUG("GetMeasuredClockRate");
335:     TRACE_DEBUG("Clock ID:   %08lx", tag.clockID);
336:     TRACE_DEBUG("Result: %s", result ? "OK" : "Fail");
337: 
338:     if (result)
339:     {
340:         freqHz = tag.rate;
341:         TRACE_DEBUG("Rate:       %08lx", tag.rate);
342:     }
343: 
344:     return result;
345: }
346: 
347: /// <summary>
348: /// Set clock rate for specified clock
349: /// </summary>
350: /// <param name="clockID">ID of clock to be set</param>
351: /// <param name="freqHz">Clock frequencyy in Hz</param>
352: /// <param name="skipTurbo">When true, do not switch to turbo setting if ARM clock is above default.
353: /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.</param>
354: /// <returns>Return true on success, false on failure</returns>
355: bool RPIProperties::SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo)
356: {
357:     PropertyTagClockRate tag{};
358:     RPIPropertiesInterface interface(m_mailbox);
359: 
360:     tag.clockID = static_cast<uint32>(clockID);
361:     tag.rate = freqHz;
362:     tag.skipTurbo = skipTurbo;
363:     auto result = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));
364: 
365:     // Do not write to console here, as this call is needed to set up the console
366: 
367:     return result;
368: }
369: 
370: } // namespace baremetal
```

- Line 43: We need to include the header for the `Logger` class
- Line 44: We also need to include the header for the `RPIProperties` class
- Line 51-52: We define the log module name as we will be logging log macros
- Line 83-94: We declare the tag structure to hold the MAC addres `PropertyMACTagAddress`
- Line 96-107: We declare the tag structure to memory information `PropertyTagMemory`
- Line 118-140: We implement the member function `GetFirmwareRevision()`.
This uses the `PropertyTagSimple` structure to retrieve the firmware revision, which is a simple 32 bit unsigned integer.
Note that debug statements are added, which print more information in case the trace level is at least Debug
- Line 142-164: We implement the member function `GetBoardModel()`.
Again, this uses the `PropertyTagSimple` structure to retrieve the firmware revision, which is a simple 32 bit unsigned integer
- Line 166-188: We implement the member function `GetBoardRevision()`.
Again, this uses the `PropertyTagSimple` structure to retrieve the firmware revision, which is a simple 32 bit unsigned integer
- Line 190-212: We implement the member function `GetBoardMACAddress()`.
As a MAC address consists of 6 8 bit unsigned integers, we use the `PropertyTagMACAddress` structure to retrieve the MAC address
- Line 214-236: We add tracing information to the member function `GetBoardSerial()`
- Line 238-264: We implement the member function `GetARMMemory()`.
We use the `PropertyTagMemory` structure to retrieve the base address and size of the memory allocated to the ARM cores
- Line 266-291: We implement the member function `GetVCMemory()`.
We use the `PropertyTagMemory` structure to retrieve the base address and size of the memory allocated to the VC memory
- Line 293-318: We add tracing information to the member function `GetClockRate()`.
- Line 320-345: We add tracing information to the member function `GetMeasuredClockRate()`.
- Line 355-368: We add tracing information to the member function `SetClockRate()`.
Note we do not trace here, as the clock needs to be set for UART0 and possibly UART1 before tracing can be done

### MachineInfo.h {#TUTORIAL_14_BOARD_INFORMATION_MACHINEINFO_MACHINEINFOH}

Now we can add the class `MachineInfo` which uses the methods defined in `RPIProperties` to retrieve and process information on the hardware.

Create the file `code/libraries/baremetal/include/baremetal/MachineInfo.h`

```cpp
File: code/libraries/baremetal/include/baremetal/MachineInfo.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MachineInfo.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MachineInfo
9: //
10: // Description : Basic machine info
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
42: #include "baremetal/RPIProperties.h"
43: 
44: /// @file
45: /// Machine info retrieval
46: 
47: namespace baremetal {
48: 
49: class IMemoryAccess;
50: 
51: /// <summary>
52: /// Type of SoC used. See @ref RASPBERRY_PI_BAREMETAL_DEVELOPMENT_SOC_FOR_EACH_BOARD for more information
53: /// </summary>
54: enum class SoCType
55: {
56:     /// @ brief BCM2835 as used in Raspberry Pi Model 1 boards
57:     BCM2835,
58:     /// @ brief BCM2836 as used in older Raspberry Pi Model 2 boards
59:     BCM2836,
60:     /// @ brief BCM2837 as used in newer Raspberry Pi Model 2 and Raspberry Pi Model 3 boards
61:     BCM2837,
62:     /// @ brief BCM2711 as used in Raspberry Pi Model 4 boards
63:     BCM2711,
64:     /// @ brief BCM2712 as used in Raspberry Pi Model 5 boards
65:     BCM2712,
66:     /// @brief SoC unknown / not set / invalid
67:     Unknown,
68: };
69: 
70: /// <summary>
71: /// Retrieves system info using the mailbox mechanism
72: ///
73: /// Note that this class is created as a singleton, using the GetMachineInfo() function.
74: /// </summary>
75: class MachineInfo
76: {
77:     /// <summary>
78:     /// Retrieves the singleton MachineInfo instance. It is created in the first call to this function. This is a friend function of class MachineInfo
79:     /// </summary>
80:     /// <returns>A reference to the singleton MachineInfo</returns>
81:     friend MachineInfo& GetMachineInfo();
82: 
83: private:
84:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
85:     bool m_initialized;
86:     /// @brief Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
87:     IMemoryAccess& m_memoryAccess;
88:     /// @brief Raw revision code retrieved through the mailbox
89:     BoardRevision m_revisionRaw;
90:     /// @brief Board model determined from the raw revision code
91:     BoardModel m_boardModel;
92:     /// @brief Board model major number determined from the raw revision code
93:     uint32 m_boardModelMajor;
94:     /// @brief Board model revision number determined from the raw revision code
95:     uint32 m_boardModelRevision;
96:     /// @brief Board SoC type determined from the raw revision code
97:     SoCType m_SoCType;
98:     /// @brief Amount of physical RAM determined from the raw revision code (in Mb)
99:     uint32 m_ramSize;
100:     /// @brief Board serial number retrieved through the mailbox
101:     uint64 m_boardSerial;
102:     /// @brief Board FW revision number retrieved through the mailbox
103:     uint32 m_fwRevision;
104:     /// @brief Ethernet MAC address retrieved through the mailbox
105:     uint8 m_macAddress[6];
106:     /// @brief ARM assigned memory base address retrieved through the mailbox
107:     uint32 m_armBaseAddress;
108:     /// @brief ARM assigned memory size retrieved through the mailbox
109:     uint32 m_armMemorySize;
110:     /// @brief VideoCore assigned memory base address retrieved through the mailbox
111:     uint32 m_vcBaseAddress;
112:     /// @brief VideoCore assigned memory size retrieved through the mailbox
113:     uint32 m_vcMemorySize;
114: 
115:     MachineInfo();
116: 
117: public:
118:     MachineInfo(IMemoryAccess& memoryAccess);
119:     bool Initialize();
120: 
121:     BoardModel GetModel();
122:     const char* GetName();
123:     uint32 GetModelMajor();
124:     uint32 GetModelRevision();
125:     SoCType GetSoCType();
126:     const char* GetSoCName();
127:     uint32 GetRAMSize();
128:     uint64 GetSerial();
129:     uint32 GetFWRevision();
130:     void GetMACAddress(uint8 macAddress[6]);
131:     uint32 GetARMMemoryBaseAddress();
132:     uint32 GetARMMemorySize();
133:     uint32 GetVCMemoryBaseAddress();
134:     uint32 GetVCMemorySize();
135:     unsigned GetClockRate(ClockID clockID) const; // See RPIPropertiesInterface (PROPTAG_GET_CLOCK_RATE)
136: 
137:     BoardRevision GetBoardRevision();
138: };
139: 
140: MachineInfo& GetMachineInfo();
141: 
142: } // namespace baremetal
```

- Line 51-68: We declare an enum to represent the type of SoC (System-on-Chip) on the board
- Line 70-138: We declare the class `MachineInfo`
  - Line 77-81: We declare the friend function `GetMachineInfo()` which creates, initializes and returns a reference to the singleton `MachineInfo` instance
  - Line 115: We declare the prive constructor, which is used by `GetMachineInfo()` to instantiate a default version of `MachineInfo`
  - Line 118: We declare a constructor, which injects a memory access interface for testing purposes
  - Line 119: We declare the method `Initialize()` which initialized the instance.
  This perform the bulk of the work to be done
  - Line 121: We declare the method `GetModel()` which returns the board model
  - Line 122: We declare the method `GetName()` which returns the board name
  - Line 123: We declare the method `GetModelMajor()` which returns the board major model number
  - Line 124: We declare the method `GetModelRevision()` which returns the board revision number
  - Line 125: We declare the method `GetSoCType()` which returns the board SoC type
  - Line 126: We declare the method `GetSoCName()` which returns the board SoC name
  - Line 127: We declare the method `GetRAMSize()` which returns the physical RAM size in Mb
  - Line 128: We declare the method `GetSerial()` which returns the board serial number
  - Line 129: We declare the method `GetFWRevision()` which returns the board firmware revision
  - Line 130: We declare the method `GetMACAddress()` which returns the ethernet MAC address
  - Line 131: We declare the method `GetARMMemoryBaseAddress()` which returns the ARM assign memory base address (normally 0)
  - Line 132: We declare the method `GetARMMemorySize()` which returns the ARM assigned memory size in bytes.
This is the memory available to the ARM cores up to the 1 Gb border, memory above 1 Gb is not specified here. The memory reserved for the VC is always at the end of the 1Gb range
  - Line 133: We declare the method `GetVCMemoryBaseAddress()` which returns the VideoCore assigned memory base address
  - Line 134: We declare the method `GetVCMemorySize()` which returns the VideoCore assigned memory size in bytes
  - Line 135: We declare the method `GetClockRate()` which returns the clock rate for the specified clock ID
  - Line 137: We declare the method `GetBoardRevision()` which returns the raw board revision
- Line 140: We declare the friend function `GetMachineInfo()`

### MachineInfo.cpp {#TUTORIAL_14_BOARD_INFORMATION_MACHINEINFO_MACHINEINFOCPP}

We'll implement the method for class `MachineInfo`.

Create the file `code/libraries/baremetal/src/MachineInfo.cpp`

```cpp
File: code/libraries/baremetal/src/MachineInfo.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : MachineInfo.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : MachineInfo
9: //
10: // Description : Basic machine info
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
40: #include "baremetal/MachineInfo.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Console.h"
44: #include "baremetal/Mailbox.h"
45: #include "baremetal/MemoryAccess.h"
46: #include "stdlib/Util.h"
47: 
48: /// @file
49: /// Machine info retrieval implementation
50: 
51: namespace baremetal {
52: 
53: /// <summary>
54: /// Raspberry Pi board information
55: /// </summary>
56: struct BoardInfo
57: {
58:     /// @brief Board type
59:     unsigned type;
60:     /// @brief Board model
61:     BoardModel model;
62:     /// @brief Board major revision number
63:     unsigned majorRevision;
64: };
65: 
66: /// @brief Mapping from raw board revision to board model and major revision number
67: static BoardInfo s_boardInfo[]{
68:     {0,  BoardModel::RaspberryPi_A,              1},
69:     {1,  BoardModel::RaspberryPi_BRelease2MB512, 1}, // can be other revision
70:     {2,  BoardModel::RaspberryPi_APlus,          1},
71:     {3,  BoardModel::RaspberryPi_BPlus,          1},
72:     {4,  BoardModel::RaspberryPi_2B,             2},
73:     {6,  BoardModel::RaspberryPi_CM,             1},
74:     {8,  BoardModel::RaspberryPi_3B,             3},
75:     {9,  BoardModel::RaspberryPi_Zero,           1},
76:     {10, BoardModel::RaspberryPi_CM3,            3},
77:     {12, BoardModel::RaspberryPi_ZeroW,          1},
78:     {13, BoardModel::RaspberryPi_3BPlus,         3},
79:     {14, BoardModel::RaspberryPi_3APlus,         3},
80:     {16, BoardModel::RaspberryPi_CM3Plus,        3},
81:     {17, BoardModel::RaspberryPi_4B,             4},
82:     {18, BoardModel::RaspberryPi_Zero2W,         3},
83:     {19, BoardModel::RaspberryPi_400,            4},
84:     {20, BoardModel::RaspberryPi_CM4,            4},
85:     {21, BoardModel::RaspberryPi_CM4S,           4},
86:     {99, BoardModel::RaspberryPi_5B,             4}
87: };
88: 
89: /// <summary>
90: /// Mapping from BoardModel to board name
91: ///
92: /// Must match BoardModel one to one
93: /// </summary>
94: /* clang-format off */
95: static const char* m_boardName[] =
96: {
97:     "Raspberry Pi Model A",
98:     "Raspberry Pi Model B R1",
99:     "Raspberry Pi Model B R2",
100:     "Raspberry Pi Model B R2",
101:     "Raspberry Pi Model A+",
102:     "Raspberry Pi Model B+",
103:     "Raspberry Pi Zero",
104:     "Raspberry Pi Zero W",
105:     "Raspberry Pi Zero 2 W",
106:     "Raspberry Pi 2 Model B",
107:     "Raspberry Pi 3 Model B",
108:     "Raspberry Pi 3 Model A+",
109:     "Raspberry Pi 3 Model B+",
110:     "Compute Module",
111:     "Compute Module 3",
112:     "Compute Module 3+",
113:     "Raspberry Pi 4 Model B",
114:     "Raspberry Pi 400",
115:     "Compute Module 4",
116:     "Compute Module 4S",
117:     "Raspberry Pi 5 Model B",
118:     "Unknown",
119: };
120: 
121: /// <summary>
122: /// Mapping from SoC type to SoC name
123: ///
124: /// Must match SoCType one to one
125: /// </summary>
126: static const char* s_SoCName[] =
127: {
128:     "BCM2835",
129:     "BCM2836",
130:     "BCM2837",
131:     "BCM2711",
132:     "BCM2712",
133:     "Unknown",
134: };
135: /* clang-format on */
136: 
137: /// <summary>
138: /// Constructs a default MachineInfo instance (a singleton). Note that the constructor is private, so GetMachineInfo() is needed to instantiate the MachineInfo.
139: /// </summary>
140: MachineInfo::MachineInfo()
141:     : m_initialized{}
142:     , m_memoryAccess{GetMemoryAccess()}
143:     , m_revisionRaw{}
144:     , m_boardModel{BoardModel::Unknown}
145:     , m_boardModelMajor{}
146:     , m_boardModelRevision{}
147:     , m_SoCType{SoCType::Unknown}
148:     , m_ramSize{}
149:     , m_boardSerial{}
150:     , m_fwRevision{}
151:     , m_macAddress{}
152:     , m_armBaseAddress{}
153:     , m_armMemorySize{}
154:     , m_vcBaseAddress{}
155:     , m_vcMemorySize{}
156: {
157: }
158: 
159: /// <summary>
160: /// Constructs a specialized MachineInfo instance which injects a custom IMemoryAccess instance. This is intended for testing.
161: /// </summary>
162: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
163: MachineInfo::MachineInfo(IMemoryAccess& memoryAccess)
164:     : m_initialized{}
165:     , m_memoryAccess{memoryAccess}
166:     , m_revisionRaw{}
167:     , m_boardModel{BoardModel::Unknown}
168:     , m_boardModelMajor{}
169:     , m_boardModelRevision{}
170:     , m_SoCType{SoCType::Unknown}
171:     , m_ramSize{}
172:     , m_boardSerial{}
173:     , m_fwRevision{}
174:     , m_macAddress{}
175:     , m_armBaseAddress{}
176:     , m_armMemorySize{}
177:     , m_vcBaseAddress{}
178:     , m_vcMemorySize{}
179: {
180: }
181: 
182: /// <summary>
183: /// Initialize a MachineInfo instance
184: ///
185: /// The member variable m_initialized is used to guard against multiple initialization.
186: /// The initialization will determine information concerning the board as well as memory and division between ARM and VideoCore, and store this for later retrieval
187: /// </summary>
188: /// <returns>Returns true on success, false on failure</returns>
189: bool MachineInfo::Initialize()
190: {
191:     if (!m_initialized)
192:     {
193:         Mailbox mailbox{MailboxChannel::ARM_MAILBOX_CH_PROP_OUT};
194:         RPIProperties properties(mailbox);
195: 
196:         if (!properties.GetFirmwareRevision(m_fwRevision))
197:         {
198:             GetConsole().Write("Failed to retrieve FW revision\n");
199:         }
200: 
201:         if (!properties.GetBoardRevision(m_revisionRaw))
202:         {
203:             GetConsole().Write("Failed to retrieve board revision\n");
204:         }
205: 
206:         if (!properties.GetBoardSerial(m_boardSerial))
207:         {
208:             GetConsole().Write("Failed to retrieve board serial number\n");
209:         }
210: 
211:         if (!properties.GetBoardMACAddress(m_macAddress))
212:         {
213:             GetConsole().Write("Failed to retrieve MAC address\n");
214:         }
215: 
216:         if (!properties.GetARMMemory(m_armBaseAddress, m_armMemorySize))
217:         {
218:             GetConsole().Write("Failed to retrieve ARM memory info\n");
219:         }
220: 
221:         if (!properties.GetVCMemory(m_vcBaseAddress, m_vcMemorySize))
222:         {
223:             GetConsole().Write("Failed to retrieve VC memory info\n");
224:         }
225: 
226:         unsigned type = (static_cast<unsigned>(m_revisionRaw) >> 4) & 0xFF;
227:         size_t index{};
228:         size_t count = sizeof(s_boardInfo) / sizeof(s_boardInfo[0]);
229:         for (index = 0; index < count; ++index)
230:         {
231:             if (s_boardInfo[index].type == type)
232:             {
233:                 break;
234:             }
235:         }
236: 
237:         if (index >= count)
238:         {
239:             return false;
240:         }
241: 
242:         m_boardModel = s_boardInfo[index].model;
243:         m_boardModelMajor = s_boardInfo[index].majorRevision;
244:         m_boardModelRevision = (static_cast<unsigned>(m_revisionRaw) & 0xF) + 1;
245:         m_SoCType = static_cast<SoCType>((static_cast<unsigned>(m_revisionRaw) >> 12) & 0xF);
246:         m_ramSize = 256 << ((static_cast<unsigned>(m_revisionRaw) >> 20) & 7);
247:         if (m_boardModel == BoardModel::RaspberryPi_BRelease2MB512 && m_ramSize == 256)
248:         {
249:             m_boardModel = (m_boardModelRevision == 1) ? BoardModel::RaspberryPi_BRelease1MB256 : BoardModel::RaspberryPi_BRelease2MB256;
250:         }
251:         if (static_cast<unsigned>(m_SoCType) >= static_cast<unsigned>(SoCType::Unknown))
252:         {
253:             m_SoCType = SoCType::Unknown;
254:         }
255: 
256:         m_initialized = true;
257:     }
258:     return true;
259: }
260: 
261: /// <summary>
262: /// Returns board model
263: /// </summary>
264: /// <returns>Board model</returns>
265: BoardModel MachineInfo::GetModel()
266: {
267:     return m_boardModel;
268: }
269: 
270: /// <summary>
271: /// Returns board name
272: /// </summary>
273: /// <returns>Board name</returns>
274: const char* MachineInfo::GetName()
275: {
276:     return m_boardName[static_cast<size_t>(m_boardModel)];
277: }
278: 
279: /// <summary>
280: /// Returns the major board model number
281: /// </summary>
282: /// <returns>Major board model number</returns>
283: uint32 MachineInfo::GetModelMajor()
284: {
285:     return m_boardModelMajor;
286: }
287: 
288: /// <summary>
289: /// Returns the board model revision
290: /// </summary>
291: /// <returns>Board model revision</returns>
292: uint32 MachineInfo::GetModelRevision()
293: {
294:     return m_boardModelRevision;
295: }
296: 
297: /// <summary>
298: /// Returns the SoC type
299: /// </summary>
300: /// <returns>SoC type</returns>
301: SoCType MachineInfo::GetSoCType()
302: {
303:     return m_SoCType;
304: }
305: 
306: /// <summary>
307: /// Returns the SoC name
308: /// </summary>
309: /// <returns>SoC name</returns>
310: const char* MachineInfo::GetSoCName()
311: {
312:     return s_SoCName[static_cast<size_t>(m_SoCType)];
313: }
314: 
315: /// <summary>
316: /// Returns the amount of RAM on board in Mb
317: /// </summary>
318: /// <returns>RAM size in Mb</returns>
319: uint32 MachineInfo::GetRAMSize()
320: {
321:     return m_ramSize;
322: }
323: 
324: /// <summary>
325: /// Returns the board serial number
326: /// </summary>
327: /// <returns>Board serial number</returns>
328: uint64 MachineInfo::GetSerial()
329: {
330:     return m_boardSerial;
331: }
332: 
333: /// <summary>
334: /// Returns the board FW revision
335: /// </summary>
336: /// <returns>Board FW revision</returns>
337: uint32 MachineInfo::GetFWRevision()
338: {
339:     return m_fwRevision;
340: }
341: 
342: /// <summary>
343: /// Returns the raw board revision
344: /// </summary>
345: /// <returns>Raw board revision</returns>
346: BoardRevision MachineInfo::GetBoardRevision()
347: {
348:     return m_revisionRaw;
349: }
350: 
351: /// <summary>
352: /// Returns the MAC address for the network interface
353: /// </summary>
354: /// <param name="macAddress">Network MAC address</param>
355: void MachineInfo::GetMACAddress(uint8 macAddress[6])
356: {
357:     memcpy(macAddress, m_macAddress, sizeof(m_macAddress));
358: }
359: 
360: /// <summary>
361: /// Returns the ARM memory base address
362: /// </summary>
363: /// <returns>ARM memory base address</returns>
364: uint32 MachineInfo::GetARMMemoryBaseAddress()
365: {
366:     return m_armBaseAddress;
367: }
368: 
369: /// <summary>
370: /// Returns the amount of memory assigned to the ARM cores in bytes
371: /// </summary>
372: /// <returns>Amount of memory assigned to the ARM cores in bytes</returns>
373: uint32 MachineInfo::GetARMMemorySize()
374: {
375:     return m_armMemorySize;
376: }
377: 
378: /// <summary>
379: /// Returns the VideoCore memory base address
380: /// </summary>
381: /// <returns>VideoCore memory base address</returns>
382: uint32 MachineInfo::GetVCMemoryBaseAddress()
383: {
384:     return m_vcBaseAddress;
385: }
386: 
387: /// <summary>
388: /// Returns the amount of memory assigned to the VideoCore in bytes
389: /// </summary>
390: /// <returns>Amount of memory assigned to the VideoCore in bytes</returns>
391: uint32 MachineInfo::GetVCMemorySize()
392: {
393:     return m_vcMemorySize;
394: }
395: 
396: /// <summary>
397: /// Determine and return the clock rate for a specific clock, or return an estimate
398: /// </summary>
399: /// <param name="clockID"></param>
400: /// <returns></returns>
401: unsigned MachineInfo::GetClockRate(ClockID clockID) const
402: {
403:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
404:     RPIProperties properties(mailbox);
405:     uint32 clockRate{};
406:     if (properties.GetClockRate(clockID, clockRate))
407:         return clockRate;
408:     if (properties.GetMeasuredClockRate(clockID, clockRate))
409:         return clockRate;
410: 
411:     // if clock rate can not be requested, use a default rate
412:     unsigned result = 0;
413: 
414:     switch (clockID)
415:     {
416:     case ClockID::EMMC:
417:     case ClockID::EMMC2:
418:         result = 100000000;
419:         break;
420: 
421:     case ClockID::UART:
422:         result = 48000000;
423:         break;
424: 
425:     case ClockID::CORE:
426:         result = 300000000; /// \todo Check this
427:         break;
428: 
429:     case ClockID::PIXEL_BVB:
430:         break;
431: 
432:     default:
433:         assert(0);
434:         break;
435:     }
436: 
437:     return result;
438: }
439: 
440: /// <summary>
441: /// Create the singleton MachineInfo instance if needed, initialize it, and return a reference
442: /// </summary>
443: /// <returns>Singleton MachineInfo reference</returns>
444: MachineInfo& GetMachineInfo()
445: {
446:     static MachineInfo machineInfo;
447:     machineInfo.Initialize();
448:     return machineInfo;
449: }
450: 
451: } // namespace baremetal
```

- Line 53-64: We declare a structure `BoardInfo` to hold definitions for the different board models.
This is used to map the raw board revision code to a board model
- Line 66-87: We define an array of `BoardInfo` structure for the mapping to board models
- Line 89-119: We define an array of strings to map board models to names
- Line 121-134: We define an array of strings to map SoC types to names
- Line 137-157: We implement the default constructor
- Line 159-180: We implement the constructor taking a `MemoryAccess` instance
- Line 182-259: We implement the `Initialize()` method.
This does most of the work
  - Line 193-194: We set up the mailbox
  - Line 196-199: We request the firmware revision number
  - Line 201-204: We request the board revision number
  - Line 206-209: We request the board serial number
  - Line 211-214: We request the MAC address
  - Line 216-219: We request the ARM assigned memory information
  - Line 221-224: We request the VideoCore assigned memory information
  - Line 226-235: We do some trickery to extract a type code (bits 4-11 of the revision number) and look up the board information
  - Line 242-243: We set the board model and board major revision number from the board information found
  - Line 244: We extract the board revision number (bits 0 to 3)
  - Line 245: We extract the SoC type (bit 12 to 15)
  - Line 246: We extract the RAM size (bits 20-22)
  - Line 247-250: We adjust for some special cases for Raspberry Pi 1 and 2
  - Line 251-254: We check whether the SoC type is valid
- Line 261-268: We implement the `GetModel()` method which simply returns the saved board model
- Line 270-277: We implement the `GetName()` method which returns the name for the board model
- Line 279-286: We implement the `GetModelMajor()` method which returns the saved board major revision number
- Line 288-295: We implement the `GetModelRevision()` method which returns the saved board revision number
- Line 297-304: We implement the `GetSoCType()` method which returns the saved SoC type
- Line 306-313: We implement the `GetSoCName()` method which returns the name of the SoC type
- Line 315-322: We implement the `GetRAMSize()` method which returns the saved memory size
- Line 324-331: We implement the `GetSerial()` method which returns the saved board serial number
- Line 333-340: We implement the `GetFWRevision()` method which returns the saved firmware revision number
- Line 342-349: We implement the `GetBoardRevision()` method which returns the saved raw board revision code
- Line 351-358: We implement the `GetMACAddress()` method which returns the saved MAC address.
Notice that this uses the standard function `memcpy()` which we will need to add
- Line 360-367: We implement the `GetARMMemoryBaseAddress()` method which simply returns the saved ARM memory base address
- Line 369-376: We implement the `GetARMMemorySize()` method which simply returns the saved ARM memory size
- Line 378-385: We implement the `GetVCMemoryBaseAddress()` method which simply returns the saved VideoCore memory base address
- Line 387-394: We implement the `GetVCMemorySize()` method which simply returns the saved VideoCore memory size
- Line 396-438: We implement the `GetClockRate()` method, which tries to request the set clock rate,
if not available the measured clock rate, and if all fails an estimate of the clock frequency
- Line 440-449: We implement the `GetMachineInfo()` function

### Logger.cpp {#TUTORIAL_14_BOARD_INFORMATION_MACHINEINFO_LOGGERCPP}

We can now make use of information from `MachineInfo` to print where initializing the `Logger` instance.

Update the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
...
40: #include "baremetal/Logger.h"
41: 
42: #include "baremetal/Console.h"
43: #include "baremetal/Format.h"
44: #include "baremetal/MachineInfo.h"
45: #include "baremetal/System.h"
46: #include "baremetal/Timer.h"
47: #include "baremetal/Version.h"
48: #include "stdlib/Util.h"
...
81: /// <summary>
82: /// Initialize logger
83: /// </summary>
84: /// <returns>true on succes, false on failure</returns>
85: bool Logger::Initialize()
86: {
87:     if (m_isInitialized)
88:         return true;
89:     SetupVersion();
90:     m_isInitialized = true; // Stop reentrant calls from happening
91:     LOG_INFO(BAREMETAL_NAME " %s started on %s (AArch64) using %s SoC", BAREMETAL_VERSION_STRING, GetMachineInfo().GetName(), GetMachineInfo().GetSoCName());
92: 
93:     return true;
94: }
...
```

- Line 44: We need to include `MachineInfo.h`
- Line 91: We will now using the methods `GetName()` and `GetSoCName()` from `MachineInfo` to print the board name and the SoC name.

### Configuring, building and debugging {#TUTORIAL_14_BOARD_INFORMATION_MACHINEINFO_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now print the message at `Logger` initialization time using the actual board and SoC name:

<img src="images/tutorial-14-logger.png" alt="Console output" width="800"/>

As you can see we now have debug info in yellow for the RPIProperties.
Although this may be handy, it also pollutes the console quite a bit.
So let's change the log / trace filter level.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
...
23: int main()
24: {
25:     auto& console = GetConsole();
26: 
27:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
28:     RPIProperties properties(mailbox);
29: 
30:     uint64 serial{};
31:     if (properties.GetBoardSerial(serial))
32:     {
33:         LOG_INFO("Mailbox call succeeded");
34:         LOG_INFO("Serial: %016llx", serial);
35:     }
36:     else
37:     {
38:         LOG_ERROR("Mailbox call failed");
39:     }
40: 
41:     uint32 rate;
42:     if (properties.GetClockRate(ClockID::UART, rate))
43:     {
44:         LOG_INFO("Mailbox call succeeded");
45:         LOG_INFO("UART clock rate: %d", rate);
46:     }
47:     else
48:     {
49:         LOG_ERROR("Mailbox call failed");
50:     }
51: 
52:     LOG_INFO("Wait 5 seconds");
53:     Timer::WaitMilliSeconds(5000);
54: 
55:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
56:     char ch{};
57:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
58:     {
59:         ch = console.ReadChar();
60:         console.WriteChar(ch);
61:     }
62:     if (ch == 'p')
63:     {
64:         SetAssertionCallback(MyHandler);
65:         assert(false);
66:         ResetAssertionCallback();
67:         assert(false);
68:     }
69: 
70:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
71: }
```

- Line 26: We remove the Hello World print

Update the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
...
300: /// <summary>
301: /// Construct the singleton logger and initializat it if needed, and return a reference to the instance
302: /// </summary>
303: /// <returns>Reference to the singleton logger instance</returns>
304: Logger& baremetal::GetLogger()
305: {
306:     static Logger s_logger(LogSeverity::Info, &GetTimer());
307:     s_logger.Initialize();
308:     return s_logger;
309: }
```

- Line 306: We set the default log / trace level to `LogSeverity::Info`

<img src="images/tutorial-14-logger-filtered.png" alt="Console output" width="800"/>

Next: [15-memory-management](15-memory-management.md)

