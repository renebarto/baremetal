# Tutorial 13: Logger {#TUTORIAL_13_LOGGER}

@tableofcontents

## New tutorial setup {#TUTORIAL_13_LOGGER_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/12-logger`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_13_LOGGER_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-13.a`
- a library `output/Debug/lib/stdlib-13.a`
- an application `output/Debug/bin/13-logger.elf`
- an image in `deploy/Debug/13-logger-image`

## Adding a logger {#TUTORIAL_13_LOGGER_ADDING_A_LOGGER}

We can now write to the console.
It would be nice if we could write to the console in a more generic way, using a timestamp, a log level, a variable number of arguments, and preferably in color if supported.
A log statement might look like this:

```cpp
LOG_INFO("Serial:              %016llx", machineInfo.GetSerial());
```

Also, we might want tracing, which also shows the sourcefile and function:

```
TRACE_INFO("Return code %d", result);
```

For this, we will need to be able to set a default logger device, by using the console with an instance of a `CharDevice`, and then enable printing to the console, much like `printf()`.
Later on, we can add the screen or maybe a file as a device for logging as well.
In order to enable writing variable argument lists, we will need to use strings, which grow automatically as needed.
This then means we need to be able to allocate heap memory.
Also, we will need a way to retrieve the current time.

So, all in all, quite some work to do.

## Printing a formatted string - Step 1 {#TUTORIAL_13_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_1}

In order to introduce the actual logging functionality, we would like to be able to print using variable arguments,
much like the standard C `printf()` function.

We need to be able to handle variable arguments these. Normally we would have functions or definitions such as `va_start`, `va_end` and `va_arg` for this, by including `stdarg.h`.
Luckily, GCC offers these as builtin functions.
We will then need to implement formatting of strings to a buffer, using format strings like `printf()` uses.
We'll also want to support printing version information, so we'll provide for a way to pass on the version in the build, and make a string out of it.
Finally we can then add the logger class.

### StdArg.h {#TUTORIAL_13_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_1_STDARGH}

So let's add a header to act as the standard C library's `stdarg.h` would.

Create the file `code/libraries/stdlib/include/stdlib/StdArg.h`

```cpp
File: code/libraries/stdlib/include/stdlib/StdArg.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : StdArg.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Variable arguments handling
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
43: /// Standard variable argument list handling using builtin functionality in GCC
44:
45: // prevent warning, if <stdarg.h> from toolchain is included too
46: #ifndef _STDARG_H
47:
48: /// @brief declare standard va_list type
49: typedef __builtin_va_list va_list;
50:
51: /// @brief define standard va_start macro
52: #define va_start(arg, last) __builtin_va_start(arg, last)
53: /// @brief define standard va_end macro
54: #define va_end(arg)         __builtin_va_end(arg)
55: /// @brief define standard va_arg macro
56: #define va_arg(arg, type)   __builtin_va_arg(arg, type)
57:
58: #endif
```

- Line 46: We protect against duplicate definitions in case we use the standard C version `stdarg.h`
- Line 48-49: We define the type `va_list` as a built-in version
- Line 51-52: We define `va_start` as a built-in version
- Line 53-54: We define `va_end` as a built-in version
- Line 55-56: We define `va_arg` as a built-in version

### Format.h {#TUTORIAL_13_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_1_FORMATH}

We'll define two variants of a formatting function for printing to a string with variable arguments.
One using the ellipsis argument `...`, the other with a `va_list` argument.

Create the file `code/libraries/baremetal/include/baremetal/Format.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Format.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Format.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : String formatting using standard argument handling
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
43: /// Formatting functionality
44:
45: #include "stdlib/StdArg.h"
46: #include "stdlib/Types.h"
47:
48: namespace baremetal {
49:
50: void FormatV(char* buffer, size_t bufferSize, const char* format, va_list args);
51: void Format(char* buffer, size_t bufferSize, const char* format, ...);
52:
53: } // namespace baremetal
```

### Format.cpp {#TUTORIAL_13_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_1_FORMATCPP}

Now we will implement the formatting functions.

Be aware that this is a poor man's implementation, but it will do for now.

Create the file `code/libraries/baremetal/src/Format.cpp`

```cpp
File: code/libraries/baremetal/src/Format.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Format.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : String formatting using standard argument handling
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
40: #include "baremetal/Format.h"
41:
42: #include "baremetal/Serialization.h"
43: #include "stdlib/Util.h"
44:
45: namespace baremetal {
46:
47: /// @brief Size of buffer for destination
48: const size_t BufferSize = 4096;
49:
50: /// <summary>
51: /// Append a character to the buffer
52: /// </summary>
53: /// <param name="buffer">Buffer to write to</param>
54: /// <param name="bufferSize">Size of the buffer</param>
55: /// <param name="c">Character to append</param>
56: static void Append(char* buffer, size_t bufferSize, char c)
57: {
58:     size_t len = strlen(buffer);
59:     char* p = buffer + len;
60:     if (static_cast<size_t>(p - buffer) < bufferSize)
61:     {
62:         *p++ = c;
63:     }
64:     if (static_cast<size_t>(p - buffer) < bufferSize)
65:     {
66:         *p = '\0';
67:     }
68: }
69:
70: /// <summary>
71: /// Append a set of identical characters to the buffer
72: /// </summary>
73: /// <param name="buffer">Buffer to write to</param>
74: /// <param name="bufferSize">Size of the buffer</param>
75: /// <param name="count">Number of characters to append</param>
76: /// <param name="c">Character to append</param>
77: static void Append(char* buffer, size_t bufferSize, size_t count, char c)
78: {
79:     size_t len = strlen(buffer);
80:     char* p = buffer + len;
81:     while ((count > 0) && (static_cast<size_t>(p - buffer) < bufferSize))
82:     {
83:         *p++ = c;
84:         --count;
85:     }
86:     if (static_cast<size_t>(p - buffer) < bufferSize)
87:     {
88:         *p = '\0';
89:     }
90: }
91:
92: /// <summary>
93: /// Append a string to the buffer
94: /// </summary>
95: /// <param name="buffer">Buffer to write to</param>
96: /// <param name="bufferSize">Size of the buffer</param>
97: /// <param name="str">String to append</param>
98: static void Append(char* buffer, size_t bufferSize, const char* str)
99: {
100:     strncat(buffer, str, bufferSize);
101: }
102:
103: /// <summary>
104: /// Print a formatted string to a buffer
105: ///
106: /// This uses variable arguments
107: /// </summary>
108: /// <param name="buffer">Pointer to destination buffer</param>
109: /// <param name="bufferSize">Size of the buffer</param>
110: /// <param name="format">Format string</param>
111: void Format(char* buffer, size_t bufferSize, const char* format, ...)
112: {
113:     va_list var;
114:     va_start(var, format);
115:
116:     FormatV(buffer, bufferSize, format, var);
117:
118:     va_end(var);
119: }
120:
121: /// <summary>
122: /// Print a formatted string to a buffer
123: ///
124: /// This uses a variable argument list
125: /// </summary>
126: /// <param name="buffer">Pointer to destination buffer</param>
127: /// <param name="bufferSize">Size of the buffer</param>
128: /// <param name="format">Format string</param>
129: /// <param name="args">Variable arguments list</param>
130: void FormatV(char* buffer, size_t bufferSize, const char* format, va_list args)
131: {
132:     if (buffer == nullptr)
133:         return;
134:     buffer[0] = '\0';
135:
136:     while (*format != '\0')
137:     {
138:         if (*format == '%')
139:         {
140:             if (*++format == '%')
141:             {
142:                 Append(buffer, bufferSize, '%');
143:                 format++;
144:                 continue;
145:             }
146:
147:             bool alternate = false;
148:             if (*format == '#')
149:             {
150:                 alternate = true;
151:                 format++;
152:             }
153:
154:             bool left = false;
155:             if (*format == '-')
156:             {
157:                 left = true;
158:                 format++;
159:             }
160:
161:             bool leadingZero = false;
162:             if (*format == '0')
163:             {
164:                 leadingZero = true;
165:                 format++;
166:             }
167:
168:             size_t width = 0;
169:             while (('0' <= *format) && (*format <= '9'))
170:             {
171:                 width = width * 10 + (*format - '0');
172:                 format++;
173:             }
174:
175:             unsigned precision = 6;
176:             if (*format == '.')
177:             {
178:                 format++;
179:                 precision = 0;
180:                 while ('0' <= *format && *format <= '9')
181:                 {
182:                     precision = precision * 10 + (*format - '0');
183:
184:                     format++;
185:                 }
186:             }
187:
188:             bool haveLong{};
189:             bool haveLongLong{};
190:
191:             if (*format == 'l')
192:             {
193:                 if (*(format + 1) == 'l')
194:                 {
195:                     haveLongLong = true;
196:
197:                     format++;
198:                 }
199:                 else
200:                 {
201:                     haveLong = true;
202:                 }
203:
204:                 format++;
205:             }
206:
207:             switch (*format)
208:             {
209:             case 'c':
210:                 {
211:                     char ch = static_cast<char>(va_arg(args, int));
212:                     if (left)
213:                     {
214:                         Append(buffer, bufferSize, ch);
215:                         if (width > 1)
216:                         {
217:                             Append(buffer, bufferSize, width - 1, ' ');
218:                         }
219:                     }
220:                     else
221:                     {
222:                         if (width > 1)
223:                         {
224:                             Append(buffer, bufferSize, width - 1, ' ');
225:                         }
226:                         Append(buffer, bufferSize, ch);
227:                     }
228:                 }
229:                 break;
230:
231:             case 'd':
232:             case 'i':
233:                 if (haveLongLong)
234:                 {
235:                     char str[BufferSize]{};
236:                     Serialize(str, BufferSize, va_arg(args, int64), left ? -width : width, 10, false, leadingZero);
237:                     Append(buffer, bufferSize, str);
238:                 }
239:                 else if (haveLong)
240:                 {
241:                     char str[BufferSize]{};
242:                     Serialize(str, BufferSize, va_arg(args, int32), left ? -width : width, 10, false, leadingZero);
243:                     Append(buffer, bufferSize, str);
244:                 }
245:                 else
246:                 {
247:                     char str[BufferSize]{};
248:                     Serialize(str, BufferSize, va_arg(args, int), left ? -width : width, 10, false, leadingZero);
249:                     Append(buffer, bufferSize, str);
250:                 }
251:                 break;
252:
253:             case 'f':
254:                 {
255:                     char str[BufferSize]{};
256:                     Serialize(str, BufferSize, va_arg(args, double), left ? -width : width, precision);
257:                     Append(buffer, bufferSize, str);
258:                 }
259:                 break;
260:
261:             case 'b':
262:                 if (alternate)
263:                 {
264:                     Append(buffer, bufferSize, "0b");
265:                 }
266:                 if (haveLongLong)
267:                 {
268:                     char str[BufferSize]{};
269:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 2, false, leadingZero);
270:                     Append(buffer, bufferSize, str);
271:                 }
272:                 else if (haveLong)
273:                 {
274:                     char str[BufferSize]{};
275:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 2, false, leadingZero);
276:                     Append(buffer, bufferSize, str);
277:                 }
278:                 else
279:                 {
280:                     char str[BufferSize]{};
281:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 2, false, leadingZero);
282:                     Append(buffer, bufferSize, str);
283:                 }
284:                 break;
285:
286:             case 'o':
287:                 if (alternate)
288:                 {
289:                     Append(buffer, bufferSize, '0');
290:                 }
291:                 if (haveLongLong)
292:                 {
293:                     char str[BufferSize]{};
294:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 8, false, leadingZero);
295:                     Append(buffer, bufferSize, str);
296:                 }
297:                 else if (haveLong)
298:                 {
299:                     char str[BufferSize]{};
300:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 8, false, leadingZero);
301:                     Append(buffer, bufferSize, str);
302:                 }
303:                 else
304:                 {
305:                     char str[BufferSize]{};
306:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 8, false, leadingZero);
307:                     Append(buffer, bufferSize, str);
308:                 }
309:                 break;
310:
311:             case 's':
312:                 {
313:                     char str[BufferSize]{};
314:                     Serialize(str, BufferSize, va_arg(args, const char*), left ? -width : width, false);
315:                     Append(buffer, bufferSize, str);
316:                 }
317:                 break;
318:
319:             case 'u':
320:                 if (haveLongLong)
321:                 {
322:                     char str[BufferSize]{};
323:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 10, false, leadingZero);
324:                     Append(buffer, bufferSize, str);
325:                 }
326:                 else if (haveLong)
327:                 {
328:                     char str[BufferSize]{};
329:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 10, false, leadingZero);
330:                     Append(buffer, bufferSize, str);
331:                 }
332:                 else
333:                 {
334:                     char str[BufferSize]{};
335:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 10, false, leadingZero);
336:                     Append(buffer, bufferSize, str);
337:                 }
338:                 break;
339:
340:             case 'x':
341:             case 'X':
342:                 if (alternate)
343:                 {
344:                     Append(buffer, bufferSize, "0x");
345:                 }
346:                 if (haveLongLong)
347:                 {
348:                     char str[BufferSize]{};
349:                     Serialize(str, BufferSize, va_arg(args, uint64), left ? -width : width, 16, false, leadingZero);
350:                     Append(buffer, bufferSize, str);
351:                 }
352:                 else if (haveLong)
353:                 {
354:                     char str[BufferSize]{};
355:                     Serialize(str, BufferSize, va_arg(args, uint32), left ? -width : width, 16, false, leadingZero);
356:                     Append(buffer, bufferSize, str);
357:                 }
358:                 else
359:                 {
360:                     char str[BufferSize]{};
361:                     Serialize(str, BufferSize, va_arg(args, unsigned), left ? -width : width, 16, false, leadingZero);
362:                     Append(buffer, bufferSize, str);
363:                 }
364:                 break;
365:
366:             case 'p':
367:                 if (alternate)
368:                 {
369:                     Append(buffer, bufferSize, "0x");
370:                 }
371:                 {
372:                     char str[BufferSize]{};
373:                     Serialize(str, BufferSize, va_arg(args, unsigned long long), left ? -width : width, 16, false, leadingZero);
374:                     Append(buffer, bufferSize, str);
375:                 }
376:                 break;
377:
378:             default:
379:                 Append(buffer, bufferSize, '%');
380:                 Append(buffer, bufferSize, *format);
381:                 break;
382:             }
383:         }
384:         else
385:         {
386:             Append(buffer, bufferSize, *format);
387:         }
388:
389:         format++;
390:     }
391: }
392:
393: } // namespace baremetal
```

