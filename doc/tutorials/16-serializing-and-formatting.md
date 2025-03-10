# Tutorial 16: Serialization and formatting {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING}

@tableofcontents

## New tutorial setup {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/16-serialization-and-formatting`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-16.a`
- a library `output/Debug/lib/stdlib-16.a`
- an application `output/Debug/bin/16-serialization-and-formatting.elf`
- an image in `deploy/Debug/16-serialization-and-formatting-image`

## Memory allocating and non memory allocating printing - Step 1 {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1}

It would be nice if we could start using our `string` class for logging, serialization, etc.
However, sometimes we want to print things before the memory manager is initialized.
So we'll want to have a non memory-allocating version next to the new memory-allocating, string based version.

We'll first update the `Logger` to support both versions, and we'll move down from there.

### Logger.h {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_LOGGERH}

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
```

### Logger.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_LOGGERCPP}

We'll implement the new methods.
We'll also use the non memory-allocating macro in the `Initialize()` method.

Update the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
...
42: #include <stdlib/Util.h>
43: #include <baremetal/Console.h>
44: #include <baremetal/Format.h>
45: #include <baremetal/MachineInfo.h>
46: #include <baremetal/String.h>
47: #include <baremetal/System.h>
48: #include <baremetal/Timer.h>
49: #include <baremetal/Version.h>
...
73: /// <summary>
74: /// Initialize logger
75: /// </summary>
76: /// <returns>true on succes, false on failure</returns>
77: bool Logger::Initialize()
78: {
79:     if (m_initialized)
80:         return true;
81:     SetupVersion();
82:     m_initialized = true; // Stop reentrant calls from happening
83:     LOG_NO_ALLOC_INFO(BAREMETAL_NAME " %s started on %s (AArch64) using %s SoC", BAREMETAL_VERSION_STRING, GetMachineInfo().GetName(), GetMachineInfo().GetSoCName());
84: 
85:     return true;
86: }
...
112: /// <summary>
113: /// Write a string with variable arguments to the logger
114: /// </summary>
115: /// <param name="source">Source name or file name</param>
116: /// <param name="line">Source line number</param>
117: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
118: /// <param name="message">Formatted message string</param>
119: /// <param name="args">Variable argument list</param>
120: void Logger::WriteV(const char *source, int line, LogSeverity severity, const char *message, va_list args)
121: {
122:     if (static_cast<int>(severity) > static_cast<int>(m_level))
123:         return;
124: 
125:     string lineBuffer;
126: 
127:     auto sourceString = Format(" (%s:%d)", source, line);
128: 
129:     auto messageBuffer = FormatV(message, args);
130: 
131:     switch (severity)
132:     {
133:     case LogSeverity::Panic:
134:         lineBuffer += "!Panic!";
135:         break;
136:     case LogSeverity::Error:
137:         lineBuffer += "Error  ";
138:         break;
139:     case LogSeverity::Warning:
140:         lineBuffer += "Warning";
141:         break;
142:     case LogSeverity::Info:
143:         lineBuffer += "Info   ";
144:         break;
145:     case LogSeverity::Debug:
146:         lineBuffer += "Debug  ";
147:         break;
148:     }
149: 
150:     if (m_timer != nullptr)
151:     {
152:         const size_t TimeBufferSize = 32;
153:         char timeBuffer[TimeBufferSize]{};
154:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
155:         if (strlen(timeBuffer) > 0)
156:         {
157:             lineBuffer += timeBuffer;
158:             lineBuffer += ' ';
159:         }
160:     }
161: 
162:     lineBuffer += messageBuffer;
163:     lineBuffer += sourceString;
164:     lineBuffer += "\n";
165: 
166: #if BAREMETAL_COLOR_OUTPUT
167:     switch (severity)
168:     {
169:     case LogSeverity::Panic:
170:         m_console.Write(lineBuffer, ConsoleColor::BrightRed);
171:         break;
172:     case LogSeverity::Error:
173:         m_console.Write(lineBuffer, ConsoleColor::Red);
174:         break;
175:     case LogSeverity::Warning:
176:         m_console.Write(lineBuffer, ConsoleColor::BrightYellow);
177:         break;
178:     case LogSeverity::Info:
179:         m_console.Write(lineBuffer, ConsoleColor::Cyan);
180:         break;
181:     case LogSeverity::Debug:
182:         m_console.Write(lineBuffer, ConsoleColor::Yellow);
183:         break;
184:     default:
185:         m_console.Write(lineBuffer, ConsoleColor::White);
186:         break;
187:     }
188: #else
189:     m_console.Write(lineBuffer);
190: #endif
191: 
192:     if (severity == LogSeverity::Panic)
193:     {
194:         GetSystem().Halt();
195:     }
196: }
197: 
198: /// <summary>
199: /// Write a string with variable arguments to the logger
200: /// </summary>
201: /// <param name="source">Source name or file name</param>
202: /// <param name="line">Source line number</param>
203: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
204: /// <param name="message">Formatted message string, with variable arguments</param>
205: void Logger::WriteNoAlloc(const char* source, int line, LogSeverity severity, const char* message, ...)
206: {
207:     va_list var;
208:     va_start(var, message);
209:     WriteNoAllocV(source, line, severity, message, var);
210:     va_end(var);
211: }
212: 
213: /// <summary>
214: /// Write a string with variable arguments to the logger
215: /// </summary>
216: /// <param name="source">Source name or file name</param>
217: /// <param name="line">Source line number</param>
218: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
219: /// <param name="message">Formatted message string</param>
220: /// <param name="args">Variable argument list</param>
221: void Logger::WriteNoAllocV(const char* source, int line, LogSeverity severity, const char* message, va_list args)
222: {
223:     if (static_cast<int>(severity) > static_cast<int>(m_level))
224:         return;
225: 
226:     static const size_t BufferSize = 1024;
227:     char buffer[BufferSize]{};
228: 
229:     char sourceString[BufferSize]{};
230:     FormatNoAlloc(sourceString, BufferSize, " (%s:%d)", source, line);
231: 
232:     char messageBuffer[BufferSize]{};
233:     FormatNoAllocV(messageBuffer, BufferSize, message, args);
234: 
235:     switch (severity)
236:     {
237:     case LogSeverity::Panic:
238:         strncat(buffer, "!Panic!", BufferSize);
239:         break;
240:     case LogSeverity::Error:
241:         strncat(buffer, "Error  ", BufferSize);
242:         break;
243:     case LogSeverity::Warning:
244:         strncat(buffer, "Warning", BufferSize);
245:         break;
246:     case LogSeverity::Info:
247:         strncat(buffer, "Info   ", BufferSize);
248:         break;
249:     case LogSeverity::Debug:
250:         strncat(buffer, "Debug  ", BufferSize);
251:         break;
252:     }
253: 
254:     if (m_timer != nullptr)
255:     {
256:         const size_t TimeBufferSize = 32;
257:         char timeBuffer[TimeBufferSize]{};
258:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
259:         if (strlen(timeBuffer) > 0)
260:         {
261:             strncat(buffer, timeBuffer, BufferSize);
262:             strncat(buffer, " ", BufferSize);
263:         }
264:     }
265: 
266:     strncat(buffer, messageBuffer, BufferSize);
267:     strncat(buffer, sourceString, BufferSize);
268:     strncat(buffer, "\n", BufferSize);
269: 
270: #if BAREMETAL_COLOR_OUTPUT
271:     switch (severity)
272:     {
273:     case LogSeverity::Panic:
274:         m_console.Write(buffer, ConsoleColor::BrightRed);
275:         break;
276:     case LogSeverity::Error:
277:         m_console.Write(buffer, ConsoleColor::Red);
278:         break;
279:     case LogSeverity::Warning:
280:         m_console.Write(buffer, ConsoleColor::BrightYellow);
281:         break;
282:     case LogSeverity::Info:
283:         m_console.Write(buffer, ConsoleColor::Cyan);
284:         break;
285:     case LogSeverity::Debug:
286:         m_console.Write(buffer, ConsoleColor::Yellow);
287:         break;
288:     default:
289:         m_console.Write(buffer, ConsoleColor::White);
290:         break;
291:     }
292: #else
293:     m_console.Write(buffer);
294: #endif
295: 
296:     if (severity == LogSeverity::Panic)
297:     {
298:         GetSystem().Halt();
299:     }
300: }
...
```

- Line 46: As we're going to use the `string` class, we include its header
- Line 83: We use the macro `LOG_NO_ALLOC_INFO` to make sure the first line printed by the logger does not need memory allocation
- Line 120-196: The new, memory-allocating version of WriteV will use the `string` class
  - Line 125: We compose the line in a string
  - Line 127: We compose the source line information in a string. We use the memory allocating version of `Format()` for this
  - Line 129: We compose the message in a string. We use the memory allocating version of `FormatV()` for this
  - Line 131-164: We append to the line using the add operator `+=`
