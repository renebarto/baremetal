# Tutorial 16: String {#TUTORIAL_16_STRING}

@tableofcontents

## Tutorial setup {#TUTORIAL_16_STRING_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/16-string`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_16_STRING_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-16.a`
- a library `output/Debug/lib/stdlib-16.a`
- an application `output/Debug/bin/16-string.elf`
- an image in `deploy/Debug/16-string-image`

## Creating a string class {#TUTORIAL_16_STRING_CREATING_A_STRING_CLASS}

It would be nice if we could deal with strings in a more flexible way. For that, we need memory allocation, so we had to wait until now with that.

We will introduce a string class, and start using it for e.g. serialization, formatting, etc. It will largely be based on the `std::string` class.

### String.h {#TUTORIAL_16_STRING_CREATING_A_STRING_CLASS_STRINGH}

First we'll declare the `String` class.
This `String` class mimicks the `std::string` class in the standard C++ library quite closely, to make a later move to the standard C++ library easier.
This also means this is quite extensive class. As an exception, this class has a lower case name, and lower case methods, due to the mimicking.
The string will be insipred on the std::string class in C++.

Create the file `code/libraries/baremetal/include/baremetal/String.h`

```cpp
File: code/libraries/baremetal/include/baremetal/String.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : String.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : String
9: //
10: // Description : String handling
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
42: #include "baremetal/Iterator.h"
43: #include "stdlib/Types.h"
44: 
45: /// @file
46: /// String class
47: 
48: namespace baremetal {
49: 
50: /// <summary>
51: /// String class
52: /// </summary>
53: class String
54: {
55: public:
56:     /// @brief Type of value the string contains
57:     using ValueType = char;
58: 
59: private:
60:     /// @brief Pointer to start of allocated memory
61:     ValueType* m_buffer;
62:     /// @brief Pointer one past the end of the string
63:     ValueType* m_end;
64:     /// @brief Currently allocated size in bytes
65:     size_t m_allocatedSize;
66: 
67: public:
68:     /// @brief Signifies the position at the end of the string, e.g. for length
69:     static const size_t npos;
70: 
71:     String();
72:     String(const ValueType* str);
73:     String(const ValueType* str, size_t count);
74:     String(size_t count, ValueType ch);
75:     String(const String& other);
76:     String(String&& other);
77:     String(const String& other, size_t pos, size_t count = npos);
78:     ~String();
79: 
80:     operator const ValueType*() const;
81:     String& operator=(const ValueType* other);
82:     String& operator=(const String& other);
83:     String& operator=(String&& other);
84: 
85:     iterator<ValueType> begin();
86:     iterator<ValueType> end();
87:     const_iterator<ValueType> begin() const;
88:     const_iterator<ValueType> end() const;
89: 
90:     String& assign(const ValueType* str);
91:     String& assign(const ValueType* str, size_t count);
92:     String& assign(size_t count, ValueType ch);
93:     String& assign(const String& str);
94:     String& assign(const String& str, size_t pos, size_t count = npos);
95: 
96:     ValueType& at(size_t pos);
97:     const ValueType& at(size_t pos) const;
98:     ValueType& front();
99:     const ValueType& front() const;
100:     ValueType& back();
101:     const ValueType& back() const;
102:     ValueType& operator[](size_t pos);
103:     const ValueType& operator[](size_t pos) const;
104:     ValueType* data();
105:     const ValueType* data() const;
106:     const ValueType* c_str() const;
107: 
108:     bool empty() const;
109:     size_t size() const;
110:     size_t length() const;
111:     size_t capacity() const;
112:     size_t reserve(size_t newCapacity);
113: 
114:     String& operator+=(ValueType ch);
115:     String& operator+=(const String& str);
116:     String& operator+=(const ValueType* str);
117:     void append(size_t count, ValueType ch);
118:     void append(const String& str);
119:     void append(const String& str, size_t pos, size_t count = npos);
120:     void append(const ValueType* str);
121:     void append(const ValueType* str, size_t count);
122:     void clear();
123: 
124:     size_t find(const String& str, size_t pos = 0) const;
125:     size_t find(const ValueType* str, size_t pos = 0) const;
126:     size_t find(const ValueType* str, size_t pos, size_t count) const;
127:     size_t find(ValueType ch, size_t pos = 0) const;
128:     bool starts_with(ValueType ch) const;
129:     bool starts_with(const String& str) const;
130:     bool starts_with(const ValueType* str) const;
131:     bool ends_with(ValueType ch) const;
132:     bool ends_with(const String& str) const;
133:     bool ends_with(const ValueType* str) const;
134:     bool contains(ValueType ch) const;
135:     bool contains(const String& str) const;
136:     bool contains(const ValueType* str) const;
137:     String substr(size_t pos = 0, size_t count = npos) const;
138: 
139:     bool equals(const String& other) const;
140:     bool equals(const ValueType* other) const;
141:     bool equals_case_insensitive(const String& other) const;
142:     bool equals_case_insensitive(const ValueType* other) const;
143:     int compare(const String& str) const;
144:     int compare(size_t pos, size_t count, const String& str) const;
145:     int compare(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount = npos) const;
146:     int compare(const ValueType* str) const;
147:     int compare(size_t pos, size_t count, const ValueType* str) const;
148:     int compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const;
149: 
150:     String& replace(size_t pos, size_t count, const String& str);
151:     String& replace(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount = npos);
152:     String& replace(size_t pos, size_t count, const ValueType* str);
153:     String& replace(size_t pos, size_t count, const ValueType* str, size_t strCount);
154:     String& replace(size_t pos, size_t count, ValueType ch);
155:     String& replace(size_t pos, size_t count, ValueType ch, size_t chCount);
156:     int replace(const String& oldStr, const String& newStr);       // returns number of occurrences
157:     int replace(const ValueType* oldStr, const ValueType* newStr); // returns number of occurrences
158: 
159:     String align(int width) const;
160: 
161: private:
162:     bool reallocate(size_t requestedLength);
163:     bool reallocate_allocation_size(size_t allocationSize);
164: };
165: 
166: /// <summary>
167: /// Equality operator
168: ///
169: /// Performs a case sensitive comparison between two strings
170: /// </summary>
171: /// <param name="lhs">Left side of comparison</param>
172: /// <param name="rhs">Right side of comparison</param>
173: /// <returns>Returns true if strings are equal, false if not</returns>
174: inline bool operator==(const String& lhs, const String& rhs)
175: {
176:     return lhs.equals(rhs);
177: }
178: 
179: /// <summary>
180: /// Equality operator
181: ///
182: /// Performs a case sensitive comparison between two strings
183: /// </summary>
184: /// <param name="lhs">Left side of comparison</param>
185: /// <param name="rhs">Right side of comparison</param>
186: /// <returns>Returns true if strings are equal, false if not</returns>
187: inline bool operator==(const String& lhs, const String::ValueType* rhs)
188: {
189:     return lhs.equals(rhs);
190: }
191: 
192: /// <summary>
193: /// Equality operator
194: ///
195: /// Performs a case sensitive comparison between two strings
196: /// </summary>
197: /// <param name="lhs">Left side of comparison</param>
198: /// <param name="rhs">Right side of comparison</param>
199: /// <returns>Returns true if strings are equal, false if not</returns>
200: inline bool operator==(const String::ValueType* lhs, const String& rhs)
201: {
202:     return rhs.equals(lhs);
203: }
204: 
205: /// <summary>
206: /// Inequality operator
207: ///
208: /// Performs a case sensitive comparison between two strings
209: /// </summary>
210: /// <param name="lhs">Left side of comparison</param>
211: /// <param name="rhs">Right side of comparison</param>
212: /// <returns>Returns false if strings are equal, true if not</returns>
213: inline bool operator!=(const String& lhs, const String& rhs)
214: {
215:     return !lhs.equals(rhs);
216: }
217: 
218: /// <summary>
219: /// Inequality operator
220: ///
221: /// Performs a case sensitive comparison between two strings
222: /// </summary>
223: /// <param name="lhs">Left side of comparison</param>
224: /// <param name="rhs">Right side of comparison</param>
225: /// <returns>Returns false if strings are equal, true if not</returns>
226: inline bool operator!=(const String& lhs, const String::ValueType* rhs)
227: {
228:     return !lhs.equals(rhs);
229: }
230: 
231: /// <summary>
232: /// Inequality operator
233: ///
234: /// Performs a case sensitive comparison between two strings
235: /// </summary>
236: /// <param name="lhs">Left side of comparison</param>
237: /// <param name="rhs">Right side of comparison</param>
238: /// <returns>Returns false if strings are equal, true if not</returns>
239: inline bool operator!=(const String::ValueType* lhs, const String& rhs)
240: {
241:     return !rhs.equals(lhs);
242: }
243: 
244: /// <summary>
245: /// Add two strings
246: ///
247: /// Concatenates two strings and returns the result
248: /// </summary>
249: /// <param name="lhs">First part of the resulting string</param>
250: /// <param name="rhs">Second part of the resulting string</param>
251: /// <returns>Concatenation of first and second string</returns>
252: inline String operator+(const String& lhs, const String& rhs)
253: {
254:     String result = lhs;
255:     result.append(rhs);
256:     return result;
257: }
258: 
259: /// <summary>
260: /// Add two strings
261: ///
262: /// Concatenates two strings and returns the result
263: /// </summary>
264: /// <param name="lhs">First part of the resulting string</param>
265: /// <param name="rhs">Second part of the resulting string</param>
266: /// <returns>Concatenation of first and second string</returns>
267: inline String operator+(const String::ValueType* lhs, const String& rhs)
268: {
269:     String result{lhs};
270:     result.append(rhs);
271:     return result;
272: }
273: 
274: /// <summary>
275: /// Add two strings
276: ///
277: /// Concatenates two strings and returns the result
278: /// </summary>
279: /// <param name="lhs">First part of the resulting string</param>
280: /// <param name="rhs">Second part of the resulting string</param>
281: /// <returns>Concatenation of first and second string</returns>
282: inline String operator+(const String& lhs, const String::ValueType* rhs)
283: {
284:     String result = lhs;
285:     result.append(rhs);
286:     return result;
287: }
288: 
289: /// <summary>
290: /// Add character and string
291: ///
292: /// Concatenates the left hand character argument and the right hand string and returns the result
293: /// </summary>
294: /// <param name="lhs">First character in the resulting string</param>
295: /// <param name="rhs">Second part of the resulting string</param>
296: /// <returns>Concatenation of first and second part</returns>
297: inline String operator+(String::ValueType lhs, const String& rhs)
298: {
299:     String result;
300:     result += lhs;
301:     result += rhs;
302:     return result;
303: }
304: 
305: /// <summary>
306: /// Add string and character
307: ///
308: /// Concatenates the left hand string and the right hand character and returns the result
309: /// </summary>
310: /// <param name="lhs">First part of the resulting string</param>
311: /// <param name="rhs">Last character of the resulting string</param>
312: /// <returns>Concatenation of first and second part</returns>
313: inline String operator+(const String& lhs, String::ValueType rhs)
314: {
315:     String result;
316:     result += lhs;
317:     result += rhs;
318:     return result;
319: }
320: 
321: } // namespace baremetal
```

We'll not go into detail here, most methods should be clear, for the others you can visit the documentation of `std::string`.
There is a dependency on `Iterator.h` which we will add later.

### String.cpp {#TUTORIAL_16_STRING_CREATING_A_STRING_CLASS_STRINGCPP}

We'll implement the `String` class.

Create the file `code/libraries/baremetal/src/String.cpp`

