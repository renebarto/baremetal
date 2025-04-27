//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : I2CMasterEmulator.cpp
//
// Namespace   : baremetal
//
// Class       : I2CMasterEmulator
//
// Description : I2C Master emulator
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

#include <baremetal/I2CMasterEmulator.h>

#include <baremetal/Logger.h>

/// @file
/// I2CMasterEmulator

using namespace baremetal;

/// @brief Define log name
LOG_MODULE("I2CMasterEmulator");

MemoryAccessStubI2C I2CMasterEmulator::m_memoryAccess{};

/// <summary>
/// Constructor for I2CMaster emulator
/// </summary>
I2CMasterEmulator::I2CMasterEmulator()
    : I2CMaster(m_memoryAccess)
{
}

/// <summary>
/// Initialize the I2CMaster for a specific bus, using the specified configuration index, and setting the clock as specified.
/// </summary>
/// <param name="bus">Device number (see: comment for I2CMaster class</param>
/// <param name="mode">I2C clock rate to be used</param>
/// <param name="config">Configuration index to be used. Determines which GPIO pins are to be used for the I2C bus</param>
/// <returns></returns>
bool I2CMasterEmulator::Initialize(uint8 bus, I2CMode mode /*= I2CMode::Normal*/, uint32 config /*= 0*/)
{
    m_memoryAccess.SetBus(bus);
    return I2CMaster::Initialize(bus, mode, config);
}

/// <summary>
/// Set the address send callback
/// </summary>
/// <param name="callback">Function pointer to callback function</param>
void I2CMasterEmulator::SetSendAddressByteCallback(SendAddressByteCallback callback)
{
    m_memoryAccess.SetSendAddressByteCallback(callback);
}

/// <summary>
/// Set the data receive callback
/// </summary>
/// <param name="callback">Function pointer to callback function</param>
void I2CMasterEmulator::SetRecvDataByteCallback(RecvDataByteCallback callback)
{
    m_memoryAccess.SetRecvDataByteCallback(callback);
}

/// <summary>
/// Set the data send callback
/// </summary>
/// <param name="callback">Function pointer to callback function</param>
void I2CMasterEmulator::SetSendDataByteCallback(SendDataByteCallback callback)
{
    m_memoryAccess.SetSendDataByteCallback(callback);
}