- Line 205-211: We implement the non memory-allocating `WriteNoAlloc()`. This uses `WriteNoAllocV()` for output
- Line 221-300: We implement the non memory-allocating `WriteNoAllocV()`. This is the original function, but using `FormatNoAlloc()` and `FormatNoAllocV()`

### Format.h {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_FORMATH}

We'll add non memory-allocating methods `FormatNoAlloc()` and `FormatNoAllocV()`, and convert `Format()` and `FormatV()` into string returning versions.

Update the file `code/libraries/baremetal/include/baremetal/Format.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Format.h
...
42: /// @file
43: /// Formatting functionality
44: 
45: #include <stdlib/StdArg.h>
46: #include <stdlib/Types.h>
47: 
48: namespace baremetal {
49: 
50: class string;
51: 
52: string FormatV(const char* format, va_list args);
53: string Format(const char* format, ...);
54: void FormatNoAllocV(char* buffer, size_t bufferSize, const char* format, va_list args);
55: void FormatNoAlloc(char* buffer, size_t bufferSize, const char* format, ...);
56: 
57: } // namespace baremetal
```

### Format.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_FORMATCPP}

We'll implement the new methods.

Update the file `code/libraries/baremetal/src/Format.cpp`

```cpp
File: code/libraries/baremetal/src/Format.cpp
...
42: #include <stdlib/Util.h>
43: #include <baremetal/Serialization.h>
44: #include <baremetal/String.h>
45: 
46: /// @file
47: /// Formatting functionality implementation
48: 
49: namespace baremetal {
50: 
51: /// @brief Write characters with base above 10 as uppercase or not
52: static bool Uppercase = true;
53: 
54: static void PrintValueInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
55: static void PrintValueInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
56: 
57: /// <summary>
58: /// Convert a value to a digit. Character range is 0..9-A..Z or a..z depending on value of Uppercase
59: /// </summary>
60: /// <param name="value">Digit value</param>
61: /// <returns>Converted digit character</returns>
62: static constexpr char GetDigit(uint8 value)
63: {
64:     return value + ((value < 10) ? '0' : 'A' - 10 + (Uppercase ? 0 : 0x20));
65: }
66: 
67: /// <summary>
68: /// Calculated the amount of digits needed to represent an unsigned value of bits using base
69: /// </summary>
70: /// <param name="bits">Size of integer in bits</param>
71: /// <param name="base">Base to be used</param>
72: /// <returns>Maximum amount of digits needed</returns>
73: static constexpr int BitsToDigits(int bits, int base)
74: {
75:     int result = 0;
76:     uint64 value = 0xFFFFFFFFFFFFFFFF;
77:     if (bits < 64)
78:         value &= ((1ULL << bits) - 1);
79: 
80:     while (value > 0)
81:     {
82:         value /= base;
83:         result++;
84:     }
85: 
86:     return result;
87: }
88: 
89: /// <summary>
90: /// PrintValue a 32 bit signed value to buffer.
91: ///
92: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
93: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
94: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
95: ///
96: /// Base is the digit base, which can range from 2 to 36.
97: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
98: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
99: /// </summary>
100: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
101: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
102: /// <param name="value">Value to be serialized</param>
103: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
104: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
105: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
106: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
107: static void PrintValue(char* buffer, size_t bufferSize, int32 value, int width, int base, bool showBase, bool leadingZeros)
108: {
109:     PrintValueInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
110: }
111: 
112: /// <summary>
113: /// PrintValue a 32 bit unsigned value to buffer.
114: ///
115: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
116: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
117: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
118: ///
119: /// Base is the digit base, which can range from 2 to 36.
120: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
121: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
122: /// </summary>
123: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
124: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
125: /// <param name="value">Value to be serialized</param>
126: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
127: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
128: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
129: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
130: static void PrintValue(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros)
131: {
132:     PrintValueInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
133: }
134: 
135: /// <summary>
136: /// PrintValue a 64 bit signed value to buffer.
137: ///
138: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
139: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
140: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
141: ///
142: /// Base is the digit base, which can range from 2 to 36.
143: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
144: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
145: /// </summary>
146: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
147: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
148: /// <param name="value">Value to be serialized</param>
149: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
150: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
151: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
152: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
153: static void PrintValue(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros)
154: {
155:     PrintValueInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
156: }
157: 
158: /// <summary>
159: /// PrintValue a 64 bit unsigned value to buffer.
160: ///
161: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
162: /// Width specifies the minimum width in characters. The value is always written right aligned.
163: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
164: ///
165: /// Base is the digit base, which can range from 2 to 36.
166: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
167: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
168: /// </summary>
169: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
170: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
171: /// <param name="value">Value to be serialized</param>
172: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
173: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
174: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
175: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
176: static void PrintValue(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros)
177: {
178:     PrintValueInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
179: }
180: 
181: /// <summary>
182: /// PrintValue long long int value, type specific specialization
183: /// </summary>
184: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
185: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
186: /// <param name="value">Value to be serialized</param>
187: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
188: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
189: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
190: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
191: inline static void PrintValue(char* buffer, size_t bufferSize, long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
192: {
193:     PrintValue(buffer, bufferSize, static_cast<int64>(value), width, base, showBase, leadingZeros);
194: }
195: /// <summary>
196: /// PrintValue unsigned long long int value, type specific specialization
197: /// </summary>
198: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
199: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
200: /// <param name="value">Value to be serialized</param>
201: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
202: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
203: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
204: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
205: inline static void PrintValue(char* buffer, size_t bufferSize, unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
206: {
207:     PrintValue(buffer, bufferSize, static_cast<uint64>(value), width, base, showBase, leadingZeros);
208: }
209: 
210: /// <summary>
211: /// PrintValue a double value to buffer. The value will be printed as a fixed point number.
212: ///
213: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, the string is terminated to hold maximum bufferSize - 1 characters.
214: /// Width is currently unused.
215: /// Precision specifies the number of digits behind the decimal pointer
216: /// </summary>
217: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
218: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
219: /// <param name="value">Value to be serialized</param>
220: /// <param name="width">Unused</param>
221: /// <param name="precision">Number of digits after the decimal point to use</param>
222: static void PrintValue(char* buffer, size_t bufferSize, double value, int width, int precision)
223: {
224:     bool negative{};
225:     if (value < 0)
226:     {
227:         negative = true;
228:         value = -value;
229:     }
230: 
231:     if (bufferSize == 0)
232:         return;
233: 
234:     // We can only print values with integral parts up to what uint64 can hold
235:     if (value > static_cast<double>(static_cast<uint64>(-1)))
236:     {
237:         strncpy(buffer, "overflow", bufferSize);
238:         return;
239:     }
240: 
241:     *buffer = '\0';
242:     if (negative)
243:         strncpy(buffer, "-", bufferSize);
244: 
245:     uint64 integralPart = static_cast<uint64>(value);
246:     const size_t TmpBufferSize = 32;
247:     char tmpBuffer[TmpBufferSize];
248:     PrintValue(tmpBuffer, TmpBufferSize, integralPart, 0, 10, false, false);
249:     strncat(buffer, tmpBuffer, bufferSize);
250:     const int MaxPrecision = 7;
251: 
252:     if (precision != 0)
253:     {
254:         strncat(buffer, ".", bufferSize);
255: 
256:         if (precision > MaxPrecision)
257:         {
258:             precision = MaxPrecision;
259:         }
260: 
261:         uint64 precisionPower10 = 1;
262:         for (int i = 1; i <= precision; i++)
263:         {
264:             precisionPower10 *= 10;
265:         }
266: 
267:         value -= static_cast<double>(integralPart);
268:         value *= static_cast<double>(precisionPower10);
269: 
270:         PrintValue(tmpBuffer, TmpBufferSize, static_cast<uint64>(value), 0, 10, false, false);
271:         strncat(buffer, tmpBuffer, bufferSize);
272:         precision -= strlen(tmpBuffer);
273:         while (precision--)
274:         {
275:             strncat(buffer, "0", bufferSize);
276:         }
277:     }
278: }
279: 
280: /// <summary>
281: /// PrintValue a const char * value to buffer. The value can be quoted.
282: ///
283: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
284: /// If quote is true, the string is printed within double quotes (\")
285: /// </summary>
286: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
287: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
288: /// <param name="value">String to be serialized</param>
289: /// <param name="width">Unused</param>
290: /// <param name="quote">If true, value is printed between double quotes, if false, no quotes are used</param>
291: static void PrintValue(char* buffer, size_t bufferSize, const char* value, int width, bool quote)
292: {
293:     size_t numChars = strlen(value);
294:     if (quote)
295:         numChars += 2;
296: 
297:     // Leave one character for \0
298:     if (numChars > bufferSize - 1)
299:         return;
300: 
301:     char* bufferPtr = buffer;
302: 
303:     if (quote)
304:         *bufferPtr++ = '\"';
305:     while (*value)
306:     {
307:         *bufferPtr++ = *value++;
308:     }
309:     if (quote)
310:         *bufferPtr++ = '\"';
311: }
312: 
313: /// <summary>
314: /// Internal serialization function, to be used for all signed values.
315: ///
316: /// PrintValue a signed value to buffer.
317: ///
318: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
319: /// Width specifies the minimum width in characters. The value is always written right aligned.
320: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
321: ///
322: /// Base is the digit base, which can range from 2 to 36.
323: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
324: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
325: /// </summary>
326: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
327: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
328: /// <param name="value">Value to be serialized</param>
329: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
330: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
331: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
332: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
333: /// <param name="numBits">Specifies the number of bits used for the value</param>
334: static void PrintValueInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
335: {
336:     if ((base < 2) || (base > 36))
337:         return;
338: 
339:     int       numDigits = 0;
340:     bool      negative = (value < 0);
341:     uint64    absVal = static_cast<uint64>(negative ? -value : value);
342:     uint64    divisor = 1;
343:     uint64    divisorLast = 1;
344:     size_t    absWidth = (width < 0) ? -width : width;
345:     const int maxDigits = BitsToDigits(numBits, base);
346:     while ((absVal >= divisor) && (numDigits <= maxDigits))
347:     {
348:         divisorLast = divisor;
349:         divisor *= base;
350:         ++numDigits;
351:     }
352:     divisor = divisorLast;
353: 
354:     size_t numChars = (numDigits > 0) ? numDigits : 1;
355:     if (showBase)
356:     {
357:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
358:     }
359:     if (negative)
360:     {
361:         numChars++;
362:     }
363:     if (absWidth > numChars)
364:         numChars = absWidth;
365:     // Leave one character for \0
366:     if (numChars > bufferSize - 1)
367:         return;
368: 
369:     char* bufferPtr = buffer;
370:     if (negative)
371:     {
372:         *bufferPtr++ = '-';
373:     }
374: 
375:     if (showBase)
376:     {
377:         if (base == 2)
378:         {
379:             *bufferPtr++ = '0';
380:             *bufferPtr++ = 'b';
381:         }
382:         else if (base == 8)
383:         {
384:             *bufferPtr++ = '0';
385:         }
386:         else if (base == 16)
387:         {
388:             *bufferPtr++ = '0';
389:             *bufferPtr++ = 'x';
390:         }
391:     }
392:     if (leadingZeros)
393:     {
394:         if (absWidth == 0)
395:             absWidth = maxDigits;
396:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
397:         {
398:             *bufferPtr++ = '0';
399:         }
400:     }
401:     else
402:     {
403:         if (numDigits == 0)
404:         {
405:             *bufferPtr++ = '0';
406:         }
407:     }
408:     while (numDigits > 0)
409:     {
410:         int digit = (absVal / divisor) % base;
411:         *bufferPtr++ = GetDigit(digit);
412:         --numDigits;
413:         divisor /= base;
414:     }
415:     *bufferPtr++ = '\0';
416: }
417: 
418: /// <summary>
419: /// Internal serialization function, to be used for all unsigned values.
420: ///
421: /// PrintValue a unsigned value to buffer.
422: ///
423: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
424: /// Width specifies the minimum width in characters. The value is always written right aligned.
425: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
426: ///
427: /// Base is the digit base, which can range from 2 to 36.
428: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
429: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
430: /// </summary>
431: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
432: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
433: /// <param name="value">Value to be serialized</param>
434: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
435: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
436: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
437: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
438: /// <param name="numBits">Specifies the number of bits used for the value</param>
439: static void PrintValueInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
440: {
441:     if ((base < 2) || (base > 36))
442:         return;
443: 
444:     int       numDigits = 0;
445:     uint64    divisor = 1;
446:     uint64    divisorLast = 1;
447:     uint64    divisorHigh = 0;
448:     size_t    absWidth = (width < 0) ? -width : width;
449:     const int maxDigits = BitsToDigits(numBits, base);
450:     while ((divisorHigh == 0) && (value >= divisor) && (numDigits <= maxDigits))
451:     {
452:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
453:         divisorLast = divisor;
454:         divisor *= base;
455:         ++numDigits;
456:     }
457:     divisor = divisorLast;
458: 
459:     size_t numChars = (numDigits > 0) ? numDigits : 1;
460:     if (showBase)
461:     {
462:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
463:     }
464:     if (absWidth > numChars)
465:         numChars = absWidth;
466:     // Leave one character for \0
467:     if (numChars > bufferSize - 1)
468:         return;
469: 
470:     char* bufferPtr = buffer;
471: 
472:     if (showBase)
473:     {
474:         if (base == 2)
475:         {
476:             *bufferPtr++ = '0';
477:             *bufferPtr++ = 'b';
478:         }
479:         else if (base == 8)
480:         {
481:             *bufferPtr++ = '0';
482:         }
483:         else if (base == 16)
484:         {
485:             *bufferPtr++ = '0';
486:             *bufferPtr++ = 'x';
487:         }
488:     }
489:     if (leadingZeros)
490:     {
491:         if (absWidth == 0)
492:             absWidth = maxDigits;
493:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
494:         {
495:             *bufferPtr++ = '0';
496:         }
497:     }
498:     else
499:     {
500:         if (numDigits == 0)
501:         {
502:             *bufferPtr++ = '0';
503:         }
504:     }
505:     while (numDigits > 0)
506:     {
507:         int digit = (value / divisor) % base;
508:         *bufferPtr++ = GetDigit(digit);
509:         --numDigits;
510:         divisor /= base;
511:     }
512:     *bufferPtr++ = '\0';
513: }
514: 
515: /// <summary>
516: /// Size of buffer used by FormatNoAllocV internally
517: /// </summary>
518: static const size_t BufferSize = 4096;
...
573: /// <summary>
574: /// Format a string
575: ///
576: /// This version of Format uses the string class, and thus allocates memory
577: /// </summary>
578: /// <param name="format">Format string (printf like)</param>
579: /// <returns>Resulting string</returns>
580: string Format(const char* format, ...)
581: {
582:     va_list var;
583:     va_start(var, format);
584: 
585:     string result = FormatV(format, var);
586: 
587:     va_end(var);
588: 
589:     return result;
590: }
591: 
592: /// <summary>
593: /// Format a string
594: ///
595: /// This version of FormatV uses the string class, and thus allocates memory
596: /// </summary>
597: /// <param name="format">Format string (printf like)</param>
598: /// <param name="args">Variable argument list</param>
599: /// <returns>Resulting string</returns>
600: string FormatV(const char* format, va_list args)
601: {
602:     string result;
603: 
604:     while (*format != '\0')
605:     {
606:         if (*format == '%')
607:         {
608:             if (*++format == '%')
609:             {
610:                 result += '%';
611:                 format++;
612:                 continue;
613:             }
614: 
615:             bool alternate = false;
616:             if (*format == '#')
617:             {
618:                 alternate = true;
619:                 format++;
620:             }
621: 
622:             bool left = false;
623:             if (*format == '-')
624:             {
625:                 left = true;
626:                 format++;
627:             }
628: 
629:             bool leadingZero = false;
630:             if (*format == '0')
631:             {
632:                 leadingZero = true;
633:                 format++;
634:             }
635: 
636:             size_t width = 0;
637:             while (('0' <= *format) && (*format <= '9'))
638:             {
639:                 width = width * 10 + (*format - '0');
640:                 format++;
641:             }
642: 
643:             unsigned precision = 6;
644:             if (*format == '.')
645:             {
646:                 format++;
647:                 precision = 0;
648:                 while ('0' <= *format && *format <= '9')
649:                 {
650:                     precision = precision * 10 + (*format - '0');
651: 
652:                     format++;
653:                 }
654:             }
655: 
656:             bool haveLong{};
657:             bool haveLongLong{};
658: 
659:             if (*format == 'l')
660:             {
661:                 if (*(format + 1) == 'l')
662:                 {
663:                     haveLongLong = true;
664: 
665:                     format++;
666:                 }
667:                 else
668:                 {
669:                     haveLong = true;
670:                 }
671: 
672:                 format++;
673:             }
674: 
675:             switch (*format)
676:             {
677:             case 'c':
678:             {
679:                 char ch = static_cast<char>(va_arg(args, int));
680:                 if (left)
681:                 {
682:                     result += ch;
683:                     if (width > 1)
684:                     {
685:                         result.append(width - 1, ' ');
686:                     }
687:                 }
688:                 else
689:                 {
690:                     if (width > 1)
691:                     {
692:                         result.append(width - 1, ' ');
693:                     }
694:                     result += ch;
695:                 }
696:             }
697:             break;
698: 
699:             case 'd':
700:             case 'i':
701:                 if (haveLongLong)
702:                 {
703:                     result.append(Serialize(va_arg(args, int64), left ? -width : width, 10, false, leadingZero));
704:                 }
705:                 else if (haveLong)
706:                 {
707:                     result.append(Serialize(va_arg(args, int32), left ? -width : width, 10, false, leadingZero));
708:                 }
709:                 else
710:                 {
711:                     result.append(Serialize(va_arg(args, int), left ? -width : width, 10, false, leadingZero));
712:                 }
713:                 break;
714: 
715:             case 'f':
716:             {
717:                 result.append(Serialize(va_arg(args, double), left ? -width : width, precision));
718:             }
719:             break;
720: 
721:             case 'b':
722:                 if (alternate)
723:                 {
724:                     result.append("0b");
725:                 }
726:                 if (haveLongLong)
727:                 {
728:                     result.append(Serialize(va_arg(args, uint64), left ? -width : width, 2, false, leadingZero));
729:                 }
730:                 else if (haveLong)
731:                 {
732:                     result.append(Serialize(va_arg(args, uint32), left ? -width : width, 2, false, leadingZero));
733:                 }
734:                 else
735:                 {
736:                     result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero));
737:                 }
738:                 break;
739: 
740:             case 'o':
741:                 if (alternate)
742:                 {
743:                     result.append("0");
744:                 }
745:                 if (haveLongLong)
746:                 {
747:                     result.append(Serialize(va_arg(args, uint64), left ? -width : width, 8, false, leadingZero));
748:                 }
749:                 else if (haveLong)
750:                 {
751:                     result.append(Serialize(va_arg(args, uint32), left ? -width : width, 8, false, leadingZero));
752:                 }
753:                 else
754:                 {
755:                     result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero));
756:                 }
757:                 break;
758: 
759:             case 's':
760:             {
761:                 result.append(Serialize(va_arg(args, const char*), left ? -width : width, false));
762:             }
763:             break;
764: 
765:             case 'u':
766:                 if (haveLongLong)
767:                 {
768:                     result.append(Serialize(va_arg(args, uint64), left ? -width : width, 10, false, leadingZero));
769:                 }
770:                 else if (haveLong)
771:                 {
772:                     result.append(Serialize(va_arg(args, uint32), left ? -width : width, 10, false, leadingZero));
773:                 }
774:                 else
775:                 {
776:                     result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero));
777:                 }
778:                 break;
779: 
780:             case 'x':
781:             case 'X':
782:                 if (alternate)
783:                 {
784:                     result.append("0x");
785:                 }
786:                 if (haveLongLong)
787:                 {
788:                     result.append(Serialize(va_arg(args, uint64), left ? -width : width, 16, false, leadingZero));
789:                 }
790:                 else if (haveLong)
791:                 {
792:                     result.append(Serialize(va_arg(args, uint32), left ? -width : width, 16, false, leadingZero));
793:                 }
794:                 else
795:                 {
796:                     result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero));
797:                 }
798:                 break;
799: 
800:             case 'p':
801:                 if (alternate)
802:                 {
803:                     result.append("0x");
804:                 }
805:                 {
806:                     result.append(Serialize(va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero));
807:                 }
808:                 break;
809: 
810:             default:
811:                 result += '%';
812:                 result += *format;
813:                 break;
814:             }
815:         }
816:         else
817:         {
818:             result += *format;
819:         }
820: 
821:         format++;
822:     }
823: 
824:     return result;
825: }
826: 
827: /// <summary>
828: /// Format a string
829: ///
830: /// This version of Format writes directly to a buffer, and does not allocate memory
831: /// </summary>
832: /// <param name="buffer">Buffer to write to</param>
833: /// <param name="bufferSize">Size of the buffer</param>
834: /// <param name="format">Format string (printf like)</param>
835: void FormatNoAlloc(char* buffer, size_t bufferSize, const char* format, ...)
836: {
837:     va_list var;
838:     va_start(var, format);
839: 
840:     FormatNoAllocV(buffer, bufferSize, format, var);
841: 
842:     va_end(var);
843: }
844: 
845: /// <summary>
846: /// Format a string
847: ///
848: /// This version of FormatV writes directly to a buffer, and does not allocate memory
849: /// </summary>
850: /// <param name="buffer">Buffer to write to</param>
851: /// <param name="bufferSize">Size of the buffer</param>
852: /// <param name="format">Format string (printf like)</param>
853: /// <param name="args">Variable argument list</param>
854: void FormatNoAllocV(char* buffer, size_t bufferSize, const char* format, va_list args)
855: {
856:     buffer[0] = '\0';
857: 
858:     while (*format != '\0')
859:     {
860:         if (*format == '%')
861:         {
862:             if (*++format == '%')
863:             {
864:                 Append(buffer, bufferSize, '%');
865:                 format++;
866:                 continue;
867:             }
868: 
869:             bool alternate = false;
870:             if (*format == '#')
871:             {
872:                 alternate = true;
873:                 format++;
874:             }
875: 
876:             bool left = false;
877:             if (*format == '-')
878:             {
879:                 left = true;
880:                 format++;
881:             }
882: 
883:             bool leadingZero = false;
884:             if (*format == '0')
885:             {
886:                 leadingZero = true;
887:                 format++;
888:             }
889: 
890:             size_t width = 0;
891:             while (('0' <= *format) && (*format <= '9'))
892:             {
893:                 width = width * 10 + (*format - '0');
894:                 format++;
895:             }
896: 
897:             unsigned precision = 6;
898:             if (*format == '.')
899:             {
900:                 format++;
901:                 precision = 0;
902:                 while ('0' <= *format && *format <= '9')
903:                 {
904:                     precision = precision * 10 + (*format - '0');
905: 
906:                     format++;
907:                 }
908:             }
909: 
910:             bool haveLong{};
911:             bool haveLongLong{};
912: 
913:             if (*format == 'l')
914:             {
915:                 if (*(format + 1) == 'l')
916:                 {
917:                     haveLongLong = true;
918: 
919:                     format++;
920:                 }
921:                 else
922:                 {
923:                     haveLong = true;
924:                 }
925: 
926:                 format++;
927:             }
928: 
929:             switch (*format)
930:             {
931:             case 'c':
932:                 {
933:                     char ch = static_cast<char>(va_arg(args, int));
934:                     if (left)
935:                     {
936:                         Append(buffer, bufferSize, ch);
937:                         if (width > 1)
938:                         {
939:                             Append(buffer, bufferSize, width - 1, ' ');
940:                         }
941:                     }
942:                     else
943:                     {
944:                         if (width > 1)
945:                         {
946:                             Append(buffer, bufferSize, width - 1, ' ');
947:                         }
948:                         Append(buffer, bufferSize, ch);
949:                     }
950:                 }
951:                 break;
952: 
953:             case 'd':
954:             case 'i':
955:                 if (haveLongLong)
956:                 {
957:                     char str[BufferSize]{};
958:                     PrintValue(str, BufferSize, va_arg(args, int64), left ? -width : width, 10, false, leadingZero);
959:                     Append(buffer, bufferSize, str);
960:                 }
961:                 else if (haveLong)
962:                 {
963:                     char str[BufferSize]{};
964:                     PrintValue(str, BufferSize, va_arg(args, int32), left ? -width : width, 10, false, leadingZero);
965:                     Append(buffer, bufferSize, str);
966:                 }
967:                 else
968:                 {
969:                     char str[BufferSize]{};
970:                     PrintValue(str, BufferSize, va_arg(args, int), left ? -width : width, 10, false, leadingZero);
971:                     Append(buffer, bufferSize, str);
972:                 }
973:                 break;
974: 
975:             case 'f':
976:                 {
977:                     char str[BufferSize]{};
978:                     PrintValue(str, BufferSize, va_arg(args, double), left ? -width : width, precision);
979:                     Append(buffer, bufferSize, str);
980:                 }
981:                 break;
982: 
983:             case 'b':
984:                 if (alternate)
985:                 {
986:                     Append(buffer, bufferSize, "0b");
987:                 }
988:                 if (haveLongLong)
989:                 {
990:                     char str[BufferSize]{};
991:                     PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 2, false, leadingZero);
992:                     Append(buffer, bufferSize, str);
993:                 }
994:                 else if (haveLong)
995:                 {
996:                     char str[BufferSize]{};
997:                     PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 2, false, leadingZero);
998:                     Append(buffer, bufferSize, str);
999:                 }
1000:                 else
1001:                 {
1002:                     char str[BufferSize]{};
1003:                     PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero);
1004:                     Append(buffer, bufferSize, str);
1005:                 }
1006:                 break;
1007: 
1008:             case 'o':
1009:                 if (alternate)
1010:                 {
1011:                     Append(buffer, bufferSize, '0');
1012:                 }
1013:                 if (haveLongLong)
1014:                 {
1015:                     char str[BufferSize]{};
1016:                     PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 8, false, leadingZero);
1017:                     Append(buffer, bufferSize, str);
1018:                 }
1019:                 else if (haveLong)
1020:                 {
1021:                     char str[BufferSize]{};
1022:                     PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 8, false, leadingZero);
1023:                     Append(buffer, bufferSize, str);
1024:                 }
1025:                 else
1026:                 {
1027:                     char str[BufferSize]{};
1028:                     PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero);
1029:                     Append(buffer, bufferSize, str);
1030:                 }
1031:                 break;
1032: 
1033:             case 's':
1034:                 {
1035:                     char str[BufferSize]{};
1036:                     PrintValue(str, BufferSize, va_arg(args, const char*), left ? -width : width, false);
1037:                     Append(buffer, bufferSize, str);
1038:                 }
1039:                 break;
1040: 
1041:             case 'u':
1042:                 if (haveLongLong)
1043:                 {
1044:                     char str[BufferSize]{};
1045:                     PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 10, false, leadingZero);
1046:                     Append(buffer, bufferSize, str);
1047:                 }
1048:                 else if (haveLong)
1049:                 {
1050:                     char str[BufferSize]{};
1051:                     PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 10, false, leadingZero);
1052:                     Append(buffer, bufferSize, str);
1053:                 }
1054:                 else
1055:                 {
1056:                     char str[BufferSize]{};
1057:                     PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero);
1058:                     Append(buffer, bufferSize, str);
1059:                 }
1060:                 break;
1061: 
1062:             case 'x':
1063:             case 'X':
1064:                 if (alternate)
1065:                 {
1066:                     Append(buffer, bufferSize, "0x");
1067:                 }
1068:                 if (haveLongLong)
1069:                 {
1070:                     char str[BufferSize]{};
1071:                     PrintValue(str, BufferSize, va_arg(args, uint64), left ? -width : width, 16, false, leadingZero);
1072:                     Append(buffer, bufferSize, str);
1073:                 }
1074:                 else if (haveLong)
1075:                 {
1076:                     char str[BufferSize]{};
1077:                     PrintValue(str, BufferSize, va_arg(args, uint32), left ? -width : width, 16, false, leadingZero);
1078:                     Append(buffer, bufferSize, str);
1079:                 }
1080:                 else
1081:                 {
1082:                     char str[BufferSize]{};
1083:                     PrintValue(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero);
1084:                     Append(buffer, bufferSize, str);
1085:                 }
1086:                 break;
1087: 
1088:             case 'p':
1089:                 if (alternate)
1090:                 {
1091:                     Append(buffer, bufferSize, "0x");
1092:                 }
1093:                 {
1094:                     char str[BufferSize]{};
1095:                     PrintValue(str, BufferSize, va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero);
1096:                     Append(buffer, bufferSize, str);
1097:                 }
1098:                 break;
1099: 
1100:             default:
1101:                 Append(buffer, bufferSize, '%');
1102:                 Append(buffer, bufferSize, *format);
1103:                 break;
1104:             }
1105:         }
1106:         else
1107:         {
1108:             Append(buffer, bufferSize, *format);
1109:         }
1110: 
1111:         format++;
1112:     }
1113: }
1114: 
1115: } // namespace baremetal
```