- Line 47-48: We define a buffer size, to define the buffer, but also to check against writing outside the buffer.
- Line 50-68: We define the local function `Append()` for writing a single character to the buffer, checked for overflow
- Line 70-90: We define the local function `Append()` for writing multiple instances of a single character to the buffer, checked for overflow
- Line 92-101: We define the local function `Append()` for writing a string to the buffer, checked for overflow.
This uses a standard C function strncat, which we will need to define
- Line 103-119: We implement the `Format()` function, using the ellipsis operator.
This simple creates a `va_list` from the arguments, and calls the other version of `Format()`.
- Line 121-391: Implements the `Format()` function, using the `va_list` argument
  - Line 132-133: We check that the buffer pointer is not null, otherwise we return
  - Line 134: We make sure to have an empty buffer
  - Line 136-387: We scan through the format string
    - Line 138-383: If the format character is `%` this is a special format operator
      - Line 140-145: In case the format string holds `%%` we see this as verbatim `%`, so we add that character
      - Line 147-152: If the next character is `#` we see this as an alternative version, signalling to add the prefix (only for base 2, 8, 16 integers), and advance
      - Line 154-159: If the next character is `-` we left-align the value, and advance
      - Line 161-166: If the next character is `0` we use leading zeros to fill up the value to its normal length, and advance
      - Line 168-173: If more digits follow, e.g. `%12` or `%012`, we extract the width of the value to be printed from these digits, and advance
      - Line 175-186: If a decimal point follows, we expect this to be a floating point value, and expect the next digits to specify the length of the fraction.
We read all digits, calculate the fraction length, and advance.
In case there is no decimal point, we set the default fraction length to 6
      - Line 188-205: If the next character is `l` this is seen as a specification for a `long` (32 bit) value (only for integer values).
If another `l` follows, we see this as a `long long` (64 bit) value
      - Line 210-228: If the format character is a `c` we print the value as a character, taking into account the alignment
      - Line 233-250: If the format character is a `d` or `i` we print the value as a signed integer, taking into account the `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros
      - Line 254-258: If the format character is a `f` we print the value as a double, taking into account the precision if set, as well as the width, alignment and whether to use leading zeros.
Note that this is different from normal `printf()` behaviour, where %f means float, and %lf means double
      - Line 262-283: If the format character is a `b` we print the value as a binary unsigned integer, taking into account the `#`, `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros.
Note that this is an addition to `printf()` behaviour
      - Line 286-308: If the format character is a `o` we print the value as a octal unsigned integer, taking into account the `#`, `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros.
Note that this is an addition to `printf()` behaviour
      - Line 312-316: If the format character is a `s` we print the value as a string, taking into account the alignment and width
      - Line 319-337: If the format character is a `u` we print the value as a decimal unsigned integer, taking into account the `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros
      - Line 342-363: If the format character is a `x` or `X` we print the value as a hexadecimal unsigned integer, taking into account the `#`, `l` or `ll` prefix, as well as the width, alignment and whether to use leading zeros
      - Line 367-375: If the format character is a `p` we print the value as a pointer, meaning it is printed as a 64 bit unsigned integer in hexadecimal, taking into account the `#`, as well as the width, alignment and whether to use leading zeros
      - Line 379-380: Otherwise we simple print `%` and the character
      - Line 385-387: If the format character is not `%` we simply add the character

### Serialization.h {#TUTORIAL_13_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_1_SERIALIZATIONH}

As we wish to convert more types to string due to the variable arguments, we need to extend the set of serialization functions.
These are also used by the `Format()` functions.
We'll be adding functions for `uint8`, `int32`, `int64`, `long long`, `unsigned long long`, `const char*` and `double`.

Update the file `code/libraries/baremetal/include/baremetal/Serialization.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Serialization.h
47: namespace baremetal {
48:
49: void Serialize(char* buffer, size_t bufferSize, uint8 value, int width, int base, bool showBase, bool leadingZeros);
50: void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros);
51: void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros);
52:
53: void Serialize(char* buffer, size_t bufferSize, int32 value, int width, int base, bool showBase, bool leadingZeros);
54: void Serialize(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros);
55: /// <summary>
56: /// Serialize long long int value, type specific specialization
57: /// </summary>
58: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
59: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
60: /// <param name="value">Value to be serialized</param>
61: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
62: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
63: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
64: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
65: inline void Serialize(char* buffer, size_t bufferSize, long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
66: {
67:     Serialize(buffer, bufferSize, static_cast<int64>(value), width, base, showBase, leadingZeros);
68: }
69: /// <summary>
70: /// Serialize unsigned long long int value, type specific specialization
71: /// </summary>
72: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
73: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
74: /// <param name="value">Value to be serialized</param>
75: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
76: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
77: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
78: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
79: inline void Serialize(char* buffer, size_t bufferSize, unsigned long long value, int width = 0, int base = 10, bool showBase = false, bool leadingZeros = false)
80: {
81:     Serialize(buffer, bufferSize, static_cast<uint64>(value), width, base, showBase, leadingZeros);
82: }
83: void Serialize(char* buffer, size_t bufferSize, const char* value, int width, bool quote);
84: void Serialize(char* buffer, size_t bufferSize, double value, int width, int precision);
85:
86: } // namespace baremetal
```

- Line 49: We declare a serialization function for uint8 (unsigned 8 bit integer)
- Line 53: We declare a serialization function for int32 (signed 32 bit integer)
- Line 54: We declare a serialization function for int64 (signed 64 bit integer)
- Line 65-68: We declare a serialization function for long long (this is equal to int64, but the compiler sees it as a different type)
- Line 79-82: We declare a serialization function for unsigned long long (this is equal to uint64, but the compiler sees it as a different type)
- Line 83: We declare a serialization function for const char* (for strings)
- Line 84: We declare a serialization function for double

#### Serialization.cpp {#TUTORIAL_13_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_1_SERIALIZATIONH_SERIALIZATIONCPP}

Let's implement the new functions.

Update the file `code/libraries/baremetal/src/Serialization.cpp`

