# Tutorial 16: Serialization and formatting {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING}

@tableofcontents

## New tutorial setup {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/16-serialization-and-formatting`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-16.a`
- an application `output/Debug/bin/16-serialization-and-formatting.elf`
- an image in `deploy/Debug/16-serialization-and-formatting-image`

## Memory allocating and non memory allocating printing - Step 1 {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING__STEP_1}

It would be nice if we could start using our `string` class for logging, serialization, etc.
However, sometimes we want to print things before the memory manager is initialized.
So we'll want to have a non memory-allocating version next to the new memory-allocating, string based version.

We'll first update the `Logger` to support both versions, and we'll move down from there.

### Logger.h {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING__STEP_1_LOGGERH}

We'll add non memory-allocating methods `WriteNoAlloc()` and `WriteNoAllocV()`, and add complementary macros.

Update the file `code/libraries/baremetal/include/baremetal/Logger.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Logger.h
...
97:     void Write(const char *source, int line, LogSeverity severity, const char *message, ...);
98:     void WriteV(const char *source, int line, LogSeverity severity, const char *message, va_list args);
99:
100:     void WriteNoAlloc(const char* source, int line, LogSeverity severity, const char* message, ...);
101:     void WriteNoAllocV(const char* source, int line, LogSeverity severity, const char* message, va_list args);
102: };
103:
...
120: /// @brief Log a message with specified severity and message string
121: #define LOG(severity, message) GetLogger().Write(From, __LINE__, severity, message);
122:
123: // Non memory allocating versions
124: /// @brief Log a panic message
125: #define LOG_NO_ALLOC_PANIC(...)         GetLogger().WriteNoAlloc(From, __LINE__, LogSeverity::Panic, __VA_ARGS__)
126: /// @brief Log an error message
127: #define LOG_NO_ALLOC_ERROR(...)         GetLogger().WriteNoAlloc(From, __LINE__, LogSeverity::Error, __VA_ARGS__)
128: /// @brief Log a warning message
129: #define LOG_NO_ALLOC_WARNING(...)       GetLogger().WriteNoAlloc(From, __LINE__, LogSeverity::Warning, __VA_ARGS__)
130: /// @brief Log a info message
131: #define LOG_NO_ALLOC_INFO(...)          GetLogger().WriteNoAlloc(From, __LINE__, LogSeverity::Info, __VA_ARGS__)
132: /// @brief Log a debug message
133: #define LOG_NO_ALLOC_DEBUG(...)         GetLogger().WriteNoAlloc(From, __LINE__, LogSeverity::Debug, __VA_ARGS__)
134:
135: /// @brief Log a message with specified severity and message string
136: #define LOG_NO_ALLOC(severity, message) GetLogger().WriteNoAlloc(From, __LINE__, severity, message);
137:
138: } // namespace baremetal
139:
```

### Logger.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING__STEP_1_LOGGERCPP}

We'll implement the new methods.
We'll also use the non memory-allocating macro in the `Initialize()` method.

