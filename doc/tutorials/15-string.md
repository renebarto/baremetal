# Tutorial 15: String {#TUTORIAL_15_STRING}

@tableofcontents

## New tutorial setup {#TUTORIAL_15_STRING_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/15-string`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_15_STRING_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-15.a`
- an application `output/Debug/bin/15-string.elf`
- an image in `deploy/Debug/15-string-image`

## Creating a string class {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS}

It would be nice if we could deal with strings in a more flexible way. For that, we need memory allocation, so we had to wait until now with that.

We will introduce a string class, and start using it for e.g. serialization, formatting, etc.

### String.h {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_STRINGH}

First we'll declare the `String` class.
This `String` class mimicks the `std::string` class in the standard C++ library quite closely, to make a later move to the standard C++ library easier.
This also means this is quite extensive class. As an exception, this class has a lower case name, and lower case methods, due to the mimicking.

Create the file `code/libraries/baremetal/include/baremetal/String.h`

```cpp
File: code/libraries/baremetal/include/baremetal/String.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : String.h
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : string
9: //
10: // Description : string handling
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
42: #include <baremetal/Iterator.h>
43: #include <baremetal/Types.h>
44:
45: /// @file
46: /// string class
47:
48: namespace baremetal {
49:
50: /// <summary>
51: /// string class
52: /// </summary>
53: class string
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
71:     string();
72:     explicit string(const ValueType* str);
73:     string(const ValueType* str, size_t count);
74:     string(size_t count, ValueType ch);
75:     string(const string& other);
76:     string(string&& other);
77:     string(const string& other, size_t pos, size_t count = npos);
78:     ~string();
79:
80:     operator const ValueType *() const;
81:     string&     operator=(const ValueType* other);
82:     string&     operator=(const string& other);
83:     string&     operator=(string&& other);
84:
85:     iterator<ValueType>         begin();
86:     iterator<ValueType>         end();
87:     const_iterator<ValueType>   begin() const;
88:     const_iterator<ValueType>   end() const;
89:
90:     string&                     assign(const ValueType* str);
91:     string&                     assign(const ValueType* str, size_t count);
92:     string&                     assign(size_t count, ValueType ch);
93:     string&                     assign(const string& str);
94:     string&                     assign(const string& str, size_t pos, size_t count = npos);
95:
96:     ValueType&                  at(size_t pos);
97:     const ValueType&            at(size_t pos) const;
98:     ValueType&                  front();
99:     const ValueType&            front() const;
100:     ValueType&                  back();
101:     const ValueType&            back() const;
102:     ValueType&                  operator[] (size_t pos);
103:     const ValueType&            operator[] (size_t pos) const;
104:     ValueType*                  data();
105:     const ValueType*            data() const;
106:     const ValueType*            c_str() const;
107:
108:     bool                        empty() const;
109:     size_t                      size() const;
110:     size_t                      length() const;
111:     size_t                      capacity() const;
112:     size_t                      reserve(size_t newCapacity);
113:
114:     string&                     operator +=(ValueType ch);
115:     string&                     operator +=(const string& str);
116:     string&                     operator +=(const ValueType* str);
117:     void                        append(size_t count, ValueType ch);
118:     void                        append(const string& str);
119:     void                        append(const string& str, size_t pos, size_t count = npos);
120:     void                        append(const ValueType *str);
121:     void                        append(const ValueType* str, size_t count);
122:     void                        clear();
123:
124:     size_t                      find(const string& str, size_t pos = 0) const;
125:     size_t                      find(const ValueType* str, size_t pos = 0) const;
126:     size_t                      find(const ValueType* str, size_t pos, size_t count) const;
127:     size_t                      find(ValueType ch, size_t pos = 0) const;
128:     bool                        starts_with(ValueType ch) const;
129:     bool                        starts_with(const string& str) const;
130:     bool                        starts_with(const ValueType* str) const;
131:     bool                        ends_with(ValueType ch) const;
132:     bool                        ends_with(const string& str) const;
133:     bool                        ends_with(const ValueType* str) const;
134:     bool                        contains(ValueType ch) const;
135:     bool                        contains(const string& str) const;
136:     bool                        contains(const ValueType* str) const;
137:     string                      substr(size_t pos = 0, size_t count = npos) const;
138:
139:     bool                        equals(const string& other) const;
140:     bool                        equals(const ValueType* other) const;
141:     bool                        equals_case_insensitive(const string& other) const;
142:     bool                        equals_case_insensitive(const ValueType* other) const;
143:     int                         compare(const string& str) const;
144:     int                         compare(size_t pos, size_t count, const string& str) const;
145:     int                         compare(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount = npos) const;
146:     int                         compare(const ValueType* str) const;
147:     int                         compare(size_t pos, size_t count, const ValueType* str) const;
148:     int                         compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const;
149:
150:     string&                     replace(size_t pos, size_t count, const string& str);
151:     string&                     replace(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount = npos);
152:     string&                     replace(size_t pos, size_t count, const ValueType* str);
153:     string&                     replace(size_t pos, size_t count, const ValueType* str, size_t strCount);
154:     string&                     replace(size_t pos, size_t count, ValueType ch);
155:     string&                     replace(size_t pos, size_t count, ValueType ch, size_t chCount);
156:     int                         replace(const string& oldStr, const string& newStr); // returns number of occurrences
157:     int                         replace(const ValueType *oldStr, const ValueType *newStr); // returns number of occurrences
158:
159:     string                      align(int width) const;
160:
161: private:
162:     bool        reallocate(size_t requestedLength);
163:     bool        reallocate_allocation_size(size_t allocationSize);
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
174: inline bool operator==(const string &lhs, const string &rhs)
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
187: inline bool operator==(const string &lhs, const string::ValueType *rhs)
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
200: inline bool operator==(const string::ValueType *lhs, const string &rhs)
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
213: inline bool operator!=(const string &lhs, const string &rhs)
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
226: inline bool operator!=(const string &lhs, const string::ValueType *rhs)
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
239: inline bool operator!=(const string::ValueType *lhs, const string &rhs)
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
252: inline string operator +(const string& lhs, const string& rhs)
253: {
254:     string result = lhs;
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
267: inline string operator +(const string::ValueType* lhs, const string& rhs)
268: {
269:     string result{ lhs };
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
282: inline string operator +(const string& lhs, const string::ValueType* rhs)
283: {
284:     string result = lhs;
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
297: inline string operator +(string::ValueType lhs, const string& rhs)
298: {
299:     string result;
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
313: inline string operator +(const string& lhs, string::ValueType rhs)
314: {
315:     string result;
316:     result += lhs;
317:     result += rhs;
318:     return result;
319: }
320:
321: } // namespace baremetal
```

We'll not go into detail here, most methods should be clear, for the others you can visit the documentation of `std::string`.

### String.cpp {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_STRINGCPP}

We'll implement the `String` class.

Create the file `code/libraries/baremetal/src/String.cpp`

