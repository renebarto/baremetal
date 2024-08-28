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
109:     USE_PHYSICAL_COUNTER
110:     BAREMETAL_MAJOR=${VERSION_MAJOR}
111:     BAREMETAL_MINOR=${VERSION_MINOR}
112:     BAREMETAL_LEVEL=${VERSION_LEVEL}
113:     BAREMETAL_BUILD=${VERSION_BUILD}
114:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
115:     )
...
284: message(STATUS "-- Color log output:    ${BAREMETAL_COLOR_LOGGING}")
285: message(STATUS "-- Debug tracing output:${BAREMETAL_TRACE_DEBUG}")
...
```

- Line 69: We add the variable `BAREMETAL_TRACE_DEBUG` which will enable debug trace output. It is set to `OFF` by default.
- Line 97-101: We set variable `BAREMETAL_DEBUG_TRACING` to 1 if `BAREMETAL_TRACE_DEBUG` is `ON`, and 0 otherwise
- Line 108: We set the compiler definition `BAREMETAL_DEBUG_TRACING` to the value of the `BAREMETAL_DEBUG_TRACING` variable
- Line 285: We print the value of the `BAREMETAL_TRACE_DEBUG` variable

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
40: #include <baremetal/RPIProperties.h>
41: #include <baremetal/BCMRegisters.h>
42: #include <baremetal/Logger.h>
43: #include <baremetal/RPIPropertiesInterface.h>
44: #include <baremetal/Util.h>
...
78: /// <summary>
79: /// Mailbox property tag structure for requesting MAC address.
80: /// </summary>
81: struct PropertyMACAddress
82: {
83:     /// Tag ID, must be equal to PROPTAG_GET_MAC_ADDRESS.
84:     Property tag;
85:     /// MAC Address (6 bytes)
86:     uint8    address[6];
87:     /// Padding to align to 4 bytes
88:     uint8    padding[2];
89: } PACKED;
90: 
91: /// <summary>
92: /// Mailbox property tag structure for requesting memory information.
93: /// </summary>
94: struct PropertyMemory
95: {
96:     /// Tag ID, must be equal to PROPTAG_GET_ARM_MEMORY or PROPTAG_GET_VC_MEMORY.
97:     Property tag;
98:     /// Base address
99:     uint32   baseAddress;
100:     /// Size in bytes
101:     uint32   size;
102: } PACKED;
103: 
104: ...
113: /// <summary>
114: /// Retrieve FW revision number
115: /// </summary>
116: /// <param name="revision">FW revision (out)</param>
117: /// <returns>Return true on success, false on failure</returns>
118: bool RPIProperties::GetFirmwareRevision(uint32& revision)
119: {
120:     PropertySimple         tag{};
121:     RPIPropertiesInterface interface(m_mailbox);
122: 
123:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_FIRMWARE_REVISION, &tag, sizeof(tag));
124: 
125: #if BAREMETAL_DEBUG_TRACING
126:     LOG_DEBUG("GetFirmwareRevision");
127: 
128:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
129: #endif
130:     if (result)
131:     {
132:         revision = tag.value;
133: #if BAREMETAL_DEBUG_TRACING
134:         LOG_DEBUG("Revision: %08lx", tag.value);
135: #endif
136:     }
137: 
138:     return result;
139: }
140: 
141: /// <summary>
142: /// Retrieve Raspberry Pi board model
143: /// </summary>
144: /// <param name="model">Board model (out)</param>
145: /// <returns>Return true on success, false on failure</returns>
146: bool RPIProperties::GetBoardModel(BoardModel& model)
147: {
148:     PropertySimple         tag{};
149:     RPIPropertiesInterface interface(m_mailbox);
150: 
151:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_MODEL, &tag, sizeof(tag));
152: 
153: #if BAREMETAL_DEBUG_TRACING
154:     LOG_DEBUG("GetBoardModel");
155: 
156:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
157: #endif
158:     if (result)
159:     {
160:         model = static_cast<BoardModel>(tag.value);
161: #if BAREMETAL_DEBUG_TRACING
162:         LOG_DEBUG("Model: %08lx", tag.value);
163: #endif
164:     }
165: 
166:     return result;
167: }
168: 
169: /// <summary>
170: /// Retrieve Raspberry Pi board revision
171: /// </summary>
172: /// <param name="revision">Board revision (out)</param>
173: /// <returns>Return true on success, false on failure</returns>
174: bool RPIProperties::GetBoardRevision(BoardRevision& revision)
175: {
176:     PropertySimple         tag{};
177:     RPIPropertiesInterface interface(m_mailbox);
178: 
179:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_REVISION, &tag, sizeof(tag));
180: 
181: #if BAREMETAL_DEBUG_TRACING
182:     LOG_DEBUG("GetBoardRevision");
183: 
184:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
185: #endif
186:     if (result)
187:     {
188:         revision = static_cast<BoardRevision>(tag.value);
189: #if BAREMETAL_DEBUG_TRACING
190:         LOG_DEBUG("Revision: %08lx", tag.value);
191: #endif
192:     }
193: 
194:     return result;
195: }
196: 
197: /// <summary>
198: /// Retrieve network MAC address
199: /// </summary>
200: /// <param name="address">MAC address (out)</param>
201: /// <returns>Return true on success, false on failure</returns>
202: bool RPIProperties::GetBoardMACAddress(uint8 address[6])
203: {
204:     PropertyMACAddress     tag{};
205:     RPIPropertiesInterface interface(m_mailbox);
206: 
207:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_MAC_ADDRESS, &tag, sizeof(tag));
208: 
209: #if BAREMETAL_DEBUG_TRACING
210:     LOG_DEBUG("GetBoardMACAddress");
211: 
212:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
213: #endif
214:     if (result)
215:     {
216:         memcpy(address, tag.address, sizeof(tag.address));
217: #if BAREMETAL_DEBUG_TRACING
218:         LOG_DEBUG("Address:");
219:         GetConsole().Write(address, sizeof(tag.address));
220: #endif
221:     }
222: 
223:     return result;
224: }
225: 
226: /// <summary>
227: /// Request board serial number
228: /// </summary>
229: /// <param name="serial">On return, set to serial number, if successful</param>
230: /// <returns>Return true on success, false on failure</returns>
231: bool RPIProperties::GetBoardSerial(uint64 &serial)
232: {
233:     PropertySerial         tag{};
234:     RPIPropertiesInterface interface(m_mailbox);
235: 
236:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_BOARD_SERIAL, &tag, sizeof(tag));
237: 
238: #if BAREMETAL_DEBUG_TRACING
239:     LOG_DEBUG("GetBoardSerial");
240: 
241:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
242: #endif
243:     if (result)
244:     {
245:         serial = (static_cast<uint64>(tag.serial[1]) << 32 | static_cast<uint64>(tag.serial[0]));
246: #if BAREMETAL_DEBUG_TRACING
247:         LOG_DEBUG("Serial: %016llx", serial);
248: #endif
249:     }
250: 
251:     return result;
252: }
253: 
254: /// <summary>
255: /// Retrieve ARM assigned memory base address and size
256: /// </summary>
257: /// <param name="baseAddress">ARM assigned base address (out)</param>
258: /// <param name="size">ARM assigned memory size in bytes (out)</param>
259: /// <returns>Return true on success, false on failure</returns>
260: bool RPIProperties::GetARMMemory(uint32& baseAddress, uint32& size)
261: {
262:     PropertyMemory         tag{};
263:     RPIPropertiesInterface interface(m_mailbox);
264: 
265:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_ARM_MEMORY, &tag, sizeof(tag));
266: 
267: #if BAREMETAL_DEBUG_TRACING
268:     LOG_DEBUG("GetARMMemory");
269: 
270:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
271: #endif
272:     if (result)
273:     {
274:         baseAddress = tag.baseAddress;
275:         size = tag.size;
276: #if BAREMETAL_DEBUG_TRACING
277:         LOG_DEBUG("Base address: %08lx", baseAddress);
278:         LOG_DEBUG("Size:         %08lx", size);
279: #endif
280:     }
281: 
282:     return result;
283: }
284: 
285: /// <summary>
286: /// Retrieve VideoCore assigned memory base address and size
287: /// </summary>
288: /// <param name="baseAddress">VideoCore assigned base address (out)</param>
289: /// <param name="size">VideoCore assigned memory size in bytes (out)</param>
290: /// <returns>Return true on success, false on failure</returns>
291: bool RPIProperties::GetVCMemory(uint32& baseAddress, uint32& size)
292: {
293:     PropertyMemory         tag{};
294:     RPIPropertiesInterface interface(m_mailbox);
295: 
296:     auto                   result = interface.GetTag(PropertyID::PROPTAG_GET_VC_MEMORY, &tag, sizeof(tag));
297: 
298: #if BAREMETAL_DEBUG_TRACING
299:     LOG_DEBUG("GetARMMemory");
300: 
301:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
302: #endif
303:     if (result)
304:     {
305:         baseAddress = tag.baseAddress;
306:         size = tag.size;
307: #if BAREMETAL_DEBUG_TRACING
308:         LOG_DEBUG("Base address: %08lx", baseAddress);
309:         LOG_DEBUG("Size:         %08lx", size);
310: #endif
311:     }
312: 
313:     return result;
314: }
315: 
316: /// <summary>
317: /// Get clock rate for specified clock
318: /// </summary>
319: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
320: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
321: /// <returns>Return true on success, false on failure</returns>
322: bool RPIProperties::GetClockRate(ClockID clockID, uint32& freqHz)
323: {
324:     PropertyClockRate      tag{};
325:     RPIPropertiesInterface interface(m_mailbox);
326: 
327:     tag.clockID = static_cast<uint32>(clockID);
328:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE, &tag, sizeof(tag));
329: 
330: #if BAREMETAL_DEBUG_TRACING
331:     LOG_DEBUG("GetClockRate");
332:     LOG_DEBUG("Clock ID:   %08lx", tag.clockID);
333: 
334:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
335: #endif
336:     if (result)
337:     {
338:         freqHz = tag.rate;
339: #if BAREMETAL_DEBUG_TRACING
340:         LOG_DEBUG("Rate:       %08lx", tag.rate);
341: #endif
342:     }
343: 
344:     return result;
345: }
346: 
347: /// <summary>
348: /// Get measured clock rate for specified clock
349: /// </summary>
350: /// <param name="clockID">ID of clock for which frequency is to be retrieved</param>
351: /// <param name="freqHz">Clock frequencyy in Hz (out)</param>
352: /// <returns>Return true on success, false on failure</returns>
353: bool RPIProperties::GetMeasuredClockRate(ClockID clockID, uint32& freqHz)
354: {
355:     PropertyClockRate      tag{};
356:     RPIPropertiesInterface interface(m_mailbox);
357: 
358:     tag.clockID = static_cast<uint32>(clockID);
359:     auto result = interface.GetTag(PropertyID::PROPTAG_GET_CLOCK_RATE_MEASURED, &tag, sizeof(tag));
360: 
361: #if BAREMETAL_DEBUG_TRACING
362:     LOG_DEBUG("GetMeasuredClockRate");
363:     LOG_DEBUG("Clock ID:   %08lx", tag.clockID);
364: 
365:     LOG_DEBUG("Result: %s", result ? "OK" : "Fail");
366: #endif
367:     if (result)
368:     {
369:         freqHz = tag.rate;
370: #if BAREMETAL_DEBUG_TRACING
371:         LOG_DEBUG("Rate:       %08lx", tag.rate);
372: #endif
373:     }
374: 
375:     return result;
376: }
377: 
378: /// <summary>
379: /// Set clock rate for specified clock
380: /// </summary>
381: /// <param name="clockID">ID of clock to be set</param>
382: /// <param name="freqHz">Clock frequencyy in Hz</param>
383: /// <param name="skipTurbo">When true, do not switch to turbo setting if ARM clock is above default.
384: /// Otherwise, default behaviour is to switch to turbo setting when ARM clock is set above default frequency.</param>
385: /// <returns>Return true on success, false on failure</returns>
386: bool RPIProperties::SetClockRate(ClockID clockID, uint32 freqHz, bool skipTurbo)
387: {
388:     PropertyClockRate      tag{};
389:     RPIPropertiesInterface interface(m_mailbox);
390: 
391:     tag.clockID   = static_cast<uint32>(clockID);
392:     tag.rate      = freqHz;
393:     tag.skipTurbo = skipTurbo;
394:     auto result   = interface.GetTag(PropertyID::PROPTAG_SET_CLOCK_RATE, &tag, sizeof(tag));
395: 
396:     // Do not write to console here, as this call is needed to set up the console
397: 
398:     return result;
399: }
400: 
401: } // namespace baremetal
```

