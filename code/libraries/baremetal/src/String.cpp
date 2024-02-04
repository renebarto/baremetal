//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : String.cpp
//
// Namespace   : baremetal
//
// Class       : String
//
// Description : String handling
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later) and Odroid
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files(the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and /or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------

#include <baremetal/String.h>

#include <baremetal/Assert.h>
#include <baremetal/Util.h>

/// @file
/// String class implementation

using namespace baremetal;

/// @brief Minimum allocation size for any string
static constexpr size_t MinimumAllocationSize = 64;
const size_t String::npos = static_cast<size_t>(-1);
/// @brief Constant null character, using as string terminator, and also returned as a reference for const methods where nothing can be returned
static const String::ValueType NullCharConst = '\0';
/// @brief Non-constant null character, returned as a reference for const methods where nothing can be returned (always reinitialized before returning)
static String::ValueType NullChar = '\0';

/// <summary>
/// Default constructor
/// 
/// Constructs an empty string.
/// </summary>
String::String()
    : m_buffer{}
    , m_end{}
    , m_allocatedSize{}
{
}

/// <summary>
/// Destructor
/// 
/// Frees any allocated memory.
/// </summary>
String::~String()
{
    delete[] m_buffer;
}

/// <summary>
/// Constructor
/// 
/// Initializes the string with the specified string.
/// </summary>
/// <param name="str">String to initialize with</param>
String::String(const ValueType* str)
    : m_buffer{}
    , m_end{}
    , m_allocatedSize{}
{
    if (str == nullptr)
        return;
    auto size = strlen(str);
    if (Reallocate(size))
    {
        strncpy(m_buffer, str, size);
    }
}

/// <summary>
/// Constructor
/// 
/// Initializes the string with up to count characters in the specified string. A null character is always added.
/// </summary>
/// <param name="str">String to initialize with</param>
/// <param name="count">Maximum number of characters from str to initialize with. If count is larger than the actual string length, only the string length is used</param>
String::String(const ValueType* str, size_t count)
    : m_buffer{}
    , m_end{}
    , m_allocatedSize{}
{
    if (str == nullptr)
        return;
    auto size = strlen(str);
    if (count < size)
        size = count;
    if (Reallocate(size))
    {
        strncpy(m_buffer, str, size);
    }
}

/// <summary>
/// Constructor
/// 
/// Initializes the string with the specified count times the specified character. A null character is always added.
/// </summary>
/// <param name="count">Number of characters of value ch to initialized with</param>
/// <param name="ch">Character to initialize with</param>
String::String(size_t count, ValueType ch)
    : m_buffer{}
    , m_end{}
    , m_allocatedSize{}
{
    auto size = count;
    if (Reallocate(size))
    {
        memset(m_buffer, ch, size);
    }
}

/// <summary>
/// Copy constructor
/// 
/// Initializes the string with the specified string value.
/// </summary>
/// <param name="other">String to initialize with</param>
String::String(const String& other)
    : m_buffer{}
    , m_end{}
    , m_allocatedSize{}
{
    auto size = other.Length();
    if (Reallocate(size))
    {
        strncpy(m_buffer, other.Data(), size);
    }
}

/// <summary>
/// Constructor
/// 
/// Initializes the string with the substring starting at specified position, for the specified number of characters, from the specified string value.
/// </summary>
/// <param name="other">String to initialize with</param>
/// <param name="pos">Position in other to start copying charaters from</param>
/// <param name="count">Maximum number of characters to copy from other. Default is until end of string. If pos + count is larger than the actual length of the string, string string is copied until the end</param>
String::String(const String& other, size_t pos, size_t count /*= npos*/)
    : m_buffer{}
    , m_end{}
    , m_allocatedSize{}
{
    auto size = other.Length() - pos;
    if (count < size)
        size = count;
    if (Reallocate(size))
    {
        strncpy(m_buffer, other.Data() + pos, size);
    }
}

/// <summary>
/// Const character cast operator
/// 
/// Returns the pointer to the start of the string.
/// </summary>
String::operator const ValueType* () const
{
    return Data();
}

/// <summary>
/// Assignment operator
/// 
/// Assigns the specified string value to the string.
/// </summary>
/// <param name="str">String value to assign to the string</param>
/// <returns>A reference to the string</returns>
String& String::operator = (const ValueType* str)
{
    return Assign(str);
}

/// <summary>
/// Assignment operator
/// 
/// Assigns the specified string value to the string.
/// </summary>
/// <param name="str">String value to assign to the string</param>
/// <returns>A reference to the string</returns>
String& String::operator = (const String& str)
{
    return Assign(str);
}

/// <summary>
/// Iterator to the start of the string
/// 
/// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : String).
/// </summary>
/// <returns>ConstIterator to the value type, acting as the start of the string</returns>
ConstIterator<String::ValueType> String::begin() const
{
    return ConstIterator(m_buffer, m_end);
}

/// <summary>
/// Iterator to the end of the string + 1
/// 
/// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : String).
/// </summary>
/// <returns>ConstIterator to the value type, acting as the end of the string</returns>
ConstIterator<String::ValueType> String::end() const
{
    return ConstIterator(m_end, m_end);
}

/// <summary>
/// Assign a string value
/// 
/// Assigns the specified string value to the string
/// </summary>
/// <param name="str">String value to assign to the string</param>
/// <returns>A reference to the string</returns>
String& String::Assign(const ValueType* str)
{
    size_t size{};
    if (str != nullptr)
    {
        size = strlen(str);
    }
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return *this;
    }
    strncpy(m_buffer, str, size);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
    return *this;
}