```cpp
File: code/libraries/baremetal/src/String.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : String.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : String
9: //
10: // Description : String handling
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
40: #include "baremetal/String.h"
41: 
42: #include "baremetal/Assert.h"
43: #include "baremetal/Logger.h"
44: #include "baremetal/Malloc.h"
45: #include "stdlib/Util.h"
46: 
47: /// @file
48: /// String class implementation
49: 
50: using namespace baremetal;
51: 
52: /// @brief Minimum allocation size for any string
53: static constexpr size_t MinimumAllocationSize = 256;
54: 
55: /// @brief Maximum string size (largest 256Mb - 1 due to largest heap allocation block size)
56: static constexpr size_t MaximumStringSize = 0x80000 - 1;
57: 
58: const size_t String::npos = static_cast<size_t>(-1);
59: /// @brief Constant null character, using as string terminator, and also returned as a reference for const methods where nothing can be returned
60: static const String::ValueType NullCharConst = '\0';
61: /// @brief Non-constant null character, returned as a reference for const methods where nothing can be returned (always reinitialized before returning)
62: static String::ValueType NullChar = '\0';
63: 
64: /// @brief Define log name
65: LOG_MODULE("String");
66: 
67: /// <summary>
68: /// Default constructor
69: ///
70: /// Constructs an empty string.
71: /// </summary>
72: String::String()
73:     : m_buffer{}
74:     , m_end{}
75:     , m_allocatedSize{}
76: {
77: }
78: 
79: /// <summary>
80: /// Destructor
81: ///
82: /// Frees any allocated memory.
83: /// </summary>
84: String::~String()
85: {
86:     delete[] m_buffer;
87: }
88: 
89: /// <summary>
90: /// Constructor
91: ///
92: /// Initializes the string with the specified string.
93: /// </summary>
94: /// <param name="str">string to initialize with</param>
95: String::String(const ValueType* str)
96:     : m_buffer{}
97:     , m_end{}
98:     , m_allocatedSize{}
99: {
100:     if (str == nullptr)
101:         return;
102:     auto size = strlen(str);
103:     if (reallocate(size + 1))
104:     {
105:         strncpy(m_buffer, str, size);
106:     }
107:     m_end = m_buffer + size;
108:     m_buffer[size] = NullCharConst;
109: }
110: 
111: /// <summary>
112: /// Constructor
113: ///
114: /// Initializes the string with up to count characters in the specified string. A null character is always added.
115: /// </summary>
116: /// <param name="str">string to initialize with</param>
117: /// <param name="count">Maximum number of characters from str to initialize with. If count is larger than the actual string length, only the string length is used</param>
118: String::String(const ValueType* str, size_t count)
119:     : m_buffer{}
120:     , m_end{}
121:     , m_allocatedSize{}
122: {
123:     if (str == nullptr)
124:         return;
125:     auto size = strlen(str);
126:     if (count < size)
127:         size = count;
128:     if (reallocate(size + 1))
129:     {
130:         strncpy(m_buffer, str, size);
131:     }
132:     m_end = m_buffer + size;
133:     m_buffer[size] = NullCharConst;
134: }
135: 
136: /// <summary>
137: /// Constructor
138: ///
139: /// Initializes the string with the specified count times the specified character. A null character is always added.
140: /// </summary>
141: /// <param name="count">Number of characters of value ch to initialized with</param>
142: /// <param name="ch">Character to initialize with</param>
143: String::String(size_t count, ValueType ch)
144:     : m_buffer{}
145:     , m_end{}
146:     , m_allocatedSize{}
147: {
148:     auto size = count;
149:     if (size > MaximumStringSize)
150:         size = MaximumStringSize;
151:     if (reallocate(size + 1))
152:     {
153:         memset(m_buffer, ch, size);
154:     }
155:     m_end = m_buffer + size;
156:     m_buffer[size] = NullCharConst;
157: }
158: 
159: /// <summary>
160: /// Copy constructor
161: ///
162: /// Initializes the string with the specified string value.
163: /// </summary>
164: /// <param name="other">string to initialize with</param>
165: String::String(const String& other)
166:     : m_buffer{}
167:     , m_end{}
168:     , m_allocatedSize{}
169: {
170:     auto size = other.length();
171:     if (reallocate(size + 1))
172:     {
173:         strncpy(m_buffer, other.data(), size);
174:     }
175:     m_end = m_buffer + size;
176:     m_buffer[size] = NullCharConst;
177: }
178: 
179: /// <summary>
180: /// Move constructor
181: ///
182: /// Initializes the string by moving the contents from the specified string value.
183: /// </summary>
184: /// <param name="other">string to initialize with</param>
185: String::String(String&& other)
186:     : m_buffer{other.m_buffer}
187:     , m_end{other.m_end}
188:     , m_allocatedSize{other.m_allocatedSize}
189: {
190:     other.m_buffer = nullptr;
191:     other.m_end = nullptr;
192:     other.m_allocatedSize = 0;
193: }
194: 
195: /// <summary>
196: /// Constructor
197: ///
198: /// Initializes the string with the substring starting at specified position, for the specified number of characters, from the specified string value.
199: /// </summary>
200: /// <param name="other">string to initialize with</param>
201: /// <param name="pos">Position in other to start copying charaters from</param>
202: /// <param name="count">Maximum number of characters to copy from other. Default is until end of string. If pos + count is larger than the actual length of the string, string other is copied until the
203: /// end</param>
204: String::String(const String& other, size_t pos, size_t count /*= npos*/)
205:     : m_buffer{}
206:     , m_end{}
207:     , m_allocatedSize{}
208: {
209:     if (pos >= other.length())
210:         return;
211:     auto size = other.length() - pos;
212:     if (count < size)
213:         size = count;
214:     if (reallocate(size + 1))
215:     {
216:         strncpy(m_buffer, other.data() + pos, size);
217:     }
218:     m_end = m_buffer + size;
219:     m_buffer[size] = NullCharConst;
220: }
221: 
222: /// <summary>
223: /// Const character cast operator
224: ///
225: /// Returns the pointer to the start of the string.
226: /// </summary>
227: String::operator const ValueType*() const
228: {
229:     return data();
230: }
231: 
232: /// <summary>
233: /// Assignment operator
234: ///
235: /// Assigns the specified string value to the string.
236: /// </summary>
237: /// <param name="str">string value to assign to the string</param>
238: /// <returns>A reference to the string</returns>
239: String& String::operator=(const ValueType* str)
240: {
241:     return assign(str);
242: }
243: 
244: /// <summary>
245: /// Assignment operator
246: ///
247: /// Assigns the specified string value to the string.
248: /// </summary>
249: /// <param name="str">string value to assign to the string</param>
250: /// <returns>A reference to the string</returns>
251: String& String::operator=(const String& str)
252: {
253:     return assign(str);
254: }
255: 
256: /// <summary>
257: /// Move operator
258: ///
259: /// Assigns the specified string value to the string by moving the contents of the specified string.
260: /// </summary>
261: /// <param name="str">string value to assign to the string</param>
262: /// <returns>A reference to the string</returns>
263: String& String::operator=(String&& str)
264: {
265:     if (&str != this)
266:     {
267:         m_buffer = str.m_buffer;
268:         m_end = str.m_end;
269:         m_allocatedSize = str.m_allocatedSize;
270:         str.m_buffer = nullptr;
271:         str.m_end = nullptr;
272:         str.m_allocatedSize = 0;
273:     }
274:     return *this;
275: }
276: 
277: /// <summary>
278: /// Non-const iterator to the start of the string
279: ///
280: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
281: /// </summary>
282: /// <returns>iterator to the value type, acting as the start of the string</returns>
283: iterator<String::ValueType> String::begin()
284: {
285:     return iterator(m_buffer, m_end);
286: }
287: 
288: /// <summary>
289: /// Non-const iterator to the end of the string + 1
290: ///
291: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
292: /// </summary>
293: /// <returns>iterator to the value type, acting as the end of the string</returns>
294: iterator<String::ValueType> String::end()
295: {
296:     return iterator(m_end, m_end);
297: }
298: 
299: /// <summary>
300: /// Const iterator to the start of the string
301: ///
302: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
303: /// </summary>
304: /// <returns>const_iterator to the value type, acting as the start of the string</returns>
305: const_iterator<String::ValueType> String::begin() const
306: {
307:     return const_iterator(m_buffer, m_end);
308: }
309: 
310: /// <summary>
311: /// Const iterator to the end of the string + 1
312: ///
313: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
314: /// </summary>
315: /// <returns>const_iterator to the value type, acting as the end of the string</returns>
316: const_iterator<String::ValueType> String::end() const
317: {
318:     return const_iterator(m_end, m_end);
319: }
320: 
321: /// <summary>
322: /// assign a string value
323: ///
324: /// Assigns the specified string value to the string
325: /// </summary>
326: /// <param name="str">string value to assign to the string</param>
327: /// <returns>A reference to the string</returns>
328: String& String::assign(const ValueType* str)
329: {
330:     size_t size{};
331:     if (str != nullptr)
332:     {
333:         size = strlen(str);
334:     }
335:     if ((size + 1) > m_allocatedSize)
336:     {
337:         if (!reallocate(size + 1))
338:             return *this;
339:     }
340:     if (str != nullptr)
341:     {
342:         strncpy(m_buffer, str, size);
343:     }
344:     m_end = m_buffer + size;
345:     m_buffer[size] = NullCharConst;
346:     return *this;
347: }
348: 
349: /// <summary>
350: /// assign a string value
351: ///
352: /// Assigns the specified string value, up to the specified count of characters, to the string.
353: /// </summary>
354: /// <param name="str">string value to assign to the string</param>
355: /// <param name="count">Maximum number of characters to copy from the string. If count is larger than the string length, the length of the string is used</param>
356: /// <returns>A reference to the string</returns>
357: String& String::assign(const ValueType* str, size_t count)
358: {
359:     size_t size{};
360:     if (str != nullptr)
361:     {
362:         size = strlen(str);
363:     }
364:     if (count < size)
365:         size = count;
366:     if ((size + 1) > m_allocatedSize)
367:     {
368:         if (!reallocate(size + 1))
369:             return *this;
370:     }
371:     if (str != nullptr)
372:     {
373:         strncpy(m_buffer, str, size);
374:     }
375:     m_end = m_buffer + size;
376:     m_buffer[size] = NullCharConst;
377:     return *this;
378: }
379: 
380: /// <summary>
381: /// assign a string value
382: ///
383: /// Assigns a string containing the specified count times the specified characters to the string
384: /// </summary>
385: /// <param name="count">Number copies of ch to copy to the string</param>
386: /// <param name="ch">Character to initialize with</param>
387: /// <returns>A reference to the string</returns>
388: String& String::assign(size_t count, ValueType ch)
389: {
390:     auto size = count;
391:     if (size > MaximumStringSize)
392:         size = MaximumStringSize;
393:     if ((size + 1) > m_allocatedSize)
394:     {
395:         if (!reallocate(size + 1))
396:             return *this;
397:     }
398:     memset(m_buffer, ch, size);
399:     m_end = m_buffer + size;
400:     m_buffer[size] = NullCharConst;
401:     return *this;
402: }
403: 
404: /// <summary>
405: /// assign a string value
406: ///
407: /// Assigns the specified string value to the string
408: /// </summary>
409: /// <param name="str">string value to assign to the string</param>
410: /// <returns>A reference to the string</returns>
411: String& String::assign(const String& str)
412: {
413:     auto size = str.length();
414:     if ((size + 1) > m_allocatedSize)
415:     {
416:         if (!reallocate(size + 1))
417:             return *this;
418:     }
419:     strncpy(m_buffer, str.data(), size);
420:     m_end = m_buffer + size;
421:     m_buffer[size] = NullCharConst;
422:     return *this;
423: }
424: 
425: /// <summary>
426: /// assign a string value
427: ///
428: /// Assigns the substring start from the specified position for the specified count of characters of specified string value to the string
429: /// </summary>
430: /// <param name="str">string value to assign to the string</param>
431: /// <param name="pos">Starting position of substring to copy from str</param>
432: /// <param name="count">Maximum number of characters to copy from str.
433: /// Default is until end of string. If pos + count is larger than the stirn length, characters are copied until end of string</param>
434: /// <returns>A reference to the string</returns>
435: String& String::assign(const String& str, size_t pos, size_t count /*= npos*/)
436: {
437:     if (str.empty())
438:         return assign(str);
439: 
440:     if (pos < str.length())
441:     {
442:         auto size = str.length() - pos;
443:         if (count < size)
444:             size = count;
445:         if ((size + 1) > m_allocatedSize)
446:         {
447:             if (!reallocate(size + 1))
448:                 return *this;
449:         }
450:         strncpy(m_buffer, str.data() + pos, size);
451:         m_end = m_buffer + size;
452:         m_buffer[size] = NullCharConst;
453:     }
454:     return *this;
455: }
456: 
457: /// <summary>
458: /// Return the character at specified position
459: /// </summary>
460: /// <param name="pos">Position in string</param>
461: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, a reference to a non-const null character is returned (NullChar, is reinitialized
462: /// before returning)</returns>
463: String::ValueType& String::at(size_t pos)
464: {
465:     if (pos >= length())
466:     {
467:         NullChar = NullCharConst;
468:         return NullChar;
469:     }
470:     return m_buffer[pos];
471: }
472: 
473: /// <summary>
474: /// Return the character at specified position
475: /// </summary>
476: /// <param name="pos">Position in string</param>
477: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, a reference to a const null character is returned (NullCharConst)</returns>
478: const String::ValueType& String::at(size_t pos) const
479: {
480:     if (pos >= length())
481:         return NullCharConst;
482:     return m_buffer[pos];
483: }
484: 
485: /// <summary>
486: /// Return the first character
487: /// </summary>
488: /// <returns>Returns a non-const reference to the first character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before
489: /// returning)</returns>
490: String::ValueType& String::front()
491: {
492:     if (empty())
493:     {
494:         NullChar = NullCharConst;
495:         return NullChar;
496:     }
497:     return *m_buffer;
498: }
499: 
500: /// <summary>
501: /// Return the first character
502: /// </summary>
503: /// <returns>Returns a const reference to the first character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
504: const String::ValueType& String::front() const
505: {
506:     if (empty())
507:         return NullCharConst;
508:     return *m_buffer;
509: }
510: 
511: /// <summary>
512: /// Return the last character
513: /// </summary>
514: /// <returns>Returns a non-const reference to the last character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before
515: /// returning)</returns>
516: String::ValueType& String::back()
517: {
518:     if (empty())
519:     {
520:         NullChar = NullCharConst;
521:         return NullChar;
522:     }
523:     return *(m_end - 1);
524: }
525: 
526: /// <summary>
527: /// Return the last character
528: /// </summary>
529: /// <returns>Returns a const reference to the last character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
530: const String::ValueType& String::back() const
531: {
532:     if (empty())
533:         return NullCharConst;
534:     return *(m_end - 1);
535: }
536: 
537: /// <summary>
538: /// Return the character at specified position
539: /// </summary>
540: /// <param name="pos">Position in string</param>
541: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
542: String::ValueType& String::operator[](size_t pos)
543: {
544:     if (pos >= size())
545:     {
546:         NullChar = '\0';
547:         return NullChar;
548:     }
549:     return m_buffer[pos];
550: }
551: 
552: /// <summary>
553: /// Return the character at specified position
554: /// </summary>
555: /// <param name="pos">Position in string</param>
556: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
557: const String::ValueType& String::operator[](size_t pos) const
558: {
559:     if (pos >= size())
560:         return NullCharConst;
561:     return m_buffer[pos];
562: }
563: 
564: /// <summary>
565: /// Return the buffer pointer
566: /// </summary>
567: /// <returns>Returns a non-const pointer to the buffer. If the buffer is not allocated, a pointer to a non-const null character (NullChar, initialized before returning) is returned</returns>
568: String::ValueType* String::data()
569: {
570:     NullChar = NullCharConst;
571:     return (m_buffer == nullptr) ? &NullChar : m_buffer;
572: }
573: 
574: /// <summary>
575: /// Return the buffer pointer
576: /// </summary>
577: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
578: const String::ValueType* String::data() const
579: {
580:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
581: }
582: 
583: /// <summary>
584: /// Return the buffer pointer
585: /// </summary>
586: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
587: const String::ValueType* String::c_str() const
588: {
589:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
590: }
591: 
592: /// <summary>
593: /// Determine whether string is empty.
594: /// </summary>
595: /// <returns>Returns true when the string is empty (not allocated or no contents), false otherwise</returns>
596: bool String::empty() const
597: {
598:     return m_end == m_buffer;
599: }
600: 
601: /// <summary>
602: /// Return the size of the string
603: ///
604: /// This method is the equivalent of length().
605: /// </summary>
606: /// <returns>Returns the size (or length) of the string</returns>
607: size_t String::size() const
608: {
609:     return m_end - m_buffer;
610: }
611: 
612: /// <summary>
613: /// Return the length of the string
614: ///
615: /// This method is the equivalent of size().
616: /// </summary>
617: /// <returns>Returns the size (or length) of the string</returns>
618: size_t String::length() const
619: {
620:     return m_end - m_buffer;
621: }
622: 
623: /// <summary>
624: /// Return the capacity of the string
625: ///
626: /// The capacity is the size of the allocated buffer. The string can grow to that length before it needs to be re-allocated.
627: /// </summary>
628: /// <returns>Returns the size (or length) of the string</returns>
629: size_t String::capacity() const
630: {
631:     return m_allocatedSize;
632: }
633: 
634: /// <summary>
635: /// Reserved a buffer capacity
636: ///
637: /// Allocates a buffer of specified size
638: /// </summary>
639: /// <param name="newCapacity"></param>
640: /// <returns>Returns the capacity of the string</returns>
641: size_t String::reserve(size_t newCapacity)
642: {
643:     reallocate_allocation_size(newCapacity);
644:     return m_allocatedSize;
645: }
646: 
647: /// <summary>
648: /// append operator
649: ///
650: /// Appends a character to the string
651: /// </summary>
652: /// <param name="ch">Character to append</param>
653: /// <returns>Returns a reference to the string</returns>
654: String& String::operator+=(ValueType ch)
655: {
656:     append(1, ch);
657:     return *this;
658: }
659: 
660: /// <summary>
661: /// append operator
662: ///
663: /// Appends a string to the string
664: /// </summary>
665: /// <param name="str">string to append</param>
666: /// <returns>Returns a reference to the string</returns>
667: String& String::operator+=(const String& str)
668: {
669:     append(str);
670:     return *this;
671: }
672: 
673: /// <summary>
674: /// append operator
675: ///
676: /// Appends a string to the string
677: /// </summary>
678: /// <param name="str">string to append. If nullptr the nothing is appended</param>
679: /// <returns>Returns a reference to the string</returns>
680: String& String::operator+=(const ValueType* str)
681: {
682:     append(str);
683:     return *this;
684: }
685: 
686: /// <summary>
687: /// append operator
688: ///
689: /// Appends a sequence of count times the same character ch to the string
690: /// </summary>
691: /// <param name="count">Number of characters to append</param>
692: /// <param name="ch">Character to append</param>
693: void String::append(size_t count, ValueType ch)
694: {
695:     auto len = length();
696:     auto strLength = count;
697:     if (strLength > MaximumStringSize - len)
698:         strLength = MaximumStringSize - len;
699:     auto size = len + strLength;
700:     if ((size + 1) > m_allocatedSize)
701:     {
702:         if (!reallocate(size + 1))
703:             return;
704:     }
705:     memset(m_buffer + len, ch, strLength);
706:     m_end = m_buffer + size;
707:     m_buffer[size] = NullCharConst;
708: }
709: 
710: /// <summary>
711: /// append operator
712: ///
713: /// Appends a string to the string
714: /// </summary>
715: /// <param name="str">string to append</param>
716: void String::append(const String& str)
717: {
718:     auto len = length();
719:     auto strLength = str.length();
720:     auto size = len + strLength;
721:     if ((size + 1) > m_allocatedSize)
722:     {
723:         if (!reallocate(size + 1))
724:             return;
725:     }
726:     strncpy(m_buffer + len, str.data(), strLength);
727:     m_end = m_buffer + size;
728:     m_buffer[size] = NullCharConst;
729: }
730: 
731: /// <summary>
732: /// append operator
733: ///
734: /// Appends a substring of str to the string
735: /// </summary>
736: /// <param name="str">string to append from</param>
737: /// <param name="pos">Start position in str to copy characters from</param>
738: /// <param name="count">Number of characters to copy from str. Default is until the end of the string. If count is larger than the string length, characters are copied up to the end of the
739: /// string</param>
740: void String::append(const String& str, size_t pos, size_t count /*= npos*/)
741: {
742:     if (pos >= str.length())
743:         return;
744:     auto strLength = str.length();
745:     auto strCount = strLength - pos;
746:     if (count < strCount)
747:         strCount = count;
748:     auto len = length();
749:     auto size = len + strCount;
750:     if ((size + 1) > m_allocatedSize)
751:     {
752:         if (!reallocate(size + 1))
753:             return;
754:     }
755:     strncpy(m_buffer + len, str.data() + pos, strCount);
756:     m_end = m_buffer + size;
757:     m_buffer[size] = NullCharConst;
758: }
759: 
760: /// <summary>
761: /// append operator
762: ///
763: /// Appends a string to the string
764: /// </summary>
765: /// <param name="str">string to append. If nullptr the nothing is appended</param>
766: void String::append(const ValueType* str)
767: {
768:     if (str == nullptr)
769:         return;
770:     auto len = length();
771:     auto strLength = strlen(str);
772:     auto size = len + strLength;
773:     if ((size + 1) > m_allocatedSize)
774:     {
775:         if (!reallocate(size + 1))
776:             return;
777:     }
778:     strncpy(m_buffer + len, str, strLength);
779:     m_end = m_buffer + size;
780:     m_buffer[size] = NullCharConst;
781: }
782: 
783: /// <summary>
784: /// append operator
785: ///
786: /// Appends a number of characters from str to the string
787: /// </summary>
788: /// <param name="str">string to append. If nullptr the nothing is appended</param>
789: /// <param name="count">Number of characters to copy from str. If count is larger than the string length, the complete string is copied</param>
790: void String::append(const ValueType* str, size_t count)
791: {
792:     if (str == nullptr)
793:         return;
794:     auto len = length();
795:     auto strLength = strlen(str);
796:     auto strCount = count;
797:     if (strCount > strLength)
798:         strCount = strLength;
799:     auto size = len + strCount;
800:     if ((size + 1) > m_allocatedSize)
801:     {
802:         if (!reallocate(size + 1))
803:             return;
804:     }
805:     strncpy(m_buffer + len, str, strCount);
806:     m_end = m_buffer + size;
807:     m_buffer[size] = NullCharConst;
808: }
809: 
810: /// <summary>
811: /// clear the string
812: ///
813: /// Clears the contents of the string, but does not free or reallocate the buffer
814: /// </summary>
815: void String::clear()
816: {
817:     if (!empty())
818:     {
819:         m_end = m_buffer;
820:         m_buffer[0] = NullCharConst;
821:     }
822: }
823: 
824: /// <summary>
825: /// find a substring in the string
826: ///
827: /// If empty string, always finds the string.
828: /// </summary>
829: /// <param name="str">Substring to find</param>
830: /// <param name="pos">Starting position in string to start searching</param>
831: /// <returns>Location of first character in string of match if found, String::npos if not found</returns>
832: size_t String::find(const String& str, size_t pos /*= 0*/) const
833: {
834:     auto len = length();
835:     auto patternLength = str.length();
836:     if (pos >= len)
837:         return npos;
838:     auto needle = str.data();
839:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
840:     {
841:         if (memcmp(haystack, needle, patternLength) == 0)
842:             return haystack - m_buffer;
843:     }
844:     return npos;
845: }
846: 
847: /// <summary>
848: /// find a substring in the string
849: ///
850: /// If nullptr or empty string, always finds the string.
851: /// </summary>
852: /// <param name="str">Substring to find</param>
853: /// <param name="pos">Starting position in string to start searching</param>
854: /// <returns>Location of first character in string of match if found, String::npos if not found</returns>
855: size_t String::find(const ValueType* str, size_t pos /*= 0*/) const
856: {
857:     size_t strLength{};
858:     if (str != nullptr)
859:     {
860:         strLength = strlen(str);
861:     }
862:     auto len = length();
863:     auto patternLength = strLength;
864:     if (pos >= len)
865:         return npos;
866:     auto needle = str;
867:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
868:     {
869:         if (memcmp(haystack, needle, patternLength) == 0)
870:             return haystack - m_buffer;
871:     }
872:     return npos;
873: }
874: 
875: /// <summary>
876: /// find a substring in the string
877: ///
878: /// If nullptr or empty string, always finds the string.
879: /// </summary>
880: /// <param name="str">Substring to find</param>
881: /// <param name="pos">Starting position in string to start searching</param>
882: /// <param name="count">Number of characters from str to compare</param>
883: /// <returns>Location of first character in string of match if found, String::npos if not found</returns>
884: size_t String::find(const ValueType* str, size_t pos, size_t count) const
885: {
886:     size_t strLength{};
887:     if (str != nullptr)
888:     {
889:         strLength = strlen(str);
890:     }
891:     auto len = length();
892:     auto patternLength = strLength;
893:     if (pos >= len)
894:         return npos;
895:     if (count < patternLength)
896:         patternLength = count;
897:     auto needle = str;
898:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
899:     {
900:         if (memcmp(haystack, needle, patternLength) == 0)
901:             return haystack - m_buffer;
902:     }
903:     return npos;
904: }
905: 
906: /// <summary>
907: /// find a character in the string
908: /// </summary>
909: /// <param name="ch">Character to find</param>
910: /// <param name="pos">Starting position in string to start searching</param>
911: /// <returns>Location of first character in string of match if found, String::npos if not found</returns>
912: size_t String::find(ValueType ch, size_t pos /*= 0*/) const
913: {
914:     auto len = length();
915:     if (pos >= len)
916:         return npos;
917:     for (const ValueType* haystack = data() + pos; haystack <= m_end; ++haystack)
918:     {
919:         if (*haystack == ch)
920:             return haystack - m_buffer;
921:     }
922:     return npos;
923: }
924: 
925: /// <summary>
926: /// Check whether string starts with character
927: /// </summary>
928: /// <param name="ch">Character to find</param>
929: /// <returns>Returns true if ch is first character in string, false otherwise</returns>
930: bool String::starts_with(ValueType ch) const
931: {
932:     if (empty())
933:         return false;
934:     return m_buffer[0] == ch;
935: }
936: 
937: /// <summary>
938: /// Check whether string starts with substring
939: /// </summary>
940: /// <param name="str">SubString to find</param>
941: /// <returns>Returns true if str is first part of string, false otherwise</returns>
942: bool String::starts_with(const String& str) const
943: {
944:     auto len = length();
945:     auto strLength = str.length();
946:     if (strLength >= len)
947:         return false;
948: 
949:     return memcmp(data(), str.data(), strLength) == 0;
950: }
951: 
952: /// <summary>
953: /// Check whether string starts with substring
954: /// </summary>
955: /// <param name="str">SubString to find</param>
956: /// <returns>Returns true if str is first part of string, false otherwise</returns>
957: bool String::starts_with(const ValueType* str) const
958: {
959:     size_t strLength{};
960:     if (str != nullptr)
961:     {
962:         strLength = strlen(str);
963:     }
964:     auto len = length();
965:     if (strLength >= len)
966:         return false;
967: 
968:     return memcmp(data(), str, strLength) == 0;
969: }
970: 
971: /// <summary>
972: /// Check whether string ends with character
973: /// </summary>
974: /// <param name="ch">Character to find</param>
975: /// <returns>Returns true if ch is last character in string, false otherwise</returns>
976: bool String::ends_with(ValueType ch) const
977: {
978:     if (empty())
979:         return false;
980:     return m_buffer[length() - 1] == ch;
981: }
982: 
983: /// <summary>
984: /// Check whether string ends with substring
985: /// </summary>
986: /// <param name="str">SubString to find</param>
987: /// <returns>Returns true if str is last part of string, false otherwise</returns>
988: bool String::ends_with(const String& str) const
989: {
990:     auto len = length();
991:     auto strLength = str.length();
992:     if (strLength >= len)
993:         return false;
994: 
995:     return memcmp(m_end - strLength, str.data(), strLength) == 0;
996: }
997: 
998: /// <summary>
999: /// Check whether string ends with substring
1000: /// </summary>
1001: /// <param name="str">SubString to find</param>
1002: /// <returns>Returns true if str is last part of string, false otherwise</returns>
1003: bool String::ends_with(const ValueType* str) const
1004: {
1005:     size_t strLength{};
1006:     if (str != nullptr)
1007:     {
1008:         strLength = strlen(str);
1009:     }
1010:     auto len = length();
1011:     if (strLength >= len)
1012:         return false;
1013: 
1014:     return memcmp(m_end - strLength, str, strLength) == 0;
1015: }
1016: 
1017: /// <summary>
1018: /// Check whether string contains character
1019: /// </summary>
1020: /// <param name="ch">Character to find</param>
1021: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1022: bool String::contains(ValueType ch) const
1023: {
1024:     return find(ch) != npos;
1025: }
1026: 
1027: /// <summary>
1028: /// Check whether string contains substring
1029: /// </summary>
1030: /// <param name="str">Substring to find</param>
1031: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1032: bool String::contains(const String& str) const
1033: {
1034:     return find(str) != npos;
1035: }
1036: 
1037: /// <summary>
1038: /// Check whether string contains substring
1039: /// </summary>
1040: /// <param name="str">Substring to find</param>
1041: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1042: bool String::contains(const ValueType* str) const
1043: {
1044:     return find(str) != npos;
1045: }
1046: 
1047: /// <summary>
1048: /// Return substring
1049: /// </summary>
1050: /// <param name="pos">Starting position of substring in string</param>
1051: /// <param name="count">length of substring to return. If count is larger than the number of characters available from position pos, the rest of the string is returned</param>
1052: /// <returns>Returns the substring at position [pos, pos + count), if available </returns>
1053: String String::substr(size_t pos /*= 0*/, size_t count /*= npos*/) const
1054: {
1055:     String result;
1056:     auto size = length() - pos;
1057:     if (pos < length())
1058:     {
1059:         if (count < size)
1060:             size = count;
1061:         result.reallocate(size + 1);
1062:         memcpy(result.data(), data() + pos, size);
1063:         result.m_end = result.m_buffer + size;
1064:         result.data()[size] = NullCharConst;
1065:     }
1066: 
1067:     return result;
1068: }
1069: 
1070: /// <summary>
1071: /// Case sensitive equality to string
1072: /// </summary>
1073: /// <param name="other">string to compare to</param>
1074: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1075: bool String::equals(const String& other) const
1076: {
1077:     return compare(other) == 0;
1078: }
1079: 
1080: /// <summary>
1081: /// Case sensitive equality to string
1082: /// </summary>
1083: /// <param name="other">string to compare to</param>
1084: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1085: bool String::equals(const ValueType* other) const
1086: {
1087:     if (other == nullptr)
1088:         return empty();
1089:     if (length() != strlen(other))
1090:         return false;
1091:     return strcmp(data(), other) == 0;
1092: }
1093: 
1094: /// <summary>
1095: /// Case insensitive equality to string
1096: /// </summary>
1097: /// <param name="other">string to compare to</param>
1098: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1099: bool String::equals_case_insensitive(const String& other) const
1100: {
1101:     if (length() != other.length())
1102:         return false;
1103:     if (empty())
1104:         return true;
1105:     return strcasecmp(data(), other.data()) == 0;
1106: }
1107: 
1108: /// <summary>
1109: /// Case insensitive equality to string
1110: /// </summary>
1111: /// <param name="other">string to compare to</param>
1112: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1113: bool String::equals_case_insensitive(const ValueType* other) const
1114: {
1115:     if (other == nullptr)
1116:         return empty();
1117:     if (length() != strlen(other))
1118:         return false;
1119:     return strcasecmp(data(), other) == 0;
1120: }
1121: 
1122: /// <summary>
1123: /// Case sensitive compare to string
1124: ///
1125: /// Compares the complete string, character by character
1126: /// </summary>
1127: /// <param name="str">string to compare to</param>
1128: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1129: int String::compare(const String& str) const
1130: {
1131:     if (empty())
1132:     {
1133:         if (str.empty())
1134:             return 0;
1135:         return -1;
1136:     }
1137:     if (str.empty())
1138:         return 1;
1139: 
1140:     return strcmp(data(), str.data());
1141: }
1142: 
1143: /// <summary>
1144: /// Case sensitive compare to string
1145: ///
1146: /// Compares the substring from pos to pos+count to str
1147: /// </summary>
1148: /// <param name="pos">Starting position of substring to compare to str</param>
1149: /// <param name="count">Number of characters in substring to compare to str</param>
1150: /// <param name="str">string to compare to</param>
1151: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1152: int String::compare(size_t pos, size_t count, const String& str) const
1153: {
1154:     return substr(pos, count).compare(str);
1155: }
1156: 
1157: /// <summary>
1158: /// Case sensitive compare to string
1159: ///
1160: /// Compares the substring from pos to pos+count to the substring from strPos to strPos+strCount of str
1161: /// </summary>
1162: /// <param name="pos">Starting position of substring to compare to str</param>
1163: /// <param name="count">Number of characters in substring to compare to str</param>
1164: /// <param name="str">string to compare to</param>
1165: /// <param name="strPos">Starting position of substring of str to compare</param>
1166: /// <param name="strCount">Number of characters in substring of str to compare</param>
1167: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1168: int String::compare(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount /*= npos*/) const
1169: {
1170:     return substr(pos, count).compare(str.substr(strPos, strCount));
1171: }
1172: 
1173: /// <summary>
1174: /// Case sensitive compare to string
1175: ///
1176: /// Compares the complete string to str
1177: /// </summary>
1178: /// <param name="str">string to compare to</param>
1179: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1180: int String::compare(const ValueType* str) const
1181: {
1182:     size_t strLength{};
1183:     if (str != nullptr)
1184:         strLength = strlen(str);
1185:     if (empty())
1186:     {
1187:         if (strLength == 0)
1188:             return 0;
1189:         return -1;
1190:     }
1191:     if (strLength == 0)
1192:         return 1;
1193: 
1194:     return strcmp(data(), str);
1195: }
1196: 
1197: /// <summary>
1198: /// Case sensitive compare to string
1199: ///
1200: /// Compares the substring from pos to pos+count to str
1201: /// </summary>
1202: /// <param name="pos">Starting position of substring to compare to str</param>
1203: /// <param name="count">Number of characters in substring to compare to str</param>
1204: /// <param name="str">string to compare to</param>
1205: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1206: int String::compare(size_t pos, size_t count, const ValueType* str) const
1207: {
1208:     size_t strLength{};
1209:     if (str != nullptr)
1210:         strLength = strlen(str);
1211: 
1212:     auto len = length();
1213:     if (pos >= len)
1214:         len = 0;
1215:     len -= pos;
1216:     if (count < len)
1217:         len = count;
1218:     if (len == 0)
1219:     {
1220:         if (strLength == 0)
1221:             return 0;
1222:         return -1;
1223:     }
1224:     if (strLength == 0)
1225:         return 1;
1226: 
1227:     auto maxLen = strLength;
1228:     if (maxLen < len)
1229:         maxLen = len;
1230:     return strncmp(data() + pos, str, maxLen);
1231: }
1232: 
1233: /// <summary>
1234: /// Case sensitive compare to string
1235: ///
1236: /// Compares the substring from pos to pos+count to the first strCount characters of str
1237: /// </summary>
1238: /// <param name="pos">Starting position of substring to compare to str</param>
1239: /// <param name="count">Number of characters in substring to compare to str</param>
1240: /// <param name="str">string to compare to</param>
1241: /// <param name="strCount">Number of characters in substring of str to compare</param>
1242: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1243: int String::compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const
1244: {
1245:     size_t strLength{};
1246:     if (str != nullptr)
1247:         strLength = strlen(str);
1248: 
1249:     auto len = length();
1250:     if (pos >= len)
1251:         len = 0;
1252:     len -= pos;
1253:     if (count < len)
1254:         len = count;
1255: 
1256:     if (strCount < strLength)
1257:         strLength = strCount;
1258: 
1259:     if (len == 0)
1260:     {
1261:         if (strLength == 0)
1262:             return 0;
1263:         return -1;
1264:     }
1265:     if (strLength == 0)
1266:         return 1;
1267: 
1268:     auto maxLen = strLength;
1269:     if (maxLen < len)
1270:         maxLen = len;
1271:     return strncmp(data() + pos, str, maxLen);
1272: }
1273: 
1274: /// <summary>
1275: /// replace substring
1276: ///
1277: /// Replaces the substring from pos to pos+count with str
1278: /// </summary>
1279: /// <param name="pos">Starting position of substring to replace</param>
1280: /// <param name="count">Number of characters in substring to replace</param>
1281: /// <param name="str">string to replace with</param>
1282: /// <returns>Returns the reference to the resulting string</returns>
1283: String& String::replace(size_t pos, size_t count, const String& str)
1284: {
1285:     String result = substr(0, pos) + str + substr(pos + count);
1286:     assign(result);
1287:     return *this;
1288: }
1289: 
1290: /// <summary>
1291: /// replace substring
1292: ///
1293: /// Replaces the substring from pos to pos+count with the substring from strPos to strPos+strCount of str
1294: /// </summary>
1295: /// <param name="pos">Starting position of substring to replace</param>
1296: /// <param name="count">Number of characters in substring to replace</param>
1297: /// <param name="str">string to replace with</param>
1298: /// <param name="strPos">Starting position of substring of str to replace with</param>
1299: /// <param name="strCount">Number of characters in substring of str to replace with</param>
1300: /// <returns>Returns the reference to the resulting string</returns>
1301: String& String::replace(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount /*= npos*/)
1302: {
1303:     String result = substr(0, pos) + str.substr(strPos, strCount) + substr(pos + count);
1304:     assign(result);
1305:     return *this;
1306: }
1307: 
1308: /// <summary>
1309: /// replace substring
1310: ///
1311: /// Replaces the substring from pos to pos+count with str
1312: /// </summary>
1313: /// <param name="pos">Starting position of substring to replace</param>
1314: /// <param name="count">Number of characters in substring to replace</param>
1315: /// <param name="str">string to replace with</param>
1316: /// <returns>Returns the reference to the resulting string</returns>
1317: String& String::replace(size_t pos, size_t count, const ValueType* str)
1318: {
1319:     String result = substr(0, pos) + str + substr(pos + count);
1320:     assign(result);
1321:     return *this;
1322: }
1323: 
1324: /// <summary>
1325: /// replace substring
1326: ///
1327: /// Replaces the substring from pos to pos+count with the first strCount characters of str
1328: /// </summary>
1329: /// <param name="pos">Starting position of substring to replace</param>
1330: /// <param name="count">Number of characters in substring to replace</param>
1331: /// <param name="str">string to replace with</param>
1332: /// <param name="strCount">Number of characters in substring to replace with</param>
1333: /// <returns>Returns the reference to the resulting string</returns>
1334: String& String::replace(size_t pos, size_t count, const ValueType* str, size_t strCount)
1335: {
1336:     String result = substr(0, pos) + String(str, strCount) + substr(pos + count);
1337:     assign(result);
1338:     return *this;
1339: }
1340: 
1341: /// <summary>
1342: /// replace substring
1343: ///
1344: /// Replaces the substring from pos to pos+count with ch
1345: /// </summary>
1346: /// <param name="pos">Starting position of substring to replace</param>
1347: /// <param name="count">Number of characters in substring to replace</param>
1348: /// <param name="ch">Characters to replace with</param>
1349: /// <returns>Returns the reference to the resulting string</returns>
1350: String& String::replace(size_t pos, size_t count, ValueType ch)
1351: {
1352:     return replace(pos, count, ch, 1);
1353: }
1354: 
1355: /// <summary>
1356: /// replace substring
1357: ///
1358: /// Replaces the substring from pos to pos+count with a sequence of chCount copies of ch
1359: /// </summary>
1360: /// <param name="pos">Starting position of substring to replace</param>
1361: /// <param name="count">Number of characters in substring to replace</param>
1362: /// <param name="ch">Characters to replace with</param>
1363: /// <param name="chCount">Number of copies of ch to replace with</param>
1364: /// <returns>Returns the reference to the resulting string</returns>
1365: String& String::replace(size_t pos, size_t count, ValueType ch, size_t chCount)
1366: {
1367:     String result = substr(0, pos) + String(chCount, ch) + substr(pos + count);
1368:     assign(result);
1369:     return *this;
1370: }
1371: 
1372: /// <summary>
1373: /// replace substring
1374: ///
1375: /// Replaces all instances of the substring oldStr (if existing) with newStr
1376: /// </summary>
1377: /// <param name="oldStr">string to find in string</param>
1378: /// <param name="newStr">string to replace with</param>
1379: /// <returns>Returns the number of times the string was replaced</returns>
1380: int String::replace(const String& oldStr, const String& newStr)
1381: {
1382:     size_t pos = find(oldStr);
1383:     size_t oldLength = oldStr.length();
1384:     size_t newLength = newStr.length();
1385:     int count = 0;
1386:     while (pos != npos)
1387:     {
1388:         replace(pos, oldLength, newStr);
1389:         pos += newLength;
1390:         pos = find(oldStr, pos);
1391:         count++;
1392:     }
1393:     return count;
1394: }
1395: 
1396: /// <summary>
1397: /// replace substring
1398: ///
1399: /// Replaces all instances of the substring oldStr (if existing) with newStr
1400: /// </summary>
1401: /// <param name="oldStr">string to find in string</param>
1402: /// <param name="newStr">string to replace with</param>
1403: /// <returns>Returns the number of times the string was replaced</returns>
1404: int String::replace(const ValueType* oldStr, const ValueType* newStr)
1405: {
1406:     if ((oldStr == nullptr) || (newStr == nullptr))
1407:         return 0;
1408:     size_t pos = find(oldStr);
1409:     size_t oldLength = strlen(oldStr);
1410:     size_t newLength = strlen(newStr);
1411:     int count = 0;
1412:     while (pos != npos)
1413:     {
1414:         replace(pos, oldLength, newStr);
1415:         pos += newLength;
1416:         pos = find(oldStr, pos);
1417:         count++;
1418:     }
1419:     return count;
1420: }
1421: 
1422: /// <summary>
1423: /// Align string
1424: ///
1425: /// Pads the string on the left (width > 0) or on the right (width < 0) up to a length of width characters. If the string is larger than width characters, it is not modified.
1426: /// </summary>
1427: /// <param name="width">length of target string. If width < 0, the string is padded to the right with spaces up to -width characters.
1428: /// if width > 0, the string is padded to the left with space up to width characters</param>
1429: /// <returns>Returns the number of times the string was replaced</returns>
1430: String String::align(int width) const
1431: {
1432:     String result;
1433:     int absWidth = (width > 0) ? width : -width;
1434:     auto len = length();
1435:     if (static_cast<size_t>(absWidth) > len)
1436:     {
1437:         if (width < 0)
1438:         {
1439:             result = *this + String(static_cast<unsigned int>(-width) - len, ' ');
1440:         }
1441:         else
1442:         {
1443:             result = String(static_cast<unsigned int>(width) - len, ' ') + *this;
1444:         }
1445:     }
1446:     else
1447:         result = *this;
1448:     return result;
1449: }
1450: 
1451: /// <summary>
1452: /// Allocate or re-allocate string to have a capacity of requestedLength characters
1453: /// </summary>
1454: /// <param name="requestedLength">Amount of characters in the string to allocate space for</param>
1455: /// <returns>True if successful, false otherwise</returns>
1456: bool String::reallocate(size_t requestedLength)
1457: {
1458:     auto requestedSize = requestedLength;
1459:     auto allocationSize = NextPowerOf2((requestedSize < MinimumAllocationSize) ? MinimumAllocationSize : requestedSize);
1460: 
1461:     if (!reallocate_allocation_size(allocationSize))
1462:         return false;
1463:     return true;
1464: }
1465: 
1466: /// <summary>
1467: /// Allocate or re-allocate string to have a capacity of allocationSize bytes
1468: /// </summary>
1469: /// <param name="allocationSize">Amount of bytes to allocate space for</param>
1470: /// <returns>True if successful, false otherwise</returns>
1471: bool String::reallocate_allocation_size(size_t allocationSize)
1472: {
1473:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1474:     if (newBuffer == nullptr)
1475:     {
1476:         return false;
1477:     }
1478:     m_buffer = newBuffer;
1479:     if (m_end == nullptr)
1480:         m_end = m_buffer;
1481:     if (m_end > m_buffer + allocationSize)
1482:         m_end = m_buffer + allocationSize;
1483:     m_allocatedSize = allocationSize;
1484:     return true;
1485: }
```