- Line 42: We need to include the header for the `Logger` class
- Line 81-89: We declare the tag structure to hold the MAC addres `PropertyMACAddress`
- Line 94-102: We declare the tag structure to memory information `PropertyMemory`
- Line 118-139: We implement the member function `GetFirmwareRevision()`.
Note that debug statements are added, which print more information in case `BAREMETAL_DEBUG_TRACING` is defined
- Line 146-167: We implement the member function `GetBoardModel()`
- Line 174-195: We implement the member function `GetBoardRevision()`
- Line 202-224: We implement the member function `GetBoardMACAddress()`
- Line 231-252: We add tracing information to the member function `GetBoardSerial()`
- Line 260-283: We implement the member function `GetARMMemory()`
- Line 291-314: We implement the member function `GetVCMemory()`
- Line 322-345: We implement the member function `GetClockRate()`.
Note that this re-uses the property structure `PropertyClockRate`
- Line 353-376: We implement the member function `GetMeasuredClockRate()`.
Note that this re-uses the property structure `PropertyClockRate`
- Line 386-399: Note that we do not add tracing to the method `SetClockRate()`.
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
57:     /// @ brief BCM2836 as used in older Raspberry Pi Model 2 boards
58:     BCM2836,
59:     /// @ brief BCM2837 as used in newer Raspberry Pi Model 2 and Raspberry Pi Model 3 boards
60:     BCM2837,
61:     /// @ brief BCM2711 as used in Raspberry Pi Model 4 boards
62:     BCM2711,
63:     /// @ brief BCM2712 as used in Raspberry Pi Model 5 boards
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
  - Line 131: We declare the method `GetARMMemorySize()` which returns the ARM assigned memory size in bytes (up to the 1 Gb border, memory above 1 Gb is not specified here)
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
211: 
212:         if (!properties.GetARMMemory(m_armBaseAddress, m_armMemorySize))
213:         {
214:             GetConsole().Write("Failed to retrieve ARM memory info\n");
215:         }
216:         
217:         if (!properties.GetVCMemory(m_vcBaseAddress, m_vcMemorySize))
218:         {
219:             GetConsole().Write("Failed to retrieve VC memory info\n");
220:         }
221: 
222:         unsigned type = (static_cast<unsigned>(m_revisionRaw) >> 4) & 0xFF;
223:         size_t   index{};
224:         size_t   count = sizeof(s_boardInfo) / sizeof(s_boardInfo[0]);
225:         for (index = 0; index < count; ++index)
226:         {
227:             if (s_boardInfo[index].type == type)
228:             {
229:                 break;
230:             }
231:         }
232: 
233:         if (index >= count)
234:         {
235:             return false;
236:         }
237: 
238:         m_boardModel         = s_boardInfo[index].model;
239:         m_boardModelMajor    = s_boardInfo[index].majorRevision;
240:         m_boardModelRevision = (static_cast<unsigned>(m_revisionRaw) & 0xF) + 1;
241:         m_SoCType            = static_cast<SoCType>((static_cast<unsigned>(m_revisionRaw) >> 12) & 0xF);
242:         m_ramSize            = 256 << ((static_cast<unsigned>(m_revisionRaw) >> 20) & 7);
243:         if (m_boardModel == BoardModel::RaspberryPi_BRelease2MB512 && m_ramSize == 256)
244:         {
245:             m_boardModel = (m_boardModelRevision == 1) ? BoardModel::RaspberryPi_BRelease1MB256 : BoardModel::RaspberryPi_BRelease2MB256;
246:         }
247:         if (static_cast<unsigned>(m_SoCType) >= static_cast<unsigned>(SoCType::Unknown))
248:         {
249:             m_SoCType = SoCType::Unknown;
250:         }
251: 
252:         m_initialized = true;
253:     }
254:     return true;
255: }
256: 
257: /// <summary>
258: /// Returns board model
259: /// </summary>
260: /// <returns>Board model</returns>
261: BoardModel MachineInfo::GetModel()
262: {
263:     return m_boardModel;
264: }
265: 
266: /// <summary>
267: /// Returns board name
268: /// </summary>
269: /// <returns>Board name</returns>
270: const char *MachineInfo::GetName()
271: {
272:     return m_boardName[static_cast<size_t>(m_boardModel)];
273: }
274: 
275: /// <summary>
276: /// Returns the major board model number
277: /// </summary>
278: /// <returns>Major board model number</returns>
279: uint32 MachineInfo::GetModelMajor()
280: {
281:     return m_boardModelMajor;
282: }
283: 
284: /// <summary>
285: /// Returns the board model revision
286: /// </summary>
287: /// <returns>Board model revision</returns>
288: uint32 MachineInfo::GetModelRevision()
289: {
290:     return m_boardModelRevision;
291: }
292: 
293: /// <summary>
294: /// Returns the SoC type
295: /// </summary>
296: /// <returns>SoC type</returns>
297: SoCType MachineInfo::GetSoCType()
298: {
299:     return m_SoCType;
300: }
301: 
302: /// <summary>
303: /// Returns the SoC name
304: /// </summary>
305: /// <returns>SoC name</returns>
306: const char *MachineInfo::GetSoCName()
307: {
308:     return s_SoCName[static_cast<size_t>(m_SoCType)];
309: }
310: 
311: /// <summary>
312: /// Returns the amount of RAM on board in Mb
313: /// </summary>
314: /// <returns>RAM size in Mb</returns>
315: uint32 MachineInfo::GetRAMSize()
316: {
317:     return m_ramSize;
318: }
319: 
320: /// <summary>
321: /// Returns the board serial number
322: /// </summary>
323: /// <returns>Board serial number</returns>
324: uint64 MachineInfo::GetSerial()
325: {
326:     return m_boardSerial;
327: }
328: 
329: /// <summary>
330: /// Returns the board FW revision
331: /// </summary>
332: /// <returns>Board FW revision</returns>
333: uint32 MachineInfo::GetFWRevision()
334: {
335:     return m_fwRevision;
336: }
337: 
338: /// <summary>
339: /// Returns the raw board revision
340: /// </summary>
341: /// <returns>Raw board revision</returns>
342: BoardRevision MachineInfo::GetBoardRevision()
343: {
344:     return m_revisionRaw;
345: }
346: 
347: /// <summary>
348: /// Returns the MAC address for the network interface
349: /// </summary>
350: /// <param name="macAddress">Network MAC address</param>
351: void MachineInfo::GetMACAddress(uint8 macAddress[6])
352: {
353:     memcpy(macAddress, m_macAddress, sizeof(m_macAddress));
354: }
355: 
356: /// <summary>
357: /// Returns the ARM memory base address
358: /// </summary>
359: /// <returns>ARM memory base address</returns>
360: uint32 MachineInfo::GetARMMemoryBaseAddress()
361: {
362:     return m_armBaseAddress;
363: }
364: 
365: /// <summary>
366: /// Returns the amount of memory assigned to the ARM cores in bytes
367: /// </summary>
368: /// <returns>Amount of memory assigned to the ARM cores in bytes</returns>
369: uint32 MachineInfo::GetARMMemorySize()
370: {
371:     return m_armMemorySize;
372: }
373: 
374: /// <summary>
375: /// Returns the VideoCore memory base address
376: /// </summary>
377: /// <returns>VideoCore memory base address</returns>
378: uint32 MachineInfo::GetVCMemoryBaseAddress()
379: {
380:     return m_vcBaseAddress;
381: }
382: 
383: /// <summary>
384: /// Returns the amount of memory assigned to the VideoCore in bytes
385: /// </summary>
386: /// <returns>Amount of memory assigned to the VideoCore in bytes</returns>
387: uint32 MachineInfo::GetVCMemorySize()
388: {
389:     return m_vcMemorySize;
390: }
391: 
392: /// <summary>
393: /// Determine and return the clock rate for a specific clock, or return an estimate
394: /// </summary>
395: /// <param name="clockID"></param>
396: /// <returns></returns>
397: unsigned MachineInfo::GetClockRate(ClockID clockID) const
398: {
399:     Mailbox       mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
400:     RPIProperties properties(mailbox);
401:     uint32        clockRate{};
402:     if (properties.GetClockRate(clockID, clockRate))
403:         return clockRate;
404:     if (properties.GetMeasuredClockRate(clockID, clockRate))
405:         return clockRate;
406: 
407:     // if clock rate can not be requested, use a default rate
408:     unsigned result = 0;
409: 
410:     switch (clockID)
411:     {
412:     case ClockID::EMMC:
413:     case ClockID::EMMC2:
414:         result = 100000000;
415:         break;
416: 
417:     case ClockID::UART:
418:         result = 48000000;
419:         break;
420: 
421:     case ClockID::CORE:
422:         result = 300000000; /// \todo Check this
423:         break;
424: 
425:     case ClockID::PIXEL_BVB:
426:         break;
427: 
428:     default:
429:         assert(0);
430:         break;
431:     }
432: 
433:     return result;
434: }
435: 
436: /// <summary>
437: /// Create the singleton MachineInfo instance if needed, initialize it, and return a reference
438: /// </summary>
439: /// <returns>Singleton MachineInfo reference</returns>
440: MachineInfo &baremetal::GetMachineInfo()
441: {
442:     static MachineInfo machineInfo;
443:     machineInfo.Initialize();
444:     return machineInfo;
445: }
```

- Line 54-62: We declare a structure `BoardInfo` to hold information for the different board models.
This is used to map the raw board revision code to a board model
- Line 65-85: We define an array of `BoardInfo` structure for the mapping to board models
- Line 92-116: We define an array of strings to map board models to names
- Line 123-131: We define an array of strings to map SoC types to names
- Line 136-153: We implement the default constructor
- Line 159-176: We implement the constructor taking a `MemoryAccess` instance
- Line 185-253: We implement the `Initialize()` method
  - Line 189-190: We set up the mailbox
  - Line 192-195: We request the firmware revision number
  - Line 197-200: We request the board revision number
  - Line 202-205: We request the board serial number
  - Line 207-210: We request the MAC address
  - Line 212-215: We request the ARM assigned memory information
  - Line 217-220: We request the VideoCore assigned memory information
  - Line 222-231: We do some trickery to extract a type code (bits 4-11 of the revision number) and look up the board information
  - Line 238-239: We set the board model and board major revision number from the board information found
  - Line 240: We extract the board revision number (bits 0 to 3)
  - Line 241: We extract the SoC type (bit 12 to 15)
  - Line 242: We extract the RAM size (bits 20-22)
  - Line 243-246: We adjust for some special cases for Raspberry Pi 1 and 2
  - Line 247-250: We check whether the SoC type is valid
- Line 261-264: We implement the `GetModel()` method which simply returns the saved board model
- Line 270-273: We implement the `GetName()` method which returns the name for the board model
- Line 279-282: We implement the `GetModelMajor()` method which simply returns the saved board major revision number
- Line 288-291: We implement the `GetModelRevision()` method which simply returns the saved board revision number
- Line 297-300: We implement the `GetSoCType()` method which simply returns the saved SoC type
- Line 306-309: We implement the `GetSoCName()` method which returns the name of the SoC type
- Line 315-318: We implement the `GetRAMSize()` method which simply returns the saved memory size
- Line 324-327: We implement the `GetSerial()` method which simply returns the saved board serial number
- Line 333-336: We implement the `GetFWRevision()` method which simply returns the saved firmware revision number
- Line 342-345: We implement the `GetBoardRevision()` method which simply returns the saved raw board revision code
- Line 351-354: We implement the `GetMACAddress()` method which simply returns the saved MAC address
- Line 360-363: We implement the `GetARMMemoryBaseAddress()` method which simply returns the saved ARM memory base address
- Line 369-372: We implement the `GetARMMemorySize()` method which simply returns the saved ARM memory size
- Line 378-381: We implement the `GetVCMemoryBaseAddress()` method which simply returns the saved VideoCore memory base address
- Line 387-390: We implement the `GetVCMemorySize()` method which simply returns the saved VideoCore memory size
- Line 397-434: We implement the `GetClockRate()` method, which tries to request the set clock rate,
if not available the measured clock rate, and if all fails an estimate of the clock frequency
- Line 440-445: We implement the `GetMachineInfo()` function

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

<img src="images/tutorial-13-logger.png" alt="Console output" width="800"/>

Next: [14-memory-management](14-memory-management.md)