/// <summary>
/// Assign a string value
/// 
/// Assigns the specified string value, up to the specified count of characters, to the string.
/// </summary>
/// <param name="str">String value to assign to the string</param>
/// <param name="count">Maximum number of characters to copy from the string. If count is larger than the string length, the length of the string is used</param>
/// <returns>A reference to the string</returns>
String& String::Assign(const ValueType* str, size_t count)
{
    size_t size{};
    if (str != nullptr)
    {
        size = strlen(str);
    }
    if (count < size)
        size = count;
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return *this;
    }
    strncpy(m_buffer, str, size);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
    return *this;
}

/// <summary>
/// Assign a string value
/// 
/// Assigns a string containing the specified count times the specified characters to the string
/// </summary>
/// <param name="count">Number copies of ch to copy to the string</param>
/// <param name="ch">Character to initialize with</param>
/// <returns>A reference to the string</returns>
String& String::Assign(size_t count, ValueType ch)
{
    auto size = count;
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return *this;
    }
    memset(m_buffer, ch, size);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
    return *this;
}

/// <summary>
/// Assign a string value
/// 
/// Assigns the specified string value to the string
/// </summary>
/// <param name="str">String value to assign to the string</param>
/// <returns>A reference to the string</returns>
String& String::Assign(const String& str)
{
    auto size = str.Length();
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return *this;
    }
    strncpy(m_buffer, str.Data(), size);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
    return *this;
}

/// <summary>
/// Assign a string value
/// 
/// Assigns the substring start from the specified position for the specified count of characters of specified string value to the string
/// </summary>
/// <param name="str">String value to assign to the string</param>
/// <param name="pos">Starting position of substring to copy from str</param>
/// <param name="count">Maximum number of characters to copy from str.
/// Default is until end of string. If pos + count is larger than the stirn length, characters are copied until end of string</param>
/// <returns>A reference to the string</returns>
String& String::Assign(const String& str, size_t pos, size_t count /*= npos*/)
{
    auto size = str.Length() - pos;
    if (count < size)
        size = count;
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return *this;
    }
    strncpy(m_buffer, str.Data() + pos, size);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
    return *this;
}

/// <summary>
/// Return the character at specified position
/// </summary>
/// <param name="pos">Position in string</param>
/// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
String::ValueType& String::At(size_t pos)
{
    if (pos >= Length())
    {
        NullChar = NullCharConst;
        return NullChar;
    }
    return m_buffer[pos];
}

/// <summary>
/// Return the character at specified position
/// </summary>
/// <param name="pos">Position in string</param>
/// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, a reference to a const null character is returned (NullCharConst)</returns>
const String::ValueType& String::At(size_t pos) const
{
    if (pos >= Length())
        return NullCharConst;
    return m_buffer[pos];
}