As you can see, this is a very extensive class.
A few remarks:
- Line 52-53: We use a minimum allocation size of 256, meaning that every string will have storage space for 255 characters plus a null character for termination, at the least.
This is to limit the amount of re-allocations when extending strings
- Line 55-56: We define a constant `MaximumStringSize`. We limit strings to a maximum length, which is one less than the largest heap block that can be allocated in the default bucket sizes (512 Kb)
- Line 59-60: We use a special static variable `NullCharConst` when we need to return something constant, but the string is not allocated.
- Line 61-62: Similarly, we use a special static variable `NullChar` when we need to return something non constant, but the string is not allocated.
This variable is non-const, so will have to be initialized every time we return it, to make sure it is still a null character.
- Line 179-193: We implement a move constructor, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 256-275: We also implement a move assignment, similar to the move constructor
- Line 277-297: We use our own version of a iterator template, which we'll get to, to enable creating the `begin()` and `end()` methods.
These allow us to use e.g. the standard c++ `for (x : string)` construct. We'll have to define this iterator ourselves in a minute
- Line 299-319: We also use our own version of a const_iterator template for const iterators, which we'll get to, to enable creating the `begin()` and `end()` const methods.
- Line 1017-1045: We also have the `contains()` methods, which are new to C++23, however they are quite convenient
- Line 1070-1120: We use the methods `equals()` and `equals_case_insensititive()` to compare strings. Both are not standard, but convenient.
These use the stand C functions `strcmp()` and `strcasecmp()`, which we will need to declare and define
- Line 1372-1420: We implement two extra variants of `replace()` (replacing multiple instances of a string) for convenience
- Line 1422-1449: Again for convenience, we implement the `align()` method. We'll be using this later on for aligning strings when formatting.
If the width is negative, we align to the left, if it is positive we align to the right.
- Line 1451-1464: When reallocating a string, we take care to round up the reserved space to a power of two, to minimize the number of reallocations needed.
We still need to implement the function used, `NextPowerOf2()` though

