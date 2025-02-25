# Tutorial 13: Board information {#TUTORIAL_13_BOARD_INFORMATION}

@tableofcontents

## New tutorial setup {#TUTORIAL_13_BOARD_INFORMATION_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/13-board-information`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_13_BOARD_INFORMATION_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-13.a`
- a library `output/Debug/lib/stdlib-13.a`
- an application `output/Debug/bin/13-board-information.elf`
- an image in `deploy/Debug/13-board-information-image`

## Retrieving board information {#TUTORIAL_13_BOARD_INFORMATION_RETRIEVING_BOARD_INFORMATION}

In order to start with memory management, we need to know how much memory is available.
Next to this, we wish to have insight in the actual hardware we're running on.

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
66: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
67: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
68: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
69: option(BAREMETAL_TRACE_DEBUG "Enable debug tracing output" OFF)
...
92: if (BAREMETAL_COLOR_LOGGING)
93:     set(BAREMETAL_COLOR_OUTPUT 1)
94: else ()
95:     set(BAREMETAL_COLOR_OUTPUT 0)
96: endif()
97: if (BAREMETAL_TRACE_DEBUG)
98:     set(BAREMETAL_DEBUG_TRACING 1)
99: else ()
100:     set(BAREMETAL_DEBUG_TRACING 0)
101: endif()
102: set(BAREMETAL_LOAD_ADDRESS 0x80000)
103: 
104: set(DEFINES_C
105:     PLATFORM_BAREMETAL
106:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
107:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
108:     BAREMETAL_DEBUG_TRACING=${BAREMETAL_DEBUG_TRACING}
109:     BAREMETAL_MAJOR=${VERSION_MAJOR}
110:     BAREMETAL_MINOR=${VERSION_MINOR}
111:     BAREMETAL_LEVEL=${VERSION_LEVEL}
112:     BAREMETAL_BUILD=${VERSION_BUILD}
113:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
114:     )
...
276: message(STATUS "Baremetal settings:")
277: message(STATUS "-- RPI target:                      ${BAREMETAL_RPI_TARGET}")
278: message(STATUS "-- Architecture options:            ${BAREMETAL_ARCH_CPU_OPTIONS}")
279: message(STATUS "-- Kernel name:                     ${BAREMETAL_TARGET_KERNEL}")
280: message(STATUS "-- Kernel load address:             ${BAREMETAL_LOAD_ADDRESS}")
281: message(STATUS "-- Debug output to UART0:           ${BAREMETAL_CONSOLE_UART0}")
282: message(STATUS "-- Debug output to UART1:           ${BAREMETAL_CONSOLE_UART1}")
283: message(STATUS "-- Color log output:                ${BAREMETAL_COLOR_LOGGING}")
284: message(STATUS "-- Debug tracing output:            ${BAREMETAL_TRACE_DEBUG}")
285: message(STATUS "-- Version major:                   ${VERSION_MAJOR}")
286: message(STATUS "-- Version minor:                   ${VERSION_MINOR}")
287: message(STATUS "-- Version level:                   ${VERSION_LEVEL}")
288: message(STATUS "-- Version build:                   ${VERSION_BUILD}")
289: message(STATUS "-- Version composed:                ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}")
...
```

- Line 69: We add the variable `BAREMETAL_TRACE_DEBUG` which will enable debug trace output. It is set to `OFF` by default.
- Line 97-101: We set variable `BAREMETAL_DEBUG_TRACING` to 1 if `BAREMETAL_TRACE_DEBUG` is `ON`, and 0 otherwise
- Line 108: We set the compiler definition `BAREMETAL_DEBUG_TRACING` to the value of the `BAREMETAL_DEBUG_TRACING` variable
- Line 284: We print the value of the `BAREMETAL_TRACE_DEBUG` variable

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
42: #include <stdlib/Util.h>
43: #include <baremetal/BCMRegisters.h>
44: #include <baremetal/Logger.h>
45: #include <baremetal/RPIProperties.h>
46: #include <baremetal/RPIPropertiesInterface.h>
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
91:     uint8    address[6];
92:     /// Padding to align to 4 bytes
93:     uint8    padding[2];
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
104:     uint32   baseAddress;
105:     /// Size in bytes
106:     uint32   size;
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
125:     PropertyTagSimple         tag{};
126:     RPIPropertiesInterface interface(m_mailbox);
127: 
128:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_FIRMWARE_REVISION, &tag, sizeof(tag));
129: 
130: #if BAREMETAL_DEBUG_TRACING
131:     LOG_DEBUG("GetFirmwareRevision");
132: 
133:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
134: #endif
135:     if (result)
136:     {
137:         revision = tag.value;
138: #if BAREMETAL_DEBUG_TRACING
139:         LOG_DEBUG("Revision: %08lx", tag.value);
140: #endif
141:     }
142: 
143:     return result;
144: }
145: 
146: /// <summary>
147: /// Retrieve Raspberry Pi board model
148: /// </summary>
149: /// <param name="model">Board model (out)</param>
150: /// <returns>Return true on success, false on failure</returns>
151: bool RPIProperties::GetBoardModel(BoardModel& model)
152: {
153:     PropertyTagSimple         tag{};
154:     RPIPropertiesInterface interface(m_mailbox);
155: 
156:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_MODEL, &tag, sizeof(tag));
157: 
158: #if BAREMETAL_DEBUG_TRACING
159:     LOG_DEBUG("GetBoardModel");
160: 
161:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
162: #endif
163:     if (result)
164:     {
165:         model = static_cast<BoardModel>(tag.value);
166: #if BAREMETAL_DEBUG_TRACING
167:         LOG_DEBUG("Model: %08lx", tag.value);
168: #endif
169:     }
170: 
171:     return result;
172: }
173: 
174: /// <summary>
175: /// Retrieve Raspberry Pi board revision
176: /// </summary>
177: /// <param name="revision">Board revision (out)</param>
178: /// <returns>Return true on success, false on failure</returns>
179: bool RPIProperties::GetBoardRevision(BoardRevision& revision)
180: {
181:     PropertyTagSimple         tag{};
182:     RPIPropertiesInterface interface(m_mailbox);
183: 
184:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_REVISION, &tag, sizeof(tag));
185: 
186: #if BAREMETAL_DEBUG_TRACING
187:     LOG_DEBUG("GetBoardRevision");
188: 
189:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
190: #endif
191:     if (result)
192:     {
193:         revision = static_cast<BoardRevision>(tag.value);
194: #if BAREMETAL_DEBUG_TRACING
195:         LOG_DEBUG("Revision: %08lx", tag.value);
196: #endif
197:     }
198: 
199:     return result;
200: }
201: 
202: /// <summary>
203: /// Retrieve network MAC address
204: /// </summary>
205: /// <param name="address">MAC address (out)</param>
206: /// <returns>Return true on success, false on failure</returns>
207: bool RPIProperties::GetBoardMACAddress(uint8 address[6])
208: {
209:     PropertyTagMACAddress     tag{};
210:     RPIPropertiesInterface interface(m_mailbox);
211: 
212:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_MAC_ADDRESS, &tag, sizeof(tag));
213: 
214: #if BAREMETAL_DEBUG_TRACING
215:     LOG_DEBUG("GetBoardMACAddress");
216: 
217:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
218: #endif
219:     if (result)
220:     {
221:         memcpy(address, tag.address, sizeof(tag.address));
222: #if BAREMETAL_DEBUG_TRACING
223:         LOG_DEBUG("Address:");
224:         GetConsole().Write(address, sizeof(tag.address));
225: #endif
226:     }
227: 
228:     return result;
229: }
230: 
231: /// <summary>
232: /// Request board serial number
233: /// </summary>
234: /// <param name="serial">On return, set to serial number, if successful</param>
235: /// <returns>Return true on success, false on failure</returns>
236: bool RPIProperties::GetBoardSerial(uint64 &serial)
237: {
238:     PropertyTagSerial         tag{};
239:     RPIPropertiesInterface interface(m_mailbox);
240: 
241:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_SERIAL, &tag, sizeof(tag));
242: 
243: #if BAREMETAL_DEBUG_TRACING
244:     LOG_DEBUG("GetBoardSerial");
245: 
246:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
247: #endif
248:     if (result)
249:     {
250:         serial = (static_cast<uint64>(tag.serial[1]) << 32 | static_cast<uint64>(tag.serial[0]));
251: #if BAREMETAL_DEBUG_TRACING
252:         LOG_DEBUG("Serial: %016llx", serial);
253: #endif
254:     }
255: 
256:     return result;
257: }
258: 
259: /// <summary>
260: /// Retrieve ARM assigned memory base address and size
261: /// </summary>
262: /// <param name="baseAddress">ARM assigned base address (out)</param>
263: /// <param name="size">ARM assigned memory size in bytes (out)</param>
264: /// <returns>Return true on success, false on failure</returns>
265: bool RPIProperties::GetARMMemory(uint32& baseAddress, uint32& size)
266: {
267:     PropertyTagMemory         tag{};
268:     RPIPropertiesInterface interface(m_mailbox);
269: 
270:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_ARM_MEMORY, &tag, sizeof(tag));
271: 
272: #if BAREMETAL_DEBUG_TRACING
273:     LOG_DEBUG("GetARMMemory");
274: 
275:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
276: #endif
277:     if (result)
278:     {
279:         baseAddress = tag.baseAddress;
280:         size = tag.size;
281: #if BAREMETAL_DEBUG_TRACING
282:         LOG_DEBUG("Base address: %08lx", baseAddress);
283:         LOG_DEBUG("Size:         %08lx", size);
284: #endif
285:     }
286: 
287:     return result;
288: }
289: 
290: /// <summary>
291: /// Retrieve VideoCore assigned memory base address and size
292: /// </summary>
293: /// <param name="baseAddress">VideoCore assigned base address (out)</param>
294: /// <param name="size">VideoCore assigned memory size in bytes (out)</param>
295: /// <returns>Return true on success, false on failure</returns>
296: bool RPIProperties::GetVCMemory(uint32& baseAddress, uint32& size)
297: {
298:     PropertyTagMemory         tag{};
299:     RPIPropertiesInterface interface(m_mailbox);
300: 
301:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_VC_MEMORY, &tag, sizeof(tag));
302: 
303: #if BAREMETAL_DEBUG_TRACING
304:     LOG_DEBUG("GetARMMemory");
305: 
306:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
307: #endif
308:     if (result)
309:     {
310:         baseAddress = tag.baseAddress;
311:         size = tag.size;
312: #if BAREMETAL_DEBUG_TRACING
313:         LOG_DEBUG("Base address: %08lx", baseAddress);
314:         LOG_DEBUG("Size:         %08lx", size);
315: #endif
316:     }
317: 
318:     return result;
319: }
320: 
321: /// <summary>
322: /// Get clock rate for specified clock
323: /// </summary>
324: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
325: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
326: /// <returns>Return true on success, false on failure</returns>
327: bool RPIProperties::GetClockRate(ClockID clockID, uint32& freqHz)
328: {
329:     PropertyTagClockRate      tag{};
330:     RPIPropertiesInterface interface(m_mailbox);
331: 
332:     tag.clockID = static_cast<uint32>(clockID);
333:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE, &tag, sizeof(tag));
334: 
335: #if BAREMETAL_DEBUG_TRACING
336:     LOG_DEBUG("GetClockRate");
337:     LOG_DEBUG("Clock ID:   %08lx", tag.clockID);
338: 
339:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
340: #endif
341:     if (result)
342:     {
343:         freqHz = tag.rate;
344: #if BAREMETAL_DEBUG_TRACING
345:         LOG_DEBUG("Rate:       %08lx", tag.rate);
346: #endif
347:     }
348: 
349:     return result;
350: }
351: 
352: /// <summary>
353: /// Get measured clock rate for specified clock
354: /// </summary>
355: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
356: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
357: /// <returns>Return true on success, false on failure</returns>
358: bool RPIProperties::GetMeasuredClockRate(ClockID clockID, uint32& freqHz)
359: {
360:     PropertyTagClockRate      tag{};
361:     RPIPropertiesInterface interface(m_mailbox);
362: 
363:     tag.clockID = static_cast<uint32>(clockID);
364:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE_MEASURED, &tag, sizeof(tag));
365: 
366: #if BAREMETAL_DEBUG_TRACING
367:     LOG_DEBUG("GetMeasuredClockRate");
368:     LOG_DEBUG("Clock ID:   %08lx", tag.clockID);
369: 
370:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
371: #endif
372:     if (result)
373:     {
374:         freqHz = tag.rate;
375: #if BAREMETAL_DEBUG_TRACING
376:         LOG_DEBUG("Rate:       %08lx", tag.rate);
377: #endif
378:     }
379: 
380:     return result;
381: }
382: 
383: /// <summary>
384: /// Set clock rate for specified clock
385: /// </summary>
386: /// <param name="clockID">ID of clock to be set</param>
387: /// <param name="freqHz">Clock frequencyy in Hz</param>
388: /// <param name="skipTurbo">When true, do not switch to turbo setting if ARM clock is above default.
389: /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.</param>
390: /// <returns>Return true on success, false on failure</returns>
391: bool RPIProperties::SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo)
392: {
393:     PropertyTagClockRate      tag{};
394:     RPIPropertiesInterface interface(m_mailbox);
395: 
396:     tag.clockID   = static_cast<uint32>(clockID);
397:     tag.rate      = freqHz;
398:     tag.skipTurbo = skipTurbo;
399:     auto result   = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));
400: 
401:     // Do not write to console here, as this call is needed to set up the console
402: 
403:     return result;
404: }
405: 
406: } // namespace baremetal
```