/// <summary>
/// Return the first character
/// </summary>
/// <returns>Returns a non-const reference to the first character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
String::ValueType& String::Front()
{
    if (Empty())
    {
        NullChar = NullCharConst;
        return NullChar;
    }
    return *m_buffer;
}

/// <summary>
/// Return the first character
/// </summary>
/// <returns>Returns a const reference to the first character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
const String::ValueType& String::Front() const
{
    if (Empty())
        return NullCharConst;
    return *m_buffer;
}

/// <summary>
/// Return the last character
/// </summary>
/// <returns>Returns a non-const reference to the last character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before returning)</returns>
String::ValueType& String::Back()
{
    if (Empty())
    {
        NullChar = NullCharConst;
        return NullChar;
    }
    return *(m_end - 1);
}

/// <summary>
/// Return the last character
/// </summary>
/// <returns>Returns a const reference to the last character in the string. If the string is empty, a reference to a const null character is returned (NullCharConst)</returns>
const String::ValueType& String::Back() const
{
    if (Empty())
        return NullCharConst;
    return *(m_end - 1);
}

/// <summary>
/// Return the character at specified position
/// </summary>
/// <param name="pos">Position in string</param>
/// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
String::ValueType& String::operator[] (size_t pos)
{
    return m_buffer[pos];
}

/// <summary>
/// Return the character at specified position
/// </summary>
/// <param name="pos">Position in string</param>
/// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
const String::ValueType& String::operator[] (size_t pos) const
{
    return m_buffer[pos];
}

/// <summary>
/// Return the buffer pointer
/// </summary>
/// <returns>Returns a non-const pointer to the buffer. If the buffer is not allocated, a pointer to a non-const null character (NullChar, initialized before returning) is returned</returns>
String::ValueType* String::Data()
{
    NullChar = NullCharConst;
    return (m_buffer == nullptr) ? &NullChar : m_buffer;
}

/// <summary>
/// Return the buffer pointer
/// </summary>
/// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
const String::ValueType* String::Data() const
{
    return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
}

/// <summary>
/// Determine whether string is empty.
/// </summary>
/// <returns>Returns true when the string is empty (not allocated or no contents), false otherwise</returns>
bool String::Empty() const
{
    return m_end == m_buffer;
}

/// <summary>
/// Return the size of the string
/// 
/// This method is the equivalent of Length().
/// </summary>
/// <returns>Returns the size (or length) of the string</returns>
size_t String::Size() const
{
    return m_end - m_buffer;
}

/// <summary>
/// Return the length of the string
/// 
/// This method is the equivalent of Size().
/// </summary>
/// <returns>Returns the size (or length) of the string</returns>
size_t String::Length() const
{
    return m_end - m_buffer;
}

/// <summary>
/// Return the capacity of the string
/// 
/// The capacity is the size of the allocated buffer. The string can grow to that length before it needs to be re-allocated.
/// </summary>
/// <returns>Returns the size (or length) of the string</returns>
size_t String::Capacity() const
{
    return m_allocatedSize;
}

/// <summary>
/// Reserved a buffer capacity
/// 
/// Allocates a buffer of specified size
/// </summary>
/// <param name="newCapacity"></param>
/// <returns>Returns the capacity of the string</returns>
size_t String::Reserve(size_t newCapacity)
{
    ReallocateAllocationSize(newCapacity);
    return m_allocatedSize;
}

/// <summary>
/// Append operator
/// 
/// Appends a character to the string
/// </summary>
/// <param name="ch">Character to append</param>
/// <returns>Returns a reference to the string</returns>
String& String::operator +=(ValueType ch)
{
    Append(1, ch);
    return *this;
}

/// <summary>
/// Append operator
/// 
/// Appends a string to the string
/// </summary>
/// <param name="str">String to append</param>
/// <returns>Returns a reference to the string</returns>
String& String::operator +=(const String& str)
{
    Append(str);
    return *this;
}