```cpp
File: code/libraries/baremetal/src/String.cpp
File: d:\Projects\baremetal.github\code\libraries\baremetal\src\String.cpp
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
3: //
4: // File        : String.cpp
5: //
6: // Namespace   : baremetal
7: //
8: // Class       : string
9: //
10: // Description : string handling
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
40: #include <baremetal/String.h>
41: 
42: #include <baremetal/Assert.h>
43: #include <baremetal/Logger.h>
44: #include <baremetal/Util.h>
45: 
46: /// @file
47: /// string class implementation
48: 
49: using namespace baremetal;
50: 
51: /// @brief Minimum allocation size for any string
52: static constexpr size_t MinimumAllocationSize = 64;
53: 
54: /// @brief Maximum string size (largest 256Mb - 1 due to largest heap allocation block size)
55: static constexpr size_t MaximumStringSize = 0x80000 - 1;
56: 
57: const size_t string::npos = static_cast<size_t>(-1);
58: /// @brief Constant null character, using as string terminator, and also returned as a reference for const methods where nothing can be returned
59: static const string::ValueType NullCharConst = '\0';
60: /// @brief Non-constant null character, returned as a reference for const methods where nothing can be returned (always reinitialized before returning)
61: static string::ValueType NullChar = '\0';
62: 
63: /// @brief Define log name
64: LOG_MODULE("String");
65: 
66: /// <summary>
67: /// Default constructor
68: ///
69: /// Constructs an empty string.
70: /// </summary>
71: string::string()
72:     : m_buffer{}
73:     , m_end{}
74:     , m_allocatedSize{}
75: {
76: }
77: 
78: /// <summary>
79: /// Destructor
80: ///
81: /// Frees any allocated memory.
82: /// </summary>
83: string::~string()
84: {
85: #if BAREMETAL_MEMORY_TRACING_DETAIL
86:     if (m_buffer != nullptr)
87:         LOG_NO_ALLOC_DEBUG("Free string %p", m_buffer);
88: #endif
89:     delete[] m_buffer;
90: }
91: 
92: /// <summary>
93: /// Constructor
94: ///
95: /// Initializes the string with the specified string.
96: /// </summary>
97: /// <param name="str">string to initialize with</param>
98: string::string(const ValueType* str)
99:     : m_buffer{}
100:     , m_end{}
101:     , m_allocatedSize{}
102: {
103:     if (str == nullptr)
104:         return;
105:     auto size = strlen(str);
106:     if (reallocate(size + 1))
107:     {
108:         strncpy(m_buffer, str, size);
109:     }
110:     m_end = m_buffer + size;
111:     m_buffer[size] = NullCharConst;
112: }
113: 
114: /// <summary>
115: /// Constructor
116: ///
117: /// Initializes the string with up to count characters in the specified string. A null character is always added.
118: /// </summary>
119: /// <param name="str">string to initialize with</param>
120: /// <param name="count">Maximum number of characters from str to initialize with. If count is larger than the actual string length, only the string length is used</param>
121: string::string(const ValueType* str, size_t count)
122:     : m_buffer{}
123:     , m_end{}
124:     , m_allocatedSize{}
125: {
126:     if (str == nullptr)
127:         return;
128:     auto size = strlen(str);
129:     if (count < size)
130:         size = count;
131:     if (reallocate(size + 1))
132:     {
133:         strncpy(m_buffer, str, size);
134:     }
135:     m_end = m_buffer + size;
136:     m_buffer[size] = NullCharConst;
137: }
138: 
139: /// <summary>
140: /// Constructor
141: ///
142: /// Initializes the string with the specified count times the specified character. A null character is always added.
143: /// </summary>
144: /// <param name="count">Number of characters of value ch to initialized with</param>
145: /// <param name="ch">Character to initialize with</param>
146: string::string(size_t count, ValueType ch)
147:     : m_buffer{}
148:     , m_end{}
149:     , m_allocatedSize{}
150: {
151:     auto size = count;
152:     if (size > MaximumStringSize)
153:         size = MaximumStringSize;
154:     if (reallocate(size + 1))
155:     {
156:         memset(m_buffer, ch, size);
157:     }
158:     m_end = m_buffer + size;
159:     m_buffer[size] = NullCharConst;
160: }
161: 
162: /// <summary>
163: /// Copy constructor
164: ///
165: /// Initializes the string with the specified string value.
166: /// </summary>
167: /// <param name="other">string to initialize with</param>
168: string::string(const string& other)
169:     : m_buffer{}
170:     , m_end{}
171:     , m_allocatedSize{}
172: {
173:     auto size = other.length();
174:     if (reallocate(size + 1))
175:     {
176:         strncpy(m_buffer, other.data(), size);
177:     }
178:     m_end = m_buffer + size;
179:     m_buffer[size] = NullCharConst;
180: }
181: 
182: /// <summary>
183: /// Move constructor
184: ///
185: /// Initializes the string by moving the contents from the specified string value.
186: /// </summary>
187: /// <param name="other">string to initialize with</param>
188: string::string(string&& other)
189:     : m_buffer{other.m_buffer}
190:     , m_end{other.m_end}
191:     , m_allocatedSize{other.m_allocatedSize}
192: {
193:     other.m_buffer = nullptr;
194:     other.m_end = nullptr;
195:     other.m_allocatedSize = 0;
196: }
197: 
198: /// <summary>
199: /// Constructor
200: ///
201: /// Initializes the string with the substring starting at specified position, for the specified number of characters, from the specified string value.
202: /// </summary>
203: /// <param name="other">string to initialize with</param>
204: /// <param name="pos">Position in other to start copying charaters from</param>
205: /// <param name="count">Maximum number of characters to copy from other. Default is until end of string. If pos + count is larger than the actual length of the string, string string is copied until the end</param>
206: string::string(const string& other, size_t pos, size_t count /*= npos*/)
207:     : m_buffer{}
208:     , m_end{}
209:     , m_allocatedSize{}
210: {
211:     if (pos >= other.length())
212:         return;
213:     auto size = other.length() - pos;
214:     if (count < size)
215:         size = count;
216:     if (reallocate(size + 1))
217:     {
218:         strncpy(m_buffer, other.data() + pos, size);
219:     }
220:     m_end = m_buffer + size;
221:     m_buffer[size] = NullCharConst;
222: }
223: 
224: /// <summary>
225: /// Const character cast operator
226: ///
227: /// Returns the pointer to the start of the string.
228: /// </summary>
229: string::operator const ValueType* () const
230: {
231:     return data();
232: }
233: 
234: /// <summary>
235: /// Assignment operator
236: ///
237: /// Assigns the specified string value to the string.
238: /// </summary>
239: /// <param name="str">string value to assign to the string</param>
240: /// <returns>A reference to the string</returns>
241: string& string::operator = (const ValueType* str)
242: {
243:     return assign(str);
244: }
245: 
246: /// <summary>
247: /// Assignment operator
248: ///
249: /// Assigns the specified string value to the string.
250: /// </summary>
251: /// <param name="str">string value to assign to the string</param>
252: /// <returns>A reference to the string</returns>
253: string& string::operator = (const string& str)
254: {
255:     return assign(str);
256: }
257: 
258: /// <summary>
259: /// Move operator
260: ///
261: /// Assigns the specified string value to the string by moving the contents of the specified string.
262: /// </summary>
263: /// <param name="str">string value to assign to the string</param>
264: /// <returns>A reference to the string</returns>
265: string& string::operator = (string&& str)
266: {
267:     if (&str != this)
268:     {
269:         m_buffer = str.m_buffer;
270:         m_end = str.m_end;
271:         m_allocatedSize = str.m_allocatedSize;
272:         str.m_buffer = nullptr;
273:         str.m_end = nullptr;
274:         str.m_allocatedSize = 0;
275:     }
276:     return *this;
277: }
278: 
279: /// <summary>
280: /// Non-const iterator to the start of the string
281: ///
282: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
283: /// </summary>
284: /// <returns>iterator to the value type, acting as the start of the string</returns>
285: iterator<string::ValueType> string::begin()
286: {
287:     return iterator(m_buffer, m_end);
288: }
289: 
290: /// <summary>
291: /// Non-const iterator to the end of the string + 1
292: ///
293: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
294: /// </summary>
295: /// <returns>iterator to the value type, acting as the end of the string</returns>
296: iterator<string::ValueType> string::end()
297: {
298:     return iterator(m_end, m_end);
299: }
300: 
301: /// <summary>
302: /// Const iterator to the start of the string
303: ///
304: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
305: /// </summary>
306: /// <returns>const_iterator to the value type, acting as the start of the string</returns>
307: const_iterator<string::ValueType> string::begin() const
308: {
309:     return const_iterator(m_buffer, m_end);
310: }
311: 
312: /// <summary>
313: /// Const iterator to the end of the string + 1
314: ///
315: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
316: /// </summary>
317: /// <returns>const_iterator to the value type, acting as the end of the string</returns>
318: const_iterator<string::ValueType> string::end() const
319: {
320:     return const_iterator(m_end, m_end);
321: }
322: 
323: /// <summary>
324: /// assign a string value
325: ///
326: /// Assigns the specified string value to the string
327: /// </summary>
328: /// <param name="str">string value to assign to the string</param>
329: /// <returns>A reference to the string</returns>
330: string& string::assign(const ValueType* str)
331: {
332:     size_t size{};
333:     if (str != nullptr)
334:     {
335:         size = strlen(str);
336:     }
337:     if ((size + 1) > m_allocatedSize)
338:     {
339:         if (!reallocate(size + 1))
340:             return *this;
341:     }
342:     if (str != nullptr)
343:     {
344:         strncpy(m_buffer, str, size);
345:     }
346:     m_end = m_buffer + size;
347:     m_buffer[size] = NullCharConst;
348:     return *this;
349: }
350: 
351: /// <summary>
352: /// assign a string value
353: ///
354: /// Assigns the specified string value, up to the specified count of characters, to the string.
355: /// </summary>
356: /// <param name="str">string value to assign to the string</param>
357: /// <param name="count">Maximum number of characters to copy from the string. If count is larger than the string length, the length of the string is used</param>
358: /// <returns>A reference to the string</returns>
359: string& string::assign(const ValueType* str, size_t count)
360: {
361:     size_t size{};
362:     if (str != nullptr)
363:     {
364:         size = strlen(str);
365:     }
366:     if (count < size)
367:         size = count;
368:     if ((size + 1) > m_allocatedSize)
369:     {
370:         if (!reallocate(size + 1))
371:             return *this;
372:     }
373:     if (str != nullptr)
374:     {
375:         strncpy(m_buffer, str, size);
376:     }
377:     m_end = m_buffer + size;
378:     m_buffer[size] = NullCharConst;
379:     return *this;
380: }
381: 
382: /// <summary>
383: /// assign a string value
384: ///
385: /// Assigns a string containing the specified count times the specified characters to the string
386: /// </summary>
387: /// <param name="count">Number copies of ch to copy to the string</param>
388: /// <param name="ch">Character to initialize with</param>
389: /// <returns>A reference to the string</returns>
390: string& string::assign(size_t count, ValueType ch)
391: {
392:     auto size = count;
393:     if (size > MaximumStringSize)
394:         size = MaximumStringSize;
395:     if ((size + 1) > m_allocatedSize)
396:     {
397:         if (!reallocate(size + 1))
398:             return *this;
399:     }
400:     memset(m_buffer, ch, size);
401:     m_end = m_buffer + size;
402:     m_buffer[size] = NullCharConst;
403:     return *this;
404: }
405: 
406: /// <summary>
407: /// assign a string value
408: ///
409: /// Assigns the specified string value to the string
410: /// </summary>
411: /// <param name="str">string value to assign to the string</param>
412: /// <returns>A reference to the string</returns>
413: string& string::assign(const string& str)
414: {
415:     auto size = str.length();
416:     if ((size + 1) > m_allocatedSize)
417:     {
418:         if (!reallocate(size + 1))
419:             return *this;
420:     }
421:     strncpy(m_buffer, str.data(), size);
422:     m_end = m_buffer + size;
423:     m_buffer[size] = NullCharConst;
424:     return *this;
425: }
426: 
427: /// <summary>
428: /// assign a string value
429: ///
430: /// Assigns the substring start from the specified position for the specified count of characters of specified string value to the string
431: /// </summary>
432: /// <param name="str">string value to assign to the string</param>
433: /// <param name="pos">Starting position of substring to copy from str</param>
434: /// <param name="count">Maximum number of characters to copy from str.
435: /// Default is until end of string. If pos + count is larger than the stirn length, characters are copied until end of string</param>
436: /// <returns>A reference to the string</returns>
437: string& string::assign(const string& str, size_t pos, size_t count /*= npos*/)
438: {
439:     if (str.empty())
440:         return assign(str);
441: 
442:     if (pos < str.length())
443:     {
444:         auto size = str.length() - pos;
445:         if (count < size)
446:             size = count;
447:         if ((size + 1) > m_allocatedSize)
448:         {
449:             if (!reallocate(size + 1))
450:                 return *this;
451:         }
452:         strncpy(m_buffer, str.data() + pos, size);
453:         m_end = m_buffer + size;
454:         m_buffer[size] = NullCharConst;
455:     }
456:     return *this;
457: }
458: 
459: /// <summary>
460: /// Return the character at specified position
461: /// </summary>
462: /// <param name="pos">Position in string</param>
463: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
464: string::ValueType& string::at(size_t pos)
465: {
466:     if (pos >= length())
467:     {
468:         NullChar = NullCharConst;
469:         return NullChar;
470:     }
471:     return m_buffer[pos];
472: }
473: 
474: /// <summary>
475: /// Return the character at specified position
476: /// </summary>
477: /// <param name="pos">Position in string</param>
478: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, a reference to a const null character is returned (NullCharConst)</returns>
479: const string::ValueType& string::at(size_t pos) const
480: {
481:     if (pos >= length())
482:         return NullCharConst;
483:     return m_buffer[pos];
484: }
485: 
486: /// <summary>
487: /// Return the first character
488: /// </summary>
489: /// <returns>Returns a non-const reference to the first character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
490: string::ValueType& string::front()
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
504: const string::ValueType& string::front() const
505: {
506:     if (empty())
507:         return NullCharConst;
508:     return *m_buffer;
509: }
510: 
511: /// <summary>
512: /// Return the last character
513: /// </summary>
514: /// <returns>Returns a non-const reference to the last character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
515: string::ValueType& string::back()
516: {
517:     if (empty())
518:     {
519:         NullChar = NullCharConst;
520:         return NullChar;
521:     }
522:     return *(m_end - 1);
523: }
524: 
525: /// <summary>
526: /// Return the last character
527: /// </summary>
528: /// <returns>Returns a const reference to the last character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
529: const string::ValueType& string::back() const
530: {
531:     if (empty())
532:         return NullCharConst;
533:     return *(m_end - 1);
534: }
535: 
536: /// <summary>
537: /// Return the character at specified position
538: /// </summary>
539: /// <param name="pos">Position in string</param>
540: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
541: string::ValueType& string::operator[] (size_t pos)
542: {
543:     if (pos >= size())
544:     {
545:         NullChar = '\0';
546:         return NullChar;
547:     }
548:     return m_buffer[pos];
549: }
550: 
551: /// <summary>
552: /// Return the character at specified position
553: /// </summary>
554: /// <param name="pos">Position in string</param>
555: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
556: const string::ValueType& string::operator[] (size_t pos) const
557: {
558:     if (pos >= size())
559:         return NullCharConst;
560:     return m_buffer[pos];
561: }
562: 
563: /// <summary>
564: /// Return the buffer pointer
565: /// </summary>
566: /// <returns>Returns a non-const pointer to the buffer. If the buffer is not allocated, a pointer to a non-const null character (NullChar, initialized before returning) is returned</returns>
567: string::ValueType* string::data()
568: {
569:     NullChar = NullCharConst;
570:     return (m_buffer == nullptr) ? &NullChar : m_buffer;
571: }
572: 
573: /// <summary>
574: /// Return the buffer pointer
575: /// </summary>
576: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
577: const string::ValueType* string::data() const
578: {
579:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
580: }
581: 
582: /// <summary>
583: /// Return the buffer pointer
584: /// </summary>
585: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
586: const string::ValueType* string::c_str() const
587: {
588:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
589: }
590: 
591: /// <summary>
592: /// Determine whether string is empty.
593: /// </summary>
594: /// <returns>Returns true when the string is empty (not allocated or no contents), false otherwise</returns>
595: bool string::empty() const
596: {
597:     return m_end == m_buffer;
598: }
599: 
600: /// <summary>
601: /// Return the size of the string
602: ///
603: /// This method is the equivalent of length().
604: /// </summary>
605: /// <returns>Returns the size (or length) of the string</returns>
606: size_t string::size() const
607: {
608:     return m_end - m_buffer;
609: }
610: 
611: /// <summary>
612: /// Return the length of the string
613: ///
614: /// This method is the equivalent of size().
615: /// </summary>
616: /// <returns>Returns the size (or length) of the string</returns>
617: size_t string::length() const
618: {
619:     return m_end - m_buffer;
620: }
621: 
622: /// <summary>
623: /// Return the capacity of the string
624: ///
625: /// The capacity is the size of the allocated buffer. The string can grow to that length before it needs to be re-allocated.
626: /// </summary>
627: /// <returns>Returns the size (or length) of the string</returns>
628: size_t string::capacity() const
629: {
630:     return m_allocatedSize;
631: }
632: 
633: /// <summary>
634: /// Reserved a buffer capacity
635: ///
636: /// Allocates a buffer of specified size
637: /// </summary>
638: /// <param name="newCapacity"></param>
639: /// <returns>Returns the capacity of the string</returns>
640: size_t string::reserve(size_t newCapacity)
641: {
642:     reallocate_allocation_size(newCapacity);
643:     return m_allocatedSize;
644: }
645: 
646: /// <summary>
647: /// append operator
648: ///
649: /// Appends a character to the string
650: /// </summary>
651: /// <param name="ch">Character to append</param>
652: /// <returns>Returns a reference to the string</returns>
653: string& string::operator +=(ValueType ch)
654: {
655:     append(1, ch);
656:     return *this;
657: }
658: 
659: /// <summary>
660: /// append operator
661: ///
662: /// Appends a string to the string
663: /// </summary>
664: /// <param name="str">string to append</param>
665: /// <returns>Returns a reference to the string</returns>
666: string& string::operator +=(const string& str)
667: {
668:     append(str);
669:     return *this;
670: }
671: 
672: /// <summary>
673: /// append operator
674: ///
675: /// Appends a string to the string
676: /// </summary>
677: /// <param name="str">string to append. If nullptr the nothing is appended</param>
678: /// <returns>Returns a reference to the string</returns>
679: string& string::operator +=(const ValueType* str)
680: {
681:     append(str);
682:     return *this;
683: }
684: 
685: /// <summary>
686: /// append operator
687: ///
688: /// Appends a sequence of count times the same character ch to the string
689: /// </summary>
690: /// <param name="count">Number of characters to append</param>
691: /// <param name="ch">Character to append</param>
692: void string::append(size_t count, ValueType ch)
693: {
694:     auto len = length();
695:     auto strLength = count;
696:     if (strLength > MaximumStringSize - len)
697:         strLength = MaximumStringSize - len;
698:     auto size = len + strLength;
699:     if ((size + 1) > m_allocatedSize)
700:     {
701:         if (!reallocate(size + 1))
702:             return;
703:     }
704:     memset(m_buffer + len, ch, strLength);
705:     m_end = m_buffer + size;
706:     m_buffer[size] = NullCharConst;
707: }
708: 
709: /// <summary>
710: /// append operator
711: ///
712: /// Appends a string to the string
713: /// </summary>
714: /// <param name="str">string to append</param>
715: void string::append(const string& str)
716: {
717:     auto len = length();
718:     auto strLength = str.length();
719:     auto size = len + strLength;
720:     if ((size + 1) > m_allocatedSize)
721:     {
722:         if (!reallocate(size + 1))
723:             return;
724:     }
725:     strncpy(m_buffer + len, str.data(), strLength);
726:     m_end = m_buffer + size;
727:     m_buffer[size] = NullCharConst;
728: }
729: 
730: /// <summary>
731: /// append operator
732: ///
733: /// Appends a substring of str to the string
734: /// </summary>
735: /// <param name="str">string to append from</param>
736: /// <param name="pos">Start position in str to copy characters from</param>
737: /// <param name="count">Number of characters to copy from str. Default is until the end of the string. If count is larger than the string length, characters are copied up to the end of the string</param>
738: void string::append(const string& str, size_t pos, size_t count /*= npos*/)
739: {
740:     if (pos >= str.length())
741:         return;
742:     auto strLength = str.length();
743:     auto strCount = strLength - pos;
744:     if (count < strCount)
745:         strCount = count;
746:     auto len = length();
747:     auto size = len + strCount;
748:     if ((size + 1) > m_allocatedSize)
749:     {
750:         if (!reallocate(size + 1))
751:             return;
752:     }
753:     strncpy(m_buffer + len, str.data() + pos, strCount);
754:     m_end = m_buffer + size;
755:     m_buffer[size] = NullCharConst;
756: }
757: 
758: /// <summary>
759: /// append operator
760: ///
761: /// Appends a string to the string
762: /// </summary>
763: /// <param name="str">string to append. If nullptr the nothing is appended</param>
764: void string::append(const ValueType* str)
765: {
766:     if (str == nullptr)
767:         return;
768:     auto len = length();
769:     auto strLength = strlen(str);
770:     auto size = len + strLength;
771:     if ((size + 1) > m_allocatedSize)
772:     {
773:         if (!reallocate(size + 1))
774:             return;
775:     }
776:     strncpy(m_buffer + len, str, strLength);
777:     m_end = m_buffer + size;
778:     m_buffer[size] = NullCharConst;
779: }
780: 
781: /// <summary>
782: /// append operator
783: ///
784: /// Appends a number of characters from str to the string
785: /// </summary>
786: /// <param name="str">string to append. If nullptr the nothing is appended</param>
787: /// <param name="count">Number of characters to copy from str. If count is larger than the string length, the complete string is copied</param>
788: void string::append(const ValueType* str, size_t count)
789: {
790:     if (str == nullptr)
791:         return;
792:     auto len = length();
793:     auto strLength = strlen(str);
794:     auto strCount = count;
795:     if (strCount > strLength)
796:         strCount = strLength;
797:     auto size = len + strCount;
798:     if ((size + 1) > m_allocatedSize)
799:     {
800:         if (!reallocate(size + 1))
801:             return;
802:     }
803:     strncpy(m_buffer + len, str, strCount);
804:     m_end = m_buffer + size;
805:     m_buffer[size] = NullCharConst;
806: }
807: 
808: /// <summary>
809: /// clear the string
810: ///
811: /// Clears the contents of the string, but does not free or reallocate the buffer
812: /// </summary>
813: void string::clear()
814: {
815:     if (!empty())
816:     {
817:         m_end = m_buffer;
818:         m_buffer[0] = NullCharConst;
819:     }
820: }
821: 
822: /// <summary>
823: /// find a substring in the string
824: ///
825: /// If empty string, always finds the string.
826: /// </summary>
827: /// <param name="str">Substring to find</param>
828: /// <param name="pos">Starting position in string to start searching</param>
829: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
830: size_t string::find(const string& str, size_t pos /*= 0*/) const
831: {
832:     auto len = length();
833:     auto patternLength = str.length();
834:     if (pos >= len)
835:         return npos;
836:     auto needle = str.data();
837:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
838:     {
839:         if (memcmp(haystack, needle, patternLength) == 0)
840:             return haystack - m_buffer;
841:     }
842:     return npos;
843: }
844: 
845: /// <summary>
846: /// find a substring in the string
847: ///
848: /// If nullptr or empty string, always finds the string.
849: /// </summary>
850: /// <param name="str">Substring to find</param>
851: /// <param name="pos">Starting position in string to start searching</param>
852: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
853: size_t string::find(const ValueType* str, size_t pos /*= 0*/) const
854: {
855:     size_t strLength{};
856:     if (str != nullptr)
857:     {
858:         strLength = strlen(str);
859:     }
860:     auto len = length();
861:     auto patternLength = strLength;
862:     if (pos >= len)
863:         return npos;
864:     auto needle = str;
865:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
866:     {
867:         if (memcmp(haystack, needle, patternLength) == 0)
868:             return haystack - m_buffer;
869:     }
870:     return npos;
871: }
872: 
873: /// <summary>
874: /// find a substring in the string
875: ///
876: /// If nullptr or empty string, always finds the string.
877: /// </summary>
878: /// <param name="str">Substring to find</param>
879: /// <param name="pos">Starting position in string to start searching</param>
880: /// <param name="count">Number of characters from str to compare</param>
881: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
882: size_t string::find(const ValueType* str, size_t pos, size_t count) const
883: {
884:     size_t strLength{};
885:     if (str != nullptr)
886:     {
887:         strLength = strlen(str);
888:     }
889:     auto len = length();
890:     auto patternLength = strLength;
891:     if (pos >= len)
892:         return npos;
893:     if (count < patternLength)
894:         patternLength = count;
895:     auto needle = str;
896:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
897:     {
898:         if (memcmp(haystack, needle, patternLength) == 0)
899:             return haystack - m_buffer;
900:     }
901:     return npos;
902: }
903: 
904: /// <summary>
905: /// find a character in the string
906: /// </summary>
907: /// <param name="ch">Character to find</param>
908: /// <param name="pos">Starting position in string to start searching</param>
909: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
910: size_t string::find(ValueType ch, size_t pos /*= 0*/) const
911: {
912:     auto len = length();
913:     if (pos >= len)
914:         return npos;
915:     for (const ValueType* haystack = data() + pos; haystack <= m_end; ++haystack)
916:     {
917:         if (*haystack == ch)
918:             return haystack - m_buffer;
919:     }
920:     return npos;
921: }
922: 
923: /// <summary>
924: /// Check whether string starts with character
925: /// </summary>
926: /// <param name="ch">Character to find</param>
927: /// <returns>Returns true if ch is first character in string, false otherwise</returns>
928: bool string::starts_with(ValueType ch) const
929: {
930:     if (empty())
931:         return false;
932:     return m_buffer[0] == ch;
933: }
934: 
935: /// <summary>
936: /// Check whether string starts with substring
937: /// </summary>
938: /// <param name="str">SubString to find</param>
939: /// <returns>Returns true if str is first part of string, false otherwise</returns>
940: bool string::starts_with(const string& str) const
941: {
942:     auto len = length();
943:     auto strLength = str.length();
944:     if (strLength >= len)
945:         return false;
946: 
947:     return memcmp(data(), str.data(), strLength) == 0;
948: }
949: 
950: /// <summary>
951: /// Check whether string starts with substring
952: /// </summary>
953: /// <param name="str">SubString to find</param>
954: /// <returns>Returns true if str is first part of string, false otherwise</returns>
955: bool string::starts_with(const ValueType* str) const
956: {
957:     size_t strLength{};
958:     if (str != nullptr)
959:     {
960:         strLength = strlen(str);
961:     }
962:     auto len = length();
963:     if (strLength >= len)
964:         return false;
965: 
966:     return memcmp(data(), str, strLength) == 0;
967: }
968: 
969: /// <summary>
970: /// Check whether string ends with character
971: /// </summary>
972: /// <param name="ch">Character to find</param>
973: /// <returns>Returns true if ch is last character in string, false otherwise</returns>
974: bool string::ends_with(ValueType ch) const
975: {
976:     if (empty())
977:         return false;
978:     return m_buffer[length() - 1] == ch;
979: }
980: 
981: /// <summary>
982: /// Check whether string ends with substring
983: /// </summary>
984: /// <param name="str">SubString to find</param>
985: /// <returns>Returns true if str is last part of string, false otherwise</returns>
986: bool string::ends_with(const string& str) const
987: {
988:     auto len = length();
989:     auto strLength = str.length();
990:     if (strLength >= len)
991:         return false;
992: 
993:     return memcmp(m_end - strLength, str.data(), strLength) == 0;
994: }
995: 
996: /// <summary>
997: /// Check whether string ends with substring
998: /// </summary>
999: /// <param name="str">SubString to find</param>
1000: /// <returns>Returns true if str is last part of string, false otherwise</returns>
1001: bool string::ends_with(const ValueType* str) const
1002: {
1003:     size_t strLength{};
1004:     if (str != nullptr)
1005:     {
1006:         strLength = strlen(str);
1007:     }
1008:     auto len = length();
1009:     if (strLength >= len)
1010:         return false;
1011: 
1012:     return memcmp(m_end - strLength, str, strLength) == 0;
1013: }
1014: 
1015: /// <summary>
1016: /// Check whether string contains character
1017: /// </summary>
1018: /// <param name="ch">Character to find</param>
1019: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1020: bool string::contains(ValueType ch) const
1021: {
1022:     return find(ch) != npos;
1023: }
1024: 
1025: /// <summary>
1026: /// Check whether string contains substring
1027: /// </summary>
1028: /// <param name="str">Substring to find</param>
1029: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1030: bool string::contains(const string& str) const
1031: {
1032:     return find(str) != npos;
1033: }
1034: 
1035: /// <summary>
1036: /// Check whether string contains substring
1037: /// </summary>
1038: /// <param name="str">Substring to find</param>
1039: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1040: bool string::contains(const ValueType* str) const
1041: {
1042:     return find(str) != npos;
1043: }
1044: 
1045: /// <summary>
1046: /// Return substring
1047: /// </summary>
1048: /// <param name="pos">Starting position of substring in string</param>
1049: /// <param name="count">length of substring to return. If count is larger than the number of characters available from position pos, the rest of the string is returned</param>
1050: /// <returns>Returns the substring at position [pos, pos + count), if available </returns>
1051: string string::substr(size_t pos /*= 0*/, size_t count /*= npos*/) const
1052: {
1053:     string result;
1054:     auto size = length() - pos;
1055:     if (pos < length())
1056:     {
1057:         if (count < size)
1058:             size = count;
1059:         result.reallocate(size + 1);
1060:         memcpy(result.data(), data() + pos, size);
1061:         result.m_end = result.m_buffer + size;
1062:         result.data()[size] = NullCharConst;
1063:     }
1064: 
1065:     return result;
1066: }
1067: 
1068: /// <summary>
1069: /// Case sensitive equality to string
1070: /// </summary>
1071: /// <param name="other">string to compare to</param>
1072: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1073: bool string::equals(const string& other) const
1074: {
1075:     return compare(other) == 0;
1076: }
1077: 
1078: /// <summary>
1079: /// Case sensitive equality to string
1080: /// </summary>
1081: /// <param name="other">string to compare to</param>
1082: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1083: bool string::equals(const ValueType* other) const
1084: {
1085:     if (other == nullptr)
1086:         return empty();
1087:     if (length() != strlen(other))
1088:         return false;
1089:     return strcmp(data(), other) == 0;
1090: }
1091: 
1092: /// <summary>
1093: /// Case insensitive equality to string
1094: /// </summary>
1095: /// <param name="other">string to compare to</param>
1096: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1097: bool string::equals_case_insensitive(const string& other) const
1098: {
1099:     if (length() != other.length())
1100:         return false;
1101:     if (empty())
1102:         return true;
1103:     return strcasecmp(data(), other.data()) == 0;
1104: }
1105: 
1106: /// <summary>
1107: /// Case insensitive equality to string
1108: /// </summary>
1109: /// <param name="other">string to compare to</param>
1110: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1111: bool string::equals_case_insensitive(const ValueType* other) const
1112: {
1113:     if (other == nullptr)
1114:         return empty();
1115:     if (length() != strlen(other))
1116:         return false;
1117:     return strcasecmp(data(), other) == 0;
1118: }
1119: 
1120: /// <summary>
1121: /// Case sensitive compare to string
1122: ///
1123: /// Compares the complete string, character by character
1124: /// </summary>
1125: /// <param name="str">string to compare to</param>
1126: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1127: int string::compare(const string& str) const
1128: {
1129:     if (empty())
1130:     {
1131:         if (str.empty())
1132:             return 0;
1133:         return -1;
1134:     }
1135:     if (str.empty())
1136:         return 1;
1137: 
1138:     return strcmp(data(), str.data());
1139: }
1140: 
1141: /// <summary>
1142: /// Case sensitive compare to string
1143: ///
1144: /// Compares the substring from pos to pos+count to str
1145: /// </summary>
1146: /// <param name="pos">Starting position of substring to compare to str</param>
1147: /// <param name="count">Number of characters in substring to compare to str</param>
1148: /// <param name="str">string to compare to</param>
1149: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1150: int string::compare(size_t pos, size_t count, const string& str) const
1151: {
1152:     return substr(pos, count).compare(str);
1153: }
1154: 
1155: /// <summary>
1156: /// Case sensitive compare to string
1157: ///
1158: /// Compares the substring from pos to pos+count to the substring from strPos to strPos+strCount of str
1159: /// </summary>
1160: /// <param name="pos">Starting position of substring to compare to str</param>
1161: /// <param name="count">Number of characters in substring to compare to str</param>
1162: /// <param name="str">string to compare to</param>
1163: /// <param name="strPos">Starting position of substring of str to compare</param>
1164: /// <param name="strCount">Number of characters in substring of str to compare</param>
1165: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1166: int string::compare(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/) const
1167: {
1168:     return substr(pos, count).compare(str.substr(strPos, strCount));
1169: }
1170: 
1171: /// <summary>
1172: /// Case sensitive compare to string
1173: ///
1174: /// Compares the complete string to str
1175: /// </summary>
1176: /// <param name="str">string to compare to</param>
1177: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1178: int string::compare(const ValueType* str) const
1179: {
1180:     size_t strLength{};
1181:     if (str != nullptr)
1182:         strLength = strlen(str);
1183:     if (empty())
1184:     {
1185:         if (strLength == 0)
1186:             return 0;
1187:         return -1;
1188:     }
1189:     if (strLength == 0)
1190:         return 1;
1191: 
1192:     return strcmp(data(), str);
1193: }
1194: 
1195: /// <summary>
1196: /// Case sensitive compare to string
1197: ///
1198: /// Compares the substring from pos to pos+count to str
1199: /// </summary>
1200: /// <param name="pos">Starting position of substring to compare to str</param>
1201: /// <param name="count">Number of characters in substring to compare to str</param>
1202: /// <param name="str">string to compare to</param>
1203: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1204: int string::compare(size_t pos, size_t count, const ValueType* str) const
1205: {
1206:     size_t strLength{};
1207:     if (str != nullptr)
1208:         strLength = strlen(str);
1209: 
1210:     auto len = length();
1211:     if (pos >= len)
1212:         len = 0;
1213:     len -= pos;
1214:     if (count < len)
1215:         len = count;
1216:     if (len == 0)
1217:     {
1218:         if (strLength == 0)
1219:             return 0;
1220:         return -1;
1221:     }
1222:     if (strLength == 0)
1223:         return 1;
1224: 
1225:     auto maxLen = strLength;
1226:     if (maxLen < len)
1227:         maxLen = len;
1228:     return strncmp(data() + pos, str, maxLen);
1229: }
1230: 
1231: /// <summary>
1232: /// Case sensitive compare to string
1233: ///
1234: /// Compares the substring from pos to pos+count to the first strCount characters of str
1235: /// </summary>
1236: /// <param name="pos">Starting position of substring to compare to str</param>
1237: /// <param name="count">Number of characters in substring to compare to str</param>
1238: /// <param name="str">string to compare to</param>
1239: /// <param name="strCount">Number of characters in substring of str to compare</param>
1240: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1241: int string::compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const
1242: {
1243:     size_t strLength{};
1244:     if (str != nullptr)
1245:         strLength = strlen(str);
1246: 
1247:     auto len = length();
1248:     if (pos >= len)
1249:         len = 0;
1250:     len -= pos;
1251:     if (count < len)
1252:         len = count;
1253: 
1254:     if (strCount < strLength)
1255:         strLength = strCount;
1256: 
1257:     if (len == 0)
1258:     {
1259:         if (strLength == 0)
1260:             return 0;
1261:         return -1;
1262:     }
1263:     if (strLength == 0)
1264:         return 1;
1265: 
1266:     auto maxLen = strLength;
1267:     if (maxLen < len)
1268:         maxLen = len;
1269:     return strncmp(data() + pos, str, maxLen);
1270: }
1271: 
1272: /// <summary>
1273: /// replace substring
1274: ///
1275: /// Replaces the substring from pos to pos+count with str
1276: /// </summary>
1277: /// <param name="pos">Starting position of substring to replace</param>
1278: /// <param name="count">Number of characters in substring to replace</param>
1279: /// <param name="str">string to replace with</param>
1280: /// <returns>Returns the reference to the resulting string</returns>
1281: string& string::replace(size_t pos, size_t count, const string& str)
1282: {
1283:     string result = substr(0, pos) + str + substr(pos + count);
1284:     assign(result);
1285:     return *this;
1286: }
1287: 
1288: /// <summary>
1289: /// replace substring
1290: ///
1291: /// Replaces the substring from pos to pos+count with the substring from strPos to strPos+strCount of str
1292: /// </summary>
1293: /// <param name="pos">Starting position of substring to replace</param>
1294: /// <param name="count">Number of characters in substring to replace</param>
1295: /// <param name="str">string to replace with</param>
1296: /// <param name="strPos">Starting position of substring of str to replace with</param>
1297: /// <param name="strCount">Number of characters in substring of str to replace with</param>
1298: /// <returns>Returns the reference to the resulting string</returns>
1299: string& string::replace(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/)
1300: {
1301:     string result = substr(0, pos) + str.substr(strPos, strCount) + substr(pos + count);
1302:     assign(result);
1303:     return *this;
1304: }
1305: 
1306: /// <summary>
1307: /// replace substring
1308: ///
1309: /// Replaces the substring from pos to pos+count with str
1310: /// </summary>
1311: /// <param name="pos">Starting position of substring to replace</param>
1312: /// <param name="count">Number of characters in substring to replace</param>
1313: /// <param name="str">string to replace with</param>
1314: /// <returns>Returns the reference to the resulting string</returns>
1315: string& string::replace(size_t pos, size_t count, const ValueType* str)
1316: {
1317:     string result = substr(0, pos) + str + substr(pos + count);
1318:     assign(result);
1319:     return *this;
1320: }
1321: 
1322: /// <summary>
1323: /// replace substring
1324: ///
1325: /// Replaces the substring from pos to pos+count with the first strCount characters of str
1326: /// </summary>
1327: /// <param name="pos">Starting position of substring to replace</param>
1328: /// <param name="count">Number of characters in substring to replace</param>
1329: /// <param name="str">string to replace with</param>
1330: /// <param name="strCount">Number of characters in substring to replace with</param>
1331: /// <returns>Returns the reference to the resulting string</returns>
1332: string& string::replace(size_t pos, size_t count, const ValueType* str, size_t strCount)
1333: {
1334:     string result = substr(0, pos) + string(str, strCount) + substr(pos + count);
1335:     assign(result);
1336:     return *this;
1337: }
1338: 
1339: /// <summary>
1340: /// replace substring
1341: ///
1342: /// Replaces the substring from pos to pos+count with ch
1343: /// </summary>
1344: /// <param name="pos">Starting position of substring to replace</param>
1345: /// <param name="count">Number of characters in substring to replace</param>
1346: /// <param name="ch">Characters to replace with</param>
1347: /// <returns>Returns the reference to the resulting string</returns>
1348: string& string::replace(size_t pos, size_t count, ValueType ch)
1349: {
1350:     return replace(pos, count, ch, 1);
1351: }
1352: 
1353: /// <summary>
1354: /// replace substring
1355: ///
1356: /// Replaces the substring from pos to pos+count with a sequence of chCount copies of ch
1357: /// </summary>
1358: /// <param name="pos">Starting position of substring to replace</param>
1359: /// <param name="count">Number of characters in substring to replace</param>
1360: /// <param name="ch">Characters to replace with</param>
1361: /// <param name="chCount">Number of copies of ch to replace with</param>
1362: /// <returns>Returns the reference to the resulting string</returns>
1363: string& string::replace(size_t pos, size_t count, ValueType ch, size_t chCount)
1364: {
1365:     string result = substr(0, pos) + string(chCount, ch) + substr(pos + count);
1366:     assign(result);
1367:     return *this;
1368: }
1369: 
1370: /// <summary>
1371: /// replace substring
1372: ///
1373: /// Replaces all instances of the substring oldStr (if existing) with newStr
1374: /// </summary>
1375: /// <param name="oldStr">string to find in string</param>
1376: /// <param name="newStr">string to replace with</param>
1377: /// <returns>Returns the number of times the string was replaced</returns>
1378: int string::replace(const string& oldStr, const string& newStr)
1379: {
1380:     size_t pos = find(oldStr);
1381:     size_t oldLength = oldStr.length();
1382:     size_t newLength = newStr.length();
1383:     int count = 0;
1384:     while (pos != npos)
1385:     {
1386:         replace(pos, oldLength, newStr);
1387:         pos += newLength;
1388:         pos = find(oldStr, pos);
1389:         count++;
1390:     }
1391:     return count;
1392: }
1393: 
1394: /// <summary>
1395: /// replace substring
1396: ///
1397: /// Replaces all instances of the substring oldStr (if existing) with newStr
1398: /// </summary>
1399: /// <param name="oldStr">string to find in string</param>
1400: /// <param name="newStr">string to replace with</param>
1401: /// <returns>Returns the number of times the string was replaced</returns>
1402: int string::replace(const ValueType* oldStr, const ValueType* newStr)
1403: {
1404:     if ((oldStr == nullptr) || (newStr == nullptr))
1405:         return 0;
1406:     size_t pos = find(oldStr);
1407:     size_t oldLength = strlen(oldStr);
1408:     size_t newLength = strlen(newStr);
1409:     int count = 0;
1410:     while (pos != npos)
1411:     {
1412:         replace(pos, oldLength, newStr);
1413:         pos += newLength;
1414:         pos = find(oldStr, pos);
1415:         count++;
1416:     }
1417:     return count;
1418: }
1419: 
1420: /// <summary>
1421: /// Align string
1422: ///
1423: /// Pads the string on the left (width > 0) or on the right (width < 0) up to a length of width characters. If the string is larger than width characters, it is not modified.
1424: /// </summary>
1425: /// <param name="width">length of target string. If width < 0, the string is padded to the right with spaces up to -width characters.
1426: /// if width > 0, the string is padded to the left with space up to width characters</param>
1427: /// <returns>Returns the number of times the string was replaced</returns>
1428: string string::align(int width) const
1429: {
1430:     string result;
1431:     int    absWidth = (width > 0) ? width : -width;
1432:     auto   len = length();
1433:     if (static_cast<size_t>(absWidth) > len)
1434:     {
1435:         if (width < 0)
1436:         {
1437:             result = *this + string(static_cast<unsigned int>(-width) - len, ' ');
1438:         }
1439:         else
1440:         {
1441:             result = string(static_cast<unsigned int>(width) - len, ' ') + *this;
1442:         }
1443:     }
1444:     else
1445:         result = *this;
1446:     return result;
1447: }
1448: 
1449: /// <summary>
1450: /// Allocate or re-allocate string to have a capacity of requestedLength characters
1451: /// </summary>
1452: /// <param name="requestedLength">Amount of characters in the string to allocate space for</param>
1453: /// <returns>True if successful, false otherwise</returns>
1454: bool string::reallocate(size_t requestedLength)
1455: {
1456:     auto requestedSize = requestedLength;
1457:     auto allocationSize = NextPowerOf2((requestedSize < MinimumAllocationSize) ? MinimumAllocationSize : requestedSize);
1458: 
1459:     if (!reallocate_allocation_size(allocationSize))
1460:         return false;
1461:     return true;
1462: }
1463: 
1464: /// <summary>
1465: /// Allocate or re-allocate string to have a capacity of allocationSize bytes
1466: /// </summary>
1467: /// <param name="allocationSize">Amount of bytes to allocate space for</param>
1468: /// <returns>True if successful, false otherwise</returns>
1469: bool string::reallocate_allocation_size(size_t allocationSize)
1470: {
1471:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1472:     if (newBuffer == nullptr)
1473:     {
1474:         LOG_NO_ALLOC_DEBUG("Alloc failed!");
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

As you can see, this is a very extensive class.
A few remarks:
- Line 52: We use a minimum allocation size of 64, meaning that every string will have storage space for 63 characters plus a null character for termination, at the least.
- Line 55: We define a constant `MaximumStringSize". We limit strings to a maximum length, which is one less than the largest heap block that can be allocated
- Line 59: We use a special static variable `NullCharConst` when we need to return something constant, but the string is not allocated.
- Line 61: Similarly, we use a special static variable `NullChar` when we need to return something non constant, but the string is not allocated.
This variable is non-const, so will have to be initialized every time we return it, to make sure it is still a null character.
- Line 188-196: We implement a move constructor, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 265-277: We implement a move assignment, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 285-299: We use our own version of a iterator template, which we'll get to, to enable creating the begin() and end() methods. These allow us to use the standard c++ `for (x : string)` construct
- Line 307-321: We also use our own version of a const_iterator template for const iterators, which we'll get to, to enable creating the begin() and end() const methods.
- Line 1020-1043: We also have the `contains()` methods, which are new to C++23, however they are quite convenient
- Line 1073-1118: We use the methods `equals()` and `equals_case_insensititive()` to compare strings. Both are not standard, but convenient
- Line 1378-1418: We implement two extra variants of `replace()` (replacing multiple instances of a string) for convenience
- Line 1428-1447: Again for convenience, we implement the `align()` method. We'll be using this later on for aligning strings when formatting