```cpp
File: code/libraries/baremetal/src/Serialization.cpp
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
52: static void SerializeInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
53: static void SerializeInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits);
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
88: /// Serialize a 8 bit unsigned value to buffer.
89: ///
90: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
91: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
92: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
93: ///
94: /// Base is the digit base, which can range from 2 to 36.
95: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
96: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
97: /// </summary>
98: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
99: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
100: /// <param name="value">Value to be serialized</param>
101: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
102: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
103: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
104: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
105: void Serialize(char* buffer, size_t bufferSize, uint8 value, int width, int base, bool showBase, bool leadingZeros)
106: {
107:     SerializeInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 8);
108: }
109:
110: /// <summary>
111: /// Serialize a 32 bit signed value to buffer.
112: ///
113: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
114: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
115: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
116: ///
117: /// Base is the digit base, which can range from 2 to 36.
118: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
119: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
120: /// </summary>
121: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
122: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
123: /// <param name="value">Value to be serialized</param>
124: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
125: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
126: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
127: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
128: void Serialize(char* buffer, size_t bufferSize, int32 value, int width, int base, bool showBase, bool leadingZeros)
129: {
130:     SerializeInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
131: }
132:
133: /// <summary>
134: /// Serialize a 32 bit unsigned value to buffer.
135: ///
136: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
137: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
138: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
139: ///
140: /// Base is the digit base, which can range from 2 to 36.
141: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
142: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
143: /// </summary>
144: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
145: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
146: /// <param name="value">Value to be serialized</param>
147: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
148: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
149: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
150: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
151: void Serialize(char* buffer, size_t bufferSize, uint32 value, int width, int base, bool showBase, bool leadingZeros)
152: {
153:     SerializeInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 32);
154: }
155:
156: /// <summary>
157: /// Serialize a 64 bit signed value to buffer.
158: ///
159: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
160: /// Width specifies the minimum width in characters, excluding any base prefix. The value is always written right aligned.
161: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
162: ///
163: /// Base is the digit base, which can range from 2 to 36.
164: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
165: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
166: /// </summary>
167: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
168: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
169: /// <param name="value">Value to be serialized</param>
170: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
171: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
172: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
173: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
174: void Serialize(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros)
175: {
176:     SerializeInternalInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
177: }
178:
179: /// <summary>
180: /// Serialize a 64 bit unsigned value to buffer.
181: ///
182: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
183: /// Width specifies the minimum width in characters. The value is always written right aligned.
184: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
185: ///
186: /// Base is the digit base, which can range from 2 to 36.
187: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
188: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
189: /// </summary>
190: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
191: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
192: /// <param name="value">Value to be serialized</param>
193: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
194: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
195: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
196: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
197: void Serialize(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros)
198: {
199:     SerializeInternalUInt(buffer, bufferSize, value, width, base, showBase, leadingZeros, 64);
200: }
201:
202: /// <summary>
203: /// Serialize a double value to buffer. The value will be printed as a fixed point number.
204: ///
205: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, the string is terminated to hold maximum bufferSize - 1 characters.
206: /// Width is currently unused.
207: /// Precision specifies the number of digits behind the decimal pointer
208: /// </summary>
209: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
210: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
211: /// <param name="value">Value to be serialized</param>
212: /// <param name="width">Unused</param>
213: /// <param name="precision">Number of digits after the decimal point to use</param>
214: void Serialize(char* buffer, size_t bufferSize, double value, int width, int precision)
215: {
216:     bool negative{};
217:     if (value < 0)
218:     {
219:         negative = true;
220:         value = -value;
221:     }
222:
223:     if (bufferSize == 0)
224:         return;
225:
226:     // We can only print values with integral parts up to what uint64 can hold
227:     if (value > static_cast<double>(static_cast<uint64>(-1)))
228:     {
229:         strncpy(buffer, "overflow", bufferSize);
230:         return;
231:     }
232:
233:     *buffer = '\0';
234:     if (negative)
235:         strncpy(buffer, "-", bufferSize);
236:
237:     uint64 integralPart = static_cast<uint64>(value);
238:     const size_t TmpBufferSize = 32;
239:     char tmpBuffer[TmpBufferSize];
240:     Serialize(tmpBuffer, TmpBufferSize, integralPart, 0, 10, false, false);
241:     strncat(buffer, tmpBuffer, bufferSize);
242:     const int MaxPrecision = 7;
243:
244:     if (precision != 0)
245:     {
246:         strncat(buffer, ".", bufferSize);
247:
248:         if (precision > MaxPrecision)
249:         {
250:             precision = MaxPrecision;
251:         }
252:
253:         uint64 precisionPower10 = 1;
254:         for (int i = 1; i <= precision; i++)
255:         {
256:             precisionPower10 *= 10;
257:         }
258:
259:         value -= static_cast<double>(integralPart);
260:         value *= static_cast<double>(precisionPower10);
261:
262:         Serialize(tmpBuffer, TmpBufferSize, static_cast<uint64>(value), 0, 10, false, false);
263:         strncat(buffer, tmpBuffer, bufferSize);
264:         precision -= strlen(tmpBuffer);
265:         while (precision--)
266:         {
267:             strncat(buffer, "0", bufferSize);
268:         }
269:     }
270: }
271:
272: /// <summary>
273: /// Serialize a const char * value to buffer. The value can be quoted.
274: ///
275: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
276: /// If quote is true, the string is printed within double quotes (\")
277: /// </summary>
278: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
279: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
280: /// <param name="value">String to be serialized</param>
281: /// <param name="width">Unused</param>
282: /// <param name="quote">If true, value is printed between double quotes, if false, no quotes are used</param>
283: void Serialize(char* buffer, size_t bufferSize, const char* value, int width, bool quote)
284: {
285:     size_t numChars = strlen(value);
286:     if (quote)
287:         numChars += 2;
288:
289:     // Leave one character for \0
290:     if (numChars > bufferSize - 1)
291:         return;
292:
293:     char* bufferPtr = buffer;
294:
295:     if (quote)
296:         *bufferPtr++ = '\"';
297:     while (*value)
298:     {
299:         *bufferPtr++ = *value++;
300:     }
301:     if (quote)
302:         *bufferPtr++ = '\"';
303: }
304:
305: /// <summary>
306: /// Internal serialization function, to be used for all signed values.
307: ///
308: /// Serialize a signed value to buffer.
309: ///
310: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
311: /// Width specifies the minimum width in characters. The value is always written right aligned.
312: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
313: ///
314: /// Base is the digit base, which can range from 2 to 36.
315: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
316: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
317: /// </summary>
318: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
319: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
320: /// <param name="value">Value to be serialized</param>
321: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
322: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
323: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
324: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
325: /// <param name="numBits">Specifies the number of bits used for the value</param>
326: static void SerializeInternalInt(char* buffer, size_t bufferSize, int64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
327: {
328:     if ((base < 2) || (base > 36))
329:         return;
330:
331:     int numDigits = 0;
332:     bool negative = (value < 0);
333:     uint64 absVal = static_cast<uint64>(negative ? -value : value);
334:     uint64 divisor = 1;
335:     uint64 divisorLast = 1;
336:     size_t absWidth = (width < 0) ? -width : width;
337:     const int maxDigits = BitsToDigits(numBits, base);
338:     while ((absVal >= divisor) && (numDigits <= maxDigits))
339:     {
340:         divisorLast = divisor;
341:         divisor *= base;
342:         ++numDigits;
343:     }
344:     divisor = divisorLast;
345:
346:     size_t numChars = (numDigits > 0) ? numDigits : 1;
347:     if (showBase)
348:     {
349:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
350:     }
351:     if (negative)
352:     {
353:         numChars++;
354:     }
355:     if (absWidth > numChars)
356:         numChars = absWidth;
357:     // Leave one character for \0
358:     if (numChars > bufferSize - 1)
359:         return;
360:
361:     char* bufferPtr = buffer;
362:     if (negative)
363:     {
364:         *bufferPtr++ = '-';
365:     }
366:
367:     if (showBase)
368:     {
369:         if (base == 2)
370:         {
371:             *bufferPtr++ = '0';
372:             *bufferPtr++ = 'b';
373:         }
374:         else if (base == 8)
375:         {
376:             *bufferPtr++ = '0';
377:         }
378:         else if (base == 16)
379:         {
380:             *bufferPtr++ = '0';
381:             *bufferPtr++ = 'x';
382:         }
383:     }
384:     if (leadingZeros)
385:     {
386:         if (absWidth == 0)
387:             absWidth = maxDigits;
388:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
389:         {
390:             *bufferPtr++ = '0';
391:         }
392:     }
393:     else
394:     {
395:         if (numDigits == 0)
396:         {
397:             *bufferPtr++ = '0';
398:         }
399:     }
400:     while (numDigits > 0)
401:     {
402:         int digit = (absVal / divisor) % base;
403:         *bufferPtr++ = GetDigit(digit);
404:         --numDigits;
405:         divisor /= base;
406:     }
407:     *bufferPtr++ = '\0';
408: }
409:
410: /// <summary>
411: /// Internal serialization function, to be used for all unsigned values.
412: ///
413: /// Serialize a unsigned value to buffer.
414: ///
415: /// The buffer will be filled to a maximum of bufferSize bytes, including end of string character. If this does not fit, nothing is written.
416: /// Width specifies the minimum width in characters. The value is always written right aligned.
417: /// If 0 is specified, the value will take as many characters as it needs to serialize, taking into account digit base and prefix.
418: ///
419: /// Base is the digit base, which can range from 2 to 36.
420: /// If showBase is true, and the base is either 2, 8, or 16, a prefix is added to the serialization (0b for base 2, 0 for base 8 and 0x for base 16.
421: /// If leadingZeros is true, the maximum amount of digits for the type and base is used, and '0' characters are prefixed to the value to fill up to this amount of characters.
422: /// </summary>
423: /// <param name="buffer">Pointer to buffer receiving the characters written</param>
424: /// <param name="bufferSize">Size of buffer, up to this size the buffer may be filled, if more space would be needed, nothing is written</param>
425: /// <param name="value">Value to be serialized</param>
426: /// <param name="width">Minimum width in characters, excluding any base prefix. If 0, uses as many characters as needed</param>
427: /// <param name="base">Digit base for serialization. Must be between 2 and 36</param>
428: /// <param name="showBase">If true, prefix value with base dependent string (0b for base 2, 0 for base 8, 0x for base 16)</param>
429: /// <param name="leadingZeros">If true, use as many digits as needed for the maximum value</param>
430: /// <param name="numBits">Specifies the number of bits used for the value</param>
431: static void SerializeInternalUInt(char* buffer, size_t bufferSize, uint64 value, int width, int base, bool showBase, bool leadingZeros, int numBits)
432: {
433:     if ((base < 2) || (base > 36))
434:         return;
435:
436:     int numDigits = 0;
437:     uint64 divisor = 1;
438:     uint64 divisorLast = 1;
439:     uint64 divisorHigh = 0;
440:     size_t absWidth = (width < 0) ? -width : width;
441:     const int maxDigits = BitsToDigits(numBits, base);
442:     while ((divisorHigh == 0) && (value >= divisor) && (numDigits <= maxDigits))
443:     {
444:         divisorHigh = ((divisor >> 32) * base >> 32); // Take care of overflow
445:         divisorLast = divisor;
446:         divisor *= base;
447:         ++numDigits;
448:     }
449:     divisor = divisorLast;
450:
451:     size_t numChars = (numDigits > 0) ? numDigits : 1;
452:     if (showBase)
453:     {
454:         numChars += ((base == 2) || (base == 16)) ? 2 : (base == 8) ? 1 : 0;
455:     }
456:     if (absWidth > numChars)
457:         numChars = absWidth;
458:     // Leave one character for \0
459:     if (numChars > bufferSize - 1)
460:         return;
461:
462:     char* bufferPtr = buffer;
463:
464:     if (showBase)
465:     {
466:         if (base == 2)
467:         {
468:             *bufferPtr++ = '0';
469:             *bufferPtr++ = 'b';
470:         }
471:         else if (base == 8)
472:         {
473:             *bufferPtr++ = '0';
474:         }
475:         else if (base == 16)
476:         {
477:             *bufferPtr++ = '0';
478:             *bufferPtr++ = 'x';
479:         }
480:     }
481:     if (leadingZeros)
482:     {
483:         if (absWidth == 0)
484:             absWidth = maxDigits;
485:         for (size_t digitIndex = numDigits; digitIndex < absWidth; ++digitIndex)
486:         {
487:             *bufferPtr++ = '0';
488:         }
489:     }
490:     else
491:     {
492:         if (numDigits == 0)
493:         {
494:             *bufferPtr++ = '0';
495:         }
496:     }
497:     while (numDigits > 0)
498:     {
499:         int digit = (value / divisor) % base;
500:         *bufferPtr++ = GetDigit(digit);
501:         --numDigits;
502:         divisor /= base;
503:     }
504:     *bufferPtr++ = '\0';
505: }
506:
507: } // namespace baremetal
```