### Iterator.h {#TUTORIAL_16_STRING_CREATING_A_STRING_CLASS_ITERATORH}

As said before, we use our own version of `const_iterator` and `iterator` to be used in the `string` methods `begin()` and `end()`.
We'll declare and implement these.

Create the file `code/libraries/baremetal/include/baremetal/Iterator.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Iterator.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
3: //
4: // File        : Iterator.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : Iterator
9: //
10: // Description : Generic iterator template
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
43: /// Iterator classes
44: 
45: /// <summary>
46: /// Const iterator template
47: ///
48: /// Based on arrays of type T pointed to by const pointers.
49: /// </summary>
50: /// <typeparam name="T">Value type to iterate through</typeparam>
51: template <class T> class const_iterator
52: {
53: private:
54:     /// @brief Start of the range
55:     T const* m_begin;
56:     /// @brief End of the range + 1
57:     T const* m_end;
58:     /// @brief Current position
59:     T const* m_current;
60: 
61: public:
62:     /// <summary>
63:     /// Construct a const_iterator
64:     /// </summary>
65:     /// <param name="begin">Start of the range</param>
66:     /// <param name="end">End of the range + 1</param>
67:     explicit const_iterator(const T* begin, const T* end)
68:         : m_begin{begin}
69:         , m_end{end}
70:         , m_current{begin}
71:     {
72:     }
73:     /// <summary>
74:     /// Advance in range by 1 step (post increment)
75:     /// </summary>
76:     /// <returns>Position after advancing as const_iterator</returns>
77:     const_iterator& operator++()
78:     {
79:         ++m_current;
80:         return *this;
81:     }
82:     /// <summary>
83:     /// Advance in range by 1 step (pre increment)
84:     /// </summary>
85:     /// <returns>Position before advancing as const_iterator</returns>
86:     const_iterator operator++(int)
87:     {
88:         const_iterator retval = *this;
89:         ++(*this);
90:         return retval;
91:     }
92:     /// <summary>
93:     /// Equality comparison
94:     ///
95:     /// Compares current position with current position in passed iterator
96:     /// </summary>
97:     /// <param name="other">const_iterator to compare with</param>
98:     /// <returns>Returns true if positions are equal, false if not</returns>
99:     bool operator==(const const_iterator& other) const
100:     {
101:         return m_current == other.m_current;
102:     }
103:     /// <summary>
104:     /// Inequality comparison
105:     ///
106:     /// Compares current position with current position in passed iterator
107:     /// </summary>
108:     /// <param name="other">const_iterator to compare with</param>
109:     /// <returns>Returns false if positions are equal, true if not</returns>
110:     bool operator!=(const const_iterator& other) const
111:     {
112:         return !(*this == other);
113:     }
114:     /// <summary>
115:     /// Return value at current location
116:     /// </summary>
117:     /// <returns>Value to which the current position points</returns>
118:     T const& operator*() const
119:     {
120:         return *m_current;
121:     }
122: };
123: 
124: /// <summary>
125: /// Non-const iterator template
126: ///
127: /// Based on arrays of type T pointed to by pointers.
128: /// </summary>
129: /// <typeparam name="T">Value type to iterate through</typeparam>
130: template <class T> class iterator
131: {
132: private:
133:     /// @brief Start of the range
134:     T* m_begin;
135:     /// @brief End of the range + 1
136:     T* m_end;
137:     /// @brief Current position
138:     T* m_current;
139: 
140: public:
141:     /// <summary>
142:     /// Construct a iterator
143:     /// </summary>
144:     /// <param name="begin">Start of the range</param>
145:     /// <param name="end">End of the range + 1</param>
146:     explicit iterator(T* begin, T* end)
147:         : m_begin{begin}
148:         , m_end{end}
149:         , m_current{begin}
150:     {
151:     }
152:     /// <summary>
153:     /// Advance in range by 1 step (post increment)
154:     /// </summary>
155:     /// <returns>Position after advancing as iterator</returns>
156:     iterator& operator++()
157:     {
158:         ++m_current;
159:         return *this;
160:     }
161:     /// <summary>
162:     /// Advance in range by 1 step (pre increment)
163:     /// </summary>
164:     /// <returns>Position before advancing as iterator</returns>
165:     iterator operator++(int)
166:     {
167:         iterator retval = *this;
168:         ++(*this);
169:         return retval;
170:     }
171:     /// <summary>
172:     /// Equality comparison
173:     ///
174:     /// Compares current position with current position in passed iterator
175:     /// </summary>
176:     /// <param name="other">iterator to compare with</param>
177:     /// <returns>Returns true if positions are equal, false if not</returns>
178:     bool operator==(const iterator& other) const
179:     {
180:         return m_current == other.m_current;
181:     }
182:     /// <summary>
183:     /// Inequality comparison
184:     ///
185:     /// Compares current position with current position in passed iterator
186:     /// </summary>
187:     /// <param name="other">iterator to compare with</param>
188:     /// <returns>Returns false if positions are equal, true if not</returns>
189:     bool operator!=(const iterator& other) const
190:     {
191:         return !(*this == other);
192:     }
193:     /// <summary>
194:     /// Return value at current location
195:     /// </summary>
196:     /// <returns>Value to which the current position points</returns>
197:     T& operator*() const
198:     {
199:         return *m_current;
200:     }
201: };
```