### Iterator.h {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_ITERATORH}

As said before, we use our own version of const_iterator and iterator to be used in the `string` methods `begin()` and `end()`.
We'll declare and implement these.

Create the file `code/libraries/baremetal/include/baremetal/Iterator.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Iterator.h
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2024 Rene Barto
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
42: /// @file
43: /// Iterator classes
44: 
45: /// <summary>
46: /// Const iterator template
47: ///
48: /// Based on arrays of type T pointed to by const pointers.
49: /// </summary>
50: /// <typeparam name="T">Value type to iterate through</typeparam>
51: template<class T>
52: class const_iterator
53: {
54: private:
55:     /// @brief Start of the range
56:     T const* m_begin;
57:     /// @brief End of the range + 1
58:     T const* m_end;
59:     /// @brief Current position
60:     T const* m_current;
61: 
62: public:
63:     /// <summary>
64:     /// Construct a const_iterator
65:     /// </summary>
66:     /// <param name="begin">Start of the range</param>
67:     /// <param name="end">End of the range + 1</param>
68:     explicit const_iterator(const T* begin, const T* end)
69:         : m_begin{ begin }
70:         , m_end{ end }
71:         , m_current{ begin }
72:     {
73:     }
74:     /// <summary>
75:     /// Advance in range by 1 step (post increment)
76:     /// </summary>
77:     /// <returns>Position after advancing as const_iterator</returns>
78:     const_iterator& operator++()
79:     {
80:         ++m_current;
81:         return *this;
82:     }
83:     /// <summary>
84:     /// Advance in range by 1 step (pre increment)
85:     /// </summary>
86:     /// <returns>Position before advancing as const_iterator</returns>
87:     const_iterator operator++(int)
88:     {
89:         const_iterator retval = *this;
90:         ++(*this);
91:         return retval;
92:     }
93:     /// <summary>
94:     /// Equality comparison
95:     ///
96:     /// Compares current position with current position in passed iterator
97:     /// </summary>
98:     /// <param name="other">const_iterator to compare with</param>
99:     /// <returns>Returns true if positions are equal, false if not</returns>
100:     bool operator==(const const_iterator& other) const
101:     {
102:         return m_current == other.m_current;
103:     }
104:     /// <summary>
105:     /// Inequality comparison
106:     ///
107:     /// Compares current position with current position in passed iterator
108:     /// </summary>
109:     /// <param name="other">const_iterator to compare with</param>
110:     /// <returns>Returns false if positions are equal, true if not</returns>
111:     bool operator!=(const const_iterator& other) const
112:     {
113:         return !(*this == other);
114:     }
115:     /// <summary>
116:     /// Return value at current location
117:     /// </summary>
118:     /// <returns>Value to which the current position points</returns>
119:     T const& operator*() const
120:     {
121:         return *m_current;
122:     }
123: };
124: 
125: /// <summary>
126: /// Non-const iterator template
127: ///
128: /// Based on arrays of type T pointed to by pointers.
129: /// </summary>
130: /// <typeparam name="T">Value type to iterate through</typeparam>
131: template<class T>
132: class iterator
133: {
134: private:
135:     /// @brief Start of the range
136:     T* m_begin;
137:     /// @brief End of the range + 1
138:     T* m_end;
139:     /// @brief Current position
140:     T* m_current;
141: 
142: public:
143:     /// <summary>
144:     /// Construct a iterator
145:     /// </summary>
146:     /// <param name="begin">Start of the range</param>
147:     /// <param name="end">End of the range + 1</param>
148:     explicit iterator(T* begin, T* end)
149:         : m_begin{ begin }
150:         , m_end{ end }
151:         , m_current{ begin }
152:     {
153:     }
154:     /// <summary>
155:     /// Advance in range by 1 step (post increment)
156:     /// </summary>
157:     /// <returns>Position after advancing as iterator</returns>
158:     iterator& operator++()
159:     {
160:         ++m_current;
161:         return *this;
162:     }
163:     /// <summary>
164:     /// Advance in range by 1 step (pre increment)
165:     /// </summary>
166:     /// <returns>Position before advancing as iterator</returns>
167:     iterator operator++(int)
168:     {
169:         iterator retval = *this;
170:         ++(*this);
171:         return retval;
172:     }
173:     /// <summary>
174:     /// Equality comparison
175:     ///
176:     /// Compares current position with current position in passed iterator
177:     /// </summary>
178:     /// <param name="other">iterator to compare with</param>
179:     /// <returns>Returns true if positions are equal, false if not</returns>
180:     bool operator==(const iterator& other) const
181:     {
182:         return m_current == other.m_current;
183:     }
184:     /// <summary>
185:     /// Inequality comparison
186:     ///
187:     /// Compares current position with current position in passed iterator
188:     /// </summary>
189:     /// <param name="other">iterator to compare with</param>
190:     /// <returns>Returns false if positions are equal, true if not</returns>
191:     bool operator!=(const iterator& other) const
192:     {
193:         return !(*this == other);
194:     }
195:     /// <summary>
196:     /// Return value at current location
197:     /// </summary>
198:     /// <returns>Value to which the current position points</returns>
199:     T& operator*() const
200:     {
201:         return *m_current;
202:     }
203: };
```