- Line 43: We need the `string` class, so we'll add its header
- Line 52: We copy the variable Uppercase from `Serialization.cpp`
- Line 54-55: We copy the functions `SerializeInteralUInt()` and `SerializeInteralInt()` from `Serialization.cpp` and rename `Serialization` to `PrintValue`
- Line 62-65: We also copy the function `GetDigit()`
- Line 73-87: We also copy the function `BitsToDigits()`
- Line 107-110: We also copy the function `Serialize()` for int32, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 130-133: We also copy the function `Serialize()` for uint32, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 153-156: We also copy the function `Serialize()` for int64, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 176-179: We also copy the function `Serialize()` for uint64, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 191-194: We also copy the function `Serialize()` for long long from `Serialization.h`, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 205-208: We also copy the function `Serialize()` for unsigned long long from `Serialization.h`, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 222-278: We also copy the function `Serialize()` for double, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 291-311: We also copy the function `Serialize()` for const char*, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 334-416: We also copy the function `SerializeInternalInt()`, and rename it to `PrintValueInternalInt()`
- Line 439-513: We also copy the function `SerializeInternalUInt()`, and rename it to `PrintValueInternalUInt()`
- Line 526-571: We leave the `Append()` functions as is
- Line 580-590: We implement the new `Format()` function. This uses `FormatV()`
- Line 600-825: We implement the new `FormatV()` function.
This is a copy of the original `FormatV()`, but appending to a string, instead of into a buffer, and using `Serialize()` calls to serialize data.
The string is finally returned
- Line 835-843: We rename the old `Format()` to `FormatNoAlloc()` and change the call to `FormatNoAllocV()`
- Line 854-1113: We rename the old `FormatV()` to `FormatNoAllocV()`.
The implementation only changes in the calls to `Serialize()` being replaced with calls to the internal `PrintValue()` functions)

