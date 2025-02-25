# Tutorial 15: String {#TUTORIAL_15_STRING}

@tableofcontents

## New tutorial setup {#TUTORIAL_15_STRING_NEW_TUTORIAL_SETUP}

As in the previous tutorial, you will find the code integrated into the CMake structure, in `tutorial/15-string`.
In the same way, the project names are adapted to make sure there are no conflicts.

### Tutorial results {#TUTORIAL_15_STRING_NEW_TUTORIAL_SETUP_TUTORIAL_RESULTS}

This tutorial will result in (next to the main project structure):
- a library `output/Debug/lib/baremetal-15.a`
- a library `output/Debug/lib/stdlib-15.a`
- an application `output/Debug/bin/15-string.elf`
- an image in `deploy/Debug/15-string-image`

## Creating a string class {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS}

It would be nice if we could deal with strings in a more flexible way. For that, we need memory allocation, so we had to wait until now with that.

We will introduce a string class, and start using it for e.g. serialization, formatting, etc.

### String.h {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_STRINGH}

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
8: // Class       : string
9: //
10: // Description : string handling
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
42: #include <stdlib/Types.h>
43: #include <baremetal/Iterator.h>
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
72:     string(const ValueType* str);
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
1: //------------------------------------------------------------------------------
2: // Copyright   : Copyright(c) 2025 Rene Barto
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
40: #include <baremetal/String.h>
41: 
42: #include <stdlib/Util.h>
43: #include <baremetal/Assert.h>
44: #include <baremetal/Logger.h>
45: #include <baremetal/Malloc.h>
46: 
47: /// @file
48: /// string class implementation
49: 
50: using namespace baremetal;
51: 
52: /// @brief Minimum allocation size for any string
53: static constexpr size_t MinimumAllocationSize = 256;
54: 
55: /// @brief Maximum string size (largest 256Mb - 1 due to largest heap allocation block size)
56: static constexpr size_t MaximumStringSize = 0x80000 - 1;
57: 
58: const size_t string::npos = static_cast<size_t>(-1);
59: /// @brief Constant null character, using as string terminator, and also returned as a reference for const methods where nothing can be returned
60: static const string::ValueType NullCharConst = '\0';
61: /// @brief Non-constant null character, returned as a reference for const methods where nothing can be returned (always reinitialized before returning)
62: static string::ValueType NullChar = '\0';
63: 
64: /// @brief Define log name
65: LOG_MODULE("String");
66: 
67: /// <summary>
68: /// Default constructor
69: ///
70: /// Constructs an empty string.
71: /// </summary>
72: string::string()
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
84: string::~string()
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
95: string::string(const ValueType* str)
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
118: string::string(const ValueType* str, size_t count)
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
143: string::string(size_t count, ValueType ch)
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
165: string::string(const string& other)
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
185: string::string(string&& other)
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
202: /// <param name="count">Maximum number of characters to copy from other. Default is until end of string. If pos + count is larger than the actual length of the string, string string is copied until the end</param>
203: string::string(const string& other, size_t pos, size_t count /*= npos*/)
204:     : m_buffer{}
205:     , m_end{}
206:     , m_allocatedSize{}
207: {
208:     if (pos >= other.length())
209:         return;
210:     auto size = other.length() - pos;
211:     if (count < size)
212:         size = count;
213:     if (reallocate(size + 1))
214:     {
215:         strncpy(m_buffer, other.data() + pos, size);
216:     }
217:     m_end = m_buffer + size;
218:     m_buffer[size] = NullCharConst;
219: }
220: 
221: /// <summary>
222: /// Const character cast operator
223: ///
224: /// Returns the pointer to the start of the string.
225: /// </summary>
226: string::operator const ValueType* () const
227: {
228:     return data();
229: }
230: 
231: /// <summary>
232: /// Assignment operator
233: ///
234: /// Assigns the specified string value to the string.
235: /// </summary>
236: /// <param name="str">string value to assign to the string</param>
237: /// <returns>A reference to the string</returns>
238: string& string::operator = (const ValueType* str)
239: {
240:     return assign(str);
241: }
242: 
243: /// <summary>
244: /// Assignment operator
245: ///
246: /// Assigns the specified string value to the string.
247: /// </summary>
248: /// <param name="str">string value to assign to the string</param>
249: /// <returns>A reference to the string</returns>
250: string& string::operator = (const string& str)
251: {
252:     return assign(str);
253: }
254: 
255: /// <summary>
256: /// Move operator
257: ///
258: /// Assigns the specified string value to the string by moving the contents of the specified string.
259: /// </summary>
260: /// <param name="str">string value to assign to the string</param>
261: /// <returns>A reference to the string</returns>
262: string& string::operator = (string&& str)
263: {
264:     if (&str != this)
265:     {
266:         m_buffer = str.m_buffer;
267:         m_end = str.m_end;
268:         m_allocatedSize = str.m_allocatedSize;
269:         str.m_buffer = nullptr;
270:         str.m_end = nullptr;
271:         str.m_allocatedSize = 0;
272:     }
273:     return *this;
274: }
275: 
276: /// <summary>
277: /// Non-const iterator to the start of the string
278: ///
279: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
280: /// </summary>
281: /// <returns>iterator to the value type, acting as the start of the string</returns>
282: iterator<string::ValueType> string::begin()
283: {
284:     return iterator(m_buffer, m_end);
285: }
286: 
287: /// <summary>
288: /// Non-const iterator to the end of the string + 1
289: ///
290: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
291: /// </summary>
292: /// <returns>iterator to the value type, acting as the end of the string</returns>
293: iterator<string::ValueType> string::end()
294: {
295:     return iterator(m_end, m_end);
296: }
297: 
298: /// <summary>
299: /// Const iterator to the start of the string
300: ///
301: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
302: /// </summary>
303: /// <returns>const_iterator to the value type, acting as the start of the string</returns>
304: const_iterator<string::ValueType> string::begin() const
305: {
306:     return const_iterator(m_buffer, m_end);
307: }
308: 
309: /// <summary>
310: /// Const iterator to the end of the string + 1
311: ///
312: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
313: /// </summary>
314: /// <returns>const_iterator to the value type, acting as the end of the string</returns>
315: const_iterator<string::ValueType> string::end() const
316: {
317:     return const_iterator(m_end, m_end);
318: }
319: 
320: /// <summary>
321: /// assign a string value
322: ///
323: /// Assigns the specified string value to the string
324: /// </summary>
325: /// <param name="str">string value to assign to the string</param>
326: /// <returns>A reference to the string</returns>
327: string& string::assign(const ValueType* str)
328: {
329:     size_t size{};
330:     if (str != nullptr)
331:     {
332:         size = strlen(str);
333:     }
334:     if ((size + 1) > m_allocatedSize)
335:     {
336:         if (!reallocate(size + 1))
337:             return *this;
338:     }
339:     if (str != nullptr)
340:     {
341:         strncpy(m_buffer, str, size);
342:     }
343:     m_end = m_buffer + size;
344:     m_buffer[size] = NullCharConst;
345:     return *this;
346: }
347: 
348: /// <summary>
349: /// assign a string value
350: ///
351: /// Assigns the specified string value, up to the specified count of characters, to the string.
352: /// </summary>
353: /// <param name="str">string value to assign to the string</param>
354: /// <param name="count">Maximum number of characters to copy from the string. If count is larger than the string length, the length of the string is used</param>
355: /// <returns>A reference to the string</returns>
356: string& string::assign(const ValueType* str, size_t count)
357: {
358:     size_t size{};
359:     if (str != nullptr)
360:     {
361:         size = strlen(str);
362:     }
363:     if (count < size)
364:         size = count;
365:     if ((size + 1) > m_allocatedSize)
366:     {
367:         if (!reallocate(size + 1))
368:             return *this;
369:     }
370:     if (str != nullptr)
371:     {
372:         strncpy(m_buffer, str, size);
373:     }
374:     m_end = m_buffer + size;
375:     m_buffer[size] = NullCharConst;
376:     return *this;
377: }
378: 
379: /// <summary>
380: /// assign a string value
381: ///
382: /// Assigns a string containing the specified count times the specified characters to the string
383: /// </summary>
384: /// <param name="count">Number copies of ch to copy to the string</param>
385: /// <param name="ch">Character to initialize with</param>
386: /// <returns>A reference to the string</returns>
387: string& string::assign(size_t count, ValueType ch)
388: {
389:     auto size = count;
390:     if (size > MaximumStringSize)
391:         size = MaximumStringSize;
392:     if ((size + 1) > m_allocatedSize)
393:     {
394:         if (!reallocate(size + 1))
395:             return *this;
396:     }
397:     memset(m_buffer, ch, size);
398:     m_end = m_buffer + size;
399:     m_buffer[size] = NullCharConst;
400:     return *this;
401: }
402: 
403: /// <summary>
404: /// assign a string value
405: ///
406: /// Assigns the specified string value to the string
407: /// </summary>
408: /// <param name="str">string value to assign to the string</param>
409: /// <returns>A reference to the string</returns>
410: string& string::assign(const string& str)
411: {
412:     auto size = str.length();
413:     if ((size + 1) > m_allocatedSize)
414:     {
415:         if (!reallocate(size + 1))
416:             return *this;
417:     }
418:     strncpy(m_buffer, str.data(), size);
419:     m_end = m_buffer + size;
420:     m_buffer[size] = NullCharConst;
421:     return *this;
422: }
423: 
424: /// <summary>
425: /// assign a string value
426: ///
427: /// Assigns the substring start from the specified position for the specified count of characters of specified string value to the string
428: /// </summary>
429: /// <param name="str">string value to assign to the string</param>
430: /// <param name="pos">Starting position of substring to copy from str</param>
431: /// <param name="count">Maximum number of characters to copy from str.
432: /// Default is until end of string. If pos + count is larger than the stirn length, characters are copied until end of string</param>
433: /// <returns>A reference to the string</returns>
434: string& string::assign(const string& str, size_t pos, size_t count /*= npos*/)
435: {
436:     if (str.empty())
437:         return assign(str);
438: 
439:     if (pos < str.length())
440:     {
441:         auto size = str.length() - pos;
442:         if (count < size)
443:             size = count;
444:         if ((size + 1) > m_allocatedSize)
445:         {
446:             if (!reallocate(size + 1))
447:                 return *this;
448:         }
449:         strncpy(m_buffer, str.data() + pos, size);
450:         m_end = m_buffer + size;
451:         m_buffer[size] = NullCharConst;
452:     }
453:     return *this;
454: }
455: 
456: /// <summary>
457: /// Return the character at specified position
458: /// </summary>
459: /// <param name="pos">Position in string</param>
460: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
461: string::ValueType& string::at(size_t pos)
462: {
463:     if (pos >= length())
464:     {
465:         NullChar = NullCharConst;
466:         return NullChar;
467:     }
468:     return m_buffer[pos];
469: }
470: 
471: /// <summary>
472: /// Return the character at specified position
473: /// </summary>
474: /// <param name="pos">Position in string</param>
475: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, a reference to a const null character is returned (NullCharConst)</returns>
476: const string::ValueType& string::at(size_t pos) const
477: {
478:     if (pos >= length())
479:         return NullCharConst;
480:     return m_buffer[pos];
481: }
482: 
483: /// <summary>
484: /// Return the first character
485: /// </summary>
486: /// <returns>Returns a non-const reference to the first character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
487: string::ValueType& string::front()
488: {
489:     if (empty())
490:     {
491:         NullChar = NullCharConst;
492:         return NullChar;
493:     }
494:     return *m_buffer;
495: }
496: 
497: /// <summary>
498: /// Return the first character
499: /// </summary>
500: /// <returns>Returns a const reference to the first character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
501: const string::ValueType& string::front() const
502: {
503:     if (empty())
504:         return NullCharConst;
505:     return *m_buffer;
506: }
507: 
508: /// <summary>
509: /// Return the last character
510: /// </summary>
511: /// <returns>Returns a non-const reference to the last character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
512: string::ValueType& string::back()
513: {
514:     if (empty())
515:     {
516:         NullChar = NullCharConst;
517:         return NullChar;
518:     }
519:     return *(m_end - 1);
520: }
521: 
522: /// <summary>
523: /// Return the last character
524: /// </summary>
525: /// <returns>Returns a const reference to the last character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
526: const string::ValueType& string::back() const
527: {
528:     if (empty())
529:         return NullCharConst;
530:     return *(m_end - 1);
531: }
532: 
533: /// <summary>
534: /// Return the character at specified position
535: /// </summary>
536: /// <param name="pos">Position in string</param>
537: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
538: string::ValueType& string::operator[] (size_t pos)
539: {
540:     if (pos >= size())
541:     {
542:         NullChar = '\0';
543:         return NullChar;
544:     }
545:     return m_buffer[pos];
546: }
547: 
548: /// <summary>
549: /// Return the character at specified position
550: /// </summary>
551: /// <param name="pos">Position in string</param>
552: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
553: const string::ValueType& string::operator[] (size_t pos) const
554: {
555:     if (pos >= size())
556:         return NullCharConst;
557:     return m_buffer[pos];
558: }
559: 
560: /// <summary>
561: /// Return the buffer pointer
562: /// </summary>
563: /// <returns>Returns a non-const pointer to the buffer. If the buffer is not allocated, a pointer to a non-const null character (NullChar, initialized before returning) is returned</returns>
564: string::ValueType* string::data()
565: {
566:     NullChar = NullCharConst;
567:     return (m_buffer == nullptr) ? &NullChar : m_buffer;
568: }
569: 
570: /// <summary>
571: /// Return the buffer pointer
572: /// </summary>
573: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
574: const string::ValueType* string::data() const
575: {
576:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
577: }
578: 
579: /// <summary>
580: /// Return the buffer pointer
581: /// </summary>
582: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
583: const string::ValueType* string::c_str() const
584: {
585:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
586: }
587: 
588: /// <summary>
589: /// Determine whether string is empty.
590: /// </summary>
591: /// <returns>Returns true when the string is empty (not allocated or no contents), false otherwise</returns>
592: bool string::empty() const
593: {
594:     return m_end == m_buffer;
595: }
596: 
597: /// <summary>
598: /// Return the size of the string
599: ///
600: /// This method is the equivalent of length().
601: /// </summary>
602: /// <returns>Returns the size (or length) of the string</returns>
603: size_t string::size() const
604: {
605:     return m_end - m_buffer;
606: }
607: 
608: /// <summary>
609: /// Return the length of the string
610: ///
611: /// This method is the equivalent of size().
612: /// </summary>
613: /// <returns>Returns the size (or length) of the string</returns>
614: size_t string::length() const
615: {
616:     return m_end - m_buffer;
617: }
618: 
619: /// <summary>
620: /// Return the capacity of the string
621: ///
622: /// The capacity is the size of the allocated buffer. The string can grow to that length before it needs to be re-allocated.
623: /// </summary>
624: /// <returns>Returns the size (or length) of the string</returns>
625: size_t string::capacity() const
626: {
627:     return m_allocatedSize;
628: }
629: 
630: /// <summary>
631: /// Reserved a buffer capacity
632: ///
633: /// Allocates a buffer of specified size
634: /// </summary>
635: /// <param name="newCapacity"></param>
636: /// <returns>Returns the capacity of the string</returns>
637: size_t string::reserve(size_t newCapacity)
638: {
639:     reallocate_allocation_size(newCapacity);
640:     return m_allocatedSize;
641: }
642: 
643: /// <summary>
644: /// append operator
645: ///
646: /// Appends a character to the string
647: /// </summary>
648: /// <param name="ch">Character to append</param>
649: /// <returns>Returns a reference to the string</returns>
650: string& string::operator +=(ValueType ch)
651: {
652:     append(1, ch);
653:     return *this;
654: }
655: 
656: /// <summary>
657: /// append operator
658: ///
659: /// Appends a string to the string
660: /// </summary>
661: /// <param name="str">string to append</param>
662: /// <returns>Returns a reference to the string</returns>
663: string& string::operator +=(const string& str)
664: {
665:     append(str);
666:     return *this;
667: }
668: 
669: /// <summary>
670: /// append operator
671: ///
672: /// Appends a string to the string
673: /// </summary>
674: /// <param name="str">string to append. If nullptr the nothing is appended</param>
675: /// <returns>Returns a reference to the string</returns>
676: string& string::operator +=(const ValueType* str)
677: {
678:     append(str);
679:     return *this;
680: }
681: 
682: /// <summary>
683: /// append operator
684: ///
685: /// Appends a sequence of count times the same character ch to the string
686: /// </summary>
687: /// <param name="count">Number of characters to append</param>
688: /// <param name="ch">Character to append</param>
689: void string::append(size_t count, ValueType ch)
690: {
691:     auto len = length();
692:     auto strLength = count;
693:     if (strLength > MaximumStringSize - len)
694:         strLength = MaximumStringSize - len;
695:     auto size = len + strLength;
696:     if ((size + 1) > m_allocatedSize)
697:     {
698:         if (!reallocate(size + 1))
699:             return;
700:     }
701:     memset(m_buffer + len, ch, strLength);
702:     m_end = m_buffer + size;
703:     m_buffer[size] = NullCharConst;
704: }
705: 
706: /// <summary>
707: /// append operator
708: ///
709: /// Appends a string to the string
710: /// </summary>
711: /// <param name="str">string to append</param>
712: void string::append(const string& str)
713: {
714:     auto len = length();
715:     auto strLength = str.length();
716:     auto size = len + strLength;
717:     if ((size + 1) > m_allocatedSize)
718:     {
719:         if (!reallocate(size + 1))
720:             return;
721:     }
722:     strncpy(m_buffer + len, str.data(), strLength);
723:     m_end = m_buffer + size;
724:     m_buffer[size] = NullCharConst;
725: }
726: 
727: /// <summary>
728: /// append operator
729: ///
730: /// Appends a substring of str to the string
731: /// </summary>
732: /// <param name="str">string to append from</param>
733: /// <param name="pos">Start position in str to copy characters from</param>
734: /// <param name="count">Number of characters to copy from str. Default is until the end of the string. If count is larger than the string length, characters are copied up to the end of the string</param>
735: void string::append(const string& str, size_t pos, size_t count /*= npos*/)
736: {
737:     if (pos >= str.length())
738:         return;
739:     auto strLength = str.length();
740:     auto strCount = strLength - pos;
741:     if (count < strCount)
742:         strCount = count;
743:     auto len = length();
744:     auto size = len + strCount;
745:     if ((size + 1) > m_allocatedSize)
746:     {
747:         if (!reallocate(size + 1))
748:             return;
749:     }
750:     strncpy(m_buffer + len, str.data() + pos, strCount);
751:     m_end = m_buffer + size;
752:     m_buffer[size] = NullCharConst;
753: }
754: 
755: /// <summary>
756: /// append operator
757: ///
758: /// Appends a string to the string
759: /// </summary>
760: /// <param name="str">string to append. If nullptr the nothing is appended</param>
761: void string::append(const ValueType* str)
762: {
763:     if (str == nullptr)
764:         return;
765:     auto len = length();
766:     auto strLength = strlen(str);
767:     auto size = len + strLength;
768:     if ((size + 1) > m_allocatedSize)
769:     {
770:         if (!reallocate(size + 1))
771:             return;
772:     }
773:     strncpy(m_buffer + len, str, strLength);
774:     m_end = m_buffer + size;
775:     m_buffer[size] = NullCharConst;
776: }
777: 
778: /// <summary>
779: /// append operator
780: ///
781: /// Appends a number of characters from str to the string
782: /// </summary>
783: /// <param name="str">string to append. If nullptr the nothing is appended</param>
784: /// <param name="count">Number of characters to copy from str. If count is larger than the string length, the complete string is copied</param>
785: void string::append(const ValueType* str, size_t count)
786: {
787:     if (str == nullptr)
788:         return;
789:     auto len = length();
790:     auto strLength = strlen(str);
791:     auto strCount = count;
792:     if (strCount > strLength)
793:         strCount = strLength;
794:     auto size = len + strCount;
795:     if ((size + 1) > m_allocatedSize)
796:     {
797:         if (!reallocate(size + 1))
798:             return;
799:     }
800:     strncpy(m_buffer + len, str, strCount);
801:     m_end = m_buffer + size;
802:     m_buffer[size] = NullCharConst;
803: }
804: 
805: /// <summary>
806: /// clear the string
807: ///
808: /// Clears the contents of the string, but does not free or reallocate the buffer
809: /// </summary>
810: void string::clear()
811: {
812:     if (!empty())
813:     {
814:         m_end = m_buffer;
815:         m_buffer[0] = NullCharConst;
816:     }
817: }
818: 
819: /// <summary>
820: /// find a substring in the string
821: ///
822: /// If empty string, always finds the string.
823: /// </summary>
824: /// <param name="str">Substring to find</param>
825: /// <param name="pos">Starting position in string to start searching</param>
826: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
827: size_t string::find(const string& str, size_t pos /*= 0*/) const
828: {
829:     auto len = length();
830:     auto patternLength = str.length();
831:     if (pos >= len)
832:         return npos;
833:     auto needle = str.data();
834:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
835:     {
836:         if (memcmp(haystack, needle, patternLength) == 0)
837:             return haystack - m_buffer;
838:     }
839:     return npos;
840: }
841: 
842: /// <summary>
843: /// find a substring in the string
844: ///
845: /// If nullptr or empty string, always finds the string.
846: /// </summary>
847: /// <param name="str">Substring to find</param>
848: /// <param name="pos">Starting position in string to start searching</param>
849: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
850: size_t string::find(const ValueType* str, size_t pos /*= 0*/) const
851: {
852:     size_t strLength{};
853:     if (str != nullptr)
854:     {
855:         strLength = strlen(str);
856:     }
857:     auto len = length();
858:     auto patternLength = strLength;
859:     if (pos >= len)
860:         return npos;
861:     auto needle = str;
862:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
863:     {
864:         if (memcmp(haystack, needle, patternLength) == 0)
865:             return haystack - m_buffer;
866:     }
867:     return npos;
868: }
869: 
870: /// <summary>
871: /// find a substring in the string
872: ///
873: /// If nullptr or empty string, always finds the string.
874: /// </summary>
875: /// <param name="str">Substring to find</param>
876: /// <param name="pos">Starting position in string to start searching</param>
877: /// <param name="count">Number of characters from str to compare</param>
878: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
879: size_t string::find(const ValueType* str, size_t pos, size_t count) const
880: {
881:     size_t strLength{};
882:     if (str != nullptr)
883:     {
884:         strLength = strlen(str);
885:     }
886:     auto len = length();
887:     auto patternLength = strLength;
888:     if (pos >= len)
889:         return npos;
890:     if (count < patternLength)
891:         patternLength = count;
892:     auto needle = str;
893:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
894:     {
895:         if (memcmp(haystack, needle, patternLength) == 0)
896:             return haystack - m_buffer;
897:     }
898:     return npos;
899: }
900: 
901: /// <summary>
902: /// find a character in the string
903: /// </summary>
904: /// <param name="ch">Character to find</param>
905: /// <param name="pos">Starting position in string to start searching</param>
906: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
907: size_t string::find(ValueType ch, size_t pos /*= 0*/) const
908: {
909:     auto len = length();
910:     if (pos >= len)
911:         return npos;
912:     for (const ValueType* haystack = data() + pos; haystack <= m_end; ++haystack)
913:     {
914:         if (*haystack == ch)
915:             return haystack - m_buffer;
916:     }
917:     return npos;
918: }
919: 
920: /// <summary>
921: /// Check whether string starts with character
922: /// </summary>
923: /// <param name="ch">Character to find</param>
924: /// <returns>Returns true if ch is first character in string, false otherwise</returns>
925: bool string::starts_with(ValueType ch) const
926: {
927:     if (empty())
928:         return false;
929:     return m_buffer[0] == ch;
930: }
931: 
932: /// <summary>
933: /// Check whether string starts with substring
934: /// </summary>
935: /// <param name="str">SubString to find</param>
936: /// <returns>Returns true if str is first part of string, false otherwise</returns>
937: bool string::starts_with(const string& str) const
938: {
939:     auto len = length();
940:     auto strLength = str.length();
941:     if (strLength >= len)
942:         return false;
943: 
944:     return memcmp(data(), str.data(), strLength) == 0;
945: }
946: 
947: /// <summary>
948: /// Check whether string starts with substring
949: /// </summary>
950: /// <param name="str">SubString to find</param>
951: /// <returns>Returns true if str is first part of string, false otherwise</returns>
952: bool string::starts_with(const ValueType* str) const
953: {
954:     size_t strLength{};
955:     if (str != nullptr)
956:     {
957:         strLength = strlen(str);
958:     }
959:     auto len = length();
960:     if (strLength >= len)
961:         return false;
962: 
963:     return memcmp(data(), str, strLength) == 0;
964: }
965: 
966: /// <summary>
967: /// Check whether string ends with character
968: /// </summary>
969: /// <param name="ch">Character to find</param>
970: /// <returns>Returns true if ch is last character in string, false otherwise</returns>
971: bool string::ends_with(ValueType ch) const
972: {
973:     if (empty())
974:         return false;
975:     return m_buffer[length() - 1] == ch;
976: }
977: 
978: /// <summary>
979: /// Check whether string ends with substring
980: /// </summary>
981: /// <param name="str">SubString to find</param>
982: /// <returns>Returns true if str is last part of string, false otherwise</returns>
983: bool string::ends_with(const string& str) const
984: {
985:     auto len = length();
986:     auto strLength = str.length();
987:     if (strLength >= len)
988:         return false;
989: 
990:     return memcmp(m_end - strLength, str.data(), strLength) == 0;
991: }
992: 
993: /// <summary>
994: /// Check whether string ends with substring
995: /// </summary>
996: /// <param name="str">SubString to find</param>
997: /// <returns>Returns true if str is last part of string, false otherwise</returns>
998: bool string::ends_with(const ValueType* str) const
999: {
1000:     size_t strLength{};
1001:     if (str != nullptr)
1002:     {
1003:         strLength = strlen(str);
1004:     }
1005:     auto len = length();
1006:     if (strLength >= len)
1007:         return false;
1008: 
1009:     return memcmp(m_end - strLength, str, strLength) == 0;
1010: }
1011: 
1012: /// <summary>
1013: /// Check whether string contains character
1014: /// </summary>
1015: /// <param name="ch">Character to find</param>
1016: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1017: bool string::contains(ValueType ch) const
1018: {
1019:     return find(ch) != npos;
1020: }
1021: 
1022: /// <summary>
1023: /// Check whether string contains substring
1024: /// </summary>
1025: /// <param name="str">Substring to find</param>
1026: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1027: bool string::contains(const string& str) const
1028: {
1029:     return find(str) != npos;
1030: }
1031: 
1032: /// <summary>
1033: /// Check whether string contains substring
1034: /// </summary>
1035: /// <param name="str">Substring to find</param>
1036: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1037: bool string::contains(const ValueType* str) const
1038: {
1039:     return find(str) != npos;
1040: }
1041: 
1042: /// <summary>
1043: /// Return substring
1044: /// </summary>
1045: /// <param name="pos">Starting position of substring in string</param>
1046: /// <param name="count">length of substring to return. If count is larger than the number of characters available from position pos, the rest of the string is returned</param>
1047: /// <returns>Returns the substring at position [pos, pos + count), if available </returns>
1048: string string::substr(size_t pos /*= 0*/, size_t count /*= npos*/) const
1049: {
1050:     string result;
1051:     auto size = length() - pos;
1052:     if (pos < length())
1053:     {
1054:         if (count < size)
1055:             size = count;
1056:         result.reallocate(size + 1);
1057:         memcpy(result.data(), data() + pos, size);
1058:         result.m_end = result.m_buffer + size;
1059:         result.data()[size] = NullCharConst;
1060:     }
1061: 
1062:     return result;
1063: }
1064: 
1065: /// <summary>
1066: /// Case sensitive equality to string
1067: /// </summary>
1068: /// <param name="other">string to compare to</param>
1069: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1070: bool string::equals(const string& other) const
1071: {
1072:     return compare(other) == 0;
1073: }
1074: 
1075: /// <summary>
1076: /// Case sensitive equality to string
1077: /// </summary>
1078: /// <param name="other">string to compare to</param>
1079: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1080: bool string::equals(const ValueType* other) const
1081: {
1082:     if (other == nullptr)
1083:         return empty();
1084:     if (length() != strlen(other))
1085:         return false;
1086:     return strcmp(data(), other) == 0;
1087: }
1088: 
1089: /// <summary>
1090: /// Case insensitive equality to string
1091: /// </summary>
1092: /// <param name="other">string to compare to</param>
1093: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1094: bool string::equals_case_insensitive(const string& other) const
1095: {
1096:     if (length() != other.length())
1097:         return false;
1098:     if (empty())
1099:         return true;
1100:     return strcasecmp(data(), other.data()) == 0;
1101: }
1102: 
1103: /// <summary>
1104: /// Case insensitive equality to string
1105: /// </summary>
1106: /// <param name="other">string to compare to</param>
1107: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1108: bool string::equals_case_insensitive(const ValueType* other) const
1109: {
1110:     if (other == nullptr)
1111:         return empty();
1112:     if (length() != strlen(other))
1113:         return false;
1114:     return strcasecmp(data(), other) == 0;
1115: }
1116: 
1117: /// <summary>
1118: /// Case sensitive compare to string
1119: ///
1120: /// Compares the complete string, character by character
1121: /// </summary>
1122: /// <param name="str">string to compare to</param>
1123: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1124: int string::compare(const string& str) const
1125: {
1126:     if (empty())
1127:     {
1128:         if (str.empty())
1129:             return 0;
1130:         return -1;
1131:     }
1132:     if (str.empty())
1133:         return 1;
1134: 
1135:     return strcmp(data(), str.data());
1136: }
1137: 
1138: /// <summary>
1139: /// Case sensitive compare to string
1140: ///
1141: /// Compares the substring from pos to pos+count to str
1142: /// </summary>
1143: /// <param name="pos">Starting position of substring to compare to str</param>
1144: /// <param name="count">Number of characters in substring to compare to str</param>
1145: /// <param name="str">string to compare to</param>
1146: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1147: int string::compare(size_t pos, size_t count, const string& str) const
1148: {
1149:     return substr(pos, count).compare(str);
1150: }
1151: 
1152: /// <summary>
1153: /// Case sensitive compare to string
1154: ///
1155: /// Compares the substring from pos to pos+count to the substring from strPos to strPos+strCount of str
1156: /// </summary>
1157: /// <param name="pos">Starting position of substring to compare to str</param>
1158: /// <param name="count">Number of characters in substring to compare to str</param>
1159: /// <param name="str">string to compare to</param>
1160: /// <param name="strPos">Starting position of substring of str to compare</param>
1161: /// <param name="strCount">Number of characters in substring of str to compare</param>
1162: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1163: int string::compare(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/) const
1164: {
1165:     return substr(pos, count).compare(str.substr(strPos, strCount));
1166: }
1167: 
1168: /// <summary>
1169: /// Case sensitive compare to string
1170: ///
1171: /// Compares the complete string to str
1172: /// </summary>
1173: /// <param name="str">string to compare to</param>
1174: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1175: int string::compare(const ValueType* str) const
1176: {
1177:     size_t strLength{};
1178:     if (str != nullptr)
1179:         strLength = strlen(str);
1180:     if (empty())
1181:     {
1182:         if (strLength == 0)
1183:             return 0;
1184:         return -1;
1185:     }
1186:     if (strLength == 0)
1187:         return 1;
1188: 
1189:     return strcmp(data(), str);
1190: }
1191: 
1192: /// <summary>
1193: /// Case sensitive compare to string
1194: ///
1195: /// Compares the substring from pos to pos+count to str
1196: /// </summary>
1197: /// <param name="pos">Starting position of substring to compare to str</param>
1198: /// <param name="count">Number of characters in substring to compare to str</param>
1199: /// <param name="str">string to compare to</param>
1200: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1201: int string::compare(size_t pos, size_t count, const ValueType* str) const
1202: {
1203:     size_t strLength{};
1204:     if (str != nullptr)
1205:         strLength = strlen(str);
1206: 
1207:     auto len = length();
1208:     if (pos >= len)
1209:         len = 0;
1210:     len -= pos;
1211:     if (count < len)
1212:         len = count;
1213:     if (len == 0)
1214:     {
1215:         if (strLength == 0)
1216:             return 0;
1217:         return -1;
1218:     }
1219:     if (strLength == 0)
1220:         return 1;
1221: 
1222:     auto maxLen = strLength;
1223:     if (maxLen < len)
1224:         maxLen = len;
1225:     return strncmp(data() + pos, str, maxLen);
1226: }
1227: 
1228: /// <summary>
1229: /// Case sensitive compare to string
1230: ///
1231: /// Compares the substring from pos to pos+count to the first strCount characters of str
1232: /// </summary>
1233: /// <param name="pos">Starting position of substring to compare to str</param>
1234: /// <param name="count">Number of characters in substring to compare to str</param>
1235: /// <param name="str">string to compare to</param>
1236: /// <param name="strCount">Number of characters in substring of str to compare</param>
1237: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1238: int string::compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const
1239: {
1240:     size_t strLength{};
1241:     if (str != nullptr)
1242:         strLength = strlen(str);
1243: 
1244:     auto len = length();
1245:     if (pos >= len)
1246:         len = 0;
1247:     len -= pos;
1248:     if (count < len)
1249:         len = count;
1250: 
1251:     if (strCount < strLength)
1252:         strLength = strCount;
1253: 
1254:     if (len == 0)
1255:     {
1256:         if (strLength == 0)
1257:             return 0;
1258:         return -1;
1259:     }
1260:     if (strLength == 0)
1261:         return 1;
1262: 
1263:     auto maxLen = strLength;
1264:     if (maxLen < len)
1265:         maxLen = len;
1266:     return strncmp(data() + pos, str, maxLen);
1267: }
1268: 
1269: /// <summary>
1270: /// replace substring
1271: ///
1272: /// Replaces the substring from pos to pos+count with str
1273: /// </summary>
1274: /// <param name="pos">Starting position of substring to replace</param>
1275: /// <param name="count">Number of characters in substring to replace</param>
1276: /// <param name="str">string to replace with</param>
1277: /// <returns>Returns the reference to the resulting string</returns>
1278: string& string::replace(size_t pos, size_t count, const string& str)
1279: {
1280:     string result = substr(0, pos) + str + substr(pos + count);
1281:     assign(result);
1282:     return *this;
1283: }
1284: 
1285: /// <summary>
1286: /// replace substring
1287: ///
1288: /// Replaces the substring from pos to pos+count with the substring from strPos to strPos+strCount of str
1289: /// </summary>
1290: /// <param name="pos">Starting position of substring to replace</param>
1291: /// <param name="count">Number of characters in substring to replace</param>
1292: /// <param name="str">string to replace with</param>
1293: /// <param name="strPos">Starting position of substring of str to replace with</param>
1294: /// <param name="strCount">Number of characters in substring of str to replace with</param>
1295: /// <returns>Returns the reference to the resulting string</returns>
1296: string& string::replace(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/)
1297: {
1298:     string result = substr(0, pos) + str.substr(strPos, strCount) + substr(pos + count);
1299:     assign(result);
1300:     return *this;
1301: }
1302: 
1303: /// <summary>
1304: /// replace substring
1305: ///
1306: /// Replaces the substring from pos to pos+count with str
1307: /// </summary>
1308: /// <param name="pos">Starting position of substring to replace</param>
1309: /// <param name="count">Number of characters in substring to replace</param>
1310: /// <param name="str">string to replace with</param>
1311: /// <returns>Returns the reference to the resulting string</returns>
1312: string& string::replace(size_t pos, size_t count, const ValueType* str)
1313: {
1314:     string result = substr(0, pos) + str + substr(pos + count);
1315:     assign(result);
1316:     return *this;
1317: }
1318: 
1319: /// <summary>
1320: /// replace substring
1321: ///
1322: /// Replaces the substring from pos to pos+count with the first strCount characters of str
1323: /// </summary>
1324: /// <param name="pos">Starting position of substring to replace</param>
1325: /// <param name="count">Number of characters in substring to replace</param>
1326: /// <param name="str">string to replace with</param>
1327: /// <param name="strCount">Number of characters in substring to replace with</param>
1328: /// <returns>Returns the reference to the resulting string</returns>
1329: string& string::replace(size_t pos, size_t count, const ValueType* str, size_t strCount)
1330: {
1331:     string result = substr(0, pos) + string(str, strCount) + substr(pos + count);
1332:     assign(result);
1333:     return *this;
1334: }
1335: 
1336: /// <summary>
1337: /// replace substring
1338: ///
1339: /// Replaces the substring from pos to pos+count with ch
1340: /// </summary>
1341: /// <param name="pos">Starting position of substring to replace</param>
1342: /// <param name="count">Number of characters in substring to replace</param>
1343: /// <param name="ch">Characters to replace with</param>
1344: /// <returns>Returns the reference to the resulting string</returns>
1345: string& string::replace(size_t pos, size_t count, ValueType ch)
1346: {
1347:     return replace(pos, count, ch, 1);
1348: }
1349: 
1350: /// <summary>
1351: /// replace substring
1352: ///
1353: /// Replaces the substring from pos to pos+count with a sequence of chCount copies of ch
1354: /// </summary>
1355: /// <param name="pos">Starting position of substring to replace</param>
1356: /// <param name="count">Number of characters in substring to replace</param>
1357: /// <param name="ch">Characters to replace with</param>
1358: /// <param name="chCount">Number of copies of ch to replace with</param>
1359: /// <returns>Returns the reference to the resulting string</returns>
1360: string& string::replace(size_t pos, size_t count, ValueType ch, size_t chCount)
1361: {
1362:     string result = substr(0, pos) + string(chCount, ch) + substr(pos + count);
1363:     assign(result);
1364:     return *this;
1365: }
1366: 
1367: /// <summary>
1368: /// replace substring
1369: ///
1370: /// Replaces all instances of the substring oldStr (if existing) with newStr
1371: /// </summary>
1372: /// <param name="oldStr">string to find in string</param>
1373: /// <param name="newStr">string to replace with</param>
1374: /// <returns>Returns the number of times the string was replaced</returns>
1375: int string::replace(const string& oldStr, const string& newStr)
1376: {
1377:     size_t pos = find(oldStr);
1378:     size_t oldLength = oldStr.length();
1379:     size_t newLength = newStr.length();
1380:     int count = 0;
1381:     while (pos != npos)
1382:     {
1383:         replace(pos, oldLength, newStr);
1384:         pos += newLength;
1385:         pos = find(oldStr, pos);
1386:         count++;
1387:     }
1388:     return count;
1389: }
1390: 
1391: /// <summary>
1392: /// replace substring
1393: ///
1394: /// Replaces all instances of the substring oldStr (if existing) with newStr
1395: /// </summary>
1396: /// <param name="oldStr">string to find in string</param>
1397: /// <param name="newStr">string to replace with</param>
1398: /// <returns>Returns the number of times the string was replaced</returns>
1399: int string::replace(const ValueType* oldStr, const ValueType* newStr)
1400: {
1401:     if ((oldStr == nullptr) || (newStr == nullptr))
1402:         return 0;
1403:     size_t pos = find(oldStr);
1404:     size_t oldLength = strlen(oldStr);
1405:     size_t newLength = strlen(newStr);
1406:     int count = 0;
1407:     while (pos != npos)
1408:     {
1409:         replace(pos, oldLength, newStr);
1410:         pos += newLength;
1411:         pos = find(oldStr, pos);
1412:         count++;
1413:     }
1414:     return count;
1415: }
1416: 
1417: /// <summary>
1418: /// Align string
1419: ///
1420: /// Pads the string on the left (width > 0) or on the right (width < 0) up to a length of width characters. If the string is larger than width characters, it is not modified.
1421: /// </summary>
1422: /// <param name="width">length of target string. If width < 0, the string is padded to the right with spaces up to -width characters.
1423: /// if width > 0, the string is padded to the left with space up to width characters</param>
1424: /// <returns>Returns the number of times the string was replaced</returns>
1425: string string::align(int width) const
1426: {
1427:     string result;
1428:     int    absWidth = (width > 0) ? width : -width;
1429:     auto   len = length();
1430:     if (static_cast<size_t>(absWidth) > len)
1431:     {
1432:         if (width < 0)
1433:         {
1434:             result = *this + string(static_cast<unsigned int>(-width) - len, ' ');
1435:         }
1436:         else
1437:         {
1438:             result = string(static_cast<unsigned int>(width) - len, ' ') + *this;
1439:         }
1440:     }
1441:     else
1442:         result = *this;
1443:     return result;
1444: }
1445: 
1446: /// <summary>
1447: /// Allocate or re-allocate string to have a capacity of requestedLength characters
1448: /// </summary>
1449: /// <param name="requestedLength">Amount of characters in the string to allocate space for</param>
1450: /// <returns>True if successful, false otherwise</returns>
1451: bool string::reallocate(size_t requestedLength)
1452: {
1453:     auto requestedSize = requestedLength;
1454:     auto allocationSize = NextPowerOf2((requestedSize < MinimumAllocationSize) ? MinimumAllocationSize : requestedSize);
1455: 
1456:     if (!reallocate_allocation_size(allocationSize))
1457:         return false;
1458:     return true;
1459: }
1460: 
1461: /// <summary>
1462: /// Allocate or re-allocate string to have a capacity of allocationSize bytes
1463: /// </summary>
1464: /// <param name="allocationSize">Amount of bytes to allocate space for</param>
1465: /// <returns>True if successful, false otherwise</returns>
1466: bool string::reallocate_allocation_size(size_t allocationSize)
1467: {
1468:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1469:     if (newBuffer == nullptr)
1470:     {
1471:         return false;
1472:     }
1473:     m_buffer = newBuffer;
1474:     if (m_end == nullptr)
1475:         m_end = m_buffer;
1476:     if (m_end > m_buffer + allocationSize)
1477:         m_end = m_buffer + allocationSize;
1478:     m_allocatedSize = allocationSize;
1479:     return true;
1480: }
```

As you can see, this is a very extensive class.
A few remarks:
- Line 52: We use a minimum allocation size of 256, meaning that every string will have storage space for 255 characters plus a null character for termination, at the least.
This is to limit the amount of re-allocations when extending strings
- Line 55: We define a constant `MaximumStringSize`. We limit strings to a maximum length, which is one less than the largest heap block that can be allocated in the default bucket sizes (512 Kb)
- Line 59: We use a special static variable `NullCharConst` when we need to return something constant, but the string is not allocated.
- Line 61: Similarly, we use a special static variable `NullChar` when we need to return something non constant, but the string is not allocated.
This variable is non-const, so will have to be initialized every time we return it, to make sure it is still a null character.
- Line 184-192: We implement a move constructor, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 261-273: We implement a move assignment, similar to the move constructor
- Line 281-295: We use our own version of a iterator template, which we'll get to, to enable creating the begin() and end() methods.
These allow us to use e.g. the standard c++ `for (x : string)` construct. We'll have to define this iterator ourselves in a minute
- Line 303-317: We also use our own version of a const_iterator template for const iterators, which we'll get to, to enable creating the begin() and end() const methods.
- Line 1016-1039: We also have the `contains()` methods, which are new to C++23, however they are quite convenient
- Line 1069-1114: We use the methods `equals()` and `equals_case_insensititive()` to compare strings. Both are not standard, but convenient.
These use the stand C functions `strcmp()` and `strcasecmp()`, which we will need to declare and define.
- Line 1374-1414: We implement two extra variants of `replace()` (replacing multiple instances of a string) for convenience
- Line 1424-1443: Again for convenience, we implement the `align()` method. We'll be using this later on for aligning strings when formatting.
If the width is negative, we align to the left, if it is positive we align to the right.
- Line 1451-1459: We reallocating a string, we take care to round up the reserved space to a power of two, to minimize the number of reallocations needed

### Iterator.h {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_ITERATORH}

As said before, we use our own version of const_iterator and iterator to be used in the `string` methods `begin()` and `end()`.
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
We define both the `iterator` and `const_iterator` templates here._

### Util.h {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_UTILH}

We need to declare and define the standard C functions `strcmp()` and `strcasecmp()` for our implementation of the `string` class.

Update the file `code/libraries/stdlib/include/stdlib/Util.h`

```cpp
File: code/libraries/stdlib/include/stdlib/Util.h
...
47: #ifdef __cplusplus
48: extern "C" {
49: #endif
50: 
51: void *memset(void *buffer, int value, size_t length);
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
76:     unsigned bitCount{ 0 };
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

- Line 53: We add the standard `memcmp()` function
- Line 55-56: We add the standard `toupper()` amd `tolower()` functions
- Line 58-61: We add the standard `strcmp()` (compare strings to end of string), `strncmp()` (compare strings with maximum size), and the case insensitive variants `strcasecmp()` and `strncasecmp()`
- Line 74-84: We add an inline function `NextPowerOf2Bits()` to compute the number of significant bits in n, the number of bits b required to hold a value n, so 2^b >= n
- Line 91-94: We add an inline function `NextPowerOf2()` to compute the value x larger or equal to a number n, such that x is a power of 2

### Util.cpp {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_UTILCPP}

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

### Update project configuration {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_UPDATE_PROJECT_CONFIGURATION}

As we added some files, we need to update the CMake file.

Update the file `code/libraries/baremetal/CMakeLists.txt`

```cmake
File: code/libraries/baremetal/CMakeLists.txt
30: set(PROJECT_SOURCES
31:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Assert.cpp
32:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Console.cpp
33:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Format.cpp
34:     ${CMAKE_CURRENT_SOURCE_DIR}/src/HeapAllocator.cpp
35:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
36:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Mailbox.cpp
37:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineInfo.cpp
38:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Malloc.cpp
39:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryAccess.cpp
40:     ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
41:     ${CMAKE_CURRENT_SOURCE_DIR}/src/New.cpp
42:     ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicalGPIOPin.cpp
43:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIProperties.cpp
44:     ${CMAKE_CURRENT_SOURCE_DIR}/src/RPIPropertiesInterface.cpp
45:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization.cpp
46:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Startup.S
47:     ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
48:     ${CMAKE_CURRENT_SOURCE_DIR}/src/System.cpp
49:     ${CMAKE_CURRENT_SOURCE_DIR}/src/Timer.cpp
50:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART0.cpp
51:     ${CMAKE_CURRENT_SOURCE_DIR}/src/UART1.cpp
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
66:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Iterator.h
67:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Logger.h
68:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MachineInfo.h
69:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Mailbox.h
70:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Malloc.h
71:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryAccess.h
72:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryManager.h
73:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/MemoryMap.h
74:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/New.h
75:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/PhysicalGPIOPin.h
76:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIProperties.h
77:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/RPIPropertiesInterface.h
78:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Serialization.h
79:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/String.h
80:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Synchronization.h
81:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/SysConfig.h
82:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/System.h
83:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Timer.h
84:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART0.h
85:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/UART1.h
86:     ${CMAKE_CURRENT_SOURCE_DIR}/include/baremetal/Version.h
87:     )
88: set(PROJECT_INCLUDES_PRIVATE )
```

### Application code {#TUTORIAL_15_STRING_CREATING_A_STRING_CLASS_APPLICATION_CODE}

We'll start making use of the string class we just added, but we'll do it in a way that shows that the string methods function as expected.
This is a first attempt at creating class / micro / unit tests for our code, which we will start doing soon.

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
37:     LOG_INFO("s4");
38:     for (auto ch : s4)
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
97:     assert(s8c.capacity() == 256);
98:     assert(s8.reserve(1024) == 1024);
99:     assert(s8.capacity() == 1024);
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

As you can see the code has grown quite a bit due to all the tests we perform.

- Line 1: we need to include `Util.h`
- Line 12: we need to include `String.h`
- Line 26-35: We construct some strings, in different ways, to cover all the variants of constructors
- Line 37-41: We use a for loop to iterate through the characters in a string.
This will use the `begin()` and `end()` methods
- Line 42-51: We check whether the strings are initialized as expected.
Notice that we use the `assert` macro here
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
Next to this nothing special is shown, except for the memory status at start and end of the application, as the tests will all succeed.
If a test were to fail, the assertion would fire, and the application would crash.
This is a way to test code, however we'd like to see all of the failure found in our code, not have the first one crash the application.

```text
Info   Baremetal 0.0.1 started on Raspberry Pi 3 Model B (AArch64) using BCM2837 SoC (Logger:82)
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
Info   s4 (main:37)
Info   b (main:40)
Info   b (main:40)
Info   b (main:40)
Info   b (main:40)
Info   Wait 5 seconds (main:432)
Press r to reboot, h to halt, p to fail assertion and panic
hDebug  Low heap: (MemoryManager:220)
Debug  Heap allocator info:     heaplow (HeapAllocator:272)
Debug  Current #allocations:    14 (HeapAllocator:273)
Debug  Max #allocations:        26 (HeapAllocator:274)
Debug  Current #allocated bytes:14336 (HeapAllocator:275)
Debug  Total #allocated blocks: 106 (HeapAllocator:276)
Debug  Total #allocated bytes:  108544 (HeapAllocator:277)
Debug  Total #freed blocks:     92 (HeapAllocator:278)
Debug  Total #freed bytes:      94208 (HeapAllocator:279)
Debug  malloc(64): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(1024): 14 blocks (max 26) total alloc #blocks = 106, #bytes = 108544, total free #blocks = 92, #bytes = 94208 (HeapAllocator:283)
Debug  malloc(4096): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(16384): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(65536): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(262144): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Debug  malloc(524288): 0 blocks (max 0) total alloc #blocks = 0, #bytes = 0, total free #blocks = 0, #bytes = 0 (HeapAllocator:283)
Info   Halt (System:122)
```

Next: [16-serializing-and-formatting](16-serializing-and-formatting.md)