Again, we will not go into any detail, the classes should speak for themselves.
We define both the `iterator` and `const_iterator` templates here.

### Util.h {#TUTORIAL_16_STRING_CREATING_A_STRING_CLASS_UTILH}

We need to declare and define the standard C functions `strcmp()` and `strcasecmp()` for our implementation of the `string` class,
as well as the `NextPowerOf2()` utility function.

Update the file `code/libraries/stdlib/include/stdlib/Util.h`

```cpp
File: code/libraries/stdlib/include/stdlib/Util.h
...
47: #ifdef __cplusplus
48: extern "C" {
49: #endif
50: 
51: void* memset(void* buffer, int value, size_t length);
52: void* memcpy(void* dest, const void* src, size_t length);
53: int memcmp(const void* buffer1, const void* buffer2, size_t length);
54: 
55: int toupper(int c);
56: int tolower(int c);
57: size_t strlen(const char* str);
58: int strcmp(const char* str1, const char* str2);
59: int strcasecmp(const char* str1, const char* str2);
60: int strncmp(const char* str1, const char* str2, size_t maxLen);
61: int strncasecmp(const char* str1, const char* str2, size_t maxLen);
62: char* strncpy(char* dest, const char* src, size_t maxLen);
63: char* strncat(char* dest, const char* src, size_t maxLen);
64: 
65: #ifdef __cplusplus
66: }
67: #endif
68: 
69: /// <summary>
70: /// Determine the number of bits needed to represent the specified value
71: /// </summary>
72: /// <param name="value">Value to check</param>
73: /// <returns>Number of bits used for value</returns>
74: inline constexpr unsigned NextPowerOf2Bits(size_t value)
75: {
76:     unsigned bitCount{0};
77:     size_t temp = value;
78:     while (temp >= 1)
79:     {
80:         ++bitCount;
81:         temp >>= 1;
82:     }
83:     return bitCount;
84: }
85: 
86: /// <summary>
87: /// Determine the next power of 2 greater than or equal to the specified value
88: /// </summary>
89: /// <param name="value">Value to check</param>
90: /// <returns>Power of two greater or equal to value</returns>
91: inline constexpr size_t NextPowerOf2(size_t value)
92: {
93:     return 1 << NextPowerOf2Bits((value != 0) ? value - 1 : 0);
94: }
```