## Updating serializers - Step 2 {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS___STEP_2}

Now we'll change all the `Serialize()` methods to be returning a `string`.
We'll also be adding some more functions to expand the different types supported.

### Serialization.h {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS___STEP_2_SERIALIZATIONH}

We'll change the serialization functions to return a `string`.

Update the file `code/libraries/baremetal/include/baremetal/Serialization.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Serialization.h
...
42: #include <stdlib/Types.h>
43: #include <baremetal/String.h>
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
56: 
57: /// <summary>
58: /// Serialize boolean
59: /// </summary>
60: /// <param name="value">Value</param>
61: /// <returns>Resulting string</returns>
62: inline string Serialize(const bool& value)
63: {
64:     return string(value ? "true" : "false");
65: }
66: 
67: string Serialize(char value, int width = 0);
68: string Serialize(int8 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
69: string Serialize(uint8 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
70: string Serialize(int16 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
71: string Serialize(uint16 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
72: string Serialize(int32 value, int width= 0, int base = 10, bool showBase = false, bool leadingZeros = false);
73: string Serialize(uint32 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
74: string Serialize(int64 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
75: string Serialize(uint64 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
76: 
77: /// <summary>
78: /// Serialize long long int value, type specific specialization
79: /// </summary>
80: /// <param name="value">Value to be serialized</param>
81: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
82: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
83: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
84: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
85: /// <returns>Resulting string</returns>
86: inline string Serialize(long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
87: {
88:     return Serialize(static_cast<int64>(value), width, base, showBase, leadingZeros);
89: }
90: /// <summary>
91: /// Serialize unsigned long long int value, type specific specialization
92: /// </summary>
93: /// <param name="value">Value to be serialized</param>
94: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
95: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
96: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
97: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
98: /// <returns>Resulting string</returns>
99: inline string Serialize(unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
100: {
101:     return Serialize(static_cast<uint64>(value), width, base, showBase, leadingZeros);
102: }
103: string Serialize(float value, int width = 0, int precision = 16);
104: string Serialize(double value, int width = 0, int precision = 16);
105: string Serialize(const string& value, int width = 0, bool quote = false);
106: string Serialize(const char* value, int width = 0, bool quote = false);
107: string Serialize(const void* value, int width = 0);
108: string Serialize(void* value, int width = 0);
109: 
110: } // namespace baremetal
```