/// <summary>
/// Append operator
/// 
/// Appends a string to the string
/// </summary>
/// <param name="str">String to append. If nullptr the nothing is appended</param>
/// <returns>Returns a reference to the string</returns>
String& String::operator +=(const ValueType* str)
{
    Append(str);
    return *this;
}

/// <summary>
/// Append operator
/// 
/// Appends a sequence of count times the same character ch to the string
/// </summary>
/// <param name="count">Number of characters to append</param>
/// <param name="ch">Character to append</param>
void String::Append(size_t count, ValueType ch)
{
    auto length = Length();
    auto size = length + count;
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return;
    }
    memset(m_buffer + length, ch, count);
    m_end += count;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// Append operator
/// 
/// Appends a string to the string
/// </summary>
/// <param name="str">String to append</param>
void String::Append(const String& str)
{
    auto length = Length();
    auto strLength = str.Length();
    auto size = length + strLength;
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return;
    }
    strncpy(m_buffer + length, str.Data(), strLength);
    m_end += strLength;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// Append operator
/// 
/// Appends a substring of str to the string
/// </summary>
/// <param name="str">String to append from</param>
/// <param name="pos">Start position in str to copy characters from</param>
/// <param name="count">Number of characters to copy from str. Default is until the end of the string. If count is larger than the string length, characters are copied up to the end of the string</param>
void String::Append(const String& str, size_t pos, size_t count /*= npos*/)
{
    auto length = Length();
    auto strLength = str.Length();
    auto strCount = count;
    if (pos >= strLength)
        return;
    if (pos + strCount > strLength)
        strCount = strLength - pos;
    auto size = length + strCount;
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return;
    }
    strncpy(m_buffer + length, str.Data() + pos, strCount);
    m_end += strCount;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// Append operator
/// 
/// Appends a string to the string
/// </summary>
/// <param name="str">String to append. If nullptr the nothing is appended</param>
void String::Append(const ValueType* str)
{
    if (str == nullptr)
        return;
    auto length = Length();
    auto strLength = strlen(str);
    auto size = length + strLength;
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return;
    }
    strncpy(m_buffer + length, str, strLength);
    m_end += strLength;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// Append operator
