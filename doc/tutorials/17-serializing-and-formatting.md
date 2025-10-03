# Tutorial 17: Serialization and formatting {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING}

@tableofcontents

## New tutorial setup {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/17-serialization-and-formatting`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-17.a`
- a library `output/Debug/lib/stdlib-17.a`
- an application `output/Debug/bin/17-serialization-and-formatting.elf`
- an image in `deploy/Debug/17-serialization-and-formatting-image`

## Memory allocating and non memory allocating printing - Step 1 {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1}

It would be nice if we could start using our `String` class for logging, serialization, etc.
However, sometimes we want to print things before the memory manager is initialized.
So we'll want to have a non memory-allocating version next to the new memory-allocating, string based version.

We'll first update the `Logger` to support both versions, and we'll move down from there.

### Logger.h {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_LOGGERH}

We'll add non memory-allocating methods `WriteNoAlloc()` and `WriteNoAllocV()`, and add complementary macros.

Update the file `code/libraries/baremetal/include/baremetal/Logger.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Logger.h
...
99:     void Log(const char* from, int line, LogSeverity severity, const char* message, ...);
100:     void LogV(const char* from, int line, LogSeverity severity, const char* message, va_list args);
101: 
102:     void LogNoAlloc(const char* from, int line, LogSeverity severity, const char* message, ...);
103:     void LogNoAllocV(const char* from, int line, LogSeverity severity, const char* message, va_list args);
104: 
105:     void Trace(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...);
106:     void TraceV(const char* filename, int line, const char* function, LogSeverity severity, const char* message, va_list args);
107: 
108:     void TraceNoAlloc(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...);
109:     void TraceNoAllocV(const char* filename, int line, const char* function, LogSeverity severity, const char* message, va_list args);
...
117: /// @brief Log a panic message
118: #define LOG_PANIC(...)           GetLogger().Log(From, __LINE__, LogSeverity::Panic, __VA_ARGS__)
119: /// @brief Log an error message
120: #define LOG_ERROR(...)           GetLogger().Log(From, __LINE__, LogSeverity::Error, __VA_ARGS__)
121: /// @brief Log a warning message
122: #define LOG_WARNING(...)         GetLogger().Log(From, __LINE__, LogSeverity::Warning, __VA_ARGS__)
123: /// @brief Log a info message
124: #define LOG_INFO(...)            GetLogger().Log(From, __LINE__, LogSeverity::Info, __VA_ARGS__)
125: /// @brief Log a debug message
126: #define LOG_DEBUG(...)           GetLogger().Log(From, __LINE__, LogSeverity::Debug, __VA_ARGS__)
127: 
128: /// @brief Log a message with specified severity and message string
129: #define LOG(severity, message)   GetLogger().Log(From, __LINE__, severity, message);
130: 
131: /// @brief Log a panic message
132: #define LOG_NO_ALLOC_PANIC(...)           GetLogger().LogNoAlloc(From, __LINE__, LogSeverity::Panic, __VA_ARGS__)
133: /// @brief Log an error message
134: #define LOG_NO_ALLOC_ERROR(...)           GetLogger().LogNoAlloc(From, __LINE__, LogSeverity::Error, __VA_ARGS__)
135: /// @brief Log a warning message
136: #define LOG_NO_ALLOC_WARNING(...)         GetLogger().LogNoAlloc(From, __LINE__, LogSeverity::Warning, __VA_ARGS__)
137: /// @brief Log a info message
138: #define LOG_NO_ALLOC_INFO(...)            GetLogger().LogNoAlloc(From, __LINE__, LogSeverity::Info, __VA_ARGS__)
139: /// @brief Log a debug message
140: #define LOG_NO_ALLOC_DEBUG(...)           GetLogger().LogNoAlloc(From, __LINE__, LogSeverity::Debug, __VA_ARGS__)
141: 
142: /// @brief Log a message with specified severity and message string
143: #define LOG_NO_ALLOC(severity, message)   GetLogger().LogNoAlloc(From, __LINE__, severity, message);
144: 
145: /// @brief Log a warning message
146: #define TRACE_WARNING(...)       GetLogger().Trace(__FILE_NAME__, __LINE__, __func__, LogSeverity::Warning, __VA_ARGS__)
147: /// @brief Log a info message
148: #define TRACE_INFO(...)          GetLogger().Trace(__FILE_NAME__, __LINE__, __func__, LogSeverity::Info, __VA_ARGS__)
149: /// @brief Log a debug message
150: #define TRACE_DEBUG(...)         GetLogger().Trace(__FILE_NAME__, __LINE__, __func__, LogSeverity::Debug, __VA_ARGS__)
151: 
152: /// @brief Log a message with specified severity and message string
153: #define TRACE(severity, message) GetLogger().Trace(From, __FILE_NAME__, __LINE__, __func__, severity, message);
154: 
155: /// @brief Log a warning message
156: #define TRACE_NO_ALLOC_WARNING(...)       GetLogger().TraceNoAlloc(__FILE_NAME__, __LINE__, __func__, LogSeverity::Warning, __VA_ARGS__)
157: /// @brief Log a info message
158: #define TRACE_NO_ALLOC_INFO(...)          GetLogger().TraceNoAlloc(__FILE_NAME__, __LINE__, __func__, LogSeverity::Info, __VA_ARGS__)
159: /// @brief Log a debug message
160: #define TRACE_NO_ALLOC_DEBUG(...)         GetLogger().TraceNoAlloc(__FILE_NAME__, __LINE__, __func__, LogSeverity::Debug, __VA_ARGS__)
161: 
162: /// @brief Log a message with specified severity and message string
163: #define TRACE_NO_ALLOC(severity, message) GetLogger().TraceNoAlloc(From, __FILE_NAME__, __LINE__, __func__, severity, message);
164: 
165: } // namespace baremetal
```

### Logger.cpp {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_LOGGERCPP}

We'll implement the new methods.
We'll also use the non memory-allocating macro in the `Initialize()` method.