- Line 53: We add the standard `memcmp()` function to compare two memory blocks
- Line 55-56: We add the standard `toupper()` amd `tolower()` functions to convert a character to upper or lower case
- Line 58-61: We add the standard `strcmp()` (compare strings to end of string), `strncmp()` (compare strings with maximum size), and the case insensitive variants `strcasecmp()` and `strncasecmp()`
- Line 69-84: We add an inline function `NextPowerOf2Bits()` to compute the number of significant bits in n, the number of bits b required to hold a value n, so 2^b >= n
- Line 86-94: We add an inline function `NextPowerOf2()` to compute the value x larger or equal to a number n, such that x is a power of 2

### Util.cpp {#TUTORIAL_16_STRING_CREATING_A_STRING_CLASS_UTILCPP}

We'll implement the new utility functions.

Update the file `code/libraries/baremetal/src/Util.cpp`

```cpp
File: code/libraries/baremetal/src/Util.cpp
...
82: /// <summary>
83: /// Compare two regions of memory
84: /// </summary>
85: /// <param name="buffer1">Pointer to first memory buffer</param>
86: /// <param name="buffer2">Pointer to second memory buffer</param>
87: /// <param name="length">Number of bytes to compare</param>
88: /// <returns>Returns 0 if the two regions are equal,
89: /// 1 if the values in the first buffer are greater,
90: /// -1 if the values in the second buffer are greater</returns>
91: int memcmp(const void* buffer1, const void* buffer2, size_t length)
92: {
93:     const unsigned char* p1 = reinterpret_cast<const unsigned char*>(buffer1);
94:     const unsigned char* p2 = reinterpret_cast<const unsigned char*>(buffer2);
95:
96:     while (length-- > 0)
97:     {
98:         if (*p1 > *p2)
99:         {
100:             return 1;
101:         }
102:         else if (*p1 < *p2)
103:         {
104:             return -1;
105:         }
106:
107:         p1++;
108:         p2++;
109:     }
110:
111:     return 0;
112: }
113:
114: /// <summary>
115: /// Convert character to upper case
116: /// </summary>
117: /// <param name="c">Character</param>
118: /// <returns>Uper case version of character c, if alphabetic, otherwise c</returns>
119: int toupper(int c)
120: {
121:     if (('a' <= c) && (c <= 'z'))
122:     {
123:         c -= 'a' - 'A';
124:     }
125:
126:     return c;
127: }
128:
129: /// <summary>
130: /// Convert character to lower case
131: /// </summary>
132: /// <param name="c">Character</param>
133: /// <returns>Lower case version of character c, if alphabetic, otherwise c</returns>
134: int tolower(int c)
135: {
136:     if (('A' <= c) && (c <= 'Z'))
137:     {
138:         c += 'a' - 'A';
139:     }
140:
141:     return c;
142: }
143:
...
161: /// <summary>
162: /// Compare two strings
163: /// </summary>
164: /// <param name="str1">Pointer to first string</param>
165: /// <param name="str2">Pointer to second string</param>
166: /// <returns>Returns 0 if the two strings are equal,
167: /// 1 if the values in the first string are greater,
168: /// -1 if the values in the second string are greater</returns>
169: int strcmp(const char* str1, const char* str2)
170: {
171:     while ((*str1 != '\0') && (*str2 != '\0'))
172:     {
173:         if (*str1 > *str2)
174:         {
175:             return 1;
176:         }
177:         else if (*str1 < *str2)
178:         {
179:             return -1;
180:         }
181:
182:         str1++;
183:         str2++;
184:     }
185:
186:     if (*str1 > *str2)
187:     {
188:         return 1;
189:     }
190:     else if (*str1 < *str2)
191:     {
192:         return -1;
193:     }
194:
195:     return 0;
196: }
197:
198: /// <summary>
199: /// Compare two strings, case insensitive
200: /// </summary>
201: /// <param name="str1">Pointer to first string</param>
202: /// <param name="str2">Pointer to second string</param>
203: /// <returns>Returns 0 is the two strings are equal ignoring case,
204: /// 1 if the values in the first string are greater,
205: /// -1 if the values in the second string are greater</returns>
206: int strcasecmp(const char* str1, const char* str2)
207: {
208:     int chr1, chr2;
209:
210:     while (((chr1 = toupper(*str1)) != '\0') && ((chr2 = toupper(*str2)) != '\0'))
211:     {
212:         if (chr1 > chr2)
213:         {
214:             return 1;
215:         }
216:         else if (chr1 < chr2)
217:         {
218:             return -1;
219:         }
220:
221:         str1++;
222:         str2++;
223:     }
224:
225:     chr2 = toupper(*str2);
226:
227:     if (chr1 > chr2)
228:     {
229:         return 1;
230:     }
231:     else if (chr1 < chr2)
232:     {
233:         return -1;
234:     }
235:
236:     return 0;
237: }
238:
239: /// <summary>
240: /// Compare two strings, up to maxLen characters
241: /// </summary>
242: /// <param name="str1">Pointer to first string</param>
243: /// <param name="str2">Pointer to second string</param>
244: /// <param name="maxLen">Maximum number of characters to compare</param>
245: /// <returns>Returns 0 is the two strings are equal,
246: /// 1 if the values in the first string are greater,
247: /// -1 if the values in the second string are greater</returns>
248: int strncmp(const char* str1, const char* str2, size_t maxLen)
249: {
250:     while ((maxLen > 0) && (*str1 != '\0') && (*str2 != '\0'))
251:     {
252:         if (*str1 > *str2)
253:         {
254:             return 1;
255:         }
256:         else if (*str1 < *str2)
257:         {
258:             return -1;
259:         }
260:
261:         maxLen--;
262:         str1++;
263:         str2++;
264:     }
265:
266:     if (maxLen == 0)
267:     {
268:         return 0;
269:     }
270:
271:     if (*str1 > *str2)
272:     {
273:         return 1;
274:     }
275:     else if (*str1 < *str2)
276:     {
277:         return -1;
278:     }
279:
280:     return 0;
281: }
282:
283: /// <summary>
284: /// Compare two strings, up to maxLen characters, case insensitive
285: /// </summary>
286: /// <param name="str1">Pointer to first string</param>
287: /// <param name="str2">Pointer to second string</param>
288: /// <param name="maxLen">Maximum number of characters to compare</param>
289: /// <returns>Returns 0 is the two strings are equal ignoring case,
290: /// 1 if the values in the first string are greater,
291: /// -1 if the values in the second string are greater</returns>
292: int strncasecmp(const char* str1, const char* str2, size_t maxLen)
293: {
294:     int chr1, chr2;
295:
296:     while ((maxLen > 0) && ((chr1 = toupper(*str1)) != '\0') && ((chr2 = toupper(*str2)) != '\0'))
297:     {
298:         if (chr1 > chr2)
299:         {
300:             return 1;
301:         }
302:         else if (chr1 < chr2)
303:         {
304:             return -1;
305:         }
306:
307:         maxLen--;
308:         str1++;
309:         str2++;
310:     }
311:
312:     chr2 = toupper(*str2);
313:
314:     if (maxLen == 0)
315:     {
316:         return 0;
317:     }
318:
319:     if (chr1 > chr2)
320:     {
321:         return 1;
322:     }
323:     else if (chr1 < chr2)
324:     {
325:         return -1;
326:     }
327:
328:     return 0;
329: }
330:
...
```