- Line 44: We need to include the header for the `Logger` class
- Line 45: We also need to include the header for the `RPIProperties` class
- Line 52: We define the log module name as we will be logging log macros
- Line 86-94: We declare the tag structure to hold the MAC addres `PropertyMACTagAddress`
- Line 99-107: We declare the tag structure to memory information `PropertyTagMemory`
- Line 123-144: We implement the member function `GetFirmwareRevision()`.
This uses the `PropertyTagSimple` structure to retrieve the firmware revision, which is a simple 32 bit unsigned integer.
Note that debug statements are added, which print more information in case `BAREMETAL_DEBUG_TRACING` is defined
- Line 141-172: We implement the member function `GetBoardModel()`.
Again, this uses the `PropertyTagSimple` structure to retrieve the firmware revision, which is a simple 32 bit unsigned integer
- Line 179-200: We implement the member function `GetBoardRevision()`.
Again, this uses the `PropertyTagSimple` structure to retrieve the firmware revision, which is a simple 32 bit unsigned integer
- Line 207-229: We implement the member function `GetBoardMACAddress()`.
As a MAC address consists of 6 8 bit unsigned integers, we use the `PropertyTagMACAddress` structure to retrieve the MAC address
- Line 236-257: We add tracing information to the member function `GetBoardSerial()`
- Line 265-288: We implement the member function `GetARMMemory()`.
We use the `PropertyTagMemory` structure to retrieve the base address and size of the memory allocated to the ARM cores
- Line 296-319: We implement the member function `GetVCMemory()`.
We use the `PropertyTagMemory` structure to retrieve the base address and size of the memory allocated to the VC memory
- Line 327-350: We implement the member function `GetClockRate()`.
Note that this re-uses the property structure `PropertyTagClockRate`
- Line 358-381: We implement the member function `GetMeasuredClockRate()`.
Note that this re-uses the property structure `PropertyTagClockRate`
- Line 391-404: Note that we do not add tracing to the method `SetClockRate()`.
This is because we need to set the clock rate in order to set up UART0

