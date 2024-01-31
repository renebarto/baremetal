//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2024 Rene Barto
//
// File        : ArmInstructions.h
//
// Namespace   : -
//
// Class       : -
//
// Description : Common instructions for e.g. synchronization
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

// ARM instructions represented as macros for ease of use.
//
// For specific registers, we also define the fields and their possible values.

// NOP instruction
#define NOP()                           asm volatile("nop")

// Data sync barrier
#define DataSyncBarrier()               asm volatile ("dsb sy" ::: "memory")
// Data memory barrier
#define DataMemBarrier()                asm volatile ("dmb sy" ::: "memory")

// Wait for interrupt
#define WaitForInterrupt()              asm volatile ("wfi")

// Enable IRQss. Clear bit 1 of DAIF register.
#define	EnableIRQs()                    asm volatile ("msr DAIFClr, #2")
// Disable IRQs. Set bit 1 of DAIF register.
#define	DisableIRQs()                   asm volatile ("msr DAIFSet, #2")
// Enable FIQs. Clear bit 0 of DAIF register.
#define	EnableFIQs()                    asm volatile ("msr DAIFClr, #1")
// Disable FIQs. Set bit 0 of DAIF register.
#define	DisableFIQs()                   asm volatile ("msr DAIFSet, #1")

// Get counter timer frequency.
#define GetTimerFrequency(freq)         asm volatile ("mrs %0, CNTFRQ_EL0" : "=r"(freq))
// Get counter timer value.
#define GetTimerCounter(count)          asm volatile ("mrs %0, CNTPCT_EL0" : "=r"(count))

// Get Physical counter-timer control register.
#define GetTimerControl(value)          asm volatile ("mrs %0, CNTP_CTL_EL0" : "=r" (value))
// Set Physical counter-timer control register.
#define SetTimerControl(value)          asm volatile ("msr CNTP_CTL_EL0, %0" :: "r" (value))

// IStatus bit, flags if Physical counter-timer condition is met.
#define CNTP_CTL_EL0_STATUS BIT(2)
// IMask bit, flags if interrupts for Physical counter-timer are masked.
#define CNTP_CTL_EL0_IMASK BIT(1)
// Enable bit, flags if Physical counter-timer is enabled.
#define CNTP_CTL_EL0_ENABLE BIT(0)

// Get Physical counter-timer comparison value.
#define GetTimerCompareValue(value)     asm volatile ("mrs %0, CNTP_CVAL_EL0" : "=r" (value))
// Set Physical counter-timer comparison value.
#define SetTimerCompareValue(value)     asm volatile ("msr CNTP_CVAL_EL0, %0" :: "r" (value))