The new functions added should be self-explanatory.

### Application code {#TUTORIAL_16_STRING_CREATING_A_STRING_CLASS_APPLICATION_CODE}

We'll start making use of the string class we just added, but we'll do it in a way that shows that the string methods function as expected.
This is a first attempt at creating class / micro / unit tests for our code, which we pick up on later.

Update the file `code\applications\demo\src\main.cpp`

```cpp
File: code\applications\demo\src\main.cpp
1: #include "baremetal/ARMInstructions.h"
2: #include "baremetal/Assert.h"
3: #include "baremetal/BCMRegisters.h"
4: #include "baremetal/Console.h"
5: #include "baremetal/Logger.h"
6: #include "baremetal/Mailbox.h"
7: #include "baremetal/MemoryManager.h"
8: #include "baremetal/New.h"
9: #include "baremetal/RPIProperties.h"
10: #include "baremetal/Serialization.h"
11: #include "baremetal/String.h"
12: #include "baremetal/SysConfig.h"
13: #include "baremetal/System.h"
14: #include "baremetal/Timer.h"
15: #include "stdlib/Util.h"
16: 
17: LOG_MODULE("main");
18: 
19: using namespace baremetal;
20: 
21: int main()
22: {
23:     auto& console = GetConsole();
24:     GetLogger().SetLogLevel(LogSeverity::Debug);
25: 
26:     MemoryManager& memoryManager = GetMemoryManager();
27:     LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
28:     LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
29:     LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));
30: 
31:     memoryManager.DumpStatus();
32: 
33:     String s1{"a"};
34:     String s2{"a"};
35:     String s3{"aaaa", 3};
36:     String s4{4, 'b'};
37:     String s5{s3};
38:     String s6{s3, 1};
39:     String s7{s3, 1, 1};
40:     String s8{nullptr};
41:     String s9{""};
42:     String s10{nullptr, 3};
43: 
44:     LOG_INFO("s4");
45:     for (auto ch : s4)
46:     {
47:         LOG_INFO("%c", ch);
48:     }
49:     assert(strcmp(s1, "a") == 0);
50:     assert(strcmp(s2, "a") == 0);
51:     assert(strcmp(s3, "aaa") == 0);
52:     assert(strcmp(s4, "bbbb") == 0);
53:     assert(strcmp(s5, "aaa") == 0);
54:     assert(strcmp(s6, "aa") == 0);
55:     assert(strcmp(s7, "a") == 0);
56:     assert(strcmp(s8, "") == 0);
57:     assert(strcmp(s9, "") == 0);
58:     assert(strcmp(s10, "") == 0);
59: 
60:     s7 = "abcde";
61:     assert(strcmp(s7, "abcde") == 0);
62:     s7 = nullptr;
63:     assert(strcmp(s7, "") == 0);
64:     s7 = "";
65:     assert(strcmp(s7, "") == 0);
66:     s6 = s4;
67:     assert(strcmp(s6, "bbbb") == 0);
68:     {
69:         String s99{"cccc"};
70:         s6 = s99;
71:     }
72:     assert(strcmp(s6, "cccc") == 0);
73: 
74:     const char* s = "abcdefghijklmnopqrstuvwxyz";
75:     s1.assign(s);
76:     assert(strcmp(s1, "abcdefghijklmnopqrstuvwxyz") == 0);
77:     s1.assign("");
78:     assert(strcmp(s1, "") == 0);
79:     s1.assign(nullptr);
80:     assert(strcmp(s1, "") == 0);
81:     s1.assign(s, 6);
82:     assert(strcmp(s1, "abcdef") == 0);
83:     s1.assign("", 6);
84:     assert(strcmp(s1, "") == 0);
85:     s1.assign(nullptr, 6);
86:     assert(strcmp(s1, "") == 0);
87:     s8 = s;
88:     s1.assign(s8, 3);
89:     assert(strcmp(s1, "defghijklmnopqrstuvwxyz") == 0);
90:     s1.assign(s8, 4, 6);
91:     assert(strcmp(s1, "efghij") == 0);
92:     s1.assign(6, 'c');
93:     assert(strcmp(s1, "cccccc") == 0);
94: 
95:     const String s8c{s8};
96:     assert(s8.at(3) == 'd');
97:     assert(s8c.at(3) == 'd');
98:     assert(s8.front() == 'a');
99:     assert(s8c.front() == 'a');
100:     assert(s8.back() == 'z');
101:     assert(s8c.back() == 'z');
102:     assert(s8[3] == 'd');
103:     assert(s8c[3] == 'd');
104:     assert(s8c.capacity() == 256);
105:     assert(s8.reserve(1024) == 1024);
106:     assert(s8.capacity() == 1024);
107: 
108:     s1 = "a";
109:     assert(strcmp(s1, "a") == 0);
110:     s1 += 'b';
111:     assert(strcmp(s1, "ab") == 0);
112:     s2 = "a";
113:     s1 += s2;
114:     assert(strcmp(s1, "aba") == 0);
115:     s1 += "abcde";
116:     assert(strcmp(s1, "abaabcde") == 0);
117:     s1 = "a";
118:     s1 += "";
119:     assert(strcmp(s1, "a") == 0);
120:     s1 += nullptr;
121:     assert(strcmp(s1, "a") == 0);
122: 
123:     s3 = "";
124:     s4 = s1 + s2;
125:     assert(strcmp(s4, "aa") == 0);
126:     s4 = s1 + s3;
127:     assert(strcmp(s4, "a") == 0);
128:     s4 = s1 + "b";
129:     assert(strcmp(s4, "ab") == 0);
130:     s4 = s1 + "";
131:     assert(strcmp(s4, "a") == 0);
132:     s4 = s1 + nullptr;
133:     assert(strcmp(s4, "a") == 0);
134:     s4 = "b" + s1;
135:     assert(strcmp(s4, "ba") == 0);
136:     s4 = "" + s1;
137:     assert(strcmp(s4, "a") == 0);
138:     s4 = nullptr + s1;
139:     assert(strcmp(s4, "a") == 0);
140: 
141:     s1 = "a";
142:     s1.append(4, 'b');
143:     assert(strcmp(s1, "abbbb") == 0);
144:     s1.append(s2);
145:     assert(strcmp(s1, "abbbba") == 0);
146:     s1.append(s8, 3, 5);
147:     assert(strcmp(s1, "abbbbadefgh") == 0);
148:     s1.append("ccc");
149:     assert(strcmp(s1, "abbbbadefghccc") == 0);
150:     s1.append("dddddd", 3);
151:     assert(strcmp(s1, "abbbbadefghcccddd") == 0);
152:     s1.clear();
153:     assert(strcmp(s1, "") == 0);
154:     s1.append("");
155:     assert(strcmp(s1, "") == 0);
156:     s1.append(nullptr);
157:     assert(strcmp(s1, "") == 0);
158:     s1.append("", 3);
159:     assert(strcmp(s1, "") == 0);
160:     s1.append(nullptr, 3);
161:     assert(strcmp(s1, "") == 0);
162: 
163:     s1 = s;
164:     s2 = "c";
165:     auto pos = s1.find(s2);
166:     assert(pos == 2);
167:     pos = s1.find(s2, 1);
168:     assert(pos == 2);
169:     pos = s1.find(s2, 3);
170:     assert(pos == String::npos);
171:     s2 = "deg";
172:     pos = s1.find(s2, 3);
173:     assert(pos == String::npos);
174:     pos = s1.find(s2, 3, 2);
175:     assert(pos == 3);
176:     s2 = "xyz";
177:     pos = s1.find(s2);
178:     assert(pos == 23);
179: 
180:     pos = s1.find("d");
181:     assert(pos == 3);
182:     pos = s1.find("d", 1);
183:     assert(pos == 3);
184:     pos = s1.find("d", 4);
185:     assert(pos == String::npos);
186:     pos = s1.find("def", 2);
187:     assert(pos == 3);
188:     pos = s1.find("deg", 2);
189:     assert(pos == String::npos);
190:     pos = s1.find("deg", 2, 2);
191:     assert(pos == 3);
192:     pos = s1.find("xyz");
193:     assert(pos == 23);
194:     pos = s1.find("");
195:     assert(pos == 0);
196:     pos = s1.find(nullptr);
197:     assert(pos == 0);
198:     pos = s1.find("", 2);
199:     assert(pos == 2);
200:     pos = s1.find(nullptr, 2);
201:     assert(pos == 2);
202:     pos = s1.find(nullptr, 26);
203:     assert(pos == String::npos);
204:     pos = s1.find("", 2, 2);
205:     assert(pos == 2);
206:     pos = s1.find(nullptr, 2, 2);
207:     assert(pos == 2);
208:     pos = s1.find(nullptr, 26, 1);
209:     assert(pos == String::npos);
210: 
211:     pos = s1.find('d');
212:     assert(pos == 3);
213:     pos = s1.find('d', 2);
214:     assert(pos == 3);
215:     pos = s1.find('d', 4);
216:     assert(pos == String::npos);
217:     pos = s1.find('A');
218:     assert(pos == String::npos);
219:     pos = s1.find("z");
220:     assert(pos == 25);
221: 
222:     s2 = "abc";
223:     s3 = "xyz";
224:     auto isTrue = s1.starts_with('a');
225:     assert(isTrue);
226:     isTrue = s1.starts_with('z');
227:     assert(!isTrue);
228:     isTrue = s1.starts_with("abc");
229:     assert(isTrue);
230:     isTrue = s1.starts_with("xyz");
231:     assert(!isTrue);
232:     isTrue = s1.starts_with("");
233:     assert(isTrue);
234:     isTrue = s1.starts_with(nullptr);
235:     assert(isTrue);
236:     isTrue = s1.starts_with(s2);
237:     assert(isTrue);
238:     isTrue = s1.starts_with(s3);
239:     assert(!isTrue);
240: 
241:     isTrue = s1.ends_with('a');
242:     assert(!isTrue);
243:     isTrue = s1.ends_with('z');
244:     assert(isTrue);
245:     isTrue = s1.ends_with("abc");
246:     assert(!isTrue);
247:     isTrue = s1.ends_with("xyz");
248:     assert(isTrue);
249:     isTrue = s1.ends_with("");
250:     assert(isTrue);
251:     isTrue = s1.ends_with(nullptr);
252:     assert(isTrue);
253:     isTrue = s1.ends_with(s2);
254:     assert(!isTrue);
255:     isTrue = s1.ends_with(s3);
256:     assert(isTrue);
257: 
258:     isTrue = s1.contains('a');
259:     assert(isTrue);
260:     isTrue = s1.contains('A');
261:     assert(!isTrue);
262:     isTrue = s1.contains("abc");
263:     assert(isTrue);
264:     isTrue = s1.contains("XYZ");
265:     assert(!isTrue);
266:     isTrue = s1.contains("");
267:     assert(isTrue);
268:     isTrue = s1.contains(nullptr);
269:     assert(isTrue);
270:     isTrue = s1.contains(s2);
271:     assert(isTrue);
272:     isTrue = s1.contains(s3);
273:     assert(isTrue);
274: 
275:     s2 = s1.substr();
276:     assert(strcmp(s2, "abcdefghijklmnopqrstuvwxyz") == 0);
277:     s2 = s1.substr(6);
278:     assert(strcmp(s2, "ghijklmnopqrstuvwxyz") == 0);
279:     s2 = s1.substr(6, 6);
280:     assert(strcmp(s2, "ghijkl") == 0);
281: 
282:     s1 = "abcdefg";
283:     s2 = "abcdefG";
284:     s3 = "abcdefg";
285:     isTrue = s1.equals(s2);
286:     assert(!isTrue);
287:     isTrue = s1.equals(s3);
288:     assert(isTrue);
289:     isTrue = s1.equals("abcefg");
290:     assert(!isTrue);
291:     isTrue = s1.equals("abcdefg");
292:     assert(isTrue);
293:     isTrue = s1.equals("");
294:     assert(!isTrue);
295:     isTrue = s1.equals(nullptr);
296:     assert(!isTrue);
297:     s4 = "";
298:     isTrue = s4.equals_case_insensitive(s3);
299:     assert(!isTrue);
300:     isTrue = s4.equals("");
301:     assert(isTrue);
302:     isTrue = s4.equals(nullptr);
303:     assert(isTrue);
304: 
305:     isTrue = s1.equals_case_insensitive(s2);
306:     assert(isTrue);
307:     isTrue = s1.equals_case_insensitive(s3);
308:     assert(isTrue);
309:     isTrue = s1.equals_case_insensitive("abcefg");
310:     assert(!isTrue);
311:     isTrue = s1.equals_case_insensitive("abcdefg");
312:     assert(isTrue);
313:     isTrue = s1.equals_case_insensitive("");
314:     assert(!isTrue);
315:     isTrue = s1.equals_case_insensitive(nullptr);
316:     assert(!isTrue);
317:     s4 = "";
318:     isTrue = s4.equals_case_insensitive(s3);
319:     assert(!isTrue);
320:     isTrue = s4.equals_case_insensitive("");
321:     assert(isTrue);
322:     isTrue = s4.equals_case_insensitive(nullptr);
323:     assert(isTrue);
324: 
325:     assert(s1 == s3);
326:     assert(s1 != s2);
327:     assert(s1 == "abcdefg");
328:     assert(s1 != "abcdefG");
329:     assert(s1 != "");
330:     assert(s1 != nullptr);
331:     assert("abcdefg" == s1);
332:     assert("abcdefG" != s1);
333:     assert("" != s1);
334:     assert(nullptr != s1);
335:     assert(s4 != s3);
336:     assert(s4 == "");
337:     assert(s4 == nullptr);
338:     assert("" == s4);
339:     assert(nullptr == s4);
340: 
341:     s4 = "bcdefg";
342:     s5 = "def";
343:     auto result = s1.compare(s2);
344:     assert(result == 1);
345:     result = s2.compare(s1);
346:     assert(result == -1);
347:     result = s1.compare(s3);
348:     assert(result == 0);
349:     result = s3.compare(s1);
350:     assert(result == 0);
351:     result = s1.compare(1, 6, s4);
352:     assert(result == 0);
353:     result = s1.compare(1, 5, s4);
354:     assert(result == -1);
355:     result = s1.compare(3, 6, s4, 2);
356:     assert(result == 0);
357:     result = s1.compare(3, 6, s4, 2, 1);
358:     assert(result == 1);
359:     result = s1.compare(3, 3, s4, 2, 6);
360:     assert(result == -1);
361: 
362:     result = s1.compare("a");
363:     assert(result == 1);
364:     result = s1.compare("Abcdefg");
365:     assert(result == 1);
366:     result = s1.compare("abdecfg");
367:     assert(result == -1);
368:     result = s1.compare("");
369:     assert(result == 1);
370:     result = s1.compare(nullptr);
371:     assert(result == 1);
372:     s2 = "";
373:     result = s2.compare("a");
374:     assert(result == -1);
375:     result = s2.compare("");
376:     assert(result == 0);
377:     result = s2.compare(nullptr);
378:     assert(result == 0);
379: 
380:     s1 = "abcde";
381:     s2 = "fghijk";
382:     s3 = s1.replace(0, 1, s2);
383:     assert(s1.equals("fghijkbcde"));
384:     assert(s3.equals("fghijkbcde"));
385:     s1 = "abcde";
386:     s3 = s1.replace(1, 2, s2, 2);
387:     assert(s1.equals("ahijkde"));
388:     assert(s3.equals("ahijkde"));
389:     s1 = "abcde";
390:     s3 = s1.replace(1, 2, s2, 2, 2);
391:     assert(s1.equals("ahide"));
392:     assert(s3.equals("ahide"));
393:     s1 = "abcde";
394:     s3 = s1.replace(0, 1, "uvwxyz");
395:     assert(s1.equals("uvwxyzbcde"));
396:     assert(s3.equals("uvwxyzbcde"));
397:     s1 = "abcde";
398:     s3 = s1.replace(1, 2, "uvwxyz", 2);
399:     assert(s1.equals("auvde"));
400:     assert(s3.equals("auvde"));
401:     s1 = "abcde";
402:     s3 = s1.replace(0, 1, 'x');
403:     assert(s1.equals("xbcde"));
404:     assert(s3.equals("xbcde"));
405:     s1 = "abcde";
406:     s3 = s1.replace(1, 2, 'x', 3);
407:     assert(s1.equals("axxxde"));
408:     assert(s3.equals("axxxde"));
409: 
410:     s1 = "abcde";
411:     s2 = "cd";
412:     s3 = "xy";
413:     int count = s1.replace(s2, s3);
414:     assert(count == 1);
415:     assert(s1.equals("abxye"));
416:     s1 = "abababab";
417:     s2 = "ab";
418:     s3 = "cd";
419:     count = s1.replace(s2, s3);
420:     assert(count == 4);
421:     assert(s1.equals("cdcdcdcd"));
422:     s1 = "abcde";
423:     count = s1.replace("cd", "xy");
424:     assert(count == 1);
425:     assert(s1.equals("abxye"));
426:     s1 = "abababab";
427:     count = s1.replace("ab", "cd");
428:     assert(count == 4);
429:     assert(s1.equals("cdcdcdcd"));
430: 
431:     s1 = "abcd";
432:     s2 = s1.align(8);
433:     assert(s2.equals("    abcd"));
434:     s2 = s1.align(-8);
435:     assert(s2.equals("abcd    "));
436:     s2 = s1.align(0);
437:     assert(s2.equals("abcd"));
438: 
439:     LOG_INFO("Wait 5 seconds");
440:     Timer::WaitMilliSeconds(5000);
441: 
442:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
443:     char ch{};
444:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
445:     {
446:         ch = console.ReadChar();
447:         console.WriteChar(ch);
448:     }
449:     if (ch == 'p')
450:         assert(false);
451: 
452:     LOG_INFO("Heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::LOW));
453:     LOG_INFO("High heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::HIGH));
454:     LOG_INFO("DMA heap space available: %llu bytes", memoryManager.GetHeapFreeSpace(HeapType::ANY));
455: 
456:     memoryManager.DumpStatus();
457: 
458:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
459: }
```

