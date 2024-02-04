//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
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

/// <summary>
/// Constant iterator template
/// 
/// Based on arrays of type T pointed to by const pointers.
/// </summary>
/// <typeparam name="T">Value type to iterate through</typeparam>
template<class T>
class ConstIterator
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
    /// Construct a ConstIterator
    /// </summary>
    /// <param name="begin">Start of the range</param>
    /// <param name="end">End of the range + 1</param>
    explicit ConstIterator(const T* begin, const T* end)
        : m_begin{ begin }
        , m_end{ end }
        , m_current{ begin }
    {
    }
    /// <summary>
    /// Advance in range by 1 step (post increment)
    /// </summary>
    /// <returns>Position after advancing as ConstIterator</returns>
    ConstIterator& operator++()
    {
        ++m_current;
        return *this;
    }
    /// <summary>
    /// Advance in range by 1 step (pre increment)
    /// </summary>
    /// <returns>Position before advancing as ConstIterator</returns>
    ConstIterator operator++(int)
    {
        ConstIterator retval = *this;
        ++(*this);
        return retval;
    }
    /// <summary>
    /// Equality comparison
    /// 
    /// Compares current position with current position in passed iterator
    /// </summary>
    /// <param name="other">ConstIterator to compare with</param>
    /// <returns>Returns true if positions are equal, false if not</returns>
    bool operator==(const ConstIterator& other) const
    {
        return m_current == other.m_current;
    }
    /// <summary>
    /// Inequality comparison
    /// 
    /// Compares current position with current position in passed iterator
    /// </summary>
    /// <param name="other">ConstIterator to compare with</param>
    /// <returns>Returns false if positions are equal, true if not</returns>
    bool operator!=(const ConstIterator& other) const
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