Update the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
...
42: #include <baremetal/Console.h>
43: #include <baremetal/Format.h>
44: #include <baremetal/MachineInfo.h>
45: #include <baremetal/String.h>
46: #include <baremetal/System.h>
47: #include <baremetal/Timer.h>
48: #include <baremetal/Util.h>
49: #include <baremetal/Version.h>
...
70: /// <summary>
71: /// Initialize logger
72: /// </summary>
73: /// <returns>true on succes, false on failure</returns>
74: bool Logger::Initialize()
75: {
76:     if (m_initialized)
77:         return true;
78:     SetupVersion();
79:     m_initialized = true; // Stop reentrant calls from happening
80:     LOG_NO_ALLOC_INFO(BAREMETAL_NAME " %s started on %s (AArch64) using %s SoC", BAREMETAL_VERSION_STRING, GetMachineInfo().GetName(), GetMachineInfo().GetSoCName());
81:
82:     return true;
83: }
...
109: /// <summary>
110: /// Write a string with variable arguments to the logger
111: /// </summary>
112: /// <param name="source">Source name or file name</param>
113: /// <param name="line">Source line number</param>
114: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
115: /// <param name="message">Formatted message string</param>
116: /// <param name="args">Variable argument list</param>
117: void Logger::WriteV(const char *source, int line, LogSeverity severity, const char *message, va_list args)
118: {
119:     if (static_cast<int>(severity) > static_cast<int>(m_level))
120:         return;
121:
122:     string lineBuffer;
123:
124:     auto sourceString = Format(" (%s:%d)", source, line);
125:
126:     auto messageBuffer = FormatV(message, args);
127:
128:     switch (severity)
129:     {
130:     case LogSeverity::Panic:
131:         lineBuffer += "!Panic!";
132:         break;
133:     case LogSeverity::Error:
134:         lineBuffer += "Error  ";
135:         break;
136:     case LogSeverity::Warning:
137:         lineBuffer += "Warning";
138:         break;
139:     case LogSeverity::Info:
140:         lineBuffer += "Info   ";
141:         break;
142:     case LogSeverity::Debug:
143:         lineBuffer += "Debug  ";
144:         break;
145:     }
146:
147:     if (m_timer != nullptr)
148:     {
149:         const size_t TimeBufferSize = 32;
150:         char timeBuffer[TimeBufferSize]{};
151:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
152:         if (strlen(timeBuffer) > 0)
153:         {
154:             lineBuffer += timeBuffer;
155:             lineBuffer += ' ';
156:         }
157:     }
158:
159:     lineBuffer += messageBuffer;
160:     lineBuffer += sourceString;
161:     lineBuffer += "\n";
162:
163: #if BAREMETAL_COLOR_OUTPUT
164:     switch (severity)
165:     {
166:     case LogSeverity::Panic:
167:         m_console.Write(lineBuffer, ConsoleColor::BrightRed);
168:         break;
169:     case LogSeverity::Error:
170:         m_console.Write(lineBuffer, ConsoleColor::Red);
171:         break;
172:     case LogSeverity::Warning:
173:         m_console.Write(lineBuffer, ConsoleColor::BrightYellow);
174:         break;
175:     case LogSeverity::Info:
176:         m_console.Write(lineBuffer, ConsoleColor::Cyan);
177:         break;
178:     case LogSeverity::Debug:
179:         m_console.Write(lineBuffer, ConsoleColor::Yellow);
180:         break;
181:     default:
182:         m_console.Write(lineBuffer, ConsoleColor::White);
183:         break;
184:     }
185: #else
186:     m_console.Write(lineBuffer);
187: #endif
188:
189:     if (severity == LogSeverity::Panic)
190:     {
191:         GetSystem().Halt();
192:     }
193: }
194:
195: /// <summary>
196: /// Write a string with variable arguments to the logger
197: /// </summary>
198: /// <param name="source">Source name or file name</param>
199: /// <param name="line">Source line number</param>
200: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
201: /// <param name="message">Formatted message string, with variable arguments</param>
202: void Logger::WriteNoAlloc(const char* source, int line, LogSeverity severity, const char* message, ...)
203: {
204:     va_list var;
205:     va_start(var, message);
206:     WriteNoAllocV(source, line, severity, message, var);
207:     va_end(var);
208: }
209:
210: /// <summary>
211: /// Write a string with variable arguments to the logger
212: /// </summary>
213: /// <param name="source">Source name or file name</param>
214: /// <param name="line">Source line number</param>
215: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
216: /// <param name="message">Formatted message string</param>
217: /// <param name="args">Variable argument list</param>
218: void Logger::WriteNoAllocV(const char* source, int line, LogSeverity severity, const char* message, va_list args)
219: {
220:     if (static_cast<int>(severity) > static_cast<int>(m_level))
221:         return;
222:
223:     static const size_t BufferSize = 1024;
224:     char buffer[BufferSize]{};
225:
226:     char sourceString[BufferSize]{};
227:     FormatNoAlloc(sourceString, BufferSize, " (%s:%d)", source, line);
228:
229:     char messageBuffer[BufferSize]{};
230:     FormatNoAllocV(messageBuffer, BufferSize, message, args);
231:
232:     switch (severity)
233:     {
234:     case LogSeverity::Panic:
235:         strncat(buffer, "!Panic!", BufferSize);
236:         break;
237:     case LogSeverity::Error:
238:         strncat(buffer, "Error  ", BufferSize);
239:         break;
240:     case LogSeverity::Warning:
241:         strncat(buffer, "Warning", BufferSize);
242:         break;
243:     case LogSeverity::Info:
244:         strncat(buffer, "Info   ", BufferSize);
245:         break;
246:     case LogSeverity::Debug:
247:         strncat(buffer, "Debug  ", BufferSize);
248:         break;
249:     }
250:
251:     if (m_timer != nullptr)
252:     {
253:         const size_t TimeBufferSize = 32;
254:         char timeBuffer[TimeBufferSize]{};
255:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
256:         if (strlen(timeBuffer) > 0)
257:         {
258:             strncat(buffer, timeBuffer, BufferSize);
259:             strncat(buffer, " ", BufferSize);
260:         }
261:     }
262:
263:     strncat(buffer, messageBuffer, BufferSize);
264:     strncat(buffer, sourceString, BufferSize);
265:     strncat(buffer, "\n", BufferSize);
266:
267: #if BAREMETAL_COLOR_OUTPUT
268:     switch (severity)
269:     {
270:     case LogSeverity::Panic:
271:         m_console.Write(buffer, ConsoleColor::BrightRed);
272:         break;
273:     case LogSeverity::Error:
274:         m_console.Write(buffer, ConsoleColor::Red);
275:         break;
276:     case LogSeverity::Warning:
277:         m_console.Write(buffer, ConsoleColor::BrightYellow);
278:         break;
279:     case LogSeverity::Info:
280:         m_console.Write(buffer, ConsoleColor::Cyan);
281:         break;
282:     case LogSeverity::Debug:
283:         m_console.Write(buffer, ConsoleColor::Yellow);
284:         break;
285:     default:
286:         m_console.Write(buffer, ConsoleColor::White);
287:         break;
288:     }
289: #else
290:     m_console.Write(buffer);
291: #endif
292:
293:     if (severity == LogSeverity::Panic)
294:     {
295:         GetSystem().Halt();
296:     }
297: }
...
```

- Line 45: As we're going to use the `string` class, we include its header
- Line 80: We use the macro `LOG_NO_ALLOC_INFO` to make sure the first line printed by the logger does not need memory allocation
- Line 117-193: The new, memory-allocating version of WriteV will use the `string` class
  - Line 122: We compose the line in a string
  - Line 124: We compose the source line information in a string. We use the memory allocating version of `Format()` for this
  - Line 126: We compose the message in a string. We use the memory allocating version of `FormatV()` for this
  - Line 128-161: We append to the line using the add operator `+=`
- Line 202-208: We implement the non memory-allocating `WriteNoAlloc()`. This uses `WriteNoAllocV()` for output
- Line 218-297: We implement the non memory-allocating `WriteNoAllocV()`. This is the original function, but using `FormatNoAlloc()` and `FormatNoAllocV()`

### Format.h {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING__STEP_1_FORMATH}

We'll add non memory-allocating methods `FormatNoAlloc()` and `FormatNoAllocV()`, and convert `Format()` and `FormatV()` into string returning versions.

Update the file `code/libraries/baremetal/include/baremetal/Format.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Format.h
...
45: namespace baremetal {
46:
47: class string;
48:
49: string FormatV(const char* format, va_list args);
50: string Format(const char* format, ...);
51: void FormatNoAllocV(char* buffer, size_t bufferSize, const char* format, va_list args);
52: void FormatNoAlloc(char* buffer, size_t bufferSize, const char* format, ...);
53:
54: } // namespace baremetal
```

### Format.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING__STEP_1_FORMATCPP}

We'll implement the new methods.

Update the file `code/libraries/baremetal/src/Format.cpp`

```cpp
File: code/libraries/baremetal/src/Format.cpp
...
42: #include <baremetal/String.h>
43: #include <baremetal/Util.h>
44:
45: namespace baremetal {
46:
47: /// @brief Write characters with base above 10 as uppercase or not
48: static bool           Uppercase = true;
49:
50: static void PrintValueInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
51: static void PrintValueInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
52:
53: /// <summary>
54: /// Convert a value to a digit. Character range is 0..9-A..Z or a..z depending on value of Uppercase
55: /// </summary>
56: /// <param name="value">Digit value</param>
57: /// <returns>Converted digit character</returns>
58: static constexpr char GetDigit(uint8 value)
59: {
60:     return value + ((value < 10) ? '0' : 'A' - 10 + (Uppercase ? 0 : 0x20));
61: }
62:
63: /// <summary>
64: /// Calculated the amount of digits needed to represent an unsigned value of bits using base
65: /// </summary>
66: /// <param name="bits">Size of integer in bits</param>
67: /// <param name="base">Base to be used</param>
68: /// <returns>Maximum amount of digits needed</returns>
69: static constexpr int BitsToDigits(int bits, int base)
70: {
71:     int result = 0;
72:     uint64 value = 0xFFFFFFFFFFFFFFFF;
73:     if (bits < 64)
74:         value &= ((1ULL << bits) - 1);
75:
76:     while (value > 0)
77:     {
78:         value /= base;
79:         result++;
80:     }
81:
82:     return result;
83: }
84:
85: /// <summary>
86: /// PrintValue a 32 bit signed value to buffer.
87: ///
88: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
89: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
90: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
91: ///
92: /// Base is the digit base, which can range from 2 to 36.
93: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
94: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
95: /// </summary>
96: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
97: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
98: /// <param name="value">Value to be serialized</param>
99: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
100: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
101: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
102: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
103: static void PrintValue(char* buffer, size_t bufferSize, int32 value, int width, int base, bool showBase, bool leadingZeros)
104: {
105:     PrintValueInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
106: }
107:
108: /// <summary>
109: /// PrintValue a 32 bit unsigned value to buffer.
110: ///
111: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
112: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
113: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
114: ///
115: /// Base is the digit base, which can range from 2 to 36.
116: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
117: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
118: /// </summary>
119: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
120: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
121: /// <param name="value">Value to be serialized</param>
122: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
123: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
124: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
125: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
126: static void PrintValue(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros)
127: {
128:     PrintValueInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
129: }
130:
131: /// <summary>
132: /// PrintValue a 64 bit signed value to buffer.
133: ///
134: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
135: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
136: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
137: ///
138: /// Base is the digit base, which can range from 2 to 36.
139: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
140: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
141: /// </summary>
142: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
143: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
144: /// <param name="value">Value to be serialized</param>
145: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
146: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
147: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
148: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
149: static void PrintValue(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros)
150: {
151:     PrintValueInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
152: }
153:
154: /// <summary>
155: /// PrintValue a 64 bit unsigned value to buffer.
156: ///
157: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
158: /// Width specifies the minimum width in characters. The value is always written right aligned.
159: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
160: ///
161: /// Base is the digit base, which can range from 2 to 36.
162: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
163: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
164: /// </summary>
165: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
166: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
167: /// <param name="value">Value to be serialized</param>
168: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
169: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
170: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
171: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
172: static void PrintValue(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros)
173: {
174:     PrintValueInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
175: }
176:
177: /// <summary>
178: /// PrintValue long long int value, type specific specialization
179: /// </summary>
180: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
181: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
182: /// <param name="value">Value to be serialized</param>
183: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
184: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
185: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
186: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
187: inline static void PrintValue(char* buffer, size_t bufferSize, long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
188: {
189:     PrintValue(buffer, bufferSize, static_cast<int64>(value), width, base, showBase, leadingZeros);
190: }
191: /// <summary>
192: /// PrintValue unsigned long long int value, type specific specialization
193: /// </summary>
194: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
195: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
196: /// <param name="value">Value to be serialized</param>
197: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
198: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
199: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
200: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
201: inline static void PrintValue(char* buffer, size_t bufferSize, unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
202: {
203:     PrintValue(buffer, bufferSize, static_cast<uint64>(value), width, base, showBase, leadingZeros);
204: }
205:
206: /// <summary>
207: /// PrintValue a double value to buffer. The value will be printed as a fixed point number.
208: ///
209: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, the string is terminated to hold maximum bufferSize - 1 characters.
210: /// Width is currently unused.
211: /// Precision specifies the number of digits behind the decimal pointer
212: /// </summary>
213: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
214: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
215: /// <param name="value">Value to be serialized</param>
216: /// <param name="width">Unused</param>
217: /// <param name="precision">Number of digits after the decimal point to use</param>
218: static void PrintValue(char* buffer, size_t bufferSize, double value, int width, int precision)
219: {
220:     bool negative{};
221:     if (value < 0)
222:     {
223:         negative = true;
224:         value = -value;
225:     }
226:
227:     if (bufferSize == 0)
228:         return;
229:
230:     // We can only print values with integral parts up to what uint64 can hold
231:     if (value > static_cast<double>(static_cast<uint64>(-1)))
232:     {
233:         strncpy(buffer, "overflow", bufferSize);
234:         return;
235:     }
236:
237:     *buffer = '\0';
238:     if (negative)
239:         strncpy(buffer, "-", bufferSize);
240:
241:     uint64 integralPart = static_cast<uint64>(value);
242:     const size_t TmpBufferSize = 32;
243:     char tmpBuffer[TmpBufferSize];
244:     PrintValue(tmpBuffer, TmpBufferSize, integralPart, 0, 10, false, false);
245:     strncat(buffer, tmpBuffer, bufferSize);
246:     const int MaxPrecision = 7;
247:
248:     if (precision != 0)
249:     {
250:         strncat(buffer, ".", bufferSize);
251:
252:         if (precision > MaxPrecision)
253:         {
254:             precision = MaxPrecision;
255:         }
256:
257:         uint64 precisionPower10 = 1;
258:         for (int i = 1; i <= precision; i++)
259:         {
260:             precisionPower10 *= 10;
261:         }
262:
263:         value -= static_cast<double>(integralPart);
264:         value *= static_cast<double>(precisionPower10);
265:
266:         PrintValue(tmpBuffer, TmpBufferSize, static_cast<uint64>(value), 0, 10, false, false);
267:         strncat(buffer, tmpBuffer, bufferSize);
268:         precision -= strlen(tmpBuffer);
269:         while (precision--)
270:         {
271:             strncat(buffer, "0", bufferSize);
272:         }
273:     }
274: }
275:
276: /// <summary>
277: /// PrintValue a const char * value to buffer. The value can be quoted.
278: ///
279: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
280: /// If quote is true, the string is printed within double quotes (\")
281: /// </summary>
282: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
283: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
284: /// <param name="value">String to be serialized</param>
285: /// <param name="width">Unused</param>
286: /// <param name="quote">If true, value is printed between double quotes, if false, no quotes are used</param>
287: static void PrintValue(char* buffer, size_t bufferSize, const char* value, int width, bool quote)
288: {
289:     size_t numChars = strlen(value);
290:     if (quote)
291:         numChars += 2;
292:
293:     // Leave one character for \0
294:     if (numChars > bufferSize - 1)
295:         return;
296:
297:     char* bufferPtr = buffer;
298:
299:     if (quote)
300:         *bufferPtr++ = '\"';
301:     while (*value)
302:     {
303:         *bufferPtr++ = *value++;
304:     }
305:     if (quote)
306:         *bufferPtr++ = '\"';
307: }
308:
309: /// <summary>
310: /// Internal serialization function, to be used for all signed values.
311: ///
312: /// PrintValue a signed value to buffer.
313: ///
314: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
315: /// Width specifies the minimum width in characters. The value is always written right aligned.
316: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
317: ///
318: /// Base is the digit base, which can range from 2 to 36.
319: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
320: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
321: /// </summary>
322: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
323: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
324: /// <param name="value">Value to be serialized</param>
325: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
326: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
327: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
328: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
329: /// <param name="numBits">Specifies the number of bits used for the value</param>
330: static void PrintValueInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
331: {
332:     if ((base < 2) || (base > 36))
333:         return;
334:
335:     int       numDigits = 0;
336:     bool      negative = (value < 0);
337:     uint64    absVal = static_cast<uint64>(negative ? -value : value);
338:     uint64    divisor = 1;
339:     uint64    divisorLast = 1;
340:     size_t    absWidth = (width < 0) ? -width : width;
341:     const int maxDigits = BitsToDigits(numBits, base);
342:     while ((absVal >= divisor) && (numDigits <= maxDigits))
343:     {
344:         divisorLast = divisor;
345:         divisor *= base;
346:         ++numDigits;
347:     }
348:     divisor = divisorLast;
349:
350:     size_t numChars = (numDigits > 0) ? numDigits : 1;
351:     if (showBase)
352:     {
353:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
354:     }
355:     if (negative)
356:     {
357:         numChars++;
358:     }
359:     if (absWidth > numChars)
360:         numChars = absWidth;
361:     // Leave one character for \0
362:     if (numChars > bufferSize - 1)
363:         return;
364:
365:     char* bufferPtr = buffer;
366:     if (negative)
367:     {
368:         *bufferPtr++ = '-';
369:     }
370:
371:     if (showBase)
372:     {
373:         if (base == 2)
374:         {
375:             *bufferPtr++ = '0';
376:             *bufferPtr++ = 'b';
377:         }
378:         else if (base == 8)
379:         {
380:             *bufferPtr++ = '0';
381:         }
382:         else if (base == 16)
383:         {
384:             *bufferPtr++ = '0';
385:             *bufferPtr++ = 'x';
386:         }
387:     }
388:     if (leadingZeros)
389:     {
390:         if (absWidth == 0)
391:             absWidth = maxDigits;
392:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
393:         {
394:             *bufferPtr++ = '0';
395:         }
396:     }
397:     else
398:     {
399:         if (numDigits == 0)
400:         {
401:             *bufferPtr++ = '0';
402:         }
403:     }
404:     while (numDigits > 0)
405:     {
406:         int digit = (absVal / divisor) % base;
407:         *bufferPtr++ = GetDigit(digit);
408:         --numDigits;
409:         divisor /= base;
410:     }
411:     *bufferPtr++ = '\0';
412: }
413:
414: /// <summary>
415: /// Internal serialization function, to be used for all unsigned values.
416: ///
417: /// PrintValue a unsigned value to buffer.
418: ///
419: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
420: /// Width specifies the minimum width in characters. The value is always written right aligned.
421: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
422: ///
423: /// Base is the digit base, which can range from 2 to 36.
424: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
425: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
426: /// </summary>
427: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
428: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
429: /// <param name="value">Value to be serialized</param>
430: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
431: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
432: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
433: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
434: /// <param name="numBits">Specifies the number of bits used for the value</param>
435: static void PrintValueInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
436: {
437:     if ((base < 2) || (base > 36))
438:         return;
439:
440:     int       numDigits = 0;
441:     uint64    divisor = 1;
442:     uint64    divisorLast = 1;
443:     uint64    divisorHigh = 0;
444:     size_t    absWidth = (width < 0) ? -width : width;
445:     const int maxDigits = BitsToDigits(numBits, base);
446:     while ((divisorHigh == 0) && (value >= divisor) && (numDigits <= maxDigits))
447:     {
448:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
449:         divisorLast = divisor;
450:         divisor *= base;
451:         ++numDigits;
452:     }
453:     divisor = divisorLast;
454:
455:     size_t numChars = (numDigits > 0) ? numDigits : 1;
456:     if (showBase)
457:     {
458:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
459:     }
460:     if (absWidth > numChars)
461:         numChars = absWidth;
462:     // Leave one character for \0
463:     if (numChars > bufferSize - 1)
464:         return;
465:
466:     char* bufferPtr = buffer;
467:
468:     if (showBase)
469:     {
470:         if (base == 2)
471:         {
472:             *bufferPtr++ = '0';
473:             *bufferPtr++ = 'b';
474:         }
475:         else if (base == 8)
476:         {
477:             *bufferPtr++ = '0';
478:         }
479:         else if (base == 16)
480:         {
481:             *bufferPtr++ = '0';
482:             *bufferPtr++ = 'x';
483:         }
484:     }
485:     if (leadingZeros)
486:     {
487:         if (absWidth == 0)
488:             absWidth = maxDigits;
489:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
490:         {
491:             *bufferPtr++ = '0';
492:         }
493:     }
494:     else
495:     {
496:         if (numDigits == 0)
497:         {
498:             *bufferPtr++ = '0';
499:         }
500:     }
501:     while (numDigits > 0)
502:     {
503:         int digit = (value / divisor) % base;
504:         *bufferPtr++ = GetDigit(digit);
505:         --numDigits;
506:         divisor /= base;
507:     }
508:     *bufferPtr++ = '\0';
509: }
510:
511: const size_t BufferSize = 1024;
512:
513: static void Append(char* buffer, size_t bufferSize, char c)
514: {
515:     size_t len = strlen(buffer);
516:     char* p = buffer + len;
517:     if (static_cast<size_t>(p - buffer) < bufferSize)
518:     {
519:         *p++ = c;
520:     }
521:     if (static_cast<size_t>(p - buffer) < bufferSize)
522:     {
523:         *p = '\0';
524:     }
525: }
526:
527: static void Append(char* buffer, size_t bufferSize, size_t count, char c)
528: {
529:     size_t len = strlen(buffer);
530:     char* p = buffer + len;
531:     while ((count > 0) && (static_cast<size_t>(p - buffer) < bufferSize))
532:     {
533:         *p++ = c;
534:         --count;
535:     }
536:     if (static_cast<size_t>(p - buffer) < bufferSize)
537:     {
538:         *p = '\0';
539:     }
540: }
541:
542: static void Append(char* buffer, size_t bufferSize, const char* str)
543: {
544:     strncat(buffer, str, bufferSize);
545: }
546:
547: string Format(const char* format, ...)
548: {
549:     static const size_t BufferSize = 1024;
550:     char buffer[BufferSize]{};
551:     va_list var;
552:     va_start(var, format);
553:
554:     FormatNoAllocV(buffer, BufferSize, format, var);
555:
556:     va_end(var);
557:
558:     string result = buffer;
559:     return result;
560: }
561:
562: string FormatV(const char* format, va_list args)
563: {
564:     static const size_t BufferSize = 1024;
565:     char buffer[BufferSize]{};
566:     FormatNoAllocV(buffer, BufferSize, format, args);
567:
568:     string result = buffer;
569:     return result;
570: }
571:
572: void FormatNoAlloc(char* buffer, size_t bufferSize, const char* format, ...)
573: {
574:     va_list var;
575:     va_start(var, format);
576:
577:     FormatNoAllocV(buffer, bufferSize, format, var);
578:
579:     va_end(var);
580: }
581:
582: void FormatNoAllocV(char* buffer, size_t bufferSize, const char* format, va_list args)
583: {
584:     buffer[0] = '\0';
585:
586:     while (*format != '\0')
587:     {
588:         if (*format == '%')
589:         {
590:             if (*++format == '%')
591:             {
592:                 Append(buffer, bufferSize, '%');
593:                 format++;
594:                 continue;
595:             }
596:
597:             bool alternate = false;
598:             if (*format == '#')
599:             {
600:                 alternate = true;
601:                 format++;
602:             }
603:
604:             bool left = false;
605:             if (*format == '-')
606:             {
607:                 left = true;
608:                 format++;
609:             }
610:
611:             bool leadingZero = false;
612:             if (*format == '0')
613:             {
614:                 leadingZero = true;
615:                 format++;
616:             }
617:
618:             size_t width = 0;
619:             while (('0' <= *format) && (*format <= '9'))
620:             {
621:                 width = width * 10 + (*format - '0');
622:                 format++;
623:             }
624:
625:             unsigned precision = 6;
626:             if (*format == '.')
627:             {
628:                 format++;
629:                 precision = 0;
630:                 while ('0' <= *format && *format <= '9')
631:                 {
632:                     precision = precision * 10 + (*format - '0');
633:
634:                     format++;
635:                 }
636:             }
637:
638:             bool haveLong{};
639:             bool haveLongLong{};
640:
641:             if (*format == 'l')
642:             {
643:                 if (*(format + 1) == 'l')
644:                 {
645:                     haveLongLong = true;
646:
647:                     format++;
648:                 }
649:                 else
650:                 {
651:                     haveLong = true;
652:                 }
653:
654:                 format++;
655:             }
656:
657:             switch (*format)
658:             {
659:             case 'c':
660:             {
661:                 char ch = static_cast<char>(va_arg(args, int));
662:                 if (left)
663:                 {
664:                     Append(buffer, bufferSize, ch);
665:                     if (width > 1)
666:                     {
667:                         Append(buffer, bufferSize, width - 1, ' ');
668:                     }
669:                 }
670:                 else
671:                 {
672:                     if (width > 1)
673:                     {
674:                         Append(buffer, bufferSize, width - 1, ' ');
675:                     }
676:                     Append(buffer, bufferSize, ch);
677:                 }
678:             }
679:             break;
680:
681:             case 'd':
682:             case 'i':
683:                 if (haveLongLong)
684:                 {
685:                     char str[BufferSize]{};
686:                     PrintValue(str, BufferSize, va_arg(args, int64), left ? -width : width, 10, false, leadingZero);
687:                     Append(buffer, bufferSize, str);
688:                 }
689:                 else if (haveLong)
690:                 {
691:                     char str[BufferSize]{};
692:                     PrintValue(str, BufferSize, va_arg(args, int32), left ? -width : width, 10, false, leadingZero);
693:                     Append(buffer, bufferSize, str);
694:                 }
695:                 else
696:                 {
697:                     char str[BufferSize]{};
698:                     PrintValue(str, BufferSize, va_arg(args, int), left ? -width : width, 10, false, leadingZero);
699:                     Append(buffer, bufferSize, str);
700:                 }
701:                 break;
702:
703:             case 'f':
704:             {
705:                 char str[BufferSize]{};
706:                 PrintValue(str, BufferSize, va_arg(args, double), left ? -width : width, precision);
707:                 Append(buffer, bufferSize, str);
708:             }
709:             break;
710:
711:             case 'b':
712:                 if (alternate)
713:                 {
714:                     Append(buffer, bufferSize, "0b");
715:                 }
716:                 if (haveLongLong)
717:                 {
718:                     char str[BufferSize]{};
719:                     PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 2, false, leadingZero);
720:                     Append(buffer, bufferSize, str);
721:                 }
722:                 else if (haveLong)
723:                 {
724:                     char str[BufferSize]{};
725:                     PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 2, false, leadingZero);
726:                     Append(buffer, bufferSize, str);
727:                 }
728:                 else
729:                 {
730:                     char str[BufferSize]{};
731:                     PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero);
732:                     Append(buffer, bufferSize, str);
733:                 }
734:                 break;
735:
736:             case 'o':
737:                 if (alternate)
738:                 {
739:                     Append(buffer, bufferSize, '0');
740:                 }
741:                 if (haveLongLong)
742:                 {
743:                     char str[BufferSize]{};
744:                     PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 8, false, leadingZero);
745:                     Append(buffer, bufferSize, str);
746:                 }
747:                 else if (haveLong)
748:                 {
749:                     char str[BufferSize]{};
750:                     PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 8, false, leadingZero);
751:                     Append(buffer, bufferSize, str);
752:                 }
753:                 else
754:                 {
755:                     char str[BufferSize]{};
756:                     PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero);
757:                     Append(buffer, bufferSize, str);
758:                 }
759:                 break;
760:
761:             case 's':
762:             {
763:                 char str[BufferSize]{};
764:                 PrintValue(str, BufferSize, va_arg(args, const char*), left ? -width : width, false);
765:                 Append(buffer, bufferSize, str);
766:             }
767:             break;
768:
769:             case 'u':
770:                 if (haveLongLong)
771:                 {
772:                     char str[BufferSize]{};
773:                     PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 10, false, leadingZero);
774:                     Append(buffer, bufferSize, str);
775:                 }
776:                 else if (haveLong)
777:                 {
778:                     char str[BufferSize]{};
779:                     PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 10, false, leadingZero);
780:                     Append(buffer, bufferSize, str);
781:                 }
782:                 else
783:                 {
784:                     char str[BufferSize]{};
785:                     PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero);
786:                     Append(buffer, bufferSize, str);
787:                 }
788:                 break;
789:
790:             case 'x':
791:             case 'X':
792:                 if (alternate)
793:                 {
794:                     Append(buffer, bufferSize, "0x");
795:                 }
796:                 if (haveLongLong)
797:                 {
798:                     char str[BufferSize]{};
799:                     PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 16, false, leadingZero);
800:                     Append(buffer, bufferSize, str);
801:                 }
802:                 else if (haveLong)
803:                 {
804:                     char str[BufferSize]{};
805:                     PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 16, false, leadingZero);
806:                     Append(buffer, bufferSize, str);
807:                 }
808:                 else
809:                 {
810:                     char str[BufferSize]{};
811:                     PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero);
812:                     Append(buffer, bufferSize, str);
813:                 }
814:                 break;
815:
816:             case 'p':
817:                 if (alternate)
818:                 {
819:                     Append(buffer, bufferSize, "0x");
820:                 }
821:                 {
822:                     char str[BufferSize]{};
823:                     PrintValue(str, BufferSize, va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero);
824:                     Append(buffer, bufferSize, str);
825:                 }
826:                 break;
827:
828:             default:
829:                 Append(buffer, bufferSize, '%');
830:                 Append(buffer, bufferSize, *format);
831:                 break;
832:             }
833:         }
834:         else
835:         {
836:             Append(buffer, bufferSize, *format);
837:         }
838:
839:         format++;
840:     }
841: }
...
```

- Line 42: We will be moving functions from Serialization to here, so we'll remove the include for `Serialization.h`.
We however do need the `string` class, so we'll add its header
- Line 48-51: We copy the functions `SerializeInteralUInt()` and `SerializeInteralInt()` from `Serialization.cpp` and renamed `Serialization` to `PrintValue`
- Line 58-61: We also copy the function `GetDigit()`
- Line 69-83: We also copy the function `BitsToDigits()`
- Line 103-106: We also copy the function `Serialize()` for int32, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 126-129: We also copy the function `Serialize()` for uint32, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 149-152: We also copy the function `Serialize()` for int64, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 172-175: We also copy the function `Serialize()` for uint64, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 187-190: We also copy the function `Serialize()` for long long from `Serialization.h`, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 201-204: We also copy the function `Serialize()` for unsigned long long from `Serialization.h`, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 218-274: We also copy the function `Serialize()` for double, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 287-307: We also copy the function `Serialize()` for const char*, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 330-412: We also copy the function `SerializeInternalInt()` for int32, and rename it to `PrintValueInternalInt()`
- Line 435-509: We also copy the function `SerializeInternalUInt()` for int32, and rename it to `PrintValueInternalUInt()`
- Line 511-545: We leave the `Append` functions as is
- Line 547-560: We implement the new `Format()` function. This uses `FormatNoAllocV()`, and copies the resulting string into a `string` instance, which is returned
- Line 562-570: We implement the new `FormatV()` function. This uses `FormatNoAllocV()`, and copies the resulting string into a `string` instance, which is returned
- Line 572-580: We rename the old `Format()` to `FormatNoAlloc()` and change the call to `FormatNoAllocV()`
- Line 582-841: We rename the old `FormatV()` to `FormatNoAllocV()`. The implementation only changes in the calls to `Serialize()` being replaced with calls to the internal `PrintValue()` functions)

## Updating serializers - Step 2 {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS__STEP_2}

Now we'll change all the `Serialize()` methods to be returning a `string`.
We'll also be adding some more functions to expand the different types supported.

### Serialization.h {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS__STEP_2_SERIALIZATIONH}

We'll change the serialization functions to return a `string`.

Update the file `code/libraries/baremetal/include/baremetal/Serialization.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Serialization.h
...
42: #include <baremetal/String.h>
43: #include <baremetal/Types.h>
44:
45: /// @file
46: /// Type serialization functions
47:
48: namespace baremetal {
49:
50: // Specializations
51:
52: // Every serialization specialization uses a width parameter, which is used for expansion
53: // width < 0 Left aligned
54: // width > 0 right aligned
55: // width < actual length no alignment
56: inline string Serialize(const bool& value)
57: {
58:     return string(value ? "true" : "false");
59: }
60:
61: string Serialize(char value, int width = 0);
62: string Serialize(int8 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
63: string Serialize(uint8 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
64: string Serialize(int16 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
65: string Serialize(uint16 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
66: string Serialize(int32 value, int width= 0, int base = 10, bool showBase = false, bool leadingZeros = false);
67: string Serialize(uint32 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
68: string Serialize(int64 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
69: string Serialize(uint64 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
70:
71: /// <summary>
72: /// Serialize long long int value, type specific specialization
73: /// </summary>
74: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
75: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
76: /// <param name="value">Value to be serialized</param>
77: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
78: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
79: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
80: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
81: inline string Serialize(long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
82: {
83:     return Serialize(static_cast<int64>(value), width, base, showBase, leadingZeros);
84: }
85: /// <summary>
86: /// Serialize unsigned long long int value, type specific specialization
87: /// </summary>
88: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
89: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
90: /// <param name="value">Value to be serialized</param>
91: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
92: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
93: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
94: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
95: inline string Serialize(unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
96: {
97:     return Serialize(static_cast<uint64>(value), width, base, showBase, leadingZeros);
98: }
99: string Serialize(float value, int width = 0, int precision = 16);
100: string Serialize(double value, int width = 0, int precision = 16);
101: string Serialize(const string& value, int width = 0, bool quote = false);
102: string Serialize(const char* value, int width = 0, bool quote = false);
103: string Serialize(const void* value, int width = 0);
104: string Serialize(void* value, int width = 0);
105:
106: } // namespace baremetal
```

- Line 42: We need to include the header for the `string` class, as we will also be implementing some inline functions returning a string
- Line 56-59: We implement the specialization to serialize a boolean as an inline function
- Line 61: We declare the specialization for char
- Line 62-69: We declare the specializations for all integer types. Note that we've added some more to make the list complete
- Line 81-84: We implement the specialization for long long as a inline function. Notice that as before, we simply re-use the version for int64
- Line 95-98: We implement the specialization for unsigned long long as a inline function. Notice that as before, we simply re-use the version for uint64
- Line 99-100: We declare the specializations for floating point types. Note that we've added float here as well
- Line 101-102: We declare the specializations for strings. Note that we've added a serialization for `string` here as well as the `const char*` version
- Line 103-104: We declare the specializations for pointers. Any pointer can be simply cast to either `void*` or `const void*`. Pointers will simply be serialized by printing their address as a hexadecimal value

### Serialization.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS__STEP_2_SERIALIZATIONCPP}

We'll implement the new and updated functions.

Update the file `code/libraries/baremetal/src/Serialization.cpp`

```cpp
File: code/libraries/baremetal/src/Serialization.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Serialization.cpp
5: //
6: // Namespace   : serialization
7: //
8: // Class       : -
9: //
10: // Description : Serialization of types to character buffer
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
40: #include <baremetal/Serialization.h>
41:
42: #include <baremetal/Util.h>
43:
44: /// @file
45: /// Type serialization functions implementation
46:
47: namespace baremetal {
48:
49: /// @brief Write characters with base above 10 as uppercase or not
50: static bool           Uppercase = true;
51:
52: static string SerializeInternalInt(int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
53: static string SerializeInternalUInt(uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
54:
55: /// <summary>
56: /// Convert a value to a digit. Character range is 0..9-A..Z or a..z depending on value of Uppercase
57: /// </summary>
58: /// <param name="value">Digit value</param>
59: /// <returns>Converted digit character</returns>
60: static constexpr char GetDigit(uint8 value)
61: {
62:     return value + ((value < 10) ? '0' : 'A' - 10 + (Uppercase ? 0 : 0x20));
63: }
64:
65: /// <summary>
66: /// Calculated the amount of digits needed to represent an unsigned value of bits using base
67: /// </summary>
68: /// <param name="bits">Size of integer in bits</param>
69: /// <param name="base">Base to be used</param>
70: /// <returns>Maximum amount of digits needed</returns>
71: static constexpr int BitsToDigits(int bits, int base)
72: {
73:     int result = 0;
74:     uint64 value = 0xFFFFFFFFFFFFFFFF;
75:     if (bits < 64)
76:         value &= ((1ULL << bits) - 1);
77:
78:     while (value > 0)
79:     {
80:         value /= base;
81:         result++;
82:     }
83:
84:     return result;
85: }
86:
87: /// <summary>
88: /// Serialize a character value to string.
89: ///
90: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
91: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
92: /// </summary>
93: /// <param name="value">Value to be serialized</param>
94: /// <param name="width">Minimum width in characters, if negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
95: /// <returns>Serialized string value</returns>
96:
97: string Serialize(char value, int width)
98: {
99:     string result;
100:
101:     int    numDigits = 0;
102:     bool   negative = (value < 0);
103:     uint64 absVal = static_cast<uint64>(negative ? -value : value);
104:     uint64 divisor = 1;
105:     int    absWidth = (width < 0) ? -width : width;
106:     while (absVal >= divisor)
107:     {
108:         divisor *= 10;
109:         ++numDigits;
110:     }
111:
112:     if (numDigits == 0)
113:     {
114:         result = "0";
115:         return result;
116:     }
117:     if (negative)
118:     {
119:         result += '-';
120:         absWidth--;
121:     }
122:     while (numDigits > 0)
123:     {
124:         divisor /= 10;
125:         int digit = (absVal / divisor) % 10;
126:         result += GetDigit(digit);
127:         --numDigits;
128:     }
129:     return result.align(width);
130: }
131:
132: /// <summary>
133: /// Serialize a 8 bit signed value to string.
134: ///
135: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
136: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
137: ///
138: /// Base is the digit base, which can range from 2 to 36.
139: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
140: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
141: /// </summary>
142: /// <param name="value">Value to be serialized</param>
143: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
144: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
145: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
146: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
147: /// <returns>Serialized string value</returns>
148: string Serialize(int8 value, int width, int base, bool showBase, bool leadingZeros)
149: {
150:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 8);
151: }
152:
153: /// <summary>
154: /// Serialize a 8 bit unsigned value to string.
155: ///
156: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
157: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
158: ///
159: /// Base is the digit base, which can range from 2 to 36.
160: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
161: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
162: /// </summary>
163: /// <param name="value">Value to be serialized</param>
164: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
165: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
166: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
167: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
168: /// <returns>Serialized string value</returns>
169: string Serialize(uint8 value, int width, int base, bool showBase, bool leadingZeros)
170: {
171:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 8);
172: }
173:
174: /// <summary>
175: /// Serialize a 16 bit signed value to string.
176: ///
177: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
178: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
179: ///
180: /// Base is the digit base, which can range from 2 to 36.
181: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
182: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
183: /// </summary>
184: /// <param name="value">Value to be serialized</param>
185: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
186: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
187: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
188: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
189: /// <returns>Serialized string value</returns>
190: string Serialize(int16 value, int width, int base, bool showBase, bool leadingZeros)
191: {
192:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 16);
193: }
194:
195: /// <summary>
196: /// Serialize a 16 bit unsigned value to string.
197: ///
198: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
199: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
200: ///
201: /// Base is the digit base, which can range from 2 to 36.
202: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
203: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
204: /// </summary>
205: /// <param name="value">Value to be serialized</param>
206: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
207: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
208: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
209: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
210: /// <returns>Serialized string value</returns>
211: string Serialize(uint16 value, int width, int base, bool showBase, bool leadingZeros)
212: {
213:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 16);
214: }
215:
216: /// <summary>
217: /// Serialize a 32 bit signed value to string.
218: ///
219: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
220: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
221: ///
222: /// Base is the digit base, which can range from 2 to 36.
223: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
224: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
225: /// </summary>
226: /// <param name="value">Value to be serialized</param>
227: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
228: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
229: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
230: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
231: /// <returns>Serialized string value</returns>
232: string Serialize(int32 value, int width, int base, bool showBase, bool leadingZeros)
233: {
234:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 32);
235: }
236:
237: /// <summary>
238: /// Serialize a 32 bit unsigned value to string.
239: ///
240: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
241: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
242: ///
243: /// Base is the digit base, which can range from 2 to 36.
244: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
245: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
246: /// </summary>
247: /// <param name="value">Value to be serialized</param>
248: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
249: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
250: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
251: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
252: /// <returns>Serialized string value</returns>
253: string Serialize(uint32 value, int width, int base, bool showBase, bool leadingZeros)
254: {
255:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 32);
256: }
257:
258: /// <summary>
259: /// Serialize a 64 bit signed value to string.
260: ///
261: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
262: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
263: ///
264: /// Base is the digit base, which can range from 2 to 36.
265: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
266: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
267: /// </summary>
268: /// <param name="value">Value to be serialized</param>
269: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
270: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
271: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
272: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
273: /// <returns>Serialized string value</returns>
274: string Serialize(int64 value, int width, int base, bool showBase, bool leadingZeros)
275: {
276:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 64);
277: }
278:
279: /// <summary>
280: /// Serialize a 64 bit unsigned value to string.
281: ///
282: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
283: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
284: ///
285: /// Base is the digit base, which can range from 2 to 36.
286: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
287: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
288: /// </summary>
289: /// <param name="value">Value to be serialized</param>
290: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
291: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
292: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
293: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
294: /// <returns>Serialized string value</returns>
295: string Serialize(uint64 value, int width, int base, bool showBase, bool leadingZeros)
296: {
297:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 64);
298: }
299:
300: /// <summary>
301: /// Serialize a float value to string. The value will be printed as a fixed point number.
302: ///
303: /// Width specifies the minimum width in characters. The value is written right aligned if width is positive, left aligned if width is negative.
304: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
305: ///
306: /// Precision specifies the number of digits behind the decimal pointer
307: /// </summary>
308: /// <param name="value">Value to be serialized</param>
309: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
310: /// <param name="precision">Number of digits after the decimal point to use (limited to 7 decimals</param>
311: /// <returns>Serialized string value</returns>
312: string Serialize(float value, int width, int precision)
313: {
314:     bool negative{};
315:     if (value < 0)
316:     {
317:         negative = true;
318:         value = -value;
319:     }
320:
321:     // We can only print values with integral parts up to what uint64 can hold
322:     if (value > static_cast<float>(static_cast<uint64>(-1)))
323:     {
324:         return string("overflow");
325:     }
326:
327:     string result;
328:     if (negative)
329:         result += '-';
330:
331:     uint64 integralPart = static_cast<uint64>(value);
332:     result += Serialize(integralPart, 0, 10, false, false);
333:     const int MaxPrecision = 7;
334:
335:     if (precision != 0)
336:     {
337:         result += '.';
338:
339:         if (precision > MaxPrecision)
340:         {
341:             precision = MaxPrecision;
342:         }
343:
344:         uint64 precisionPower10 = 1;
345:         for (int i = 1; i <= precision; i++)
346:         {
347:             precisionPower10 *= 10;
348:         }
349:
350:         value -= static_cast<float>(integralPart);
351:         value *= static_cast<float>(precisionPower10);
352:
353:         string fractional = Serialize(static_cast<uint64>(value + 0.5F), 0, 10, false, false);
354:         result += fractional;
355:         precision -= fractional.length();
356:         while (precision--)
357:         {
358:             result += '0';
359:         }
360:     }
361:     return result.align(width);
362: }
363:
364: /// <summary>
365: /// Serialize a double value to string. The value will be printed as a fixed point number.
366: ///
367: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
368: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
369: ///
370: /// Precision specifies the number of digits behind the decimal pointer
371: /// </summary>
372: /// <param name="value">Value to be serialized</param>
373: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
374: /// <param name="precision">Number of digits after the decimal point to use (limited to 10 decimals</param>
375: /// <returns>Serialized string value</returns>
376: string Serialize(double value, int width, int precision)
377: {
378:     bool negative{};
379:     if (value < 0)
380:     {
381:         negative = true;
382:         value = -value;
383:     }
384:
385:     // We can only print values with integral parts up to what uint64 can hold
386:     if (value > static_cast<double>(static_cast<uint64>(-1)))
387:     {
388:         return string("overflow");
389:     }
390:
391:     string result;
392:     if (negative)
393:         result += '-';
394:
395:     uint64 integralPart = static_cast<uint64>(value);
396:     result += Serialize(integralPart, 0, 10, false, false);
397:     const int MaxPrecision = 14;
398:
399:     if (precision != 0)
400:     {
401:         result += '.';
402:
403:         if (precision > MaxPrecision)
404:         {
405:             precision = MaxPrecision;
406:         }
407:
408:         uint64 precisionPower10 = 1;
409:         for (int i = 1; i <= precision; i++)
410:         {
411:             precisionPower10 *= 10;
412:         }
413:
414:         value -= static_cast<double>(integralPart);
415:         value *= static_cast<double>(precisionPower10);
416:
417:         string fractional = Serialize(static_cast<uint64>(value + 0.5), 0, 10, false, false);
418:         result += fractional;
419:         precision -= fractional.length();
420:         while (precision--)
421:         {
422:             result += '0';
423:         }
424:     }
425:     return result.align(width);
426: }
427:
428: /// <summary>
429: /// Serialize a string to string.
430: ///
431: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
432: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
433: ///
434: /// If requested, the string is placed between double quotes (").
435: /// </summary>
436: /// <param name="value">Value to be serialized</param>
437: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
438: /// <param name="quote">If true places string between double quotes</param>
439: /// <returns>Serialized string value</returns>
440: string Serialize(const string& value, int width, bool quote)
441: {
442:     string result;
443:
444:     if (quote)
445:         result += '\"';
446:     result += value;
447:     if (quote)
448:         result += '\"';
449:
450:     return result.align(width);
451: }
452:
453: /// <summary>
454: /// Serialize a string to string.
455: ///
456: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
457: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
458: ///
459: /// If requested, the string is placed between double quotes (").
460: /// </summary>
461: /// <param name="value">Value to be serialized</param>
462: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
463: /// <param name="quote">If true places string between double quotes</param>
464: /// <returns>Serialized string value</returns>
465: string Serialize(const char* value, int width, bool quote)
466: {
467:     return Serialize(string(value), width, quote);
468: }
469:
470: /// <summary>
471: /// Serialize a const void pointer to string.
472: ///
473: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
474: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
475: /// </summary>
476: /// <param name="value">Value to be serialized</param>
477: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
478: /// <returns>Serialized string value</returns>
479: string Serialize(const void* value, int width)
480: {
481:     string result;
482:
483:     if (value != nullptr)
484:     {
485:         result = Serialize(reinterpret_cast<uintptr>(value), 16, 16, true, true);
486:     }
487:     else
488:     {
489:         result = "null";
490:     }
491:
492:     return result.align(width);
493: }
494:
495: /// <summary>
496: /// Serialize a void pointer to string.
497: ///
498: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
499: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
500: /// </summary>
501: /// <param name="value">Value to be serialized</param>
502: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
503: /// <returns>Serialized string value</returns>
504: string Serialize(void* value, int width)
505: {
506:     return Serialize(const_cast<const void*>(value), width);
507: }
508:
509: /// <summary>
510: /// Internal serialization function returning string, to be used for all signed values.
511: ///
512: /// Serialize a signed value to string.
513: ///
514: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
515: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
516: ///
517: /// Base is the digit base, which can range from 2 to 36.
518: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
519: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
520: /// </summary>
521: /// <param name="value">Value to be serialized</param>
522: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
523: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
524: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
525: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
526: /// <param name="numBits">Specifies the number of bits used for the value</param>
527: /// <returns>Serialized stirng</returns>
528: static string SerializeInternalInt(int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
529: {
530:     if ((base < 2) || (base > 36))
531:         return {};
532:
533:     int       numDigits = 0;
534:     bool      negative = (value < 0);
535:     uint64    absVal = static_cast<uint64>(negative ? -value : value);
536:     uint64    divisor = 1;
537:     uint64    divisorHigh = 0;
538:     uint64    divisorLast = 1;
539:     size_t    absWidth = (width < 0) ? -width : width;
540:     const int maxDigits = BitsToDigits(numBits, base);
541:     while ((divisorHigh == 0) && (absVal >= divisor) && (numDigits <= maxDigits))
542:     {
543:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
544:         divisorLast = divisor;
545:         divisor *= base;
546:         ++numDigits;
547:     }
548:     divisor = divisorLast;
549:
550:     size_t numChars = (numDigits > 0) ? numDigits : 1;
551:     if (showBase)
552:     {
553:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
554:     }
555:     if (negative)
556:     {
557:         numChars++;
558:     }
559:     if (absWidth > numChars)
560:         numChars = absWidth;
561:     // Leave one character for \0
562:     string result;
563:     result.reserve(numChars + 1);
564:
565:     if (negative)
566:     {
567:         result += '-';
568:     }
569:
570:     if (showBase)
571:     {
572:         if (base == 2)
573:         {
574:             result += "0b";
575:         }
576:         else if (base == 8)
577:         {
578:             result += '0';
579:         }
580:         else if (base == 16)
581:         {
582:             result += "0x";
583:         }
584:     }
585:     if (leadingZeros)
586:     {
587:         if (absWidth == 0)
588:             absWidth = maxDigits;
589:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
590:         {
591:             result += '0';
592:         }
593:     }
594:     else
595:     {
596:         if (numDigits == 0)
597:         {
598:             result += '0';
599:         }
600:     }
601:     while (numDigits > 0)
602:     {
603:         int digit = (absVal / divisor) % base;
604:         result += GetDigit(digit);
605:         --numDigits;
606:         divisor /= base;
607:     }
608:     return result.align(width);
609: }
610:
611: /// <summary>
612: /// Internal serialization function returning string, to be used for all unsigned values.
613: ///
614: /// Serialize a unsigned value to string.
615: ///
616: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
617: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
618: ///
619: /// Base is the digit base, which can range from 2 to 36.
620: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
621: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
622: /// </summary>
623: /// <param name="value">Value to be serialized</param>
624: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
625: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
626: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
627: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
628: /// <param name="numBits">Specifies the number of bits used for the value</param>
629: /// <returns>Serialized stirng</returns>
630: static string SerializeInternalUInt(uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
631: {
632:     if ((base < 2) || (base > 36))
633:         return {};
634:
635:     int       numDigits = 0;
636:     uint64    divisor = 1;
637:     uint64    divisorLast = 1;
638:     uint64    divisorHigh = 0;
639:     size_t    absWidth = (width < 0) ? -width : width;
640:     const int maxDigits = BitsToDigits(numBits, base);
641:     while ((divisorHigh == 0) && (value >= divisor) && (numDigits <= maxDigits))
642:     {
643:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
644:         divisorLast = divisor;
645:         divisor *= base;
646:         ++numDigits;
647:     }
648:     divisor = divisorLast;
649:
650:     size_t numChars = (numDigits > 0) ? numDigits : 1;
651:     if (showBase)
652:     {
653:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
654:     }
655:     if (absWidth > numChars)
656:         numChars = absWidth;
657:     // Leave one character for \0
658:     string result;
659:     result.reserve(numChars + 1);
660:
661:     if (showBase)
662:     {
663:         if (base == 2)
664:         {
665:             result += "0b";
666:         }
667:         else if (base == 8)
668:         {
669:             result += '0';
670:         }
671:         else if (base == 16)
672:         {
673:             result += "0x";
674:         }
675:     }
676:     if (leadingZeros)
677:     {
678:         if (absWidth == 0)
679:             absWidth = maxDigits;
680:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
681:         {
682:             result += '0';
683:         }
684:     }
685:     else
686:     {
687:         if (numDigits == 0)
688:         {
689:             result += '0';
690:         }
691:     }
692:     while (numDigits > 0)
693:     {
694:         int digit = (value / divisor) % base;
695:         result += GetDigit(digit);
696:         --numDigits;
697:         divisor /= base;
698:     }
699:     return result.align(width);
700: }
701:
702: } // namespace baremetal
```

- Line 97-130: We implement the char specialization
- Line 147-150: We implement the int8 specialization
- Line 169-172: We implement the uint8 specialization
- Line 190-193: We implement the int16 specialization
- Line 211-214: We implement the uint16 specialization
- Line 232-235: We implement the int32 specialization
- Line 253-256: We implement the uint32 specialization
- Line 274-277: We implement the int64 specialization
- Line 295-298: We implement the uint64 specialization
- Line 312-362: We implement the float specialization
- Line 376-426: We implement the double specialization
- Line 440-451: We implement the string specialization
- Line 465-467: We implement the const char* specialization
- Line 479-493: We implement the const void* specialization
- Line 504-507: We implement the const char* specialization
- Line 504-507: We implement the const char* specialization
- Line 528-609: We implement internal `SerializeInternalInt()` function
- Line 630-700: We implement internal `SerializeInternalUInt()` function

### HeapAllocator.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS__STEP_2_HEAPALLOCATORCPP}

As the `HeapAllocator` class implements part of memory management, if we wish to print debug information, we need to do so without using memory allocation.

Update the file `code/libraries/baremetal/src/HeapAllocator.cpp`

```cpp
File: code/libraries/baremetal/src/HeapAllocator.cpp
...
109: void* HeapAllocator::Allocate(size_t size)
110: {
...
141: #if BAREMETAL_MEMORY_TRACING_DETAIL
142:         LOG_NO_ALLOC_DEBUG("Reuse %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
143:         LOG_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
144: #endif
...
153:         if ((nextBlock <= m_next) ||                    // may have wrapped
154:             (nextBlock > m_limit - m_reserve))
155:         {
156: #if BAREMETAL_MEMORY_TRACING
157:             DumpStatus();
158: #endif
159:             LOG_NO_ALLOC_ERROR("%s: Out of memory", m_heapName);
160:             return nullptr;
161:         }
162:
...
168: #if BAREMETAL_MEMORY_TRACING_DETAIL
169:         LOG_NO_ALLOC_DEBUG("Allocate %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
170:         LOG_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
171: #endif
...
180: }
...
227: void HeapAllocator::Free(void* block)
228: {
...
248: #if BAREMETAL_MEMORY_TRACING_DETAIL
249:             LOG_NO_ALLOC_DEBUG("Free %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
250:             LOG_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
251: #endif
252: #endif
253:
254:             return;
255:         }
256:     }
257:
258: #if BAREMETAL_MEMORY_TRACING
259:     LOG_NO_ALLOC_WARNING("%s: Trying to free large block (size %lu)", m_heapName, blockHeader->size);
260: #endif
261: }
262:
263: #if BAREMETAL_MEMORY_TRACING
264: /// <summary>
265: /// Display the current status of the heap allocator
266: /// </summary>
267: void HeapAllocator::DumpStatus()
268: {
269:     LOG_NO_ALLOC_DEBUG("Heap allocator info:     %s", m_heapName);
270:     LOG_NO_ALLOC_DEBUG("Current #allocations:    %llu", GetCurrentAllocatedBlockCount());
271:     LOG_NO_ALLOC_DEBUG("Max #allocations:        %llu", GetMaxAllocatedBlockCount());
272:     LOG_NO_ALLOC_DEBUG("Current #allocated bytes:%llu", GetCurrentAllocationSize());
273:     LOG_NO_ALLOC_DEBUG("Total #allocated blocks: %llu", GetTotalAllocatedBlockCount());
274:     LOG_NO_ALLOC_DEBUG("Total #allocated bytes:  %llu", GetTotalAllocationSize());
275:     LOG_NO_ALLOC_DEBUG("Total #freed blocks:     %llu", GetTotalFreedBlockCount());
276:     LOG_NO_ALLOC_DEBUG("Total #freed bytes:      %llu", GetTotalFreeSize());
277:
278:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
279:     {
280:         LOG_NO_ALLOC_DEBUG("malloc(%lu): %lu blocks (max %lu) total alloc #blocks = %llu, #bytes = %llu, total free #blocks = %llu, #bytes = %llu",
281:             bucket->size, bucket->count, bucket->maxCount, bucket->totalAllocatedCount, bucket->totalAllocated, bucket->totalFreedCount, bucket->totalFreed);
282:     }
283: }
...
383: #endif
384:
```

### String.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS__STEP_2_STRINGCPP}

It is convenient to get some more information if desired on which memory allocations and de-allocations are done by strings, so we'll add some logging there.

Update the file `code/libraries/baremetal/src/String.cpp`

```cpp
File: code/libraries/baremetal/src/String.cpp
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44: #include <baremetal/Util.h>
...
59: LOG_MODULE("String");
60:
61: /// <summary>
62: /// Default constructor
63: ///
64: /// Constructs an empty string.
65: /// </summary>
66: string::string()
67:     : m_buffer{}
68:     , m_end{}
69:     , m_allocatedSize{}
70: {
71: }
72:
73: /// <summary>
74: /// Destructor
75: ///
76: /// Frees any allocated memory.
77: /// </summary>
78: string::~string()
79: {
80: #if BAREMETAL_MEMORY_TRACING_DETAIL
81:     if (m_buffer != nullptr)
82:         LOG_NO_ALLOC_DEBUG("Free string %p", m_buffer);
83: #endif
84:     delete[] m_buffer;
85: }
...
1433: bool string::reallocate_allocation_size(size_t allocationSize)
1434: {
1435:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1436:     if (newBuffer == nullptr)
1437:     {
1438:         LOG_NO_ALLOC_DEBUG("Alloc failed!");
1439:         return false;
1440:     }
1441:     m_buffer = newBuffer;
1442: #if BAREMETAL_MEMORY_TRACING_DETAIL
1443:     LOG_NO_ALLOC_DEBUG("Alloc string %p", m_buffer);
1444: #endif
1445:     if (m_end == nullptr)
1446:         m_end = m_buffer;
1447:     if (m_end > m_buffer + allocationSize)
1448:         m_end = m_buffer + allocationSize;
1449:     m_allocatedSize = allocationSize;
1450:     return true;
1451: }
```

### System.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS__STEP_2_SYSTEMCPP}

We print an overview of the status of memory management at startup (if `BAREMETAL_MEMORY_TRACING` is defined).
If would be even more helpful, if we would print the memory management status just before halting or rebooting, so we can see whether there is any memory leaked

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
200:     GetLogger();
201:     LOG_INFO("Starting up");
202:
203:     extern int main();
204:
205:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
206:     {
207: #if BAREMETAL_MEMORY_TRACING
208:         GetMemoryManager().DumpStatus();
209: #endif
210:         GetSystem().Reboot();
211:     }
212:
213: #if BAREMETAL_MEMORY_TRACING
214:     GetMemoryManager().DumpStatus();
215: #endif
216:     GetSystem().Halt();
217: }
...
```

### Version.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS__STEP_2_VERSIONCPP}

The version string is composed using a call to `Format()`. We need to again use the non-allocating version.

Update the file `code/libraries/baremetal/src/Version.cpp`

```cpp
File: code/libraries/baremetal/src/Version.cpp
40: #include <baremetal/Version.h>
41:
42: #include <baremetal/Format.h>
43: #include <baremetal/String.h>
44: #include <baremetal/Util.h>
45:
46: static const size_t BufferSize = 20;
47: static char s_baremetalVersionString[BufferSize]{};
48: static bool s_baremetalVersionSetupDone = false;
49:
50: void baremetal::SetupVersion()
51: {
52:     if (!s_baremetalVersionSetupDone)
53:     {
54:         FormatNoAlloc(s_baremetalVersionString, BufferSize, "%d.%d.%d", BAREMETAL_MAJOR_VERSION, BAREMETAL_MINOR_VERSION, BAREMETAL_PATCH_VERSION);
55:         s_baremetalVersionSetupDone = true;
56:     }
57: }
58:
59: const char* baremetal::GetVersion()
60: {
61:     return s_baremetalVersionString;
62: }
```

## Application code {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_APPLICATION_CODE}

We'll add some tests for check whether the serialization functions work as expected.
This is the second attempt at creating class / micro / unit tests for our code, which we will start doing in the next tutorial.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include <baremetal/ARMInstructions.h>
2: #include <baremetal/Assert.h>
3: #include <baremetal/BCMRegisters.h>
4: #include <baremetal/Console.h>
5: #include <baremetal/Logger.h>
6: #include <baremetal/Mailbox.h>
7: #include <baremetal/MemoryManager.h>
8: #include <baremetal/New.h>
9: #include <baremetal/RPIProperties.h>
10: #include <baremetal/Serialization.h>
11: #include <baremetal/String.h>
12: #include <baremetal/SysConfig.h>
13: #include <baremetal/System.h>
14: #include <baremetal/Timer.h>
15: #include <baremetal/Util.h>
16:
17: LOG_MODULE("main");
18:
19: using namespace baremetal;
20:
21: int main()
22: {
23:     auto& console = GetConsole();
24:     LOG_DEBUG("Hello World!");
25:
26:     char c = 'A';
27:     assert(Serialize(c) ==                          "65");
28:     assert(Serialize(c, 4) ==                       "  65");
29:     int8 i8 = 123;
30:     assert(Serialize(i8) ==                         "123");
31:     assert(Serialize(i8, 8) ==                      "     123");
32:     assert(Serialize(i8, -8) ==                     "123     ");
33:     assert(Serialize(i8, 8, 16) ==                  "      7B");
34:     assert(Serialize(i8, 8, 16, true) ==            "    0x7B");
35:     assert(Serialize(i8, 8, 16, true, true) ==      "0x0000007B");
36:     uint8 u8 = 234;
37:     assert(Serialize(u8) ==                         "234");
38:     assert(Serialize(u8, 8) ==                      "     234");
39:     assert(Serialize(u8, -8) ==                     "234     ");
40:     assert(Serialize(u8, 8, 16) ==                  "      EA");
41:     assert(Serialize(u8, 8, 16, true) ==            "    0xEA");
42:     assert(Serialize(u8, 8, 16, true, true) ==      "0x000000EA");
43:     int16 i16 = 12345;
44:     assert(Serialize(i16) ==                        "12345");
45:     assert(Serialize(i16, 8) ==                     "   12345");
46:     assert(Serialize(i16, -8) ==                    "12345   ");
47:     assert(Serialize(i16, 8, 16) ==                 "    3039");
48:     assert(Serialize(i16, 8, 16, true) ==           "  0x3039");
49:     assert(Serialize(i16, 8, 16, true, true) ==     "0x00003039");
50:     uint16 u16 = 34567;
51:     assert(Serialize(u16) ==                        "34567");
52:     assert(Serialize(u16, 8) ==                     "   34567");
53:     assert(Serialize(u16, -8) ==                    "34567   ");
54:     assert(Serialize(u16, 8, 16) ==                 "    8707");
55:     assert(Serialize(u16, 8, 16, true) ==           "  0x8707");
56:     assert(Serialize(u16, 8, 16, true, true) ==     "0x00008707");
57:     int32 i32 = 1234567890l;
58:     assert(Serialize(i32) ==                        "1234567890");
59:     assert(Serialize(i32, 12) ==                    "  1234567890");
60:     assert(Serialize(i32, -12) ==                   "1234567890  ");
61:     assert(Serialize(i32, 12, 16) ==                "    499602D2");
62:     assert(Serialize(i32, 12, 16, true) ==          "  0x499602D2");
63:     assert(Serialize(i32, 12, 16, true, true) ==    "0x0000499602D2");
64:     uint32 u32 = 2345678900ul;
65:     assert(Serialize(u32) ==                        "2345678900");
66:     assert(Serialize(u32, 12) ==                    "  2345678900");
67:     assert(Serialize(u32, -12) ==                   "2345678900  ");
68:     assert(Serialize(u32, 12, 16) ==                "    8BD03834");
69:     assert(Serialize(u32, 12, 16, true) ==          "  0x8BD03834");
70:     assert(Serialize(u32, 12, 16, true, true) ==    "0x00008BD03834");
71:     int64 i64 = 9223372036854775807ll;
72:     assert(Serialize(i64) ==                        "9223372036854775807");
73:     assert(Serialize(i64, 20) ==                    " 9223372036854775807");
74:     assert(Serialize(i64, -20) ==                   "9223372036854775807 ");
75:     assert(Serialize(i64, 20, 16) ==                "    7FFFFFFFFFFFFFFF");
76:     assert(Serialize(i64, 20, 16, true) ==          "  0x7FFFFFFFFFFFFFFF");
77:     assert(Serialize(i64, 20, 16, true, true) ==    "0x00007FFFFFFFFFFFFFFF");
78:     uint64 u64 = 9223372036854775808ull;
79:     assert(Serialize(u64) ==                        "9223372036854775808");
80:     assert(Serialize(u64, 20) ==                    " 9223372036854775808");
81:     assert(Serialize(u64, -20) ==                   "9223372036854775808 ");
82:     assert(Serialize(u64, 20, 16) ==                "    8000000000000000");
83:     assert(Serialize(u64, 20, 16, true) ==          "  0x8000000000000000");
84:     assert(Serialize(u64, 20, 16, true, true) ==    "0x00008000000000000000");
85:     float f = 1.23456789F;
86:     assert(Serialize(f) ==                          "1.2345679");
87:     assert(Serialize(f, 12) ==                      "   1.2345679");
88:     assert(Serialize(f, -12) ==                     "1.2345679   ");
89:     assert(Serialize(f, 12, 2) ==                   "        1.23");
90:     assert(Serialize(f, 12, 7) ==                   "   1.2345679");
91:     assert(Serialize(f, 12, 8) ==                   "   1.2345679");
92:
93:     double d = 1.234567890123456;
94:     assert(Serialize(d) ==                          "1.23456789012346");
95:     assert(Serialize(d, 18) ==                      "  1.23456789012346");
96:     assert(Serialize(d, -18) ==                     "1.23456789012346  ");
97:     assert(Serialize(d, 18, 5) ==                   "           1.23457");
98:     assert(Serialize(d, 18, 7) ==                   "         1.2345679");
99:     assert(Serialize(d, 18, 12) ==                  "    1.234567890123");
100:
101:     string s = "hello world";
102:     assert(Serialize(s) ==                          "hello world");
103:     assert(Serialize(s, 15) ==                      "    hello world");
104:     assert(Serialize(s, -15) ==                     "hello world    ");
105:     assert(Serialize(s, 15, true) ==                "  \"hello world\"");
106:
107:     const char* str = "hello world";
108:     assert(Serialize(str) ==                        "hello world");
109:     assert(Serialize(str, 15) ==                    "    hello world");
110:     assert(Serialize(str, -15) ==                   "hello world    ");
111:     assert(Serialize(str, 15, true) ==              "  \"hello world\"");
112:
113:     const void* pvc = reinterpret_cast<const void*>(0x0123456789ABCDEF);
114:     assert(Serialize(pvc) ==                        "0x0123456789ABCDEF");
115:     assert(Serialize(pvc, 20) ==                    "  0x0123456789ABCDEF");
116:     assert(Serialize(pvc, -20) ==                   "0x0123456789ABCDEF  ");
117:
118:     void* pv = reinterpret_cast<void*>(0x0123456789ABCDEF);
119:     assert(Serialize(pv) == "0x0123456789ABCDEF");
120:     assert(Serialize(pv, 20) == "  0x0123456789ABCDEF");
121:     assert(Serialize(pv, -20) == "0x0123456789ABCDEF  ");
122:
123:     LOG_INFO("Wait 5 seconds");
124:     Timer::WaitMilliSeconds(5000);
125:
126:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
127:     char ch{};
128:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
129:     {
130:         ch = console.ReadChar();
131:         console.WriteChar(ch);
132:     }
133:     if (ch == 'p')
134:         assert(false);
135:
136:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
137: }
```

## Update project configuration {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATE_PROJECT_CONFIGURATION}

As we did not add any new files, we don't need to update the CMake file.

## Configuring, building and debugging {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

You will see an overview of memory management before and after `main()` was run, as we still have `BAREMETAL_MEMORY_TRACING` defined.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:80)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        0 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 0 (HeapAllocator:273)
Debug  Total #allocated bytes:  0 (HeapAllocator:274)
Debug  Total #freed blocks:     0 (HeapAllocator:275)
Debug  Total #freed bytes:      0 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Starting up (System:201)
Debug  Hello World! (main:24)
Info   Wait 5 seconds (main:123)
Press r to reboot, h to halt, p to fail assertion and panic
hDebug  Low heap: (MemoryManager:222)
Debug  Heap allocator info:     heaplow (HeapAllocator:269)
Debug  Current #allocations:    0 (HeapAllocator:270)
Debug  Max #allocations:        5 (HeapAllocator:271)
Debug  Current #allocated bytes:0 (HeapAllocator:272)
Debug  Total #allocated blocks: 276 (HeapAllocator:273)
Debug  Total #allocated bytes:  17664 (HeapAllocator:274)
Debug  Total #freed blocks:     276 (HeapAllocator:275)
Debug  Total #freed bytes:      17664 (HeapAllocator:276)
Debug  malloc(64): 0 blocks (max 5) total alloc #blocks = 276, #bytes = 17664, total free #blocks = 276, #bytes = 17664 (HeapAllocator:280)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:280)
Info   Halt (System:122)
```

As you can see we performeed a total of 276 allocations, all blocks were freed again, and all block were 64 bytes max in size.
At any moment, there were a maximum of 5 blocks allocated.

Next: [17-unit-tests](17-unit-tests.md)