- Line 42: We need to include `Util.h` for functions we are going to use: `strlen()`, `strncat()` and `strncpy()`
- Line 52: We declare a static function `SerializeInternalUInt()`, which is equal to the previous `SerializeInternal()` function
- Line 53: We declare a static function `SerializeInternalInt()`, which is the version of `SerializeInternal()` for signed integers
- Line 87-108: We implement serialization for uint8 using `SerializeInternalUInt()`
- Line 110-131: We implement serialization for int32 using `SerializeInternalInt()`
- Line 133-154: Serialization for uint32 now uses `SerializeInternalUInt()`
- Line 156-177: We implement serialization for int64 using `SerializeInternalInt()`
- Line 179-200: Serialization for uint64 now uses `SerializeInternalUInt()`
- Line 202-270: We implement `Serialize()` for double.
Note that we use `strncpy()` and `strlen()` next to `strncat()`
- Line 272-303: We implement `Serialize()` for const char*
- Line 305-408: We implement `SerializeInternalInt()`.
The main difference with `SerializeInternalUInt()` is that we can have negative values.
Therefore we take the absolute value and set a flag for negative values
- Line 410-505: We implement `SerializeInternalUInt()`, which is the same as the previous `SerializeInternal()` function

### Util.h {#TUTORIAL_13_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_1_UTILH}

We need to add the function `strncat()`, and `strncpy()` for later on for serialization.
You may recognize these as standard C functions.

Update the file `code/libraries/stdlib/include/stdlib/Util.h`

```cpp
File: code/libraries/stdlib/include/stdlib/Util.h
54: size_t strlen(const char* str);
55: char* strncpy(char* dest, const char* src, size_t maxLen);
56: char* strncat(char* dest, const char* src, size_t maxLen);
57:
```

### Util.cpp {#TUTORIAL_13_LOGGER_PRINTING_A_FORMATTED_STRING___STEP_1_UTILCPP}

Let's implement the new functions. They should be quite straightforward.

Update the file `code/libraries/stdlib/src/Util.cpp`

```cpp
File: code/libraries/stdlib/src/Util.cpp
98:
99: /// <summary>
100: /// Standard C strncpy function. Copies a string, up to maxLen characters. If maxLen characters are used, the last character is replaced by '\0'
101: /// </summary>
102: /// <param name="dest">Pointer to destination buffer</param>
103: /// <param name="src">Pointer to source buffer</param>
104: /// <param name="maxLen">Maximum number of characters to copy</param>
105: /// <returns>Pointer to destination buffer</returns>
106: char* strncpy(char* dest, const char* src, size_t maxLen)
107: {
108:     char* p = dest;
109:
110:     while (maxLen > 0)
111:     {
112:         if (*src == '\0')
113:         {
114:             break;
115:         }
116:
117:         *p++ = *src++;
118:         maxLen--;
119:     }
120:
121:     if (maxLen == 0)
122:     {
123:         p = dest + maxLen - 1;
124:     }
125:     if (p >= dest)
126:         *p = '\0';
127:
128:     return dest;
129: }
130:
131: /// <summary>
132: /// Standard C strncat function. Appends a string to the destination, up to maxLen characters. If maxLen characters are used, the last character is replaced by '\0'
133: /// </summary>
134: /// <param name="dest">Pointer to destination buffer</param>
135: /// <param name="src">Pointer to source buffer</param>
136: /// <param name="maxLen">Maximum number of characters to copy</param>
137: /// <returns>Pointer to destination buffer</returns>
138: char* strncat(char* dest, const char* src, size_t maxLen)
139: {
140:     char* p = dest;
141:
142:     while ((maxLen > 0) && *p)
143:     {
144:         p++;
145:         maxLen--;
146:     }
147:
148:     while ((maxLen > 0) && *src)
149:     {
150:         *p++ = *src++;
151:         maxLen--;
152:     }
153:
154:     if (maxLen == 0)
155:     {
156:         p = dest + maxLen - 1;
157:     }
158:     if (p >= dest)
159:         *p = '\0';
160:
161:     return dest;
162: }
```

## Adding the Logger class - Step 2 {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2}

### Adding version information {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_ADDING_VERSION_INFORMATION}

In order to be able to print the version of our code, we will add some infrastructure to CMake, to pass definitions on to our code.
We will extract the latest tag from our repository, and use that as the version number. If there is no tag, we will assume the version is `0.0.0`.

#### Updating the root CMake file {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_ADDING_VERSION_INFORMATION_UPDATING_THE_ROOT_CMAKE_FILE}

We'll add reading the most recent tag from `Git`, converting it to a version number, and then splitting the version number into components.
Additionaly, we'll add a variable to control whether logging is done using ANSI color coding.

Update the file `CMakeLists.txt`.

```cmake
File: CMakeLists.txt
13: include (functions)
14:
15: get_git_tag(GIT_TAG)
16:
17: message(STATUS "Tag found: ${GIT_TAG}")
18:
19: if (NOT "${GIT_TAG}" STREQUAL "")
20:     string(REGEX MATCH "^[0-9]*\.[0-9]*\.[0-9]*" VERSION_NUMBER ${GIT_TAG})
21: endif()
22:
23: if("${VERSION_NUMBER}" STREQUAL "")
24:     set(VERSION_NUMBER 0.0.0)
25: endif()
26:
27: parse_version(VERSION_NUMBER VERSION_MAJOR VERSION_MINOR VERSION_LEVEL VERSION_BUILD)
28:
29: set(VERSION_COMPOSED ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}.${VERSION_BUILD})
30: message(STATUS "Version: ${VERSION_COMPOSED}")
31:
...
66: option(BAREMETAL_CONSOLE_UART0 "Debug output to UART0" OFF)
67: option(BAREMETAL_CONSOLE_UART1 "Debug output to UART1" OFF)
68: option(BAREMETAL_COLOR_LOGGING "Use ANSI colors in logging" ON)
...
91: if (BAREMETAL_COLOR_LOGGING)
92:     set(BAREMETAL_COLOR_OUTPUT 1)
93: else ()
94:     set(BAREMETAL_COLOR_OUTPUT 0)
95: endif()
96: set(BAREMETAL_LOAD_ADDRESS 0x80000)
97:
98: set(DEFINES_C
99:     PLATFORM_BAREMETAL
100:     BAREMETAL_RPI_TARGET=${BAREMETAL_RPI_TARGET}
101:     BAREMETAL_COLOR_OUTPUT=${BAREMETAL_COLOR_OUTPUT}
102:     BAREMETAL_MAJOR=${VERSION_MAJOR}
103:     BAREMETAL_MINOR=${VERSION_MINOR}
104:     BAREMETAL_LEVEL=${VERSION_LEVEL}
105:     BAREMETAL_BUILD=${VERSION_BUILD}
106:     BAREMETAL_VERSION="${VERSION_COMPOSED}"
107:     )
108:
...
270: message(STATUS "Baremetal settings:")
271: message(STATUS "-- RPI target:                      ${BAREMETAL_RPI_TARGET}")
272: message(STATUS "-- Architecture options:            ${BAREMETAL_ARCH_CPU_OPTIONS}")
273: message(STATUS "-- Kernel name:                     ${BAREMETAL_TARGET_KERNEL}")
274: message(STATUS "-- Kernel load address:             ${BAREMETAL_LOAD_ADDRESS}")
275: message(STATUS "-- Debug output to UART0:           ${BAREMETAL_CONSOLE_UART0}")
276: message(STATUS "-- Debug output to UART1:           ${BAREMETAL_CONSOLE_UART1}")
277: message(STATUS "-- Color log output:                ${BAREMETAL_COLOR_LOGGING}")
278: message(STATUS "-- Version major:                   ${VERSION_MAJOR}")
279: message(STATUS "-- Version minor:                   ${VERSION_MINOR}")
280: message(STATUS "-- Version level:                   ${VERSION_LEVEL}")
281: message(STATUS "-- Version build:                   ${VERSION_BUILD}")
282: message(STATUS "-- Version composed:                ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}")
```

- Line 15: We use a utility function (to be defined) `get_git_tag` to retrieve the latest tag in `Git` and store it in variable `GIT_TAG`
- Line 17: We print the tag retrieved
- Line 19-21: If we found a tag, we match it to the pattern `{digit}+.{digit}+.{digit}+` and ignore the rest. This is stored in the variable `VERSION_NUMBER`
- Line 23-25: If the version extracted is empty, we set it to the default `0.0.0`
- Line 27: We use a utility function (to be defined) `parse_version` to extract the components from the version.
The components are stored respectively in variables `VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_LEVEL` and `VERSION_BUILD`.
Notice that we have 4 components to the version, so for now the last one will always be equal to 0
- Line 29-30: We recompose the version from the components into variable `VERSION_COMPOSED`, and print it
- Line 68: We define the variable `BAREMETAL_COLOR_LOGGING` which is `ON` by default
- Line 91-95: If `BAREMETAL_COLOR_LOGGING` is `ON`, we set the variable `BAREMETAL_COLOR_OUTPUT` to 1, otherwise we set it to 0
- Line 98-107: We add compiler definitions using the version components, the composed version, and the ANSI color log output selection
- Line 270-282: We print the variable settings for ANSI color log output, and version information

#### Adding functionality to the CMake utility file {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_ADDING_VERSION_INFORMATION_ADDING_FUNCTIONALITY_TO_THE_CMAKE_UTILITY_FILE}