/// 
/// Appends a number of characters from str to the string
/// </summary>
/// <param name="str">String to append. If nullptr the nothing is appended</param>
/// <param name="count">Number of characters to copy from str. If count is larger than the string length, the complete string is copied</param>
void String::Append(const ValueType* str, size_t count)
{
    if (str == nullptr)
        return;
    auto length = Length();
    auto strLength = strlen(str);
    auto strCount = count;
    if (strCount > strLength)
        strCount = strLength;
    auto size = length + strCount;
    if (size > m_allocatedSize)
    {
        if (!Reallocate(size))
            return;
    }
    strncpy(m_buffer + length, str, strCount);
    m_end += strCount;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// Clear the string
/// 
/// Clears the contents of the string, but does not free or reallocate the buffer
/// </summary>
void String::Clear()
{
    if (!Empty())
    {
        m_end = m_buffer;
        m_buffer[0] = NullCharConst;
    }
}

/// <summary>
/// Find a substring in the string
/// 
/// If empty string, always finds the string.
/// </summary>
/// <param name="str">Substring to find</param>
/// <param name="pos">Starting position in string to start searching</param>
/// <returns>Location of first character in string of match if found, String::npos if not found</returns>
size_t String::Find(const String& str, size_t pos /*= 0*/) const
{
    auto length = Length();
    auto patternLength = str.Length();
    if (pos >= length)
        return npos;
    auto needle = str.Data();
    for (const ValueType* haystack = Data() + pos; haystack <= m_end - patternLength; ++haystack)
    {
        if (memcmp(haystack, needle, patternLength) == 0)
            return haystack - m_buffer;
    }
    return npos;
}

/// <summary>
/// Find a substring in the string
/// 
/// If nullptr or empty string, always finds the string.
/// </summary>
/// <param name="str">Substring to find</param>
/// <param name="pos">Starting position in string to start searching</param>
/// <returns>Location of first character in string of match if found, String::npos if not found</returns>
size_t String::Find(const ValueType* str, size_t pos /*= 0*/) const
{
    size_t strLength{};
    if (str != nullptr)
    {
        strLength = strlen(str);
    }
    auto length = Length();
    auto patternLength = strLength;
    if (pos >= length)
        return npos;
    auto needle = str;
    for (const ValueType* haystack = Data() + pos; haystack <= m_end - patternLength; ++haystack)
    {
        if (memcmp(haystack, needle, patternLength) == 0)
            return haystack - m_buffer;
    }
    return npos;
}

/// <summary>
/// Find a substring in the string
/// 
/// If nullptr or empty string, always finds the string.
/// </summary>
/// <param name="str">Substring to find</param>
/// <param name="pos">Starting position in string to start searching</param>
/// <param name="count">Number of characters from str to compare</param>
/// <returns>Location of first character in string of match if found, String::npos if not found</returns>
size_t String::Find(const ValueType* str, size_t pos, size_t count) const
{
    size_t strLength{};
    if (str != nullptr)
    {
        strLength = strlen(str);
    }
    auto length = Length();
    auto patternLength = strLength;
    if (pos >= length)
        return npos;
    if (count < patternLength)
        patternLength = count;
    auto needle = str;
    for (const ValueType* haystack = Data() + pos; haystack <= m_end - patternLength; ++haystack)
    {
        if (memcmp(haystack, needle, patternLength) == 0)
            return haystack - m_buffer;
    }
    return npos;
}

/// <summary>
/// Find a character in the string
/// </summary>
/// <param name="ch">Character to find</param>
/// <param name="pos">Starting position in string to start searching</param>
/// <returns>Location of first character in string of match if found, String::npos if not found</returns>
size_t String::Find(ValueType ch, size_t pos /*= 0*/) const
{
    auto length = Length();
    if (pos >= length)
        return npos;
    for (const ValueType* haystack = Data() + pos; haystack <= m_end; ++haystack)
    {
        if (*haystack == ch)
            return haystack - m_buffer;
    }
    return npos;
}

/// <summary>
/// Check whether string starts with character
/// </summary>
/// <param name="ch">Character to find</param>
/// <returns>Returns true if ch is first character in string, false otherwise</returns>
bool String::StartsWith(ValueType ch) const
{
    if (Empty())
        return false;
    return m_buffer[0] == ch;
}

/// <summary>
/// Check whether string starts with substring
/// </summary>
/// <param name="str">SubString to find</param>
/// <returns>Returns true if str is first part of string, false otherwise</returns>
bool String::StartsWith(const String& str) const
{
    auto length = Length();
    auto strLength = str.Length();
    if (strLength >= length)
        return false;

    return memcmp(Data(), str.Data(), strLength) == 0;
}

/// <summary>
/// Check whether string starts with substring
/// </summary>
/// <param name="str">SubString to find</param>
/// <returns>Returns true if str is first part of string, false otherwise</returns>
bool String::StartsWith(const ValueType* str) const
{
    size_t strLength{};
    if (str != nullptr)
    {
        strLength = strlen(str);
    }
    auto length = Length();
    if (strLength >= length)
        return false;

    return memcmp(Data(), str, strLength) == 0;
}

/// <summary>
/// Check whether string ends with character
/// </summary>
/// <param name="ch">Character to find</param>
/// <returns>Returns true if ch is last character in string, false otherwise</returns>
bool String::EndsWith(ValueType ch) const
{
    if (Empty())
        return false;
    return m_buffer[Length() - 1] == ch;
}

/// <summary>
/// Check whether string ends with substring
/// </summary>
/// <param name="str">SubString to find</param>
/// <returns>Returns true if str is last part of string, false otherwise</returns>
bool String::EndsWith(const String& str) const
{
    auto length = Length();
    auto strLength = str.Length();
    if (strLength >= length)
        return false;

    return memcmp(m_end - strLength, str.Data(), strLength) == 0;
}

/// <summary>
/// Check whether string ends with substring
/// </summary>
/// <param name="str">SubString to find</param>
/// <returns>Returns true if str is last part of string, false otherwise</returns>
bool String::EndsWith(const ValueType* str) const
{
    size_t strLength{};
    if (str != nullptr)
    {
        strLength = strlen(str);
    }
    auto length = Length();
    if (strLength >= length)
        return false;

    return memcmp(m_end - strLength, str, strLength) == 0;
}

/// <summary>
/// Check whether string contains character
/// </summary>
/// <param name="ch">Character to find</param>
/// <returns>Returns true if ch is contained in string, false otherwise</returns>
bool String::Contains(ValueType ch) const
{
    return Find(ch) != npos;
}

/// <summary>
/// Check whether string contains substring
/// </summary>
/// <param name="str">Substring to find</param>
/// <returns>Returns true if ch is contained in string, false otherwise</returns>
bool String::Contains(const String& str) const
{
    return Find(str) != npos;
}

/// <summary>
/// Check whether string contains substring
/// </summary>
/// <param name="str">Substring to find</param>
/// <returns>Returns true if ch is contained in string, false otherwise</returns>
bool String::Contains(const ValueType* str) const
{
    return Find(str) != npos;
}

/// <summary>
/// Return substring
/// </summary>
/// <param name="pos">Starting position of substring in string</param>
/// <param name="count">Length of substring to return. If count is larger than the number of characters available from position pos, the rest of the string is returned</param>
/// <returns>Returns the substring at position [pos, pos + count), if available </returns>
String String::SubStr(size_t pos /*= 0*/, size_t count /*= npos*/) const
{
    String result;
    auto length = Length();
    auto size = count;
    if (pos < length)
    {
        if (count > length - pos)
            count = length - pos;
        result.Reallocate(count);
        memcpy(result.Data(), Data() + pos, count);
        result.Data()[count] = NullCharConst;
    }

    return result;
}

/// <summary>
/// Case sensitive equality to string
/// </summary>
/// <param name="other">String to compare to</param>
/// <returns>Returns true if the strings are equal, false otherwise</returns>
bool String::Equals(const String& other) const
{
    return Compare(other) == 0;
}

/// <summary>
/// Case sensitive equality to string
/// </summary>
/// <param name="other">String to compare to</param>
/// <returns>Returns true if the strings are equal, false otherwise</returns>
bool String::Equals(const ValueType* other) const
{
    if (other == nullptr)
        return Empty();
    if (Length() != strlen(other))
        return false;
    return strcmp(Data(), other) == 0;
}

/// <summary>
/// Case insensitive equality to string
/// </summary>
/// <param name="other">String to compare to</param>
/// <returns>Returns true if the strings are equal, false otherwise</returns>
bool String::EqualsCaseInsensitive(const String& other) const
{
    if (Length() != other.Length())
        return false;
    if (Empty())
        return true;
    return strcasecmp(Data(), other.Data()) == 0;
}

/// <summary>
/// Case insensitive equality to string
/// </summary>
/// <param name="other">String to compare to</param>
/// <returns>Returns true if the strings are equal, false otherwise</returns>
bool String::EqualsCaseInsensitive(const ValueType* other) const
{
    if (other == nullptr)
        return Empty();
    if (Length() != strlen(other))
        return false;
    return strcasecmp(Data(), other) == 0;
}

/// <summary>
/// Case sensitive compare to string
/// 
/// Compares the complete string, character by character
/// </summary>
/// <param name="str">String to compare to</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::Compare(const String& str) const
{
    if (Empty())
    {
        if (str.Empty())
            return 0;
        return -1;
    }
    if (str.Empty())
        return 1;

    return strcmp(Data(), str.Data());
}

/// <summary>
/// Case sensitive compare to string
/// 
/// Compares the substring from pos to pos+count to str
/// </summary>
/// <param name="pos">Starting position of substring to compare to str</param>
/// <param name="count">Number of characters in substring to compare to str</param>
/// <param name="str">String to compare to</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::Compare(size_t pos, size_t count, const String& str) const
{
    return SubStr(pos, count).Compare(str);
}

/// <summary>
/// Case sensitive compare to string
/// 
/// Compares the substring from pos to pos+count to the substring from strPos to strPos+strCount of str
/// </summary>
/// <param name="pos">Starting position of substring to compare to str</param>
/// <param name="count">Number of characters in substring to compare to str</param>
/// <param name="str">String to compare to</param>
/// <param name="strPos">Starting position of substring of str to compare</param>
/// <param name="strCount">Number of characters in substring of str to compare</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::Compare(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount /*= npos*/) const
{
    return SubStr(pos, count).Compare(str.SubStr(strPos, strCount));
}

/// <summary>
/// Case sensitive compare to string
/// 
/// Compares the complete string to str
/// </summary>
/// <param name="str">String to compare to</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::Compare(const ValueType* str) const
{
    size_t strLength{};
    if (str != nullptr)
        strLength = strlen(str);
    if (Empty())
    {
        if (strLength == 0)
            return 0;
        return -1;
    }
    if (strLength == 0)
        return 1;

    return strcmp(Data(), str);
}

/// <summary>
/// Case sensitive compare to string
/// 
/// Compares the substring from pos to pos+count to str
/// </summary>
/// <param name="pos">Starting position of substring to compare to str</param>
/// <param name="count">Number of characters in substring to compare to str</param>
/// <param name="str">String to compare to</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::Compare(size_t pos, size_t count, const ValueType* str) const
{
    size_t strLength{};
    if (str != nullptr)
        strLength = strlen(str);

    auto length = Length();
    if (pos >= length)
        length = 0;
    length = length - pos;
    if (count < length)
        length = count;
    if (length == 0)
    {
        if (strLength == 0)
            return 0;
        return -1;
    }
    if (strLength == 0)
        return 1;

    auto maxLen = strLength;
    if (maxLen < length)
        maxLen = length;
    return strncmp(Data() + pos, str, maxLen);
}

/// <summary>
/// Case sensitive compare to string
/// 
/// Compares the substring from pos to pos+count to the first strCount characters of str
/// </summary>
/// <param name="pos">Starting position of substring to compare to str</param>
/// <param name="count">Number of characters in substring to compare to str</param>
/// <param name="str">String to compare to</param>
/// <param name="strCount">Number of characters in substring of str to compare</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::Compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const
{
    size_t strLength{};
    if (str != nullptr)
        strLength = strlen(str);

    auto length = Length();
    if (pos >= length)
        length = 0;
    length = length - pos;
    if (count < length)
        length = count;

    if (strCount < strLength)
        strLength = strCount;

    if (length == 0)
    {
        if (strLength == 0)
            return 0;
        return -1;
    }
    if (strLength == 0)
        return 1;

    auto maxLen = strLength;
    if (maxLen < length)
        maxLen = length;
    return strncmp(Data() + pos, str, maxLen);
}

/// <summary>
/// Replace substring
/// 
/// Replaces the substring from pos to pos+count with str
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="str">String to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::Replace(size_t pos, size_t count, const String& str)
{
    String result = SubStr(0, pos) + str + SubStr(pos + count);
    Assign(result);
    return *this;
}

/// <summary>
/// Replace substring
/// 
/// Replaces the substring from pos to pos+count with the substring from strPos to strPos+strCount of str
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="str">String to replace with</param>
/// <param name="strPos">Starting position of substring of str to replace with</param>
/// <param name="strCount">Number of characters in substring of str to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::Replace(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount /*= npos*/)
{
    String result = SubStr(0, pos) + str.SubStr(strPos, strCount) + SubStr(pos + count);
    Assign(result);
    return *this;
}

/// <summary>
/// Replace substring
/// 
/// Replaces the substring from pos to pos+count with str
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="str">String to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::Replace(size_t pos, size_t count, const ValueType* str)
{
    String result = SubStr(0, pos) + str + SubStr(pos + count);
    Assign(result);
    return *this;
}

/// <summary>
/// Replace substring
/// 
/// Replaces the substring from pos to pos+count with the first strCount characters of str
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="str">String to replace with</param>
/// <param name="strCount">Number of characters in substring to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::Replace(size_t pos, size_t count, const ValueType* str, size_t strCount)
{
    String result = SubStr(0, pos) + String(str, strCount) + SubStr(pos + count);
    Assign(result);
    return *this;
}

/// <summary>
/// Replace substring
/// 
/// Replaces the substring from pos to pos+count with ch
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="ch">Characters to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::Replace(size_t pos, size_t count, ValueType ch)
{
    return Replace(pos, count, ch, 1);
}

/// <summary>
/// Replace substring
/// 
/// Replaces the substring from pos to pos+count with a sequence of chCount copies of ch
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="ch">Characters to replace with</param>
/// <param name="chCount">Number of copies of ch to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::Replace(size_t pos, size_t count, ValueType ch, size_t chCount)
{
    String result = SubStr(0, pos) + String(chCount, ch) + SubStr(pos + count);
    Assign(result);
    return *this;
}

/// <summary>
/// Replace substring
/// 
/// Replaces all instances of the substring oldStr (if existing) with newStr
/// </summary>
/// <param name="oldStr">String to find in string</param>
/// <param name="newStr">String to replace with</param>
/// <returns>Returns the number of times the string was replaced</returns>
int String::Replace(const String& oldStr, const String& newStr)
{
    size_t pos = Find(oldStr);
    size_t oldLength = oldStr.Length();
    size_t newLength = newStr.Length();
    int count = 0;
    while (pos != npos)
    {
        Replace(pos, oldLength, newStr);
        pos += newLength;
        pos = Find(oldStr, pos);
        count++;
    }
    return count;
}

/// <summary>
/// Replace substring
/// 
/// Replaces all instances of the substring oldStr (if existing) with newStr
/// </summary>
/// <param name="oldStr">String to find in string</param>
/// <param name="newStr">String to replace with</param>
/// <returns>Returns the number of times the string was replaced</returns>
int String::Replace(const ValueType* oldStr, const ValueType* newStr)
{
    if ((oldStr == nullptr) || (newStr == nullptr))
        return 0;
    size_t pos = Find(oldStr);
    size_t oldLength = strlen(oldStr);
    size_t newLength = strlen(newStr);
    int count = 0;
    while (pos != npos)
    {
        Replace(pos, oldLength, newStr);
        pos += newLength;
        pos = Find(oldStr, pos);
        count++;
    }
    return count;
}

/// <summary>
/// Align string
/// 
/// Pads the string on the left (width > 0) or on the right (width < 0) up to a length of width characters. If the string is larger than width characters, it is not modified.
/// </summary>
/// <param name="width">Length of target string. If width < 0, the string is padded to the right with spaces up to -width characters.
/// if width > 0, the string is padded to the left with space up to width characters</param>
/// <returns>Returns the number of times the string was replaced</returns>
String String::Align(int width) const
{
    String result;
    int    absWidth = (width > 0) ? width : -width;
    auto   length = Length();
    if (static_cast<size_t>(absWidth) > length)
    {
        if (width < 0)
        {
            result = *this + String(static_cast<unsigned int>(-width) - length, ' ');
        }
        else
        {
            result = String(static_cast<unsigned int>(width) - length, ' ') + *this;
        }
    }
    else
        result = *this;
    return result;
}

/// <summary>
/// Allocate or re-allocate string to have a capacity of requestedLength characters (+1 is added for the terminating null character)
/// </summary>
/// <param name="requestedLength">Amount of characters in the string to allocate space for (excluding the terminating null character)</param>
/// <returns>True if successful, false otherwise</returns>
bool String::Reallocate(size_t requestedLength)
{
    auto requestedSize = requestedLength + 1;
    auto allocationSize = NextPowerOf2((requestedSize < MinimumAllocationSize) ? MinimumAllocationSize : requestedSize);

    if (!ReallocateAllocationSize(allocationSize))
        return false;
    m_end = m_buffer + requestedLength;
    return true;
}

/// <summary>
/// Allocate or re-allocate string to have a capacity of allocationSize bytes
/// </summary>
/// <param name="allocationSize">Amount of bytes to allocate space for</param>
/// <returns>True if successful, false otherwise</returns>
bool String::ReallocateAllocationSize(size_t allocationSize)
{
    auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
    if (newBuffer == nullptr)
    {
        return false;
    }
    m_buffer = newBuffer;
    m_allocatedSize = allocationSize;
    return true;
}
