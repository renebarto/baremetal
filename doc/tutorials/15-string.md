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
53: const size_t string::npos = static_cast<size_t>(-1);
54: /// @brief Constant null character, using as string terminator, and also returned as a reference for const methods where nothing can be returned
55: static const string::ValueType NullCharConst = '\0';
56: /// @brief Non-constant null character, returned as a reference for const methods where nothing can be returned (always reinitialized before returning)
57: static string::ValueType NullChar = '\0';
58: 
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
86: 
87: /// <summary>
88: /// Constructor
89: ///
90: /// Initializes the string with the specified string.
91: /// </summary>
92: /// <param name="str">string to initialize with</param>
93: string::string(const ValueType* str)
94:     : m_buffer{}
95:     , m_end{}
96:     , m_allocatedSize{}
97: {
98:     if (str == nullptr)
99:         return;
100:     auto size = strlen(str);
101:     if (reallocate(size + 1))
102:     {
103:         strncpy(m_buffer, str, size);
104:     }
105:     m_end = m_buffer + size;
106:     m_buffer[size] = NullCharConst;
107: }
108: 
109: /// <summary>
110: /// Constructor
111: ///
112: /// Initializes the string with up to count characters in the specified string. A null character is always added.
113: /// </summary>
114: /// <param name="str">string to initialize with</param>
115: /// <param name="count">Maximum number of characters from str to initialize with. If count is larger than the actual string length, only the string length is used</param>
116: string::string(const ValueType* str, size_t count)
117:     : m_buffer{}
118:     , m_end{}
119:     , m_allocatedSize{}
120: {
121:     if (str == nullptr)
122:         return;
123:     auto size = strlen(str);
124:     if (count < size)
125:         size = count;
126:     if (reallocate(size + 1))
127:     {
128:         strncpy(m_buffer, str, size);
129:     }
130:     m_end = m_buffer + size;
131:     m_buffer[size] = NullCharConst;
132: }
133: 
134: /// <summary>
135: /// Constructor
136: ///
137: /// Initializes the string with the specified count times the specified character. A null character is always added.
138: /// </summary>
139: /// <param name="count">Number of characters of value ch to initialized with</param>
140: /// <param name="ch">Character to initialize with</param>
141: string::string(size_t count, ValueType ch)
142:     : m_buffer{}
143:     , m_end{}
144:     , m_allocatedSize{}
145: {
146:     auto size = count;
147:     if (reallocate(size + 1))
148:     {
149:         memset(m_buffer, ch, size);
150:     }
151:     m_end = m_buffer + size;
152:     m_buffer[size] = NullCharConst;
153: }
154: 
155: /// <summary>
156: /// Copy constructor
157: ///
158: /// Initializes the string with the specified string value.
159: /// </summary>
160: /// <param name="other">string to initialize with</param>
161: string::string(const string& other)
162:     : m_buffer{}
163:     , m_end{}
164:     , m_allocatedSize{}
165: {
166:     auto size = other.length();
167:     if (reallocate(size + 1))
168:     {
169:         strncpy(m_buffer, other.data(), size);
170:     }
171:     m_end = m_buffer + size;
172:     m_buffer[size] = NullCharConst;
173: }
174: 
175: /// <summary>
176: /// Move constructor
177: ///
178: /// Initializes the string by moving the contents from the specified string value.
179: /// </summary>
180: /// <param name="other">string to initialize with</param>
181: string::string(string&& other)
182:     : m_buffer{other.m_buffer}
183:     , m_end{other.m_end}
184:     , m_allocatedSize{other.m_allocatedSize}
185: {
186:     other.m_buffer = nullptr;
187:     other.m_end = nullptr;
188:     other.m_allocatedSize = 0;
189: }
190: 
191: /// <summary>
192: /// Constructor
193: ///
194: /// Initializes the string with the substring starting at specified position, for the specified number of characters, from the specified string value.
195: /// </summary>
196: /// <param name="other">string to initialize with</param>
197: /// <param name="pos">Position in other to start copying charaters from</param>
198: /// <param name="count">Maximum number of characters to copy from other. Default is until end of string. If pos + count is larger than the actual length of the string, string string is copied until the end</param>
199: string::string(const string& other, size_t pos, size_t count /*= npos*/)
200:     : m_buffer{}
201:     , m_end{}
202:     , m_allocatedSize{}
203: {
204:     auto size = other.length() - pos;
205:     if (count < size)
206:         size = count;
207:     if (reallocate(size + 1))
208:     {
209:         strncpy(m_buffer, other.data() + pos, size);
210:     }
211:     m_end = m_buffer + size;
212:     m_buffer[size] = NullCharConst;
213: }
214: 
215: /// <summary>
216: /// Const character cast operator
217: ///
218: /// Returns the pointer to the start of the string.
219: /// </summary>
220: string::operator const ValueType* () const
221: {
222:     return data();
223: }
224: 
225: /// <summary>
226: /// Assignment operator
227: ///
228: /// Assigns the specified string value to the string.
229: /// </summary>
230: /// <param name="str">string value to assign to the string</param>
231: /// <returns>A reference to the string</returns>
232: string& string::operator = (const ValueType* str)
233: {
234:     return assign(str);
235: }
236: 
237: /// <summary>
238: /// Assignment operator
239: ///
240: /// Assigns the specified string value to the string.
241: /// </summary>
242: /// <param name="str">string value to assign to the string</param>
243: /// <returns>A reference to the string</returns>
244: string& string::operator = (const string& str)
245: {
246:     return assign(str);
247: }
248: 
249: /// <summary>
250: /// Move operator
251: ///
252: /// Assigns the specified string value to the string by moving the contents of the specified string.
253: /// </summary>
254: /// <param name="str">string value to assign to the string</param>
255: /// <returns>A reference to the string</returns>
256: string& string::operator = (string&& str)
257: {
258:     if (&str != this)
259:     {
260:         m_buffer = str.m_buffer;
261:         m_end = str.m_end;
262:         m_allocatedSize = str.m_allocatedSize;
263:         str.m_buffer = nullptr;
264:         str.m_end = nullptr;
265:         str.m_allocatedSize = 0;
266:     }
267:     return *this;
268: }
269: 
270: /// <summary>
271: /// Non-const iterator to the start of the string
272: ///
273: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
274: /// </summary>
275: /// <returns>iterator to the value type, acting as the start of the string</returns>
276: iterator<string::ValueType> string::begin()
277: {
278:     return iterator(m_buffer, m_end);
279: }
280: 
281: /// <summary>
282: /// Non-const iterator to the end of the string + 1
283: ///
284: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
285: /// </summary>
286: /// <returns>iterator to the value type, acting as the end of the string</returns>
287: iterator<string::ValueType> string::end()
288: {
289:     return iterator(m_end, m_end);
290: }
291: 
292: /// <summary>
293: /// Const iterator to the start of the string
294: ///
295: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
296: /// </summary>
297: /// <returns>const_iterator to the value type, acting as the start of the string</returns>
298: const_iterator<string::ValueType> string::begin() const
299: {
300:     return const_iterator(m_buffer, m_end);
301: }
302: 
303: /// <summary>
304: /// Const iterator to the end of the string + 1
305: ///
306: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
307: /// </summary>
308: /// <returns>const_iterator to the value type, acting as the end of the string</returns>
309: const_iterator<string::ValueType> string::end() const
310: {
311:     return const_iterator(m_end, m_end);
312: }
313: 
314: /// <summary>
315: /// assign a string value
316: ///
317: /// Assigns the specified string value to the string
318: /// </summary>
319: /// <param name="str">string value to assign to the string</param>
320: /// <returns>A reference to the string</returns>
321: string& string::assign(const ValueType* str)
322: {
323:     size_t size{};
324:     if (str != nullptr)
325:     {
326:         size = strlen(str);
327:     }
328:     if ((size + 1) > m_allocatedSize)
329:     {
330:         if (!reallocate(size + 1))
331:             return *this;
332:     }
333:     if (str != nullptr)
334:     {
335:         strncpy(m_buffer, str, size);
336:     }
337:     m_end = m_buffer + size;
338:     m_buffer[size] = NullCharConst;
339:     return *this;
340: }
341: 
342: /// <summary>
343: /// assign a string value
344: ///
345: /// Assigns the specified string value, up to the specified count of characters, to the string.
346: /// </summary>
347: /// <param name="str">string value to assign to the string</param>
348: /// <param name="count">Maximum number of characters to copy from the string. If count is larger than the string length, the length of the string is used</param>
349: /// <returns>A reference to the string</returns>
350: string& string::assign(const ValueType* str, size_t count)
351: {
352:     size_t size{};
353:     if (str != nullptr)
354:     {
355:         size = strlen(str);
356:     }
357:     if (count < size)
358:         size = count;
359:     if ((size + 1) > m_allocatedSize)
360:     {
361:         if (!reallocate(size + 1))
362:             return *this;
363:     }
364:     if (str != nullptr)
365:     {
366:         strncpy(m_buffer, str, size);
367:     }
368:     m_end = m_buffer + size;
369:     m_buffer[size] = NullCharConst;
370:     return *this;
371: }
372: 
373: /// <summary>
374: /// assign a string value
375: ///
376: /// Assigns a string containing the specified count times the specified characters to the string
377: /// </summary>
378: /// <param name="count">Number copies of ch to copy to the string</param>
379: /// <param name="ch">Character to initialize with</param>
380: /// <returns>A reference to the string</returns>
381: string& string::assign(size_t count, ValueType ch)
382: {
383:     auto size = count;
384:     if ((size + 1) > m_allocatedSize)
385:     {
386:         if (!reallocate(size + 1))
387:             return *this;
388:     }
389:     memset(m_buffer, ch, size);
390:     m_end = m_buffer + size;
391:     m_buffer[size] = NullCharConst;
392:     return *this;
393: }
394: 
395: /// <summary>
396: /// assign a string value
397: ///
398: /// Assigns the specified string value to the string
399: /// </summary>
400: /// <param name="str">string value to assign to the string</param>
401: /// <returns>A reference to the string</returns>
402: string& string::assign(const string& str)
403: {
404:     auto size = str.length();
405:     if ((size + 1) > m_allocatedSize)
406:     {
407:         if (!reallocate(size + 1))
408:             return *this;
409:     }
410:     strncpy(m_buffer, str.data(), size);
411:     m_end = m_buffer + size;
412:     m_buffer[size] = NullCharConst;
413:     return *this;
414: }
415: 
416: /// <summary>
417: /// assign a string value
418: ///
419: /// Assigns the substring start from the specified position for the specified count of characters of specified string value to the string
420: /// </summary>
421: /// <param name="str">string value to assign to the string</param>
422: /// <param name="pos">Starting position of substring to copy from str</param>
423: /// <param name="count">Maximum number of characters to copy from str.
424: /// Default is until end of string. If pos + count is larger than the stirn length, characters are copied until end of string</param>
425: /// <returns>A reference to the string</returns>
426: string& string::assign(const string& str, size_t pos, size_t count /*= npos*/)
427: {
428:     auto size = str.length() - pos;
429:     if (count < size)
430:         size = count;
431:     if ((size + 1) > m_allocatedSize)
432:     {
433:         if (!reallocate(size + 1))
434:             return *this;
435:     }
436:     strncpy(m_buffer, str.data() + pos, size);
437:     m_end = m_buffer + size;
438:     m_buffer[size] = NullCharConst;
439:     return *this;
440: }
441: 
442: /// <summary>
443: /// Return the character at specified position
444: /// </summary>
445: /// <param name="pos">Position in string</param>
446: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
447: string::ValueType& string::at(size_t pos)
448: {
449:     if (pos >= length())
450:     {
451:         NullChar = NullCharConst;
452:         return NullChar;
453:     }
454:     return m_buffer[pos];
455: }
456: 
457: /// <summary>
458: /// Return the character at specified position
459: /// </summary>
460: /// <param name="pos">Position in string</param>
461: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, a reference to a const null character is returned (NullCharConst)</returns>
462: const string::ValueType& string::at(size_t pos) const
463: {
464:     if (pos >= length())
465:         return NullCharConst;
466:     return m_buffer[pos];
467: }
468: 
469: /// <summary>
470: /// Return the first character
471: /// </summary>
472: /// <returns>Returns a non-const reference to the first character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
473: string::ValueType& string::front()
474: {
475:     if (empty())
476:     {
477:         NullChar = NullCharConst;
478:         return NullChar;
479:     }
480:     return *m_buffer;
481: }
482: 
483: /// <summary>
484: /// Return the first character
485: /// </summary>
486: /// <returns>Returns a const reference to the first character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
487: const string::ValueType& string::front() const
488: {
489:     if (empty())
490:         return NullCharConst;
491:     return *m_buffer;
492: }
493: 
494: /// <summary>
495: /// Return the last character
496: /// </summary>
497: /// <returns>Returns a non-const reference to the last character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
498: string::ValueType& string::back()
499: {
500:     if (empty())
501:     {
502:         NullChar = NullCharConst;
503:         return NullChar;
504:     }
505:     return *(m_end - 1);
506: }
507: 
508: /// <summary>
509: /// Return the last character
510: /// </summary>
511: /// <returns>Returns a const reference to the last character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
512: const string::ValueType& string::back() const
513: {
514:     if (empty())
515:         return NullCharConst;
516:     return *(m_end - 1);
517: }
518: 
519: /// <summary>
520: /// Return the character at specified position
521: /// </summary>
522: /// <param name="pos">Position in string</param>
523: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
524: string::ValueType& string::operator[] (size_t pos)
525: {
526:     return m_buffer[pos];
527: }
528: 
529: /// <summary>
530: /// Return the character at specified position
531: /// </summary>
532: /// <param name="pos">Position in string</param>
533: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
534: const string::ValueType& string::operator[] (size_t pos) const
535: {
536:     return m_buffer[pos];
537: }
538: 
539: /// <summary>
540: /// Return the buffer pointer
541: /// </summary>
542: /// <returns>Returns a non-const pointer to the buffer. If the buffer is not allocated, a pointer to a non-const null character (NullChar, initialized before returning) is returned</returns>
543: string::ValueType* string::data()
544: {
545:     NullChar = NullCharConst;
546:     return (m_buffer == nullptr) ? &NullChar : m_buffer;
547: }
548: 
549: /// <summary>
550: /// Return the buffer pointer
551: /// </summary>
552: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
553: const string::ValueType* string::data() const
554: {
555:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
556: }
557: 
558: /// <summary>
559: /// Return the buffer pointer
560: /// </summary>
561: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
562: const string::ValueType* string::c_str() const
563: {
564:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
565: }
566: 
567: /// <summary>
568: /// Determine whether string is empty.
569: /// </summary>
570: /// <returns>Returns true when the string is empty (not allocated or no contents), false otherwise</returns>
571: bool string::empty() const
572: {
573:     return m_end == m_buffer;
574: }
575: 
576: /// <summary>
577: /// Return the size of the string
578: ///
579: /// This method is the equivalent of length().
580: /// </summary>
581: /// <returns>Returns the size (or length) of the string</returns>
582: size_t string::size() const
583: {
584:     return m_end - m_buffer;
585: }
586: 
587: /// <summary>
588: /// Return the length of the string
589: ///
590: /// This method is the equivalent of size().
591: /// </summary>
592: /// <returns>Returns the size (or length) of the string</returns>
593: size_t string::length() const
594: {
595:     return m_end - m_buffer;
596: }
597: 
598: /// <summary>
599: /// Return the capacity of the string
600: ///
601: /// The capacity is the size of the allocated buffer. The string can grow to that length before it needs to be re-allocated.
602: /// </summary>
603: /// <returns>Returns the size (or length) of the string</returns>
604: size_t string::capacity() const
605: {
606:     return m_allocatedSize;
607: }
608: 
609: /// <summary>
610: /// Reserved a buffer capacity
611: ///
612: /// Allocates a buffer of specified size
613: /// </summary>
614: /// <param name="newCapacity"></param>
615: /// <returns>Returns the capacity of the string</returns>
616: size_t string::reserve(size_t newCapacity)
617: {
618:     reallocate_allocation_size(newCapacity);
619:     return m_allocatedSize;
620: }
621: 
622: /// <summary>
623: /// append operator
624: ///
625: /// Appends a character to the string
626: /// </summary>
627: /// <param name="ch">Character to append</param>
628: /// <returns>Returns a reference to the string</returns>
629: string& string::operator +=(ValueType ch)
630: {
631:     append(1, ch);
632:     return *this;
633: }
634: 
635: /// <summary>
636: /// append operator
637: ///
638: /// Appends a string to the string
639: /// </summary>
640: /// <param name="str">string to append</param>
641: /// <returns>Returns a reference to the string</returns>
642: string& string::operator +=(const string& str)
643: {
644:     append(str);
645:     return *this;
646: }
647: 
648: /// <summary>
649: /// append operator
650: ///
651: /// Appends a string to the string
652: /// </summary>
653: /// <param name="str">string to append. If nullptr the nothing is appended</param>
654: /// <returns>Returns a reference to the string</returns>
655: string& string::operator +=(const ValueType* str)
656: {
657:     append(str);
658:     return *this;
659: }
660: 
661: /// <summary>
662: /// append operator
663: ///
664: /// Appends a sequence of count times the same character ch to the string
665: /// </summary>
666: /// <param name="count">Number of characters to append</param>
667: /// <param name="ch">Character to append</param>
668: void string::append(size_t count, ValueType ch)
669: {
670:     auto len = length();
671:     auto size = len + count;
672:     if ((size + 1) > m_allocatedSize)
673:     {
674:         if (!reallocate(size + 1))
675:             return;
676:     }
677:     memset(m_buffer + len, ch, count);
678:     m_end = m_buffer + size;
679:     m_buffer[size] = NullCharConst;
680: }
681: 
682: /// <summary>
683: /// append operator
684: ///
685: /// Appends a string to the string
686: /// </summary>
687: /// <param name="str">string to append</param>
688: void string::append(const string& str)
689: {
690:     auto len = length();
691:     auto strLength = str.length();
692:     auto size = len + strLength;
693:     if ((size + 1) > m_allocatedSize)
694:     {
695:         if (!reallocate(size + 1))
696:             return;
697:     }
698:     strncpy(m_buffer + len, str.data(), strLength);
699:     m_end = m_buffer + size;
700:     m_buffer[size] = NullCharConst;
701: }
702: 
703: /// <summary>
704: /// append operator
705: ///
706: /// Appends a substring of str to the string
707: /// </summary>
708: /// <param name="str">string to append from</param>
709: /// <param name="pos">Start position in str to copy characters from</param>
710: /// <param name="count">Number of characters to copy from str. Default is until the end of the string. If count is larger than the string length, characters are copied up to the end of the string</param>
711: void string::append(const string& str, size_t pos, size_t count /*= npos*/)
712: {
713:     auto len = length();
714:     auto strLength = str.length();
715:     auto strCount = count;
716:     if (pos >= strLength)
717:         return;
718:     if (pos + strCount > strLength)
719:         strCount = strLength - pos;
720:     auto size = len + strCount;
721:     if ((size + 1) > m_allocatedSize)
722:     {
723:         if (!reallocate(size + 1))
724:             return;
725:     }
726:     strncpy(m_buffer + len, str.data() + pos, strCount);
727:     m_end = m_buffer + size;
728:     m_buffer[size] = NullCharConst;
729: }
730: 
731: /// <summary>
732: /// append operator
733: ///
734: /// Appends a string to the string
735: /// </summary>
736: /// <param name="str">string to append. If nullptr the nothing is appended</param>
737: void string::append(const ValueType* str)
738: {
739:     if (str == nullptr)
740:         return;
741:     auto len = length();
742:     auto strLength = strlen(str);
743:     auto size = len + strLength;
744:     if ((size + 1) > m_allocatedSize)
745:     {
746:         if (!reallocate(size + 1))
747:             return;
748:     }
749:     strncpy(m_buffer + len, str, strLength);
750:     m_end = m_buffer + size;
751:     m_buffer[size] = NullCharConst;
752: }
753: 
754: /// <summary>
755: /// append operator
756: ///
757: /// Appends a number of characters from str to the string
758: /// </summary>
759: /// <param name="str">string to append. If nullptr the nothing is appended</param>
760: /// <param name="count">Number of characters to copy from str. If count is larger than the string length, the complete string is copied</param>
761: void string::append(const ValueType* str, size_t count)
762: {
763:     if (str == nullptr)
764:         return;
765:     auto len = length();
766:     auto strLength = strlen(str);
767:     auto strCount = count;
768:     if (strCount > strLength)
769:         strCount = strLength;
770:     auto size = len + strCount;
771:     if ((size + 1) > m_allocatedSize)
772:     {
773:         if (!reallocate(size + 1))
774:             return;
775:     }
776:     strncpy(m_buffer + len, str, strCount);
777:     m_end = m_buffer + size;
778:     m_buffer[size] = NullCharConst;
779: }
780: 
781: /// <summary>
782: /// clear the string
783: ///
784: /// Clears the contents of the string, but does not free or reallocate the buffer
785: /// </summary>
786: void string::clear()
787: {
788:     if (!empty())
789:     {
790:         m_end = m_buffer;
791:         m_buffer[0] = NullCharConst;
792:     }
793: }
794: 
795: /// <summary>
796: /// find a substring in the string
797: ///
798: /// If empty string, always finds the string.
799: /// </summary>
800: /// <param name="str">Substring to find</param>
801: /// <param name="pos">Starting position in string to start searching</param>
802: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
803: size_t string::find(const string& str, size_t pos /*= 0*/) const
804: {
805:     auto len = length();
806:     auto patternLength = str.length();
807:     if (pos >= len)
808:         return npos;
809:     auto needle = str.data();
810:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
811:     {
812:         if (memcmp(haystack, needle, patternLength) == 0)
813:             return haystack - m_buffer;
814:     }
815:     return npos;
816: }
817: 
818: /// <summary>
819: /// find a substring in the string
820: ///
821: /// If nullptr or empty string, always finds the string.
822: /// </summary>
823: /// <param name="str">Substring to find</param>
824: /// <param name="pos">Starting position in string to start searching</param>
825: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
826: size_t string::find(const ValueType* str, size_t pos /*= 0*/) const
827: {
828:     size_t strLength{};
829:     if (str != nullptr)
830:     {
831:         strLength = strlen(str);
832:     }
833:     auto len = length();
834:     auto patternLength = strLength;
835:     if (pos >= len)
836:         return npos;
837:     auto needle = str;
838:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
839:     {
840:         if (memcmp(haystack, needle, patternLength) == 0)
841:             return haystack - m_buffer;
842:     }
843:     return npos;
844: }
845: 
846: /// <summary>
847: /// find a substring in the string
848: ///
849: /// If nullptr or empty string, always finds the string.
850: /// </summary>
851: /// <param name="str">Substring to find</param>
852: /// <param name="pos">Starting position in string to start searching</param>
853: /// <param name="count">Number of characters from str to compare</param>
854: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
855: size_t string::find(const ValueType* str, size_t pos, size_t count) const
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
866:     if (count < patternLength)
867:         patternLength = count;
868:     auto needle = str;
869:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
870:     {
871:         if (memcmp(haystack, needle, patternLength) == 0)
872:             return haystack - m_buffer;
873:     }
874:     return npos;
875: }
876: 
877: /// <summary>
878: /// find a character in the string
879: /// </summary>
880: /// <param name="ch">Character to find</param>
881: /// <param name="pos">Starting position in string to start searching</param>
882: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
883: size_t string::find(ValueType ch, size_t pos /*= 0*/) const
884: {
885:     auto len = length();
886:     if (pos >= len)
887:         return npos;
888:     for (const ValueType* haystack = data() + pos; haystack <= m_end; ++haystack)
889:     {
890:         if (*haystack == ch)
891:             return haystack - m_buffer;
892:     }
893:     return npos;
894: }
895: 
896: /// <summary>
897: /// Check whether string starts with character
898: /// </summary>
899: /// <param name="ch">Character to find</param>
900: /// <returns>Returns true if ch is first character in string, false otherwise</returns>
901: bool string::starts_with(ValueType ch) const
902: {
903:     if (empty())
904:         return false;
905:     return m_buffer[0] == ch;
906: }
907: 
908: /// <summary>
909: /// Check whether string starts with substring
910: /// </summary>
911: /// <param name="str">SubString to find</param>
912: /// <returns>Returns true if str is first part of string, false otherwise</returns>
913: bool string::starts_with(const string& str) const
914: {
915:     auto len = length();
916:     auto strLength = str.length();
917:     if (strLength >= len)
918:         return false;
919: 
920:     return memcmp(data(), str.data(), strLength) == 0;
921: }
922: 
923: /// <summary>
924: /// Check whether string starts with substring
925: /// </summary>
926: /// <param name="str">SubString to find</param>
927: /// <returns>Returns true if str is first part of string, false otherwise</returns>
928: bool string::starts_with(const ValueType* str) const
929: {
930:     size_t strLength{};
931:     if (str != nullptr)
932:     {
933:         strLength = strlen(str);
934:     }
935:     auto len = length();
936:     if (strLength >= len)
937:         return false;
938: 
939:     return memcmp(data(), str, strLength) == 0;
940: }
941: 
942: /// <summary>
943: /// Check whether string ends with character
944: /// </summary>
945: /// <param name="ch">Character to find</param>
946: /// <returns>Returns true if ch is last character in string, false otherwise</returns>
947: bool string::ends_with(ValueType ch) const
948: {
949:     if (empty())
950:         return false;
951:     return m_buffer[length() - 1] == ch;
952: }
953: 
954: /// <summary>
955: /// Check whether string ends with substring
956: /// </summary>
957: /// <param name="str">SubString to find</param>
958: /// <returns>Returns true if str is last part of string, false otherwise</returns>
959: bool string::ends_with(const string& str) const
960: {
961:     auto len = length();
962:     auto strLength = str.length();
963:     if (strLength >= len)
964:         return false;
965: 
966:     return memcmp(m_end - strLength, str.data(), strLength) == 0;
967: }
968: 
969: /// <summary>
970: /// Check whether string ends with substring
971: /// </summary>
972: /// <param name="str">SubString to find</param>
973: /// <returns>Returns true if str is last part of string, false otherwise</returns>
974: bool string::ends_with(const ValueType* str) const
975: {
976:     size_t strLength{};
977:     if (str != nullptr)
978:     {
979:         strLength = strlen(str);
980:     }
981:     auto len = length();
982:     if (strLength >= len)
983:         return false;
984: 
985:     return memcmp(m_end - strLength, str, strLength) == 0;
986: }
987: 
988: /// <summary>
989: /// Check whether string contains character
990: /// </summary>
991: /// <param name="ch">Character to find</param>
992: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
993: bool string::contains(ValueType ch) const
994: {
995:     return find(ch) != npos;
996: }
997: 
998: /// <summary>
999: /// Check whether string contains substring
1000: /// </summary>
1001: /// <param name="str">Substring to find</param>
1002: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1003: bool string::contains(const string& str) const
1004: {
1005:     return find(str) != npos;
1006: }
1007: 
1008: /// <summary>
1009: /// Check whether string contains substring
1010: /// </summary>
1011: /// <param name="str">Substring to find</param>
1012: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1013: bool string::contains(const ValueType* str) const
1014: {
1015:     return find(str) != npos;
1016: }
1017: 
1018: /// <summary>
1019: /// Return substring
1020: /// </summary>
1021: /// <param name="pos">Starting position of substring in string</param>
1022: /// <param name="count">length of substring to return. If count is larger than the number of characters available from position pos, the rest of the string is returned</param>
1023: /// <returns>Returns the substring at position [pos, pos + count), if available </returns>
1024: string string::substr(size_t pos /*= 0*/, size_t count /*= npos*/) const
1025: {
1026:     string result;
1027:     auto len = length();
1028:     auto size = count;
1029:     if (pos < len)
1030:     {
1031:         if (count > len - pos)
1032:             count = len - pos;
1033:         result.reallocate(count + 1);
1034:         memcpy(result.data(), data() + pos, count);
1035:         result.data()[count] = NullCharConst;
1036:     }
1037: 
1038:     return result;
1039: }
1040: 
1041: /// <summary>
1042: /// Case sensitive equality to string
1043: /// </summary>
1044: /// <param name="other">string to compare to</param>
1045: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1046: bool string::equals(const string& other) const
1047: {
1048:     return compare(other) == 0;
1049: }
1050: 
1051: /// <summary>
1052: /// Case sensitive equality to string
1053: /// </summary>
1054: /// <param name="other">string to compare to</param>
1055: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1056: bool string::equals(const ValueType* other) const
1057: {
1058:     if (other == nullptr)
1059:         return empty();
1060:     if (length() != strlen(other))
1061:         return false;
1062:     return strcmp(data(), other) == 0;
1063: }
1064: 
1065: /// <summary>
1066: /// Case insensitive equality to string
1067: /// </summary>
1068: /// <param name="other">string to compare to</param>
1069: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1070: bool string::equals_case_insensitive(const string& other) const
1071: {
1072:     if (length() != other.length())
1073:         return false;
1074:     if (empty())
1075:         return true;
1076:     return strcasecmp(data(), other.data()) == 0;
1077: }
1078: 
1079: /// <summary>
1080: /// Case insensitive equality to string
1081: /// </summary>
1082: /// <param name="other">string to compare to</param>
1083: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1084: bool string::equals_case_insensitive(const ValueType* other) const
1085: {
1086:     if (other == nullptr)
1087:         return empty();
1088:     if (length() != strlen(other))
1089:         return false;
1090:     return strcasecmp(data(), other) == 0;
1091: }
1092: 
1093: /// <summary>
1094: /// Case sensitive compare to string
1095: ///
1096: /// Compares the complete string, character by character
1097: /// </summary>
1098: /// <param name="str">string to compare to</param>
1099: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1100: int string::compare(const string& str) const
1101: {
1102:     if (empty())
1103:     {
1104:         if (str.empty())
1105:             return 0;
1106:         return -1;
1107:     }
1108:     if (str.empty())
1109:         return 1;
1110: 
1111:     return strcmp(data(), str.data());
1112: }
1113: 
1114: /// <summary>
1115: /// Case sensitive compare to string
1116: ///
1117: /// Compares the substring from pos to pos+count to str
1118: /// </summary>
1119: /// <param name="pos">Starting position of substring to compare to str</param>
1120: /// <param name="count">Number of characters in substring to compare to str</param>
1121: /// <param name="str">string to compare to</param>
1122: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1123: int string::compare(size_t pos, size_t count, const string& str) const
1124: {
1125:     return substr(pos, count).compare(str);
1126: }
1127: 
1128: /// <summary>
1129: /// Case sensitive compare to string
1130: ///
1131: /// Compares the substring from pos to pos+count to the substring from strPos to strPos+strCount of str
1132: /// </summary>
1133: /// <param name="pos">Starting position of substring to compare to str</param>
1134: /// <param name="count">Number of characters in substring to compare to str</param>
1135: /// <param name="str">string to compare to</param>
1136: /// <param name="strPos">Starting position of substring of str to compare</param>
1137: /// <param name="strCount">Number of characters in substring of str to compare</param>
1138: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1139: int string::compare(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/) const
1140: {
1141:     return substr(pos, count).compare(str.substr(strPos, strCount));
1142: }
1143: 
1144: /// <summary>
1145: /// Case sensitive compare to string
1146: ///
1147: /// Compares the complete string to str
1148: /// </summary>
1149: /// <param name="str">string to compare to</param>
1150: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1151: int string::compare(const ValueType* str) const
1152: {
1153:     size_t strLength{};
1154:     if (str != nullptr)
1155:         strLength = strlen(str);
1156:     if (empty())
1157:     {
1158:         if (strLength == 0)
1159:             return 0;
1160:         return -1;
1161:     }
1162:     if (strLength == 0)
1163:         return 1;
1164: 
1165:     return strcmp(data(), str);
1166: }
1167: 
1168: /// <summary>
1169: /// Case sensitive compare to string
1170: ///
1171: /// Compares the substring from pos to pos+count to str
1172: /// </summary>
1173: /// <param name="pos">Starting position of substring to compare to str</param>
1174: /// <param name="count">Number of characters in substring to compare to str</param>
1175: /// <param name="str">string to compare to</param>
1176: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1177: int string::compare(size_t pos, size_t count, const ValueType* str) const
1178: {
1179:     size_t strLength{};
1180:     if (str != nullptr)
1181:         strLength = strlen(str);
1182: 
1183:     auto len = length();
1184:     if (pos >= len)
1185:         len = 0;
1186:     len -= pos;
1187:     if (count < len)
1188:         len = count;
1189:     if (len == 0)
1190:     {
1191:         if (strLength == 0)
1192:             return 0;
1193:         return -1;
1194:     }
1195:     if (strLength == 0)
1196:         return 1;
1197: 
1198:     auto maxLen = strLength;
1199:     if (maxLen < len)
1200:         maxLen = len;
1201:     return strncmp(data() + pos, str, maxLen);
1202: }
1203: 
1204: /// <summary>
1205: /// Case sensitive compare to string
1206: ///
1207: /// Compares the substring from pos to pos+count to the first strCount characters of str
1208: /// </summary>
1209: /// <param name="pos">Starting position of substring to compare to str</param>
1210: /// <param name="count">Number of characters in substring to compare to str</param>
1211: /// <param name="str">string to compare to</param>
1212: /// <param name="strCount">Number of characters in substring of str to compare</param>
1213: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1214: int string::compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const
1215: {
1216:     size_t strLength{};
1217:     if (str != nullptr)
1218:         strLength = strlen(str);
1219: 
1220:     auto len = length();
1221:     if (pos >= len)
1222:         len = 0;
1223:     len -= pos;
1224:     if (count < len)
1225:         len = count;
1226: 
1227:     if (strCount < strLength)
1228:         strLength = strCount;
1229: 
1230:     if (len == 0)
1231:     {
1232:         if (strLength == 0)
1233:             return 0;
1234:         return -1;
1235:     }
1236:     if (strLength == 0)
1237:         return 1;
1238: 
1239:     auto maxLen = strLength;
1240:     if (maxLen < len)
1241:         maxLen = len;
1242:     return strncmp(data() + pos, str, maxLen);
1243: }
1244: 
1245: /// <summary>
1246: /// replace substring
1247: ///
1248: /// Replaces the substring from pos to pos+count with str
1249: /// </summary>
1250: /// <param name="pos">Starting position of substring to replace</param>
1251: /// <param name="count">Number of characters in substring to replace</param>
1252: /// <param name="str">string to replace with</param>
1253: /// <returns>Returns the reference to the resulting string</returns>
1254: string& string::replace(size_t pos, size_t count, const string& str)
1255: {
1256:     string result = substr(0, pos) + str + substr(pos + count);
1257:     assign(result);
1258:     return *this;
1259: }
1260: 
1261: /// <summary>
1262: /// replace substring
1263: ///
1264: /// Replaces the substring from pos to pos+count with the substring from strPos to strPos+strCount of str
1265: /// </summary>
1266: /// <param name="pos">Starting position of substring to replace</param>
1267: /// <param name="count">Number of characters in substring to replace</param>
1268: /// <param name="str">string to replace with</param>
1269: /// <param name="strPos">Starting position of substring of str to replace with</param>
1270: /// <param name="strCount">Number of characters in substring of str to replace with</param>
1271: /// <returns>Returns the reference to the resulting string</returns>
1272: string& string::replace(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/)
1273: {
1274:     string result = substr(0, pos) + str.substr(strPos, strCount) + substr(pos + count);
1275:     assign(result);
1276:     return *this;
1277: }
1278: 
1279: /// <summary>
1280: /// replace substring
1281: ///
1282: /// Replaces the substring from pos to pos+count with str
1283: /// </summary>
1284: /// <param name="pos">Starting position of substring to replace</param>
1285: /// <param name="count">Number of characters in substring to replace</param>
1286: /// <param name="str">string to replace with</param>
1287: /// <returns>Returns the reference to the resulting string</returns>
1288: string& string::replace(size_t pos, size_t count, const ValueType* str)
1289: {
1290:     string result = substr(0, pos) + str + substr(pos + count);
1291:     assign(result);
1292:     return *this;
1293: }
1294: 
1295: /// <summary>
1296: /// replace substring
1297: ///
1298: /// Replaces the substring from pos to pos+count with the first strCount characters of str
1299: /// </summary>
1300: /// <param name="pos">Starting position of substring to replace</param>
1301: /// <param name="count">Number of characters in substring to replace</param>
1302: /// <param name="str">string to replace with</param>
1303: /// <param name="strCount">Number of characters in substring to replace with</param>
1304: /// <returns>Returns the reference to the resulting string</returns>
1305: string& string::replace(size_t pos, size_t count, const ValueType* str, size_t strCount)
1306: {
1307:     string result = substr(0, pos) + string(str, strCount) + substr(pos + count);
1308:     assign(result);
1309:     return *this;
1310: }
1311: 
1312: /// <summary>
1313: /// replace substring
1314: ///
1315: /// Replaces the substring from pos to pos+count with ch
1316: /// </summary>
1317: /// <param name="pos">Starting position of substring to replace</param>
1318: /// <param name="count">Number of characters in substring to replace</param>
1319: /// <param name="ch">Characters to replace with</param>
1320: /// <returns>Returns the reference to the resulting string</returns>
1321: string& string::replace(size_t pos, size_t count, ValueType ch)
1322: {
1323:     return replace(pos, count, ch, 1);
1324: }
1325: 
1326: /// <summary>
1327: /// replace substring
1328: ///
1329: /// Replaces the substring from pos to pos+count with a sequence of chCount copies of ch
1330: /// </summary>
1331: /// <param name="pos">Starting position of substring to replace</param>
1332: /// <param name="count">Number of characters in substring to replace</param>
1333: /// <param name="ch">Characters to replace with</param>
1334: /// <param name="chCount">Number of copies of ch to replace with</param>
1335: /// <returns>Returns the reference to the resulting string</returns>
1336: string& string::replace(size_t pos, size_t count, ValueType ch, size_t chCount)
1337: {
1338:     string result = substr(0, pos) + string(chCount, ch) + substr(pos + count);
1339:     assign(result);
1340:     return *this;
1341: }
1342: 
1343: /// <summary>
1344: /// replace substring
1345: ///
1346: /// Replaces all instances of the substring oldStr (if existing) with newStr
1347: /// </summary>
1348: /// <param name="oldStr">string to find in string</param>
1349: /// <param name="newStr">string to replace with</param>
1350: /// <returns>Returns the number of times the string was replaced</returns>
1351: int string::replace(const string& oldStr, const string& newStr)
1352: {
1353:     size_t pos = find(oldStr);
1354:     size_t oldLength = oldStr.length();
1355:     size_t newLength = newStr.length();
1356:     int count = 0;
1357:     while (pos != npos)
1358:     {
1359:         replace(pos, oldLength, newStr);
1360:         pos += newLength;
1361:         pos = find(oldStr, pos);
1362:         count++;
1363:     }
1364:     return count;
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
1375: int string::replace(const ValueType* oldStr, const ValueType* newStr)
1376: {
1377:     if ((oldStr == nullptr) || (newStr == nullptr))
1378:         return 0;
1379:     size_t pos = find(oldStr);
1380:     size_t oldLength = strlen(oldStr);
1381:     size_t newLength = strlen(newStr);
1382:     int count = 0;
1383:     while (pos != npos)
1384:     {
1385:         replace(pos, oldLength, newStr);
1386:         pos += newLength;
1387:         pos = find(oldStr, pos);
1388:         count++;
1389:     }
1390:     return count;
1391: }
1392: 
1393: /// <summary>
1394: /// Align string
1395: ///
1396: /// Pads the string on the left (width > 0) or on the right (width < 0) up to a length of width characters. If the string is larger than width characters, it is not modified.
1397: /// </summary>
1398: /// <param name="width">length of target string. If width < 0, the string is padded to the right with spaces up to -width characters.
1399: /// if width > 0, the string is padded to the left with space up to width characters</param>
1400: /// <returns>Returns the number of times the string was replaced</returns>
1401: string string::align(int width) const
1402: {
1403:     string result;
1404:     int    absWidth = (width > 0) ? width : -width;
1405:     auto   len = length();
1406:     if (static_cast<size_t>(absWidth) > len)
1407:     {
1408:         if (width < 0)
1409:         {
1410:             result = *this + string(static_cast<unsigned int>(-width) - len, ' ');
1411:         }
1412:         else
1413:         {
1414:             result = string(static_cast<unsigned int>(width) - len, ' ') + *this;
1415:         }
1416:     }
1417:     else
1418:         result = *this;
1419:     return result;
1420: }
1421: 
1422: /// <summary>
1423: /// Allocate or re-allocate string to have a capacity of requestedLength characters
1424: /// </summary>
1425: /// <param name="requestedLength">Amount of characters in the string to allocate space for</param>
1426: /// <returns>True if successful, false otherwise</returns>
1427: bool string::reallocate(size_t requestedLength)
1428: {
1429:     auto requestedSize = requestedLength;
1430:     auto allocationSize = NextPowerOf2((requestedSize < MinimumAllocationSize) ? MinimumAllocationSize : requestedSize);
1431: 
1432:     if (!reallocate_allocation_size(allocationSize))
1433:         return false;
1434:     return true;
1435: }
1436: 
1437: /// <summary>
1438: /// Allocate or re-allocate string to have a capacity of allocationSize bytes
1439: /// </summary>
1440: /// <param name="allocationSize">Amount of bytes to allocate space for</param>
1441: /// <returns>True if successful, false otherwise</returns>
1442: bool string::reallocate_allocation_size(size_t allocationSize)
1443: {
1444:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1445:     if (newBuffer == nullptr)
1446:     {
1447:         LOG_NO_ALLOC_DEBUG("Alloc failed!");
1448:         return false;
1449:     }
1450:     m_buffer = newBuffer;
1451: #if BAREMETAL_MEMORY_TRACING_DETAIL
1452:     LOG_NO_ALLOC_DEBUG("Alloc string %p", m_buffer);
1453: #endif
1454:     if (m_end == nullptr)
1455:         m_end = m_buffer;
1456:     if (m_end > m_buffer + allocationSize)
1457:         m_end = m_buffer + allocationSize;
1458:     m_allocatedSize = allocationSize;
1459:     return true;
1460: }
```

As you can see, this is a very extensive class.
A few remarks:
- Line 52: We use a minimum allocation size of 64, meaning that every string will have storage space for 63 characters plus a null character for termination, at the least.
- Line 55: We use a special static variable `NullCharConst` when we need to return something constant, but the string is not allocated.
- Line 57: Similarly, we use a special static variable `NullChar` when we need to return something non constant, but the string is not allocated.
This variable is non-const, so will have to be initialized every time we return it, to make sure it is still a null character.
- Line 181-189: We implement a move constructor, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 256-268: We implement a move assignment, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 276-290: We use our own version of a iterator template, which we'll get to, to enable creating the begin() and end() methods. These allow us to use the standard c++ `for (x : string)` construct
- Line 298-312: We also use our own version of a const_iterator template for const iterators, which we'll get to, to enable creating the begin() and end() const methods.
- Line 993-1016: We also have the `contains()` methods, which are new to C++23, however they are quite convenient
- Line 1046-1091: We use the methods `equals()` and `equals_case_insensititive()` to compare strings. Both are not standard, but convenient
- Line 1351-1391: We implement two extra variants of `replace()` (replacing multiple instances of a string) for convenience
- Line 1401-1420: Again for convenience, we implement the `align()` method. We'll be using this later on for aligning strings when formatting

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
42: /// <summary>
43: /// Const iterator template
44: /// 
45: /// Based on arrays of type T pointed to by const pointers.
46: /// </summary>
47: /// <typeparam name="T">Value type to iterate through</typeparam>
48: template<class T>
49: class const_iterator
50: {
51: private:
52:     /// @brief Start of the range
53:     T const* m_begin;
54:     /// @brief End of the range + 1
55:     T const* m_end;
56:     /// @brief Current position
57:     T const* m_current;
58: 
59: public:
60:     /// <summary>
61:     /// Construct a const_iterator
62:     /// </summary>
63:     /// <param name="begin">Start of the range</param>
64:     /// <param name="end">End of the range + 1</param>
65:     explicit const_iterator(const T* begin, const T* end)
66:         : m_begin{ begin }
67:         , m_end{ end }
68:         , m_current{ begin }
69:     {
70:     }
71:     /// <summary>
72:     /// Advance in range by 1 step (post increment)
73:     /// </summary>
74:     /// <returns>Position after advancing as const_iterator</returns>
75:     const_iterator& operator++()
76:     {
77:         ++m_current;
78:         return *this;
79:     }
80:     /// <summary>
81:     /// Advance in range by 1 step (pre increment)
82:     /// </summary>
83:     /// <returns>Position before advancing as const_iterator</returns>
84:     const_iterator operator++(int)
85:     {
86:         const_iterator retval = *this;
87:         ++(*this);
88:         return retval;
89:     }
90:     /// <summary>
91:     /// Equality comparison
92:     /// 
93:     /// Compares current position with current position in passed iterator
94:     /// </summary>
95:     /// <param name="other">const_iterator to compare with</param>
96:     /// <returns>Returns true if positions are equal, false if not</returns>
97:     bool operator==(const const_iterator& other) const
98:     {
99:         return m_current == other.m_current;
100:     }
101:     /// <summary>
102:     /// Inequality comparison
103:     /// 
104:     /// Compares current position with current position in passed iterator
105:     /// </summary>
106:     /// <param name="other">const_iterator to compare with</param>
107:     /// <returns>Returns false if positions are equal, true if not</returns>
108:     bool operator!=(const const_iterator& other) const
109:     {
110:         return !(*this == other);
111:     }
112:     /// <summary>
113:     /// Return value at current location
114:     /// </summary>
115:     /// <returns>Value to which the current position points</returns>
116:     T const& operator*() const
117:     {
118:         return *m_current;
119:     }
120: };
121: 
122: /// <summary>
123: /// Non-const iterator template
124: /// 
125: /// Based on arrays of type T pointed to by pointers.
126: /// </summary>
127: /// <typeparam name="T">Value type to iterate through</typeparam>
128: template<class T>
129: class iterator
130: {
131: private:
132:     /// @brief Start of the range
133:     T* m_begin;
134:     /// @brief End of the range + 1
135:     T* m_end;
136:     /// @brief Current position
137:     T* m_current;
138: 
139: public:
140:     /// <summary>
141:     /// Construct a iterator
142:     /// </summary>
143:     /// <param name="begin">Start of the range</param>
144:     /// <param name="end">End of the range + 1</param>
145:     explicit iterator(T* begin, T* end)
146:         : m_begin{ begin }
147:         , m_end{ end }
148:         , m_current{ begin }
149:     {
150:     }
151:     /// <summary>
152:     /// Advance in range by 1 step (post increment)
153:     /// </summary>
154:     /// <returns>Position after advancing as iterator</returns>
155:     iterator& operator++()
156:     {
157:         ++m_current;
158:         return *this;
159:     }
160:     /// <summary>
161:     /// Advance in range by 1 step (pre increment)
162:     /// </summary>
163:     /// <returns>Position before advancing as iterator</returns>
164:     iterator operator++(int)
165:     {
166:         iterator retval = *this;
167:         ++(*this);
168:         return retval;
169:     }
170:     /// <summary>
171:     /// Equality comparison
172:     /// 
173:     /// Compares current position with current position in passed iterator
174:     /// </summary>
175:     /// <param name="other">iterator to compare with</param>
176:     /// <returns>Returns true if positions are equal, false if not</returns>
177:     bool operator==(const iterator& other) const
178:     {
179:         return m_current == other.m_current;
180:     }
181:     /// <summary>
182:     /// Inequality comparison
183:     /// 
184:     /// Compares current position with current position in passed iterator
185:     /// </summary>
186:     /// <param name="other">iterator to compare with</param>
187:     /// <returns>Returns false if positions are equal, true if not</returns>
188:     bool operator!=(const iterator& other) const
189:     {
190:         return !(*this == other);
191:     }
192:     /// <summary>
193:     /// Return value at current location
194:     /// </summary>
195:     /// <returns>Value to which the current position points</returns>
196:     T& operator*() const
197:     {
198:         return *m_current;
199:     }
200: };
201: 
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
...
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

- Line 53: We add the standard `memcmp` function
- Line 55-56: We add the standard `toupper` amd `tolower` functions
- Line 58-61: We add the standard `strcmp` (compare strings to end of string), `strncmp` (compare strings with maximum size), and the case insensitive variants `strcasecmp` and `strncasecmp`
- Line 79-89: We add an inline function to compute the number of bits b required to hold a value n, so 2^b >= n
- Line 96-99: We add an inline function to compute the value x larger or equal to a number n, such that x is a power of 2

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
File: d:\Projects\baremetal.github\code\libraries\baremetal\src\Util.cpp
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

Next: [16-unit-tests](16-unit-tests.md)