- Line 43: We need to include the header for the `string` class, as we will also be implementing some inline functions returning a string
- Line 62-65: We implement the specialization to serialize a boolean as an inline function
- Line 67: We declare the specialization for char
- Line 68-75: We declare the specializations for all integer types. Note that we've added some more to make the list complete
- Line 86-89: We implement the specialization for long long as a inline function.
-Notice that as before, we simply re-use the version for int64
- Line 99-102: We implement the specialization for unsigned long long as a inline function.
Notice that as before, we simply re-use the version for uint64
- Line 103-104: We declare the specializations for floating point types. Note that we've added float here as well
- Line 105-106: We declare the specializations for strings. Note that we've added a serialization for `string` here as well as the `const char*` version
- Line 107-108: We declare the specializations for pointers. Any pointer can be simply cast to either `void*` or `const void*`. Pointers will simply be serialized by printing their address as a hexadecimal value

### Serialization.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS___STEP_2_SERIALIZATIONCPP}

We'll implement the new and updated functions.

Update the file `code/libraries/baremetal/src/Serialization.cpp`

```cpp
File: code/libraries/baremetal/src/Serialization.cpp
...
40: #include <baremetal/Serialization.h>
41: 
42: #include <stdlib/Util.h>
43: 
44: /// @file
45: /// Type serialization functions implementation
46: 
47: namespace baremetal {
48: 
49: /// @brief Write characters with base above 10 as uppercase or not
50: static bool Uppercase = true;
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
96: string Serialize(char value, int width)
97: {
98:     string result;
99: 
100:     int    numDigits = 0;
101:     bool   negative = (value < 0);
102:     uint64 absVal = static_cast<uint64>(negative ? -value : value);
103:     uint64 divisor = 1;
104:     int    absWidth = (width < 0) ? -width : width;
105:     while (absVal >= divisor)
106:     {
107:         divisor *= 10;
108:         ++numDigits;
109:     }
110: 
111:     if (numDigits == 0)
112:     {
113:         result = "0";
114:         return result;
115:     }
116:     if (negative)
117:     {
118:         result += '-';
119:         absWidth--;
120:     }
121:     while (numDigits > 0)
122:     {
123:         divisor /= 10;
124:         int digit = (absVal / divisor) % 10;
125:         result += GetDigit(digit);
126:         --numDigits;
127:     }
128:     return result.align(width);
129: }
130: 
131: /// <summary>
132: /// Serialize a 8 bit signed value to string.
133: ///
134: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
135: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
136: ///
137: /// Base is the digit base, which can range from 2 to 36.
138: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
139: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
140: /// </summary>
141: /// <param name="value">Value to be serialized</param>
142: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
143: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
144: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
145: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
146: /// <returns>Serialized string value</returns>
147: string Serialize(int8 value, int width, int base, bool showBase, bool leadingZeros)
148: {
149:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 8);
150: }
151: 
152: /// <summary>
153: /// Serialize a 8 bit unsigned value to string.
154: ///
155: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
156: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
157: ///
158: /// Base is the digit base, which can range from 2 to 36.
159: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
160: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
161: /// </summary>
162: /// <param name="value">Value to be serialized</param>
163: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
164: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
165: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
166: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
167: /// <returns>Serialized string value</returns>
168: string Serialize(uint8 value, int width, int base, bool showBase, bool leadingZeros)
169: {
170:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 8);
171: }
172: 
173: /// <summary>
174: /// Serialize a 16 bit signed value to string.
175: ///
176: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
177: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
178: ///
179: /// Base is the digit base, which can range from 2 to 36.
180: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
181: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
182: /// </summary>
183: /// <param name="value">Value to be serialized</param>
184: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
185: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
186: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
187: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
188: /// <returns>Serialized string value</returns>
189: string Serialize(int16 value, int width, int base, bool showBase, bool leadingZeros)
190: {
191:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 16);
192: }
193: 
194: /// <summary>
195: /// Serialize a 16 bit unsigned value to string.
196: ///
197: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
198: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
199: ///
200: /// Base is the digit base, which can range from 2 to 36.
201: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
202: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
203: /// </summary>
204: /// <param name="value">Value to be serialized</param>
205: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
206: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
207: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
208: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
209: /// <returns>Serialized string value</returns>
210: string Serialize(uint16 value, int width, int base, bool showBase, bool leadingZeros)
211: {
212:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 16);
213: }
214: 
215: /// <summary>
216: /// Serialize a 32 bit signed value to string.
217: ///
218: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
219: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
220: ///
221: /// Base is the digit base, which can range from 2 to 36.
222: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
223: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
224: /// </summary>
225: /// <param name="value">Value to be serialized</param>
226: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
227: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
228: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
229: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
230: /// <returns>Serialized string value</returns>
231: string Serialize(int32 value, int width, int base, bool showBase, bool leadingZeros)
232: {
233:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 32);
234: }
235: 
236: /// <summary>
237: /// Serialize a 32 bit unsigned value to string.
238: ///
239: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
240: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
241: ///
242: /// Base is the digit base, which can range from 2 to 36.
243: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
244: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
245: /// </summary>
246: /// <param name="value">Value to be serialized</param>
247: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
248: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
249: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
250: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
251: /// <returns>Serialized string value</returns>
252: string Serialize(uint32 value, int width, int base, bool showBase, bool leadingZeros)
253: {
254:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 32);
255: }
256: 
257: /// <summary>
258: /// Serialize a 64 bit signed value to string.
259: ///
260: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
261: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
262: ///
263: /// Base is the digit base, which can range from 2 to 36.
264: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
265: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
266: /// </summary>
267: /// <param name="value">Value to be serialized</param>
268: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
269: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
270: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
271: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
272: /// <returns>Serialized string value</returns>
273: string Serialize(int64 value, int width, int base, bool showBase, bool leadingZeros)
274: {
275:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 64);
276: }
277: 
278: /// <summary>
279: /// Serialize a 64 bit unsigned value to string.
280: ///
281: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
282: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
283: ///
284: /// Base is the digit base, which can range from 2 to 36.
285: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
286: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
287: /// </summary>
288: /// <param name="value">Value to be serialized</param>
289: /// <param name="width">Minimum width in characters, excluding any base prefix. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
290: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
291: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
292: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
293: /// <returns>Serialized string value</returns>
294: string Serialize(uint64 value, int width, int base, bool showBase, bool leadingZeros)
295: {
296:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 64);
297: }
298: 
299: /// <summary>
300: /// Serialize a float value to string. The value will be printed as a fixed point number.
301: ///
302: /// Width specifies the minimum width in characters. The value is written right aligned if width is positive, left aligned if width is negative.
303: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
304: ///
305: /// Precision specifies the number of digits behind the decimal pointer
306: /// </summary>
307: /// <param name="value">Value to be serialized</param>
308: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
309: /// <param name="precision">Number of digits after the decimal point to use (limited to 7 decimals</param>
310: /// <returns>Serialized string value</returns>
311: string Serialize(float value, int width, int precision)
312: {
313:     bool negative{};
314:     if (value < 0)
315:     {
316:         negative = true;
317:         value = -value;
318:     }
319: 
320:     // We can only print values with integral parts up to what uint64 can hold
321:     if (value > static_cast<float>(static_cast<uint64>(-1)))
322:     {
323:         return string("overflow");
324:     }
325: 
326:     string result;
327:     if (negative)
328:         result += '-';
329: 
330:     uint64 integralPart = static_cast<uint64>(value);
331:     result += Serialize(integralPart, 0, 10, false, false);
332:     const int MaxPrecision = 7;
333: 
334:     if (precision != 0)
335:     {
336:         result += '.';
337: 
338:         if (precision > MaxPrecision)
339:         {
340:             precision = MaxPrecision;
341:         }
342: 
343:         uint64 precisionPower10 = 1;
344:         for (int i = 1; i <= precision; i++)
345:         {
346:             precisionPower10 *= 10;
347:         }
348: 
349:         value -= static_cast<float>(integralPart);
350:         value *= static_cast<float>(precisionPower10);
351: 
352:         string fractional = Serialize(static_cast<uint64>(value + 0.5F), 0, 10, false, false);
353:         result += fractional;
354:         precision -= fractional.length();
355:         while (precision--)
356:         {
357:             result += '0';
358:         }
359:     }
360:     return result.align(width);
361: }
362: 
363: /// <summary>
364: /// Serialize a double value to string. The value will be printed as a fixed point number.
365: ///
366: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
367: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
368: ///
369: /// Precision specifies the number of digits behind the decimal pointer
370: /// </summary>
371: /// <param name="value">Value to be serialized</param>
372: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
373: /// <param name="precision">Number of digits after the decimal point to use (limited to 10 decimals</param>
374: /// <returns>Serialized string value</returns>
375: string Serialize(double value, int width, int precision)
376: {
377:     bool negative{};
378:     if (value < 0)
379:     {
380:         negative = true;
381:         value = -value;
382:     }
383: 
384:     // We can only print values with integral parts up to what uint64 can hold
385:     if (value > static_cast<double>(static_cast<uint64>(-1)))
386:     {
387:         return string("overflow");
388:     }
389: 
390:     string result;
391:     if (negative)
392:         result += '-';
393: 
394:     uint64 integralPart = static_cast<uint64>(value);
395:     result += Serialize(integralPart, 0, 10, false, false);
396:     const int MaxPrecision = 14;
397: 
398:     if (precision != 0)
399:     {
400:         result += '.';
401: 
402:         if (precision > MaxPrecision)
403:         {
404:             precision = MaxPrecision;
405:         }
406: 
407:         uint64 precisionPower10 = 1;
408:         for (int i = 1; i <= precision; i++)
409:         {
410:             precisionPower10 *= 10;
411:         }
412: 
413:         value -= static_cast<double>(integralPart);
414:         value *= static_cast<double>(precisionPower10);
415: 
416:         string fractional = Serialize(static_cast<uint64>(value + 0.5), 0, 10, false, false);
417:         result += fractional;
418:         precision -= fractional.length();
419:         while (precision--)
420:         {
421:             result += '0';
422:         }
423:     }
424:     return result.align(width);
425: }
426: 
427: /// <summary>
428: /// Serialize a string to string.
429: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
430: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
431: /// If requested, the string is placed between double quotes (").
432: /// </summary>
433: /// <param name="value">Value to be serialized</param>
434: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
435: /// <param name="quote">If true places string between double quotes</param>
436: /// <returns>Serialized string value</returns>
437: string Serialize(const string& value, int width, bool quote)
438: {
439:     return Serialize(value.data(), width, quote);
440: }
441: 
442: /// <summary>
443: /// Serialize a string to string
444: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
445: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
446: /// If requested, the string is placed between double quotes (").
447: /// </summary>
448: /// <param name="value">Value to be serialized</param>
449: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
450: /// <param name="quote">If true places string between double quotes</param>
451: /// <returns>Serialized string value</returns>
452: string Serialize(const char* value, int width, bool quote)
453: {
454:     string result;
455: 
456:     if (quote)
457:         result += '\"';
458:     result += value;
459:     if (quote)
460:         result += '\"';
461: 
462:     return result.align(width);
463: }
464: 
465: /// <summary>
466: /// Serialize a const void pointer to string
467: ///
468: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
469: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
470: /// </summary>
471: /// <param name="value">Value to be serialized</param>
472: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
473: /// <returns>Serialized string value</returns>
474: string Serialize(const void* value, int width)
475: {
476:     string result;
477: 
478:     if (value != nullptr)
479:     {
480:         result = Serialize(reinterpret_cast<uintptr>(value), 16, 16, true, true);
481:     }
482:     else
483:     {
484:         result = "null";
485:     }
486: 
487:     return result.align(width);
488: }
489: 
490: /// <summary>
491: /// Serialize a void pointer to string.
492: ///
493: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
494: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
495: /// </summary>
496: /// <param name="value">Value to be serialized</param>
497: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
498: /// <returns>Serialized string value</returns>
499: string Serialize(void* value, int width)
500: {
501:     return Serialize(const_cast<const void*>(value), width);
502: }
503: 
504: /// <summary>
505: /// Internal serialization function returning string, to be used for all signed values.
506: ///
507: /// Serialize a signed value to string.
508: ///
509: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
510: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
511: ///
512: /// Base is the digit base, which can range from 2 to 36.
513: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
514: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
515: /// </summary>
516: /// <param name="value">Value to be serialized</param>
517: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
518: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
519: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
520: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
521: /// <param name="numBits">Specifies the number of bits used for the value</param>
522: /// <returns>Serialized stirng</returns>
523: static string SerializeInternalInt(int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
524: {
525:     if ((base < 2) || (base > 36))
526:         return {};
527: 
528:     int       numDigits = 0;
529:     bool      negative = (value < 0);
530:     uint64    absVal = static_cast<uint64>(negative ? -value : value);
531:     uint64    divisor = 1;
532:     uint64    divisorHigh = 0;
533:     uint64    divisorLast = 1;
534:     size_t    absWidth = (width < 0) ? -width : width;
535:     const int maxDigits = BitsToDigits(numBits, base);
536:     while ((divisorHigh == 0) && (absVal >= divisor) && (numDigits <= maxDigits))
537:     {
538:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
539:         divisorLast = divisor;
540:         divisor *= base;
541:         ++numDigits;
542:     }
543:     divisor = divisorLast;
544: 
545:     size_t numChars = (numDigits > 0) ? numDigits : 1;
546:     if (showBase)
547:     {
548:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
549:     }
550:     if (negative)
551:     {
552:         numChars++;
553:     }
554:     if (absWidth > numChars)
555:         numChars = absWidth;
556:     // Leave one character for \0
557:     string result;
558:     result.reserve(numChars + 1);
559: 
560:     if (negative)
561:     {
562:         result += '-';
563:     }
564: 
565:     if (showBase)
566:     {
567:         if (base == 2)
568:         {
569:             result += "0b";
570:         }
571:         else if (base == 8)
572:         {
573:             result += '0';
574:         }
575:         else if (base == 16)
576:         {
577:             result += "0x";
578:         }
579:     }
580:     if (leadingZeros)
581:     {
582:         if (absWidth == 0)
583:             absWidth = maxDigits;
584:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
585:         {
586:             result += '0';
587:         }
588:     }
589:     else
590:     {
591:         if (numDigits == 0)
592:         {
593:             result += '0';
594:         }
595:     }
596:     while (numDigits > 0)
597:     {
598:         int digit = (absVal / divisor) % base;
599:         result += GetDigit(digit);
600:         --numDigits;
601:         divisor /= base;
602:     }
603:     return result.align(width);
604: }
605: 
606: /// <summary>
607: /// Internal serialization function returning string, to be used for all unsigned values.
608: ///
609: /// Serialize a unsigned value to string.
610: ///
611: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
612: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
613: ///
614: /// Base is the digit base, which can range from 2 to 36.
615: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
616: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
617: /// </summary>
618: /// <param name="value">Value to be serialized</param>
619: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
620: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
621: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
622: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
623: /// <param name="numBits">Specifies the number of bits used for the value</param>
624: /// <returns>Serialized stirng</returns>
625: static string SerializeInternalUInt(uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
626: {
627:     if ((base < 2) || (base > 36))
628:         return {};
629: 
630:     int       numDigits = 0;
631:     uint64    divisor = 1;
632:     uint64    divisorLast = 1;
633:     uint64    divisorHigh = 0;
634:     size_t    absWidth = (width < 0) ? -width : width;
635:     const int maxDigits = BitsToDigits(numBits, base);
636:     while ((divisorHigh == 0) && (value >= divisor) && (numDigits <= maxDigits))
637:     {
638:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
639:         divisorLast = divisor;
640:         divisor *= base;
641:         ++numDigits;
642:     }
643:     divisor = divisorLast;
644: 
645:     size_t numChars = (numDigits > 0) ? numDigits : 1;
646:     if (showBase)
647:     {
648:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
649:     }
650:     if (absWidth > numChars)
651:         numChars = absWidth;
652:     // Leave one character for \0
653:     string result;
654:     result.reserve(numChars + 1);
655: 
656:     if (showBase)
657:     {
658:         if (base == 2)
659:         {
660:             result += "0b";
661:         }
662:         else if (base == 8)
663:         {
664:             result += '0';
665:         }
666:         else if (base == 16)
667:         {
668:             result += "0x";
669:         }
670:     }
671:     if (leadingZeros)
672:     {
673:         if (absWidth == 0)
674:             absWidth = maxDigits;
675:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
676:         {
677:             result += '0';
678:         }
679:     }
680:     else
681:     {
682:         if (numDigits == 0)
683:         {
684:             result += '0';
685:         }
686:     }
687:     while (numDigits > 0)
688:     {
689:         int digit = (value / divisor) % base;
690:         result += GetDigit(digit);
691:         --numDigits;
692:         divisor /= base;
693:     }
694:     return result.align(width);
695: }
696: 
697: } // namespace baremetal
```