Update the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
...
42: #include "baremetal/Console.h"
43: #include "baremetal/Format.h"
44: #include "baremetal/MachineInfo.h"
45: #include "baremetal/System.h"
46: #include "baremetal/String.h"
47: #include "baremetal/Timer.h"
48: #include "baremetal/Version.h"
49: #include "stdlib/Util.h"
...
82: /// <summary>
83: /// Initialize logger
84: /// </summary>
85: /// <returns>true on succes, false on failure</returns>
86: bool Logger::Initialize()
87: {
88:     if (m_isInitialized)
89:         return true;
90:     SetupVersion();
91:     m_isInitialized = true; // Stop reentrant calls from happening
92:     LOG_NO_ALLOC_INFO(BAREMETAL_NAME " %s started on %s (AArch64) using %s SoC", BAREMETAL_VERSION_STRING, GetMachineInfo().GetName(), GetMachineInfo().GetSoCName());
93: 
94:     return true;
95: }
...
121: /// <summary>
122: /// Write a string with variable arguments to the logger
123: /// </summary>
124: /// <param name="source">Source name or file name</param>
125: /// <param name="line">Source line number</param>
126: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
127: /// <param name="message">Formatted message string</param>
128: /// <param name="args">Variable argument list</param>
129: void Logger::LogV(const char* source, int line, LogSeverity severity, const char* message, va_list args)
130: {
131:     if (static_cast<int>(severity) > static_cast<int>(m_level))
132:         return;
133: 
134:     String lineBuffer;
135: 
136:     auto sourceString = Format(" (%s:%d)", source, line);
137: 
138:     auto messageBuffer = FormatV(message, args);
139: 
140:     switch (severity)
141:     {
142:     case LogSeverity::Panic:
143:         lineBuffer += "!Panic!";
144:         break;
145:     case LogSeverity::Error:
146:         lineBuffer += "Error  ";
147:         break;
148:     case LogSeverity::Warning:
149:         lineBuffer += "Warning";
150:         break;
151:     case LogSeverity::Info:
152:         lineBuffer += "Info   ";
153:         break;
154:     case LogSeverity::Debug:
155:         lineBuffer += "Debug  ";
156:         break;
157:     }
158: 
159:     if (m_timer != nullptr)
160:     {
161:         const size_t TimeBufferSize = 32;
162:         char timeBuffer[TimeBufferSize]{};
163:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
164:         if (strlen(timeBuffer) > 0)
165:         {
166:             lineBuffer += timeBuffer;
167:             lineBuffer += ' ';
168:         }
169:     }
170: 
171:     lineBuffer += messageBuffer;
172:     lineBuffer += sourceString;
173:     lineBuffer += "\n";
174: 
175: #if BAREMETAL_COLOR_OUTPUT
176:     switch (severity)
177:     {
178:     case LogSeverity::Panic:
179:         s_console.Write(lineBuffer, ConsoleColor::BrightRed);
180:         break;
181:     case LogSeverity::Error:
182:         s_console.Write(lineBuffer, ConsoleColor::Red);
183:         break;
184:     case LogSeverity::Warning:
185:         s_console.Write(lineBuffer, ConsoleColor::BrightYellow);
186:         break;
187:     case LogSeverity::Info:
188:         s_console.Write(lineBuffer, ConsoleColor::Cyan);
189:         break;
190:     case LogSeverity::Debug:
191:         s_console.Write(lineBuffer, ConsoleColor::Yellow);
192:         break;
193:     default:
194:         s_console.Write(lineBuffer, ConsoleColor::White);
195:         break;
196:     }
197: #else
198:     s_console.Write(lineBuffer);
199: #endif
200: 
201:     if (severity == LogSeverity::Panic)
202:     {
203:         GetSystem().Halt();
204:     }
205: }
206: 
207: /// <summary>
208: /// Write a string with variable arguments to the logger, not using memory allocation
209: /// </summary>
210: /// <param name="source">Source name or file name</param>
211: /// <param name="line">Source line number</param>
212: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
213: /// <param name="message">Formatted message string, with variable arguments</param>
214: void Logger::LogNoAlloc(const char* source, int line, LogSeverity severity, const char* message, ...)
215: {
216:     va_list var;
217:     va_start(var, message);
218:     LogNoAllocV(source, line, severity, message, var);
219:     va_end(var);
220: }
221: 
222: /// <summary>
223: /// Write a string with variable arguments to the logger, not using memory allocation
224: /// </summary>
225: /// <param name="source">Source name or file name</param>
226: /// <param name="line">Source line number</param>
227: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
228: /// <param name="message">Formatted message string</param>
229: /// <param name="args">Variable argument list</param>
230: void Logger::LogNoAllocV(const char* source, int line, LogSeverity severity, const char* message, va_list args)
231: {
232:     if (static_cast<int>(severity) > static_cast<int>(m_level))
233:         return;
234: 
235:     static const size_t BufferSize = 1024;
236:     char buffer[BufferSize]{};
237: 
238:     char sourceString[BufferSize]{};
239:     Format(sourceString, BufferSize, " (%s:%d)", source, line);
240: 
241:     char messageBuffer[BufferSize]{};
242:     FormatV(messageBuffer, BufferSize, message, args);
243: 
244:     switch (severity)
245:     {
246:     case LogSeverity::Panic:
247:         strncat(buffer, "!Panic!", BufferSize);
248:         break;
249:     case LogSeverity::Error:
250:         strncat(buffer, "Error  ", BufferSize);
251:         break;
252:     case LogSeverity::Warning:
253:         strncat(buffer, "Warning", BufferSize);
254:         break;
255:     case LogSeverity::Info:
256:         strncat(buffer, "Info   ", BufferSize);
257:         break;
258:     case LogSeverity::Debug:
259:         strncat(buffer, "Debug  ", BufferSize);
260:         break;
261:     }
262: 
263:     if (m_timer != nullptr)
264:     {
265:         const size_t TimeBufferSize = 32;
266:         char timeBuffer[TimeBufferSize]{};
267:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
268:         if (strlen(timeBuffer) > 0)
269:         {
270:             strncat(buffer, timeBuffer, BufferSize);
271:             strncat(buffer, " ", BufferSize);
272:         }
273:     }
274: 
275:     strncat(buffer, messageBuffer, BufferSize);
276:     strncat(buffer, sourceString, BufferSize);
277:     strncat(buffer, "\n", BufferSize);
278: 
279: #if BAREMETAL_COLOR_OUTPUT
280:     switch (severity)
281:     {
282:     case LogSeverity::Panic:
283:         s_console.Write(buffer, ConsoleColor::BrightRed);
284:         break;
285:     case LogSeverity::Error:
286:         s_console.Write(buffer, ConsoleColor::Red);
287:         break;
288:     case LogSeverity::Warning:
289:         s_console.Write(buffer, ConsoleColor::BrightYellow);
290:         break;
291:     case LogSeverity::Info:
292:         s_console.Write(buffer, ConsoleColor::Cyan);
293:         break;
294:     case LogSeverity::Debug:
295:         s_console.Write(buffer, ConsoleColor::Yellow);
296:         break;
297:     default:
298:         s_console.Write(buffer, ConsoleColor::White);
299:         break;
300:     }
301: #else
302:     s_console.Write(buffer);
303: #endif
304: 
305:     if (severity == LogSeverity::Panic)
306:     {
307:         GetSystem().Halt();
308:     }
309: }
310: 
...
327: /// <summary>
328: /// Write a trace string with variable arguments to the logger
329: /// </summary>
330: /// <param name="filename">File name</param>
331: /// <param name="line">Source line number</param>
332: /// <param name="function">Function name</param>
333: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
334: /// <param name="message">Formatted message string</param>
335: /// <param name="args">Variable argument list</param>
336: void Logger::TraceV(const char* filename, int line, const char* function, LogSeverity severity, const char* message, va_list args)
337: {
338:     if (static_cast<int>(severity) > static_cast<int>(m_level))
339:         return;
340: 
341:     String lineBuffer;
342: 
343:     auto sourceString = Format(" (%s:%d)", source, line);
344: 
345:     auto messageBuffer = FormatV(message, args);
346: 
347:     switch (severity)
348:     {
349:     case LogSeverity::Warning:
350:         lineBuffer += "Warning";
351:         break;
352:     case LogSeverity::Info:
353:         lineBuffer += "Info   ";
354:         break;
355:     case LogSeverity::Debug:
356:         lineBuffer += "Debug  ";
357:         break;
358:     default:
359:         break;
360:     }
361: 
362:     if (m_timer != nullptr)
363:     {
364:         const size_t TimeBufferSize = 32;
365:         char timeBuffer[TimeBufferSize]{};
366:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
367:         if (strlen(timeBuffer) > 0)
368:         {
369:             lineBuffer += timeBuffer;
370:             lineBuffer += ' ';
371:         }
372:     }
373: 
374:     lineBuffer += messageBuffer;
375:     lineBuffer += sourceString;
376:     lineBuffer += "\n";
377: 
378: #if BAREMETAL_COLOR_OUTPUT
379:     switch (severity)
380:     {
381:     case LogSeverity::Warning:
382:         s_console.Write(lineBuffer, ConsoleColor::BrightYellow);
383:         break;
384:     case LogSeverity::Info:
385:         s_console.Write(lineBuffer, ConsoleColor::Cyan);
386:         break;
387:     case LogSeverity::Debug:
388:         s_console.Write(lineBuffer, ConsoleColor::Yellow);
389:         break;
390:     default:
391:         s_console.Write(lineBuffer, ConsoleColor::White);
392:         break;
393:     }
394: #else
395:     s_console.Write(lineBuffer);
396: #endif
397: }
398: 
399: /// <summary>
400: /// Write a trace string with variable arguments to the logger, not using memory allocation
401: /// </summary>
402: /// <param name="filename">File name</param>
403: /// <param name="line">Source line number</param>
404: /// <param name="function">Function name</param>
405: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
406: /// <param name="message">Formatted message string, with variable arguments</param>
407: void Logger::TraceNoAlloc(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...)
408: {
409:     va_list var;
410:     va_start(var, message);
411:     TraceNoAllocV(filename, line, function, severity, message, var);
412:     va_end(var);
413: }
414: 
415: /// <summary>
416: /// Write a trace string with variable arguments to the logger, not using memory allocation
417: /// </summary>
418: /// <param name="filename">File name</param>
419: /// <param name="line">Source line number</param>
420: /// <param name="function">Function name</param>
421: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
422: /// <param name="message">Formatted message string</param>
423: /// <param name="args">Variable argument list</param>
424: void Logger::TraceNoAllocV(const char* filename, int line, const char* function, LogSeverity severity, const char* message, va_list args)
425: {
426:     if (static_cast<int>(severity) > static_cast<int>(m_level))
427:         return;
428: 
429:     static const size_t BufferSize = 1024;
430:     char buffer[BufferSize]{};
431: 
432:     char sourceString[BufferSize]{};
433:     Format(sourceString, BufferSize, "%s (%s:%d) ", function, filename, line);
434: 
435:     char messageBuffer[BufferSize]{};
436:     FormatV(messageBuffer, BufferSize, message, args);
437: 
438:     switch (severity)
439:     {
440:     case LogSeverity::Warning:
441:         strncat(buffer, "Warning", BufferSize);
442:         break;
443:     case LogSeverity::Info:
444:         strncat(buffer, "Info   ", BufferSize);
445:         break;
446:     case LogSeverity::Debug:
447:         strncat(buffer, "Debug  ", BufferSize);
448:         break;
449:     default:
450:         break;
451:     }
452: 
453:     if (m_timer != nullptr)
454:     {
455:         const size_t TimeBufferSize = 32;
456:         char timeBuffer[TimeBufferSize]{};
457:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
458:         if (strlen(timeBuffer) > 0)
459:         {
460:             strncat(buffer, timeBuffer, BufferSize);
461:             strncat(buffer, " ", BufferSize);
462:         }
463:     }
464: 
465:     strncat(buffer, sourceString, BufferSize);
466:     strncat(buffer, messageBuffer, BufferSize);
467:     strncat(buffer, "\n", BufferSize);
468: 
469: #if BAREMETAL_COLOR_OUTPUT
470:     switch (severity)
471:     {
472:     case LogSeverity::Warning:
473:         s_console.Write(buffer, ConsoleColor::BrightYellow);
474:         break;
475:     case LogSeverity::Info:
476:         s_console.Write(buffer, ConsoleColor::Cyan);
477:         break;
478:     case LogSeverity::Debug:
479:         s_console.Write(buffer, ConsoleColor::Yellow);
480:         break;
481:     default:
482:         s_console.Write(buffer, ConsoleColor::White);
483:         break;
484:     }
485: #else
486:     s_console.Write(buffer);
487: #endif
488: }
489: 
...
```

- Line 45: As we're going to use the `String` class, we include its header
- Line 92: We use the macro `LOG_NO_ALLOC_INFO` to make sure the first line printed by the logger does not need memory allocation
- Line 121-205: The new, memory-allocating version of `LogV` will use the `String` class
  - Line 134: We compose the line in a string
  - Line 136: We compose the source line information in a string. We use the memory allocating version of `Format()` for this
  - Line 138: We compose the message in a string. We use the memory allocating version of `FormatV()` for this
  - Line 140-173: We append to the line using the add operator `+=`
- Line 207-220: We implement the non memory-allocating `LogNoAlloc()`. This uses `LogNoAllocV()` for output
- Line 222-309: We implement the non memory-allocating `LogNoAllocV()`. This is the original function, but using `FormatNoAlloc()` and `FormatNoAllocV()`
- Line 327-397: The new, memory-allocating version of `TraceV()` will use the `String` class
  - Line 341: We compose the line in a string
  - Line 343: We compose the source line information in a string. We use the memory allocating version of `Format()` for this
  - Line 345: We compose the message in a string. We use the memory allocating version of `FormatV()` for this
  - Line 347-376: We append to the line using the add operator `+=`
- Line 399-413: We implement the non memory-allocating `TraceNoAlloc()`. This uses `TraceNoAllocV()` for output
- Line 415-488: We implement the non memory-allocating `TraceNoAllocV()`. This is the original function, but using `FormatNoAlloc()` and `FormatNoAllocV()`

### Format.h {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_FORMATH}

We'll add non memory-allocating methods `FormatNoAlloc()` and `FormatNoAllocV()`, and convert `Format()` and `FormatV()` into string returning versions.

Update the file `code/libraries/baremetal/include/baremetal/Format.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Format.h
...
42: /// @file
43: /// Formatting functionality
44: 
45: #include "stdlib/StdArg.h"
46: #include "stdlib/Types.h"
47: 
48: namespace baremetal {
49: 
50: class String;
51: 
52: String FormatV(const char* format, va_list args);
53: String Format(const char* format, ...);
54: void FormatNoAllocV(char* buffer, size_t bufferSize, const char* format, va_list args);
55: void FormatNoAlloc(char* buffer, size_t bufferSize, const char* format, ...);
56: 
57: } // namespace baremetal
```

### Format.cpp {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_FORMATCPP}

We'll implement the new methods.

Update the file `code/libraries/baremetal/src/Format.cpp`

```cpp
File: code/libraries/baremetal/src/Format.cpp
...
42: #include "baremetal/Serialization.h"
43: #include "baremetal/String.h"
44: #include "stdlib/Util.h"
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
195: 
196: /// <summary>
197: /// PrintValue unsigned long long int value, type specific specialization
198: /// </summary>
199: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
200: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
201: /// <param name="value">Value to be serialized</param>
202: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
203: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
204: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
205: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
206: inline static void PrintValue(char* buffer, size_t bufferSize, unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
207: {
208:     PrintValue(buffer, bufferSize, static_cast<uint64>(value), width, base, showBase, leadingZeros);
209: }
210: 
211: /// <summary>
212: /// PrintValue a double value to buffer. The value will be printed as a fixed point number.
213: ///
214: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, the string is terminated to hold maximum bufferSize - 1 characters.
215: /// Width is currently unused.
216: /// Precision specifies the number of digits behind the decimal pointer
217: /// </summary>
218: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
219: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
220: /// <param name="value">Value to be serialized</param>
221: /// <param name="width">Unused</param>
222: /// <param name="precision">Number of digits after the decimal point to use</param>
223: static void PrintValue(char* buffer, size_t bufferSize, double value, int width, int precision)
224: {
225:     bool negative{};
226:     if (value < 0)
227:     {
228:         negative = true;
229:         value = -value;
230:     }
231: 
232:     if (bufferSize == 0)
233:         return;
234: 
235:     // We can only print values with integral parts up to what uint64 can hold
236:     if (value > static_cast<double>(static_cast<uint64>(-1)))
237:     {
238:         strncpy(buffer, "overflow", bufferSize);
239:         return;
240:     }
241: 
242:     *buffer = '\0';
243:     if (negative)
244:         strncpy(buffer, "-", bufferSize);
245: 
246:     uint64 integralPart = static_cast<uint64>(value);
247:     const size_t TmpBufferSize = 32;
248:     char tmpBuffer[TmpBufferSize];
249:     PrintValue(tmpBuffer, TmpBufferSize, integralPart, 0, 10, false, false);
250:     strncat(buffer, tmpBuffer, bufferSize);
251:     const int MaxPrecision = 7;
252: 
253:     if (precision != 0)
254:     {
255:         strncat(buffer, ".", bufferSize);
256: 
257:         if (precision > MaxPrecision)
258:         {
259:             precision = MaxPrecision;
260:         }
261: 
262:         uint64 precisionPower10 = 1;
263:         for (int i = 1; i <= precision; i++)
264:         {
265:             precisionPower10 *= 10;
266:         }
267: 
268:         value -= static_cast<double>(integralPart);
269:         value *= static_cast<double>(precisionPower10);
270: 
271:         PrintValue(tmpBuffer, TmpBufferSize, static_cast<uint64>(value), 0, 10, false, false);
272:         strncat(buffer, tmpBuffer, bufferSize);
273:         precision -= strlen(tmpBuffer);
274:         while (precision--)
275:         {
276:             strncat(buffer, "0", bufferSize);
277:         }
278:     }
279: }
280: 
281: /// <summary>
282: /// PrintValue a const char * value to buffer. The value can be quoted.
283: ///
284: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
285: /// If quote is true, the string is printed within double quotes (\")
286: /// </summary>
287: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
288: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
289: /// <param name="value">String to be serialized</param>
290: /// <param name="width">Unused</param>
291: /// <param name="quote">If true, value is printed between double quotes, if false, no quotes are used</param>
292: static void PrintValue(char* buffer, size_t bufferSize, const char* value, int width, bool quote)
293: {
294:     size_t numChars = strlen(value);
295:     if (quote)
296:         numChars += 2;
297: 
298:     // Leave one character for \0
299:     if (numChars > bufferSize - 1)
300:         return;
301: 
302:     char* bufferPtr = buffer;
303: 
304:     if (quote)
305:         *bufferPtr++ = '\"';
306:     while (*value)
307:     {
308:         *bufferPtr++ = *value++;
309:     }
310:     if (quote)
311:         *bufferPtr++ = '\"';
312: }
313: 
314: /// <summary>
315: /// Internal serialization function, to be used for all signed values.
316: ///
317: /// PrintValue a signed value to buffer.
318: ///
319: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
320: /// Width specifies the minimum width in characters. The value is always written right aligned.
321: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
322: ///
323: /// Base is the digit base, which can range from 2 to 36.
324: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
325: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
326: /// </summary>
327: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
328: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
329: /// <param name="value">Value to be serialized</param>
330: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
331: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
332: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
333: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
334: /// <param name="numBits">Specifies the number of bits used for the value</param>
335: static void PrintValueInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
336: {
337:     if ((base < 2) || (base > 36))
338:         return;
339: 
340:     int numDigits = 0;
341:     bool negative = (value < 0);
342:     uint64 absVal = static_cast<uint64>(negative ? -value : value);
343:     uint64 divisor = 1;
344:     uint64 divisorLast = 1;
345:     size_t absWidth = (width < 0) ? -width : width;
346:     const int maxDigits = BitsToDigits(numBits, base);
347:     while ((absVal >= divisor) && (numDigits <= maxDigits))
348:     {
349:         divisorLast = divisor;
350:         divisor *= base;
351:         ++numDigits;
352:     }
353:     divisor = divisorLast;
354: 
355:     size_t numChars = (numDigits > 0) ? numDigits : 1;
356:     if (showBase)
357:     {
358:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
359:     }
360:     if (negative)
361:     {
362:         numChars++;
363:     }
364:     if (absWidth > numChars)
365:         numChars = absWidth;
366:     // Leave one character for \0
367:     if (numChars > bufferSize - 1)
368:         return;
369: 
370:     char* bufferPtr = buffer;
371:     if (negative)
372:     {
373:         *bufferPtr++ = '-';
374:     }
375: 
376:     if (showBase)
377:     {
378:         if (base == 2)
379:         {
380:             *bufferPtr++ = '0';
381:             *bufferPtr++ = 'b';
382:         }
383:         else if (base == 8)
384:         {
385:             *bufferPtr++ = '0';
386:         }
387:         else if (base == 16)
388:         {
389:             *bufferPtr++ = '0';
390:             *bufferPtr++ = 'x';
391:         }
392:     }
393:     if (leadingZeros)
394:     {
395:         if (absWidth == 0)
396:             absWidth = maxDigits;
397:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
398:         {
399:             *bufferPtr++ = '0';
400:         }
401:     }
402:     else
403:     {
404:         if (numDigits == 0)
405:         {
406:             *bufferPtr++ = '0';
407:         }
408:     }
409:     while (numDigits > 0)
410:     {
411:         int digit = (absVal / divisor) % base;
412:         *bufferPtr++ = GetDigit(digit);
413:         --numDigits;
414:         divisor /= base;
415:     }
416:     *bufferPtr++ = '\0';
417: }
418: 
419: /// <summary>
420: /// Internal serialization function, to be used for all unsigned values.
421: ///
422: /// PrintValue a unsigned value to buffer.
423: ///
424: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
425: /// Width specifies the minimum width in characters. The value is always written right aligned.
426: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
427: ///
428: /// Base is the digit base, which can range from 2 to 36.
429: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
430: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
431: /// </summary>
432: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
433: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
434: /// <param name="value">Value to be serialized</param>
435: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
436: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
437: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
438: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
439: /// <param name="numBits">Specifies the number of bits used for the value</param>
440: static void PrintValueInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
441: {
442:     if ((base < 2) || (base > 36))
443:         return;
444: 
445:     int numDigits = 0;
446:     uint64 divisor = 1;
447:     uint64 divisorLast = 1;
448:     uint64 divisorHigh = 0;
449:     size_t absWidth = (width < 0) ? -width : width;
450:     const int maxDigits = BitsToDigits(numBits, base);
451:     while ((divisorHigh == 0) && (value >= divisor) && (numDigits <= maxDigits))
452:     {
453:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
454:         divisorLast = divisor;
455:         divisor *= base;
456:         ++numDigits;
457:     }
458:     divisor = divisorLast;
459: 
460:     size_t numChars = (numDigits > 0) ? numDigits : 1;
461:     if (showBase)
462:     {
463:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
464:     }
465:     if (absWidth > numChars)
466:         numChars = absWidth;
467:     // Leave one character for \0
468:     if (numChars > bufferSize - 1)
469:         return;
470: 
471:     char* bufferPtr = buffer;
472: 
473:     if (showBase)
474:     {
475:         if (base == 2)
476:         {
477:             *bufferPtr++ = '0';
478:             *bufferPtr++ = 'b';
479:         }
480:         else if (base == 8)
481:         {
482:             *bufferPtr++ = '0';
483:         }
484:         else if (base == 16)
485:         {
486:             *bufferPtr++ = '0';
487:             *bufferPtr++ = 'x';
488:         }
489:     }
490:     if (leadingZeros)
491:     {
492:         if (absWidth == 0)
493:             absWidth = maxDigits;
494:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
495:         {
496:             *bufferPtr++ = '0';
497:         }
498:     }
499:     else
500:     {
501:         if (numDigits == 0)
502:         {
503:             *bufferPtr++ = '0';
504:         }
505:     }
506:     while (numDigits > 0)
507:     {
508:         int digit = (value / divisor) % base;
509:         *bufferPtr++ = GetDigit(digit);
510:         --numDigits;
511:         divisor /= base;
512:     }
513:     *bufferPtr++ = '\0';
514: }
515: 
516: /// <summary>
517: /// Size of buffer used by FormatNoAllocV internally
518: /// </summary>
519: static const size_t BufferSize = 4096;
...
574: /// <summary>
575: /// Format a string
576: ///
577: /// This version of Format uses the string class, and thus allocates memory
578: /// </summary>
579: /// <param name="format">Format string (printf like)</param>
580: /// <returns>Resulting string</returns>
581: String Format(const char* format, ...)
582: {
583:     va_list var;
584:     va_start(var, format);
585: 
586:     String result = FormatV(format, var);
587: 
588:     va_end(var);
589: 
590:     return result;
591: }
592: 
593: /// <summary>
594: /// Format a string
595: ///
596: /// This version of FormatV uses the string class, and thus allocates memory
597: /// </summary>
598: /// <param name="format">Format string (printf like)</param>
599: /// <param name="args">Variable argument list</param>
600: /// <returns>Resulting string</returns>
601: String FormatV(const char* format, va_list args)
602: {
603:     String result;
604: 
605:     while (*format != '\0')
606:     {
607:         if (*format == '%')
608:         {
609:             if (*++format == '%')
610:             {
611:                 result += '%';
612:                 format++;
613:                 continue;
614:             }
615: 
616:             bool alternate = false;
617:             if (*format == '#')
618:             {
619:                 alternate = true;
620:                 format++;
621:             }
622: 
623:             bool left = false;
624:             if (*format == '-')
625:             {
626:                 left = true;
627:                 format++;
628:             }
629: 
630:             bool leadingZero = false;
631:             if (*format == '0')
632:             {
633:                 leadingZero = true;
634:                 format++;
635:             }
636: 
637:             size_t width = 0;
638:             while (('0' <= *format) && (*format <= '9'))
639:             {
640:                 width = width * 10 + (*format - '0');
641:                 format++;
642:             }
643: 
644:             unsigned precision = 6;
645:             if (*format == '.')
646:             {
647:                 format++;
648:                 precision = 0;
649:                 while ('0' <= *format && *format <= '9')
650:                 {
651:                     precision = precision * 10 + (*format - '0');
652: 
653:                     format++;
654:                 }
655:             }
656: 
657:             bool haveLong{};
658:             bool haveLongLong{};
659: 
660:             if (*format == 'l')
661:             {
662:                 if (*(format + 1) == 'l')
663:                 {
664:                     haveLongLong = true;
665: 
666:                     format++;
667:                 }
668:                 else
669:                 {
670:                     haveLong = true;
671:                 }
672: 
673:                 format++;
674:             }
675: 
676:             switch (*format)
677:             {
678:             case 'c':
679:                 {
680:                     char ch = static_cast<char>(va_arg(args, int));
681:                     if (left)
682:                     {
683:                         result += ch;
684:                         if (width > 1)
685:                         {
686:                             result.append(width - 1, ' ');
687:                         }
688:                     }
689:                     else
690:                     {
691:                         if (width > 1)
692:                         {
693:                             result.append(width - 1, ' ');
694:                         }
695:                         result += ch;
696:                     }
697:                 }
698:                 break;
699: 
700:             case 'd':
701:             case 'i':
702:                 if (haveLongLong)
703:                 {
704:                     result.append(Serialize(va_arg(args, int64), left ? -width : width, 10, false, leadingZero));
705:                 }
706:                 else if (haveLong)
707:                 {
708:                     result.append(Serialize(va_arg(args, int32), left ? -width : width, 10, false, leadingZero));
709:                 }
710:                 else
711:                 {
712:                     result.append(Serialize(va_arg(args, int), left ? -width : width, 10, false, leadingZero));
713:                 }
714:                 break;
715: 
716:             case 'f':
717:                 {
718:                     result.append(Serialize(va_arg(args, double), left ? -width : width, precision));
719:                 }
720:                 break;
721: 
722:             case 'b':
723:                 if (alternate)
724:                 {
725:                     result.append("0b");
726:                 }
727:                 if (haveLongLong)
728:                 {
729:                     result.append(Serialize(va_arg(args, uint64), left ? -width : width, 2, false, leadingZero));
730:                 }
731:                 else if (haveLong)
732:                 {
733:                     result.append(Serialize(va_arg(args, uint32), left ? -width : width, 2, false, leadingZero));
734:                 }
735:                 else
736:                 {
737:                     result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero));
738:                 }
739:                 break;
740: 
741:             case 'o':
742:                 if (alternate)
743:                 {
744:                     result.append("0");
745:                 }
746:                 if (haveLongLong)
747:                 {
748:                     result.append(Serialize(va_arg(args, uint64), left ? -width : width, 8, false, leadingZero));
749:                 }
750:                 else if (haveLong)
751:                 {
752:                     result.append(Serialize(va_arg(args, uint32), left ? -width : width, 8, false, leadingZero));
753:                 }
754:                 else
755:                 {
756:                     result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero));
757:                 }
758:                 break;
759: 
760:             case 's':
761:                 {
762:                     result.append(Serialize(va_arg(args, const char*), left ? -width : width, false));
763:                 }
764:                 break;
765: 
766:             case 'u':
767:                 if (haveLongLong)
768:                 {
769:                     result.append(Serialize(va_arg(args, uint64), left ? -width : width, 10, false, leadingZero));
770:                 }
771:                 else if (haveLong)
772:                 {
773:                     result.append(Serialize(va_arg(args, uint32), left ? -width : width, 10, false, leadingZero));
774:                 }
775:                 else
776:                 {
777:                     result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero));
778:                 }
779:                 break;
780: 
781:             case 'x':
782:             case 'X':
783:                 if (alternate)
784:                 {
785:                     result.append("0x");
786:                 }
787:                 if (haveLongLong)
788:                 {
789:                     result.append(Serialize(va_arg(args, uint64), left ? -width : width, 16, false, leadingZero));
790:                 }
791:                 else if (haveLong)
792:                 {
793:                     result.append(Serialize(va_arg(args, uint32), left ? -width : width, 16, false, leadingZero));
794:                 }
795:                 else
796:                 {
797:                     result.append(Serialize(va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero));
798:                 }
799:                 break;
800: 
801:             case 'p':
802:                 if (alternate)
803:                 {
804:                     result.append("0x");
805:                 }
806:                 {
807:                     result.append(Serialize(va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero));
808:                 }
809:                 break;
810: 
811:             default:
812:                 result += '%';
813:                 result += *format;
814:                 break;
815:             }
816:         }
817:         else
818:         {
819:             result += *format;
820:         }
821: 
822:         format++;
823:     }
824: 
825:     return result;
826: }
827: 
828: /// <summary>
829: /// Print a formatted string to a buffer, not using memory allocation
830: ///
831: /// This uses variable arguments
832: /// </summary>
833: /// <param name="buffer">Pointer to destination buffer</param>
834: /// <param name="bufferSize">Size of the buffer</param>
835: /// <param name="format">Format string</param>
836: void FormatNoAlloc(char* buffer, size_t bufferSize, const char* format, ...)
837: {
838:     va_list var;
839:     va_start(var, format);
840: 
841:     FormatNoAllocV(buffer, bufferSize, format, var);
842: 
843:     va_end(var);
844: }
845: 
846: /// <summary>
847: /// Print a formatted string to a buffer, not using memory allocation
848: ///
849: /// This uses a variable argument list
850: /// </summary>
851: /// <param name="buffer">Pointer to destination buffer</param>
852: /// <param name="bufferSize">Size of the buffer</param>
853: /// <param name="format">Format string</param>
854: /// <param name="args">Variable arguments list</param>
855: void FormatNoAllocV(char* buffer, size_t bufferSize, const char* format, va_list args)
856: {
857:     if (buffer == nullptr)
858:         return;
859:     buffer[0] = '\0';
860: 
861:     while (*format != '\0')
862:     {
863:         if (*format == '%')
864:         {
865:             if (*++format == '%')
866:             {
867:                 Append(buffer, bufferSize, '%');
868:                 format++;
869:                 continue;
870:             }
871: 
872:             bool alternate = false;
873:             if (*format == '#')
874:             {
875:                 alternate = true;
876:                 format++;
877:             }
878: 
879:             bool left = false;
880:             if (*format == '-')
881:             {
882:                 left = true;
883:                 format++;
884:             }
885: 
886:             bool leadingZero = false;
887:             if (*format == '0')
888:             {
889:                 leadingZero = true;
890:                 format++;
891:             }
892: 
893:             size_t width = 0;
894:             while (('0' <= *format) && (*format <= '9'))
895:             {
896:                 width = width * 10 + (*format - '0');
897:                 format++;
898:             }
899: 
900:             unsigned precision = 6;
901:             if (*format == '.')
902:             {
903:                 format++;
904:                 precision = 0;
905:                 while ('0' <= *format && *format <= '9')
906:                 {
907:                     precision = precision * 10 + (*format - '0');
908: 
909:                     format++;
910:                 }
911:             }
912: 
913:             bool haveLong{};
914:             bool haveLongLong{};
915: 
916:             if (*format == 'l')
917:             {
918:                 if (*(format + 1) == 'l')
919:                 {
920:                     haveLongLong = true;
921: 
922:                     format++;
923:                 }
924:                 else
925:                 {
926:                     haveLong = true;
927:                 }
928: 
929:                 format++;
930:             }
931: 
932:             switch (*format)
933:             {
934:             case 'c':
935:                 {
936:                     char ch = static_cast<char>(va_arg(args, int));
937:                     if (left)
938:                     {
939:                         Append(buffer, bufferSize, ch);
940:                         if (width > 1)
941:                         {
942:                             Append(buffer, bufferSize, width - 1, ' ');
943:                         }
944:                     }
945:                     else
946:                     {
947:                         if (width > 1)
948:                         {
949:                             Append(buffer, bufferSize, width - 1, ' ');
950:                         }
951:                         Append(buffer, bufferSize, ch);
952:                     }
953:                 }
954:                 break;
955: 
956:             case 'd':
957:             case 'i':
958:                 if (haveLongLong)
959:                 {
960:                     char str[BufferSize]{};
961:                     Serialize(str, BufferSize, va_arg(args, int64), left ? -width : width, 10, false, leadingZero);
962:                     Append(buffer, bufferSize, str);
963:                 }
964:                 else if (haveLong)
965:                 {
966:                     char str[BufferSize]{};
967:                     Serialize(str, BufferSize, va_arg(args, int32), left ? -width : width, 10, false, leadingZero);
968:                     Append(buffer, bufferSize, str);
969:                 }
970:                 else
971:                 {
972:                     char str[BufferSize]{};
973:                     Serialize(str, BufferSize, va_arg(args, int), left ? -width : width, 10, false, leadingZero);
974:                     Append(buffer, bufferSize, str);
975:                 }
976:                 break;
977: 
978:             case 'f':
979:                 {
980:                     char str[BufferSize]{};
981:                     Serialize(str, BufferSize, va_arg(args, double), left ? -width : width, precision);
982:                     Append(buffer, bufferSize, str);
983:                 }
984:                 break;
985: 
986:             case 'b':
987:                 if (alternate)
988:                 {
989:                     Append(buffer, bufferSize, "0b");
990:                 }
991:                 if (haveLongLong)
992:                 {
993:                     char str[BufferSize]{};
994:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 2, false, leadingZero);
995:                     Append(buffer, bufferSize, str);
996:                 }
997:                 else if (haveLong)
998:                 {
999:                     char str[BufferSize]{};
1000:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 2, false, leadingZero);
1001:                     Append(buffer, bufferSize, str);
1002:                 }
1003:                 else
1004:                 {
1005:                     char str[BufferSize]{};
1006:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero);
1007:                     Append(buffer, bufferSize, str);
1008:                 }
1009:                 break;
1010: 
1011:             case 'o':
1012:                 if (alternate)
1013:                 {
1014:                     Append(buffer, bufferSize, '0');
1015:                 }
1016:                 if (haveLongLong)
1017:                 {
1018:                     char str[BufferSize]{};
1019:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 8, false, leadingZero);
1020:                     Append(buffer, bufferSize, str);
1021:                 }
1022:                 else if (haveLong)
1023:                 {
1024:                     char str[BufferSize]{};
1025:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 8, false, leadingZero);
1026:                     Append(buffer, bufferSize, str);
1027:                 }
1028:                 else
1029:                 {
1030:                     char str[BufferSize]{};
1031:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero);
1032:                     Append(buffer, bufferSize, str);
1033:                 }
1034:                 break;
1035: 
1036:             case 's':
1037:                 {
1038:                     char str[BufferSize]{};
1039:                     Serialize(str, BufferSize, va_arg(args, const char*), left ? -width : width, false);
1040:                     Append(buffer, bufferSize, str);
1041:                 }
1042:                 break;
1043: 
1044:             case 'u':
1045:                 if (haveLongLong)
1046:                 {
1047:                     char str[BufferSize]{};
1048:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 10, false, leadingZero);
1049:                     Append(buffer, bufferSize, str);
1050:                 }
1051:                 else if (haveLong)
1052:                 {
1053:                     char str[BufferSize]{};
1054:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 10, false, leadingZero);
1055:                     Append(buffer, bufferSize, str);
1056:                 }
1057:                 else
1058:                 {
1059:                     char str[BufferSize]{};
1060:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero);
1061:                     Append(buffer, bufferSize, str);
1062:                 }
1063:                 break;
1064: 
1065:             case 'x':
1066:             case 'X':
1067:                 if (alternate)
1068:                 {
1069:                     Append(buffer, bufferSize, "0x");
1070:                 }
1071:                 if (haveLongLong)
1072:                 {
1073:                     char str[BufferSize]{};
1074:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 16, false, leadingZero);
1075:                     Append(buffer, bufferSize, str);
1076:                 }
1077:                 else if (haveLong)
1078:                 {
1079:                     char str[BufferSize]{};
1080:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 16, false, leadingZero);
1081:                     Append(buffer, bufferSize, str);
1082:                 }
1083:                 else
1084:                 {
1085:                     char str[BufferSize]{};
1086:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero);
1087:                     Append(buffer, bufferSize, str);
1088:                 }
1089:                 break;
1090: 
1091:             case 'p':
1092:                 if (alternate)
1093:                 {
1094:                     Append(buffer, bufferSize, "0x");
1095:                 }
1096:                 {
1097:                     char str[BufferSize]{};
1098:                     Serialize(str, BufferSize, va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero);
1099:                     Append(buffer, bufferSize, str);
1100:                 }
1101:                 break;
1102: 
1103:             default:
1104:                 Append(buffer, bufferSize, '%');
1105:                 Append(buffer, bufferSize, *format);
1106:                 break;
1107:             }
1108:         }
1109:         else
1110:         {
1111:             Append(buffer, bufferSize, *format);
1112:         }
1113: 
1114:         format++;
1115:     }
1116: }
1117: 
1118: } // namespace baremetal
```

- Line 43: We need the `String` class, so we'll add its header
- Line 51-52: We copy the variable Uppercase from `Serialization.cpp`
- Line 54-55: We copy the functions `SerializeInteralUInt()` and `SerializeInteralInt()` from `Serialization.cpp` and rename `Serialization` to `PrintValue`
- Line 57-65: We also copy the function `GetDigit()`
- Line 67-87: We also copy the function `BitsToDigits()`
- Line 89-110: We also copy the function `Serialize()` for int32, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 112-133: We also copy the function `Serialize()` for uint32, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 135-156: We also copy the function `Serialize()` for int64, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 158-179: We also copy the function `Serialize()` for uint64, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 181-194: We also copy the function `Serialize()` for long long from `Serialization.h`, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 196-209: We also copy the function `Serialize()` for unsigned long long from `Serialization.h`, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 211-279: We also copy the function `Serialize()` for double, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 281-312: We also copy the function `Serialize()` for const char*, and rename it to `PrintValue()`, we make it static as it it only intended to be used here
- Line 314-417: We also copy the function `SerializeInternalInt()`, and rename it to `PrintValueInternalInt()`
- Line 419-514: We also copy the function `SerializeInternalUInt()`, and rename it to `PrintValueInternalUInt()`
- Line 521-572: We leave the `Append()` functions as is
- Line 574-591: We implement the new `Format()` function. This uses `FormatV()`
- Line 593-826: We implement the new `FormatV()` function.
This is a copy of the original `FormatV()`, but appending to a string, instead of into a buffer, and using `Serialize()` calls to serialize data.
The string is finally returned
- Line 828-844: We rename the old `Format()` to `FormatNoAlloc()` and change the call to `FormatNoAllocV()`
- Line 846-1114: We rename the old `FormatV()` to `FormatNoAllocV()`.
The implementation only changes in the calls to `Serialize()` being replaced with calls to the internal `PrintValue()` functions)

Notice that the calls to `Serialize()` though, as the are expected to return a `String`.

### Serialization.h {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_SERIALIZATIONH}

We'll change the serialization functions to return a `String`.
We'll also be adding some more functions to expand the different types supported.

Update the file `code/libraries/baremetal/include/baremetal/Serialization.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Serialization.h
...
42: #include "baremetal/String.h"
43: #include "stdlib/Types.h"
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
62: inline String Serialize(const bool& value)
63: {
64:     return String(value ? "true" : "false");
65: }
66: 
67: String Serialize(char value, int width = 0);
68: String Serialize(int8 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
69: String Serialize(uint8 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
70: String Serialize(int16 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
71: String Serialize(uint16 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
72: String Serialize(int32 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
73: String Serialize(uint32 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
74: String Serialize(int64 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
75: String Serialize(uint64 value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false);
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
86: inline String Serialize(long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
87: {
88:     return Serialize(static_cast<int64>(value), width, base, showBase, leadingZeros);
89: }
90: 
91: /// <summary>
92: /// Serialize unsigned long long int value, type specific specialization
93: /// </summary>
94: /// <param name="value">Value to be serialized</param>
95: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
96: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
97: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
98: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
99: /// <returns>Resulting string</returns>
100: inline String Serialize(unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
101: {
102:     return Serialize(static_cast<uint64>(value), width, base, showBase, leadingZeros);
103: }
104: 
105: String Serialize(float value, int width = 0, int precision = 16);
106: String Serialize(double value, int width = 0, int precision = 16);
107: String Serialize(const String& value, int width = 0, bool quote = false);
108: String Serialize(const char* value, int width = 0, bool quote = false);
109: String Serialize(const void* value, int width = 0);
110: String Serialize(void* value, int width = 0);
111: 
112: } // namespace baremetal
```

- Line 42: We need to include the header for the `String` class, as we will also be implementing some inline functions returning a string
- Line 57-65: We implement the specialization to serialize a boolean as an inline function
- Line 67: We declare the specialization for char
- Line 68-75: We declare the specializations for all integer types. Note that we've added some more to make the list complete
- Line 77-89: We implement the specialization for long long as a inline function.
Notice that as before, we simply re-use the version for int64
- Line 91-103: We implement the specialization for unsigned long long as a inline function.
Notice that as before, we simply re-use the version for uint64
- Line 105-106: We declare the specializations for floating point types. Note that we've added float here as well
- Line 107-108: We declare the specializations for strings. Note that we've added a serialization for `String` here as well as the `const char*` version
- Line 109-110: We declare the specializations for pointers. Any pointer can be simply cast to either `void*` or `const void*`. Pointers will simply be serialized by printing their address as a hexadecimal value

### Serialization.cpp {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_SERIALIZATIONCPP}

We'll implement the new and updated functions.

Update the file `code/libraries/baremetal/src/Serialization.cpp`

```cpp
File: code/libraries/baremetal/src/Serialization.cpp
...
40: #include "baremetal/Serialization.h"
41: 
42: #include "stdlib/Util.h"
43: 
44: /// @file
45: /// Type serialization functions implementation
46: 
47: namespace baremetal {
48: 
49: /// @brief Write characters with base above 10 as uppercase or not
50: static bool Uppercase = true;
51: 
52: static String SerializeInternalInt(int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
53: static String SerializeInternalUInt(uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
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
88: /// Serialize a character value to String.
89: ///
90: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
91: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
92: /// </summary>
93: /// <param name="value">Value to be serialized</param>
94: /// <param name="width">Minimum width in characters, if negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
95: /// <returns>Serialized String value</returns>
96: String Serialize(char value, int width)
97: {
98:     String result;
99: 
100:     int numDigits = 0;
101:     bool negative = (value < 0);
102:     uint64 absVal = static_cast<uint64>(negative ? -value : value);
103:     uint64 divisor = 1;
104:     int absWidth = (width < 0) ? -width : width;
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
132: /// Serialize a 8 bit signed value to String.
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
144: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
145: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
146: /// <returns>Serialized String value</returns>
147: String Serialize(int8 value, int width, int base, bool showBase, bool leadingZeros)
148: {
149:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 8);
150: }
151: 
152: /// <summary>
153: /// Serialize a 8 bit unsigned value to String.
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
165: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
166: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
167: /// <returns>Serialized String value</returns>
168: String Serialize(uint8 value, int width, int base, bool showBase, bool leadingZeros)
169: {
170:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 8);
171: }
172: 
173: /// <summary>
174: /// Serialize a 16 bit signed value to String.
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
186: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
187: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
188: /// <returns>Serialized String value</returns>
189: String Serialize(int16 value, int width, int base, bool showBase, bool leadingZeros)
190: {
191:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 16);
192: }
193: 
194: /// <summary>
195: /// Serialize a 16 bit unsigned value to String.
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
207: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
208: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
209: /// <returns>Serialized String value</returns>
210: String Serialize(uint16 value, int width, int base, bool showBase, bool leadingZeros)
211: {
212:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 16);
213: }
214: 
215: /// <summary>
216: /// Serialize a 32 bit signed value to String.
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
228: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
229: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
230: /// <returns>Serialized String value</returns>
231: String Serialize(int32 value, int width, int base, bool showBase, bool leadingZeros)
232: {
233:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 32);
234: }
235: 
236: /// <summary>
237: /// Serialize a 32 bit unsigned value to String.
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
249: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
250: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
251: /// <returns>Serialized String value</returns>
252: String Serialize(uint32 value, int width, int base, bool showBase, bool leadingZeros)
253: {
254:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 32);
255: }
256: 
257: /// <summary>
258: /// Serialize a 64 bit signed value to String.
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
270: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
271: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
272: /// <returns>Serialized String value</returns>
273: String Serialize(int64 value, int width, int base, bool showBase, bool leadingZeros)
274: {
275:     return SerializeInternalInt(value, width, base, showBase, leadingZeros, 64);
276: }
277: 
278: /// <summary>
279: /// Serialize a 64 bit unsigned value to String.
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
291: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
292: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
293: /// <returns>Serialized String value</returns>
294: String Serialize(uint64 value, int width, int base, bool showBase, bool leadingZeros)
295: {
296:     return SerializeInternalUInt(value, width, base, showBase, leadingZeros, 64);
297: }
298: 
299: /// <summary>
300: /// Serialize a float value to String. The value will be printed as a fixed point number.
301: ///
302: /// Width specifies the minimum width in characters. The value is written right aligned if width is positive, left aligned if width is negative.
303: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
304: ///
305: /// Precision specifies the number of digits behind the decimal pointer
306: /// </summary>
307: /// <param name="value">Value to be serialized</param>
308: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
309: /// <param name="precision">Number of digits after the decimal point to use (limited to 7 decimals</param>
310: /// <returns>Serialized String value</returns>
311: String Serialize(float value, int width, int precision)
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
323:         return String("overflow");
324:     }
325: 
326:     String result;
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
352:         String fractional = Serialize(static_cast<uint64>(value + 0.5F), 0, 10, false, false);
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
364: /// Serialize a double value to String. The value will be printed as a fixed point number.
365: ///
366: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
367: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
368: ///
369: /// Precision specifies the number of digits behind the decimal pointer
370: /// </summary>
371: /// <param name="value">Value to be serialized</param>
372: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
373: /// <param name="precision">Number of digits after the decimal point to use (limited to 10 decimals</param>
374: /// <returns>Serialized String value</returns>
375: String Serialize(double value, int width, int precision)
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
387:         return String("overflow");
388:     }
389: 
390:     String result;
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
416:         String fractional = Serialize(static_cast<uint64>(value + 0.5), 0, 10, false, false);
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
428: /// Serialize a String to String.
429: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
430: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
431: /// If requested, the String is placed between double quotes (").
432: /// </summary>
433: /// <param name="value">Value to be serialized</param>
434: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
435: /// <param name="quote">If true places String between double quotes</param>
436: /// <returns>Serialized String value</returns>
437: String Serialize(const String& value, int width, bool quote)
438: {
439:     return Serialize(value.data(), width, quote);
440: }
441: 
442: /// <summary>
443: /// Serialize a String to String
444: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
445: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
446: /// If requested, the String is placed between double quotes (").
447: /// </summary>
448: /// <param name="value">Value to be serialized</param>
449: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
450: /// <param name="quote">If true places String between double quotes</param>
451: /// <returns>Serialized String value</returns>
452: String Serialize(const char* value, int width, bool quote)
453: {
454:     String result;
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
466: /// Serialize a const void pointer to String
467: ///
468: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
469: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
470: /// </summary>
471: /// <param name="value">Value to be serialized</param>
472: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
473: /// <returns>Serialized String value</returns>
474: String Serialize(const void* value, int width)
475: {
476:     String result;
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
491: /// Serialize a void pointer to String.
492: ///
493: /// Width specifies the minimum width in characters, excluding any base prefix. The value is written right aligned if width is positive, left aligned if width is negative.
494: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
495: /// </summary>
496: /// <param name="value">Value to be serialized</param>
497: /// <param name="width">Minimum width in characters. If negative, aligns to left, if positive, aligns to right. If 0, uses as many characters as needed</param>
498: /// <returns>Serialized String value</returns>
499: String Serialize(void* value, int width)
500: {
501:     return Serialize(const_cast<const void*>(value), width);
502: }
503: 
504: /// <summary>
505: /// Internal serialization function returning String, to be used for all signed values.
506: ///
507: /// Serialize a signed value to String.
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
519: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
520: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
521: /// <param name="numBits">Specifies the number of bits used for the value</param>
522: /// <returns>Serialized stirng</returns>
523: static String SerializeInternalInt(int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
524: {
525:     if ((base < 2) || (base > 36))
526:         return {};
527: 
528:     int numDigits = 0;
529:     bool negative = (value < 0);
530:     uint64 absVal = static_cast<uint64>(negative ? -value : value);
531:     uint64 divisor = 1;
532:     uint64 divisorHigh = 0;
533:     uint64 divisorLast = 1;
534:     size_t absWidth = (width < 0) ? -width : width;
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
557:     String result;
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
607: /// Internal serialization function returning String, to be used for all unsigned values.
608: ///
609: /// Serialize a unsigned value to String.
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
621: /// <param name="showBase">If true, prefix value with base dependent String (0b for base 2, 0 for base 8, 0x for base 16)</param>
622: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
623: /// <param name="numBits">Specifies the number of bits used for the value</param>
624: /// <returns>Serialized stirng</returns>
625: static String SerializeInternalUInt(uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
626: {
627:     if ((base < 2) || (base > 36))
628:         return {};
629: 
630:     int numDigits = 0;
631:     uint64 divisor = 1;
632:     uint64 divisorLast = 1;
633:     uint64 divisorHigh = 0;
634:     size_t absWidth = (width < 0) ? -width : width;
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
653:     String result;
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

- Line 52-43: We change `SerializeInternalUInt()` and `SerializeInternalInt()` to return a `String`
- Line 87-129: We implement the char specialization
- Line 131-150: We implement the int8 specialization
- Line 152-171: We implement the uint8 specialization, which now returns a `String`
- Line 173-192: We implement the int16 specialization
- Line 294-213: We implement the uint16 specialization
- Line 215-234: We implement the int32 specialization
- Line 236-255: We implement the uint32 specialization, which now returns a `String`
- Line 257-276: We implement the int64 specialization, which now returns a `String`
- Line 278-297: We implement the uint64 specialization, which now returns a `String`
- Line 299-361: We implement the float specialization
- Line 363-425: We implement the double specialization, which now returns a `String`
- Line 427-440: We implement the string specialization
- Line 442-463: We implement the const char* specialization
- Line 465-488: We implement the const void* specialization
- Line 490-502: We implement the void* specialization
- Line 504-604: We implement internal `SerializeInternalInt()` function, which now returns a `String`
- Line 606-695: We implement internal `SerializeInternalUInt()` function, which now returns a `String`

### HeapAllocator.cpp {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_HEAPALLOCATORCPP}

As the `HeapAllocator` class implements part of memory management, if we wish to print debug information, we need to do so without using memory allocation.

Update the file `code/libraries/baremetal/src/HeapAllocator.cpp`

```cpp
File: code/libraries/baremetal/src/HeapAllocator.cpp
...
116: void* HeapAllocator::Allocate(size_t size)
117: {
118:     if (m_next == nullptr)
119:     {
120:         return nullptr;
121:     }
122: 
123:     HeapBlockBucket* bucket;
124:     for (bucket = m_buckets; bucket->size > 0; bucket++)
125:     {
126:         if (size <= bucket->size)
127:         {
128:             size = bucket->size;
129: 
130: #if BAREMETAL_MEMORY_TRACING
131:             if (++bucket->count > bucket->maxCount)
132:             {
133:                 bucket->maxCount = bucket->count;
134:             }
135:             ++bucket->totalAllocatedCount;
136:             bucket->totalAllocated += size;
137: 
138: #endif
139:             break;
140:         }
141:     }
142: 
143:     HeapBlockHeader* blockHeader{bucket->freeList};
144:     if ((bucket->size > 0) && (blockHeader != nullptr))
145:     {
146:         assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
147:         bucket->freeList = blockHeader->next;
148: #if BAREMETAL_MEMORY_TRACING_DETAIL
149:         TRACE_NO_ALLOC_DEBUG("Reuse %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
150:         TRACE_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
151: #endif
152:     }
153:     else
154:     {
155:         blockHeader = reinterpret_cast<HeapBlockHeader*>(m_next);
156: 
157:         uint8* nextBlock = m_next;
158:         nextBlock += (sizeof(HeapBlockHeader) + size + HEAP_BLOCK_ALIGN - 1) & ~HEAP_ALIGN_MASK;
159: 
160:         if ((nextBlock <= m_next) || // may have wrapped
161:             (nextBlock > m_limit - m_reserve))
162:         {
163: #if BAREMETAL_MEMORY_TRACING
164:             DumpStatus();
165: #endif
166:             LOG_NO_ALLOC_ERROR("%s: Out of memory", m_heapName);
167:             return nullptr;
168:         }
169: 
170:         m_next = nextBlock;
171: 
172:         blockHeader->magic = HEAP_BLOCK_MAGIC;
173:         blockHeader->size = static_cast<uint32>(size);
174: 
175: #if BAREMETAL_MEMORY_TRACING_DETAIL
176:         TRACE_NO_ALLOC_DEBUG("Allocate %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
177:         TRACE_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
178: #endif
179:     }
180: 
181:     blockHeader->next = nullptr;
182: 
183:     void* result = blockHeader->data;
184:     assert((reinterpret_cast<uintptr>(result) & HEAP_ALIGN_MASK) == 0);
185: 
186:     return result;
187: }
...
234: void HeapAllocator::Free(void* block)
235: {
236:     if (block == nullptr)
237:     {
238:         return;
239:     }
240: 
241:     HeapBlockHeader* blockHeader = reinterpret_cast<HeapBlockHeader*>(reinterpret_cast<uintptr>(block) - sizeof(HeapBlockHeader));
242:     assert(blockHeader->magic == HEAP_BLOCK_MAGIC);
243: 
244:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; bucket++)
245:     {
246:         if (blockHeader->size == bucket->size)
247:         {
248:             blockHeader->next = bucket->freeList;
249:             bucket->freeList = blockHeader;
250: 
251: #if BAREMETAL_MEMORY_TRACING
252:             bucket->count--;
253:             ++bucket->totalFreedCount;
254:             bucket->totalFreed += blockHeader->size;
255: #if BAREMETAL_MEMORY_TRACING_DETAIL
256:             TRACE_NO_ALLOC_DEBUG("Free %lu bytes at %016llx", blockHeader->size, reinterpret_cast<uintptr>(blockHeader->data));
257:             TRACE_NO_ALLOC_DEBUG("Current #allocations = %lu, max #allocations = %lu", bucket->count, bucket->maxCount);
258: #endif
259: #endif
260: 
261:             return;
262:         }
263:     }
264: 
265: #if BAREMETAL_MEMORY_TRACING
266:     LOG_NO_ALLOC_WARNING("%s: Trying to free large block (size %lu)", m_heapName, blockHeader->size);
267: #endif
268: }
269: 
270: #if BAREMETAL_MEMORY_TRACING
271: /// <summary>
272: /// Display the current status of the heap allocator
273: /// </summary>
274: void HeapAllocator::DumpStatus()
275: {
276:     TRACE_NO_ALLOC_DEBUG("Heap allocator info:     %s", m_heapName);
277:     TRACE_NO_ALLOC_DEBUG("Current #allocations:    %llu", GetCurrentAllocatedBlockCount());
278:     TRACE_NO_ALLOC_DEBUG("Max #allocations:        %llu", GetMaxAllocatedBlockCount());
279:     TRACE_NO_ALLOC_DEBUG("Current #allocated bytes:%llu", GetCurrentAllocationSize());
280:     TRACE_NO_ALLOC_DEBUG("Total #allocated blocks: %llu", GetTotalAllocatedBlockCount());
281:     TRACE_NO_ALLOC_DEBUG("Total #allocated bytes:  %llu", GetTotalAllocationSize());
282:     TRACE_NO_ALLOC_DEBUG("Total #freed blocks:     %llu", GetTotalFreedBlockCount());
283:     TRACE_NO_ALLOC_DEBUG("Total #freed bytes:      %llu", GetTotalFreeSize());
284: 
285: #if BAREMETAL_MEMORY_TRACING_DETAIL
286:     for (HeapBlockBucket* bucket = m_buckets; bucket->size > 0; ++bucket)
287:     {
288:         TRACE_NO_ALLOC_DEBUG("malloc(%lu): %lu blocks (max %lu) total alloc #blocks = %llu, #bytes = %llu, total free #blocks = %llu, #bytes = %llu", bucket->size, bucket->count, bucket->maxCount,
289:                              bucket->totalAllocatedCount, bucket->totalAllocated, bucket->totalFreedCount, bucket->totalFreed);
290:     }
291: #endif
292: }
...
```

### String.cpp {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_STRINGCPP}

It is convenient to get some more information if desired on which memory allocations and de-allocations are done by strings, so we'll add some tracing there.

Update the file `code/libraries/baremetal/src/String.cpp`

```cpp
File: code/libraries/baremetal/src/String.cpp
...
79: /// <summary>
80: /// Destructor
81: ///
82: /// Frees any allocated memory.
83: /// </summary>
84: String::~String()
85: {
86: #if BAREMETAL_MEMORY_TRACING_DETAIL
87:     if (m_buffer != nullptr)
88:         LOG_NO_ALLOC_DEBUG("Free string %p", m_buffer);
89: #endif
90:     delete[] m_buffer;
91: }
...
1470: /// <summary>
1471: /// Allocate or re-allocate string to have a capacity of allocationSize bytes
1472: /// </summary>
1473: /// <param name="allocationSize">Amount of bytes to allocate space for</param>
1474: /// <returns>True if successful, false otherwise</returns>
1475: bool String::reallocate_allocation_size(size_t allocationSize)
1476: {
1477:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1478:     if (newBuffer == nullptr)
1479:     {
1480:         return false;
1481:     }
1482:     m_buffer = newBuffer;
1483: #if BAREMETAL_MEMORY_TRACING_DETAIL
1484:     LOG_NO_ALLOC_DEBUG("Alloc string %p", m_buffer);
1485: #endif
1486:     if (m_end == nullptr)
1487:         m_end = m_buffer;
1488:     if (m_end > m_buffer + allocationSize)
1489:         m_end = m_buffer + allocationSize;
1490:     m_allocatedSize = allocationSize;
1491:     return true;
1492: }
```

### Version.cpp {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_VERSIONCPP}

The version string is composed using a call to `Format()`. We need to again use the non-allocating version.

Update the file `code/libraries/baremetal/src/Version.cpp`

```cpp
File: code/libraries/baremetal/src/Version.cpp
40: #include "baremetal/Version.h"
41: 
42: #include "baremetal/Format.h"
43: #include "stdlib/Util.h"
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

### Application code {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_APPLICATION_CODE}

We'll add some tests for check whether the serialization functions work as expected.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include "baremetal/Assert.h"
2: #include "baremetal/Logger.h"
3: #include "baremetal/MemoryManager.h"
4: #include "baremetal/Serialization.h"
5: #include "baremetal/System.h"
6: #include "baremetal/Timer.h"
7: 
8: LOG_MODULE("main");
9: 
10: using namespace baremetal;
11: 
12: int main()
13: {
14:     auto& console = GetConsole();
15:     GetLogger().SetLogLevel(LogSeverity::Debug);
16: 
17:     MemoryManager& memoryManager = GetMemoryManager();
18:     LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
19:     LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
20:     LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));
21: 
22:     memoryManager.DumpStatus();
23: 
24:     char c = 'A';
25:     assert(Serialize(c) == "65");
26:     assert(Serialize(c, 4) == "  65");
27:     int8 i8 = 123;
28:     assert(Serialize(i8) == "123");
29:     assert(Serialize(i8, 8) == "     123");
30:     assert(Serialize(i8, -8) == "123     ");
31:     assert(Serialize(i8, 8, 16) == "      7B");
32:     assert(Serialize(i8, 8, 16, true) == "    0x7B");
33:     assert(Serialize(i8, 8, 16, true, true) == "0x0000007B");
34:     uint8 u8 = 234;
35:     assert(Serialize(u8) == "234");
36:     assert(Serialize(u8, 8) == "     234");
37:     assert(Serialize(u8, -8) == "234     ");
38:     assert(Serialize(u8, 8, 16) == "      EA");
39:     assert(Serialize(u8, 8, 16, true) == "    0xEA");
40:     assert(Serialize(u8, 8, 16, true, true) == "0x000000EA");
41:     int16 i16 = 12345;
42:     assert(Serialize(i16) == "12345");
43:     assert(Serialize(i16, 8) == "   12345");
44:     assert(Serialize(i16, -8) == "12345   ");
45:     assert(Serialize(i16, 8, 16) == "    3039");
46:     assert(Serialize(i16, 8, 16, true) == "  0x3039");
47:     assert(Serialize(i16, 8, 16, true, true) == "0x00003039");
48:     uint16 u16 = 34567;
49:     assert(Serialize(u16) == "34567");
50:     assert(Serialize(u16, 8) == "   34567");
51:     assert(Serialize(u16, -8) == "34567   ");
52:     assert(Serialize(u16, 8, 16) == "    8707");
53:     assert(Serialize(u16, 8, 16, true) == "  0x8707");
54:     assert(Serialize(u16, 8, 16, true, true) == "0x00008707");
55:     int32 i32 = 1234567890l;
56:     assert(Serialize(i32) == "1234567890");
57:     assert(Serialize(i32, 12) == "  1234567890");
58:     assert(Serialize(i32, -12) == "1234567890  ");
59:     assert(Serialize(i32, 12, 16) == "    499602D2");
60:     assert(Serialize(i32, 12, 16, true) == "  0x499602D2");
61:     assert(Serialize(i32, 12, 16, true, true) == "0x0000499602D2");
62:     uint32 u32 = 2345678900ul;
63:     assert(Serialize(u32) == "2345678900");
64:     assert(Serialize(u32, 12) == "  2345678900");
65:     assert(Serialize(u32, -12) == "2345678900  ");
66:     assert(Serialize(u32, 12, 16) == "    8BD03834");
67:     assert(Serialize(u32, 12, 16, true) == "  0x8BD03834");
68:     assert(Serialize(u32, 12, 16, true, true) == "0x00008BD03834");
69:     int64 i64 = 9223372036854775807ll;
70:     assert(Serialize(i64) == "9223372036854775807");
71:     assert(Serialize(i64, 20) == " 9223372036854775807");
72:     assert(Serialize(i64, -20) == "9223372036854775807 ");
73:     assert(Serialize(i64, 20, 16) == "    7FFFFFFFFFFFFFFF");
74:     assert(Serialize(i64, 20, 16, true) == "  0x7FFFFFFFFFFFFFFF");
75:     assert(Serialize(i64, 20, 16, true, true) == "0x00007FFFFFFFFFFFFFFF");
76:     uint64 u64 = 9223372036854775808ull;
77:     assert(Serialize(u64) == "9223372036854775808");
78:     assert(Serialize(u64, 20) == " 9223372036854775808");
79:     assert(Serialize(u64, -20) == "9223372036854775808 ");
80:     assert(Serialize(u64, 20, 16) == "    8000000000000000");
81:     assert(Serialize(u64, 20, 16, true) == "  0x8000000000000000");
82:     assert(Serialize(u64, 20, 16, true, true) == "0x00008000000000000000");
83:     float f = 1.23456789F;
84:     assert(Serialize(f) == "1.2345679");
85:     assert(Serialize(f, 12) == "   1.2345679");
86:     assert(Serialize(f, -12) == "1.2345679   ");
87:     assert(Serialize(f, 12, 2) == "        1.23");
88:     assert(Serialize(f, 12, 7) == "   1.2345679");
89:     assert(Serialize(f, 12, 8) == "   1.2345679");
90: 
91:     double d = 1.234567890123456;
92:     assert(Serialize(d) == "1.23456789012346");
93:     assert(Serialize(d, 18) == "  1.23456789012346");
94:     assert(Serialize(d, -18) == "1.23456789012346  ");
95:     assert(Serialize(d, 18, 5) == "           1.23457");
96:     assert(Serialize(d, 18, 7) == "         1.2345679");
97:     assert(Serialize(d, 18, 12) == "    1.234567890123");
98: 
99:     String s("hello world");
100:     assert(Serialize(s) == "hello world");
101:     assert(Serialize(s, 15) == "    hello world");
102:     assert(Serialize(s, -15) == "hello world    ");
103:     assert(Serialize(s, 15, true) == "  \"hello world\"");
104: 
105:     const char* str = "hello world";
106:     assert(Serialize(str) == "hello world");
107:     assert(Serialize(str, 15) == "    hello world");
108:     assert(Serialize(str, -15) == "hello world    ");
109:     assert(Serialize(str, 15, true) == "  \"hello world\"");
110: 
111:     const void* pvc = reinterpret_cast<const void*>(0x0123456789ABCDEF);
112:     assert(Serialize(pvc) == "0x0123456789ABCDEF");
113:     assert(Serialize(pvc, 20) == "  0x0123456789ABCDEF");
114:     assert(Serialize(pvc, -20) == "0x0123456789ABCDEF  ");
115: 
116:     void* pv = reinterpret_cast<void*>(0x0123456789ABCDEF);
117:     assert(Serialize(pv) == "0x0123456789ABCDEF");
118:     assert(Serialize(pv, 20) == "  0x0123456789ABCDEF");
119:     assert(Serialize(pv, -20) == "0x0123456789ABCDEF  ");
120: 
121:     LOG_INFO("Wait 5 seconds");
122:     Timer::WaitMilliSeconds(5000);
123: 
124:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
125:     char ch{};
126:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
127:     {
128:         ch = console.ReadChar();
129:         console.WriteChar(ch);
130:     }
131:     if (ch == 'p')
132:         assert(false);
133: 
134:     LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
135:     LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
136:     LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));
137: 
138:     memoryManager.DumpStatus();
139: 
140:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
141: }
```

### Temporarily displaying detailed memory tracing {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_TEMPORARILY_DISPLAYING_DETAILED_MEMORY_TRACING}

Let's for a minute switch on the detailed debug tracing for memory allocation again. We'll switch it off after running the application.

```cmake
File: CMakeLists.txt
...
69: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" ON)
70: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" ON)
...
```

You'll need to clear the CMake cache and re-configure

### Configuring, building and debugging {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

You will see an overview of memory management before and after `main()` was run, as we still have `BAREMETAL_MEMORY_TRACING` defined.

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:92)
Info   Starting up (System:204)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900480
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 3
Debug  Alloc string 900480 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 2, max #allocations = 3
Debug  Alloc string 900040 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 00000000009008C0
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 3, max #allocations = 3
Debug  Alloc string 9008C0 (String:1484)
Debug  Free string 900040 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900040
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 2, max #allocations = 3
Debug  Free string 9008C0 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 00000000009008C0
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 1, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 64 bytes at 0000000000900D00
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 1
Debug  Alloc string 900D00 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 00000000009008C0
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 2, max #allocations = 3
Debug  Alloc string 9008C0 (String:1484)
Debug  Free string 900D00 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 64 bytes at 0000000000900D00
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Free string 9008C0 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 00000000009008C0
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 1, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 00000000009008C0
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 2, max #allocations = 3
Debug  Alloc string 9008C0 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 64 bytes at 0000000000900D00
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 1
Debug  Alloc string 900D00 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 3, max #allocations = 3
Debug  Alloc string 900040 (String:1484)
Debug  Free string 900D00 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 64 bytes at 0000000000900D00
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Free string 900040 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900040
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 2, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 3, max #allocations = 3
Debug  Alloc string 900040 (String:1484)
Info   Heap space available: 967832256 bytes (main:18)
Debug  Free string 9008C0 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 00000000009008C0
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 2, max #allocations = 3
Debug  Free string 900480 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900480
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 1, max #allocations = 3
Debug  Free string 900040 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900040
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 3
Debug  Alloc string 900040 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900480
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 2, max #allocations = 3
Debug  Alloc string 900480 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 00000000009008C0
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 3, max #allocations = 3
Debug  Alloc string 9008C0 (String:1484)
Debug  Free string 900480 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900480
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 2, max #allocations = 3
Debug  Free string 9008C0 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 00000000009008C0
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 1, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 64 bytes at 0000000000900D00
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 1
Debug  Alloc string 900D00 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 00000000009008C0
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 2, max #allocations = 3
Debug  Alloc string 9008C0 (String:1484)
Debug  Free string 900D00 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 64 bytes at 0000000000900D00
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Free string 9008C0 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 00000000009008C0
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 1, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 00000000009008C0
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 2, max #allocations = 3
Debug  Alloc string 9008C0 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 64 bytes at 0000000000900D00
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 1
Debug  Alloc string 900D00 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900480
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 3, max #allocations = 3
Debug  Alloc string 900480 (String:1484)
Debug  Free string 900D00 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 64 bytes at 0000000000900D00
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Free string 900480 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900480
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 2, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900480
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 3, max #allocations = 3
Debug  Alloc string 900480 (String:1484)
Info   High heap space available: 2147483648 bytes (main:19)
Debug  Free string 9008C0 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 00000000009008C0
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 2, max #allocations = 3
Debug  Free string 900040 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900040
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 1, max #allocations = 3
Debug  Free string 900480 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900480
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900480
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 3
Debug  Alloc string 900480 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 2, max #allocations = 3
Debug  Alloc string 900040 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 00000000009008C0
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 3, max #allocations = 3
Debug  Alloc string 9008C0 (String:1484)
Debug  Free string 900040 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900040
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 2, max #allocations = 3
Debug  Free string 9008C0 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 00000000009008C0
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 1, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 64 bytes at 0000000000900D00
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 1
Debug  Alloc string 900D00 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 00000000009008C0
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 2, max #allocations = 3
Debug  Alloc string 9008C0 (String:1484)
Debug  Free string 900D00 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 64 bytes at 0000000000900D00
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Free string 9008C0 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 00000000009008C0
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 1, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 00000000009008C0
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 2, max #allocations = 3
Debug  Alloc string 9008C0 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 64 bytes at 0000000000900D00
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 1, max #allocations = 1
Debug  Alloc string 900D00 (String:1484)
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 3, max #allocations = 3
Debug  Alloc string 900040 (String:1484)
Debug  Free string 900D00 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 64 bytes at 0000000000900D00
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 0, max #allocations = 1
Debug  Free string 900040 (String:88)
Debug  Free (HeapAllocator.cpp:256) Free 1024 bytes at 0000000000900040
Debug  Free (HeapAllocator.cpp:257) Current #allocations = 2, max #allocations = 3
Debug  Allocate (HeapAllocator.cpp:149) Reuse 1024 bytes at 0000000000900040
Debug  Allocate (HeapAllocator.cpp:150) Current #allocations = 3, max #allocations = 3
Debug  Alloc string 900040 (String:1484)
Info   DMA heap space available: 3115315904 bytes (main:20)
...
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        5
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 377
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  292928
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     377
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      292928
Debug  DumpStatus (HeapAllocator.cpp:288) malloc(64): 0 blocks (max 1) total alloc #blocks = 97, #bytes = 6208, total free #blocks = 97, #bytes = 6208
Debug  DumpStatus (HeapAllocator.cpp:288) malloc(1024): 0 blocks (max 4) total alloc #blocks = 280, #bytes = 286720, total free #blocks = 280, #bytes = 286720
Debug  DumpStatus (HeapAllocator.cpp:288) malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:288) malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:288) malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:288) malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
Debug  DumpStatus (HeapAllocator.cpp:288) malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0
...
```

This will display a lot (!) of debug information.
As you can see we performeed a total of 377 allocations, all blocks were freed again, 97 were were 64 bytes max in size, 280 were 1024 bytes in size.
At any moment, there were a maximum of 5 blocks allocated.

### Switch off memory tracing {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_SWITCH_OFF_MEMORY_TRACING}

Let's now completely switch off memory tracing.

```cmake
File: CMakeLists.txt
...
69: option(BAREMETAL_TRACE_MEMORY "Enable memory tracing output" OFF)
70: option(BAREMETAL_TRACE_MEMORY_DETAIL "Enable detailed memory tracing output" OFF)
...
```

Again you'll need to clear the CMake cache and re-configure

### Running again without memory tracing {#TUTORIAL_17_SERIALIZATION_AND_FORMATTING_MEMORY_ALLOCATING_AND_NON_MEMORY_ALLOCATING_PRINTING___STEP_1_RUNNING_AGAIN_WITHOUT_MEMORY_TRACING}

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:92)
Info   Starting up (System:204)
Info   Heap space available: 967832256 bytes (main:18)
Info   High heap space available: 2147483648 bytes (main:19)
Info   DMA heap space available: 3115315904 bytes (main:20)
Info   Wait 5 seconds (main:121)
Press r to reboot, h to halt, p to fail assertion and panic
rInfo   Heap space available: 967831168 bytes (main:134)
Info   High heap space available: 2147483648 bytes (main:135)
Info   DMA heap space available: 3115314816 bytes (main:136)
Info   Reboot (System:136)
```

Next: [18-exceptions](18-exceptions.md)
