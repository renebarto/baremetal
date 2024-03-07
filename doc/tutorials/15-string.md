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
59: /// @brief Define log name
60: LOG_MODULE("String");
61: 
62: /// <summary>
63: /// Default constructor
64: ///
65: /// Constructs an empty string.
66: /// </summary>
67: string::string()
68:     : m_buffer{}
69:     , m_end{}
70:     , m_allocatedSize{}
71: {
72: }
73: 
74: /// <summary>
75: /// Destructor
76: ///
77: /// Frees any allocated memory.
78: /// </summary>
79: string::~string()
80: {
81: #if BAREMETAL_MEMORY_TRACING_DETAIL
82:     if (m_buffer != nullptr)
83:         LOG_NO_ALLOC_DEBUG("Free string %p", m_buffer);
84: #endif
85:     delete[] m_buffer;
86: }
87: 
88: /// <summary>
89: /// Constructor
90: ///
91: /// Initializes the string with the specified string.
92: /// </summary>
93: /// <param name="str">string to initialize with</param>
94: string::string(const ValueType* str)
95:     : m_buffer{}
96:     , m_end{}
97:     , m_allocatedSize{}
98: {
99:     if (str == nullptr)
100:         return;
101:     auto size = strlen(str);
102:     if (reallocate(size + 1))
103:     {
104:         strncpy(m_buffer, str, size);
105:     }
106:     m_end = m_buffer + size;
107:     m_buffer[size] = NullCharConst;
108: }
109: 
110: /// <summary>
111: /// Constructor
112: ///
113: /// Initializes the string with up to count characters in the specified string. A null character is always added.
114: /// </summary>
115: /// <param name="str">string to initialize with</param>
116: /// <param name="count">Maximum number of characters from str to initialize with. If count is larger than the actual string length, only the string length is used</param>
117: string::string(const ValueType* str, size_t count)
118:     : m_buffer{}
119:     , m_end{}
120:     , m_allocatedSize{}
121: {
122:     if (str == nullptr)
123:         return;
124:     auto size = strlen(str);
125:     if (count < size)
126:         size = count;
127:     if (reallocate(size + 1))
128:     {
129:         strncpy(m_buffer, str, size);
130:     }
131:     m_end = m_buffer + size;
132:     m_buffer[size] = NullCharConst;
133: }
134: 
135: /// <summary>
136: /// Constructor
137: ///
138: /// Initializes the string with the specified count times the specified character. A null character is always added.
139: /// </summary>
140: /// <param name="count">Number of characters of value ch to initialized with</param>
141: /// <param name="ch">Character to initialize with</param>
142: string::string(size_t count, ValueType ch)
143:     : m_buffer{}
144:     , m_end{}
145:     , m_allocatedSize{}
146: {
147:     auto size = count;
148:     if (reallocate(size + 1))
149:     {
150:         memset(m_buffer, ch, size);
151:     }
152:     m_end = m_buffer + size;
153:     m_buffer[size] = NullCharConst;
154: }
155: 
156: /// <summary>
157: /// Copy constructor
158: ///
159: /// Initializes the string with the specified string value.
160: /// </summary>
161: /// <param name="other">string to initialize with</param>
162: string::string(const string& other)
163:     : m_buffer{}
164:     , m_end{}
165:     , m_allocatedSize{}
166: {
167:     auto size = other.length();
168:     if (reallocate(size + 1))
169:     {
170:         strncpy(m_buffer, other.data(), size);
171:     }
172:     m_end = m_buffer + size;
173:     m_buffer[size] = NullCharConst;
174: }
175: 
176: /// <summary>
177: /// Move constructor
178: ///
179: /// Initializes the string by moving the contents from the specified string value.
180: /// </summary>
181: /// <param name="other">string to initialize with</param>
182: string::string(string&& other)
183:     : m_buffer{other.m_buffer}
184:     , m_end{other.m_end}
185:     , m_allocatedSize{other.m_allocatedSize}
186: {
187:     other.m_buffer = nullptr;
188:     other.m_end = nullptr;
189:     other.m_allocatedSize = 0;
190: }
191: 
192: /// <summary>
193: /// Constructor
194: ///
195: /// Initializes the string with the substring starting at specified position, for the specified number of characters, from the specified string value.
196: /// </summary>
197: /// <param name="other">string to initialize with</param>
198: /// <param name="pos">Position in other to start copying charaters from</param>
199: /// <param name="count">Maximum number of characters to copy from other. Default is until end of string. If pos + count is larger than the actual length of the string, string string is copied until the end</param>
200: string::string(const string& other, size_t pos, size_t count /*= npos*/)
201:     : m_buffer{}
202:     , m_end{}
203:     , m_allocatedSize{}
204: {
205:     auto size = other.length() - pos;
206:     if (count < size)
207:         size = count;
208:     if (reallocate(size + 1))
209:     {
210:         strncpy(m_buffer, other.data() + pos, size);
211:     }
212:     m_end = m_buffer + size;
213:     m_buffer[size] = NullCharConst;
214: }
215: 
216: /// <summary>
217: /// Const character cast operator
218: ///
219: /// Returns the pointer to the start of the string.
220: /// </summary>
221: string::operator const ValueType* () const
222: {
223:     return data();
224: }
225: 
226: /// <summary>
227: /// Assignment operator
228: ///
229: /// Assigns the specified string value to the string.
230: /// </summary>
231: /// <param name="str">string value to assign to the string</param>
232: /// <returns>A reference to the string</returns>
233: string& string::operator = (const ValueType* str)
234: {
235:     return assign(str);
236: }
237: 
238: /// <summary>
239: /// Assignment operator
240: ///
241: /// Assigns the specified string value to the string.
242: /// </summary>
243: /// <param name="str">string value to assign to the string</param>
244: /// <returns>A reference to the string</returns>
245: string& string::operator = (const string& str)
246: {
247:     return assign(str);
248: }
249: 
250: /// <summary>
251: /// Move operator
252: ///
253: /// Assigns the specified string value to the string by moving the contents of the specified string.
254: /// </summary>
255: /// <param name="str">string value to assign to the string</param>
256: /// <returns>A reference to the string</returns>
257: string& string::operator = (string&& str)
258: {
259:     if (&str != this)
260:     {
261:         m_buffer = str.m_buffer;
262:         m_end = str.m_end;
263:         m_allocatedSize = str.m_allocatedSize;
264:         str.m_buffer = nullptr;
265:         str.m_end = nullptr;
266:         str.m_allocatedSize = 0;
267:     }
268:     return *this;
269: }
270: 
271: /// <summary>
272: /// Non-const iterator to the start of the string
273: ///
274: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
275: /// </summary>
276: /// <returns>iterator to the value type, acting as the start of the string</returns>
277: iterator<string::ValueType> string::begin()
278: {
279:     return iterator(m_buffer, m_end);
280: }
281: 
282: /// <summary>
283: /// Non-const iterator to the end of the string + 1
284: ///
285: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
286: /// </summary>
287: /// <returns>iterator to the value type, acting as the end of the string</returns>
288: iterator<string::ValueType> string::end()
289: {
290:     return iterator(m_end, m_end);
291: }
292: 
293: /// <summary>
294: /// Const iterator to the start of the string
295: ///
296: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
297: /// </summary>
298: /// <returns>const_iterator to the value type, acting as the start of the string</returns>
299: const_iterator<string::ValueType> string::begin() const
300: {
301:     return const_iterator(m_buffer, m_end);
302: }
303: 
304: /// <summary>
305: /// Const iterator to the end of the string + 1
306: ///
307: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
308: /// </summary>
309: /// <returns>const_iterator to the value type, acting as the end of the string</returns>
310: const_iterator<string::ValueType> string::end() const
311: {
312:     return const_iterator(m_end, m_end);
313: }
314: 
315: /// <summary>
316: /// assign a string value
317: ///
318: /// Assigns the specified string value to the string
319: /// </summary>
320: /// <param name="str">string value to assign to the string</param>
321: /// <returns>A reference to the string</returns>
322: string& string::assign(const ValueType* str)
323: {
324:     size_t size{};
325:     if (str != nullptr)
326:     {
327:         size = strlen(str);
328:     }
329:     if ((size + 1) > m_allocatedSize)
330:     {
331:         if (!reallocate(size + 1))
332:             return *this;
333:     }
334:     if (str != nullptr)
335:     {
336:         strncpy(m_buffer, str, size);
337:     }
338:     m_end = m_buffer + size;
339:     m_buffer[size] = NullCharConst;
340:     return *this;
341: }
342: 
343: /// <summary>
344: /// assign a string value
345: ///
346: /// Assigns the specified string value, up to the specified count of characters, to the string.
347: /// </summary>
348: /// <param name="str">string value to assign to the string</param>
349: /// <param name="count">Maximum number of characters to copy from the string. If count is larger than the string length, the length of the string is used</param>
350: /// <returns>A reference to the string</returns>
351: string& string::assign(const ValueType* str, size_t count)
352: {
353:     size_t size{};
354:     if (str != nullptr)
355:     {
356:         size = strlen(str);
357:     }
358:     if (count < size)
359:         size = count;
360:     if ((size + 1) > m_allocatedSize)
361:     {
362:         if (!reallocate(size + 1))
363:             return *this;
364:     }
365:     if (str != nullptr)
366:     {
367:         strncpy(m_buffer, str, size);
368:     }
369:     m_end = m_buffer + size;
370:     m_buffer[size] = NullCharConst;
371:     return *this;
372: }
373: 
374: /// <summary>
375: /// assign a string value
376: ///
377: /// Assigns a string containing the specified count times the specified characters to the string
378: /// </summary>
379: /// <param name="count">Number copies of ch to copy to the string</param>
380: /// <param name="ch">Character to initialize with</param>
381: /// <returns>A reference to the string</returns>
382: string& string::assign(size_t count, ValueType ch)
383: {
384:     auto size = count;
385:     if ((size + 1) > m_allocatedSize)
386:     {
387:         if (!reallocate(size + 1))
388:             return *this;
389:     }
390:     memset(m_buffer, ch, size);
391:     m_end = m_buffer + size;
392:     m_buffer[size] = NullCharConst;
393:     return *this;
394: }
395: 
396: /// <summary>
397: /// assign a string value
398: ///
399: /// Assigns the specified string value to the string
400: /// </summary>
401: /// <param name="str">string value to assign to the string</param>
402: /// <returns>A reference to the string</returns>
403: string& string::assign(const string& str)
404: {
405:     auto size = str.length();
406:     if ((size + 1) > m_allocatedSize)
407:     {
408:         if (!reallocate(size + 1))
409:             return *this;
410:     }
411:     strncpy(m_buffer, str.data(), size);
412:     m_end = m_buffer + size;
413:     m_buffer[size] = NullCharConst;
414:     return *this;
415: }
416: 
417: /// <summary>
418: /// assign a string value
419: ///
420: /// Assigns the substring start from the specified position for the specified count of characters of specified string value to the string
421: /// </summary>
422: /// <param name="str">string value to assign to the string</param>
423: /// <param name="pos">Starting position of substring to copy from str</param>
424: /// <param name="count">Maximum number of characters to copy from str.
425: /// Default is until end of string. If pos + count is larger than the stirn length, characters are copied until end of string</param>
426: /// <returns>A reference to the string</returns>
427: string& string::assign(const string& str, size_t pos, size_t count /*= npos*/)
428: {
429:     auto size = str.length() - pos;
430:     if (count < size)
431:         size = count;
432:     if ((size + 1) > m_allocatedSize)
433:     {
434:         if (!reallocate(size + 1))
435:             return *this;
436:     }
437:     strncpy(m_buffer, str.data() + pos, size);
438:     m_end = m_buffer + size;
439:     m_buffer[size] = NullCharConst;
440:     return *this;
441: }
442: 
443: /// <summary>
444: /// Return the character at specified position
445: /// </summary>
446: /// <param name="pos">Position in string</param>
447: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
448: string::ValueType& string::at(size_t pos)
449: {
450:     if (pos >= length())
451:     {
452:         NullChar = NullCharConst;
453:         return NullChar;
454:     }
455:     return m_buffer[pos];
456: }
457: 
458: /// <summary>
459: /// Return the character at specified position
460: /// </summary>
461: /// <param name="pos">Position in string</param>
462: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, a reference to a const null character is returned (NullCharConst)</returns>
463: const string::ValueType& string::at(size_t pos) const
464: {
465:     if (pos >= length())
466:         return NullCharConst;
467:     return m_buffer[pos];
468: }
469: 
470: /// <summary>
471: /// Return the first character
472: /// </summary>
473: /// <returns>Returns a non-const reference to the first character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
474: string::ValueType& string::front()
475: {
476:     if (empty())
477:     {
478:         NullChar = NullCharConst;
479:         return NullChar;
480:     }
481:     return *m_buffer;
482: }
483: 
484: /// <summary>
485: /// Return the first character
486: /// </summary>
487: /// <returns>Returns a const reference to the first character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
488: const string::ValueType& string::front() const
489: {
490:     if (empty())
491:         return NullCharConst;
492:     return *m_buffer;
493: }
494: 
495: /// <summary>
496: /// Return the last character
497: /// </summary>
498: /// <returns>Returns a non-const reference to the last character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
499: string::ValueType& string::back()
500: {
501:     if (empty())
502:     {
503:         NullChar = NullCharConst;
504:         return NullChar;
505:     }
506:     return *(m_end - 1);
507: }
508: 
509: /// <summary>
510: /// Return the last character
511: /// </summary>
512: /// <returns>Returns a const reference to the last character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
513: const string::ValueType& string::back() const
514: {
515:     if (empty())
516:         return NullCharConst;
517:     return *(m_end - 1);
518: }
519: 
520: /// <summary>
521: /// Return the character at specified position
522: /// </summary>
523: /// <param name="pos">Position in string</param>
524: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
525: string::ValueType& string::operator[] (size_t pos)
526: {
527:     return m_buffer[pos];
528: }
529: 
530: /// <summary>
531: /// Return the character at specified position
532: /// </summary>
533: /// <param name="pos">Position in string</param>
534: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
535: const string::ValueType& string::operator[] (size_t pos) const
536: {
537:     return m_buffer[pos];
538: }
539: 
540: /// <summary>
541: /// Return the buffer pointer
542: /// </summary>
543: /// <returns>Returns a non-const pointer to the buffer. If the buffer is not allocated, a pointer to a non-const null character (NullChar, initialized before returning) is returned</returns>
544: string::ValueType* string::data()
545: {
546:     NullChar = NullCharConst;
547:     return (m_buffer == nullptr) ? &NullChar : m_buffer;
548: }
549: 
550: /// <summary>
551: /// Return the buffer pointer
552: /// </summary>
553: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
554: const string::ValueType* string::data() const
555: {
556:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
557: }
558: 
559: /// <summary>
560: /// Return the buffer pointer
561: /// </summary>
562: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
563: const string::ValueType* string::c_str() const
564: {
565:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
566: }
567: 
568: /// <summary>
569: /// Determine whether string is empty.
570: /// </summary>
571: /// <returns>Returns true when the string is empty (not allocated or no contents), false otherwise</returns>
572: bool string::empty() const
573: {
574:     return m_end == m_buffer;
575: }
576: 
577: /// <summary>
578: /// Return the size of the string
579: ///
580: /// This method is the equivalent of length().
581: /// </summary>
582: /// <returns>Returns the size (or length) of the string</returns>
583: size_t string::size() const
584: {
585:     return m_end - m_buffer;
586: }
587: 
588: /// <summary>
589: /// Return the length of the string
590: ///
591: /// This method is the equivalent of size().
592: /// </summary>
593: /// <returns>Returns the size (or length) of the string</returns>
594: size_t string::length() const
595: {
596:     return m_end - m_buffer;
597: }
598: 
599: /// <summary>
600: /// Return the capacity of the string
601: ///
602: /// The capacity is the size of the allocated buffer. The string can grow to that length before it needs to be re-allocated.
603: /// </summary>
604: /// <returns>Returns the size (or length) of the string</returns>
605: size_t string::capacity() const
606: {
607:     return m_allocatedSize;
608: }
609: 
610: /// <summary>
611: /// Reserved a buffer capacity
612: ///
613: /// Allocates a buffer of specified size
614: /// </summary>
615: /// <param name="newCapacity"></param>
616: /// <returns>Returns the capacity of the string</returns>
617: size_t string::reserve(size_t newCapacity)
618: {
619:     reallocate_allocation_size(newCapacity);
620:     return m_allocatedSize;
621: }
622: 
623: /// <summary>
624: /// append operator
625: ///
626: /// Appends a character to the string
627: /// </summary>
628: /// <param name="ch">Character to append</param>
629: /// <returns>Returns a reference to the string</returns>
630: string& string::operator +=(ValueType ch)
631: {
632:     append(1, ch);
633:     return *this;
634: }
635: 
636: /// <summary>
637: /// append operator
638: ///
639: /// Appends a string to the string
640: /// </summary>
641: /// <param name="str">string to append</param>
642: /// <returns>Returns a reference to the string</returns>
643: string& string::operator +=(const string& str)
644: {
645:     append(str);
646:     return *this;
647: }
648: 
649: /// <summary>
650: /// append operator
651: ///
652: /// Appends a string to the string
653: /// </summary>
654: /// <param name="str">string to append. If nullptr the nothing is appended</param>
655: /// <returns>Returns a reference to the string</returns>
656: string& string::operator +=(const ValueType* str)
657: {
658:     append(str);
659:     return *this;
660: }
661: 
662: /// <summary>
663: /// append operator
664: ///
665: /// Appends a sequence of count times the same character ch to the string
666: /// </summary>
667: /// <param name="count">Number of characters to append</param>
668: /// <param name="ch">Character to append</param>
669: void string::append(size_t count, ValueType ch)
670: {
671:     auto len = length();
672:     auto size = len + count;
673:     if ((size + 1) > m_allocatedSize)
674:     {
675:         if (!reallocate(size + 1))
676:             return;
677:     }
678:     memset(m_buffer + len, ch, count);
679:     m_end = m_buffer + size;
680:     m_buffer[size] = NullCharConst;
681: }
682: 
683: /// <summary>
684: /// append operator
685: ///
686: /// Appends a string to the string
687: /// </summary>
688: /// <param name="str">string to append</param>
689: void string::append(const string& str)
690: {
691:     auto len = length();
692:     auto strLength = str.length();
693:     auto size = len + strLength;
694:     if ((size + 1) > m_allocatedSize)
695:     {
696:         if (!reallocate(size + 1))
697:             return;
698:     }
699:     strncpy(m_buffer + len, str.data(), strLength);
700:     m_end = m_buffer + size;
701:     m_buffer[size] = NullCharConst;
702: }
703: 
704: /// <summary>
705: /// append operator
706: ///
707: /// Appends a substring of str to the string
708: /// </summary>
709: /// <param name="str">string to append from</param>
710: /// <param name="pos">Start position in str to copy characters from</param>
711: /// <param name="count">Number of characters to copy from str. Default is until the end of the string. If count is larger than the string length, characters are copied up to the end of the string</param>
712: void string::append(const string& str, size_t pos, size_t count /*= npos*/)
713: {
714:     auto len = length();
715:     auto strLength = str.length();
716:     auto strCount = count;
717:     if (pos >= strLength)
718:         return;
719:     if (pos + strCount > strLength)
720:         strCount = strLength - pos;
721:     auto size = len + strCount;
722:     if ((size + 1) > m_allocatedSize)
723:     {
724:         if (!reallocate(size + 1))
725:             return;
726:     }
727:     strncpy(m_buffer + len, str.data() + pos, strCount);
728:     m_end = m_buffer + size;
729:     m_buffer[size] = NullCharConst;
730: }
731: 
732: /// <summary>
733: /// append operator
734: ///
735: /// Appends a string to the string
736: /// </summary>
737: /// <param name="str">string to append. If nullptr the nothing is appended</param>
738: void string::append(const ValueType* str)
739: {
740:     if (str == nullptr)
741:         return;
742:     auto len = length();
743:     auto strLength = strlen(str);
744:     auto size = len + strLength;
745:     if ((size + 1) > m_allocatedSize)
746:     {
747:         if (!reallocate(size + 1))
748:             return;
749:     }
750:     strncpy(m_buffer + len, str, strLength);
751:     m_end = m_buffer + size;
752:     m_buffer[size] = NullCharConst;
753: }
754: 
755: /// <summary>
756: /// append operator
757: ///
758: /// Appends a number of characters from str to the string
759: /// </summary>
760: /// <param name="str">string to append. If nullptr the nothing is appended</param>
761: /// <param name="count">Number of characters to copy from str. If count is larger than the string length, the complete string is copied</param>
762: void string::append(const ValueType* str, size_t count)
763: {
764:     if (str == nullptr)
765:         return;
766:     auto len = length();
767:     auto strLength = strlen(str);
768:     auto strCount = count;
769:     if (strCount > strLength)
770:         strCount = strLength;
771:     auto size = len + strCount;
772:     if ((size + 1) > m_allocatedSize)
773:     {
774:         if (!reallocate(size + 1))
775:             return;
776:     }
777:     strncpy(m_buffer + len, str, strCount);
778:     m_end = m_buffer + size;
779:     m_buffer[size] = NullCharConst;
780: }
781: 
782: /// <summary>
783: /// clear the string
784: ///
785: /// Clears the contents of the string, but does not free or reallocate the buffer
786: /// </summary>
787: void string::clear()
788: {
789:     if (!empty())
790:     {
791:         m_end = m_buffer;
792:         m_buffer[0] = NullCharConst;
793:     }
794: }
795: 
796: /// <summary>
797: /// find a substring in the string
798: ///
799: /// If empty string, always finds the string.
800: /// </summary>
801: /// <param name="str">Substring to find</param>
802: /// <param name="pos">Starting position in string to start searching</param>
803: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
804: size_t string::find(const string& str, size_t pos /*= 0*/) const
805: {
806:     auto len = length();
807:     auto patternLength = str.length();
808:     if (pos >= len)
809:         return npos;
810:     auto needle = str.data();
811:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
812:     {
813:         if (memcmp(haystack, needle, patternLength) == 0)
814:             return haystack - m_buffer;
815:     }
816:     return npos;
817: }
818: 
819: /// <summary>
820: /// find a substring in the string
821: ///
822: /// If nullptr or empty string, always finds the string.
823: /// </summary>
824: /// <param name="str">Substring to find</param>
825: /// <param name="pos">Starting position in string to start searching</param>
826: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
827: size_t string::find(const ValueType* str, size_t pos /*= 0*/) const
828: {
829:     size_t strLength{};
830:     if (str != nullptr)
831:     {
832:         strLength = strlen(str);
833:     }
834:     auto len = length();
835:     auto patternLength = strLength;
836:     if (pos >= len)
837:         return npos;
838:     auto needle = str;
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
854: /// <param name="count">Number of characters from str to compare</param>
855: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
856: size_t string::find(const ValueType* str, size_t pos, size_t count) const
857: {
858:     size_t strLength{};
859:     if (str != nullptr)
860:     {
861:         strLength = strlen(str);
862:     }
863:     auto len = length();
864:     auto patternLength = strLength;
865:     if (pos >= len)
866:         return npos;
867:     if (count < patternLength)
868:         patternLength = count;
869:     auto needle = str;
870:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
871:     {
872:         if (memcmp(haystack, needle, patternLength) == 0)
873:             return haystack - m_buffer;
874:     }
875:     return npos;
876: }
877: 
878: /// <summary>
879: /// find a character in the string
880: /// </summary>
881: /// <param name="ch">Character to find</param>
882: /// <param name="pos">Starting position in string to start searching</param>
883: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
884: size_t string::find(ValueType ch, size_t pos /*= 0*/) const
885: {
886:     auto len = length();
887:     if (pos >= len)
888:         return npos;
889:     for (const ValueType* haystack = data() + pos; haystack <= m_end; ++haystack)
890:     {
891:         if (*haystack == ch)
892:             return haystack - m_buffer;
893:     }
894:     return npos;
895: }
896: 
897: /// <summary>
898: /// Check whether string starts with character
899: /// </summary>
900: /// <param name="ch">Character to find</param>
901: /// <returns>Returns true if ch is first character in string, false otherwise</returns>
902: bool string::starts_with(ValueType ch) const
903: {
904:     if (empty())
905:         return false;
906:     return m_buffer[0] == ch;
907: }
908: 
909: /// <summary>
910: /// Check whether string starts with substring
911: /// </summary>
912: /// <param name="str">SubString to find</param>
913: /// <returns>Returns true if str is first part of string, false otherwise</returns>
914: bool string::starts_with(const string& str) const
915: {
916:     auto len = length();
917:     auto strLength = str.length();
918:     if (strLength >= len)
919:         return false;
920: 
921:     return memcmp(data(), str.data(), strLength) == 0;
922: }
923: 
924: /// <summary>
925: /// Check whether string starts with substring
926: /// </summary>
927: /// <param name="str">SubString to find</param>
928: /// <returns>Returns true if str is first part of string, false otherwise</returns>
929: bool string::starts_with(const ValueType* str) const
930: {
931:     size_t strLength{};
932:     if (str != nullptr)
933:     {
934:         strLength = strlen(str);
935:     }
936:     auto len = length();
937:     if (strLength >= len)
938:         return false;
939: 
940:     return memcmp(data(), str, strLength) == 0;
941: }
942: 
943: /// <summary>
944: /// Check whether string ends with character
945: /// </summary>
946: /// <param name="ch">Character to find</param>
947: /// <returns>Returns true if ch is last character in string, false otherwise</returns>
948: bool string::ends_with(ValueType ch) const
949: {
950:     if (empty())
951:         return false;
952:     return m_buffer[length() - 1] == ch;
953: }
954: 
955: /// <summary>
956: /// Check whether string ends with substring
957: /// </summary>
958: /// <param name="str">SubString to find</param>
959: /// <returns>Returns true if str is last part of string, false otherwise</returns>
960: bool string::ends_with(const string& str) const
961: {
962:     auto len = length();
963:     auto strLength = str.length();
964:     if (strLength >= len)
965:         return false;
966: 
967:     return memcmp(m_end - strLength, str.data(), strLength) == 0;
968: }
969: 
970: /// <summary>
971: /// Check whether string ends with substring
972: /// </summary>
973: /// <param name="str">SubString to find</param>
974: /// <returns>Returns true if str is last part of string, false otherwise</returns>
975: bool string::ends_with(const ValueType* str) const
976: {
977:     size_t strLength{};
978:     if (str != nullptr)
979:     {
980:         strLength = strlen(str);
981:     }
982:     auto len = length();
983:     if (strLength >= len)
984:         return false;
985: 
986:     return memcmp(m_end - strLength, str, strLength) == 0;
987: }
988: 
989: /// <summary>
990: /// Check whether string contains character
991: /// </summary>
992: /// <param name="ch">Character to find</param>
993: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
994: bool string::contains(ValueType ch) const
995: {
996:     return find(ch) != npos;
997: }
998: 
999: /// <summary>
1000: /// Check whether string contains substring
1001: /// </summary>
1002: /// <param name="str">Substring to find</param>
1003: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1004: bool string::contains(const string& str) const
1005: {
1006:     return find(str) != npos;
1007: }
1008: 
1009: /// <summary>
1010: /// Check whether string contains substring
1011: /// </summary>
1012: /// <param name="str">Substring to find</param>
1013: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
1014: bool string::contains(const ValueType* str) const
1015: {
1016:     return find(str) != npos;
1017: }
1018: 
1019: /// <summary>
1020: /// Return substring
1021: /// </summary>
1022: /// <param name="pos">Starting position of substring in string</param>
1023: /// <param name="count">length of substring to return. If count is larger than the number of characters available from position pos, the rest of the string is returned</param>
1024: /// <returns>Returns the substring at position [pos, pos + count), if available </returns>
1025: string string::substr(size_t pos /*= 0*/, size_t count /*= npos*/) const
1026: {
1027:     string result;
1028:     auto len = length();
1029:     auto size = count;
1030:     if (pos < len)
1031:     {
1032:         if (count > len - pos)
1033:             count = len - pos;
1034:         result.reallocate(count + 1);
1035:         memcpy(result.data(), data() + pos, count);
1036:         result.data()[count] = NullCharConst;
1037:     }
1038: 
1039:     return result;
1040: }
1041: 
1042: /// <summary>
1043: /// Case sensitive equality to string
1044: /// </summary>
1045: /// <param name="other">string to compare to</param>
1046: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1047: bool string::equals(const string& other) const
1048: {
1049:     return compare(other) == 0;
1050: }
1051: 
1052: /// <summary>
1053: /// Case sensitive equality to string
1054: /// </summary>
1055: /// <param name="other">string to compare to</param>
1056: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1057: bool string::equals(const ValueType* other) const
1058: {
1059:     if (other == nullptr)
1060:         return empty();
1061:     if (length() != strlen(other))
1062:         return false;
1063:     return strcmp(data(), other) == 0;
1064: }
1065: 
1066: /// <summary>
1067: /// Case insensitive equality to string
1068: /// </summary>
1069: /// <param name="other">string to compare to</param>
1070: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1071: bool string::equals_case_insensitive(const string& other) const
1072: {
1073:     if (length() != other.length())
1074:         return false;
1075:     if (empty())
1076:         return true;
1077:     return strcasecmp(data(), other.data()) == 0;
1078: }
1079: 
1080: /// <summary>
1081: /// Case insensitive equality to string
1082: /// </summary>
1083: /// <param name="other">string to compare to</param>
1084: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1085: bool string::equals_case_insensitive(const ValueType* other) const
1086: {
1087:     if (other == nullptr)
1088:         return empty();
1089:     if (length() != strlen(other))
1090:         return false;
1091:     return strcasecmp(data(), other) == 0;
1092: }
1093: 
1094: /// <summary>
1095: /// Case sensitive compare to string
1096: ///
1097: /// Compares the complete string, character by character
1098: /// </summary>
1099: /// <param name="str">string to compare to</param>
1100: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1101: int string::compare(const string& str) const
1102: {
1103:     if (empty())
1104:     {
1105:         if (str.empty())
1106:             return 0;
1107:         return -1;
1108:     }
1109:     if (str.empty())
1110:         return 1;
1111: 
1112:     return strcmp(data(), str.data());
1113: }
1114: 
1115: /// <summary>
1116: /// Case sensitive compare to string
1117: ///
1118: /// Compares the substring from pos to pos+count to str
1119: /// </summary>
1120: /// <param name="pos">Starting position of substring to compare to str</param>
1121: /// <param name="count">Number of characters in substring to compare to str</param>
1122: /// <param name="str">string to compare to</param>
1123: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1124: int string::compare(size_t pos, size_t count, const string& str) const
1125: {
1126:     return substr(pos, count).compare(str);
1127: }
1128: 
1129: /// <summary>
1130: /// Case sensitive compare to string
1131: ///
1132: /// Compares the substring from pos to pos+count to the substring from strPos to strPos+strCount of str
1133: /// </summary>
1134: /// <param name="pos">Starting position of substring to compare to str</param>
1135: /// <param name="count">Number of characters in substring to compare to str</param>
1136: /// <param name="str">string to compare to</param>
1137: /// <param name="strPos">Starting position of substring of str to compare</param>
1138: /// <param name="strCount">Number of characters in substring of str to compare</param>
1139: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1140: int string::compare(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/) const
1141: {
1142:     return substr(pos, count).compare(str.substr(strPos, strCount));
1143: }
1144: 
1145: /// <summary>
1146: /// Case sensitive compare to string
1147: ///
1148: /// Compares the complete string to str
1149: /// </summary>
1150: /// <param name="str">string to compare to</param>
1151: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1152: int string::compare(const ValueType* str) const
1153: {
1154:     size_t strLength{};
1155:     if (str != nullptr)
1156:         strLength = strlen(str);
1157:     if (empty())
1158:     {
1159:         if (strLength == 0)
1160:             return 0;
1161:         return -1;
1162:     }
1163:     if (strLength == 0)
1164:         return 1;
1165: 
1166:     return strcmp(data(), str);
1167: }
1168: 
1169: /// <summary>
1170: /// Case sensitive compare to string
1171: ///
1172: /// Compares the substring from pos to pos+count to str
1173: /// </summary>
1174: /// <param name="pos">Starting position of substring to compare to str</param>
1175: /// <param name="count">Number of characters in substring to compare to str</param>
1176: /// <param name="str">string to compare to</param>
1177: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1178: int string::compare(size_t pos, size_t count, const ValueType* str) const
1179: {
1180:     size_t strLength{};
1181:     if (str != nullptr)
1182:         strLength = strlen(str);
1183: 
1184:     auto len = length();
1185:     if (pos >= len)
1186:         len = 0;
1187:     len -= pos;
1188:     if (count < len)
1189:         len = count;
1190:     if (len == 0)
1191:     {
1192:         if (strLength == 0)
1193:             return 0;
1194:         return -1;
1195:     }
1196:     if (strLength == 0)
1197:         return 1;
1198: 
1199:     auto maxLen = strLength;
1200:     if (maxLen < len)
1201:         maxLen = len;
1202:     return strncmp(data() + pos, str, maxLen);
1203: }
1204: 
1205: /// <summary>
1206: /// Case sensitive compare to string
1207: ///
1208: /// Compares the substring from pos to pos+count to the first strCount characters of str
1209: /// </summary>
1210: /// <param name="pos">Starting position of substring to compare to str</param>
1211: /// <param name="count">Number of characters in substring to compare to str</param>
1212: /// <param name="str">string to compare to</param>
1213: /// <param name="strCount">Number of characters in substring of str to compare</param>
1214: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1215: int string::compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const
1216: {
1217:     size_t strLength{};
1218:     if (str != nullptr)
1219:         strLength = strlen(str);
1220: 
1221:     auto len = length();
1222:     if (pos >= len)
1223:         len = 0;
1224:     len -= pos;
1225:     if (count < len)
1226:         len = count;
1227: 
1228:     if (strCount < strLength)
1229:         strLength = strCount;
1230: 
1231:     if (len == 0)
1232:     {
1233:         if (strLength == 0)
1234:             return 0;
1235:         return -1;
1236:     }
1237:     if (strLength == 0)
1238:         return 1;
1239: 
1240:     auto maxLen = strLength;
1241:     if (maxLen < len)
1242:         maxLen = len;
1243:     return strncmp(data() + pos, str, maxLen);
1244: }
1245: 
1246: /// <summary>
1247: /// replace substring
1248: ///
1249: /// Replaces the substring from pos to pos+count with str
1250: /// </summary>
1251: /// <param name="pos">Starting position of substring to replace</param>
1252: /// <param name="count">Number of characters in substring to replace</param>
1253: /// <param name="str">string to replace with</param>
1254: /// <returns>Returns the reference to the resulting string</returns>
1255: string& string::replace(size_t pos, size_t count, const string& str)
1256: {
1257:     string result = substr(0, pos) + str + substr(pos + count);
1258:     assign(result);
1259:     return *this;
1260: }
1261: 
1262: /// <summary>
1263: /// replace substring
1264: ///
1265: /// Replaces the substring from pos to pos+count with the substring from strPos to strPos+strCount of str
1266: /// </summary>
1267: /// <param name="pos">Starting position of substring to replace</param>
1268: /// <param name="count">Number of characters in substring to replace</param>
1269: /// <param name="str">string to replace with</param>
1270: /// <param name="strPos">Starting position of substring of str to replace with</param>
1271: /// <param name="strCount">Number of characters in substring of str to replace with</param>
1272: /// <returns>Returns the reference to the resulting string</returns>
1273: string& string::replace(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/)
1274: {
1275:     string result = substr(0, pos) + str.substr(strPos, strCount) + substr(pos + count);
1276:     assign(result);
1277:     return *this;
1278: }
1279: 
1280: /// <summary>
1281: /// replace substring
1282: ///
1283: /// Replaces the substring from pos to pos+count with str
1284: /// </summary>
1285: /// <param name="pos">Starting position of substring to replace</param>
1286: /// <param name="count">Number of characters in substring to replace</param>
1287: /// <param name="str">string to replace with</param>
1288: /// <returns>Returns the reference to the resulting string</returns>
1289: string& string::replace(size_t pos, size_t count, const ValueType* str)
1290: {
1291:     string result = substr(0, pos) + str + substr(pos + count);
1292:     assign(result);
1293:     return *this;
1294: }
1295: 
1296: /// <summary>
1297: /// replace substring
1298: ///
1299: /// Replaces the substring from pos to pos+count with the first strCount characters of str
1300: /// </summary>
1301: /// <param name="pos">Starting position of substring to replace</param>
1302: /// <param name="count">Number of characters in substring to replace</param>
1303: /// <param name="str">string to replace with</param>
1304: /// <param name="strCount">Number of characters in substring to replace with</param>
1305: /// <returns>Returns the reference to the resulting string</returns>
1306: string& string::replace(size_t pos, size_t count, const ValueType* str, size_t strCount)
1307: {
1308:     string result = substr(0, pos) + string(str, strCount) + substr(pos + count);
1309:     assign(result);
1310:     return *this;
1311: }
1312: 
1313: /// <summary>
1314: /// replace substring
1315: ///
1316: /// Replaces the substring from pos to pos+count with ch
1317: /// </summary>
1318: /// <param name="pos">Starting position of substring to replace</param>
1319: /// <param name="count">Number of characters in substring to replace</param>
1320: /// <param name="ch">Characters to replace with</param>
1321: /// <returns>Returns the reference to the resulting string</returns>
1322: string& string::replace(size_t pos, size_t count, ValueType ch)
1323: {
1324:     return replace(pos, count, ch, 1);
1325: }
1326: 
1327: /// <summary>
1328: /// replace substring
1329: ///
1330: /// Replaces the substring from pos to pos+count with a sequence of chCount copies of ch
1331: /// </summary>
1332: /// <param name="pos">Starting position of substring to replace</param>
1333: /// <param name="count">Number of characters in substring to replace</param>
1334: /// <param name="ch">Characters to replace with</param>
1335: /// <param name="chCount">Number of copies of ch to replace with</param>
1336: /// <returns>Returns the reference to the resulting string</returns>
1337: string& string::replace(size_t pos, size_t count, ValueType ch, size_t chCount)
1338: {
1339:     string result = substr(0, pos) + string(chCount, ch) + substr(pos + count);
1340:     assign(result);
1341:     return *this;
1342: }
1343: 
1344: /// <summary>
1345: /// replace substring
1346: ///
1347: /// Replaces all instances of the substring oldStr (if existing) with newStr
1348: /// </summary>
1349: /// <param name="oldStr">string to find in string</param>
1350: /// <param name="newStr">string to replace with</param>
1351: /// <returns>Returns the number of times the string was replaced</returns>
1352: int string::replace(const string& oldStr, const string& newStr)
1353: {
1354:     size_t pos = find(oldStr);
1355:     size_t oldLength = oldStr.length();
1356:     size_t newLength = newStr.length();
1357:     int count = 0;
1358:     while (pos != npos)
1359:     {
1360:         replace(pos, oldLength, newStr);
1361:         pos += newLength;
1362:         pos = find(oldStr, pos);
1363:         count++;
1364:     }
1365:     return count;
1366: }
1367: 
1368: /// <summary>
1369: /// replace substring
1370: ///
1371: /// Replaces all instances of the substring oldStr (if existing) with newStr
1372: /// </summary>
1373: /// <param name="oldStr">string to find in string</param>
1374: /// <param name="newStr">string to replace with</param>
1375: /// <returns>Returns the number of times the string was replaced</returns>
1376: int string::replace(const ValueType* oldStr, const ValueType* newStr)
1377: {
1378:     if ((oldStr == nullptr) || (newStr == nullptr))
1379:         return 0;
1380:     size_t pos = find(oldStr);
1381:     size_t oldLength = strlen(oldStr);
1382:     size_t newLength = strlen(newStr);
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
1395: /// Align string
1396: ///
1397: /// Pads the string on the left (width > 0) or on the right (width < 0) up to a length of width characters. If the string is larger than width characters, it is not modified.
1398: /// </summary>
1399: /// <param name="width">length of target string. If width < 0, the string is padded to the right with spaces up to -width characters.
1400: /// if width > 0, the string is padded to the left with space up to width characters</param>
1401: /// <returns>Returns the number of times the string was replaced</returns>
1402: string string::align(int width) const
1403: {
1404:     string result;
1405:     int    absWidth = (width > 0) ? width : -width;
1406:     auto   len = length();
1407:     if (static_cast<size_t>(absWidth) > len)
1408:     {
1409:         if (width < 0)
1410:         {
1411:             result = *this + string(static_cast<unsigned int>(-width) - len, ' ');
1412:         }
1413:         else
1414:         {
1415:             result = string(static_cast<unsigned int>(width) - len, ' ') + *this;
1416:         }
1417:     }
1418:     else
1419:         result = *this;
1420:     return result;
1421: }
1422: 
1423: /// <summary>
1424: /// Allocate or re-allocate string to have a capacity of requestedLength characters
1425: /// </summary>
1426: /// <param name="requestedLength">Amount of characters in the string to allocate space for</param>
1427: /// <returns>True if successful, false otherwise</returns>
1428: bool string::reallocate(size_t requestedLength)
1429: {
1430:     auto requestedSize = requestedLength;
1431:     auto allocationSize = NextPowerOf2((requestedSize < MinimumAllocationSize) ? MinimumAllocationSize : requestedSize);
1432: 
1433:     if (!reallocate_allocation_size(allocationSize))
1434:         return false;
1435:     return true;
1436: }
1437: 
1438: /// <summary>
1439: /// Allocate or re-allocate string to have a capacity of allocationSize bytes
1440: /// </summary>
1441: /// <param name="allocationSize">Amount of bytes to allocate space for</param>
1442: /// <returns>True if successful, false otherwise</returns>
1443: bool string::reallocate_allocation_size(size_t allocationSize)
1444: {
1445:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1446:     if (newBuffer == nullptr)
1447:     {
1448:         LOG_NO_ALLOC_DEBUG("Alloc failed!");
1449:         return false;
1450:     }
1451:     m_buffer = newBuffer;
1452: #if BAREMETAL_MEMORY_TRACING_DETAIL
1453:     LOG_NO_ALLOC_DEBUG("Alloc string %p", m_buffer);
1454: #endif
1455:     if (m_end == nullptr)
1456:         m_end = m_buffer;
1457:     if (m_end > m_buffer + allocationSize)
1458:         m_end = m_buffer + allocationSize;
1459:     m_allocatedSize = allocationSize;
1460:     return true;
1461: }
```

As you can see, this is a very extensive class.
A few remarks:
- Line 52: We use a minimum allocation size of 64, meaning that every string will have storage space for 63 characters plus a null character for termination, at the least.
- Line 55: We use a special static variable `NullCharConst` when we need to return something constant, but the string is not allocated.
- Line 57: Similarly, we use a special static variable `NullChar` when we need to return something non constant, but the string is not allocated.
This variable is non-const, so will have to be initialized every time we return it, to make sure it is still a null character.
- Line 182-190: We implement a move constructor, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 257-269: We implement a move assignment, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 277-291: We use our own version of a iterator template, which we'll get to, to enable creating the begin() and end() methods. These allow us to use the standard c++ `for (x : string)` construct
- Line 299-313: We also use our own version of a const_iterator template for const iterators, which we'll get to, to enable creating the begin() and end() const methods.
- Line 994-1017: We also have the `contains()` methods, which are new to C++23, however they are quite convenient
- Line 1047-1092: We use the methods `equals()` and `equals_case_insensititive()` to compare strings. Both are not standard, but convenient
- Line 1352-1392: We implement two extra variants of `replace()` (replacing multiple instances of a string) for convenience
- Line 1402-1421: Again for convenience, we implement the `align()` method. We'll be using this later on for aligning strings when formatting

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