- Line 96-129: We implement the char specialization
- Line 147-150: We implement the int8 specialization
- Line 168-171: We implement the uint8 specialization
- Line 189-192: We implement the int16 specialization
- Line 210-213: We implement the uint16 specialization
- Line 231-234: We implement the int32 specialization
- Line 252-255: We implement the uint32 specialization
- Line 273-276: We implement the int64 specialization
- Line 294-297: We implement the uint64 specialization
- Line 311-361: We implement the float specialization
- Line 375-425: We implement the double specialization
- Line 437-440: We implement the string specialization
- Line 452-463: We implement the const char* specialization
- Line 474-488: We implement the const void* specialization
- Line 499-502: We implement the void* specialization
- Line 523-604: We implement internal `SerializeInternalInt()` function
- Line 625-695: We implement internal `SerializeInternalUInt()` function

### HeapAllocator.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS___STEP_2_HEAPALLOCATORCPP}

As the `HeapAllocator` class implements part of memory management, if we wish to print debug information, we need to do so without using memory allocation.

Update the file `code/libraries/baremetal/src/HeapAllocator.cpp`

```cpp
File: code/libraries/baremetal/src/HeapAllocator.cpp
...
112: void* HeapAllocator::Allocate(size_t size)
113: {
...
144: #if BAREMETAL_MEMORY_TRACING_DETAIL
145:         LOG_NO_ALLOC_DEBUG("Reuse %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
146:         LOG_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
147: #endif
...
156:         if ((nextBlock <= m_next) ||                    // may have wrapped
157:             (nextBlock > m_limit - m_reserve))
158:         {
159: #if BAREMETAL_MEMORY_TRACING
160:             DumpStatus();
161: #endif
162:             LOG_NO_ALLOC_ERROR("%s: Out of memory", m_heapName);
163:             return nullptr;
164:         }
165: 
...
171: #if BAREMETAL_MEMORY_TRACING_DETAIL
172:         LOG_NO_ALLOC_DEBUG("Allocate %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
173:         LOG_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
174: #endif
...
183: }
...
230: void HeapAllocator::Free(void* block)
231: {
...
251: #if BAREMETAL_MEMORY_TRACING_DETAIL
252:             LOG_NO_ALLOC_DEBUG("Free %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
253:             LOG_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
254: #endif
255: #endif
256: 
257:             return;
258:         }
259:     }
260: 
261: #if BAREMETAL_MEMORY_TRACING
262:     LOG_NO_ALLOC_WARNING("%s: Trying to free large block (size %lu)", m_heapName, blockHeader->size);
263: #endif
264: }
265: 
266: #if BAREMETAL_MEMORY_TRACING
267: /// <summary>
268: /// Display the current status of the heap allocator
269: /// </summary>
270: void HeapAllocator::DumpStatus()
271: {
272:     LOG_NO_ALLOC_DEBUG("Heap allocator info:     %s", m_heapName);
273:     LOG_NO_ALLOC_DEBUG("Current #allocations:    %llu", GetCurrentAllocatedBlockCount());
274:     LOG_NO_ALLOC_DEBUG("Max #allocations:        %llu", GetMaxAllocatedBlockCount());
275:     LOG_NO_ALLOC_DEBUG("Current #allocated bytes:%llu", GetCurrentAllocationSize());
276:     LOG_NO_ALLOC_DEBUG("Total #allocated blocks: %llu", GetTotalAllocatedBlockCount());
277:     LOG_NO_ALLOC_DEBUG("Total #allocated bytes:  %llu", GetTotalAllocationSize());
278:     LOG_NO_ALLOC_DEBUG("Total #freed blocks:     %llu", GetTotalFreedBlockCount());
279:     LOG_NO_ALLOC_DEBUG("Total #freed bytes:      %llu", GetTotalFreeSize());
280: 
281:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
282:     {
283:         LOG_NO_ALLOC_DEBUG("malloc(%lu): %lu blocks (max %lu) total alloc #blocks = %llu, #bytes = %llu, total free #blocks = %llu, #bytes = %llu",
284:             bucket->size, bucket->count, bucket->maxCount, bucket->totalAllocatedCount, bucket->totalAllocated, bucket->totalFreedCount, bucket->totalFreed);
285:     }
286: }
...
386: #endif
387:
```