Again, we will not go into any detail, the classes should speak for themselves.

### Util.h {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_UTILH}

As our implementation of the `string` class needs some additional string specific functionality, we'll add a number of standard C library functions for this/

Update the file `code/libraries/baremetal/include/baremetal/Util.h`

```cpp
File: code/libraries/baremetal/include/baremetal/Util.h
...
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
65: void* malloc(size_t size);
66: void* calloc(size_t num, size_t size);
67: void* realloc(void* ptr, size_t new_size);
68: void free(void* ptr);
69: 
70: #ifdef __cplusplus
71: }
72: #endif
73:
74: /// <summary>
75: /// Determine the number of bits needed to represent the specified value
76: /// </summary>
77: /// <param name="value">Value to check</param>
78: /// <returns>Number of bits used for value</returns>
79: inline constexpr unsigned NextPowerOf2Bits(size_t value)
80: {
81:     unsigned bitCount{ 0 };
82:     size_t temp = value;
83:     while (temp >= 1)
84:     {
85:         ++bitCount;
86:         temp >>= 1;
87:     }
88:     return bitCount;
89: }
90:
91: /// <summary>
92: /// Determine the next power of 2 greater than or equal to the specified value
93: /// </summary>
94: /// <param name="value">Value to check</param>
95: /// <returns>Power of two greater or equal to value</returns>
96: inline constexpr size_t NextPowerOf2(size_t value)
97: {
98:     return 1 << NextPowerOf2Bits((value != 0) ? value - 1 : 0);
99: }
```

