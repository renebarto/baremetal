//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : Iterator.h
//
// Namespace   : baremetal
//
// Class       : Iterator
//
// Description : Generic iterator template
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

/// @file
/// Iterator classes

/// <summary>
/// Const iterator template
///
/// Based on arrays of type T pointed to by const pointers.
/// </summary>
/// <typeparam name="T">Value type to iterate through</typeparam>
template <class T>
class const_iterator
{
private:
    /// @brief Start of the range
    T const* m_begin;
    /// @brief End of the range + 1
    T const* m_end;
    /// @brief Current position
    T const* m_current;

public:
    /// <summary>
    /// Construct a const_iterator
    /// </summary>
    /// <param name="begin">Start of the range</param>
    /// <param name="end">End of the range + 1</param>
    explicit const_iterator(const T* begin, const T* end)
        : m_begin{begin}
        , m_end{end}
        , m_current{begin}
    {
    }
    /// <summary>
    /// Advance in range by 1 step (post increment)
    /// </summary>
    /// <returns>Position after advancing as const_iterator</returns>
    const_iterator& operator++()
    {
        ++m_current;
        return *this;
    }
    /// <summary>
    /// Advance in range by 1 step (pre increment)
    /// </summary>
    /// <returns>Position before advancing as const_iterator</returns>
    const_iterator operator++(int)
    {
        const_iterator retval = *this;
        ++(*this);
        return retval;
    }
    /// <summary>
    /// Equality comparison
    ///
    /// Compares current position with current position in passed iterator
    /// </summary>
    /// <param name="other">const_iterator to compare with</param>
    /// <returns>Returns true if positions are equal, false if not</returns>
    bool operator==(const const_iterator& other) const
    {
        return m_current == other.m_current;
    }
    /// <summary>
    /// Inequality comparison
    ///
    /// Compares current position with current position in passed iterator
    /// </summary>
    /// <param name="other">const_iterator to compare with</param>
    /// <returns>Returns false if positions are equal, true if not</returns>
    bool operator!=(const const_iterator& other) const
    {
        return !(*this == other);
    }
    /// <summary>
    /// Return value at current location
    /// </summary>
    /// <returns>Value to which the current position points</returns>
    T const& operator*() const
    {
        return *m_current;
    }
};

/// <summary>
/// Non-const iterator template
///
/// Based on arrays of type T pointed to by pointers.
/// </summary>
/// <typeparam name="T">Value type to iterate through</typeparam>
template <class T>
class iterator
{
private:
    /// @brief Start of the range
    T* m_begin;
    /// @brief End of the range + 1
    T* m_end;
    /// @brief Current position
    T* m_current;

public:
    /// <summary>
    /// Construct a iterator
    /// </summary>
    /// <param name="begin">Start of the range</param>
    /// <param name="end">End of the range + 1</param>
    explicit iterator(T* begin, T* end)
        : m_begin{begin}
        , m_end{end}
        , m_current{begin}
    {
    }
    /// <summary>
    /// Advance in range by 1 step (post increment)
    /// </summary>
    /// <returns>Position after advancing as iterator</returns>
    iterator& operator++()
    {
        ++m_current;
        return *this;
    }
    /// <summary>
    /// Advance in range by 1 step (pre increment)
    /// </summary>
    /// <returns>Position before advancing as iterator</returns>
    iterator operator++(int)
    {
        iterator retval = *this;
        ++(*this);
        return retval;
    }
    /// <summary>
    /// Equality comparison
    ///
    /// Compares current position with current position in passed iterator
    /// </summary>
    /// <param name="other">iterator to compare with</param>
    /// <returns>Returns true if positions are equal, false if not</returns>
    bool operator==(const iterator& other) const
    {
        return m_current == other.m_current;
    }
    /// <summary>
    /// Inequality comparison
    ///
    /// Compares current position with current position in passed iterator
    /// </summary>
    /// <param name="other">iterator to compare with</param>
    /// <returns>Returns false if positions are equal, true if not</returns>
    bool operator!=(const iterator& other) const
    {
        return !(*this == other);
    }
    /// <summary>
    /// Return value at current location
    /// </summary>
    /// <returns>Value to which the current position points</returns>
    T& operator*() const
    {
        return *m_current;
    }
};