### String.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS___STEP_2_STRINGCPP}

It is convenient to get some more information if desired on which memory allocations and de-allocations are done by strings, so we'll add some logging there.

Update the file `code/libraries/baremetal/src/String.cpp`

```cpp
File: code/libraries/baremetal/src/String.cpp
...
79: /// <summary>
80: /// Destructor
81: ///
82: /// Frees any allocated memory.
83: /// </summary>
84: string::~string()
85: {
86: #if BAREMETAL_MEMORY_TRACING_DETAIL
87:     if (m_buffer != nullptr)
88:         LOG_NO_ALLOC_DEBUG("Free string %p", m_buffer);
89: #endif
90:     delete[] m_buffer;
91: }
...
1465: /// <summary>
1466: /// Allocate or re-allocate string to have a capacity of allocationSize bytes
1467: /// </summary>
1468: /// <param name="allocationSize">Amount of bytes to allocate space for</param>
1469: /// <returns>True if successful, false otherwise</returns>
1470: bool string::reallocate_allocation_size(size_t allocationSize)
1471: {
1472:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1473:     if (newBuffer == nullptr)
1474:     {
1475:         return false;
1476:     }
1477:     m_buffer = newBuffer;
1478: #if BAREMETAL_MEMORY_TRACING_DETAIL
1479:     LOG_NO_ALLOC_DEBUG("Alloc string %p", m_buffer);
1480: #endif
1481:     if (m_end == nullptr)
1482:         m_end = m_buffer;
1483:     if (m_end > m_buffer + allocationSize)
1484:         m_end = m_buffer + allocationSize;
1485:     m_allocatedSize = allocationSize;
1486:     return true;
1487: }
```