### MachineInfo.h {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_MACHINEINFOH}

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
42: #include <baremetal/RPIProperties.h>
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
81:     friend MachineInfo &GetMachineInfo();
82: 
83: private:
84:     /// @brief Flags if device was initialized. Used to guard against multiple initialization
85:     bool          m_initialized;
86:     /// @brief Reference to a IMemoryAccess instantiation, injected at construction time, for e.g. testing purposes.
87:     IMemoryAccess& m_memoryAccess;
88:     /// @brief Raw revision code retrieved through the mailbox
89:     BoardRevision m_revisionRaw;
90:     /// @brief Board model determined from the raw revision code
91:     BoardModel    m_boardModel;
92:     /// @brief Board model major number determined from the raw revision code
93:     uint32        m_boardModelMajor;
94:     /// @brief Board model revision number determined from the raw revision code
95:     uint32        m_boardModelRevision;
96:     /// @brief Board SoC type determined from the raw revision code
97:     SoCType       m_SoCType;
98:     /// @brief Amount of physical RAM determined from the raw revision code (in Mb)
99:     uint32        m_ramSize;
100:     /// @brief Board serial number retrieved through the mailbox
101:     uint64        m_boardSerial;
102:     /// @brief Board FW revision number retrieved through the mailbox
103:     uint32        m_fwRevision;
104:     /// @brief Ethernet MAC address retrieved through the mailbox
105:     uint8         m_macAddress[6];
106:     /// @brief ARM assigned memory base address retrieved through the mailbox
107:     uint32        m_armBaseAddress;
108:     /// @brief ARM assigned memory size retrieved through the mailbox
109:     uint32        m_armMemorySize;
110:     /// @brief VideoCore assigned memory base address retrieved through the mailbox
111:     uint32        m_vcBaseAddress;
112:     /// @brief VideoCore assigned memory size retrieved through the mailbox
113:     uint32        m_vcMemorySize;
114: 
115:     MachineInfo();
116: 
117: public:
118:     MachineInfo(IMemoryAccess& memoryAccess);
119:     bool          Initialize();
120: 
121:     BoardModel    GetModel();
122:     const char   *GetName();
123:     uint32        GetModelMajor();
124:     uint32        GetModelRevision();
125:     SoCType       GetSoCType();
126:     const char   *GetSoCName();
127:     uint32        GetRAMSize();
128:     uint64        GetSerial();
129:     uint32        GetFWRevision();
130:     void          GetMACAddress(uint8 macAddress[6]);
131:     uint32        GetARMMemoryBaseAddress();
132:     uint32        GetARMMemorySize();
133:     uint32        GetVCMemoryBaseAddress();
134:     uint32        GetVCMemorySize();
135:     unsigned      GetClockRate(ClockID clockID) const; // See RPIPropertiesInterface (PROPTAG_GET_CLOCK_RATE)
136: 
137:     BoardRevision GetBoardRevision();
138: };
139: 
140: MachineInfo &GetMachineInfo();
141: 
142: } // namespace baremetal
```

- Line 54-68: We declare an enum to represent the type of SoC (System-on-Chip) on the board
- Line 75-138: We declare the class `MachineInfo`
  - Line 81: We declare the friend function `GetMachineInfo()` which creates, initializes and returns a reference to the singleton `MachineInfo` instance
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
  - Line 132: We declare the method `GetARMMemorySize()` which returns the ARM assigned memory size in bytes (up to the 1 Gb border, memory above 1 Gb is not specified here)
  - Line 133: We declare the method `GetVCMemoryBaseAddress()` which returns the VideoCore assigned memory base address
  - Line 134: We declare the method `GetVCMemorySize()` which returns the VideoCore assigned memory size in bytes
  - Line 135: We declare the method `GetClockRate()` which returns the clock rate for the specified clock ID
  - Line 137: We declare the method `GetBoardRevision()` which returns the raw board revision
- Line 140: We declare the friend function `GetMachineInfo()`

### MachineInfo.cpp {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_MACHINEINFOCPP}

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
40: #include <baremetal/MachineInfo.h>
41: 
42: #include <stdlib/Util.h>
43: #include <baremetal/Assert.h>
44: #include <baremetal/Console.h>
45: #include <baremetal/Mailbox.h>
46: #include <baremetal/MemoryAccess.h>
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
59:     unsigned   type;
60:     /// @brief Board model
61:     BoardModel model;
62:     /// @brief Board major revision number
63:     unsigned   majorRevision;
64: };
65: 
66: /// @brief Mapping from raw board revision to board model and major revision number
67: static BoardInfo   s_boardInfo[]{
68:     {0, BoardModel::RaspberryPi_A, 1},
69:     {1, BoardModel::RaspberryPi_BRelease2MB512, 1}, // can be other revision
70:     {2, BoardModel::RaspberryPi_APlus, 1},
71:     {3, BoardModel::RaspberryPi_BPlus, 1},
72:     {4, BoardModel::RaspberryPi_2B, 2},
73:     {6, BoardModel::RaspberryPi_CM, 1},
74:     {8, BoardModel::RaspberryPi_3B, 3},
75:     {9, BoardModel::RaspberryPi_Zero, 1},
76:     {10, BoardModel::RaspberryPi_CM3, 3},
77:     {12, BoardModel::RaspberryPi_ZeroW, 1},
78:     {13, BoardModel::RaspberryPi_3BPlus, 3},
79:     {14, BoardModel::RaspberryPi_3APlus, 3},
80:     {16, BoardModel::RaspberryPi_CM3Plus, 3},
81:     {17, BoardModel::RaspberryPi_4B, 4},
82:     {18, BoardModel::RaspberryPi_Zero2W, 3},
83:     {19, BoardModel::RaspberryPi_400, 4},
84:     {20, BoardModel::RaspberryPi_CM4, 4},
85:     {21, BoardModel::RaspberryPi_CM4S, 4},
86:     {99, BoardModel::RaspberryPi_5B, 4}
87: };
88: 
89: /// <summary>
90: /// Mapping from BoardModel to board name
91: ///
92: /// Must match BoardModel one to one
93: /// </summary>
94: static const char *m_boardName[] =
95: {
96:     "Raspberry Pi Model A",
97:     "Raspberry Pi Model B R1",
98:     "Raspberry Pi Model B R2",
99:     "Raspberry Pi Model B R2",
100:     "Raspberry Pi Model A+",
101:     "Raspberry Pi Model B+",
102:     "Raspberry Pi Zero",
103:     "Raspberry Pi Zero W",
104:     "Raspberry Pi Zero 2 W",
105:     "Raspberry Pi 2 Model B",
106:     "Raspberry Pi 3 Model B",
107:     "Raspberry Pi 3 Model A+",
108:     "Raspberry Pi 3 Model B+",
109:     "Compute Module",
110:     "Compute Module 3",
111:     "Compute Module 3+",
112:     "Raspberry Pi 4 Model B",
113:     "Raspberry Pi 400",
114:     "Compute Module 4",
115:     "Compute Module 4S",
116:     "Raspberry Pi 5 Model B",
117:     "Unknown"
118: };
119: 
120: /// <summary>
121: /// Mapping from SoC type to SoC name
122: ///
123: /// Must match SoCType one to one
124: /// </summary>
125: static const char *s_SoCName[] =
126: {
127:     "BCM2835",
128:     "BCM2836",
129:     "BCM2837",
130:     "BCM2711",
131:     "BCM2712",
132:     "Unknown"
133: };
134: 
135: /// <summary>
136: /// Constructs a default MachineInfo instance (a singleton). Note that the constructor is private, so GetMachineInfo() is needed to instantiate the MachineInfo.
137: /// </summary>
138: MachineInfo::MachineInfo()
139:     : m_initialized{}
140:     , m_memoryAccess{ GetMemoryAccess() }
141:     , m_revisionRaw{}
142:     , m_boardModel{BoardModel::Unknown}
143:     , m_boardModelMajor{}
144:     , m_boardModelRevision{}
145:     , m_SoCType{SoCType::Unknown}
146:     , m_ramSize{}
147:     , m_boardSerial{}
148:     , m_fwRevision{}
149:     , m_macAddress{}
150:     , m_armBaseAddress{}
151:     , m_armMemorySize{}
152:     , m_vcBaseAddress{}
153:     , m_vcMemorySize{}
154: {
155: }
156: 
157: /// <summary>
158: /// Constructs a specialized MachineInfo instance which injects a custom IMemoryAccess instance. This is intended for testing.
159: /// </summary>
160: /// <param name="memoryAccess">Injected IMemoryAccess instance for testing</param>
161: MachineInfo::MachineInfo(IMemoryAccess& memoryAccess)
162:     : m_initialized{}
163:     , m_memoryAccess{ memoryAccess }
164:     , m_revisionRaw{}
165:     , m_boardModel{ BoardModel::Unknown }
166:     , m_boardModelMajor{}
167:     , m_boardModelRevision{}
168:     , m_SoCType{ SoCType::Unknown }
169:     , m_ramSize{}
170:     , m_boardSerial{}
171:     , m_fwRevision{}
172:     , m_macAddress{}
173:     , m_armBaseAddress{}
174:     , m_armMemorySize{}
175:     , m_vcBaseAddress{}
176:     , m_vcMemorySize{}
177: {
178: }
179: 
180: /// <summary>
181: /// Initialize a MachineInfo instance
182: ///
183: /// The member variable m_initialized is used to guard against multiple initialization.
184: /// The initialization will determine information concerning the board as well as memory and division between ARM and VideoCore, and store this for later retrieval
185: /// </summary>
186: /// <returns>Returns true on success, false on failure</returns>
187: bool MachineInfo::Initialize()
188: {
189:     if (!m_initialized)
190:     {
191:         Mailbox       mailbox{MailboxChannel::ARM_MAILBOX_CH_PROP_OUT};
192:         RPIProperties properties(mailbox);
193: 
194:         if (!properties.GetFirmwareRevision(m_fwRevision))
195:         {
196:             GetConsole().Write("Failed to retrieve FW revision\n");
197:         }
198: 
199:         if (!properties.GetBoardRevision(m_revisionRaw))
200:         {
201:             GetConsole().Write("Failed to retrieve board revision\n");
202:         }
203: 
204:         if (!properties.GetBoardSerial(m_boardSerial))
205:         {
206:             GetConsole().Write("Failed to retrieve board serial number\n");
207:         }
208: 
209:         if (!properties.GetBoardMACAddress(m_macAddress))
210:         {
211:             GetConsole().Write("Failed to retrieve MAC address\n");
212:         }
213: 
214:         if (!properties.GetARMMemory(m_armBaseAddress, m_armMemorySize))
215:         {
216:             GetConsole().Write("Failed to retrieve ARM memory info\n");
217:         }
218: 
219:         if (!properties.GetVCMemory(m_vcBaseAddress, m_vcMemorySize))
220:         {
221:             GetConsole().Write("Failed to retrieve VC memory info\n");
222:         }
223: 
224:         unsigned type = (static_cast<unsigned>(m_revisionRaw) >> 4) & 0xFF;
225:         size_t   index{};
226:         size_t   count = sizeof(s_boardInfo) / sizeof(s_boardInfo[0]);
227:         for (index = 0; index < count; ++index)
228:         {
229:             if (s_boardInfo[index].type == type)
230:             {
231:                 break;
232:             }
233:         }
234: 
235:         if (index >= count)
236:         {
237:             return false;
238:         }
239: 
240:         m_boardModel         = s_boardInfo[index].model;
241:         m_boardModelMajor    = s_boardInfo[index].majorRevision;
242:         m_boardModelRevision = (static_cast<unsigned>(m_revisionRaw) & 0xF) + 1;
243:         m_SoCType            = static_cast<SoCType>((static_cast<unsigned>(m_revisionRaw) >> 12) & 0xF);
244:         m_ramSize            = 256 << ((static_cast<unsigned>(m_revisionRaw) >> 20) & 7);
245:         if (m_boardModel == BoardModel::RaspberryPi_BRelease2MB512 && m_ramSize == 256)
246:         {
247:             m_boardModel = (m_boardModelRevision == 1) ? BoardModel::RaspberryPi_BRelease1MB256 : BoardModel::RaspberryPi_BRelease2MB256;
248:         }
249:         if (static_cast<unsigned>(m_SoCType) >= static_cast<unsigned>(SoCType::Unknown))
250:         {
251:             m_SoCType = SoCType::Unknown;
252:         }
253: 
254:         m_initialized = true;
255:     }
256:     return true;
257: }
258: 
259: /// <summary>
260: /// Returns board model
261: /// </summary>
262: /// <returns>Board model</returns>
263: BoardModel MachineInfo::GetModel()
264: {
265:     return m_boardModel;
266: }
267: 
268: /// <summary>
269: /// Returns board name
270: /// </summary>
271: /// <returns>Board name</returns>
272: const char *MachineInfo::GetName()
273: {
274:     return m_boardName[static_cast<size_t>(m_boardModel)];
275: }
276: 
277: /// <summary>
278: /// Returns the major board model number
279: /// </summary>
280: /// <returns>Major board model number</returns>
281: uint32 MachineInfo::GetModelMajor()
282: {
283:     return m_boardModelMajor;
284: }
285: 
286: /// <summary>
287: /// Returns the board model revision
288: /// </summary>
289: /// <returns>Board model revision</returns>
290: uint32 MachineInfo::GetModelRevision()
291: {
292:     return m_boardModelRevision;
293: }
294: 
295: /// <summary>
296: /// Returns the SoC type
297: /// </summary>
298: /// <returns>SoC type</returns>
299: SoCType MachineInfo::GetSoCType()
300: {
301:     return m_SoCType;
302: }
303: 
304: /// <summary>
305: /// Returns the SoC name
306: /// </summary>
307: /// <returns>SoC name</returns>
308: const char *MachineInfo::GetSoCName()
309: {
310:     return s_SoCName[static_cast<size_t>(m_SoCType)];
311: }
312: 
313: /// <summary>
314: /// Returns the amount of RAM on board in Mb
315: /// </summary>
316: /// <returns>RAM size in Mb</returns>
317: uint32 MachineInfo::GetRAMSize()
318: {
319:     return m_ramSize;
320: }
321: 
322: /// <summary>
323: /// Returns the board serial number
324: /// </summary>
325: /// <returns>Board serial number</returns>
326: uint64 MachineInfo::GetSerial()
327: {
328:     return m_boardSerial;
329: }
330: 
331: /// <summary>
332: /// Returns the board FW revision
333: /// </summary>
334: /// <returns>Board FW revision</returns>
335: uint32 MachineInfo::GetFWRevision()
336: {
337:     return m_fwRevision;
338: }
339: 
340: /// <summary>
341: /// Returns the raw board revision
342: /// </summary>
343: /// <returns>Raw board revision</returns>
344: BoardRevision MachineInfo::GetBoardRevision()
345: {
346:     return m_revisionRaw;
347: }
348: 
349: /// <summary>
350: /// Returns the MAC address for the network interface
351: /// </summary>
352: /// <param name="macAddress">Network MAC address</param>
353: void MachineInfo::GetMACAddress(uint8 macAddress[6])
354: {
355:     memcpy(macAddress, m_macAddress, sizeof(m_macAddress));
356: }
357: 
358: /// <summary>
359: /// Returns the ARM memory base address
360: /// </summary>
361: /// <returns>ARM memory base address</returns>
362: uint32 MachineInfo::GetARMMemoryBaseAddress()
363: {
364:     return m_armBaseAddress;
365: }
366: 
367: /// <summary>
368: /// Returns the amount of memory assigned to the ARM cores in bytes
369: /// </summary>
370: /// <returns>Amount of memory assigned to the ARM cores in bytes</returns>
371: uint32 MachineInfo::GetARMMemorySize()
372: {
373:     return m_armMemorySize;
374: }
375: 
376: /// <summary>
377: /// Returns the VideoCore memory base address
378: /// </summary>
379: /// <returns>VideoCore memory base address</returns>
380: uint32 MachineInfo::GetVCMemoryBaseAddress()
381: {
382:     return m_vcBaseAddress;
383: }
384: 
385: /// <summary>
386: /// Returns the amount of memory assigned to the VideoCore in bytes
387: /// </summary>
388: /// <returns>Amount of memory assigned to the VideoCore in bytes</returns>
389: uint32 MachineInfo::GetVCMemorySize()
390: {
391:     return m_vcMemorySize;
392: }
393: 
394: /// <summary>
395: /// Determine and return the clock rate for a specific clock, or return an estimate
396: /// </summary>
397: /// <param name="clockID"></param>
398: /// <returns></returns>
399: unsigned MachineInfo::GetClockRate(ClockID clockID) const
400: {
401:     Mailbox       mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
402:     RPIProperties properties(mailbox);
403:     uint32        clockRate{};
404:     if (properties.GetClockRate(clockID, clockRate))
405:         return clockRate;
406:     if (properties.GetMeasuredClockRate(clockID, clockRate))
407:         return clockRate;
408: 
409:     // if clock rate can not be requested, use a default rate
410:     unsigned result = 0;
411: 
412:     switch (clockID)
413:     {
414:     case ClockID::EMMC:
415:     case ClockID::EMMC2:
416:         result = 100000000;
417:         break;
418: 
419:     case ClockID::UART:
420:         result = 48000000;
421:         break;
422: 
423:     case ClockID::CORE:
424:         result = 300000000; /// \todo Check this
425:         break;
426: 
427:     case ClockID::PIXEL_BVB:
428:         break;
429: 
430:     default:
431:         assert(0);
432:         break;
433:     }
434: 
435:     return result;
436: }
437: 
438: /// <summary>
439: /// Create the singleton MachineInfo instance if needed, initialize it, and return a reference
440: /// </summary>
441: /// <returns>Singleton MachineInfo reference</returns>
442: MachineInfo &GetMachineInfo()
443: {
444:     static MachineInfo machineInfo;
445:     machineInfo.Initialize();
446:     return machineInfo;
447: }
448: 
449: } // namespace baremetal
```

