//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : FIFO.h
//
// Namespace   : baremetal
//
// Class       : FIFO
//
// Description : FIFO for mock classes
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

#include "stdlib/Macros.h"
#include "stdlib/Types.h"

/// @file
/// Memory access mock class

namespace baremetal {

/// @brief FIFO template class
template <int N>
class FIFO
{
private:
    /// @brief Data in FIFO (max 16 bytes)
    uint8 m_data[N]; // Size 16
    /// @brief Read index. If m_readIndex == m_writeIndex, the FIFO is either empty or full, depending on the full flag
    int m_readIndex; // Size 4
    /// @brief Write index. If m_readIndex == m_writeIndex, the FIFO is either empty or full, depending on the full flag
    int m_writeIndex; // Size 4
    /// @brief Flag whether the FIFO is full
    bool m_isFull; // Size 1
    /// @brief Force alignment to 16 bytes
    uint8 align[7]; // Size 7

public:
    /// <summary>
    /// FIFO constructor
    /// </summary>
    FIFO()
        : m_data{}
        , m_readIndex{}
        , m_writeIndex{}
        , m_isFull{}
    {
    }
    /// <summary>
    /// Read a byte from the FIFO
    /// </summary>
    /// <returns>Byte read. If nothing can be read, returns 0</returns>
    uint8 Read()
    {
        uint8 result{};
        if (!IsEmpty())
        {
            result = m_data[m_readIndex];
            m_readIndex = (m_readIndex + 1) % N;
            m_isFull = false;
        }
        return result;
    }
    /// <summary>
    /// Write a byte to the FIFO
    /// </summary>
    /// <param name="data">Data to write</param>
    void Write(uint8 data)
    {
        if (!IsFull())
        {
            m_data[m_writeIndex] = data;
            m_writeIndex = (m_writeIndex + 1) % N;
            if (m_readIndex == m_writeIndex)
                m_isFull = true;
        }
    }
    /// <summary>
    /// Check if FIFO is empty
    /// </summary>
    /// <returns>true if FIFO is empty, false otherwise</returns>
    bool IsEmpty()
    {
        return (m_readIndex == m_writeIndex) && !m_isFull;
    }
    /// <summary>
    /// Check if FIFO is full
    /// </summary>
    /// <returns>true if FIFO is full, false otherwise</returns>
    bool IsFull()
    {
        return (m_readIndex == m_writeIndex) && m_isFull;
    }
    /// <summary>
    /// Get number of bytes used in FIFO
    /// </summary>
    /// <returns>Number of bytes used in FIFO</returns>
    size_t UsedSpace()
    {
        if (m_isFull)
            return N;
        return (m_writeIndex - m_readIndex + N) % N;
    }
    /// <summary>
    /// Get number of bytes free in FIFO
    /// </summary>
    /// <returns>Number of bytes free used in FIFO</returns>
    size_t FreeSpace()
    {
        return N - UsedSpace();
    }
    /// <summary>
    /// Check if FIFO is at most 25% full
    /// </summary>
    /// <returns>true if FIFO is at most 25% full, false otherwise</returns>
    bool IsOneQuarterOrLessFull()
    {
        return UsedSpace() <= (N / 4);
    }
    /// <summary>
    /// Check if FIFO is at least 75% full (at most 25% empty)
    /// </summary>
    /// <returns>true if FIFO is at least 75% full, false otherwise</returns>
    bool IsThreeQuartersOrMoreFull()
    {
        return FreeSpace() <= (N / 4);
    }
    /// <summary>
    /// Flush the FIFO
    /// </summary>
    void Flush()
    {
        m_readIndex = m_writeIndex = 0;
        m_isFull = false;
    }
}
/// @cond
ALIGN(8)
/// @endcond
;

} // namespace baremetal