### Version.cpp {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_UPDATING_SERIALIZERS___STEP_2_VERSIONCPP}

The version string is composed using a call to `Format()`. We need to again use the non-allocating version.

Update the file `code/libraries/baremetal/src/Version.cpp`

```cpp
File: code/libraries/baremetal/src/Version.cpp
40: #include <baremetal/Version.h>
41: 
42: #include <stdlib/Util.h>
43: #include <baremetal/Format.h>
44: 
45: /// @file
46: /// Build version implementation
47: 
48: /// @brief Buffer size of version string buffer
49: static const size_t BufferSize = 20;
50: /// @brief Version string buffer
51: static char s_baremetalVersionString[BufferSize]{};
52: /// @brief Flag to check if version set up was already done
53: static bool s_baremetalVersionSetupDone = false;
54: 
55: /// <summary>
56: /// Set up version string
57: ///
58: /// The version string is written into a buffer without allocating memory.
59: /// This is important, as we may be logging before memory management is set up.
60: /// </summary>
61: void baremetal::SetupVersion()
62: {
63:     if (!s_baremetalVersionSetupDone)
64:     {
65:         FormatNoAlloc(s_baremetalVersionString, BufferSize, "%d.%d.%d", BAREMETAL_MAJOR_VERSION, BAREMETAL_MINOR_VERSION, BAREMETAL_LEVEL_VERSION);
66:         s_baremetalVersionSetupDone = true;
67:     }
68: }
69: 
70: /// <summary>
71: /// Return version string
72: /// </summary>
73: /// <returns>Version string</returns>
74: const char* baremetal::GetVersion()
75: {
76:     return s_baremetalVersionString;
77: }
```

## Application code {#TUTORIAL_16_SERIALIZATION_AND_FORMATTING_APPLICATION_CODE}

We'll add some tests for check whether the serialization functions work as expected.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include <stdlib/Util.h>
2: #include <baremetal/ARMInstructions.h>
3: #include <baremetal/Assert.h>
4: #include <baremetal/BCMRegisters.h>
5: #include <baremetal/Console.h>
6: #include <baremetal/Logger.h>
7: #include <baremetal/Mailbox.h>
8: #include <baremetal/MemoryManager.h>
9: #include <baremetal/New.h>
10: #include <baremetal/RPIProperties.h>
11: #include <baremetal/Serialization.h>
12: #include <baremetal/String.h>
13: #include <baremetal/SysConfig.h>
14: #include <baremetal/System.h>
15: #include <baremetal/Timer.h>
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
101:     string s("hello world");
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
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:83)
Debug  Heap allocator info:     heaplow (HeapAllocator:272)
Debug  Current #allocations:    0 (HeapAllocator:273)
Debug  Max #allocations:        0 (HeapAllocator:274)
Debug  Current #allocated bytes:0 (HeapAllocator:275)
Debug  Total #allocated blocks: 0 (HeapAllocator:276)
Debug  Total #allocated bytes:  0 (HeapAllocator:277)
Debug  Total #freed blocks:     0 (HeapAllocator:278)
Debug  Total #freed bytes:      0 (HeapAllocator:279)
Debug  malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(1024): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Info   Starting up (System:199)
Debug  Hello World! (main:24)
Info   Wait 5 seconds (main:123)
Press r to reboot, h to halt, p to fail assertion and panic
hDebug  Low heap: (MemoryManager:220)
Debug  Heap allocator info:     heaplow (HeapAllocator:272)
Debug  Current #allocations:    0 (HeapAllocator:273)
Debug  Max #allocations:        5 (HeapAllocator:274)
Debug  Current #allocated bytes:0 (HeapAllocator:275)
Debug  Total #allocated blocks: 302 (HeapAllocator:276)
Debug  Total #allocated bytes:  230528 (HeapAllocator:277)
Debug  Total #freed blocks:     302 (HeapAllocator:278)
Debug  Total #freed bytes:      230528 (HeapAllocator:279)
Debug  malloc(64): 0 blocks (max 1) total alloc #blocks = 82, #bytes = 5248, total free #blocks = 82, #bytes = 5248 (HeapAllocator:283)
Debug  malloc(1024): 0 blocks (max 4) total alloc #blocks = 220, #bytes = 225280, total free #blocks = 220, #bytes = 225280 (HeapAllocator:283)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Info   Halt (System:122)
```

As you can see we performeed a total of 302 allocations, all blocks were freed again, 82 were were 64 bytes max in size, 220 were 1024 bytes in size.
At any moment, there were a maximum of 5 blocks allocated.

Next: [17-exceptions](17-exceptions.md)

