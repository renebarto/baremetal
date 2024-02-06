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
98: }
99: 
100: /// <summary>
101: /// Constructor
102: /// 
103: /// Initializes the string with up to count characters in the specified string. A null character is always added.
104: /// </summary>
105: /// <param name="str">string to initialize with</param>
106: /// <param name="count">Maximum number of characters from str to initialize with. If count is larger than the actual string length, only the string length is used</param>
107: string::string(const ValueType* str, size_t count)
108:     : m_buffer{}
109:     , m_end{}
110:     , m_allocatedSize{}
111: {
112:     if (str == nullptr)
113:         return;
114:     auto size = strlen(str);
115:     if (count < size)
116:         size = count;
117:     if (reallocate(size))
118:     {
119:         strncpy(m_buffer, str, size);
120:     }
121: }
122: 
123: /// <summary>
124: /// Constructor
125: /// 
126: /// Initializes the string with the specified count times the specified character. A null character is always added.
127: /// </summary>
128: /// <param name="count">Number of characters of value ch to initialized with</param>
129: /// <param name="ch">Character to initialize with</param>
130: string::string(size_t count, ValueType ch)
131:     : m_buffer{}
132:     , m_end{}
133:     , m_allocatedSize{}
134: {
135:     auto size = count;
136:     if (reallocate(size))
137:     {
138:         memset(m_buffer, ch, size);
139:     }
140: }
141: 
142: /// <summary>
143: /// Copy constructor
144: /// 
145: /// Initializes the string with the specified string value.
146: /// </summary>
147: /// <param name="other">string to initialize with</param>
148: string::string(const string& other)
149:     : m_buffer{}
150:     , m_end{}
151:     , m_allocatedSize{}
152: {
153:     auto size = other.length();
154:     if (reallocate(size))
155:     {
156:         strncpy(m_buffer, other.data(), size);
157:     }
158: }
159: 
160: /// <summary>
161: /// Move constructor
162: /// 
163: /// Initializes the string by moving the contents from the specified string value.
164: /// </summary>
165: /// <param name="other">string to initialize with</param>
166: string::string(string&& other)
167:     : m_buffer{other.m_buffer}
168:     , m_end{other.m_end}
169:     , m_allocatedSize{other.m_allocatedSize}
170: {
171:     other.m_buffer = nullptr;
172:     other.m_end = nullptr;
173:     other.m_allocatedSize = 0;
174: }
175: 
176: /// <summary>
177: /// Constructor
178: /// 
179: /// Initializes the string with the substring starting at specified position, for the specified number of characters, from the specified string value.
180: /// </summary>
181: /// <param name="other">string to initialize with</param>
182: /// <param name="pos">Position in other to start copying charaters from</param>
183: /// <param name="count">Maximum number of characters to copy from other. Default is until end of string. If pos + count is larger than the actual length of the string, string string is copied until the end</param>
184: string::string(const string& other, size_t pos, size_t count /*= npos*/)
185:     : m_buffer{}
186:     , m_end{}
187:     , m_allocatedSize{}
188: {
189:     auto size = other.length() - pos;
190:     if (count < size)
191:         size = count;
192:     if (reallocate(size))
193:     {
194:         strncpy(m_buffer, other.data() + pos, size);
195:     }
196: }
197: 
198: /// <summary>
199: /// Const character cast operator
200: /// 
201: /// Returns the pointer to the start of the string.
202: /// </summary>
203: string::operator const ValueType* () const
204: {
205:     return data();
206: }
207: 
208: /// <summary>
209: /// Assignment operator
210: /// 
211: /// Assigns the specified string value to the string.
212: /// </summary>
213: /// <param name="str">string value to assign to the string</param>
214: /// <returns>A reference to the string</returns>
215: string& string::operator = (const ValueType* str)
216: {
217:     return assign(str);
218: }
219: 
220: /// <summary>
221: /// Assignment operator
222: /// 
223: /// Assigns the specified string value to the string.
224: /// </summary>
225: /// <param name="str">string value to assign to the string</param>
226: /// <returns>A reference to the string</returns>
227: string& string::operator = (const string& str)
228: {
229:     return assign(str);
230: }
231: 
232: /// <summary>
233: /// Move operator
234: /// 
235: /// Assigns the specified string value to the string by moving the contents of the specified string.
236: /// </summary>
237: /// <param name="str">string value to assign to the string</param>
238: /// <returns>A reference to the string</returns>
239: string& string::operator = (string&& str)
240: {
241:     if (&str != this)
242:     {
243:         m_buffer = str.m_buffer;
244:         m_end = str.m_end;
245:         m_allocatedSize = str.m_allocatedSize;
246:         str.m_buffer = nullptr;
247:         str.m_end = nullptr;
248:         str.m_allocatedSize = 0;
249:     }
250:     return *this;
251: }
252: 
253: /// <summary>
254: /// Non-const iterator to the start of the string
255: /// 
256: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
257: /// </summary>
258: /// <returns>iterator to the value type, acting as the start of the string</returns>
259: iterator<string::ValueType> string::begin()
260: {
261:     return iterator(m_buffer, m_end);
262: }
263: 
264: /// <summary>
265: /// Non-const iterator to the end of the string + 1
266: /// 
267: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
268: /// </summary>
269: /// <returns>iterator to the value type, acting as the end of the string</returns>
270: iterator<string::ValueType> string::end()
271: {
272:     return iterator(m_end, m_end);
273: }
274: 
275: /// <summary>
276: /// Const iterator to the start of the string
277: /// 
278: /// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
279: /// </summary>
280: /// <returns>const_iterator to the value type, acting as the start of the string</returns>
281: const_iterator<string::ValueType> string::begin() const
282: {
283:     return const_iterator(m_buffer, m_end);
284: }
285: 
286: /// <summary>
287: /// Const iterator to the end of the string + 1
288: /// 
289: /// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
290: /// </summary>
291: /// <returns>const_iterator to the value type, acting as the end of the string</returns>
292: const_iterator<string::ValueType> string::end() const
293: {
294:     return const_iterator(m_end, m_end);
295: }
296: 
297: /// <summary>
298: /// assign a string value
299: /// 
300: /// Assigns the specified string value to the string
301: /// </summary>
302: /// <param name="str">string value to assign to the string</param>
303: /// <returns>A reference to the string</returns>
304: string& string::assign(const ValueType* str)
305: {
306:     size_t size{};
307:     if (str != nullptr)
308:     {
309:         size = strlen(str);
310:     }
311:     if (size > m_allocatedSize)
312:     {
313:         if (!reallocate(size))
314:             return *this;
315:     }
316:     strncpy(m_buffer, str, size);
317:     m_end = m_buffer + size;
318:     m_buffer[size] = NullCharConst;
319:     return *this;
320: }
321: 
322: /// <summary>
323: /// assign a string value
324: /// 
325: /// Assigns the specified string value, up to the specified count of characters, to the string.
326: /// </summary>
327: /// <param name="str">string value to assign to the string</param>
328: /// <param name="count">Maximum number of characters to copy from the string. If count is larger than the string length, the length of the string is used</param>
329: /// <returns>A reference to the string</returns>
330: string& string::assign(const ValueType* str, size_t count)
331: {
332:     size_t size{};
333:     if (str != nullptr)
334:     {
335:         size = strlen(str);
336:     }
337:     if (count < size)
338:         size = count;
339:     if (size > m_allocatedSize)
340:     {
341:         if (!reallocate(size))
342:             return *this;
343:     }
344:     strncpy(m_buffer, str, size);
345:     m_end = m_buffer + size;
346:     m_buffer[size] = NullCharConst;
347:     return *this;
348: }
349: 
350: /// <summary>
351: /// assign a string value
352: /// 
353: /// Assigns a string containing the specified count times the specified characters to the string
354: /// </summary>
355: /// <param name="count">Number copies of ch to copy to the string</param>
356: /// <param name="ch">Character to initialize with</param>
357: /// <returns>A reference to the string</returns>
358: string& string::assign(size_t count, ValueType ch)
359: {
360:     auto size = count;
361:     if (size > m_allocatedSize)
362:     {
363:         if (!reallocate(size))
364:             return *this;
365:     }
366:     memset(m_buffer, ch, size);
367:     m_end = m_buffer + size;
368:     m_buffer[size] = NullCharConst;
369:     return *this;
370: }
371: 
372: /// <summary>
373: /// assign a string value
374: /// 
375: /// Assigns the specified string value to the string
376: /// </summary>
377: /// <param name="str">string value to assign to the string</param>
378: /// <returns>A reference to the string</returns>
379: string& string::assign(const string& str)
380: {
381:     auto size = str.length();
382:     if (size > m_allocatedSize)
383:     {
384:         if (!reallocate(size))
385:             return *this;
386:     }
387:     strncpy(m_buffer, str.data(), size);
388:     m_end = m_buffer + size;
389:     m_buffer[size] = NullCharConst;
390:     return *this;
391: }
392: 
393: /// <summary>
394: /// assign a string value
395: /// 
396: /// Assigns the substring start from the specified position for the specified count of characters of specified string value to the string
397: /// </summary>
398: /// <param name="str">string value to assign to the string</param>
399: /// <param name="pos">Starting position of substring to copy from str</param>
400: /// <param name="count">Maximum number of characters to copy from str.
401: /// Default is until end of string. If pos + count is larger than the stirn length, characters are copied until end of string</param>
402: /// <returns>A reference to the string</returns>
403: string& string::assign(const string& str, size_t pos, size_t count /*= npos*/)
404: {
405:     auto size = str.length() - pos;
406:     if (count < size)
407:         size = count;
408:     if (size > m_allocatedSize)
409:     {
410:         if (!reallocate(size))
411:             return *this;
412:     }
413:     strncpy(m_buffer, str.data() + pos, size);
414:     m_end = m_buffer + size;
415:     m_buffer[size] = NullCharConst;
416:     return *this;
417: }
418: 
419: /// <summary>
420: /// Return the character at specified position
421: /// </summary>
422: /// <param name="pos">Position in string</param>
423: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
424: string::ValueType& string::at(size_t pos)
425: {
426:     if (pos >= length())
427:     {
428:         NullChar = NullCharConst;
429:         return NullChar;
430:     }
431:     return m_buffer[pos];
432: }
433: 
434: /// <summary>
435: /// Return the character at specified position
436: /// </summary>
437: /// <param name="pos">Position in string</param>
438: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, a reference to a const null character is returned (NullCharConst)</returns>
439: const string::ValueType& string::at(size_t pos) const
440: {
441:     if (pos >= length())
442:         return NullCharConst;
443:     return m_buffer[pos];
444: }
445: 
446: /// <summary>
447: /// Return the first character
448: /// </summary>
449: /// <returns>Returns a non-const reference to the first character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
450: string::ValueType& string::front()
451: {
452:     if (empty())
453:     {
454:         NullChar = NullCharConst;
455:         return NullChar;
456:     }
457:     return *m_buffer;
458: }
459: 
460: /// <summary>
461: /// Return the first character
462: /// </summary>
463: /// <returns>Returns a const reference to the first character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
464: const string::ValueType& string::front() const
465: {
466:     if (empty())
467:         return NullCharConst;
468:     return *m_buffer;
469: }
470: 
471: /// <summary>
472: /// Return the last character
473: /// </summary>
474: /// <returns>Returns a non-const reference to the last character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
475: string::ValueType& string::back()
476: {
477:     if (empty())
478:     {
479:         NullChar = NullCharConst;
480:         return NullChar;
481:     }
482:     return *(m_end - 1);
483: }
484: 
485: /// <summary>
486: /// Return the last character
487: /// </summary>
488: /// <returns>Returns a const reference to the last character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
489: const string::ValueType& string::back() const
490: {
491:     if (empty())
492:         return NullCharConst;
493:     return *(m_end - 1);
494: }
495: 
496: /// <summary>
497: /// Return the character at specified position
498: /// </summary>
499: /// <param name="pos">Position in string</param>
500: /// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
501: string::ValueType& string::operator[] (size_t pos)
502: {
503:     return m_buffer[pos];
504: }
505: 
506: /// <summary>
507: /// Return the character at specified position
508: /// </summary>
509: /// <param name="pos">Position in string</param>
510: /// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
511: const string::ValueType& string::operator[] (size_t pos) const
512: {
513:     return m_buffer[pos];
514: }
515: 
516: /// <summary>
517: /// Return the buffer pointer
518: /// </summary>
519: /// <returns>Returns a non-const pointer to the buffer. If the buffer is not allocated, a pointer to a non-const null character (NullChar, initialized before returning) is returned</returns>
520: string::ValueType* string::data()
521: {
522:     NullChar = NullCharConst;
523:     return (m_buffer == nullptr) ? &NullChar : m_buffer;
524: }
525: 
526: /// <summary>
527: /// Return the buffer pointer
528: /// </summary>
529: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
530: const string::ValueType* string::data() const
531: {
532:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
533: }
534: 
535: /// <summary>
536: /// Return the buffer pointer
537: /// </summary>
538: /// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
539: const string::ValueType* string::c_str() const
540: {
541:     return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
542: }
543: 
544: /// <summary>
545: /// Determine whether string is empty.
546: /// </summary>
547: /// <returns>Returns true when the string is empty (not allocated or no contents), false otherwise</returns>
548: bool string::empty() const
549: {
550:     return m_end == m_buffer;
551: }
552: 
553: /// <summary>
554: /// Return the size of the string
555: /// 
556: /// This method is the equivalent of length().
557: /// </summary>
558: /// <returns>Returns the size (or length) of the string</returns>
559: size_t string::size() const
560: {
561:     return m_end - m_buffer;
562: }
563: 
564: /// <summary>
565: /// Return the length of the string
566: /// 
567: /// This method is the equivalent of size().
568: /// </summary>
569: /// <returns>Returns the size (or length) of the string</returns>
570: size_t string::length() const
571: {
572:     return m_end - m_buffer;
573: }
574: 
575: /// <summary>
576: /// Return the capacity of the string
577: /// 
578: /// The capacity is the size of the allocated buffer. The string can grow to that length before it needs to be re-allocated.
579: /// </summary>
580: /// <returns>Returns the size (or length) of the string</returns>
581: size_t string::capacity() const
582: {
583:     return m_allocatedSize;
584: }
585: 
586: /// <summary>
587: /// Reserved a buffer capacity
588: /// 
589: /// Allocates a buffer of specified size
590: /// </summary>
591: /// <param name="newCapacity"></param>
592: /// <returns>Returns the capacity of the string</returns>
593: size_t string::reserve(size_t newCapacity)
594: {
595:     reallocate_allocation_size(newCapacity);
596:     return m_allocatedSize;
597: }
598: 
599: /// <summary>
600: /// append operator
601: /// 
602: /// Appends a character to the string
603: /// </summary>
604: /// <param name="ch">Character to append</param>
605: /// <returns>Returns a reference to the string</returns>
606: string& string::operator +=(ValueType ch)
607: {
608:     append(1, ch);
609:     return *this;
610: }
611: 
612: /// <summary>
613: /// append operator
614: /// 
615: /// Appends a string to the string
616: /// </summary>
617: /// <param name="str">string to append</param>
618: /// <returns>Returns a reference to the string</returns>
619: string& string::operator +=(const string& str)
620: {
621:     append(str);
622:     return *this;
623: }
624: 
625: /// <summary>
626: /// append operator
627: /// 
628: /// Appends a string to the string
629: /// </summary>
630: /// <param name="str">string to append. If nullptr the nothing is appended</param>
631: /// <returns>Returns a reference to the string</returns>
632: string& string::operator +=(const ValueType* str)
633: {
634:     append(str);
635:     return *this;
636: }
637: 
638: /// <summary>
639: /// append operator
640: /// 
641: /// Appends a sequence of count times the same character ch to the string
642: /// </summary>
643: /// <param name="count">Number of characters to append</param>
644: /// <param name="ch">Character to append</param>
645: void string::append(size_t count, ValueType ch)
646: {
647:     auto len = length();
648:     auto size = len + count;
649:     if (size > m_allocatedSize)
650:     {
651:         if (!reallocate(size))
652:             return;
653:     }
654:     memset(m_buffer + len, ch, count);
655:     m_end += count;
656:     m_buffer[size] = NullCharConst;
657: }
658: 
659: /// <summary>
660: /// append operator
661: /// 
662: /// Appends a string to the string
663: /// </summary>
664: /// <param name="str">string to append</param>
665: void string::append(const string& str)
666: {
667:     auto len = length();
668:     auto strLength = str.length();
669:     auto size = len+ strLength;
670:     if (size > m_allocatedSize)
671:     {
672:         if (!reallocate(size))
673:             return;
674:     }
675:     strncpy(m_buffer + len, str.data(), strLength);
676:     m_end += strLength;
677:     m_buffer[size] = NullCharConst;
678: }
679: 
680: /// <summary>
681: /// append operator
682: /// 
683: /// Appends a substring of str to the string
684: /// </summary>
685: /// <param name="str">string to append from</param>
686: /// <param name="pos">Start position in str to copy characters from</param>
687: /// <param name="count">Number of characters to copy from str. Default is until the end of the string. If count is larger than the string length, characters are copied up to the end of the string</param>
688: void string::append(const string& str, size_t pos, size_t count /*= npos*/)
689: {
690:     auto len = length();
691:     auto strLength = str.length();
692:     auto strCount = count;
693:     if (pos >= strLength)
694:         return;
695:     if (pos + strCount > strLength)
696:         strCount = strLength - pos;
697:     auto size = len + strCount;
698:     if (size > m_allocatedSize)
699:     {
700:         if (!reallocate(size))
701:             return;
702:     }
703:     strncpy(m_buffer + len, str.data() + pos, strCount);
704:     m_end += strCount;
705:     m_buffer[size] = NullCharConst;
706: }
707: 
708: /// <summary>
709: /// append operator
710: /// 
711: /// Appends a string to the string
712: /// </summary>
713: /// <param name="str">string to append. If nullptr the nothing is appended</param>
714: void string::append(const ValueType* str)
715: {
716:     if (str == nullptr)
717:         return;
718:     auto len = length();
719:     auto strLength = strlen(str);
720:     auto size = len + strLength;
721:     if (size > m_allocatedSize)
722:     {
723:         if (!reallocate(size))
724:             return;
725:     }
726:     strncpy(m_buffer + len, str, strLength);
727:     m_end += strLength;
728:     m_buffer[size] = NullCharConst;
729: }
730: 
731: /// <summary>
732: /// append operator
733: /// 
734: /// Appends a number of characters from str to the string
735: /// </summary>
736: /// <param name="str">string to append. If nullptr the nothing is appended</param>
737: /// <param name="count">Number of characters to copy from str. If count is larger than the string length, the complete string is copied</param>
738: void string::append(const ValueType* str, size_t count)
739: {
740:     if (str == nullptr)
741:         return;
742:     auto len = length();
743:     auto strLength = strlen(str);
744:     auto strCount = count;
745:     if (strCount > strLength)
746:         strCount = strLength;
747:     auto size = len + strCount;
748:     if (size > m_allocatedSize)
749:     {
750:         if (!reallocate(size))
751:             return;
752:     }
753:     strncpy(m_buffer + len, str, strCount);
754:     m_end += strCount;
755:     m_buffer[size] = NullCharConst;
756: }
757: 
758: /// <summary>
759: /// clear the string
760: /// 
761: /// Clears the contents of the string, but does not free or reallocate the buffer
762: /// </summary>
763: void string::clear()
764: {
765:     if (!empty())
766:     {
767:         m_end = m_buffer;
768:         m_buffer[0] = NullCharConst;
769:     }
770: }
771: 
772: /// <summary>
773: /// find a substring in the string
774: /// 
775: /// If empty string, always finds the string.
776: /// </summary>
777: /// <param name="str">Substring to find</param>
778: /// <param name="pos">Starting position in string to start searching</param>
779: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
780: size_t string::find(const string& str, size_t pos /*= 0*/) const
781: {
782:     auto len = length();
783:     auto patternLength = str.length();
784:     if (pos >= len)
785:         return npos;
786:     auto needle = str.data();
787:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
788:     {
789:         if (memcmp(haystack, needle, patternLength) == 0)
790:             return haystack - m_buffer;
791:     }
792:     return npos;
793: }
794: 
795: /// <summary>
796: /// find a substring in the string
797: /// 
798: /// If nullptr or empty string, always finds the string.
799: /// </summary>
800: /// <param name="str">Substring to find</param>
801: /// <param name="pos">Starting position in string to start searching</param>
802: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
803: size_t string::find(const ValueType* str, size_t pos /*= 0*/) const
804: {
805:     size_t strLength{};
806:     if (str != nullptr)
807:     {
808:         strLength = strlen(str);
809:     }
810:     auto len = length();
811:     auto patternLength = strLength;
812:     if (pos >= len)
813:         return npos;
814:     auto needle = str;
815:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
816:     {
817:         if (memcmp(haystack, needle, patternLength) == 0)
818:             return haystack - m_buffer;
819:     }
820:     return npos;
821: }
822: 
823: /// <summary>
824: /// find a substring in the string
825: /// 
826: /// If nullptr or empty string, always finds the string.
827: /// </summary>
828: /// <param name="str">Substring to find</param>
829: /// <param name="pos">Starting position in string to start searching</param>
830: /// <param name="count">Number of characters from str to compare</param>
831: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
832: size_t string::find(const ValueType* str, size_t pos, size_t count) const
833: {
834:     size_t strLength{};
835:     if (str != nullptr)
836:     {
837:         strLength = strlen(str);
838:     }
839:     auto len = length();
840:     auto patternLength = strLength;
841:     if (pos >= len)
842:         return npos;
843:     if (count < patternLength)
844:         patternLength = count;
845:     auto needle = str;
846:     for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
847:     {
848:         if (memcmp(haystack, needle, patternLength) == 0)
849:             return haystack - m_buffer;
850:     }
851:     return npos;
852: }
853: 
854: /// <summary>
855: /// find a character in the string
856: /// </summary>
857: /// <param name="ch">Character to find</param>
858: /// <param name="pos">Starting position in string to start searching</param>
859: /// <returns>Location of first character in string of match if found, string::npos if not found</returns>
860: size_t string::find(ValueType ch, size_t pos /*= 0*/) const
861: {
862:     auto len = length();
863:     if (pos >= len)
864:         return npos;
865:     for (const ValueType* haystack = data() + pos; haystack <= m_end; ++haystack)
866:     {
867:         if (*haystack == ch)
868:             return haystack - m_buffer;
869:     }
870:     return npos;
871: }
872: 
873: /// <summary>
874: /// Check whether string starts with character
875: /// </summary>
876: /// <param name="ch">Character to find</param>
877: /// <returns>Returns true if ch is first character in string, false otherwise</returns>
878: bool string::starts_with(ValueType ch) const
879: {
880:     if (empty())
881:         return false;
882:     return m_buffer[0] == ch;
883: }
884: 
885: /// <summary>
886: /// Check whether string starts with substring
887: /// </summary>
888: /// <param name="str">SubString to find</param>
889: /// <returns>Returns true if str is first part of string, false otherwise</returns>
890: bool string::starts_with(const string& str) const
891: {
892:     auto len = length();
893:     auto strLength = str.length();
894:     if (strLength >= len)
895:         return false;
896: 
897:     return memcmp(data(), str.data(), strLength) == 0;
898: }
899: 
900: /// <summary>
901: /// Check whether string starts with substring
902: /// </summary>
903: /// <param name="str">SubString to find</param>
904: /// <returns>Returns true if str is first part of string, false otherwise</returns>
905: bool string::starts_with(const ValueType* str) const
906: {
907:     size_t strLength{};
908:     if (str != nullptr)
909:     {
910:         strLength = strlen(str);
911:     }
912:     auto len = length();
913:     if (strLength >= len)
914:         return false;
915: 
916:     return memcmp(data(), str, strLength) == 0;
917: }
918: 
919: /// <summary>
920: /// Check whether string ends with character
921: /// </summary>
922: /// <param name="ch">Character to find</param>
923: /// <returns>Returns true if ch is last character in string, false otherwise</returns>
924: bool string::ends_with(ValueType ch) const
925: {
926:     if (empty())
927:         return false;
928:     return m_buffer[length() - 1] == ch;
929: }
930: 
931: /// <summary>
932: /// Check whether string ends with substring
933: /// </summary>
934: /// <param name="str">SubString to find</param>
935: /// <returns>Returns true if str is last part of string, false otherwise</returns>
936: bool string::ends_with(const string& str) const
937: {
938:     auto len = length();
939:     auto strLength = str.length();
940:     if (strLength >= len)
941:         return false;
942: 
943:     return memcmp(m_end - strLength, str.data(), strLength) == 0;
944: }
945: 
946: /// <summary>
947: /// Check whether string ends with substring
948: /// </summary>
949: /// <param name="str">SubString to find</param>
950: /// <returns>Returns true if str is last part of string, false otherwise</returns>
951: bool string::ends_with(const ValueType* str) const
952: {
953:     size_t strLength{};
954:     if (str != nullptr)
955:     {
956:         strLength = strlen(str);
957:     }
958:     auto len = length();
959:     if (strLength >= len)
960:         return false;
961: 
962:     return memcmp(m_end - strLength, str, strLength) == 0;
963: }
964: 
965: /// <summary>
966: /// Check whether string contains character
967: /// </summary>
968: /// <param name="ch">Character to find</param>
969: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
970: bool string::contains(ValueType ch) const
971: {
972:     return find(ch) != npos;
973: }
974: 
975: /// <summary>
976: /// Check whether string contains substring
977: /// </summary>
978: /// <param name="str">Substring to find</param>
979: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
980: bool string::contains(const string& str) const
981: {
982:     return find(str) != npos;
983: }
984: 
985: /// <summary>
986: /// Check whether string contains substring
987: /// </summary>
988: /// <param name="str">Substring to find</param>
989: /// <returns>Returns true if ch is contained in string, false otherwise</returns>
990: bool string::contains(const ValueType* str) const
991: {
992:     return find(str) != npos;
993: }
994: 
995: /// <summary>
996: /// Return substring
997: /// </summary>
998: /// <param name="pos">Starting position of substring in string</param>
999: /// <param name="count">length of substring to return. If count is larger than the number of characters available from position pos, the rest of the string is returned</param>
1000: /// <returns>Returns the substring at position [pos, pos + count), if available </returns>
1001: string string::substr(size_t pos /*= 0*/, size_t count /*= npos*/) const
1002: {
1003:     string result;
1004:     auto len = length();
1005:     auto size = count;
1006:     if (pos < len)
1007:     {
1008:         if (count > len - pos)
1009:             count = len - pos;
1010:         result.reallocate(count);
1011:         memcpy(result.data(), data() + pos, count);
1012:         result.data()[count] = NullCharConst;
1013:     }
1014: 
1015:     return result;
1016: }
1017: 
1018: /// <summary>
1019: /// Case sensitive equality to string
1020: /// </summary>
1021: /// <param name="other">string to compare to</param>
1022: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1023: bool string::equals(const string& other) const
1024: {
1025:     return compare(other) == 0;
1026: }
1027: 
1028: /// <summary>
1029: /// Case sensitive equality to string
1030: /// </summary>
1031: /// <param name="other">string to compare to</param>
1032: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1033: bool string::equals(const ValueType* other) const
1034: {
1035:     if (other == nullptr)
1036:         return empty();
1037:     if (length() != strlen(other))
1038:         return false;
1039:     return strcmp(data(), other) == 0;
1040: }
1041: 
1042: /// <summary>
1043: /// Case insensitive equality to string
1044: /// </summary>
1045: /// <param name="other">string to compare to</param>
1046: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1047: bool string::equals_case_insensitive(const string& other) const
1048: {
1049:     if (length() != other.length())
1050:         return false;
1051:     if (empty())
1052:         return true;
1053:     return strcasecmp(data(), other.data()) == 0;
1054: }
1055: 
1056: /// <summary>
1057: /// Case insensitive equality to string
1058: /// </summary>
1059: /// <param name="other">string to compare to</param>
1060: /// <returns>Returns true if the strings are equal, false otherwise</returns>
1061: bool string::equals_case_insensitive(const ValueType* other) const
1062: {
1063:     if (other == nullptr)
1064:         return empty();
1065:     if (length() != strlen(other))
1066:         return false;
1067:     return strcasecmp(data(), other) == 0;
1068: }
1069: 
1070: /// <summary>
1071: /// Case sensitive compare to string
1072: /// 
1073: /// Compares the complete string, character by character
1074: /// </summary>
1075: /// <param name="str">string to compare to</param>
1076: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1077: int string::compare(const string& str) const
1078: {
1079:     if (empty())
1080:     {
1081:         if (str.empty())
1082:             return 0;
1083:         return -1;
1084:     }
1085:     if (str.empty())
1086:         return 1;
1087: 
1088:     return strcmp(data(), str.data());
1089: }
1090: 
1091: /// <summary>
1092: /// Case sensitive compare to string
1093: /// 
1094: /// Compares the substring from pos to pos+count to str
1095: /// </summary>
1096: /// <param name="pos">Starting position of substring to compare to str</param>
1097: /// <param name="count">Number of characters in substring to compare to str</param>
1098: /// <param name="str">string to compare to</param>
1099: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1100: int string::compare(size_t pos, size_t count, const string& str) const
1101: {
1102:     return substr(pos, count).compare(str);
1103: }
1104: 
1105: /// <summary>
1106: /// Case sensitive compare to string
1107: /// 
1108: /// Compares the substring from pos to pos+count to the substring from strPos to strPos+strCount of str
1109: /// </summary>
1110: /// <param name="pos">Starting position of substring to compare to str</param>
1111: /// <param name="count">Number of characters in substring to compare to str</param>
1112: /// <param name="str">string to compare to</param>
1113: /// <param name="strPos">Starting position of substring of str to compare</param>
1114: /// <param name="strCount">Number of characters in substring of str to compare</param>
1115: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1116: int string::compare(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/) const
1117: {
1118:     return substr(pos, count).compare(str.substr(strPos, strCount));
1119: }
1120: 
1121: /// <summary>
1122: /// Case sensitive compare to string
1123: /// 
1124: /// Compares the complete string to str
1125: /// </summary>
1126: /// <param name="str">string to compare to</param>
1127: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1128: int string::compare(const ValueType* str) const
1129: {
1130:     size_t strLength{};
1131:     if (str != nullptr)
1132:         strLength = strlen(str);
1133:     if (empty())
1134:     {
1135:         if (strLength == 0)
1136:             return 0;
1137:         return -1;
1138:     }
1139:     if (strLength == 0)
1140:         return 1;
1141: 
1142:     return strcmp(data(), str);
1143: }
1144: 
1145: /// <summary>
1146: /// Case sensitive compare to string
1147: /// 
1148: /// Compares the substring from pos to pos+count to str
1149: /// </summary>
1150: /// <param name="pos">Starting position of substring to compare to str</param>
1151: /// <param name="count">Number of characters in substring to compare to str</param>
1152: /// <param name="str">string to compare to</param>
1153: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1154: int string::compare(size_t pos, size_t count, const ValueType* str) const
1155: {
1156:     size_t strLength{};
1157:     if (str != nullptr)
1158:         strLength = strlen(str);
1159: 
1160:     auto len = length();
1161:     if (pos >= len)
1162:         len = 0;
1163:     len -= pos;
1164:     if (count < len)
1165:         len = count;
1166:     if (len == 0)
1167:     {
1168:         if (strLength == 0)
1169:             return 0;
1170:         return -1;
1171:     }
1172:     if (strLength == 0)
1173:         return 1;
1174: 
1175:     auto maxLen = strLength;
1176:     if (maxLen < len)
1177:         maxLen = len;
1178:     return strncmp(data() + pos, str, maxLen);
1179: }
1180: 
1181: /// <summary>
1182: /// Case sensitive compare to string
1183: /// 
1184: /// Compares the substring from pos to pos+count to the first strCount characters of str
1185: /// </summary>
1186: /// <param name="pos">Starting position of substring to compare to str</param>
1187: /// <param name="count">Number of characters in substring to compare to str</param>
1188: /// <param name="str">string to compare to</param>
1189: /// <param name="strCount">Number of characters in substring of str to compare</param>
1190: /// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
1191: int string::compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const
1192: {
1193:     size_t strLength{};
1194:     if (str != nullptr)
1195:         strLength = strlen(str);
1196: 
1197:     auto len = length();
1198:     if (pos >= len)
1199:         len = 0;
1200:     len -= pos;
1201:     if (count < len)
1202:         len = count;
1203: 
1204:     if (strCount < strLength)
1205:         strLength = strCount;
1206: 
1207:     if (len == 0)
1208:     {
1209:         if (strLength == 0)
1210:             return 0;
1211:         return -1;
1212:     }
1213:     if (strLength == 0)
1214:         return 1;
1215: 
1216:     auto maxLen = strLength;
1217:     if (maxLen < len)
1218:         maxLen = len;
1219:     return strncmp(data() + pos, str, maxLen);
1220: }
1221: 
1222: /// <summary>
1223: /// replace substring
1224: /// 
1225: /// Replaces the substring from pos to pos+count with str
1226: /// </summary>
1227: /// <param name="pos">Starting position of substring to replace</param>
1228: /// <param name="count">Number of characters in substring to replace</param>
1229: /// <param name="str">string to replace with</param>
1230: /// <returns>Returns the reference to the resulting string</returns>
1231: string& string::replace(size_t pos, size_t count, const string& str)
1232: {
1233:     string result = substr(0, pos) + str + substr(pos + count);
1234:     assign(result);
1235:     return *this;
1236: }
1237: 
1238: /// <summary>
1239: /// replace substring
1240: /// 
1241: /// Replaces the substring from pos to pos+count with the substring from strPos to strPos+strCount of str
1242: /// </summary>
1243: /// <param name="pos">Starting position of substring to replace</param>
1244: /// <param name="count">Number of characters in substring to replace</param>
1245: /// <param name="str">string to replace with</param>
1246: /// <param name="strPos">Starting position of substring of str to replace with</param>
1247: /// <param name="strCount">Number of characters in substring of str to replace with</param>
1248: /// <returns>Returns the reference to the resulting string</returns>
1249: string& string::replace(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount /*= npos*/)
1250: {
1251:     string result = substr(0, pos) + str.substr(strPos, strCount) + substr(pos + count);
1252:     assign(result);
1253:     return *this;
1254: }
1255: 
1256: /// <summary>
1257: /// replace substring
1258: /// 
1259: /// Replaces the substring from pos to pos+count with str
1260: /// </summary>
1261: /// <param name="pos">Starting position of substring to replace</param>
1262: /// <param name="count">Number of characters in substring to replace</param>
1263: /// <param name="str">string to replace with</param>
1264: /// <returns>Returns the reference to the resulting string</returns>
1265: string& string::replace(size_t pos, size_t count, const ValueType* str)
1266: {
1267:     string result = substr(0, pos) + str + substr(pos + count);
1268:     assign(result);
1269:     return *this;
1270: }
1271: 
1272: /// <summary>
1273: /// replace substring
1274: /// 
1275: /// Replaces the substring from pos to pos+count with the first strCount characters of str
1276: /// </summary>
1277: /// <param name="pos">Starting position of substring to replace</param>
1278: /// <param name="count">Number of characters in substring to replace</param>
1279: /// <param name="str">string to replace with</param>
1280: /// <param name="strCount">Number of characters in substring to replace with</param>
1281: /// <returns>Returns the reference to the resulting string</returns>
1282: string& string::replace(size_t pos, size_t count, const ValueType* str, size_t strCount)
1283: {
1284:     string result = substr(0, pos) + string(str, strCount) + substr(pos + count);
1285:     assign(result);
1286:     return *this;
1287: }
1288: 
1289: /// <summary>
1290: /// replace substring
1291: /// 
1292: /// Replaces the substring from pos to pos+count with ch
1293: /// </summary>
1294: /// <param name="pos">Starting position of substring to replace</param>
1295: /// <param name="count">Number of characters in substring to replace</param>
1296: /// <param name="ch">Characters to replace with</param>
1297: /// <returns>Returns the reference to the resulting string</returns>
1298: string& string::replace(size_t pos, size_t count, ValueType ch)
1299: {
1300:     return replace(pos, count, ch, 1);
1301: }
1302: 
1303: /// <summary>
1304: /// replace substring
1305: /// 
1306: /// Replaces the substring from pos to pos+count with a sequence of chCount copies of ch
1307: /// </summary>
1308: /// <param name="pos">Starting position of substring to replace</param>
1309: /// <param name="count">Number of characters in substring to replace</param>
1310: /// <param name="ch">Characters to replace with</param>
1311: /// <param name="chCount">Number of copies of ch to replace with</param>
1312: /// <returns>Returns the reference to the resulting string</returns>
1313: string& string::replace(size_t pos, size_t count, ValueType ch, size_t chCount)
1314: {
1315:     string result = substr(0, pos) + string(chCount, ch) + substr(pos + count);
1316:     assign(result);
1317:     return *this;
1318: }
1319: 
1320: /// <summary>
1321: /// replace substring
1322: /// 
1323: /// Replaces all instances of the substring oldStr (if existing) with newStr
1324: /// </summary>
1325: /// <param name="oldStr">string to find in string</param>
1326: /// <param name="newStr">string to replace with</param>
1327: /// <returns>Returns the number of times the string was replaced</returns>
1328: int string::replace(const string& oldStr, const string& newStr)
1329: {
1330:     size_t pos = find(oldStr);
1331:     size_t oldLength = oldStr.length();
1332:     size_t newLength = newStr.length();
1333:     int count = 0;
1334:     while (pos != npos)
1335:     {
1336:         replace(pos, oldLength, newStr);
1337:         pos += newLength;
1338:         pos = find(oldStr, pos);
1339:         count++;
1340:     }
1341:     return count;
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
1352: int string::replace(const ValueType* oldStr, const ValueType* newStr)
1353: {
1354:     if ((oldStr == nullptr) || (newStr == nullptr))
1355:         return 0;
1356:     size_t pos = find(oldStr);
1357:     size_t oldLength = strlen(oldStr);
1358:     size_t newLength = strlen(newStr);
1359:     int count = 0;
1360:     while (pos != npos)
1361:     {
1362:         replace(pos, oldLength, newStr);
1363:         pos += newLength;
1364:         pos = find(oldStr, pos);
1365:         count++;
1366:     }
1367:     return count;
1368: }
1369: 
1370: /// <summary>
1371: /// Align string
1372: /// 
1373: /// Pads the string on the left (width > 0) or on the right (width < 0) up to a length of width characters. If the string is larger than width characters, it is not modified.
1374: /// </summary>
1375: /// <param name="width">length of target string. If width < 0, the string is padded to the right with spaces up to -width characters.
1376: /// if width > 0, the string is padded to the left with space up to width characters</param>
1377: /// <returns>Returns the number of times the string was replaced</returns>
1378: string string::align(int width) const
1379: {
1380:     string result;
1381:     int    absWidth = (width > 0) ? width : -width;
1382:     auto   len = length();
1383:     if (static_cast<size_t>(absWidth) > len)
1384:     {
1385:         if (width < 0)
1386:         {
1387:             result = *this + string(static_cast<unsigned int>(-width) - len, ' ');
1388:         }
1389:         else
1390:         {
1391:             result = string(static_cast<unsigned int>(width) - len, ' ') + *this;
1392:         }
1393:     }
1394:     else
1395:         result = *this;
1396:     return result;
1397: }
1398: 
1399: /// <summary>
1400: /// Allocate or re-allocate string to have a capacity of requestedLength characters (+1 is added for the terminating null character)
1401: /// </summary>
1402: /// <param name="requestedLength">Amount of characters in the string to allocate space for (excluding the terminating null character)</param>
1403: /// <returns>True if successful, false otherwise</returns>
1404: bool string::reallocate(size_t requestedLength)
1405: {
1406:     auto requestedSize = requestedLength + 1;
1407:     auto allocationSize = NextPowerOf2((requestedSize < MinimumAllocationSize) ? MinimumAllocationSize : requestedSize);
1408: 
1409:     if (!reallocate_allocation_size(allocationSize))
1410:         return false;
1411:     m_end = m_buffer + requestedLength;
1412:     return true;
1413: }
1414: 
1415: /// <summary>
1416: /// Allocate or re-allocate string to have a capacity of allocationSize bytes
1417: /// </summary>
1418: /// <param name="allocationSize">Amount of bytes to allocate space for</param>
1419: /// <returns>True if successful, false otherwise</returns>
1420: bool string::reallocate_allocation_size(size_t allocationSize)
1421: {
1422:     auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
1423:     if (newBuffer == nullptr)
1424:     {
1425:         return false;
1426:     }
1427:     m_buffer = newBuffer;
1428:     m_allocatedSize = allocationSize;
1429:     return true;
1430: }
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

