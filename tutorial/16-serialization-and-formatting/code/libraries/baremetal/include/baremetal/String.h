//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : String.h
//
// Namespace   : baremetal
//
// Class       : string
//
// Description : string handling
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

#pragma once

#include <baremetal/Iterator.h>
#include <baremetal/Types.h>

/// @file
/// string class

namespace baremetal {

/// <summary>
/// string class
/// </summary>
class string
{
public:
    /// @brief Type of value the string contains
    using ValueType = char;

private:
    /// @brief Pointer to start of allocated memory
    ValueType* m_buffer;
    /// @brief Pointer one past the end of the string
    ValueType* m_end;
    /// @brief Currently allocated size in bytes
    size_t m_allocatedSize;

public:
    /// @brief Signifies the position at the end of the string, e.g. for length
    static const size_t npos;

    string();
    string(const ValueType* str);
    string(const ValueType* str, size_t count);
    string(size_t count, ValueType ch);
    string(const string& other);
    string(string&& other);
    string(const string& other, size_t pos, size_t count = npos);
    ~string();

    operator const ValueType *() const;
    string&     operator=(const ValueType* other);
    string&     operator=(const string& other);
    string&     operator=(string&& other);

    iterator<ValueType>         begin();
    iterator<ValueType>         end();
    const_iterator<ValueType>   begin() const;
    const_iterator<ValueType>   end() const;

    string&                     assign(const ValueType* str);
    string&                     assign(const ValueType* str, size_t count);
    string&                     assign(size_t count, ValueType ch);
    string&                     assign(const string& str);
    string&                     assign(const string& str, size_t pos, size_t count = npos);

    ValueType&                  at(size_t pos);
    const ValueType&            at(size_t pos) const;
    ValueType&                  front();
    const ValueType&            front() const;
    ValueType&                  back();
    const ValueType&            back() const;
    ValueType&                  operator[] (size_t pos);
    const ValueType&            operator[] (size_t pos) const;
    ValueType*                  data();
    const ValueType*            data() const;
    const ValueType*            c_str() const;

    bool                        empty() const;
    size_t                      size() const;
    size_t                      length() const;
    size_t                      capacity() const;
    size_t                      reserve(size_t newCapacity);

    string&                     operator +=(ValueType ch);
    string&                     operator +=(const string& str);
    string&                     operator +=(const ValueType* str);
    void                        append(size_t count, ValueType ch);
    void                        append(const string& str);
    void                        append(const string& str, size_t pos, size_t count = npos);
    void                        append(const ValueType *str);
    void                        append(const ValueType* str, size_t count);
    void                        clear();

    size_t                      find(const string& str, size_t pos = 0) const;
    size_t                      find(const ValueType* str, size_t pos = 0) const;
    size_t                      find(const ValueType* str, size_t pos, size_t count) const;
    size_t                      find(ValueType ch, size_t pos = 0) const;
    bool                        starts_with(ValueType ch) const;
    bool                        starts_with(const string& str) const;
    bool                        starts_with(const ValueType* str) const;
    bool                        ends_with(ValueType ch) const;
    bool                        ends_with(const string& str) const;
    bool                        ends_with(const ValueType* str) const;
    bool                        contains(ValueType ch) const;
    bool                        contains(const string& str) const;
    bool                        contains(const ValueType* str) const;
    string                      substr(size_t pos = 0, size_t count = npos) const;

    bool                        equals(const string& other) const;
    bool                        equals(const ValueType* other) const;
    bool                        equals_case_insensitive(const string& other) const;
    bool                        equals_case_insensitive(const ValueType* other) const;
    int                         compare(const string& str) const;
    int                         compare(size_t pos, size_t count, const string& str) const;
    int                         compare(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount = npos) const;
    int                         compare(const ValueType* str) const;
    int                         compare(size_t pos, size_t count, const ValueType* str) const;
    int                         compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const;