- Line 56-64: We declare a structure `BoardInfo` to hold definitions for the different board models.
This is used to map the raw board revision code to a board model
- Line 67-87: We define an array of `BoardInfo` structure for the mapping to board models
- Line 94-118: We define an array of strings to map board models to names
- Line 125-133: We define an array of strings to map SoC types to names
- Line 138-155: We implement the default constructor
- Line 161-178: We implement the constructor taking a `MemoryAccess` instance
- Line 187-257: We implement the `Initialize()` method
  - Line 191-192: We set up the mailbox
  - Line 194-197: We request the firmware revision number
  - Line 199-202: We request the board revision number
  - Line 204-207: We request the board serial number
  - Line 209-212: We request the MAC address
  - Line 214-217: We request the ARM assigned memory information
  - Line 219-222: We request the VideoCore assigned memory information
  - Line 224-233: We do some trickery to extract a type code (bits 4-11 of the revision number) and look up the board information
  - Line 240-241: We set the board model and board major revision number from the board information found
  - Line 242: We extract the board revision number (bits 0 to 3)
  - Line 243: We extract the SoC type (bit 12 to 15)
  - Line 244: We extract the RAM size (bits 20-22)
  - Line 245-248: We adjust for some special cases for Raspberry Pi 1 and 2
  - Line 249-252: We check whether the SoC type is valid
