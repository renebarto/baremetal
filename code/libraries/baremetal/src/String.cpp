//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
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
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or later)
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

#include "baremetal/String.h"

#include "baremetal/Assert.h"
#include "baremetal/Logger.h"
#include "baremetal/Malloc.h"
#include "stdlib/Util.h"

/// @file
/// String class implementation

using namespace baremetal;

/// @brief Minimum allocation size for any string
static constexpr size_t MinimumAllocationSize = 256;

/// @brief Maximum string size (largest 256Mb - 1 due to largest heap allocation block size)
static constexpr size_t MaximumStringSize = 0x80000 - 1;

const size_t String::npos = static_cast<size_t>(-1);
/// @brief Constant null character, using as string terminator, and also returned as a reference for const methods where nothing can be returned
static const String::ValueType NullCharConst = '\0';
/// @brief Non-constant null character, returned as a reference for const methods where nothing can be returned (always reinitialized before returning)
static String::ValueType NullChar = '\0';

/// @brief Define log name
LOG_MODULE("String");

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
#if BAREMETAL_MEMORY_TRACING_DETAIL
    if (m_buffer != nullptr)
        LOG_NO_ALLOC_DEBUG("Free string %p", m_buffer);
#endif
    delete[] m_buffer;
}

/// <summary>
/// Constructor
///
/// Initializes the string with the specified string.
/// </summary>
/// <param name="str">string to initialize with</param>
String::String(const ValueType* str)
    : m_buffer{}
    , m_end{}
    , m_allocatedSize{}
{
    if (str == nullptr)
        return;
    auto size = strlen(str);
    if (reallocate(size + 1))
    {
        strncpy(m_buffer, str, size);
    }
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// Constructor
///
/// Initializes the string with up to count characters in the specified string. A null character is always added.
/// </summary>
/// <param name="str">string to initialize with</param>
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
    if (reallocate(size + 1))
    {
        strncpy(m_buffer, str, size);
    }
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
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
    if (size > MaximumStringSize)
        size = MaximumStringSize;
    if (reallocate(size + 1))
    {
        memset(m_buffer, ch, size);
    }
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// Copy constructor
///
/// Initializes the string with the specified string value.
/// </summary>
/// <param name="other">string to initialize with</param>
String::String(const String& other)
    : m_buffer{}
    , m_end{}
    , m_allocatedSize{}
{
    auto size = other.length();
    if (reallocate(size + 1))
    {
        strncpy(m_buffer, other.data(), size);
    }
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// Move constructor
///
/// Initializes the string by moving the contents from the specified string value.
/// </summary>
/// <param name="other">string to initialize with</param>
String::String(String&& other)
    : m_buffer{other.m_buffer}
    , m_end{other.m_end}
    , m_allocatedSize{other.m_allocatedSize}
{
    other.m_buffer = nullptr;
    other.m_end = nullptr;
    other.m_allocatedSize = 0;
}

/// <summary>
/// Constructor
///
/// Initializes the string with the substring starting at specified position, for the specified number of characters, from the specified string value.
/// </summary>
/// <param name="other">string to initialize with</param>
/// <param name="pos">Position in other to start copying charaters from</param>
/// <param name="count">Maximum number of characters to copy from other. Default is until end of string. If pos + count is larger than the actual length of the string, string other is copied until the
/// end</param>
String::String(const String& other, size_t pos, size_t count /*= npos*/)
    : m_buffer{}
    , m_end{}
    , m_allocatedSize{}
{
    if (pos >= other.length())
        return;
    auto size = other.length() - pos;
    if (count < size)
        size = count;
    if (reallocate(size + 1))
    {
        strncpy(m_buffer, other.data() + pos, size);
    }
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// Const character cast operator
///
/// Returns the pointer to the start of the string.
/// </summary>
String::operator const ValueType*() const
{
    return data();
}

/// <summary>
/// Assignment operator
///
/// Assigns the specified string value to the string.
/// </summary>
/// <param name="str">string value to assign to the string</param>
/// <returns>A reference to the string</returns>
String& String::operator=(const ValueType* str)
{
    return assign(str);
}

/// <summary>
/// Assignment operator
///
/// Assigns the specified string value to the string.
/// </summary>
/// <param name="str">string value to assign to the string</param>
/// <returns>A reference to the string</returns>
String& String::operator=(const String& str)
{
    return assign(str);
}

/// <summary>
/// Move operator
///
/// Assigns the specified string value to the string by moving the contents of the specified string.
/// </summary>
/// <param name="str">string value to assign to the string</param>
/// <returns>A reference to the string</returns>
String& String::operator=(String&& str)
{
    if (&str != this)
    {
        m_buffer = str.m_buffer;
        m_end = str.m_end;
        m_allocatedSize = str.m_allocatedSize;
        str.m_buffer = nullptr;
        str.m_end = nullptr;
        str.m_allocatedSize = 0;
    }
    return *this;
}

/// <summary>
/// Non-const iterator to the start of the string
///
/// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
/// </summary>
/// <returns>iterator to the value type, acting as the start of the string</returns>
iterator<String::ValueType> String::begin()
{
    return iterator(m_buffer, m_end);
}

/// <summary>
/// Non-const iterator to the end of the string + 1
///
/// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
/// </summary>
/// <returns>iterator to the value type, acting as the end of the string</returns>
iterator<String::ValueType> String::end()
{
    return iterator(m_end, m_end);
}

/// <summary>
/// Const iterator to the start of the string
///
/// Iterator is initialized with the start of the string. This has the prototype needed to used an iterator in for (auto x : string).
/// </summary>
/// <returns>const_iterator to the value type, acting as the start of the string</returns>
const_iterator<String::ValueType> String::begin() const
{
    return const_iterator(m_buffer, m_end);
}

/// <summary>
/// Const iterator to the end of the string + 1
///
/// Iterator is initialized with one position beyound the end of the string. This has the prototype needed to used an iterator in for (auto x : string).
/// </summary>
/// <returns>const_iterator to the value type, acting as the end of the string</returns>
const_iterator<String::ValueType> String::end() const
{
    return const_iterator(m_end, m_end);
}

/// <summary>
/// assign a string value
///
/// Assigns the specified string value to the string
/// </summary>
/// <param name="str">string value to assign to the string</param>
/// <returns>A reference to the string</returns>
String& String::assign(const ValueType* str)
{
    size_t size{};
    if (str != nullptr)
    {
        size = strlen(str);
    }
    if ((size + 1) > m_allocatedSize)
    {
        if (!reallocate(size + 1))
            return *this;
    }
    if (str != nullptr)
    {
        strncpy(m_buffer, str, size);
    }
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
    return *this;
}

/// <summary>
/// assign a string value
///
/// Assigns the specified string value, up to the specified count of characters, to the string.
/// </summary>
/// <param name="str">string value to assign to the string</param>
/// <param name="count">Maximum number of characters to copy from the string. If count is larger than the string length, the length of the string is used</param>
/// <returns>A reference to the string</returns>
String& String::assign(const ValueType* str, size_t count)
{
    size_t size{};
    if (str != nullptr)
    {
        size = strlen(str);
    }
    if (count < size)
        size = count;
    if ((size + 1) > m_allocatedSize)
    {
        if (!reallocate(size + 1))
            return *this;
    }
    if (str != nullptr)
    {
        strncpy(m_buffer, str, size);
    }
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
    return *this;
}

/// <summary>
/// assign a string value
///
/// Assigns a string containing the specified count times the specified characters to the string
/// </summary>
/// <param name="count">Number copies of ch to copy to the string</param>
/// <param name="ch">Character to initialize with</param>
/// <returns>A reference to the string</returns>
String& String::assign(size_t count, ValueType ch)
{
    auto size = count;
    if (size > MaximumStringSize)
        size = MaximumStringSize;
    if ((size + 1) > m_allocatedSize)
    {
        if (!reallocate(size + 1))
            return *this;
    }
    memset(m_buffer, ch, size);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
    return *this;
}

/// <summary>
/// assign a string value
///
/// Assigns the specified string value to the string
/// </summary>
/// <param name="str">string value to assign to the string</param>
/// <returns>A reference to the string</returns>
String& String::assign(const String& str)
{
    auto size = str.length();
    if ((size + 1) > m_allocatedSize)
    {
        if (!reallocate(size + 1))
            return *this;
    }
    strncpy(m_buffer, str.data(), size);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
    return *this;
}

/// <summary>
/// assign a string value
///
/// Assigns the substring start from the specified position for the specified count of characters of specified string value to the string
/// </summary>
/// <param name="str">string value to assign to the string</param>
/// <param name="pos">Starting position of substring to copy from str</param>
/// <param name="count">Maximum number of characters to copy from str.
/// Default is until end of string. If pos + count is larger than the stirn length, characters are copied until end of string</param>
/// <returns>A reference to the string</returns>
String& String::assign(const String& str, size_t pos, size_t count /*= npos*/)
{
    if (str.empty())
        return assign(str);

    if (pos < str.length())
    {
        auto size = str.length() - pos;
        if (count < size)
            size = count;
        if ((size + 1) > m_allocatedSize)
        {
            if (!reallocate(size + 1))
                return *this;
        }
        strncpy(m_buffer, str.data() + pos, size);
        m_end = m_buffer + size;
        m_buffer[size] = NullCharConst;
    }
    return *this;
}

/// <summary>
/// Return the character at specified position
/// </summary>
/// <param name="pos">Position in string</param>
/// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, a reference to a non-const null character is returned (NullChar, is reinitialized
/// before returning)</returns>
String::ValueType& String::at(size_t pos)
{
    if (pos >= length())
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
const String::ValueType& String::at(size_t pos) const
{
    if (pos >= length())
        return NullCharConst;
    return m_buffer[pos];
}

/// <summary>
/// Return the first character
/// </summary>
/// <returns>Returns a non-const reference to the first character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before
/// returning)</returns>
String::ValueType& String::front()
{
    if (empty())
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
const String::ValueType& String::front() const
{
    if (empty())
        return NullCharConst;
    return *m_buffer;
}

/// <summary>
/// Return the last character
/// </summary>
/// <returns>Returns a non-const reference to the last character in the string. If the string is empty, a reference to a non-const null character is returned (NullChar, is reinitialized before
/// returning)</returns>
String::ValueType& String::back()
{
    if (empty())
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
const String::ValueType& String::back() const
{
    if (empty())
        return NullCharConst;
    return *(m_end - 1);
}

/// <summary>
/// Return the character at specified position
/// </summary>
/// <param name="pos">Position in string</param>
/// <returns>Returns a non-const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
String::ValueType& String::operator[](size_t pos)
{
    if (pos >= size())
    {
        NullChar = '\0';
        return NullChar;
    }
    return m_buffer[pos];
}

/// <summary>
/// Return the character at specified position
/// </summary>
/// <param name="pos">Position in string</param>
/// <returns>Returns a const reference to the character at offset pos. If the position pos is outside the string, the result is undetermined</returns>
const String::ValueType& String::operator[](size_t pos) const
{
    if (pos >= size())
        return NullCharConst;
    return m_buffer[pos];
}

/// <summary>
/// Return the buffer pointer
/// </summary>
/// <returns>Returns a non-const pointer to the buffer. If the buffer is not allocated, a pointer to a non-const null character (NullChar, initialized before returning) is returned</returns>
String::ValueType* String::data()
{
    NullChar = NullCharConst;
    return (m_buffer == nullptr) ? &NullChar : m_buffer;
}

/// <summary>
/// Return the buffer pointer
/// </summary>
/// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
const String::ValueType* String::data() const
{
    return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
}

/// <summary>
/// Return the buffer pointer
/// </summary>
/// <returns>Returns a const pointer to the buffer. If the buffer is not allocated, a pointer to a const null character (NullCharConst) is returned</returns>
const String::ValueType* String::c_str() const
{
    return (m_buffer == nullptr) ? &NullCharConst : m_buffer;
}

/// <summary>
/// Determine whether string is empty.
/// </summary>
/// <returns>Returns true when the string is empty (not allocated or no contents), false otherwise</returns>
bool String::empty() const
{
    return m_end == m_buffer;
}

/// <summary>
/// Return the size of the string
///
/// This method is the equivalent of length().
/// </summary>
/// <returns>Returns the size (or length) of the string</returns>
size_t String::size() const
{
    return m_end - m_buffer;
}

/// <summary>
/// Return the length of the string
///
/// This method is the equivalent of size().
/// </summary>
/// <returns>Returns the size (or length) of the string</returns>
size_t String::length() const
{
    return m_end - m_buffer;
}

/// <summary>
/// Return the capacity of the string
///
/// The capacity is the size of the allocated buffer. The string can grow to that length before it needs to be re-allocated.
/// </summary>
/// <returns>Returns the size (or length) of the string</returns>
size_t String::capacity() const
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
size_t String::reserve(size_t newCapacity)
{
    reallocate_allocation_size(newCapacity);
    return m_allocatedSize;
}

/// <summary>
/// append operator
///
/// Appends a character to the string
/// </summary>
/// <param name="ch">Character to append</param>
/// <returns>Returns a reference to the string</returns>
String& String::operator+=(ValueType ch)
{
    append(1, ch);
    return *this;
}

/// <summary>
/// append operator
///
/// Appends a string to the string
/// </summary>
/// <param name="str">string to append</param>
/// <returns>Returns a reference to the string</returns>
String& String::operator+=(const String& str)
{
    append(str);
    return *this;
}

/// <summary>
/// append operator
///
/// Appends a string to the string
/// </summary>
/// <param name="str">string to append. If nullptr the nothing is appended</param>
/// <returns>Returns a reference to the string</returns>
String& String::operator+=(const ValueType* str)
{
    append(str);
    return *this;
}

/// <summary>
/// append operator
///
/// Appends a sequence of count times the same character ch to the string
/// </summary>
/// <param name="count">Number of characters to append</param>
/// <param name="ch">Character to append</param>
void String::append(size_t count, ValueType ch)
{
    auto len = length();
    auto strLength = count;
    if (strLength > MaximumStringSize - len)
        strLength = MaximumStringSize - len;
    auto size = len + strLength;
    if ((size + 1) > m_allocatedSize)
    {
        if (!reallocate(size + 1))
            return;
    }
    memset(m_buffer + len, ch, strLength);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// append operator
///
/// Appends a string to the string
/// </summary>
/// <param name="str">string to append</param>
void String::append(const String& str)
{
    auto len = length();
    auto strLength = str.length();
    auto size = len + strLength;
    if ((size + 1) > m_allocatedSize)
    {
        if (!reallocate(size + 1))
            return;
    }
    strncpy(m_buffer + len, str.data(), strLength);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// append operator
///
/// Appends a substring of str to the string
/// </summary>
/// <param name="str">string to append from</param>
/// <param name="pos">Start position in str to copy characters from</param>
/// <param name="count">Number of characters to copy from str. Default is until the end of the string. If count is larger than the string length, characters are copied up to the end of the
/// string</param>
void String::append(const String& str, size_t pos, size_t count /*= npos*/)
{
    if (pos >= str.length())
        return;
    auto strLength = str.length();
    auto strCount = strLength - pos;
    if (count < strCount)
        strCount = count;
    auto len = length();
    auto size = len + strCount;
    if ((size + 1) > m_allocatedSize)
    {
        if (!reallocate(size + 1))
            return;
    }
    strncpy(m_buffer + len, str.data() + pos, strCount);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// append operator
///
/// Appends a string to the string
/// </summary>
/// <param name="str">string to append. If nullptr the nothing is appended</param>
void String::append(const ValueType* str)
{
    if (str == nullptr)
        return;
    auto len = length();
    auto strLength = strlen(str);
    auto size = len + strLength;
    if ((size + 1) > m_allocatedSize)
    {
        if (!reallocate(size + 1))
            return;
    }
    strncpy(m_buffer + len, str, strLength);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// append operator
///
/// Appends a number of characters from str to the string
/// </summary>
/// <param name="str">string to append. If nullptr the nothing is appended</param>
/// <param name="count">Number of characters to copy from str. If count is larger than the string length, the complete string is copied</param>
void String::append(const ValueType* str, size_t count)
{
    if (str == nullptr)
        return;
    auto len = length();
    auto strLength = strlen(str);
    auto strCount = count;
    if (strCount > strLength)
        strCount = strLength;
    auto size = len + strCount;
    if ((size + 1) > m_allocatedSize)
    {
        if (!reallocate(size + 1))
            return;
    }
    strncpy(m_buffer + len, str, strCount);
    m_end = m_buffer + size;
    m_buffer[size] = NullCharConst;
}

/// <summary>
/// clear the string
///
/// Clears the contents of the string, but does not free or reallocate the buffer
/// </summary>
void String::clear()
{
    if (!empty())
    {
        m_end = m_buffer;
        m_buffer[0] = NullCharConst;
    }
}

/// <summary>
/// find a substring in the string
///
/// If empty string, always finds the string.
/// </summary>
/// <param name="str">Substring to find</param>
/// <param name="pos">Starting position in string to start searching</param>
/// <returns>Location of first character in string of match if found, String::npos if not found</returns>
size_t String::find(const String& str, size_t pos /*= 0*/) const
{
    auto len = length();
    auto patternLength = str.length();
    if (pos >= len)
        return npos;
    auto needle = str.data();
    for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
    {
        if (memcmp(haystack, needle, patternLength) == 0)
            return haystack - m_buffer;
    }
    return npos;
}

/// <summary>
/// find a substring in the string
///
/// If nullptr or empty string, always finds the string.
/// </summary>
/// <param name="str">Substring to find</param>
/// <param name="pos">Starting position in string to start searching</param>
/// <returns>Location of first character in string of match if found, String::npos if not found</returns>
size_t String::find(const ValueType* str, size_t pos /*= 0*/) const
{
    size_t strLength{};
    if (str != nullptr)
    {
        strLength = strlen(str);
    }
    auto len = length();
    auto patternLength = strLength;
    if (pos >= len)
        return npos;
    auto needle = str;
    for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
    {
        if (memcmp(haystack, needle, patternLength) == 0)
            return haystack - m_buffer;
    }
    return npos;
}

/// <summary>
/// find a substring in the string
///
/// If nullptr or empty string, always finds the string.
/// </summary>
/// <param name="str">Substring to find</param>
/// <param name="pos">Starting position in string to start searching</param>
/// <param name="count">Number of characters from str to compare</param>
/// <returns>Location of first character in string of match if found, String::npos if not found</returns>
size_t String::find(const ValueType* str, size_t pos, size_t count) const
{
    size_t strLength{};
    if (str != nullptr)
    {
        strLength = strlen(str);
    }
    auto len = length();
    auto patternLength = strLength;
    if (pos >= len)
        return npos;
    if (count < patternLength)
        patternLength = count;
    auto needle = str;
    for (const ValueType* haystack = data() + pos; haystack <= m_end - patternLength; ++haystack)
    {
        if (memcmp(haystack, needle, patternLength) == 0)
            return haystack - m_buffer;
    }
    return npos;
}

/// <summary>
/// find a character in the string
/// </summary>
/// <param name="ch">Character to find</param>
/// <param name="pos">Starting position in string to start searching</param>
/// <returns>Location of first character in string of match if found, String::npos if not found</returns>
size_t String::find(ValueType ch, size_t pos /*= 0*/) const
{
    auto len = length();
    if (pos >= len)
        return npos;
    for (const ValueType* haystack = data() + pos; haystack <= m_end; ++haystack)
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
bool String::starts_with(ValueType ch) const
{
    if (empty())
        return false;
    return m_buffer[0] == ch;
}

/// <summary>
/// Check whether string starts with substring
/// </summary>
/// <param name="str">SubString to find</param>
/// <returns>Returns true if str is first part of string, false otherwise</returns>
bool String::starts_with(const String& str) const
{
    auto len = length();
    auto strLength = str.length();
    if (strLength >= len)
        return false;

    return memcmp(data(), str.data(), strLength) == 0;
}

/// <summary>
/// Check whether string starts with substring
/// </summary>
/// <param name="str">SubString to find</param>
/// <returns>Returns true if str is first part of string, false otherwise</returns>
bool String::starts_with(const ValueType* str) const
{
    size_t strLength{};
    if (str != nullptr)
    {
        strLength = strlen(str);
    }
    auto len = length();
    if (strLength >= len)
        return false;

    return memcmp(data(), str, strLength) == 0;
}

/// <summary>
/// Check whether string ends with character
/// </summary>
/// <param name="ch">Character to find</param>
/// <returns>Returns true if ch is last character in string, false otherwise</returns>
bool String::ends_with(ValueType ch) const
{
    if (empty())
        return false;
    return m_buffer[length() - 1] == ch;
}

/// <summary>
/// Check whether string ends with substring
/// </summary>
/// <param name="str">SubString to find</param>
/// <returns>Returns true if str is last part of string, false otherwise</returns>
bool String::ends_with(const String& str) const
{
    auto len = length();
    auto strLength = str.length();
    if (strLength >= len)
        return false;

    return memcmp(m_end - strLength, str.data(), strLength) == 0;
}

/// <summary>
/// Check whether string ends with substring
/// </summary>
/// <param name="str">SubString to find</param>
/// <returns>Returns true if str is last part of string, false otherwise</returns>
bool String::ends_with(const ValueType* str) const
{
    size_t strLength{};
    if (str != nullptr)
    {
        strLength = strlen(str);
    }
    auto len = length();
    if (strLength >= len)
        return false;

    return memcmp(m_end - strLength, str, strLength) == 0;
}

/// <summary>
/// Check whether string contains character
/// </summary>
/// <param name="ch">Character to find</param>
/// <returns>Returns true if ch is contained in string, false otherwise</returns>
bool String::contains(ValueType ch) const
{
    return find(ch) != npos;
}

/// <summary>
/// Check whether string contains substring
/// </summary>
/// <param name="str">Substring to find</param>
/// <returns>Returns true if ch is contained in string, false otherwise</returns>
bool String::contains(const String& str) const
{
    return find(str) != npos;
}

/// <summary>
/// Check whether string contains substring
/// </summary>
/// <param name="str">Substring to find</param>
/// <returns>Returns true if ch is contained in string, false otherwise</returns>
bool String::contains(const ValueType* str) const
{
    return find(str) != npos;
}

/// <summary>
/// Return substring
/// </summary>
/// <param name="pos">Starting position of substring in string</param>
/// <param name="count">length of substring to return. If count is larger than the number of characters available from position pos, the rest of the string is returned</param>
/// <returns>Returns the substring at position [pos, pos + count), if available </returns>
String String::substr(size_t pos /*= 0*/, size_t count /*= npos*/) const
{
    String result;
    auto size = length() - pos;
    if (pos < length())
    {
        if (count < size)
            size = count;
        result.reallocate(size + 1);
        memcpy(result.data(), data() + pos, size);
        result.m_end = result.m_buffer + size;
        result.data()[size] = NullCharConst;
    }

    return result;
}

/// <summary>
/// Case sensitive equality to string
/// </summary>
/// <param name="other">string to compare to</param>
/// <returns>Returns true if the strings are equal, false otherwise</returns>
bool String::equals(const String& other) const
{
    return compare(other) == 0;
}

/// <summary>
/// Case sensitive equality to string
/// </summary>
/// <param name="other">string to compare to</param>
/// <returns>Returns true if the strings are equal, false otherwise</returns>
bool String::equals(const ValueType* other) const
{
    if (other == nullptr)
        return empty();
    if (length() != strlen(other))
        return false;
    return strcmp(data(), other) == 0;
}

/// <summary>
/// Case insensitive equality to string
/// </summary>
/// <param name="other">string to compare to</param>
/// <returns>Returns true if the strings are equal, false otherwise</returns>
bool String::equals_case_insensitive(const String& other) const
{
    if (length() != other.length())
        return false;
    if (empty())
        return true;
    return strcasecmp(data(), other.data()) == 0;
}

/// <summary>
/// Case insensitive equality to string
/// </summary>
/// <param name="other">string to compare to</param>
/// <returns>Returns true if the strings are equal, false otherwise</returns>
bool String::equals_case_insensitive(const ValueType* other) const
{
    if (other == nullptr)
        return empty();
    if (length() != strlen(other))
        return false;
    return strcasecmp(data(), other) == 0;
}

/// <summary>
/// Case sensitive compare to string
///
/// Compares the complete string, character by character
/// </summary>
/// <param name="str">string to compare to</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::compare(const String& str) const
{
    if (empty())
    {
        if (str.empty())
            return 0;
        return -1;
    }
    if (str.empty())
        return 1;

    return strcmp(data(), str.data());
}

/// <summary>
/// Case sensitive compare to string
///
/// Compares the substring from pos to pos+count to str
/// </summary>
/// <param name="pos">Starting position of substring to compare to str</param>
/// <param name="count">Number of characters in substring to compare to str</param>
/// <param name="str">string to compare to</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::compare(size_t pos, size_t count, const String& str) const
{
    return substr(pos, count).compare(str);
}

/// <summary>
/// Case sensitive compare to string
///
/// Compares the substring from pos to pos+count to the substring from strPos to strPos+strCount of str
/// </summary>
/// <param name="pos">Starting position of substring to compare to str</param>
/// <param name="count">Number of characters in substring to compare to str</param>
/// <param name="str">string to compare to</param>
/// <param name="strPos">Starting position of substring of str to compare</param>
/// <param name="strCount">Number of characters in substring of str to compare</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::compare(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount /*= npos*/) const
{
    return substr(pos, count).compare(str.substr(strPos, strCount));
}

/// <summary>
/// Case sensitive compare to string
///
/// Compares the complete string to str
/// </summary>
/// <param name="str">string to compare to</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::compare(const ValueType* str) const
{
    size_t strLength{};
    if (str != nullptr)
        strLength = strlen(str);
    if (empty())
    {
        if (strLength == 0)
            return 0;
        return -1;
    }
    if (strLength == 0)
        return 1;

    return strcmp(data(), str);
}

/// <summary>
/// Case sensitive compare to string
///
/// Compares the substring from pos to pos+count to str
/// </summary>
/// <param name="pos">Starting position of substring to compare to str</param>
/// <param name="count">Number of characters in substring to compare to str</param>
/// <param name="str">string to compare to</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::compare(size_t pos, size_t count, const ValueType* str) const
{
    size_t strLength{};
    if (str != nullptr)
        strLength = strlen(str);

    auto len = length();
    if (pos >= len)
        len = 0;
    len -= pos;
    if (count < len)
        len = count;
    if (len == 0)
    {
        if (strLength == 0)
            return 0;
        return -1;
    }
    if (strLength == 0)
        return 1;

    auto maxLen = strLength;
    if (maxLen < len)
        maxLen = len;
    return strncmp(data() + pos, str, maxLen);
}

/// <summary>
/// Case sensitive compare to string
///
/// Compares the substring from pos to pos+count to the first strCount characters of str
/// </summary>
/// <param name="pos">Starting position of substring to compare to str</param>
/// <param name="count">Number of characters in substring to compare to str</param>
/// <param name="str">string to compare to</param>
/// <param name="strCount">Number of characters in substring of str to compare</param>
/// <returns>Returns 0 if the strings are equal (case sensitive), -1 if str is larger, 1 if it is smaller</returns>
int String::compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const
{
    size_t strLength{};
    if (str != nullptr)
        strLength = strlen(str);

    auto len = length();
    if (pos >= len)
        len = 0;
    len -= pos;
    if (count < len)
        len = count;

    if (strCount < strLength)
        strLength = strCount;

    if (len == 0)
    {
        if (strLength == 0)
            return 0;
        return -1;
    }
    if (strLength == 0)
        return 1;

    auto maxLen = strLength;
    if (maxLen < len)
        maxLen = len;
    return strncmp(data() + pos, str, maxLen);
}

/// <summary>
/// replace substring
///
/// Replaces the substring from pos to pos+count with str
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="str">string to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::replace(size_t pos, size_t count, const String& str)
{
    String result = substr(0, pos) + str + substr(pos + count);
    assign(result);
    return *this;
}

/// <summary>
/// replace substring
///
/// Replaces the substring from pos to pos+count with the substring from strPos to strPos+strCount of str
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="str">string to replace with</param>
/// <param name="strPos">Starting position of substring of str to replace with</param>
/// <param name="strCount">Number of characters in substring of str to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::replace(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount /*= npos*/)
{
    String result = substr(0, pos) + str.substr(strPos, strCount) + substr(pos + count);
    assign(result);
    return *this;
}

/// <summary>
/// replace substring
///
/// Replaces the substring from pos to pos+count with str
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="str">string to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::replace(size_t pos, size_t count, const ValueType* str)
{
    String result = substr(0, pos) + str + substr(pos + count);
    assign(result);
    return *this;
}

/// <summary>
/// replace substring
///
/// Replaces the substring from pos to pos+count with the first strCount characters of str
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="str">string to replace with</param>
/// <param name="strCount">Number of characters in substring to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::replace(size_t pos, size_t count, const ValueType* str, size_t strCount)
{
    String result = substr(0, pos) + String(str, strCount) + substr(pos + count);
    assign(result);
    return *this;
}

/// <summary>
/// replace substring
///
/// Replaces the substring from pos to pos+count with ch
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="ch">Characters to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::replace(size_t pos, size_t count, ValueType ch)
{
    return replace(pos, count, ch, 1);
}

/// <summary>
/// replace substring
///
/// Replaces the substring from pos to pos+count with a sequence of chCount copies of ch
/// </summary>
/// <param name="pos">Starting position of substring to replace</param>
/// <param name="count">Number of characters in substring to replace</param>
/// <param name="ch">Characters to replace with</param>
/// <param name="chCount">Number of copies of ch to replace with</param>
/// <returns>Returns the reference to the resulting string</returns>
String& String::replace(size_t pos, size_t count, ValueType ch, size_t chCount)
{
    String result = substr(0, pos) + String(chCount, ch) + substr(pos + count);
    assign(result);
    return *this;
}

/// <summary>
/// replace substring
///
/// Replaces all instances of the substring oldStr (if existing) with newStr
/// </summary>
/// <param name="oldStr">string to find in string</param>
/// <param name="newStr">string to replace with</param>
/// <returns>Returns the number of times the string was replaced</returns>
int String::replace(const String& oldStr, const String& newStr)
{
    size_t pos = find(oldStr);
    size_t oldLength = oldStr.length();
    size_t newLength = newStr.length();
    int count = 0;
    while (pos != npos)
    {
        replace(pos, oldLength, newStr);
        pos += newLength;
        pos = find(oldStr, pos);
        count++;
    }
    return count;
}

/// <summary>
/// replace substring
///
/// Replaces all instances of the substring oldStr (if existing) with newStr
/// </summary>
/// <param name="oldStr">string to find in string</param>
/// <param name="newStr">string to replace with</param>
/// <returns>Returns the number of times the string was replaced</returns>
int String::replace(const ValueType* oldStr, const ValueType* newStr)
{
    if ((oldStr == nullptr) || (newStr == nullptr))
        return 0;
    size_t pos = find(oldStr);
    size_t oldLength = strlen(oldStr);
    size_t newLength = strlen(newStr);
    int count = 0;
    while (pos != npos)
    {
        replace(pos, oldLength, newStr);
        pos += newLength;
        pos = find(oldStr, pos);
        count++;
    }
    return count;
}

/// <summary>
/// Align string
///
/// Pads the string on the left (width > 0) or on the right (width < 0) up to a length of width characters. If the string is larger than width characters, it is not modified.
/// </summary>
/// <param name="width">length of target string. If width < 0, the string is padded to the right with spaces up to -width characters.
/// if width > 0, the string is padded to the left with space up to width characters</param>
/// <returns>Returns the number of times the string was replaced</returns>
String String::align(int width) const
{
    String result;
    int absWidth = (width > 0) ? width : -width;
    auto len = length();
    if (static_cast<size_t>(absWidth) > len)
    {
        if (width < 0)
        {
            result = *this + String(static_cast<unsigned int>(-width) - len, ' ');
        }
        else
        {
            result = String(static_cast<unsigned int>(width) - len, ' ') + *this;
        }
    }
    else
        result = *this;
    return result;
}

/// <summary>
/// Allocate or re-allocate string to have a capacity of requestedLength characters
/// </summary>
/// <param name="requestedLength">Amount of characters in the string to allocate space for</param>
/// <returns>True if successful, false otherwise</returns>
bool String::reallocate(size_t requestedLength)
{
    auto requestedSize = requestedLength;
    auto allocationSize = NextPowerOf2((requestedSize < MinimumAllocationSize) ? MinimumAllocationSize : requestedSize);

    if (!reallocate_allocation_size(allocationSize))
        return false;
    return true;
}

/// <summary>
/// Allocate or re-allocate string to have a capacity of allocationSize bytes
/// </summary>
/// <param name="allocationSize">Amount of bytes to allocate space for</param>
/// <returns>True if successful, false otherwise</returns>
bool String::reallocate_allocation_size(size_t allocationSize)
{
    auto newBuffer = reinterpret_cast<ValueType*>(realloc(m_buffer, allocationSize));
    if (newBuffer == nullptr)
    {
        return false;
    }
    m_buffer = newBuffer;
#if BAREMETAL_MEMORY_TRACING_DETAIL
    LOG_NO_ALLOC_DEBUG("Alloc string %p", m_buffer);
#endif
    if (m_end == nullptr)
        m_end = m_buffer;
    if (m_end > m_buffer + allocationSize)
        m_end = m_buffer + allocationSize;
    m_allocatedSize = allocationSize;
    return true;
}