- Line 53: We add the standard `memcmp()` function
- Line 55-56: We add the standard `toupper()` amd `tolower()` functions
- Line 58-61: We add the standard `strcmp()` (compare strings to end of string), `strncmp()` (compare strings with maximum size), and the case insensitive variants `strcasecmp()` and `strncasecmp()`
- Line 79-89: We add an inline function `NextPowerOf2Bits()` to compute the number of bits b required to hold a value n, so 2^b >= n
- Line 96-99: We add an inline function `NextPowerOf2()` to compute the value x larger or equal to a number n, such that x is a power of 2

### Util.cpp {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_UTILCPP}

We'll implement the new utility functions.

Update the file `code/libraries/baremetal/src/Util.cpp`

```cpp
File: code/libraries/baremetal/src/Util.cpp
...
84: /// <summary>
85: /// Compare two regions of memory
86: /// </summary>
87: /// <param name="buffer1">Pointer to first memory buffer</param>
88: /// <param name="buffer2">Pointer to second memory buffer</param>
89: /// <param name="length">Number of bytes to compare</param>
90: /// <returns>Returns 0 if the two regions are equal,
91: /// 1 if the values in the first buffer are greater,
92: /// -1 if the values in the second buffer are greater</returns>
93: int memcmp(const void* buffer1, const void* buffer2, size_t length)
94: {
95:     const unsigned char* p1 = reinterpret_cast<const unsigned char*>(buffer1);
96:     const unsigned char* p2 = reinterpret_cast<const unsigned char*>(buffer2);
97:
98:     while (length-- > 0)
99:     {
100:         if (*p1 > *p2)
101:         {
102:             return 1;
103:         }
104:         else if (*p1 < *p2)
105:         {
106:             return -1;
107:         }
108:
109:         p1++;
110:         p2++;
111:     }
112:
113:     return 0;
114: }
115:
116: /// <summary>
117: /// Convert character to upper case
118: /// </summary>
119: /// <param name="c">Character</param>
120: /// <returns>Uper case version of character c, if alphabetic, otherwise c</returns>
121: int toupper(int c)
122: {
123:     if (('a' <= c) && (c <= 'z'))
124:     {
125:         c -= 'a' - 'A';
126:     }
127:
128:     return c;
129: }
130:
131: /// <summary>
132: /// Convert character to lower case
133: /// </summary>
134: /// <param name="c">Character</param>
135: /// <returns>Lower case version of character c, if alphabetic, otherwise c</returns>
136: int tolower(int c)
137: {
138:     if (('A' <= c) && (c <= 'Z'))
139:     {
140:         c += 'a' - 'A';
141:     }
142:
143:     return c;
144: }
...
163: /// <summary>
164: /// Compare two strings
165: /// </summary>
166: /// <param name="str1">Pointer to first string</param>
167: /// <param name="str2">Pointer to second string</param>
168: /// <returns>Returns 0 if the two strings are equal,
169: /// 1 if the values in the first string are greater,
170: /// -1 if the values in the second string are greater</returns>
171: int strcmp(const char* str1, const char* str2)
172: {
173:     while ((*str1 != '\0') && (*str2 != '\0'))
174:     {
175:         if (*str1 > *str2)
176:         {
177:             return 1;
178:         }
179:         else if (*str1 < *str2)
180:         {
181:             return -1;
182:         }
183:
184:         str1++;
185:         str2++;
186:     }
187:
188:     if (*str1 > *str2)
189:     {
190:         return 1;
191:     }
192:     else if (*str1 < *str2)
193:     {
194:         return -1;
195:     }
196:
197:     return 0;
198: }
199:
200: /// <summary>
201: /// Compare two strings, case insensitive
202: /// </summary>
203: /// <param name="str1">Pointer to first string</param>
204: /// <param name="str2">Pointer to second string</param>
205: /// <returns>Returns 0 is the two strings are equal ignoring case,
206: /// 1 if the values in the first string are greater,
207: /// -1 if the values in the second string are greater</returns>
208: int strcasecmp(const char* str1, const char* str2)
209: {
210:     int chr1, chr2;
211:
212:     while (((chr1 = toupper(*str1)) != '\0') && ((chr2 = toupper(*str2)) != '\0'))
213:     {
214:         if (chr1 > chr2)
215:         {
216:             return 1;
217:         }
218:         else if (chr1 < chr2)
219:         {
220:             return -1;
221:         }
222:
223:         str1++;
224:         str2++;
225:     }
226:
227:     chr2 = toupper(*str2);
228:
229:     if (chr1 > chr2)
230:     {
231:         return 1;
232:     }
233:     else if (chr1 < chr2)
234:     {
235:         return -1;
236:     }
237:
238:     return 0;
239: }
240:
241: /// <summary>
242: /// Compare two strings, up to maxLen characters
243: /// </summary>
244: /// <param name="str1">Pointer to first string</param>
245: /// <param name="str2">Pointer to second string</param>
246: /// <param name="maxLen">Maximum number of characters to compare</param>
247: /// <returns>Returns 0 is the two strings are equal,
248: /// 1 if the values in the first string are greater,
249: /// -1 if the values in the second string are greater</returns>
250: int strncmp(const char* str1, const char* str2, size_t maxLen)
251: {
252:     while ((maxLen > 0) && (*str1 != '\0') && (*str2 != '\0'))
253:     {
254:         if (*str1 > *str2)
255:         {
256:             return 1;
257:         }
258:         else if (*str1 < *str2)
259:         {
260:             return -1;
261:         }
262:
263:         maxLen--;
264:         str1++;
265:         str2++;
266:     }
267:
268:     if (maxLen == 0)
269:     {
270:         return 0;
271:     }
272:
273:     if (*str1 > *str2)
274:     {
275:         return 1;
276:     }
277:     else if (*str1 < *str2)
278:     {
279:         return -1;
280:     }
281:
282:     return 0;
283: }
284:
285: /// <summary>
286: /// Compare two strings, up to maxLen characters, case insensitive
287: /// </summary>
288: /// <param name="str1">Pointer to first string</param>
289: /// <param name="str2">Pointer to second string</param>
290: /// <param name="maxLen">Maximum number of characters to compare</param>
291: /// <returns>Returns 0 is the two strings are equal ignoring case,
292: /// 1 if the values in the first string are greater,
293: /// -1 if the values in the second string are greater</returns>
294: int strncasecmp(const char* str1, const char* str2, size_t maxLen)
295: {
296:     int chr1, chr2;
297:
298:     while ((maxLen > 0) && ((chr1 = toupper(*str1)) != '\0') && ((chr2 = toupper(*str2)) != '\0'))
299:     {
300:         if (chr1 > chr2)
301:         {
302:             return 1;
303:         }
304:         else if (chr1 < chr2)
305:         {
306:             return -1;
307:         }
308:
309:         maxLen--;
310:         str1++;
311:         str2++;
312:     }
313:
314:     chr2 = toupper(*str2);
315:
316:     if (maxLen == 0)
317:     {
318:         return 0;
319:     }
320:
321:     if (chr1 > chr2)
322:     {
323:         return 1;
324:     }
325:     else if (chr1 < chr2)
326:     {
327:         return -1;
328:     }
329:
330:     return 0;
331: }
```

