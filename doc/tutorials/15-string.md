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
43: #include <baremetal/Util.h>
44: 
45: /// @file
46: /// string class implementation
47: 
48: using namespace baremetal;
49: 
50: /// @brief Minimum allocation size for any string
51: static constexpr size_t MinimumAllocationSize = 64;
52: const size_t string::npos = static_cast<size_t>(-1);
53: /// @brief Constant null character, using as string terminator, and also returned as a reference for const methods where nothing can be returned
54: static const string::ValueType NullCharConst = '\0';
55: /// @brief Non-constant null character, returned as a reference for const methods where nothing can be returned (always reinitialized before returning)
56: static string::ValueType NullChar = '\0';
57: 
58: /// <summary>
59: /// Default constructor
60: ///
61: /// Constructs an empty string.
62: /// </summary>
63: string::string()
64:     : m_buffer{}
65:     , m_end{}
66:     , m_allocatedSize{}
67: {
68: }
69: 
70: /// <summary>
71: /// Destructor
72: ///
73: /// Frees any allocated memory.
74: /// </summary>
75: string::~string()
76: {
77:     delete[] m_buffer;
78: }
79: 
80: /// <summary>
81: /// Constructor
82: ///
83: /// Initializes the string with the specified string.
84: /// </summary>
85: /// <param name="str">string to initialize with</param>
86: string::string(const ValueType* str)
87:     : m_buffer{}
88:     , m_end{}
89:     , m_allocatedSize{}
90: {
91:     if (str == nullptr)
92:         return;
93:     auto size = strlen(str);
94:     if (reallocate(size))
95:     {
96:         strncpy(m_buffer, str, size);
97:     }
98:     m_end = m_buffer + size;
99: }
100: 
101: /// <summary>
102: /// Constructor
103: ///
104: /// Initializes the string with up to count characters in the specified string. A null character is always added.
105: /// </summary>
106: /// <param name="str">string to initialize with</param>
107: /// <param name="count">Maximum number of characters from str to initialize with. If count is larger than the actual string length, only the string length is used</param>
108: string::string(const ValueType* str, size_t count)
109:     : m_buffer{}
110:     , m_end{}
111:     , m_allocatedSize{}
112: {
113:     if (str == nullptr)
114:         return;
115:     auto size = strlen(str);
116:     if (count < size)
117:         size = count;
118:     if (reallocate(size))
119:     {
120:         strncpy(m_buffer, str, size);
121:     }
122:     m_end = m_buffer + size;
123: }
124: 
125: /// <summary>
126: /// Constructor
127: ///
128: /// Initializes the string with the specified count times the specified character. A null character is always added.
129: /// </summary>
130: /// <param name="count">Number of characters of value ch to initialized with</param>
131: /// <param name="ch">Character to initialize with</param>
132: string::string(size_t count, ValueType ch)
133:     : m_buffer{}
134:     , m_end{}
135:     , m_allocatedSize{}
136: {
137:     auto size = count;
138:     if (reallocate(size))
139:     {
140:         memset(m_buffer, ch, size);
141:     }
142:     m_end = m_buffer + size;
143: }
144: 
145: /// <summary>
146: /// Copy constructor
147: ///
148: /// Initializes the string with the specified string value.
149: /// </summary>
150: /// <param name="other">string to initialize with</param>
151: string::string(const string& other)
152:     : m_buffer{}
153:     , m_end{}
154:     , m_allocatedSize{}
155: {
156:     auto size = other.length();
157:     if (reallocate(size))
158:     {
159:         strncpy(m_buffer, other.data(), size);
160:     }
161:     m_end = m_buffer + size;
162: }
163: 
164: /// <summary>
165: /// Move constructor
166: ///
167: /// Initializes the string by moving the contents from the specified string value.
168: /// </summary>
169: /// <param name="other">string to initialize with</param>
170: string::string(string&& other)
171:     : m_buffer{other.m_buffer}
172:     , m_end{other.m_end}
173:     , m_allocatedSize{other.m_allocatedSize}
174: {
175:     other.m_buffer = nullptr;
176:     other.m_end = nullptr;
177:     other.m_allocatedSize = 0;
178: }
179: 
180: /// <summary>
181: /// Constructor
182: ///
183: /// Initializes the string with the substring starting at specified position, for the specified number of characters, from the specified string value.
184: /// </summary>
185: /// <param name="other">string to initialize with</param>
186: /// <param name="pos">Position in other to start copying charaters from</param>
187: /// <param name="count">Maximum number of characters to copy from other. Default is until end of string. If pos + count is larger than the actual length of the string, string string is copied until the end</param>
188: string::string(const string& other, size_t pos, size_t count /*= npos*/)
189:     : m_buffer{}
190:     , m_end{}
191:     , m_allocatedSize{}
192: {
193:     auto size = other.length() - pos;
194:     if (count < size)
195:         size = count;
196:     if (reallocate(size))
197:     {
198:         strncpy(m_buffer, other.data() + pos, size);
199:     }
200:     m_end = m_buffer + size;
201: }
202: 
203: /// <summary>
204: /// Const character cast operator
205: ///
206: /// Returns the pointer to the start of the string.
207: /// </summary>
208: string::operator const ValueType* () const
209: {
210:     return data();
211: }
212: 
213: /// <summary>
214: /// Assignment operator
215: ///
216: /// Assigns the specified string value to the string.
217: /// </summary>
218: /// <param name="str">string value to assign to the string</param>
219: /// <returns>A reference to the string</returns>
220: string& string::operator = (const ValueType* str)
221: {
222:     return assign(str);
223: }
224: 
225: /// <summary>
226: /// Assignment operator
227: ///
228: /// Assigns the specified string value to the string.
229: /// </summary>
230: /// <param name="str">string value to assign to the string</param>
231: /// <returns>A reference to the string</returns>
232: string& string::operator = (const string& str)
233: {
234:     return assign(str);
235: }
236: 
237: /// <summary>
238: /// Move operator
239: ///
240: /// Assigns the specified string value to the string by moving the contents of the specified string.
241: /// </summary>
242: /// <param name="str">string value to assign to the string</param>
243: /// <returns>A reference to the string</returns>
244: string& string::operator = (string&& str)
245: {
246:     if (&str != this)
247:     {
248:         m_buffer = str.m_buffer;
249:         m_end = str.m_end;
250:         m_allocatedSize = str.m_allocatedSize;
251:         str.m_buffer = nullptr;
252:         str.m_end = nullptr;
253:         str.m_allocatedSize = 0;
254:     }
255:     return *this;
256: }
257: 
258: /// <summary>
259: /// Non-const iterator to the start of the string
260: ///
261: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
262: /// </summary>
263: /// <returns>iterator to the value type, acting as the start of the string</returns>
264: iterator<string::ValueType> string::begin()
265: {
266:     return iterator(m_buffer, m_end);
267: }
268: 
269: /// <summary>
270: /// Non-const iterator to the end of the string + 1
271: ///
272: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
273: /// </summary>
274: /// <returns>iterator to the value type, acting as the end of the string</returns>
275: iterator<string::ValueType> string::end()
276: {
277:     return iterator(m_end, m_end);
278: }
279: 
280: /// <summary>
281: /// Const iterator to the start of the string
282: ///
283: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
284: /// </summary>
285: /// <returns>const_iterator to the value type, acting as the start of the string</returns>
286: const_iterator<string::ValueType> string::begin() const
287: {
288:     return const_iterator(m_buffer, m_end);
289: }
290: 
291: /// <summary>
292: /// Const iterator to the end of the string + 1
293: ///
294: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
295: /// </summary>
296: /// <returns>const_iterator to the value type, acting as the end of the string</returns>
297: const_iterator<string::ValueType> string::end() const
298: {
299:     return const_iterator(m_end, m_end);
300: }
301: 
302: /// <summary>
303: /// assign a string value
304: ///
305: /// Assigns the specified string value to the string
306: /// </summary>
307: /// <param name="str">string value to assign to the string</param>
308: /// <returns>A reference to the string</returns>
309: string& string::assign(const ValueType* str)
310: {
311:     size_t size{};
312:     if (str != nullptr)
313:     {
314:         size = strlen(str);
315:     }
316:     if (size > m_allocatedSize)
317:     {
318:         if (!reallocate(size))
319:             return *this;
320:     }
321:     strncpy(m_buffer, str, size);
322:     m_end = m_buffer + size;
323:     m_buffer[size] = NullCharConst;
324:     return *this;
325: }
326: 
327: /// <summary>
328: /// assign a string value
329: ///
330: /// Assigns the specified string value, up to the specified count of characters, to the string.
331: /// </summary>
332: /// <param name="str">string value to assign to the string</param>
333: /// <param name="count">Maximum number of characters to copy from the string. If count is larger than the string length, the length of the string is used</param>
334: /// <returns>A reference to the string</returns>
335: string& string::assign(const ValueType* str, size_t count)
336: {
337:     size_t size{};
338:     if (str != nullptr)
339:     {
340:         size = strlen(str);
341:     }
342:     if (count < size)
343:         size = count;
344:     if (size > m_allocatedSize)
345:     {
346:         if (!reallocate(size))
347:             return *this;
348:     }
349:     strncpy(m_buffer, str, size);
350:     m_end = m_buffer + size;
351:     m_buffer[size] = NullCharConst;
352:     return *this;
353: }
354: 
355: /// <summary>
356: /// assign a string value
357: ///
358: /// Assigns a string containing the specified count times the specified characters to the string
359: /// </summary>
360: /// <param name="count">Number copies of ch to copy to the string</param>
361: /// <param name="ch">Character to initialize with</param>
362: /// <returns>A reference to the string</returns>
363: string& string::assign(size_t count, ValueType ch)
364: {
365:     auto size = count;
366:     if (size > m_allocatedSize)
367:     {
368:         if (!reallocate(size))
369:             return *this;
370:     }
371:     memset(m_buffer, ch, size);
372:     m_end = m_buffer + size;
373:     m_buffer[size] = NullCharConst;
374:     return *this;
375: }
376: 
377: /// <summary>
378: /// assign a string value
379: ///
380: /// Assigns the specified string value to the string
381: /// </summary>
382: /// <param name="str">string value to assign to the string</param>
383: /// <returns>A reference to the string</returns>
384: string& string::assign(const string& str)
385: {
386:     auto size = str.length();
387:     if (size > m_allocatedSize)
388:     {
389:         if (!reallocate(size))
390:             return *this;
391:     }
392:     strncpy(m_buffer, str.data(), size);
393:     m_end = m_buffer + size;
394:     m_buffer[size] = NullCharConst;
395:     return *this;
396: }
397: 
398: /// <summary>
399: /// assign a string value
400: ///
401: /// Assigns the substring start from the specified position for the specified count of characters of specified string value to the string
402: /// </summary>
403: /// <param name="str">string value to assign to the string</param>
404: /// <param name="pos">Starting position of substring to copy from str</param>
405: /// <param name="count">Maximum number of characters to copy from str.
406: /// Default is until end of string. If pos + count is larger than the stirn length, characters are copied until end of string</param>
407: /// <returns>A reference to the string</returns>
408: string& string::assign(const string& str, size_t pos, size_t count /*= npos*/)
409: {
410:     auto size = str.length() - pos;
411:     if (count < size)
412:         size = count;
413:     if (size > m_allocatedSize)
414:     {
415:         if (!reallocate(size))
416:             return *this;
417:     }
418:     strncpy(m_buffer, str.data() + pos, size);
419:     m_end = m_buffer + size;
420:     m_buffer[size] = NullCharConst;
421:     return *this;
422: }
423: 
424: /// <summary>
425: /// Return the character at specified position
426: /// </summary>
427: /// <param name="pos">Position in string</param>
428: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
429: string::ValueType& string::at(size_t pos)
430: {
431:     if (pos >= length())
432:     {
433:         NullChar = NullCharConst;
434:         return NullChar;
435:     }
436:     return m_buffer[pos];
437: }
438: 
439: /// <summary>
440: /// Return the character at specified position
441: /// </summary>
442: /// <param name="pos">Position in string</param>
443: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, a reference to a const null character is returned (NullCharConst)</returns>
444: const string::ValueType& string::at(size_t pos) const
445: {
446:     if (pos >= length())
447:         return NullCharConst;
448:     return m_buffer[pos];
449: }
450: 
451: /// <summary>
452: /// Return the first character
453: /// </summary>
454: /// <returns>Returns a non-const reference to the first character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
455: string::ValueType& string::front()
456: {
457:     if (empty())
458:     {
459:         NullChar = NullCharConst;
460:         return NullChar;
461:     }
462:     return *m_buffer;
463: }
464: 
465: /// <summary>
466: /// Return the first character
467: /// </summary>
468: /// <returns>Returns a const reference to the first character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
469: const string::ValueType& string::front() const
470: {
471:     if (empty())
472:         return NullCharConst;
473:     return *m_buffer;
474: }
475: 
476: /// <summary>
477: /// Return the last character
478: /// </summary>
479: /// <returns>Returns a non-const reference to the last character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
480: string::ValueType& string::back()
481: {
482:     if (empty())
483:     {
484:         NullChar = NullCharConst;
485:         return NullChar;
486:     }
487:     return *(m_end - 1);
488: }
489: 
490: /// <summary>
491: /// Return the last character
492: /// </summary>
493: /// <returns>Returns a const reference to the last character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
494: const string::ValueType& string::back() const
495: {
496:     if (empty())
497:         return NullCharConst;
498:     return *(m_end - 1);
499: }
500: 
501: /// <summary>
502: /// Return the character at specified position
503: /// </summary>
504: /// <param name="pos">Position in string</param>
505: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
506: string::ValueType& string::operator[] (size_t pos)
507: {
508:     return m_buffer[pos];
509: }
510: 
511: /// <summary>
512: /// Return the character at specified position
513: /// </summary>
514: /// <param name="pos">Position in string</param>
515: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
516: const string::ValueType& string::operator[] (size_t pos) const
517: {
518:     return m_buffer[pos];
519: }
520: 
521: /// <summary>
522: /// Return the buffer pointer
523: /// </summary>
524: /// <returns>Returns a non-const pointer to the buffer. If the buffer is not allocated, a pointer to a non-const null character (NullChar, initialized before returning) is returned</returns>
525: string::ValueType* string::data()
526: {
527:     NullChar = NullCharConst;
528:     return (m_buffer == nullptr) ? &NullChar : m_buffer;
529: }
530: 
531: /// <summary>
532: /// Return the buffer pointer
533: /// </summary>
534: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
535: const string::ValueType* string::data() const
536: {
537:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
538: }
539: 
540: /// <summary>
541: /// Return the buffer pointer
542: /// </summary>
543: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
544: const string::ValueType* string::c_str() const
545: {
546:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
547: }
548: 
549: /// <summary>
550: /// Determine whether string is empty.
551: /// </summary>
552: /// <returns>Returns true when the string is empty (not allocated or no contents), false otherwise</returns>
553: bool string::empty() const
554: {
555:     return m_end == m_buffer;
556: }
557: 
558: /// <summary>
559: /// Return the size of the string
560: ///
561: /// This method is the equivalent of length().
562: /// </summary>
563: /// <returns>Returns the size (or length) of the string</returns>
564: size_t string::size() const
565: {
566:     return m_end - m_buffer;
567: }
568: 
569: /// <summary>
570: /// Return the length of the string
571: ///
572: /// This method is the equivalent of size().
573: /// </summary>
574: /// <returns>Returns the size (or length) of the string</returns>
575: size_t string::length() const
576: {
577:     return m_end - m_buffer;
578: }
579: 
580: /// <summary>
581: /// Return the capacity of the string
582: ///
583: /// The capacity is the size of the allocated buffer. The string can grow to that length before it needs to be re-allocated.
584: /// </summary>
585: /// <returns>Returns the size (or length) of the string</returns>
586: size_t string::capacity() const
587: {
588:     return m_allocatedSize;
589: }
590: 
591: /// <summary>
592: /// Reserved a buffer capacity
593: ///
594: /// Allocates a buffer of specified size
595: /// </summary>
596: /// <param name="newCapacity"></param>
597: /// <returns>Returns the capacity of the string</returns>
598: size_t string::reserve(size_t newCapacity)
599: {
600:     reallocate_allocation_size(newCapacity);
601:     return m_allocatedSize;
602: }
603: 
604: /// <summary>
605: /// append operator
606: ///
607: /// Appends a character to the string
608: /// </summary>
609: /// <param name="ch">Character to append</param>
610: /// <returns>Returns a reference to the string</returns>
611: string& string::operator +=(ValueType ch)
612: {
613:     append(1, ch);
614:     return *this;
615: }
616: 
617: /// <summary>
618: /// append operator
619: ///
620: /// Appends a string to the string
621: /// </summary>
622: /// <param name="str">string to append</param>
623: /// <returns>Returns a reference to the string</returns>
624: string& string::operator +=(const string& str)
625: {
626:     append(str);
627:     return *this;
628: }
629: 
630: /// <summary>
631: /// append operator
632: ///
633: /// Appends a string to the string
634: /// </summary>
635: /// <param name="str">string to append. If nullptr the nothing is appended</param>
636: /// <returns>Returns a reference to the string</returns>
637: string& string::operator +=(const ValueType* str)
638: {
639:     append(str);
640:     return *this;
641: }
642: 
643: /// <summary>
644: /// append operator
645: ///
646: /// Appends a sequence of count times the same character ch to the string
647: /// </summary>
648: /// <param name="count">Number of characters to append</param>
649: /// <param name="ch">Character to append</param>
650: void string::append(size_t count, ValueType ch)
651: {
652:     auto len = length();
653:     auto size = len + count;
654:     if (size > m_allocatedSize)
655:     {
656:         if (!reallocate(size))
657:             return;
658:     }
659:     memset(m_buffer + len, ch, count);
660:     m_end += count;
661:     m_buffer[size] = NullCharConst;
662: }
663: 
664: /// <summary>
665: /// append operator
666: ///
667: /// Appends a string to the string
668: /// </summary>
669: /// <param name="str">string to append</param>
670: void string::append(const string& str)
671: {
672:     auto len = length();
673:     auto strLength = str.length();
674:     auto size = len+ strLength;
675:     if (size > m_allocatedSize)
676:     {
677:         if (!reallocate(size))
678:             return;
679:     }
680:     strncpy(m_buffer + len, str.data(), strLength);
681:     m_end += strLength;
682:     m_buffer[size] = NullCharConst;
683: }
684: 
685: /// <summary>
686: /// append operator
687: ///
688: /// Appends a substring of str to the string
689: /// </summary>
690: /// <param name="str">string to append from</param>
691: /// <param name="pos">Start position in str to copy characters from</param>
692: /// <param name="count">Number of characters to copy from str. Default is until the end of the string. If count is larger than the string length, characters are copied up to the end of the string</param>
693: void string::append(const string& str, size_t pos, size_t count /*= npos*/)
694: {
695:     auto len = length();
696:     auto strLength = str.length();
697:     auto strCount = count;
698:     if (pos >= strLength)
699:         return;
700:     if (pos + strCount > strLength)
701:         strCount = strLength - pos;
702:     auto size = len + strCount;
703:     if (size > m_allocatedSize)
704:     {
705:         if (!reallocate(size))
706:             return;
707:     }
708:     strncpy(m_buffer + len, str.data() + pos, strCount);
709:     m_end += strCount;
710:     m_buffer[size] = NullCharConst;
711: }
712: 
713: /// <summary>
714: /// append operator
715: ///
716: /// Appends a string to the string
717: /// </summary>
718: /// <param name="str">string to append. If nullptr the nothing is appended</param>
719: void string::append(const ValueType* str)
720: {
721:     if (str == nullptr)
722:         return;
723:     auto len = length();
724:     auto strLength = strlen(str);
725:     auto size = len + strLength;
726:     if (size > m_allocatedSize)
727:     {
728:         if (!reallocate(size))
729:             return;
730:     }
731:     strncpy(m_buffer + len, str, strLength);
732:     m_end += strLength;
733:     m_buffer[size] = NullCharConst;
734: }
735: 
736: /// <summary>
737: /// append operator
738: ///
739: /// Appends a number of characters from str to the string
740: /// </summary>
741: /// <param name="str">string to append. If nullptr the nothing is appended</param>
742: /// <param name="count">Number of characters to copy from str. If count is larger than the string length, the complete string is copied</param>
743: void string::append(const ValueType* str, size_t count)
744: {
745:     if (str == nullptr)
746:         return;
747:     auto len = length();
748:     auto strLength = strlen(str);
749:     auto strCount = count;
750:     if (strCount > strLength)
751:         strCount = strLength;
752:     auto size = len + strCount;
753:     if (size > m_allocatedSize)
754:     {
755:         if (!reallocate(size))
756:             return;
757:     }
758:     strncpy(m_buffer + len, str, strCount);
759:     m_end += strCount;
760:     m_buffer[size] = NullCharConst;
761: }
762: 
763: /// <summary>
764: /// clear the string
765: ///
766: /// Clears the contents of the string, but does not free or reallocate the buffer
767: /// </summary>
768: void string::clear()
769: {
770:     if (!empty())
771:     {
772:         m_end = m_buffer;
773:         m_buffer[0] = NullCharConst;
774:     }
775: }
776: 
777: /// <summary>
778: /// find a substring in the string
779: ///
780: /// If empty string, always finds the string.
781: /// </summary>
782: /// <param name="str">Substring to find</param>
783: /// <param name="pos">Starting position in string to start searching</param>
784: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
785: size_t string::find(const string& str, size_t pos /*= 0*/) const
786: {
787:     auto len = length();
788:     auto patternLength = str.length();
789:     if (pos >= len)
790:         return npos;
791:     auto needle = str.data();
792:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
793:     {
794:         if (memcmp(haystack, needle, patternLength) == 0)
795:             return haystack - m_buffer;
796:     }
797:     return npos;
798: }
799: 
800: /// <summary>
801: /// find a substring in the string
802: ///
803: /// If nullptr or empty string, always finds the string.
804: /// </summary>
805: /// <param name="str">Substring to find</param>
806: /// <param name="pos">Starting position in string to start searching</param>
807: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
808: size_t string::find(const ValueType* str, size_t pos /*= 0*/) const
809: {
810:     size_t strLength{};
811:     if (str != nullptr)
812:     {
813:         strLength = strlen(str);
814:     }
815:     auto len = length();
816:     auto patternLength = strLength;
817:     if (pos >= len)
818:         return npos;
819:     auto needle = str;
820:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
821:     {
822:         if (memcmp(haystack, needle, patternLength) == 0)
823:             return haystack - m_buffer;
824:     }
825:     return npos;
826: }
827: 
828: /// <summary>
829: /// find a substring in the string
830: ///
831: /// If nullptr or empty string, always finds the string.
832: /// </summary>
833: /// <param name="str">Substring to find</param>
834: /// <param name="pos">Starting position in string to start searching</param>
835: /// <param name="count">Number of characters from str to compare</param>
836: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
837: size_t string::find(const ValueType* str, size_t pos, size_t count) const
838: {
839:     size_t strLength{};
840:     if (str != nullptr)
841:     {
842:         strLength = strlen(str);
843:     }
844:     auto len = length();
845:     auto patternLength = strLength;
846:     if (pos >= len)
847:         return npos;
848:     if (count < patternLength)
849:         patternLength = count;
850:     auto needle = str;
851:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
852:     {
853:         if (memcmp(haystack, needle, patternLength) == 0)
854:             return haystack - m_buffer;
855:     }
856:     return npos;
857: }
858: 
859: /// <summary>
860: /// find a character in the string
861: /// </summary>
862: /// <param name="ch">Character to find</param>
863: /// <param name="pos">Starting position in string to start searching</param>
864: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
865: size_t string::find(ValueType ch, size_t pos /*= 0*/) const
866: {
867:     auto len = length();
868:     if (pos >= len)
869:         return npos;
870:     for (const ValueType* haystack = data() + pos; haystack <= m_end; ++haystack)
871:     {
872:         if (*haystack == ch)
873:             return haystack - m_buffer;
874:     }
875:     return npos;
876: }
877: 
878: /// <summary>
879: /// Check whether string starts with character
880: /// </summary>
881: /// <param name="ch">Character to find</param>
882: /// <returns>Returns true if ch is first character in string, false otherwise</returns>
883: bool string::starts_with(ValueType ch) const
884: {
885:     if (empty())
886:         return false;
887:     return m_buffer[0] == ch;
888: }
889: 
890: /// <summary>
891: /// Check whether string starts with substring
892: /// </summary>
893: /// <param name="str">SubString to find</param>
894: /// <returns>Returns true if str is first part of string, false otherwise</returns>
895: bool string::starts_with(const string& str) const
896: {
897:     auto len = length();
898:     auto strLength = str.length();
899:     if (strLength >= len)
900:         return false;
901: 
902:     return memcmp(data(), str.data(), strLength) == 0;
903: }
904: 
905: /// <summary>
906: /// Check whether string starts with substring
907: /// </summary>
908: /// <param name="str">SubString to find</param>
909: /// <returns>Returns true if str is first part of string, false otherwise</returns>
910: bool string::starts_with(const ValueType* str) const
911: {
912:     size_t strLength{};
913:     if (str != nullptr)
914:     {
915:         strLength = strlen(str);
916:     }
917:     auto len = length();
918:     if (strLength >= len)
919:         return false;
920: 
921:     return memcmp(data(), str, strLength) == 0;
922: }
923: 
924: /// <summary>
925: /// Check whether string ends with character
926: /// </summary>
927: /// <param name="ch">Character to find</param>
928: /// <returns>Returns true if ch is last character in string, false otherwise</returns>
929: bool string::ends_with(ValueType ch) const
930: {
931:     if (empty())
932:         return false;
933:     return m_buffer[length() - 1] == ch;
934: }
935: 
936: /// <summary>
937: /// Check whether string ends with substring
938: /// </summary>
939: /// <param name="str">SubString to find</param>
940: /// <returns>Returns true if str is last part of string, false otherwise</returns>
941: bool string::ends_with(const string& str) const
942: {
943:     auto len = length();
944:     auto strLength = str.length();
945:     if (strLength >= len)
946:         return false;
947: 
948:     return memcmp(m_end - strLength, str.data(), strLength) == 0;
949: }
950: 
951: /// <summary>
952: /// Check whether string ends with substring
953: /// </summary>
954: /// <param name="str">SubString to find</param>
955: /// <returns>Returns true if str is last part of string, false otherwise</returns>
956: bool string::ends_with(const ValueType* str) const
957: {
958:     size_t strLength{};
959:     if (str != nullptr)
960:     {
961:         strLength = strlen(str);
962:     }
963:     auto len = length();
964:     if (strLength >= len)
965:         return false;
966: 
967:     return memcmp(m_end - strLength, str, strLength) == 0;
968: }
969: 
970: /// <summary>
971: /// Check whether string contains character
972: /// </summary>
973: /// <param name="ch">Character to find</param>
974: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
975: bool string::contains(ValueType ch) const
976: {
977:     return find(ch) != npos;
978: }
979: 
980: /// <summary>
981: /// Check whether string contains substring
982: /// </summary>
983: /// <param name="str">Substring to find</param>
984: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
985: bool string::contains(const string& str) const
986: {
987:     return find(str) != npos;
988: }
989: 
990: /// <summary>
991: /// Check whether string contains substring
992: /// </summary>
993: /// <param name="str">Substring to find</param>
994: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
995: bool string::contains(const ValueType* str) const
996: {
997:     return find(str) != npos;
998: }
999: 
1000: /// <summary>
1001: /// Return substring
1002: /// </summary>
1003: /// <param name="pos">Starting position of substring in string</param>
1004: /// <param name="count">length of substring to return. If count is larger than the number of characters available from position pos, the rest of the string is returned</param>
1005: /// <returns>Returns the substring at position [pos, pos + count), if available </returns>
1006: string string::substr(size_t pos /*= 0*/, size_t count /*= npos*/) const
1007: {
1008:     string result;
1009:     auto len = length();
1010:     auto size = count;
1011:     if (pos < len)
1012:     {
1013:         if (count > len - pos)
1014:             count = len - pos;
1015:         result.reallocate(count);
1016:         memcpy(result.data(), data() + pos, count);
1017:         result.data()[count] = NullCharConst;
1018:     }
1019: 
1020:     return result;
1021: }
1022: 
1023: /// <summary>
1024: /// Case sensitive equality to string
1025: /// </summary>
1026: /// <param name="other">string to compare to</param>
1027: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1028: bool string::equals(const string& other) const
1029: {
1030:     return compare(other) == 0;
1031: }
1032: 
1033: /// <summary>
1034: /// Case sensitive equality to string
1035: /// </summary>
1036: /// <param name="other">string to compare to</param>
1037: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1038: bool string::equals(const ValueType* other) const
1039: {
1040:     if (other == nullptr)
1041:         return empty();
1042:     if (length() != strlen(other))
1043:         return false;
1044:     return strcmp(data(), other) == 0;
1045: }
1046: 
1047: /// <summary>
1048: /// Case insensitive equality to string
1049: /// </summary>
1050: /// <param name="other">string to compare to</param>
1051: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1052: bool string::equals_case_insensitive(const string& other) const
1053: {
1054:     if (length() != other.length())
1055:         return false;
1056:     if (empty())
1057:         return true;
1058:     return strcasecmp(data(), other.data()) == 0;
1059: }
1060: 
1061: /// <summary>
1062: /// Case insensitive equality to string
1063: /// </summary>
1064: /// <param name="other">string to compare to</param>
1065: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1066: bool string::equals_case_insensitive(const ValueType* other) const
1067: {
1068:     if (other == nullptr)
1069:         return empty();
1070:     if (length() != strlen(other))
1071:         return false;
1072:     return strcasecmp(data(), other) == 0;
1073: }
1074: 
1075: /// <summary>
1076: /// Case sensitive compare to string
1077: ///
1078: /// Compares the complete string, character by character
1079: /// </summary>
1080: /// <param name="str">string to compare to</param>
1081: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1082: int string::compare(const string& str) const
1083: {
1084:     if (empty())
1085:     {
1086:         if (str.empty())
1087:             return 0;
1088:         return -1;
1089:     }
1090:     if (str.empty())
1091:         return 1;
1092: 
1093:     return strcmp(data(), str.data());
1094: }
1095: 
1096: /// <summary>
1097: /// Case sensitive compare to string
1098: ///
1099: /// Compares the substring from pos to pos+count to str
1100: /// </summary>
1101: /// <param name="pos">Starting position of substring to compare to str</param>
1102: /// <param name="count">Number of characters in substring to compare to str</param>
1103: /// <param name="str">string to compare to</param>
1104: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1105: int string::compare(size_t pos, size_t count, const string& str) const
1106: {
1107:     return substr(pos, count).compare(str);
1108: }
1109: 
1110: /// <summary>
1111: /// Case sensitive compare to string
1112: ///
1113: /// Compares the substring from pos to pos+count to the substring from strPos to strPos+strCount of str
1114: /// </summary>
1115: /// <param name="pos">Starting position of substring to compare to str</param>
1116: /// <param name="count">Number of characters in substring to compare to str</param>
1117: /// <param name="str">string to compare to</param>
1118: /// <param name="strPos">Starting position of substring of str to compare</param>
1119: /// <param name="strCount">Number of characters in substring of str to compare</param>
1120: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1121: int string::compare(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/) const
1122: {
1123:     return substr(pos, count).compare(str.substr(strPos, strCount));
1124: }
1125: 
1126: /// <summary>
1127: /// Case sensitive compare to string
1128: ///
1129: /// Compares the complete string to str
1130: /// </summary>
1131: /// <param name="str">string to compare to</param>
1132: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1133: int string::compare(const ValueType* str) const
1134: {
1135:     size_t strLength{};
1136:     if (str != nullptr)
1137:         strLength = strlen(str);
1138:     if (empty())
1139:     {
1140:         if (strLength == 0)
1141:             return 0;
1142:         return -1;
1143:     }
1144:     if (strLength == 0)
1145:         return 1;
1146: 
1147:     return strcmp(data(), str);
1148: }
1149: 
1150: /// <summary>
1151: /// Case sensitive compare to string
1152: ///
1153: /// Compares the substring from pos to pos+count to str
1154: /// </summary>
1155: /// <param name="pos">Starting position of substring to compare to str</param>
1156: /// <param name="count">Number of characters in substring to compare to str</param>
1157: /// <param name="str">string to compare to</param>
1158: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1159: int string::compare(size_t pos, size_t count, const ValueType* str) const
1160: {
1161:     size_t strLength{};
1162:     if (str != nullptr)
1163:         strLength = strlen(str);
1164: 
1165:     auto len = length();
1166:     if (pos >= len)
1167:         len = 0;
1168:     len -= pos;
1169:     if (count < len)
1170:         len = count;
1171:     if (len == 0)
1172:     {
1173:         if (strLength == 0)
1174:             return 0;
1175:         return -1;
1176:     }
1177:     if (strLength == 0)
1178:         return 1;
1179: 
1180:     auto maxLen = strLength;
1181:     if (maxLen < len)
1182:         maxLen = len;
1183:     return strncmp(data() + pos, str, maxLen);
1184: }
1185: 
1186: /// <summary>
1187: /// Case sensitive compare to string
1188: ///
1189: /// Compares the substring from pos to pos+count to the first strCount characters of str
1190: /// </summary>
1191: /// <param name="pos">Starting position of substring to compare to str</param>
1192: /// <param name="count">Number of characters in substring to compare to str</param>
1193: /// <param name="str">string to compare to</param>
1194: /// <param name="strCount">Number of characters in substring of str to compare</param>
1195: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1196: int string::compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const
1197: {
1198:     size_t strLength{};
1199:     if (str != nullptr)
1200:         strLength = strlen(str);
1201: 
1202:     auto len = length();
1203:     if (pos >= len)
1204:         len = 0;
1205:     len -= pos;
1206:     if (count < len)
1207:         len = count;
1208: 
1209:     if (strCount < strLength)
1210:         strLength = strCount;
1211: 
1212:     if (len == 0)
1213:     {
1214:         if (strLength == 0)
1215:             return 0;
1216:         return -1;
1217:     }
1218:     if (strLength == 0)
1219:         return 1;
1220: 
1221:     auto maxLen = strLength;
1222:     if (maxLen < len)
1223:         maxLen = len;
1224:     return strncmp(data() + pos, str, maxLen);
1225: }
1226: 
1227: /// <summary>
1228: /// replace substring
1229: ///
1230: /// Replaces the substring from pos to pos+count with str
1231: /// </summary>
1232: /// <param name="pos">Starting position of substring to replace</param>
1233: /// <param name="count">Number of characters in substring to replace</param>
1234: /// <param name="str">string to replace with</param>
1235: /// <returns>Returns the reference to the resulting string</returns>
1236: string& string::replace(size_t pos, size_t count, const string& str)
1237: {
1238:     string result = substr(0, pos) + str + substr(pos + count);
1239:     assign(result);
1240:     return *this;
1241: }
1242: 
1243: /// <summary>
1244: /// replace substring
1245: ///
1246: /// Replaces the substring from pos to pos+count with the substring from strPos to strPos+strCount of str
1247: /// </summary>
1248: /// <param name="pos">Starting position of substring to replace</param>
1249: /// <param name="count">Number of characters in substring to replace</param>
1250: /// <param name="str">string to replace with</param>
1251: /// <param name="strPos">Starting position of substring of str to replace with</param>
1252: /// <param name="strCount">Number of characters in substring of str to replace with</param>
1253: /// <returns>Returns the reference to the resulting string</returns>
1254: string& string::replace(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/)
1255: {
1256:     string result = substr(0, pos) + str.substr(strPos, strCount) + substr(pos + count);
1257:     assign(result);
1258:     return *this;
1259: }
1260: 
1261: /// <summary>
1262: /// replace substring
1263: ///
1264: /// Replaces the substring from pos to pos+count with str
1265: /// </summary>
1266: /// <param name="pos">Starting position of substring to replace</param>
1267: /// <param name="count">Number of characters in substring to replace</param>
1268: /// <param name="str">string to replace with</param>
1269: /// <returns>Returns the reference to the resulting string</returns>
1270: string& string::replace(size_t pos, size_t count, const ValueType* str)
1271: {
1272:     string result = substr(0, pos) + str + substr(pos + count);
1273:     assign(result);
1274:     return *this;
1275: }
1276: 
1277: /// <summary>
1278: /// replace substring
1279: ///
1280: /// Replaces the substring from pos to pos+count with the first strCount characters of str
1281: /// </summary>
1282: /// <param name="pos">Starting position of substring to replace</param>
1283: /// <param name="count">Number of characters in substring to replace</param>
1284: /// <param name="str">string to replace with</param>
1285: /// <param name="strCount">Number of characters in substring to replace with</param>
1286: /// <returns>Returns the reference to the resulting string</returns>
1287: string& string::replace(size_t pos, size_t count, const ValueType* str, size_t strCount)
1288: {
1289:     string result = substr(0, pos) + string(str, strCount) + substr(pos + count);
1290:     assign(result);
1291:     return *this;
1292: }
1293: 
1294: /// <summary>
1295: /// replace substring
1296: ///
1297: /// Replaces the substring from pos to pos+count with ch
1298: /// </summary>
1299: /// <param name="pos">Starting position of substring to replace</param>
1300: /// <param name="count">Number of characters in substring to replace</param>
1301: /// <param name="ch">Characters to replace with</param>
1302: /// <returns>Returns the reference to the resulting string</returns>
1303: string& string::replace(size_t pos, size_t count, ValueType ch)
1304: {
1305:     return replace(pos, count, ch, 1);
1306: }
1307: 
1308: /// <summary>
1309: /// replace substring
1310: ///
1311: /// Replaces the substring from pos to pos+count with a sequence of chCount copies of ch
1312: /// </summary>
1313: /// <param name="pos">Starting position of substring to replace</param>
1314: /// <param name="count">Number of characters in substring to replace</param>
1315: /// <param name="ch">Characters to replace with</param>
1316: /// <param name="chCount">Number of copies of ch to replace with</param>
1317: /// <returns>Returns the reference to the resulting string</returns>
1318: string& string::replace(size_t pos, size_t count, ValueType ch, size_t chCount)
1319: {
1320:     string result = substr(0, pos) + string(chCount, ch) + substr(pos + count);
1321:     assign(result);
1322:     return *this;
1323: }
1324: 
1325: /// <summary>
1326: /// replace substring
1327: ///
1328: /// Replaces all instances of the substring oldStr (if existing) with newStr
1329: /// </summary>
1330: /// <param name="oldStr">string to find in string</param>
1331: /// <param name="newStr">string to replace with</param>
1332: /// <returns>Returns the number of times the string was replaced</returns>
1333: int string::replace(const string& oldStr, const string& newStr)
1334: {
1335:     size_t pos = find(oldStr);
1336:     size_t oldLength = oldStr.length();
1337:     size_t newLength = newStr.length();
1338:     int count = 0;
1339:     while (pos != npos)
1340:     {
1341:         replace(pos, oldLength, newStr);
1342:         pos += newLength;
1343:         pos = find(oldStr, pos);
1344:         count++;
1345:     }
1346:     return count;
1347: }
1348: 
1349: /// <summary>
1350: /// replace substring
1351: ///
1352: /// Replaces all instances of the substring oldStr (if existing) with newStr
1353: /// </summary>
1354: /// <param name="oldStr">string to find in string</param>
1355: /// <param name="newStr">string to replace with</param>
1356: /// <returns>Returns the number of times the string was replaced</returns>
1357: int string::replace(const ValueType* oldStr, const ValueType* newStr)
1358: {
1359:     if ((oldStr == nullptr) || (newStr == nullptr))
1360:         return 0;
1361:     size_t pos = find(oldStr);
1362:     size_t oldLength = strlen(oldStr);
1363:     size_t newLength = strlen(newStr);
1364:     int count = 0;
1365:     while (pos != npos)
1366:     {
1367:         replace(pos, oldLength, newStr);
1368:         pos += newLength;
1369:         pos = find(oldStr, pos);
1370:         count++;
1371:     }
1372:     return count;
1373: }
1374: 
1375: /// <summary>
1376: /// Align string
1377: ///
1378: /// Pads the string on the left (width > 0) or on the right (width < 0) up to a length of width characters. If the string is larger than width characters, it is not modified.
1379: /// </summary>
1380: /// <param name="width">length of target string. If width < 0, the string is padded to the right with spaces up to -width characters.
1381: /// if width > 0, the string is padded to the left with space up to width characters</param>
1382: /// <returns>Returns the number of times the string was replaced</returns>
1383: string string::align(int width) const
1384: {
1385:     string result;
1386:     int    absWidth = (width > 0) ? width : -width;
1387:     auto   len = length();
1388:     if (static_cast<size_t>(absWidth) > len)
1389:     {
1390:         if (width < 0)
1391:         {
1392:             result = *this + string(static_cast<unsigned int>(-width) - len, ' ');
1393:         }
1394:         else
1395:         {
1396:             result = string(static_cast<unsigned int>(width) - len, ' ') + *this;
1397:         }
1398:     }
1399:     else
1400:         result = *this;
1401:     return result;
1402: }
1403: 
1404: /// <summary>
1405: /// Allocate or re-allocate string to have a capacity of requestedLength characters (+1 is added for the terminating null character)
1406: /// </summary>
1407: /// <param name="requestedLength">Amount of characters in the string to allocate space for (excluding the terminating null character)</param>
1408: /// <returns>True if successful, false otherwise</returns>
1409: bool string::reallocate(size_t requestedLength)
1410: {
1411:     auto requestedSize = requestedLength + 1;
1412:     auto allocationSize = NextPowerOf2((requestedSize < MinimumAllocationSize) ? MinimumAllocationSize : requestedSize);
1413: 
1414:     if (!reallocate_allocation_size(allocationSize))
1415:         return false;
1416:     if (m_end > m_buffer + requestedLength)
1417:         m_end = m_buffer + requestedLength;
1418:     return true;
1419: }
1420: 
1421: /// <summary>
1422: /// Allocate or re-allocate string to have a capacity of allocationSize bytes
1423: /// </summary>
1424: /// <param name="allocationSize">Amount of bytes to allocate space for</param>
1425: /// <returns>True if successful, false otherwise</returns>
1426: bool string::reallocate_allocation_size(size_t allocationSize)
1427: {
1428:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1429:     if (newBuffer == nullptr)
1430:     {
1431:         return false;
1432:     }
1433:     m_buffer = newBuffer;
1434:     if (m_end == nullptr)
1435:         m_end = m_buffer;
1436:     if (m_end > m_buffer + allocationSize)
1437:         m_end = m_buffer + allocationSize;
1438:     m_allocatedSize = allocationSize;
1439:     return true;
1440: }
```

As you can see, this is a very extensive class.
A few remarks:
- Line 51: We use a minimum allocation size of 64, meaning that every string will have storage space for 63 characters plus a null character for termination, at the least.
- Line 54: We use a special static variable `NullCharConst` when we need to return something constant, but the string is not allocated.
- Line 56: Similarly, we use a special static variable `NullChar` when we need to return something non constant, but the string is not allocated.
This variable is non-const, so will have to be initialized every time we return it, to make sure it is still a null character.
- Line 166-174: We implement a move constructor, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 239-251: We implement a move assignment, even though we do not have the `std::move` operation which is part of the standard C++ library.
The compiler however will sometimes use the move constructor to optimize
- Line 259-273: We use our own version of a iterator template, which we'll get to, to enable creating the begin() and end() methods. These allow us to use the standard c++ `for (x : string)` construct
- Line 281-284: We also use our own version of a const_iterator template for const iterators, which we'll get to, to enable creating the begin() and end() const methods.
- Line 970-993: We also have the `contains()` methods, which are new to C++23, however they are quite convenient
- Line 1023-1068: We use the methods `equals()` and `equals_case_insenstitive()` to compare strings. Both are not standard, but convenient
- Line 1328-1368: We implement two extra variants of `replace(0` (replacing multiple instances of a string) for convenience
- Line 1378-1397: Again for convenience, we implement the `align()` method. We'll be using this later on for aligning strings when formatting

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
This is a first attempt at creating class / micro / unit tests for our code, which we will start doing in the next tutorial.

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
This is the subject of the next tutorial.

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

