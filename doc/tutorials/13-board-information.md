# Tutorial 13: Board information {#TUTORIAL_13_BOARD_INFORMATION}

@tableofcontents

## New tutorial setup {#TUTORIAL_13_BOARD_INFORMATION_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/13-board-information`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_13_BOARD_INFORMATION_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-13.a`
- an application `output/Debug/bin/13-board-information.elf`
- an image in `deploy/Debug/13-board-information-image`

## Retrieving board information {#TUTORIAL_13_BOARD_INFORMATION_RETRIEVING_BOARD_INFORMATION}

In order to start with memory management, we need to know how much memory is available.
Next to this, we wish to have insight in the actual hardware, we're running on.

So let's gather some information, and print it.

## MachineInfo {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO}

We will create a class `MachineInfo` to retrieve all machine / board specific information.
Later we will update the `Logger` class to print the board type when starting up.

In order to retrieve board information and other info, we will start by extending the functionality for the class `RPIProperties`.

For this we will also extend debug information, so we'll add some variables to the root CMake file.

### root CMake file {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_ROOT_CMAKE_FILE}

We will add a variable to control debug tracing.

Update the file `CMakeLists.txt`.

```cmake
File: CMakeLists.txt
...
61: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
62: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
63: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
64: option(BAREMETAL_TRACE_DEBUG "Enable debug tracing output" OFF)
...
83: if (BAREMETAL_COLOR_LOGGING)
84:     set(BAREMETAL_COLOR_OUTPUT 1)
85: else ()
86:     set(BAREMETAL_COLOR_OUTPUT 0)
87: endif()
88: if (BAREMETAL_TRACE_DEBUG)
89:     set(BAREMETAL_DEBUG_TRACING 1)
90: else ()
91:     set(BAREMETAL_DEBUG_TRACING 0)
92: endif()
93: set(BAREMETAL_LOAD_ADDRESS 0x80000)
94:
95: set(DEFINES_C
96:     PLATFORM_BAREMETAL
97:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
98:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
99:     BAREMETAL_DEBUG_TRACING=${BAREMETAL_DEBUG_TRACING}
100:     USE_PHYSICAL_COUNTER
101:     BAREMETAL_MAJOR=${VERSION_MAJOR}
102:     BAREMETAL_MINOR=${VERSION_MINOR}
103:     BAREMETAL_LEVEL=${VERSION_LEVEL}
104:     BAREMETAL_BUILD=${VERSION_BUILD}
105:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
106:     )
...
270: message(STATUS "-- Color log output:    ${BAREMETAL_COLOR_LOGGING}")
271: message(STATUS "-- Debug tracing output:${BAREMETAL_TRACE_DEBUG}")
...
```

- Line 64: We add the variable `BAREMETAL_TRACE_DEBUG` which will enable debug trace output. It is set to `OFF` by default.
- Line 88-92: We set variable `BAREMETAL_DEBUG_TRACING` to 1 if `BAREMETAL_TRACE_DEBUG` is `ON`, and 0 otherwise
- Line 99: We set the compiler definition `BAREMETAL_DEBUG_TRACING` to the value of the `BAREMETAL_DEBUG_TRACING` variable
- Line 271: We print the value of the `BAREMETAL_TRACE_DEBUG` variable