The new functions added should be self-explanatory.

### Update project configuration {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
29: set(PROJECT_SOURCES
30:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/CXAGuard.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
51:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
52:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp
53:     )
54:
55: set(PROJECT_INCLUDES_PUBLIC
56:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/ARMInstructions.h
57:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Assert.h
58:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/BCMRegisters.h
59:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/CharDevice.h
60:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Console.h
61:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Format.h
62:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/HeapAllocator.h
63:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IGPIOPin.h
64:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMailbox.h
65:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/IMemoryAccess.h
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Macros.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/StdArg.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Types.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
86:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Util.h
87:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
88:     )
89: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_APPLICATION_CODE}

We'll start making use of the string class we just added, but we'll do it in a way that shows that the string methods function as expected.
This is a first attempt at creating class / micro / unit tests for our code, which we will start doing soon.

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
26:     string s1{ "a" };
27:     string s2{ "a" };
28:     string s3{ "aaaa", 3 };
29:     string s4{ 4, 'b' };
30:     string s5{ s3 };
31:     string s6{ s3, 1 };
32:     string s7{ s3, 1, 1 };
33:     string s8{ nullptr };
34:     string s9{ "" };
35:     string s10{ nullptr, 3 };
36:
37:     LOG_INFO("s1");
38:     for (auto ch : s1)
39:     {
40:         LOG_INFO("%c", ch);
41:     }
42:     assert(strcmp(s1, "a") == 0);
43:     assert(strcmp(s2, "a") == 0);
44:     assert(strcmp(s3, "aaa") == 0);
45:     assert(strcmp(s4, "bbbb") == 0);
46:     assert(strcmp(s5, "aaa") == 0);
47:     assert(strcmp(s6, "aa") == 0);
48:     assert(strcmp(s7, "a") == 0);
49:     assert(strcmp(s8, "") == 0);
50:     assert(strcmp(s9, "") == 0);
51:     assert(strcmp(s10, "") == 0);
52:
53:     s7 = "abcde";
54:     assert(strcmp(s7, "abcde") == 0);
55:     s7 = nullptr;
56:     assert(strcmp(s7, "") == 0);
57:     s7 = "";
58:     assert(strcmp(s7, "") == 0);
59:     s6 = s4;
60:     assert(strcmp(s6, "bbbb") == 0);
61:     {
62:         string s99{ "cccc" };
63:         s6 = s99;
64:     }
65:     assert(strcmp(s6, "cccc") == 0);
66:
67:     const char* s = "abcdefghijklmnopqrstuvwxyz";
68:     s1.assign(s);
69:     assert(strcmp(s1, "abcdefghijklmnopqrstuvwxyz") == 0);
70:     s1.assign("");
71:     assert(strcmp(s1, "") == 0);
72:     s1.assign(nullptr);
73:     assert(strcmp(s1, "") == 0);
74:     s1.assign(s, 6);
75:     assert(strcmp(s1, "abcdef") == 0);
76:     s1.assign("", 6);
77:     assert(strcmp(s1, "") == 0);
78:     s1.assign(nullptr, 6);
79:     assert(strcmp(s1, "") == 0);
80:     s8 = s;
81:     s1.assign(s8, 3);
82:     assert(strcmp(s1, "defghijklmnopqrstuvwxyz") == 0);
83:     s1.assign(s8, 4, 6);
84:     assert(strcmp(s1, "efghij") == 0);
85:     s1.assign(6, 'c');
86:     assert(strcmp(s1, "cccccc") == 0);
87:
88:     const string s8c{ s8 };
89:     assert(s8.at(3) == 'd');
90:     assert(s8c.at(3) == 'd');
91:     assert(s8.front() == 'a');
92:     assert(s8c.front() == 'a');
93:     assert(s8.back() == 'z');
94:     assert(s8c.back() == 'z');
95:     assert(s8[3] == 'd');
96:     assert(s8c[3] == 'd');
97:     assert(s8c.capacity() == 64);
98:     assert(s8.reserve(128) == 128);
99:     assert(s8.capacity() == 128);
100:
101:     s1 = "a";
102:     assert(strcmp(s1, "a") == 0);
103:     s1 += 'b';
104:     assert(strcmp(s1, "ab") == 0);
105:     s2 = "a";
106:     s1 += s2;
107:     assert(strcmp(s1, "aba") == 0);
108:     s1 += "abcde";
109:     assert(strcmp(s1, "abaabcde") == 0);
110:     s1 = "a";
111:     s1 += "";
112:     assert(strcmp(s1, "a") == 0);
113:     s1 += nullptr;
114:     assert(strcmp(s1, "a") == 0);
115:
116:     s3 = "";
117:     s4 = s1 + s2;
118:     assert(strcmp(s4, "aa") == 0);
119:     s4 = s1 + s3;
120:     assert(strcmp(s4, "a") == 0);
121:     s4 = s1 + "b";
122:     assert(strcmp(s4, "ab") == 0);
123:     s4 = s1 + "";
124:     assert(strcmp(s4, "a") == 0);
125:     s4 = s1 + nullptr;
126:     assert(strcmp(s4, "a") == 0);
127:     s4 = "b" + s1;
128:     assert(strcmp(s4, "ba") == 0);
129:     s4 = "" + s1;
130:     assert(strcmp(s4, "a") == 0);
131:     s4 = nullptr + s1;
132:     assert(strcmp(s4, "a") == 0);
133:
134:     s1 = "a";
135:     s1.append(4, 'b');
136:     assert(strcmp(s1, "abbbb") == 0);
137:     s1.append(s2);
138:     assert(strcmp(s1, "abbbba") == 0);
139:     s1.append(s8, 3, 5);
140:     assert(strcmp(s1, "abbbbadefgh") == 0);
141:     s1.append("ccc");
142:     assert(strcmp(s1, "abbbbadefghccc") == 0);
143:     s1.append("dddddd", 3);
144:     assert(strcmp(s1, "abbbbadefghcccddd") == 0);
145:     s1.clear();
146:     assert(strcmp(s1, "") == 0);
147:     s1.append("");
148:     assert(strcmp(s1, "") == 0);
149:     s1.append(nullptr);
150:     assert(strcmp(s1, "") == 0);
151:     s1.append("", 3);
152:     assert(strcmp(s1, "") == 0);
153:     s1.append(nullptr, 3);
154:     assert(strcmp(s1, "") == 0);
155:
156:     s1 = s;
157:     s2 = "c";
158:     auto pos = s1.find(s2);
159:     assert(pos == 2);
160:     pos = s1.find(s2, 1);
161:     assert(pos == 2);
162:     pos = s1.find(s2, 3);
163:     assert(pos == string::npos);
164:     s2 = "deg";
165:     pos = s1.find(s2, 3);
166:     assert(pos == string::npos);
167:     pos = s1.find(s2, 3, 2);
168:     assert(pos == 3);
169:     s2 = "xyz";
170:     pos = s1.find(s2);
171:     assert(pos == 23);
172:
173:     pos = s1.find("d");
174:     assert(pos == 3);
175:     pos = s1.find("d", 1);
176:     assert(pos == 3);
177:     pos = s1.find("d", 4);
178:     assert(pos == string::npos);
179:     pos = s1.find("def", 2);
180:     assert(pos == 3);
181:     pos = s1.find("deg", 2);
182:     assert(pos == string::npos);
183:     pos = s1.find("deg", 2, 2);
184:     assert(pos == 3);
185:     pos = s1.find("xyz");
186:     assert(pos == 23);
187:     pos = s1.find("");
188:     assert(pos == 0);
189:     pos = s1.find(nullptr);
190:     assert(pos == 0);
191:     pos = s1.find("", 2);
192:     assert(pos == 2);
193:     pos = s1.find(nullptr, 2);
194:     assert(pos == 2);
195:     pos = s1.find(nullptr, 26);
196:     assert(pos == string::npos);
197:     pos = s1.find("", 2, 2);
198:     assert(pos == 2);
199:     pos = s1.find(nullptr, 2, 2);
200:     assert(pos == 2);
201:     pos = s1.find(nullptr, 26, 1);
202:     assert(pos == string::npos);
203:
204:     pos = s1.find('d');
205:     assert(pos == 3);
206:     pos = s1.find('d', 2);
207:     assert(pos == 3);
208:     pos = s1.find('d', 4);
209:     assert(pos == string::npos);
210:     pos = s1.find('A');
211:     assert(pos == string::npos);
212:     pos = s1.find("z");
213:     assert(pos == 25);
214:
215:     s2 = "abc";
216:     s3 = "xyz";
217:     auto isTrue = s1.starts_with('a');
218:     assert(isTrue);
219:     isTrue = s1.starts_with('z');
220:     assert(!isTrue);
221:     isTrue = s1.starts_with("abc");
222:     assert(isTrue);
223:     isTrue = s1.starts_with("xyz");
224:     assert(!isTrue);
225:     isTrue = s1.starts_with("");
226:     assert(isTrue);
227:     isTrue = s1.starts_with(nullptr);
228:     assert(isTrue);
229:     isTrue = s1.starts_with(s2);
230:     assert(isTrue);
231:     isTrue = s1.starts_with(s3);
232:     assert(!isTrue);
233:
234:     isTrue = s1.ends_with('a');
235:     assert(!isTrue);
236:     isTrue = s1.ends_with('z');
237:     assert(isTrue);
238:     isTrue = s1.ends_with("abc");
239:     assert(!isTrue);
240:     isTrue = s1.ends_with("xyz");
241:     assert(isTrue);
242:     isTrue = s1.ends_with("");
243:     assert(isTrue);
244:     isTrue = s1.ends_with(nullptr);
245:     assert(isTrue);
246:     isTrue = s1.ends_with(s2);
247:     assert(!isTrue);
248:     isTrue = s1.ends_with(s3);
249:     assert(isTrue);
250:
251:     isTrue = s1.contains('a');
252:     assert(isTrue);
253:     isTrue = s1.contains('A');
254:     assert(!isTrue);
255:     isTrue = s1.contains("abc");
256:     assert(isTrue);
257:     isTrue = s1.contains("XYZ");
258:     assert(!isTrue);
259:     isTrue = s1.contains("");
260:     assert(isTrue);
261:     isTrue = s1.contains(nullptr);
262:     assert(isTrue);
263:     isTrue = s1.contains(s2);
264:     assert(isTrue);
265:     isTrue = s1.contains(s3);
266:     assert(isTrue);
267:
268:     s2 = s1.substr();
269:     assert(strcmp(s2, "abcdefghijklmnopqrstuvwxyz") == 0);
270:     s2 = s1.substr(6);
271:     assert(strcmp(s2, "ghijklmnopqrstuvwxyz") == 0);
272:     s2 = s1.substr(6, 6);
273:     assert(strcmp(s2, "ghijkl") == 0);
274:
275:     s1 = "abcdefg";
276:     s2 = "abcdefG";
277:     s3 = "abcdefg";
278:     isTrue = s1.equals(s2);
279:     assert(!isTrue);
280:     isTrue = s1.equals(s3);
281:     assert(isTrue);
282:     isTrue = s1.equals("abcefg");
283:     assert(!isTrue);
284:     isTrue = s1.equals("abcdefg");
285:     assert(isTrue);
286:     isTrue = s1.equals("");
287:     assert(!isTrue);
288:     isTrue = s1.equals(nullptr);
289:     assert(!isTrue);
290:     s4 = "";
291:     isTrue = s4.equals_case_insensitive(s3);
292:     assert(!isTrue);
293:     isTrue = s4.equals("");
294:     assert(isTrue);
295:     isTrue = s4.equals(nullptr);
296:     assert(isTrue);
297:
298:     isTrue = s1.equals_case_insensitive(s2);
299:     assert(isTrue);
300:     isTrue = s1.equals_case_insensitive(s3);
301:     assert(isTrue);
302:     isTrue = s1.equals_case_insensitive("abcefg");
303:     assert(!isTrue);
304:     isTrue = s1.equals_case_insensitive("abcdefg");
305:     assert(isTrue);
306:     isTrue = s1.equals_case_insensitive("");
307:     assert(!isTrue);
308:     isTrue = s1.equals_case_insensitive(nullptr);
309:     assert(!isTrue);
310:     s4 = "";
311:     isTrue = s4.equals_case_insensitive(s3);
312:     assert(!isTrue);
313:     isTrue = s4.equals_case_insensitive("");
314:     assert(isTrue);
315:     isTrue = s4.equals_case_insensitive(nullptr);
316:     assert(isTrue);
317:
318:     assert(s1 == s3);
319:     assert(s1 != s2);
320:     assert(s1 == "abcdefg");
321:     assert(s1 != "abcdefG");
322:     assert(s1 != "");
323:     assert(s1 != nullptr);
324:     assert("abcdefg" == s1);
325:     assert("abcdefG" != s1);
326:     assert("" != s1);
327:     assert(nullptr != s1);
328:     assert(s4 != s3);
329:     assert(s4 == "");
330:     assert(s4 == nullptr);
331:     assert("" == s4);
332:     assert(nullptr == s4);
333:
334:     s4 = "bcdefg";
335:     s5 = "def";
336:     auto result = s1.compare(s2);
337:     assert(result == 1);
338:     result = s2.compare(s1);
339:     assert(result == -1);
340:     result = s1.compare(s3);
341:     assert(result == 0);
342:     result = s3.compare(s1);
343:     assert(result == 0);
344:     result = s1.compare(1, 6, s4);
345:     assert(result == 0);
346:     result = s1.compare(1, 5, s4);
347:     assert(result == -1);
348:     result = s1.compare(3, 6, s4, 2);
349:     assert(result == 0);
350:     result = s1.compare(3, 6, s4, 2, 1);
351:     assert(result == 1);
352:     result = s1.compare(3, 3, s4, 2, 6);
353:     assert(result == -1);
354:
355:     result = s1.compare("a");
356:     assert(result == 1);
357:     result = s1.compare("Abcdefg");
358:     assert(result == 1);
359:     result = s1.compare("abdecfg");
360:     assert(result == -1);
361:     result = s1.compare("");
362:     assert(result == 1);
363:     result = s1.compare(nullptr);
364:     assert(result == 1);
365:     s2 = "";
366:     result = s2.compare("a");
367:     assert(result == -1);
368:     result = s2.compare("");
369:     assert(result == 0);
370:     result = s2.compare(nullptr);
371:     assert(result == 0);
372:
373:     s1 = "abcde";
374:     s2 = "fghijk";
375:     s3 = s1.replace(0, 1, s2);
376:     assert(s1.equals("fghijkbcde"));
377:     assert(s3.equals("fghijkbcde"));
378:     s1 = "abcde";
379:     s3 = s1.replace(1, 2, s2, 2);
380:     assert(s1.equals("ahijkde"));
381:     assert(s3.equals("ahijkde"));
382:     s1 = "abcde";
383:     s3 = s1.replace(1, 2, s2, 2, 2);
384:     assert(s1.equals("ahide"));
385:     assert(s3.equals("ahide"));
386:     s1 = "abcde";
387:     s3 = s1.replace(0, 1, "uvwxyz");
388:     assert(s1.equals("uvwxyzbcde"));
389:     assert(s3.equals("uvwxyzbcde"));
390:     s1 = "abcde";
391:     s3 = s1.replace(1, 2, "uvwxyz", 2);
392:     assert(s1.equals("auvde"));
393:     assert(s3.equals("auvde"));
394:     s1 = "abcde";
395:     s3 = s1.replace(0, 1, 'x');
396:     assert(s1.equals("xbcde"));
397:     assert(s3.equals("xbcde"));
398:     s1 = "abcde";
399:     s3 = s1.replace(1, 2, 'x', 3);
400:     assert(s1.equals("axxxde"));
401:     assert(s3.equals("axxxde"));
402:
403:     s1 = "abcde";
404:     s2 = "cd";
405:     s3 = "xy";
406:     int count = s1.replace(s2, s3);
407:     assert(count == 1);
408:     assert(s1.equals("abxye"));
409:     s1 = "abababab";
410:     s2 = "ab";
411:     s3 = "cd";
412:     count = s1.replace(s2, s3);
413:     assert(count == 4);
414:     assert(s1.equals("cdcdcdcd"));
415:     s1 = "abcde";
416:     count = s1.replace("cd", "xy");
417:     assert(count == 1);
418:     assert(s1.equals("abxye"));
419:     s1 = "abababab";
420:     count = s1.replace("ab", "cd");
421:     assert(count == 4);
422:     assert(s1.equals("cdcdcdcd"));
423:
424:     s1 = "abcd";
425:     s2 = s1.align(8);
426:     assert(s2.equals("    abcd"));
427:     s2 = s1.align(-8);
428:     assert(s2.equals("abcd    "));
429:     s2 = s1.align(0);
430:     assert(s2.equals("abcd"));
431:
432:     LOG_INFO("Wait 5 seconds");
433:     Timer::WaitMilliSeconds(5000);
434:
435:     console.Write("Press r to reboot, h to halt, p to fail assertion and panic\n");
436:     char ch{};
437:     while ((ch != 'r') && (ch != 'h') && (ch != 'p'))
438:     {
439:         ch = console.ReadChar();
440:         console.WriteChar(ch);
441:     }
442:     if (ch == 'p')
443:         assert(false);
444:
445:     return static_cast<int>((ch == 'r') ? ReturnCode::ExitReboot : ReturnCode::ExitHalt);
446: }
```

As you can see the code has grown quite abit due to all the tests that we perform.

- Line 11: we need to include `String.h`
- Line 26-35: We construct some strings, in different ways, to cover all the variants of constructors
- Line 37-41: We use a for loop to iterate through the characters in the string. This will use the `begin()` and `end()` methods
- Line 42-51: We check whether the strings are initialized as expected. Notice that we use the `assert` macro here
- Line 53-65: We test assignment operators
- Line 67-86: We test the `assign()` methods
- Line 88-99: We test the `at()`, `front()`, `back()`, `capacity()` and `reserve()` methods, as well as the index operator `[]`
- Line 101-114: We test the addition assignment (in this case concatenation) operator `+=`
- Line 116-132: We test the addition (concatenation) operator `+`
- Line 134-154: We test the `append()` methods
- Line 156-213: We test the `find()` methods
- Line 215-232: We test the `starts_with()` methods
- Line 234-249: We test the `ends_with()` methods
- Line 251-266: We test the `contains()` methods
- Line 268-273: We test the `substr()` methods
- Line 275-316: We test the `equals()` and `equals_case_insensitive()` methods
- Line 318-332: We test the equality operators `==` and the inequality operators `!=`
- Line 334-371: We test the `compare()` methods
- Line 373-422: We test the `replace()` methods
- Line 424-430: We test the `align()` method

### Configuring, building and debugging {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_CONFIGURING_BUILDING_AND_DEBUGGING}

We can now configure and build our code, and start debugging.

The application will print the characters in the string "bbbb" in log statements.
Next to this nothing special is shown, as the tests will all succeed. If a test were to fail, the assertion would fire, and the application would crash.
This is a way to test code, however we'd like to see all of the failure found in our code, not have the first one crash the application.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:79)
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
Info   s4 (main:37)
Info   b (main:40)
Info   b (main:40)
Info   b (main:40)
Info   b (main:40)
Info   Wait 5 seconds (main:432)
Press r to reboot, h to halt, p to fail assertion and panic
hInfo   Halt (System:122)
```

Next: [16-serializing-and-formatting](16-serializing-and-formatting.md)