We just used two utility functions that still need to be defined.

Update the file `cmake/functions.cmake`

```cmake
File: cmake/functions.cmake
...
18: function(get_git_tag out)
19:     if (UNIX)
20:         find_package(Git)
21:     else()
22:         find_program(GIT_EXECUTABLE git PATHS "C:/Program Files/Git/bin")
23:     endif()
24:     execute_process(COMMAND ${GIT_EXECUTABLE} describe --match "[0-9]*.[0-9]*.[0-9]*" --tags --abbrev=5 HEAD
25:         RESULT_VARIABLE RESULT
26:         OUTPUT_VARIABLE OUTPUT
27:         ERROR_QUIET
28:         OUTPUT_STRIP_TRAILING_WHITESPACE)
29:     if (${RESULT} EQUAL 0)
30:         set(${out} "${OUTPUT}" PARENT_SCOPE)
31:     else()
32:         set(${out} "" PARENT_SCOPE)
33:     endif()
34: endfunction()
35:
36: function(parse_version version_number version_major version_minor version_level version_build)
37:     if ("${${version_number}}" STREQUAL "")
38:         set(${version_number} "0.0.0.0" PARENT_SCOPE)
39:     endif()
40:     string(REPLACE "." ";" VERSION_PARTS "${${version_number}}")
41:     LIST(LENGTH VERSION_PARTS VERSION_NUM_PARTS)
42:
43:     set(VERSION_MAJOR 0)
44:     if (VERSION_NUM_PARTS GREATER_EQUAL 1)
45:         list(GET VERSION_PARTS 0 VERSION_MAJOR)
46:     endif()
47:     set(${version_major} "${VERSION_MAJOR}" PARENT_SCOPE)
48:
49:     set(VERSION_MINOR 0)
50:     if (VERSION_NUM_PARTS GREATER_EQUAL 2)
51:         list(GET VERSION_PARTS 1 VERSION_MINOR)
52:     endif()
53:     set(${version_minor} "${VERSION_MINOR}" PARENT_SCOPE)
54:
55:     set(VERSION_LEVEL 0)
56:     if (VERSION_NUM_PARTS GREATER_EQUAL 3)
57:         list(GET VERSION_PARTS 2 VERSION_LEVEL)
58:     endif()
59:     set(${version_level} "${VERSION_LEVEL}" PARENT_SCOPE)
60:
61:     set(VERSION_BUILD 0)
62:     if (VERSION_NUM_PARTS GREATER_EQUAL 4)
63:         list(GET VERSION_PARTS 3 VERSION_BUILD)
64:     endif()
65:     set(${version_build} "${VERSION_BUILD}" PARENT_SCOPE)
66:
67:     if ("${VERSION_MAJOR}" STREQUAL "" OR
68:         "${VERSION_MINOR}" STREQUAL "" OR
69:         "${VERSION_LEVEL}" STREQUAL "" OR
70:         "${VERSION_BUILD}" STREQUAL "")
71:         message(SEND_ERROR "Incorrectly specified version number: ${version_number}")
72:     endif()
73: endfunction()
74:
...
```

- Line 18-34: We define the function `get_git_tag` which returns the latest tag
  - Line 19-23: Depending on the platform, we determine the executable for git.
On Linux, we can simple try to find the package, on Windows, we assume that it is installed in `C:\Program Files\git`.
  - Line 24-28: We run git to get tags matching `<digit>+.<digit>+.<digit>+` and take the one from `HEAD`, i.e. the latest version.
This tag is stored in variable `OUTPUT`, the variable `RESULT` is used to store the execution return code
  - Line 29-33: If the command was executed without problems (`RESULT` is equal to 0), we set the output variable to `OUTPUT` (we used `PARENT_SCOPE` to return the value to outside the function).
If the command failed, we set the output to an empty string
- Line 36-73: We define the function `parse_version` which splits the version string into components `<major>.<minor>.<level>.<build>`.
Any parts that are not present are set to 0
  - Line 37-39: If the version string is empty we set it to `0.0.0.0` (again we use `PARENT_SCOPE`)
  - Line 40-41: We replace the dots with semicolons, so we can create a list from the string, the result is stored in variable `VERSION_PARTS`.
We determine the length of the list and store it in variable `VERSION_NUM_PARTS`
  - Line 43-47: We set the major part to 0 as a default.
If the number of parts is at least 1, we get the first item in the list and stored it in variable `VERSION_MAJOR`
  - Line 49-53: We set the minor part to 0 as a default.
If the number of parts is at least 2, we get the second item in the list and stored it in variable `VERSION_MINOR`
  - Line 55-59: We set the level part to 0 as a default.
If the number of parts is at least 3, we get the third item in the list and stored it in variable `VERSION_LEVEL`
  - Line 61-65: We set the build part to 0 as a default.
If the number of parts is at least 4, we get the fourth item in the list and stored it in variable `VERSION_BUILD`
  - Line 67-72: We perform a sanity check on the parts, and print an error message if one of them is empty

#### Version.h {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_ADDING_VERSION_INFORMATION_VERSIONH}

Now that we have set compiler definitions for the version, we can use them in code.

Create the file `code/libraries/baremetal/include/baremetal/Version.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Version.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Version.h
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Baremetal version information
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
43: /// Build version
44:
45: /// @brief Platform name
46: #define BAREMETAL_NAME              "Baremetal"
47:
48: /// @brief Major version number (specified by define at compile time)
49: #define BAREMETAL_MAJOR_VERSION     BAREMETAL_MAJOR
50: /// @brief Minor version number (specified by define at compile time)
51: #define BAREMETAL_MINOR_VERSION     BAREMETAL_MINOR
52: /// @brief Level version number (specified by define at compile time)
53: #define BAREMETAL_LEVEL_VERSION     BAREMETAL_LEVEL
54: /// @brief Version string
55: #define BAREMETAL_VERSION_STRING    GetVersion()
56:
57: namespace baremetal {
58:
59: void SetupVersion();
60: const char* GetVersion();
61:
62: }
```

- Line 46: We create a definition for the name of our platform
- Line 49-53: We create definitions of the platform version parts, using the compiler definitions passed from `CMake`
- Line 55: We create a definition to get the version string
- Line 59: We declare a function `SetupVersion()` to build the version string
- Line 60: We declare a function `GetVersion()` to return the version string

#### Version.cpp {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_ADDING_VERSION_INFORMATION_VERSIONCPP}

We need to implement the functions we just declared.

Create the file `code/libraries/baremetal/src/Version.cpp`