### RPIProperties.h {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_RPIPROPERTIESH}

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
105:     RaspberryPi_BRev1        = 0x00000002,
106:     /// @brief Raspberry Pi 1 Model B Revision 1 no fuse
107:     RaspberryPi_BRev1NoFuse  = 0x00000003,
108:     /// @brief Raspberry Pi 1 Model B Revision 2 256 Mb RAM
109:     RaspberryPi_BRev2_256_1  = 0x00000004,
110:     /// @brief Raspberry Pi 1 Model B Revision 2 256 Mb RAM
111:     RaspberryPi_BRev2_256_2  = 0x00000005,
112:     /// @brief Raspberry Pi 1 Model B Revision 2 256 Mb RAM
113:     RaspberryPi_BRev2_256_3  = 0x00000006,
114:     /// @brief Raspberry Pi 1 Model A Revision 1
115:     RaspberryPi_A_1          = 0x00000007,
116:     /// @brief Raspberry Pi 1 Model A Revision 1
117:     RaspberryPi_A_2          = 0x00000008,
118:     /// @brief Raspberry Pi 1 Model A Revision 1
119:     RaspberryPi_A_3          = 0x00000009,
120:     /// @brief Raspberry Pi 1 Model B Revision 2 512 Mb RAM
121:     RaspberryPi_BRev2_512_1  = 0x0000000D,
122:     /// @brief Raspberry Pi 1 Model B Revision 2 512 Mb RAM
123:     RaspberryPi_BRev2_512_2  = 0x0000000E,
124:     /// @brief Raspberry Pi 1 Model B Revision 2 512 Mb RAM
125:     RaspberryPi_BRev2_512_3  = 0x0000000F,
126:     /// @brief Raspberry Pi 1 Model B+
127:     RaspberryPi_BPlus_1      = 0x00000010,
128:     /// @brief Raspberry Pi 1 Model B+
129:     RaspberryPi_BPlus_2      = 0x00000013,
130:     /// @brief Raspberry Pi 1 Model B+
131:     RaspberryPi_BPlus_3      = 0x00900032,
132:     /// @brief Raspberry Pi 1 Compute Module 1
133:     RaspberryPi_CM_1         = 0x00000011,
134:     /// @brief Raspberry Pi 1 Compute Module 1
135:     RaspberryPi_CM_2         = 0x00000014,
136:     /// @brief Raspberry Pi 1 Model A+ Revision 1 256 Mb RAM
137:     RaspberryPi_APlus_256    = 0x00000012,
138:     /// @brief Raspberry Pi 1 Model A+ Revision 1 512 Mb RAM
139:     RaspberryPi_APlus_512    = 0x00000015,
140:     /// @brief Raspberry Pi 2 Model B Revision 1.1.1
141:     RaspberryPi_2BRev1_1_1   = 0x00A01041,
142:     /// @brief Raspberry Pi 2 Model B Revision 1.1.2
143:     RaspberryPi_2BRev1_1_2   = 0x00A21041,
144:     /// @brief Raspberry Pi 2 Model B Revision 1.2
145:     RaspberryPi_2BRev1_2     = 0x00A22042,
146:     /// @brief Raspberry Pi Zero Revision 1.2
147:     RaspberryPi_ZeroRev1_2   = 0x00900092,
148:     /// @brief Raspberry Pi Zero Revision 1.4
149:     RaspberryPi_ZeroRev1_4   = 0x00900093,
150:     /// @brief Raspberry Pi Zero W
151:     RaspberryPi_ZeroW        = 0x009000C1,
152:     /// @brief Raspberry Pi 3 Model B Revision 1
153:     RaspberryPi_3B_1         = 0x00A02082,
154:     /// @brief Raspberry Pi 3 Model B Revision 2
155:     RaspberryPi_3B_2         = 0x00A22082,
156:     /// @brief Raspberry Pi 3 Model B+ Revision 1
157:     RaspberryPi_3BPlus       = 0x00A200D3,
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
173:     RaspberryPi_400          = 0x00C03130,
174:     /// @brief Raspberry Pi Zero Model 2 W
175:     RaspberryPi_Zero2W       = 0x00902120,
176:     /// @brief Raspberry Pi 5 Model B Revision 1, code TBD
177:     RaspberryPi_5B = 0x00000000,
178: };
179:
180: /// <summary>
181: /// Clock ID number. Used to retrieve and set the clock frequency for several clocks
182: /// </summary>
183: enum class ClockID : uint32
184: {
185:     /// @brief EMMC clock
186:     EMMC      = 1,
187:     /// @brief UART0 clock
188:     UART      = 2,
189:     /// @brief ARM processor clock
190:     ARM       = 3,
191:     /// @brief Core SoC clock
192:     CORE      = 4,
193:     /// @brief EMMC clock 2
194:     EMMC2     = 12,
195:     /// @brief Pixel clock
196:     PIXEL_BVB = 14,
197: };
198:
199: /// <summary>
200: /// Top level functionality for requests on Mailbox interface
201: /// </summary>
202: class RPIProperties
203: {
204: private:
205:     /// @brief Reference to mailbox for functions requested
206:     IMailbox &m_mailbox;
207:
208: public:
209:     explicit RPIProperties(IMailbox &mailbox);
210:
211:     bool GetFirmwareRevision(uint32& revision);
212:     bool GetBoardModel(BoardModel& model);
213:     bool GetBoardRevision(BoardRevision& revision);
214:     bool GetBoardMACAddress(uint8 address[6]);
215:     bool GetBoardSerial(uint64& serial);
216:     bool GetARMMemory(uint32& baseAddress, uint32& size);
217:     bool GetVCMemory(uint32& baseAddress, uint32& size);
218:     bool GetClockRate(ClockID clockID, uint32& freqHz);
219:     bool GetMeasuredClockRate(ClockID clockID, uint32& freqHz);
220:     bool SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo);
221: };
...
```

- Line 51-97: We add the enum `BoardModel` to define all different Raspberry Pi board models
- Line 102-178: We add the enum `BoardRevision` to define all different revision codes for Raspberry Pi boards
- Line 211: We add the method `GetFirmwareRevision()` to retrieve the FW revision of the board
- Line 212: We add the method `GetBoardModel()` to retrieve the board model
- Line 213: We add the method `GetBoardRevision()` to retrieve the board revision
- Line 214: We add the method `GetBoardMACAddress()` to retrieve the network MAC address
- Line 216: We add the method `GetARMMemory()` to retrieve the base address and size of memory allocated to the ARM cores
- Line 217: We add the method `GetVCMemory()` to retrieve the base address and size of memory allocated to the VideoCore
- Line 218: We add the method `GetClockRate()` to retrieve clock rates
- Line 219: We add the method `GetMeasuredClockRate()` to retrieve measured clock rates

### RPIProperties.cpp {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_RPIPROPERTIESCPP}

Next we'll implement the new methods for class `RPIProperties`.

Update the file `code/libraries/baremetal/src/RPIProperties.cpp`.

```cpp
File: code/libraries/baremetal/src/RPIProperties.cpp
...
66: /// <summary>
67: /// Mailbox property tag structure for requesting MAC address.
68: /// </summary>
69: struct PropertyMACAddress
70: {
71:     /// Tag ID, must be equal to PROPTAG_GET_MAC_ADDRESS.
72:     Property tag;
73:     /// MAC Address (6 bytes)
74:     uint8    address[6];
75:     /// Padding to align to 4 bytes
76:     uint8    padding[2];
77: } PACKED;
78:
79: /// <summary>
80: /// Mailbox property tag structure for requesting memory information.
81: /// </summary>
82: struct PropertyMemory
83: {
84:     /// Tag ID, must be equal to PROPTAG_GET_ARM_MEMORY or PROPTAG_GET_VC_MEMORY.
85:     Property tag;
86:     /// Base address
87:     uint32   baseAddress;
88:     /// Size in bytes
89:     uint32   size;
90: } PACKED;
91:
...
117: /// <summary>
118: /// Retrieve FW revision number
119: /// </summary>
120: /// <param name="revision">FW revision (out)</param>
121: /// <returns>Return true on success, false on failure</returns>
122: bool RPIProperties::GetFirmwareRevision(uint32& revision)
123: {
124:     PropertySimple         tag{};
125:     RPIPropertiesInterface interface(m_mailbox);
126:
127:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_FIRMWARE_REVISION, &tag, sizeof(tag));
128:
129: #if BAREMETAL_DEBUG_TRACING
130:     LOG_DEBUG("GetFirmwareRevision");
131:
132:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
133: #endif
134:     if (result)
135:     {
136:         revision = tag.value;
137: #if BAREMETAL_DEBUG_TRACING
138:         LOG_DEBUG("Revision: %08lx", tag.value);
139: #endif
140:     }
141:
142:     return result;
143: }
144:
145: /// <summary>
146: /// Retrieve Raspberry Pi board model
147: /// </summary>
148: /// <param name="model">Board model (out)</param>
149: /// <returns>Return true on success, false on failure</returns>
150: bool RPIProperties::GetBoardModel(BoardModel& model)
151: {
152:     PropertySimple         tag{};
153:     RPIPropertiesInterface interface(m_mailbox);
154:
155:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_MODEL, &tag, sizeof(tag));
156:
157: #if BAREMETAL_DEBUG_TRACING
158:     LOG_DEBUG("GetBoardModel");
159:
160:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
161: #endif
162:     if (result)
163:     {
164:         model = static_cast<BoardModel>(tag.value);
165: #if BAREMETAL_DEBUG_TRACING
166:         LOG_DEBUG("Model: %08lx", tag.value);
167: #endif
168:     }
169:
170:     return result;
171: }
172:
173: /// <summary>
174: /// Retrieve Raspberry Pi board revision
175: /// </summary>
176: /// <param name="revision">Board revision (out)</param>
177: /// <returns>Return true on success, false on failure</returns>
178: bool RPIProperties::GetBoardRevision(BoardRevision& revision)
179: {
180:     PropertySimple         tag{};
181:     RPIPropertiesInterface interface(m_mailbox);
182:
183:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_REVISION, &tag, sizeof(tag));
184:
185: #if BAREMETAL_DEBUG_TRACING
186:     LOG_DEBUG("GetBoardRevision");
187:
188:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
189: #endif
190:     if (result)
191:     {
192:         revision = static_cast<BoardRevision>(tag.value);
193: #if BAREMETAL_DEBUG_TRACING
194:         LOG_DEBUG("Revision: %08lx", tag.value);
195: #endif
196:     }
197:
198:     return result;
199: }
200:
201: /// <summary>
202: /// Retrieve network MAC address
203: /// </summary>
204: /// <param name="address">MAC address (out)</param>
205: /// <returns>Return true on success, false on failure</returns>
206: bool RPIProperties::GetBoardMACAddress(uint8 address[6])
207: {
208:     PropertyMACAddress     tag{};
209:     RPIPropertiesInterface interface(m_mailbox);
210:
211:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_MAC_ADDRESS, &tag, sizeof(tag));
212:
213: #if BAREMETAL_DEBUG_TRACING
214:     LOG_DEBUG("GetBoardMACAddress");
215:
216:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
217: #endif
218:     if (result)
219:     {
220:         memcpy(address, tag.address, sizeof(tag.address));
221: #if BAREMETAL_DEBUG_TRACING
222:         LOG_DEBUG("Address:");
223:         GetConsole().Write(address, sizeof(tag.address));
224: #endif
225:     }
226:
227:     return result;
228: }
229:
230: /// <summary>
231: /// Request board serial number
232: /// </summary>
233: /// <param name="serial">On return, set to serial number, if successful</param>
234: /// <returns>Return true on success, false on failure</returns>
235: bool RPIProperties::GetBoardSerial(uint64 &serial)
236: {
237:     PropertySerial         tag{};
238:     RPIPropertiesInterface interface(m_mailbox);
239:
240:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_SERIAL, &tag, sizeof(tag));
241:
242: #if BAREMETAL_DEBUG_TRACING
243:     LOG_DEBUG("GetBoardSerial");
244:
245:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
246: #endif
247:     if (result)
248:     {
249:         serial = (static_cast<uint64>(tag.serial[1]) << 32 | static_cast<uint64>(tag.serial[0]));
250: #if BAREMETAL_DEBUG_TRACING
251:         LOG_DEBUG("Serial: %016llx", serial);
252: #endif
253:     }
254:
255:     return result;
256: }
257:
258: /// <summary>
259: /// Retrieve ARM assigned memory base address and size
260: /// </summary>
261: /// <param name="baseAddress">ARM assigned base address (out)</param>
262: /// <param name="size">ARM assigned memory size in bytes (out)</param>
263: /// <returns>Return true on success, false on failure</returns>
264: bool RPIProperties::GetARMMemory(uint32& baseAddress, uint32& size)
265: {
266:     PropertyMemory         tag{};
267:     RPIPropertiesInterface interface(m_mailbox);
268:
269:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_ARM_MEMORY, &tag, sizeof(tag));
270:
271: #if BAREMETAL_DEBUG_TRACING
272:     LOG_DEBUG("GetARMMemory");
273:
274:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
275: #endif
276:     if (result)
277:     {
278:         baseAddress = tag.baseAddress;
279:         size = tag.size;
280: #if BAREMETAL_DEBUG_TRACING
281:         LOG_DEBUG("Base address: %08lx", baseAddress);
282:         LOG_DEBUG("Size:         %08lx", size);
283: #endif
284:     }
285:
286:     return result;
287: }
288:
289: /// <summary>
290: /// Retrieve VideoCore assigned memory base address and size
291: /// </summary>
292: /// <param name="baseAddress">VideoCore assigned base address (out)</param>
293: /// <param name="size">VideoCore assigned memory size in bytes (out)</param>
294: /// <returns>Return true on success, false on failure</returns>
295: bool RPIProperties::GetVCMemory(uint32& baseAddress, uint32& size)
296: {
297:     PropertyMemory         tag{};
298:     RPIPropertiesInterface interface(m_mailbox);
299:
300:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_VC_MEMORY, &tag, sizeof(tag));
301:
302: #if BAREMETAL_DEBUG_TRACING
303:     LOG_DEBUG("GetARMMemory");
304:
305:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
306: #endif
307:     if (result)
308:     {
309:         baseAddress = tag.baseAddress;
310:         size = tag.size;
311: #if BAREMETAL_DEBUG_TRACING
312:         LOG_DEBUG("Base address: %08lx", baseAddress);
313:         LOG_DEBUG("Size:         %08lx", size);
314: #endif
315:     }
316:
317:     return result;
318: }
319:
320: /// <summary>
321: /// Get clock rate for specified clock
322: /// </summary>
323: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
324: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
325: /// <returns>Return true on success, false on failure</returns>
326: bool RPIProperties::GetClockRate(ClockID clockID, uint32& freqHz)
327: {
328:     PropertyClockRate      tag{};
329:     RPIPropertiesInterface interface(m_mailbox);
330:
331:     tag.clockID = static_cast<uint32>(clockID);
332:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE, &tag, sizeof(tag));
333:
334: #if BAREMETAL_DEBUG_TRACING
335:     LOG_DEBUG("GetClockRate");
336:     LOG_DEBUG("Clock ID:   %08lx", tag.clockID);
337:
338:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
339: #endif
340:     if (result)
341:     {
342:         freqHz = tag.rate;
343: #if BAREMETAL_DEBUG_TRACING
344:         LOG_DEBUG("Rate:       %08lx", tag.rate);
345: #endif
346:     }
347:
348:     return result;
349: }
350:
351: /// <summary>
352: /// Get measured clock rate for specified clock
353: /// </summary>
354: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
355: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
356: /// <returns>Return true on success, false on failure</returns>
357: bool RPIProperties::GetMeasuredClockRate(ClockID clockID, uint32& freqHz)
358: {
359:     PropertyClockRate      tag{};
360:     RPIPropertiesInterface interface(m_mailbox);
361:
362:     tag.clockID = static_cast<uint32>(clockID);
363:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE_MEASURED, &tag, sizeof(tag));
364:
365: #if BAREMETAL_DEBUG_TRACING
366:     LOG_DEBUG("GetMeasuredClockRate");
367:     LOG_DEBUG("Clock ID:   %08lx", tag.clockID);
368:
369:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
370: #endif
371:     if (result)
372:     {
373:         freqHz = tag.rate;
374: #if BAREMETAL_DEBUG_TRACING
375:         LOG_DEBUG("Rate:       %08lx", tag.rate);
376: #endif
377:     }
378:
379:     return result;
380: }
381:
382: /// <summary>
383: /// Set clock rate for specified clock
384: /// </summary>
385: /// <param name="clockID">ID of clock to be set</param>
386: /// <param name="freqHz">Clock frequencyy in Hz</param>
387: /// <param name="skipTurbo">When true, do not switch to turbo setting if ARM clock is above default.
388: /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.</param>
389: /// <returns>Return true on success, false on failure</returns>
390: bool RPIProperties::SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo)
391: {
392:     PropertyClockRate      tag{};
393:     RPIPropertiesInterface interface(m_mailbox);
394:
395:     tag.clockID   = static_cast<uint32>(clockID);
396:     tag.rate      = freqHz;
397:     tag.skipTurbo = skipTurbo;
398:     auto result   = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));
399:
400:     // Do not write to console here, as this call is needed to set up the console
401:
402:     return result;
403: }
404:
405: } // namespace baremetal
...
```

- Line 69-77: We declare the tag structure to hold the MAC addres `PropertyMACAddress`
- Line 82-90: We declare the tag structure to memory information `PropertyMemory`
- Line 122-143: We implement the member function `GetFirmwareRevision()`.
Note that debug statements are added, which print more information in case `BAREMETAL_DEBUG_TRACING` is defined
- Line 150-171: We implement the member function `GetBoardModel()`
- Line 178-199: We implement the member function `GetBoardRevision()`
- Line 206-228: We implement the member function `GetBoardMACAddress()`
- Line 235-256: We add tracing information to the member function `GetBoardSerial()`
- Line 264-287: We implement the member function `GetARMMemory()`
- Line 295-318: We implement the member function `GetVCMemory()`
- Line 326-349: We implement the member function `GetClockRate()`.
Note that this re-uses the property structure `PropertyClockRate`
- Line 357-380: We implement the member function `GetMeasuredClockRate()`.
Note that this re-uses the property structure `PropertyClockRate`
- Line 390-403: Note that we do not add tracing to the method `SetClockRate()`.
This is because we need to set the clock rate in order to set up UART0

### MachineInfo.h {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_MACHINEINFOH}

Now we can add the class `MachineInfo` which uses the methods defined in `RPIProperties` to retrieve and process information on the hardware.

Create the file `code/libraries/baremetal/include/baremetal/MachineInfo.h`

```cpp
File: code/libraries/baremetal/include/baremetal/MachineInfo.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
42: #include <baremetal/Mailbox.h>
43: #include <baremetal/RPIProperties.h>
44: 
45: /// @file
46: /// Machine info retrieval
47: 
48: namespace baremetal {
49: 
50: /// <summary>
51: /// Type of SoC used. See @ref RASPBERRY_PI_BAREMETAL_DEVELOPMENT_SOC_FOR_EACH_BOARD for more information
52: /// </summary>
53: enum class SoCType
54: {
55:     /// @ brief BCM2835 as used in Raspberry Pi Model 1 boards
56:     BCM2835,
57:     /// @ brief BCM2835 as used in older Raspberry Pi Model 2 boards
58:     BCM2836,
59:     /// @ brief BCM2835 as used in newer Raspberry Pi Model 2 and Raspberry Pi Model 3 boards
60:     BCM2837,
61:     /// @ brief BCM2835 as used in Raspberry Pi Model 4 boards
62:     BCM2711,
63:     /// @ brief BCM2835 as used in Raspberry Pi Model 5 boards
64:     BCM2712,
65:     /// @brief SoC unknown / not set / invalid
66:     Unknown,
67: };
68: 
69: /// <summary>
70: /// Retrieves system info using the mailbox mechanism
71: ///
72: /// Note that this class is created as a singleton, using the GetMachineInfo() function.
73: /// </summary>
74: class MachineInfo
75: {
76:     /// <summary>
77:     /// Retrieves the singleton MachineInfo instance. It is created in the first call to this function. This is a friend function of class MachineInfo
78:     /// </summary>
79:     /// <returns>A reference to the singleton MachineInfo</returns>
80:     friend MachineInfo &GetMachineInfo();
81: 
82: private:
83:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
84:     bool          m_initialized;
85:     /// @brief Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
86:     IMemoryAccess& m_memoryAccess;
87:     /// @brief Raw revision code retrieved through the mailbox
88:     BoardRevision m_revisionRaw;
89:     /// @brief Board model determined from the raw revision code
90:     BoardModel    m_boardModel;
91:     /// @brief Board model major number determined from the raw revision code
92:     uint32        m_boardModelMajor;
93:     /// @brief Board model revision number determined from the raw revision code
94:     uint32        m_boardModelRevision;
95:     /// @brief Board SoC type determined from the raw revision code
96:     SoCType       m_SoCType;
97:     /// @brief Amount of physical RAM determined from the raw revision code (in Mb)
98:     uint32        m_ramSize;
99:     /// @brief Board serial number retrieved through the mailbox
100:     uint64        m_boardSerial;
101:     /// @brief Board FW revision number retrieved through the mailbox
102:     uint32        m_fwRevision;
103:     /// @brief Ethernet MAC address retrieved through the mailbox
104:     uint8         m_macAddress[6];
105:     /// @brief ARM assigned memory base address retrieved through the mailbox
106:     uint32        m_armBaseAddress;
107:     /// @brief ARM assigned memory size retrieved through the mailbox
108:     uint32        m_armMemorySize;
109:     /// @brief VideoCore assigned memory base address retrieved through the mailbox
110:     uint32        m_vcBaseAddress;
111:     /// @brief VideoCore assigned memory size retrieved through the mailbox
112:     uint32        m_vcMemorySize;
113: 
114:     MachineInfo();
115: 
116: public:
117:     MachineInfo(IMemoryAccess& memoryAccess);
118:     bool          Initialize();
119: 
120:     BoardModel    GetModel();
121:     const char   *GetName();
122:     uint32        GetModelMajor();
123:     uint32        GetModelRevision();
124:     SoCType       GetSoCType();
125:     const char   *GetSoCName();
126:     uint32        GetRAMSize();
127:     uint64        GetSerial();
128:     uint32        GetFWRevision();
129:     void          GetMACAddress(uint8 macAddress[6]);
130:     uint32        GetARMMemoryBaseAddress();
131:     uint32        GetARMMemorySize();
132:     uint32        GetVCMemoryBaseAddress();
133:     uint32        GetVCMemorySize();
134:     unsigned      GetClockRate(ClockID clockID) const; // See RPIPropertiesInterface (PROPTAG_GET_CLOCK_RATE)
135: 
136:     BoardRevision GetBoardRevision();
137: };
138: 
139: MachineInfo &GetMachineInfo();
140: 
141: } // namespace baremetal
```

- Line 53-67: We declare an enum to represent the type of SoC (System-on-Chip) on the board
- Line 74-137: We declare the class `MachineInfo`
  - Line 80: We declare the friend function `GetMachineInfo()` which creates, initializes and returns a reference to the singleton `MachineInfo` instance
  - Line 114: We declare the prive constructor, which is used by `GetMachineInfo()` to instantiate a default version of `MachineInfo`
  - Line 117: We declare a constructor, which injects a memory access interface for testing purposes
  - Line 118: We declare the method `Initialize()` which initialized the instance.
  This perform the bulk of the work to be done
  - Line 120: We declare the method `GetModel()` which returns the board model
  - Line 121: We declare the method `GetName()` which returns the board name
  - Line 122: We declare the method `GetModelMajor()` which returns the board major model number
  - Line 123: We declare the method `GetModelRevision()` which returns the board revision number
  - Line 124: We declare the method `GetSoCType()` which returns the board SoC type
  - Line 125: We declare the method `GetSoCName()` which returns the board SoC name
  - Line 126: We declare the method `GetRAMSize()` which returns the physical RAM size in Mb
  - Line 127: We declare the method `GetSerial()` which returns the board serial number
  - Line 128: We declare the method `GetFWRevision()` which returns the board firmware revision
  - Line 129: We declare the method `GetMACAddress()` which returns the ethernet MAC address
  - Line 130: We declare the method `GetARMMemoryBaseAddress()` which returns the ARM assign memory base address (normally 0)
  - Line 131: We declare the method `GetARMMemorySize()` which returns the ARM assigned memory size in bytes (up to 1Gb border)
  - Line 132: We declare the method `GetVCMemoryBaseAddress()` which returns the VideoCore assigned memory base address
  - Line 133: We declare the method `GetVCMemorySize()` which returns the VideoCore assigned memory size in bytes
  - Line 134: We declare the method `GetClockRate()` which returns the clock rate for the specified clock ID
  - Line 136: We declare the method `GetBoardRevision()` which returns the raw board revision
- Line 139: We declare the friend function `GetMachineInfo()`

### MachineInfo.cpp {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_MACHINEINFOCPP}

We'll implement the method for class `MachineInfo`.

Create the file `code/libraries/baremetal/src/MachineInfo.cpp`

```cpp
File: code/libraries/baremetal/src/MachineInfo.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
40: #include <baremetal/MachineInfo.h>
41: 
42: #include <baremetal/Assert.h>
43: #include <baremetal/Console.h>
44: #include <baremetal/Util.h>
45: 
46: /// @file
47: /// Machine info retrieval implementation
48: 
49: using namespace baremetal;
50: 
51: /// <summary>
52: /// Raspberry Pi board information
53: /// </summary>
54: struct BoardInfo
55: {
56:     /// @brief Board type
57:     unsigned   type;
58:     /// @brief Board model
59:     BoardModel model;
60:     /// @brief Board major revision number
61:     unsigned   majorRevision;
62: };
63: 
64: /// @brief Mapping from raw board revision to board model and major revision number
65: static BoardInfo   s_boardInfo[]{
66:     {0, BoardModel::RaspberryPi_A, 1},
67:     {1, BoardModel::RaspberryPi_BRelease2MB512, 1}, // can be other revision
68:     {2, BoardModel::RaspberryPi_APlus, 1},
69:     {3, BoardModel::RaspberryPi_BPlus, 1},
70:     {4, BoardModel::RaspberryPi_2B, 2},
71:     {6, BoardModel::RaspberryPi_CM, 1},
72:     {8, BoardModel::RaspberryPi_3B, 3},
73:     {9, BoardModel::RaspberryPi_Zero, 1},
74:     {10, BoardModel::RaspberryPi_CM3, 3},
75:     {12, BoardModel::RaspberryPi_ZeroW, 1},
76:     {13, BoardModel::RaspberryPi_3BPlus, 3},
77:     {14, BoardModel::RaspberryPi_3APlus, 3},
78:     {16, BoardModel::RaspberryPi_CM3Plus, 3},
79:     {17, BoardModel::RaspberryPi_4B, 4},
80:     {18, BoardModel::RaspberryPi_Zero2W, 3},
81:     {19, BoardModel::RaspberryPi_400, 4},
82:     {20, BoardModel::RaspberryPi_CM4, 4},
83:     {21, BoardModel::RaspberryPi_CM4S, 4},
84:     {99, BoardModel::RaspberryPi_5B, 4}
85: };
86: 
87: /// <summary>
88: /// Mapping from BoardModel to board name
89: ///
90: /// Must match BoardModel one to one
91: /// </summary>
92: static const char *m_boardName[] =
93: {
94:     "Raspberry Pi Model A",
95:     "Raspberry Pi Model B R1",
96:     "Raspberry Pi Model B R2",
97:     "Raspberry Pi Model B R2",
98:     "Raspberry Pi Model A+",
99:     "Raspberry Pi Model B+",
100:     "Raspberry Pi Zero",
101:     "Raspberry Pi Zero W",
102:     "Raspberry Pi Zero 2 W",
103:     "Raspberry Pi 2 Model B",
104:     "Raspberry Pi 3 Model B",
105:     "Raspberry Pi 3 Model A+",
106:     "Raspberry Pi 3 Model B+",
107:     "Compute Module",
108:     "Compute Module 3",
109:     "Compute Module 3+",
110:     "Raspberry Pi 4 Model B",
111:     "Raspberry Pi 400",
112:     "Compute Module 4",
113:     "Compute Module 4S",
114:     "Raspberry Pi 5 Model B",
115:     "Unknown"
116: };
117: 
118: /// <summary>
119: /// Mapping from SoC type to SoC name
120: ///
121: /// Must match SoCType one to one
122: /// </summary>
123: static const char *s_SoCName[] =
124: {
125:     "BCM2835",
126:     "BCM2836",
127:     "BCM2837",
128:     "BCM2711",
129:     "BCM2712",
130:     "Unknown"
131: };
132: 
133: /// <summary>
134: /// Constructs a default MachineInfo instance (a singleton). Note that the constructor is private, so GetMachineInfo() is needed to instantiate the MachineInfo.
135: /// </summary>
136: MachineInfo::MachineInfo()
137:     : m_initialized{}
138:     , m_memoryAccess{ GetMemoryAccess() }
139:     , m_revisionRaw{}
140:     , m_boardModel{BoardModel::Unknown}
141:     , m_boardModelMajor{}
142:     , m_boardModelRevision{}
143:     , m_SoCType{SoCType::Unknown}
144:     , m_ramSize{}
145:     , m_boardSerial{}
146:     , m_fwRevision{}
147:     , m_macAddress{}
148:     , m_armBaseAddress{}
149:     , m_armMemorySize{}
150:     , m_vcBaseAddress{}
151:     , m_vcMemorySize{}
152: {
153: }
154: 
155: /// <summary>
156: /// Constructs a specialized MachineInfo instance which injects a custom IMemoryAccess instance. This is intended for testing.
157: /// </summary>
158: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
159: MachineInfo::MachineInfo(IMemoryAccess& memoryAccess)
160:     : m_initialized{}
161:     , m_memoryAccess{ memoryAccess }
162:     , m_revisionRaw{}
163:     , m_boardModel{ BoardModel::Unknown }
164:     , m_boardModelMajor{}
165:     , m_boardModelRevision{}
166:     , m_SoCType{ SoCType::Unknown }
167:     , m_ramSize{}
168:     , m_boardSerial{}
169:     , m_fwRevision{}
170:     , m_macAddress{}
171:     , m_armBaseAddress{}
172:     , m_armMemorySize{}
173:     , m_vcBaseAddress{}
174:     , m_vcMemorySize{}
175: {
176: }
177: 
178: /// <summary>
179: /// Initialize a MachineInfo instance
180: ///
181: /// The member variable m_initialized is used to guard against multiple initialization.
182: /// The initialization will determine information concerning the board as well as memory and division between ARM and VideoCore, and store this for later retrieval
183: /// </summary>
184: /// <returns>Returns true on success, false on failure</returns>
185: bool MachineInfo::Initialize()
186: {
187:     if (!m_initialized)
188:     {
189:         Mailbox       mailbox{MailboxChannel::ARM_MAILBOX_CH_PROP_OUT};
190:         RPIProperties properties(mailbox);
191: 
192:         if (!properties.GetFirmwareRevision(m_fwRevision))
193:         {
194:             GetConsole().Write("Failed to retrieve FW revision\n");
195:         }
196: 
197:         if (!properties.GetBoardRevision(m_revisionRaw))
198:         {
199:             GetConsole().Write("Failed to retrieve board revision\n");
200:         }
201: 
202:         if (!properties.GetBoardSerial(m_boardSerial))
203:         {
204:             GetConsole().Write("Failed to retrieve board serial number\n");
205:         }
206: 
207:         if (!properties.GetBoardMACAddress(m_macAddress))
208:         {
209:             GetConsole().Write("Failed to retrieve MAC address\n");
210:         }
211:         if (!properties.GetARMMemory(m_armBaseAddress, m_armMemorySize))
212:         {
213:             GetConsole().Write("Failed to retrieve ARM memory info\n");
214:         }
215:         if (!properties.GetVCMemory(m_vcBaseAddress, m_vcMemorySize))
216:         {
217:             GetConsole().Write("Failed to retrieve VC memory info\n");
218:         }
219: 
220:         unsigned type = (static_cast<unsigned>(m_revisionRaw) >> 4) & 0xFF;
221:         size_t   index{};
222:         size_t   count = sizeof(s_boardInfo) / sizeof(s_boardInfo[0]);
223:         for (index = 0; index < count; ++index)
224:         {
225:             if (s_boardInfo[index].type == type)
226:             {
227:                 break;
228:             }
229:         }
230: 
231:         if (index >= count)
232:         {
233:             return false;
234:         }
235: 
236:         m_boardModel         = s_boardInfo[index].model;
237:         m_boardModelMajor    = s_boardInfo[index].majorRevision;
238:         m_boardModelRevision = (static_cast<unsigned>(m_revisionRaw) & 0xF) + 1;
239:         m_SoCType            = static_cast<SoCType>((static_cast<unsigned>(m_revisionRaw) >> 12) & 0xF);
240:         m_ramSize            = 256 << ((static_cast<unsigned>(m_revisionRaw) >> 20) & 7);
241:         if (m_boardModel == BoardModel::RaspberryPi_BRelease2MB512 && m_ramSize == 256)
242:         {
243:             m_boardModel = (m_boardModelRevision == 1) ? BoardModel::RaspberryPi_BRelease1MB256 : BoardModel::RaspberryPi_BRelease2MB256;
244:         }
245:         if (static_cast<unsigned>(m_SoCType) >= static_cast<unsigned>(SoCType::Unknown))
246:         {
247:             m_SoCType = SoCType::Unknown;
248:         }
249: 
250:         m_initialized = true;
251:     }
252:     return true;
253: }
254: 
255: /// <summary>
256: /// Returns board model
257: /// </summary>
258: /// <returns>Board model</returns>
259: BoardModel MachineInfo::GetModel()
260: {
261:     return m_boardModel;
262: }
263: 
264: /// <summary>
265: /// Returns board name
266: /// </summary>
267: /// <returns>Board name</returns>
268: const char *MachineInfo::GetName()
269: {
270:     return m_boardName[static_cast<size_t>(m_boardModel)];
271: }
272: 
273: /// <summary>
274: /// Returns the major board model number
275: /// </summary>
276: /// <returns>Major board model number</returns>
277: uint32 MachineInfo::GetModelMajor()
278: {
279:     return m_boardModelMajor;
280: }
281: 
282: /// <summary>
283: /// Returns the board model revision
284: /// </summary>
285: /// <returns>Board model revision</returns>
286: uint32 MachineInfo::GetModelRevision()
287: {
288:     return m_boardModelRevision;
289: }
290: 
291: /// <summary>
292: /// Returns the SoC type
293: /// </summary>
294: /// <returns>SoC type</returns>
295: SoCType MachineInfo::GetSoCType()
296: {
297:     return m_SoCType;
298: }
299: 
300: /// <summary>
301: /// Returns the SoC name
302: /// </summary>
303: /// <returns>SoC name</returns>
304: const char *MachineInfo::GetSoCName()
305: {
306:     return s_SoCName[static_cast<size_t>(m_SoCType)];
307: }
308: 
309: /// <summary>
310: /// Returns the amount of RAM on board in Mb
311: /// </summary>
312: /// <returns>RAM size in Mb</returns>
313: uint32 MachineInfo::GetRAMSize()
314: {
315:     return m_ramSize;
316: }
317: 
318: /// <summary>
319: /// Returns the board serial number
320: /// </summary>
321: /// <returns>Board serial number</returns>
322: uint64 MachineInfo::GetSerial()
323: {
324:     return m_boardSerial;
325: }
326: 
327: /// <summary>
328: /// Returns the board FW revision
329: /// </summary>
330: /// <returns>Board FW revision</returns>
331: uint32 MachineInfo::GetFWRevision()
332: {
333:     return m_fwRevision;
334: }
335: 
336: /// <summary>
337: /// Returns the raw board revision
338: /// </summary>
339: /// <returns>Raw board revision</returns>
340: BoardRevision MachineInfo::GetBoardRevision()
341: {
342:     return m_revisionRaw;
343: }
344: 
345: /// <summary>
346: /// Returns the MAC address for the network interface
347: /// </summary>
348: /// <param name="macAddress">Network MAC address</param>
349: void MachineInfo::GetMACAddress(uint8 macAddress[6])
350: {
351:     memcpy(macAddress, m_macAddress, sizeof(m_macAddress));
352: }
353: 
354: /// <summary>
355: /// Returns the ARM memory base address
356: /// </summary>
357: /// <returns>ARM memory base address</returns>
358: uint32 MachineInfo::GetARMMemoryBaseAddress()
359: {
360:     return m_armBaseAddress;
361: }
362: 
363: /// <summary>
364: /// Returns the amount of memory assigned to the ARM cores in bytes
365: /// </summary>
366: /// <returns>Amount of memory assigned to the ARM cores in bytes</returns>
367: uint32 MachineInfo::GetARMMemorySize()
368: {
369:     return m_armMemorySize;
370: }
371: 
372: /// <summary>
373: /// Returns the VideoCore memory base address
374: /// </summary>
375: /// <returns>VideoCore memory base address</returns>
376: uint32 MachineInfo::GetVCMemoryBaseAddress()
377: {
378:     return m_vcBaseAddress;
379: }
380: 
381: /// <summary>
382: /// Returns the amount of memory assigned to the VideoCore in bytes
383: /// </summary>
384: /// <returns>Amount of memory assigned to the VideoCore in bytes</returns>
385: uint32 MachineInfo::GetVCMemorySize()
386: {
387:     return m_vcMemorySize;
388: }
389: 
390: /// <summary>
391: /// Determine and return the clock rate for a specific clock, or return an estimate
392: /// </summary>
393: /// <param name="clockID"></param>
394: /// <returns></returns>
395: unsigned MachineInfo::GetClockRate(ClockID clockID) const
396: {
397:     Mailbox       mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
398:     RPIProperties properties(mailbox);
399:     uint32        clockRate{};
400:     if (properties.GetClockRate(clockID, clockRate))
401:         return clockRate;
402:     if (properties.GetMeasuredClockRate(clockID, clockRate))
403:         return clockRate;
404: 
405:     // if clock rate can not be requested, use a default rate
406:     unsigned result = 0;
407: 
408:     switch (clockID)
409:     {
410:     case ClockID::EMMC:
411:     case ClockID::EMMC2:
412:         result = 100000000;
413:         break;
414: 
415:     case ClockID::UART:
416:         result = 48000000;
417:         break;
418: 
419:     case ClockID::CORE:
420:         result = 300000000; /// \todo Check this
421:         break;
422: 
423:     case ClockID::PIXEL_BVB:
424:         break;
425: 
426:     default:
427:         assert(0);
428:         break;
429:     }
430: 
431:     return result;
432: }
433: 
434: /// <summary>
435: /// Create the singleton MachineInfo instance if needed, initialize it, and return a reference
436: /// </summary>
437: /// <returns>Singleton MachineInfo reference</returns>
438: MachineInfo &baremetal::GetMachineInfo()
439: {
440:     static MachineInfo machineInfo;
441:     machineInfo.Initialize();
442:     return machineInfo;
443: }
```

- Line 54-62: We declare a structure `BoardInfo` to hold information for the different board models.
This is used to map the raw board revision code to a board model
- Line 65-85: We define an array of `BoardInfo` structure for the mapping to board models
- Line 92-116: We define an array of strings to map board models to names
- Line 123-131: We define an array of strings to map SoC types to names
- Line 136-153: We implement the default constructor
- Line 159-176: We implement the custom constructor
- Line 185-253: We implement the `Initialize()` method
  - Line 189-190: We set up the mailbox
  - Line 192-195: We request the firmware revision number
  - Line 197-200: We request the board revision number
  - Line 202-205: We request the board serial number
  - Line 207-210: We request the MAC address
  - Line 211-214: We request the ARM assigned memory information
  - Line 215-218: We request the VideoCore assigned memory information
  - Line 220-229: We do some trickery to extract a type code (bits 4-11 of the revision number) and look up the board information
  - Line 236-237: We set the board model and board major revision number from the board information found
  - Line 238: We extract the board revision number (bits 0 to 3)
  - Line 239: We extract the SoC type (bit 12 to 15)
  - Line 240: We extract the RAM size (bits 20-22)
  - Line 241-244: We adjust for some special cases for Raspberry Pi 1 and 2
  - Line 245-248: We check whether the SoC type is valid
- Line 259-262: We implement the `GetModel()` method which simply returns the saved board model
- Line 268-271: We implement the `GetName()` method which returns the name for the board model
- Line 277-280: We implement the `GetModelMajor()` method which simply returns the saved board major revision number
- Line 286-289: We implement the `GetModelRevision()` method which simply returns the saved board revision number
- Line 295-298: We implement the `GetSoCType()` method which simply returns the saved SoC type
- Line 304-307: We implement the `GetSoCName()` method which returns the name of the SoC type
- Line 313-316: We implement the `GetRAMSize()` method which simply returns the saved memory size
- Line 322-325: We implement the `GetSerial()` method which simply returns the saved board serial number
- Line 331-334: We implement the `GetFWRevision()` method which simply returns the saved firmware revision number
- Line 340-343: We implement the `GetBoardRevision()` method which simply returns the saved raw board revision code
- Line 349-352: We implement the `GetMACAddress()` method which simply returns the saved MAC address
- Line 358-361: We implement the `GetARMMemoryBaseAddress()` method which simply returns the saved ARM memory base address
- Line 367-370: We implement the `GetARMMemorySize()` method which simply returns the saved ARM memory size
- Line 376-379: We implement the `GetVCMemoryBaseAddress()` method which simply returns the saved VideoCore memory base address
- Line 385-388: We implement the `GetVCMemorySize()` method which simply returns the saved VideoCore memory size
- Line 395-432: We implement the `GetClockRate()` method, which tries to request the set clock rate,
if not available the measured clock rate, and if all fails an estimate of the clock frequency
- Line 438-443: We implement the `GetMachineInfo()` function

### Logger.cpp {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_LOGGERCPP}

We can now make use of information from `MachineInfo` to print where initializing the `Logger` instance.

Update the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
...
42: #include <baremetal/Console.h>
43: #include <baremetal/Format.h>
44: #include <baremetal/MachineInfo.h>
45: #include <baremetal/System.h>
46: #include <baremetal/Timer.h>
47: #include <baremetal/Util.h>
48: #include <baremetal/Version.h>
...
72: /// <summary>
73: /// Initialize logger
74: /// </summary>
75: /// <returns>true on succes, false on failure</returns>
76: bool Logger::Initialize()
77: {
78:     if (m_initialized)
79:         return true;
80:     SetupVersion();
81:     m_initialized = true; // Stop reentrant calls from happening
82:     LOG_INFO(BAREMETAL_NAME " %s started on %s (AArch64) using %s SoC", BAREMETAL_VERSION_STRING, GetMachineInfo().GetName(), GetMachineInfo().GetSoCName());
83: 
84:     return true;
85: }
...
```

- Line 44: We need to include `MachineInfo.h`
- Line 82: We will now using the methods `GetName()` and `GetSoCName()` from `MachineInfo` to print the board name and the SoC name.

### Update project configuration {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
51:     )
52:
53: set(PROJECT_INCLUDES_PUBLIC
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/StdArg.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
84:     )
85: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now print the message at `Logger` initialization time using the actual board and SoC name:

<img src="images/tutorial-13-logger.png" alt="Console output" width="700"/>

Next: [14-memory-management](14-memory-management.md)