    string&                     replace(size_t pos, size_t count, const string& str);
    string&                     replace(size_t pos, size_t count, const string& str, size_t strPos, size_t strCount = npos);
    string&                     replace(size_t pos, size_t count, const ValueType* str);
    string&                     replace(size_t pos, size_t count, const ValueType* str, size_t strCount);
    string&                     replace(size_t pos, size_t count, ValueType ch);
    string&                     replace(size_t pos, size_t count, ValueType ch, size_t chCount);
    int                         replace(const string& oldStr, const string& newStr); // returns number of occurrences
    int                         replace(const ValueType *oldStr, const ValueType *newStr); // returns number of occurrences

    string                      align(int width) const;

private:
    bool        reallocate(size_t requestedLength);
    bool        reallocate_allocation_size(size_t allocationSize);
};

/// <summary>
/// Equality operator
///
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns true if strings are equal, false if not</returns>
inline bool operator==(const string &lhs, const string &rhs)
{
    return lhs.equals(rhs);
}

/// <summary>
/// Equality operator
///
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns true if strings are equal, false if not</returns>
inline bool operator==(const string &lhs, const string::ValueType *rhs)
{
    return lhs.equals(rhs);
}

/// <summary>
/// Equality operator
///
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns true if strings are equal, false if not</returns>
inline bool operator==(const string::ValueType *lhs, const string &rhs)
{
    return rhs.equals(lhs);
}

/// <summary>
/// Inequality operator
///
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns false if strings are equal, true if not</returns>
inline bool operator!=(const string &lhs, const string &rhs)
{
    return !lhs.equals(rhs);
}

/// <summary>
/// Inequality operator
///
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns false if strings are equal, true if not</returns>
inline bool operator!=(const string &lhs, const string::ValueType *rhs)
{
    return !lhs.equals(rhs);
}

/// <summary>
/// Inequality operator
///
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns false if strings are equal, true if not</returns>
inline bool operator!=(const string::ValueType *lhs, const string &rhs)
{
    return !rhs.equals(lhs);
}

/// <summary>
/// Add two strings
///
/// Concatenates two strings and returns the result
/// </summary>
/// <param name="lhs">First part of the resulting string</param>
/// <param name="rhs">Second part of the resulting string</param>
/// <returns>Concatenation of first and second string</returns>
inline string operator +(const string& lhs, const string& rhs)
{
    string result = lhs;
    result.append(rhs);
    return result;
}

/// <summary>
/// Add two strings
///
/// Concatenates two strings and returns the result
/// </summary>
/// <param name="lhs">First part of the resulting string</param>
/// <param name="rhs">Second part of the resulting string</param>
/// <returns>Concatenation of first and second string</returns>
inline string operator +(const string::ValueType* lhs, const string& rhs)
{
    string result{ lhs };
    result.append(rhs);
    return result;
}

/// <summary>
/// Add two strings
///
/// Concatenates two strings and returns the result
/// </summary>
/// <param name="lhs">First part of the resulting string</param>
/// <param name="rhs">Second part of the resulting string</param>
/// <returns>Concatenation of first and second string</returns>
inline string operator +(const string& lhs, const string::ValueType* rhs)
{
    string result = lhs;
    result.append(rhs);
    return result;
}

/// <summary>
/// Add character and string
///
/// Concatenates the left hand character argument and the right hand string and returns the result
/// </summary>
/// <param name="lhs">First character in the resulting string</param>
/// <param name="rhs">Second part of the resulting string</param>
/// <returns>Concatenation of first and second part</returns>
inline string operator +(string::ValueType lhs, const string& rhs)
{
    string result;
    result += lhs;
    result += rhs;
    return result;
}

/// <summary>
/// Add string and character
///
/// Concatenates the left hand string and the right hand character and returns the result
/// </summary>
/// <param name="lhs">First part of the resulting string</param>
/// <param name="rhs">Last character of the resulting string</param>
/// <returns>Concatenation of first and second part</returns>
inline string operator +(const string& lhs, string::ValueType rhs)
{
    string result;
    result += lhs;
    result += rhs;
    return result;
}

} // namespace baremetal