```cpp
File: code/libraries/baremetal/src/Version.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Version.cpp
5: //
6: // Namespace   : -
7: //
8: // Class       : -
9: //
10: // Description : Baremetal version information
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
40: #include "baremetal/Version.h"
41:
42: #include "stdlib/Util.h"
43: #include "baremetal/Format.h"
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
65:         Format(s_baremetalVersionString, BufferSize, "%d.%d.%d", BAREMETAL_MAJOR_VERSION, BAREMETAL_MINOR_VERSION, BAREMETAL_LEVEL_VERSION);
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

- Line 48-51: We define the local variable `s_baremetalVersionString` and its size
- Line 55-68: We implement `SetupVersion()`, which prints a formatted string to `s_baremetalVersionString`
- Line 70-77: We implement `GetVersion()`, which simply returns the string `s_baremetalVersionString`

#### Logger.h {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_ADDING_VERSION_INFORMATION_LOGGERH}

We'll now add the `Logger` class, which can be used to log to the console, at different log levels.

Create the file `code/libraries/baremetal/include/baremetal/Logger.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Logger.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Logger.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Logger
9: //
10: // Description : Basic logging to a device
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
42: #include "baremetal/Console.h"
43: #include "stdlib/StdArg.h"
44: #include "stdlib/Types.h"
45:
46: /// @file
47: /// Logger functionality
48:
49: namespace baremetal {
50:
51: /// <summary>
52: /// Logging severity classes
53: /// </summary>
54: enum class LogSeverity
55: {
56:     /// @brief Halt the system after processing this message
57:     Panic,
58:     /// @brief Severe error in this component, system may continue to work
59:     Error,
60:     /// @brief Non-severe problem, component continues to work
61:     Warning,
62:     /// @brief Informative message, which is interesting for the system user
63:     Info,
64:     /// @brief Message, which is only interesting for debugging this component
65:     Debug
66: };
67:
68: class Timer;
69:
70: /// <summary>
71: /// Logger class
72: /// </summary>
73: class Logger
74: {
75:     /// <summary>
76:     /// Construct the singleton Logger instance if needed, and return a reference to the instance. This is a friend function of class Logger
77:     /// </summary>
78:     /// <returns>Reference to the singleton logger instance</returns>
79:     friend Logger& GetLogger();
80:
81: private:
82:     /// @brief True if class is already initialized
83:     bool m_isInitialized;
84:     /// @brief Pointer to timer instance
85:     Timer* m_timer;
86:     /// @brief Currently set logging severity level
87:     LogSeverity m_level;
88:     /// @brief Singleton console instance
89:     static Console s_console;
90:
91:     explicit Logger(LogSeverity logLevel, Timer* timer = nullptr);
92:
93: public:
94:     static bool HaveLogger();
95:
96:     bool Initialize();
97:     void SetLogLevel(LogSeverity logLevel);
98:
99:     void Log(const char* from, int line, LogSeverity severity, const char* message, ...);
100:     void LogV(const char* from, int line, LogSeverity severity, const char* message, va_list args);
101:
102:     void Trace(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...);
103:     void TraceV(const char* filename, int line, const char* function, LogSeverity severity, const char* message, va_list args);
104: };
105:
106: Logger& GetLogger();
107:
108: /// @brief Define the static variable From to the specified name, to support printing a different file specification in LOG_* macros
109: #define LOG_MODULE(name)         static const char From[] = name
110:
111: /// @brief Log a panic message
112: #define LOG_PANIC(...)           GetLogger().Log(From, __LINE__, LogSeverity::Panic, __VA_ARGS__)
113: /// @brief Log an error message
114: #define LOG_ERROR(...)           GetLogger().Log(From, __LINE__, LogSeverity::Error, __VA_ARGS__)
115: /// @brief Log a warning message
116: #define LOG_WARNING(...)         GetLogger().Log(From, __LINE__, LogSeverity::Warning, __VA_ARGS__)
117: /// @brief Log a info message
118: #define LOG_INFO(...)            GetLogger().Log(From, __LINE__, LogSeverity::Info, __VA_ARGS__)
119: /// @brief Log a debug message
120: #define LOG_DEBUG(...)           GetLogger().Log(From, __LINE__, LogSeverity::Debug, __VA_ARGS__)
121:
122: /// @brief Log a message with specified severity and message string
123: #define LOG(severity, message)   GetLogger().Log(From, __LINE__, severity, message);
124:
125: /// @brief Log a warning message
126: #define TRACE_WARNING(...)       GetLogger().Trace(__FILE_NAME__, __LINE__, __func__, LogSeverity::Warning, __VA_ARGS__)
127: /// @brief Log a info message
128: #define TRACE_INFO(...)          GetLogger().Trace(__FILE_NAME__, __LINE__, __func__, LogSeverity::Info, __VA_ARGS__)
129: /// @brief Log a debug message
130: #define TRACE_DEBUG(...)         GetLogger().Trace(__FILE_NAME__, __LINE__, __func__, LogSeverity::Debug, __VA_ARGS__)
131:
132: /// @brief Log a message with specified severity and message string
133: #define TRACE(severity, message) GetLogger().Trace(From, __FILE_NAME__, __LINE__, __func__, severity, message);
134:
135: } // namespace baremetal
```

- Line 51-66: We declare an enum type `LogSeverity` to signify the logging / tracing level
- Line 70-106: We declare the `Logger` class
  - Line 79: We declare the `GetLogger()` function as a friend.
As before, this is the way to instantiate and retrieve the singleton
  - Line 82-83: The member variable `m_isInitialized` is used to guard against multiple initializations
  - Line 84-85: We keep a pointer in the member variable `m_timer` to the `Timer` instance which will be used to request the current time for logging
  - Line 86-87: The member variable `m_level` is used as comparison level for logging / tracing.
  Any log or trace statements with a priority equal to or higher than `m_level` (with a value equal to or lower than `m_level`) will be shown, others will be ignored
  - Line 88-89: We keep a reference in the member variable `m_console` to the `Console` instance
  - Line 91: We declare the constructor for `Logger`
  - Line 94: The method `HaveLogger()` checks whether the singleton instance of `Logger` is initialized.
  It returns true if there is an initialized instance, false if not
  - Line 96: The method `Initialize()` initializes the logger, guarded by `m_isInitialized`
  - Line 97: The method `SetLogLevel()` sets the value of `m_level` to change filtering of log / trace messages
  - Line 99: The method `Log()` writes a log statement for a specific module name and line number, using a specific severity level, and with a format string and variable arguments
  - Line 100: The method `LogV()` writes a log statement for a specific module name and line number, using a specific severity level, and with a format string and a variable argument list
  - Line 102: The method `Trace()` writes a trace statement for a specific source file, line number and function, using a specific severity level, and with a format string and variable arguments
  - Line 103: The method `TraceV()` writes a trace statement for a specific source file, line number and function, using a specific severity level, and with a format string and a variable argument list
- Line 106: We declare the accessor for the singleton `Logger` instance, `GetLogger()`
- Line 109: The macro `LOG_MODULE` is meant to define a variable that is used by the `LOG_` macros, to specify the module name to be used
- Line 111-112: The macro `LOG_PANIC` is meant to log at the highest level `Panic` (which will cause a Halt)
- Line 113-114: The macro `LOG_ERROR` writes to the log at level `Error`
- Line 115-116: The macro `LOG_WARNING` writes to the log at level `Warning`
- Line 117-118: The macro `LOG_INFO` writes to the log at level `Info`
- Line 119-120: The macro `LOG_DEBUG` writes to the log at level `Debug`
- Line 122-123: The macro `LOG` writes to the log at the specified level, with a single string as the argument
- Line 125-126: The macro `TRACE_WARNING` writes a trace at level `Warning`
- Line 127-128: The macro `TRACE_INFO` writes a trace at level `Info`
- Line 129-130: The macro `TRACE_DEBUG` writes a trace at level `Debug`
- Line 132-133: The macro `TRACE` writes a trace at the specified level, with a single string as the argument

#### Logger.cpp {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_ADDING_VERSION_INFORMATION_LOGGERCPP}

We will add the implementation for the `Logger` class.

Create the file `code/libraries/baremetal/src/Logger.cpp`

```cpp
File: code/libraries/baremetal/src/Logger.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Logger.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Logger
9: //
10: // Description : Basic logging to a device
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
40: #include "baremetal/Logger.h"
41:
42: #include "baremetal/Console.h"
43: #include "baremetal/Format.h"
44: #include "baremetal/System.h"
45: #include "baremetal/Timer.h"
46: #include "baremetal/Version.h"
47: #include "stdlib/Util.h"
48:
49: /// @file
50: /// Logger functionality implementation
51:
52: using namespace baremetal;
53:
54: /// @brief Define log name
55: LOG_MODULE("Logger");
56:
57: Console Logger::s_console(nullptr);
58:
59: /// <summary>
60: /// Construct a logger
61: /// </summary>
62: /// <param name="logLevel">Only messages with (severity <= m_level) will be logged</param>
63: /// <param name="timer">Pointer to system timer object (time is not logged, if this is nullptr). Defaults to nullptr</param>
64: Logger::Logger(LogSeverity logLevel, Timer* timer /*= nullptr*/)
65:     : m_isInitialized{}
66:     , m_timer{timer}
67:     , m_level{logLevel}
68: {
69: }
70:
71: /// <summary>
72: /// Check whether the singleton logger was instantiated and initialized
73: /// </summary>
74: /// <returns>Returns true if the singleton logger instance is created and initialized, false otherwise</returns>
75: bool Logger::HaveLogger()
76: {
77:     return GetLogger().m_isInitialized;
78: }
79:
80: /// <summary>
81: /// Initialize logger
82: /// </summary>
83: /// <returns>true on succes, false on failure</returns>
84: bool Logger::Initialize()
85: {
86:     if (m_isInitialized)
87:         return true;
88:     SetupVersion();
89:     m_isInitialized = true; // Stop reentrant calls from happening
90:     LOG_INFO(BAREMETAL_NAME " %s started on %s %s (AArch64)", BAREMETAL_VERSION_STRING, "Raspberry Pi", BAREMETAL_RPI_TARGET);
91:
92:     return true;
93: }
94:
95: /// <summary>
96: /// Set maximum log level (minimum log priority). Any log statements with a value below this level will be ignored
97: /// </summary>
98: /// <param name="logLevel">Maximum log level</param>
99: void Logger::SetLogLevel(LogSeverity logLevel)
100: {
101:     m_level = logLevel;
102: }
103:
104: /// <summary>
105: /// Write a string with variable arguments to the logger
106: /// </summary>
107: /// <param name="source">Source name or file name</param>
108: /// <param name="line">Source line number</param>
109: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
110: /// <param name="message">Formatted message string, with variable arguments</param>
111: void Logger::Log(const char* source, int line, LogSeverity severity, const char* message, ...)
112: {
113:     va_list var;
114:     va_start(var, message);
115:     LogV(source, line, severity, message, var);
116:     va_end(var);
117: }
118:
119: /// <summary>
120: /// Write a string with variable arguments to the logger
121: /// </summary>
122: /// <param name="source">Source name or file name</param>
123: /// <param name="line">Source line number</param>
124: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
125: /// <param name="message">Formatted message string</param>
126: /// <param name="args">Variable argument list</param>
127: void Logger::LogV(const char* source, int line, LogSeverity severity, const char* message, va_list args)
128: {
129:     if (static_cast<int>(severity) > static_cast<int>(m_level))
130:         return;
131:
132:     static const size_t BufferSize = 1024;
133:     char buffer[BufferSize]{};
134:
135:     char sourceString[BufferSize]{};
136:     Format(sourceString, BufferSize, " (%s:%d)", source, line);
137:
138:     char messageBuffer[BufferSize]{};
139:     FormatV(messageBuffer, BufferSize, message, args);
140:
141:     switch (severity)
142:     {
143:     case LogSeverity::Panic:
144:         strncat(buffer, "!Panic!", BufferSize);
145:         break;
146:     case LogSeverity::Error:
147:         strncat(buffer, "Error  ", BufferSize);
148:         break;
149:     case LogSeverity::Warning:
150:         strncat(buffer, "Warning", BufferSize);
151:         break;
152:     case LogSeverity::Info:
153:         strncat(buffer, "Info   ", BufferSize);
154:         break;
155:     case LogSeverity::Debug:
156:         strncat(buffer, "Debug  ", BufferSize);
157:         break;
158:     }
159:
160:     if (m_timer != nullptr)
161:     {
162:         const size_t TimeBufferSize = 32;
163:         char timeBuffer[TimeBufferSize]{};
164:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
165:         if (strlen(timeBuffer) > 0)
166:         {
167:             strncat(buffer, timeBuffer, BufferSize);
168:             strncat(buffer, " ", BufferSize);
169:         }
170:     }
171:
172:     strncat(buffer, messageBuffer, BufferSize);
173:     strncat(buffer, sourceString, BufferSize);
174:     strncat(buffer, "\n", BufferSize);
175:
176: #if BAREMETAL_COLOR_OUTPUT
177:     switch (severity)
178:     {
179:     case LogSeverity::Panic:
180:         s_console.Write(buffer, ConsoleColor::BrightRed);
181:         break;
182:     case LogSeverity::Error:
183:         s_console.Write(buffer, ConsoleColor::Red);
184:         break;
185:     case LogSeverity::Warning:
186:         s_console.Write(buffer, ConsoleColor::BrightYellow);
187:         break;
188:     case LogSeverity::Info:
189:         s_console.Write(buffer, ConsoleColor::Cyan);
190:         break;
191:     case LogSeverity::Debug:
192:         s_console.Write(buffer, ConsoleColor::Yellow);
193:         break;
194:     default:
195:         s_console.Write(buffer, ConsoleColor::White);
196:         break;
197:     }
198: #else
199:     s_console.Write(buffer);
200: #endif
201:
202:     if (severity == LogSeverity::Panic)
203:     {
204:         GetSystem().Halt();
205:     }
206: }
207:
208: /// <summary>
209: /// Write a trace string with variable arguments to the logger
210: /// </summary>
211: /// <param name="filename">File name</param>
212: /// <param name="line">Source line number</param>
213: /// <param name="function">Function name</param>
214: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
215: /// <param name="message">Formatted message string, with variable arguments</param>
216: void Logger::Trace(const char* filename, int line, const char* function, LogSeverity severity, const char* message, ...)
217: {
218:     va_list var;
219:     va_start(var, message);
220:     TraceV(filename, line, function, severity, message, var);
221:     va_end(var);
222: }
223:
224: /// <summary>
225: /// Write a trace string with variable arguments to the logger
226: /// </summary>
227: /// <param name="filename">File name</param>
228: /// <param name="line">Source line number</param>
229: /// <param name="function">Function name</param>
230: /// <param name="severity">Severity to log with (log severity levels equal to or greater than the current set log level wil be ignored</param>
231: /// <param name="message">Formatted message string</param>
232: /// <param name="args">Variable argument list</param>
233: void Logger::TraceV(const char* filename, int line, const char* function, LogSeverity severity, const char* message, va_list args)
234: {
235:     if (static_cast<int>(severity) > static_cast<int>(m_level))
236:         return;
237:
238:     static const size_t BufferSize = 1024;
239:     char buffer[BufferSize]{};
240:
241:     char sourceString[BufferSize]{};
242:     Format(sourceString, BufferSize, "%s (%s:%d) ", function, filename, line);
243:
244:     char messageBuffer[BufferSize]{};
245:     FormatV(messageBuffer, BufferSize, message, args);
246:
247:     switch (severity)
248:     {
249:     case LogSeverity::Warning:
250:         strncat(buffer, "Warning", BufferSize);
251:         break;
252:     case LogSeverity::Info:
253:         strncat(buffer, "Info   ", BufferSize);
254:         break;
255:     case LogSeverity::Debug:
256:         strncat(buffer, "Debug  ", BufferSize);
257:         break;
258:     default:
259:         break;
260:     }
261:
262:     if (m_timer != nullptr)
263:     {
264:         const size_t TimeBufferSize = 32;
265:         char timeBuffer[TimeBufferSize]{};
266:         m_timer->GetTimeString(timeBuffer, TimeBufferSize);
267:         if (strlen(timeBuffer) > 0)
268:         {
269:             strncat(buffer, timeBuffer, BufferSize);
270:             strncat(buffer, " ", BufferSize);
271:         }
272:     }
273:
274:     strncat(buffer, sourceString, BufferSize);
275:     strncat(buffer, messageBuffer, BufferSize);
276:     strncat(buffer, "\n", BufferSize);
277:
278: #if BAREMETAL_COLOR_OUTPUT
279:     switch (severity)
280:     {
281:     case LogSeverity::Warning:
282:         s_console.Write(buffer, ConsoleColor::BrightYellow);
283:         break;
284:     case LogSeverity::Info:
285:         s_console.Write(buffer, ConsoleColor::Cyan);
286:         break;
287:     case LogSeverity::Debug:
288:         s_console.Write(buffer, ConsoleColor::Yellow);
289:         break;
290:     default:
291:         s_console.Write(buffer, ConsoleColor::White);
292:         break;
293:     }
294: #else
295:     s_console.Write(buffer);
296: #endif
297: }
298:
299: /// <summary>
300: /// Construct the singleton logger and initializat it if needed, and return a reference to the instance
301: /// </summary>
302: /// <returns>Reference to the singleton logger instance</returns>
303: Logger& baremetal::GetLogger()
304: {
305:     static Logger s_logger(LogSeverity::Debug, &GetTimer());
306:     return s_logger;
307: }
```

- Line 54-55: We use the macro `LOG_MODULE` also internally to specify that we are in the `Logger` class itself, and we can use the `LOG_` and `TRACE_` macros
- Line 59-69: We implement the constructor
- Line 71-78: We implement the `HaveLogger()` method
- Line 80-93: We implement the `Initialize()` method
  - Line 86-87: We guard against multiple initialization
  - Line 88: We use the function `SetupVersion()` from `Version.h` to set up the version string
  - Line 90: We use the `Logger` itself to log the first message, stating the platform name, the target platform and its version
- Line 95-102: We implement the `SetLogLevel()` method. This simply sets the maximum log level for filtering
- Line 104-117: We implement the `Log()` method. This simply calls `LogV()` after setting up the variable argument list
- Line 119-206: We implement the `LogV()` method
  - Line 129-130: If the severity level passed in is too high (priority too low) we simply return without printing
  - Line 132-133: We define a buffer to hold the line to write
  - Line 135-136: We print source name and line number into a separate buffer
  - Line 138-139: We print the message with arguments into a separate buffer
  - Line 141-158: For each level, we add a string to the line buffer denoting the severity level
  - Line 160-170: If a `Timer` was passed in, we request the current time, and print it into the line buffer.
We'll add the timer method in a minute
  - Line 172-174: We add the message, source information and end of line to the buffer
  - Line 176-200: Depending on whether we defined `BAREMETAL_COLOR_OUTPUT`, we either simply print the buffer without color, or we use a severity level specific color
  - Line 202-205: If the severity level is `Panic` we halt the system
- Line 208-222: We implement the `Trace()` method. This simply calls `TraceV()` after setting up the variable argument list
- Line 224-297: We implement the `TraceV()` method
  - Line 235-236: If the severity level passed in is too high (priority too low) we simply return without printing
  - Line 238-239: We define a buffer to hold the line to write
  - Line 241-242: We print source name and line number into a separate buffer
  - Line 244-245: We print the message with arguments into a separate buffer
  - Line 247-260: For each level, we add a string to the line buffer denoting the severity level
  - Line 262-272: If a `Timer` was passed in, we request the current time, and print it into the line buffer.
We'll add the timer method in a minute
  - Line 274-276: We add the message, source information and end of line to the buffer
  - Line 278-296: Depending on whether we defined `BAREMETAL_COLOR_OUTPUT`, we either simply print the buffer without color, or we use a severity level specific color
- Line 299-308: We implement the friend function `GetLogger()` to retrieve the singleton instance of the logger.
As a default, we set the maximum log level to `Debug` meaning that everything is logged. We also use the singleton `Timer` instance

### Timer.h {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_TIMERH}

We need to add the method `GetTimeString()` to the `Timer` class.

Update the file `code/libraries/baremetal/include/baremetal/Timer.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Timer.h
...
72: public:
73:     Timer(IMemoryAccess& memoryAccess);
74:
75:     void GetTimeString(char* buffer, size_t bufferSize);
76:
77:     static void WaitCycles(uint32 numCycles);
...
```

### Timer.cpp {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_TIMERCPP}

We then implement method `GetTimeString()` for the `Timer` class.

Update the file `code/libraries/baremetal/src/Timer.cpp`

```cpp
File: code/libraries/baremetal/src/Timer.cpp
...
75: /// <summary>
76: /// Write string representing current time according to our time zone to the buffer.
77: ///
78: /// For now returns an empty string
79: /// </summary>
80: /// <param name="buffer">Buffer to receive the time string</param>
81: /// <param name="bufferSize">Size of the buffer, to protect against overflowing the buffer</param>
82: void Timer::GetTimeString(char* buffer, size_t bufferSize)
83: {
84:     if (bufferSize > 0)
85:         *buffer = '\0';
86: }
87:
...
```

So for now we simply return an empty string. Updating the clock will take some more effort, which will be done later.

### Using the Logger class {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_USING_THE_LOGGER_CLASS}

#### System.cpp {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_USING_THE_LOGGER_CLASS_SYSTEMCPP}

We will be using the logger in the `sysinit()` function, to instantiate it and print the first log message.
Also, we will print logging info in `Halt()` and `Reboot()`.

Update the file `code/libraries/baremetal/src/System.cpp`

```cpp
File: code/libraries/baremetal/src/System.cpp
...
40: #include "baremetal/System.h"
41:
42: #include "stdlib/Util.h"
43: #include "baremetal/ARMInstructions.h"
44: #include "baremetal/BCMRegisters.h"
45: #include "baremetal/Logger.h"
46: #include "baremetal/MemoryAccess.h"
47: #include "baremetal/SysConfig.h"
48: #include "baremetal/Timer.h"
49: #include "baremetal/UART0.h"
50: #include "baremetal/UART1.h"
...
57: /// @brief Define log name for this module
58: LOG_MODULE("System");
59:
...
119: void System::Halt()
120: {
121:     LOG_INFO("Halt");
122:     Timer::WaitMilliSeconds(WaitTime);
...
132: void System::Reboot()
133: {
134:     LOG_INFO("Reboot");
135:     Timer::WaitMilliSeconds(WaitTime);
...
178:     for (void (**func)(void) = &__init_start; func < &__init_end; func++)
179:     {
180:         (**func)();
181:     }
182:
183:     Device* logDevice{};
184: #if defined(BAREMETAL_CONSOLE_UART0)
185:     auto& uart = GetUART0();
186:     uart.Initialize(115200);
187:     uart.WriteString("\nSetting up UART0\n");
188:     logDevice = &uart;
189: #elif defined(BAREMETAL_CONSOLE_UART1)
190:     auto& uart = GetUART1();
191:     uart.Initialize(115200);
192:     uart.WriteString("\nSetting up UART1\n");
193:     logDevice = &uart;
194: #endif
195:     GetConsole().AssignDevice(logDevice);
196:     GetLogger().Initialize();
197:     LOG_INFO("Starting up");
198:
199:     if (static_cast<ReturnCode>(main()) == ReturnCode::ExitReboot)
```

- Line 45: We replace the include for `Console.h` with `Logger.h`
- Line 57-58: We set the module name for logging to `System`
- Line 121: We use `LOG_INFO` to log the message `Halt`
- Line 134: We use `LOG_INFO` to log the message `Reboot`
- Line 183: We keep a pointer to the device to be used for logging
- Line 195: We get the console and assign the device to be used
- Line 196: We instantiate  the `Logger` singleton by calling `GetLogger()` and initialize it.
If the `Logger` instance would be created earlier, it will not be initialized before this point, so any tracing would get lost as we did not assign a device to the console yet
- Line 197: We use `LOG_INFO` to log the message `Starting up`

### Update the application code {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_UPDATE_THE_APPLICATION_CODE}

We can now also update the application to use the logging macros.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/BCMRegisters.h"
3: #include "baremetal/Console.h"
4: #include "baremetal/Logger.h"
5: #include "baremetal/Mailbox.h"
6: #include "baremetal/MemoryManager.h"
7: #include "baremetal/RPIProperties.h"
8: #include "baremetal/SysConfig.h"
9: #include "baremetal/Serialization.h"
10: #include "baremetal/System.h"
11: #include "baremetal/Timer.h"
12:
13: LOG_MODULE("main");
14:
15: using namespace baremetal;
16:
17: int main()
18: {
19:     auto& console = GetConsole();
20:     LOG_DEBUG("Hello World!");
21:
22:     char buffer[128];
23:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
24:     RPIProperties properties(mailbox);
25:
26:     uint64 serial{};
27:     if (properties.GetBoardSerial(serial))
28:     {
29:         LOG_INFO("Mailbox call succeeded");
30:         LOG_INFO("Serial: %016llx", serial);
31:     }
32:     else
33:     {
34:         LOG_ERROR("Mailbox call failed");
35:     }
36:
37:     LOG_INFO("Wait 5 seconds");
38:     Timer::WaitMilliSeconds(5000);
39:
40:     console.Write("Press r to reboot, h to halt\n");
41:     char ch{};
42:     while ((ch != 'r') && (ch != 'h'))
43:     {
44:         ch = console.ReadChar();
45:         console.WriteChar(ch);
46:     }
47:
48:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
49: }
```

- Line 4: We add the include for `Logger.h`
- Line 13: We use `LOG_MODULE` to set the logging name to `main`
- Line 20: We use `LOG_DEBUG` in this case (to show the color differences) to log the `Hello World!` message
- Line 29-30: We can now use the variable arguments to shorten the logging for the serial number.
Notice that we use the format string `%016llx` to write a zero leading 16 digit hexedecimal representation of the serial number, using a signed 64 bit integer
- Line 34: We use `LOG_ERROR` to write an error message if the mailbox call fails
- Line 37: We use `LOG_INFO` to write the message

Notice also that we keep using the console, as we need to read and write characters from and to the console.

### Configuring, building and debugging {#TUTORIAL_13_LOGGER_ADDING_THE_LOGGER_CLASS___STEP_2_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will now print output in color, depending on the log severity level, and also print source name and line number:

<img src="images/tutorial-13-logger.png" alt="Console output" width="600"/>

## Assertion - Step 3 {#TUTORIAL_13_LOGGER_ASSERTION___STEP_3}

It is handy to have a function or macro available to check for a condition, and panic if this is not fulfilled.
This is very similar to the standard C assert() macro.

So let's add this, and log a Panic message if the condition check fails.

### Assert.h {#TUTORIAL_13_LOGGER_ASSERTION___STEP_3_ASSERTH}

Create the file `code/libraries/baremetal/include/baremetal/Assert.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Assert.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Assert.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : Assertion functions
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
42: #include "stdlib/Macros.h"
43: #include "stdlib/Types.h"
44:
45: /// @file
46: /// Assertion functions
47:
48: namespace baremetal {
49:
50: /// @brief Assertion callback function, which can be installed to handle a failed assertion
51: using AssertionCallback = void(const char* expression, const char* fileName, int lineNumber);
52:
53: #ifdef NDEBUG
54: /// If building for release, assert is replaced by nothing
55: #define assert(expr) expr;
56: void AssertionFailed(const char* expression, const char* fileName, int lineNumber);
57: #else
58:
59: /// @brief Assertion. If the assertion fails, AssertionFailed is called.
60: ///
61: /// <param name="expression">Expression to evaluate.
62: /// If true the assertion succeeds and nothing happens, if false the assertion fails, and the assertion failure handler is invoked.</param>
63: #define assert(expression) (likely(expression) ? ((void)0) : baremetal::AssertionFailed(#expression, __FILE__, __LINE__))
64:
65: #endif
66:
67: void ResetAssertionCallback();
68: void SetAssertionCallback(AssertionCallback* callback);
69:
70: } // namespace baremetal
```

- Line 50-51: We declare the prototype of the assertion failure handler function `AssertionCallback`
- Line 53: If we build for release, we simply ignore the assertion
- Line 56: We declare the assertion failure function `AssertionFailed()`.
This will call the assertion failure handler, which logs a `Panic` message and halts the system by default, unless a custom assertion failure handler is installed
- Line 59-63: We define the `assert()` macro. This will invoke the assertion failure function `AssertionFailed()` in case the assertion fails
- Line 67: We declare the function `ResetAssertionCallback()` which resets the assertion failure handler to the default
- Line 68: We declare the function `SetAssertionCallback()` which sets a custom assertion failure handler

### Assert.cpp {#TUTORIAL_13_LOGGER_ASSERTION___STEP_3_ASSERTCPP}

Create the file `code/libraries/baremetal/src/Assert.cpp`

```cpp
File: code/libraries/baremetal/src/Assert.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : Assert.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : -
9: //
10: // Description : Assertion functions
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
40: #include "baremetal/Assert.h"
41:
42: #include "baremetal/Logger.h"
43: #include "baremetal/System.h"
44:
45: /// @file
46: /// Assertion functions implementation
47:
48: /// @brief Define log name
49: LOG_MODULE("Assert");
50:
51: namespace baremetal {
52:
53: #ifndef NDEBUG
54:
55: static void AssertionFailedDefault(const char* expression, const char* fileName, int lineNumber);
56:
57: /// @brief Assertion callback function
58: ///
59: /// Set to the default assertion handler function at startup, but can be overriden
60: static AssertionCallback* s_callback = AssertionFailedDefault;
61:
62: /// <summary>
63: /// Log assertion failure and halt, is not expected to return (but may if a different assertion failure function is set up)
64: /// </summary>
65: /// <param name="expression">Expression to be printed</param>
66: /// <param name="fileName">Filename of file causing the failed assertion</param>
67: /// <param name="lineNumber">Line number causing the failed assertion</param>
68: void AssertionFailed(const char* expression, const char* fileName, int lineNumber)
69: {
70:     if (s_callback != nullptr)
71:         s_callback(expression, fileName, lineNumber);
72: }
73:
74: /// <summary>
75: /// Default failed assertion handler
76: /// </summary>
77: /// <param name="expression">Expression to be printed</param>
78: /// <param name="fileName">Filename of file causing the failed assertion</param>
79: /// <param name="lineNumber">Line number causing the failed assertion</param>
80: static void AssertionFailedDefault(const char* expression, const char* fileName, int lineNumber)
81: {
82:     GetLogger().Log(fileName, lineNumber, LogSeverity::Panic, "assertion failed: %s", expression);
83: }
84:
85: /// <summary>
86: /// Reset the assertion failure handler to the default
87: /// </summary>
88: void ResetAssertionCallback()
89: {
90:     s_callback = AssertionFailedDefault;
91: }
92:
93: /// <summary>
94: /// Sets up a custom assertion failure handler
95: /// </summary>
96: /// <param name="callback">Assertion failure handler</param>
97: void SetAssertionCallback(AssertionCallback* callback)
98: {
99:     s_callback = callback;
100: }
101:
102: #else
103:
104: /// <summary>
105: /// Reset the assertion failure handler to the default
106: /// </summary>
107: void ResetAssertionCallback()
108: {
109: }
110:
111: /// <summary>
112: /// Sets up a custom assertion failure handler
113: /// </summary>
114: /// <param name="callback">Assertion failure handler</param>
115: void SetAssertionCallback(AssertionCallback* callback)
116: {
117: }
118:
119: #endif
120:
121: } // namespace baremetal
```

- Line 54-101: This section is only for non release builds
- Line 55: We declare a static function `AssertionFailedDefault()` as the default assertion failure function
- Line 57-60: We define a static variable to point to the set assertion failure handler function, which is set to `AssertionFailedDefault()` by default
- Line 62-72: We implement the function `AssertionFailed()` which is called when the `assert()` macro fails.
This will call the set assertion failure handler function, if any
- Line 74-83: We implement the default assertion failure handler function `AssertionFailedDefault()`, which logs a `Panic` message, and will then halt the system
- Line 85-91: We implement the function `ResetAssertionCallback()` which will reset the assertion failure handler function to default
- Line 93-100: We implement the function `SetAssertionCallback()` which will set a custom assertion failure handler function
- Line 103-118: This section is only for release builds
- Line 104-109: We implement the function `ResetAssertionCallback()` which will not do anything
- Line 111-117: We implement the function `SetAssertionCallback()` which will not do anything

### Macros.h {#TUTORIAL_13_LOGGER_ASSERTION___STEP_3_MACROSH}

We use the construct `likely()` in the `assert()` macro. This needs to be defined.

Update the file `code/libraries/baremetal/include/baremetal/Macros.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Macros.h
53: /// @brief Make branch prediction expect exp to be true (GCC compiler only)
54: /// @param exp Expression to be evaluated
55: #define likely(exp)         __builtin_expect (!!(exp), 1)
56: /// @brief Make branch prediction expect exp to be false (GCC compiler only)
57: /// @param exp Expression to be evaluated
58: #define unlikely(exp)       __builtin_expect (!!(exp), 0)
59:
```

These macros make use of builtin functionality in the compiler to influence the branch prediction.

### Update application code {#TUTORIAL_13_LOGGER_ASSERTION___STEP_3_UPDATE_APPLICATION_CODE}

Let us for the sake of demonstration add a failing assertion to the application code.
This will fire twice, once using our own handler function, the second time using the default handler function.

Update the file `code/applications/demo/src/main.cpp`

```cpp
File: code/applications/demo/src/main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/BCMRegisters.h"
4: #include "baremetal/Console.h"
5: #include "baremetal/Logger.h"
6: #include "baremetal/Mailbox.h"
7: #include "baremetal/MemoryManager.h"
8: #include "baremetal/RPIProperties.h"
9: #include "baremetal/Serialization.h"
10: #include "baremetal/SysConfig.h"
11: #include "baremetal/System.h"
12: #include "baremetal/Timer.h"
13:
14: LOG_MODULE("main");
15:
16: using namespace baremetal;
17:
18: void MyHandler(const char* expression, const char* fileName, int lineNumber)
19: {
20:     LOG_INFO("An assertion failed on location %s:%d, %s", fileName, lineNumber, expression);
21: }
22:
23: int main()
24: {
25:     auto& console = GetConsole();
26:     LOG_DEBUG("Hello World!");
27:
28:     Mailbox mailbox(MailboxChannel::ARM_MAILBOX_CH_PROP_OUT);
29:     RPIProperties properties(mailbox);
30:
31:     uint64 serial{};
32:     if (properties.GetBoardSerial(serial))
33:     {
34:         LOG_INFO("Mailbox call succeeded");
35:         LOG_INFO("Serial: %016llx", serial);
36:     }
37:     else
38:     {
39:         LOG_ERROR("Mailbox call failed");
40:     }
41:
42:     LOG_INFO("Wait 5 seconds");
43:     Timer::WaitMilliSeconds(5000);
44:
45:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
46:     char ch{};
47:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
48:     {
49:         ch = console.ReadChar();
50:         console.WriteChar(ch);
51:     }
52:     if (ch == 'p')
53:     {
54:         SetAssertionCallback(MyHandler);
55:         assert(false);
56:         ResetAssertionCallback();
57:         assert(false);
58:     }
59:
60:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
61: }
```

- Line 2: We add the include for `Assert.h`
- Line 18-21: We define an assertion callback function `MyHandler()` which will simply write a log message
- Line 45: We print a different message, adding the input `p` to force a panic
- Line 52-58: If `p` was pressed, we perform a failed assertion.
First we set the handler for assertion to our own and fire the assertion.
Next we reset the handler to default and fire the assertion again

### Configuring, building and debugging {#TUTORIAL_13_LOGGER_ASSERTION___STEP_3_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

When pressing the `p` key, the application will assert and halt:

<img src="images/tutorial-13-assert.png" alt="Console output" width="800"/>

We can now start adding sanity checks that fail assertion if not successful.

Next: [14-board-information](14-board-information.md)