- Line 263-266: We implement the `GetModel()` method which simply returns the saved board model
- Line 272-275: We implement the `GetName()` method which returns the name for the board model
- Line 281-284: We implement the `GetModelMajor()` method which simply returns the saved board major revision number
- Line 290-293: We implement the `GetModelRevision()` method which simply returns the saved board revision number
- Line 299-302: We implement the `GetSoCType()` method which simply returns the saved SoC type
- Line 308-311: We implement the `GetSoCName()` method which returns the name of the SoC type
- Line 317-320: We implement the `GetRAMSize()` method which simply returns the saved memory size
- Line 326-329: We implement the `GetSerial()` method which simply returns the saved board serial number
- Line 335-338: We implement the `GetFWRevision()` method which simply returns the saved firmware revision number
- Line 344-347: We implement the `GetBoardRevision()` method which simply returns the saved raw board revision code
- Line 353-356: We implement the `GetMACAddress()` method which simply returns the saved MAC address.
Notice that this uses the standard function `memcpy()` which we will need to add
- Line 362-365: We implement the `GetARMMemoryBaseAddress()` method which simply returns the saved ARM memory base address
- Line 371-374: We implement the `GetARMMemorySize()` method which simply returns the saved ARM memory size
- Line 380-383: We implement the `GetVCMemoryBaseAddress()` method which simply returns the saved VideoCore memory base address
- Line 389-392: We implement the `GetVCMemorySize()` method which simply returns the saved VideoCore memory size
- Line 399-436: We implement the `GetClockRate()` method, which tries to request the set clock rate,
if not available the measured clock rate, and if all fails an estimate of the clock frequency
- Line 442-447: We implement the `GetMachineInfo()` function

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

- Line 45: We need to include `MachineInfo.h`
- Line 82: We will now using the methods `GetName()` and `GetSoCName()` from `MachineInfo` to print the board name and the SoC name.

### Update project configuration {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_UPDATE_PROJECT_CONFIGURATION}

As we added some files to the baremetal project, we need to update its CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
...
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
49:     )
50: 
51: set(PROJECT_INCLUDES_PUBLIC
52:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
53:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
54:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
55:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
77:     )
78: set(PROJECT_INCLUDES_PRIVATE )
...
```

### Configuring, building and debugging {#TUTORIAL_13_BOARD_INFORMATION_MACHINEINFO_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now print the message at `Logger` initialization time using the actual board and SoC name:

<img src="images/tutorial-13-logger.png" alt="Console output" width="800"/>

Next: [14-memory-management](14-memory-management.md)