As you can see the code has grown quite a bit due to all the tests we perform.

- Line 11: We need to include `String.h`
- Line 15: We need to include `Util.h`
- Line 26-29: We display the status of the memory manager
- Line 33-42: We construct some strings, in different ways, to cover all the variants of constructors
- Line 44-48: We use a for loop to iterate through the characters in a string.
This will use the `begin()` and `end()` methods
- Line 49-58: We check whether the strings are initialized as expected.
Notice that we use the `assert` macro here
- Line 60-72: We test assignment operators
- Line 74-93: We test the `assign()` methods
- Line 95-106: We test the `at()`, `front()`, `back()`, `capacity()` and `reserve()` methods, as well as the index operator `[]`
- Line 108-121: We test the addition assignment (in this case concatenation) operator `+=`
- Line 123-139: We test the addition (concatenation) operator `+`
- Line 141-161: We test the `append()` methods
- Line 163-220: We test the `find()` methods
- Line 222-239: We test the `starts_with()` methods
- Line 241-256: We test the `ends_with()` methods
- Line 258-273: We test the `contains()` methods
- Line 275-280: We test the `substr()` methods
- Line 282-323: We test the `equals()` and `equals_case_insensitive()` methods
- Line 325-339: We test the equality operators `==` and the inequality operators `!=`
- Line 341-378: We test the `compare()` methods
- Line 380-429: We test the `replace()` methods
- Line 431-437: We test the `align()` method
- Line 452-456: We again display the status of the memory manager

### Configuring, building and debugging {#TUTORIAL_16_STRING_CREATING_A_STRING_CLASS_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will print the characters in the string "bbbb" in log statements.
Next to this nothing special is shown, except for the memory status at start and end of the application, as the tests will all succeed.
If a test were to fail, the assertion would fire, and the application would crash.
This is a way to test code, however we'd like to see all of the failure found in our code, not have the first one crash the application.

```text
Setting up UART0
Info   Baremetal 0.0.1 started on Raspberry Pi 4 Model B (AArch64) using BCM2711 SoC (Logger:91)
Info   Starting up (System:204)
Info   Heap space available: 967835648 bytes (main:27)
Info   High heap space available: 2147483648 bytes (main:28)
Info   DMA heap space available: 3115319296 bytes (main:29)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Info   s4 (main:44)
Info   b (main:47)
Info   b (main:47)
Info   b (main:47)
Info   b (main:47)
Info   Wait 5 seconds (main:439)
Press r to reboot, h to halt, p to fail assertion and panic
rInfo   Heap space available: 967807360 bytes (main:452)
Info   High heap space available: 2147483648 bytes (main:453)
Info   DMA heap space available: 3115291008 bytes (main:454)
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    24
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        26
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:24576
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 106
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  108544
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     82
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      83968
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Debug  Low heap: (MemoryManager:230)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaplow
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    14
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        26
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:14336
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 106
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  108544
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     92
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      94208
Debug  High heap: (MemoryManager:233)
Debug  DumpStatus (HeapAllocator.cpp:276) Heap allocator info:     heaphigh
Debug  DumpStatus (HeapAllocator.cpp:277) Current #allocations:    0
Debug  DumpStatus (HeapAllocator.cpp:278) Max #allocations:        0
Debug  DumpStatus (HeapAllocator.cpp:279) Current #allocated bytes:0
Debug  DumpStatus (HeapAllocator.cpp:280) Total #allocated blocks: 0
Debug  DumpStatus (HeapAllocator.cpp:281) Total #allocated bytes:  0
Debug  DumpStatus (HeapAllocator.cpp:282) Total #freed blocks:     0
Debug  DumpStatus (HeapAllocator.cpp:283) Total #freed bytes:      0
Info   Reboot (System:136)
```

Next: [17-serializing-and-formatting](17-serializing-and-formatting.md)
