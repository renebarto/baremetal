//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : String.h
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

#pragma once

#include <baremetal/Iterator.h>
#include <baremetal/Types.h>

/// @file
/// String class

namespace baremetal {

/// <summary>
/// String class
/// </summary>
class String
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

    String();
    explicit String(const ValueType* str);
    String(const ValueType* str, size_t count);
    String(size_t count, ValueType ch);
    String(const String& other);
    String(const String& other, size_t pos, size_t count = npos);
    ~String();

    operator const ValueType *() const;
    String&     operator=(const ValueType* other);
    String&     operator=(const String& other);

    ConstIterator<ValueType>    begin() const;
    ConstIterator<ValueType>    end() const;

    String&                     Assign(const ValueType* str);
    String&                     Assign(const ValueType* str, size_t count);
    String&                     Assign(size_t count, ValueType ch);
    String&                     Assign(const String& str);
    String&                     Assign(const String& str, size_t pos, size_t count = npos);

    ValueType&                  At(size_t pos);
    const ValueType&            At(size_t pos) const;
    ValueType&                  Front();
    const ValueType&            Front() const;
    ValueType&                  Back();
    const ValueType&            Back() const;
    ValueType&                  operator[] (size_t pos);
    const ValueType&            operator[] (size_t pos) const;
    ValueType*                  Data();
    const ValueType*            Data() const;

    bool                        Empty() const;
    size_t                      Size() const;
    size_t                      Length() const;
    size_t                      Capacity() const;
    size_t                      Reserve(size_t newCapacity);

    String&                     operator +=(ValueType ch);
    String&                     operator +=(const String & str);
    String&                     operator +=(const ValueType* str);
    void                        Append(size_t count, ValueType ch);
    void                        Append(const String& str);
    void                        Append(const String& str, size_t pos, size_t count = npos);
    void                        Append(const ValueType *str);
    void                        Append(const ValueType* str, size_t count);
    void                        Clear();

    size_t                      Find(const String& str, size_t pos = 0) const;
    size_t                      Find(const ValueType* str, size_t pos = 0) const;
    size_t                      Find(const ValueType* str, size_t pos, size_t count) const;
    size_t                      Find(ValueType ch, size_t pos = 0) const;
    bool                        StartsWith(ValueType ch) const;
    bool                        StartsWith(const String& str) const;
    bool                        StartsWith(const ValueType* str) const;
    bool                        EndsWith(ValueType ch) const;
    bool                        EndsWith(const String& str) const;
    bool                        EndsWith(const ValueType* str) const;
    bool                        Contains(ValueType ch) const;
    bool                        Contains(const String& str) const;
    bool                        Contains(const ValueType* str) const;
    String                      SubStr(size_t pos = 0, size_t count = npos) const;

    bool                        Equals(const String& other) const;
    bool                        Equals(const ValueType* other) const;
    bool                        EqualsCaseInsensitive(const String& other) const;
    bool                        EqualsCaseInsensitive(const ValueType* other) const;
    int                         Compare(const String& str) const;
    int                         Compare(size_t pos, size_t count, const String& str) const;
    int                         Compare(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount = npos) const;
    int                         Compare(const ValueType* str) const;
    int                         Compare(size_t pos, size_t count, const ValueType* str) const;
    int                         Compare(size_t pos, size_t count, const ValueType* str, size_t strCount) const;

    String&                     Replace(size_t pos, size_t count, const String& str);
    String&                     Replace(size_t pos, size_t count, const String& str, size_t strPos, size_t strCount = npos);
    String&                     Replace(size_t pos, size_t count, const ValueType* str);
    String&                     Replace(size_t pos, size_t count, const ValueType* str, size_t strCount);
    String&                     Replace(size_t pos, size_t count, ValueType ch);
    String&                     Replace(size_t pos, size_t count, ValueType ch, size_t chCount);
    int                         Replace(const String& oldStr, const String& newStr); // returns number of occurrences
    int                         Replace(const ValueType *oldStr, const ValueType *newStr); // returns number of occurrences

    String                      Align(int width) const;

private:
    bool        Reallocate(size_t requestedLength);
    bool        ReallocateAllocationSize(size_t allocationSize);
};

/// <summary>
/// Equality operator
/// 
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns true if strings are equal, false if not</returns>
inline bool operator==(const String &lhs, const String &rhs)
{
    return lhs.Equals(rhs);
}

/// <summary>
/// Equality operator
/// 
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns true if strings are equal, false if not</returns>
inline bool operator==(const String &lhs, const String::ValueType *rhs)
{
    return lhs.Equals(rhs);
}

/// <summary>
/// Equality operator
/// 
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns true if strings are equal, false if not</returns>
inline bool operator==(const String::ValueType *lhs, const String &rhs)
{
    return rhs.Equals(lhs);
}

/// <summary>
/// Inequality operator
/// 
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns false if strings are equal, true if not</returns>
inline bool operator!=(const String &lhs, const String &rhs)
{
    return !lhs.Equals(rhs);
}

/// <summary>
/// Inequality operator
/// 
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns false if strings are equal, true if not</returns>
inline bool operator!=(const String &lhs, const String::ValueType *rhs)
{
    return !lhs.Equals(rhs);
}

/// <summary>
/// Inequality operator
/// 
/// Performs a case sensitive comparison between two strings
/// </summary>
/// <param name="lhs">Left side of comparison</param>
/// <param name="rhs">Right side of comparison</param>
/// <returns>Returns false if strings are equal, true if not</returns>
inline bool operator!=(const String::ValueType *lhs, const String &rhs)
{
    return !rhs.Equals(lhs);
}

/// <summary>
/// Add two strings
/// 
/// Concatenates two strings and returns the result
/// </summary>
/// <param name="lhs">First part of the resulting string</param>
/// <param name="rhs">Second part of the resulting string</param>
/// <returns>Concatenation of first and second string</returns>
inline String operator +(const String& lhs, const String& rhs)
{
    String result = lhs;
    result.Append(rhs);
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
inline String operator +(const String::ValueType* lhs, const String& rhs)
{
    String result{ lhs };
    result.Append(rhs);
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
inline String operator +(const String& lhs, const String::ValueType* rhs)
{
    String result = lhs;
    result.Append(rhs);
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
inline String operator +(String::ValueType lhs, const String& rhs)
{
    String result;
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
inline String operator +(const String& lhs, String::ValueType rhs)
{
    String result;
    result += lhs;
    result += rhs;
    return result;
}

} // namespace baremetal

