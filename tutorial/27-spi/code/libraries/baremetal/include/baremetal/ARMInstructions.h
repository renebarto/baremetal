//------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2025 Rene Barto
//
// File        : ARMInstructions.h
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

/// @file
/// ARM instructions represented as macros for ease of use.
///
/// For specific registers, we also define the fields and their possible values.

/// @brief NOP instruction
#define NOP()                                          asm volatile("nop")

/// @brief Data sync barrier
#define DataSyncBarrier()                              asm volatile("dsb sy" ::: "memory")
/// @brief Data memory barrier
#define DataMemBarrier()                               asm volatile("dmb sy" ::: "memory")

/// @brief Wait for interrupt
#define WaitForInterrupt()                             asm volatile("wfi")

/// @brief Enable IRQs. Clear bit 1 of DAIF register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER
#define EnableIRQs()                                   asm volatile("msr DAIFClr, #2")
/// @brief Disable IRQs. Set bit 1 of DAIF register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER
#define DisableIRQs()                                  asm volatile("msr DAIFSet, #2")
/// @brief Enable FIQs. Clear bit 0 of DAIF register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER
#define EnableFIQs()                                   asm volatile("msr DAIFClr, #1")
/// @brief Disable FIQs. Set bit 0 of DAIF register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_DAIF_REGISTER
#define DisableFIQs()                                  asm volatile("msr DAIFSet, #1")

/// @brief Get counter timer frequency. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTFRQ_EL0_REGISTER
#define GetTimerFrequency(freq)                        asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(freq))
/// @brief Get counter timer value. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTPCT_EL0_REGISTER
#define GetTimerCounter(count)                         asm volatile("mrs %0, CNTPCT_EL0" : "=r"(count))

/// @brief Get Physical counter-timer control register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
#define GetTimerControl(value)                         asm volatile("mrs %0, CNTP_CTL_EL0" : "=r"(value))
/// @brief Set Physical counter-timer control register. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
#define SetTimerControl(value)                         asm volatile("msr CNTP_CTL_EL0, %0" ::"r"(value))

/// @brief IStatus bit, flags if Physical counter-timer condition is met. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
#define CNTP_CTL_EL0_STATUS                            BIT1(2)
/// @brief IMask bit, flags if interrupts for Physical counter-timer are masked. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
#define CNTP_CTL_EL0_IMASK                             BIT1(1)
/// @brief Enable bit, flags if Physical counter-timer is enabled. See @ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CTL_EL0_REGISTER
#define CNTP_CTL_EL0_ENABLE                            BIT1(0)

/// @brief Get Physical counter-timer comparison value. See \ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER
#define GetTimerCompareValue(value)                    asm volatile("mrs %0, CNTP_CVAL_EL0" : "=r"(value))
/// @brief Set Physical counter-timer comparison value. See \ref ARM_REGISTERS_REGISTER_OVERVIEW_CNTP_CVAL_EL0_REGISTER
#define SetTimerCompareValue(value)                    asm volatile("msr CNTP_CVAL_EL0, %0" ::"r"(value))

/// @brief Get current exception level
#define GetCurrentEL(value)                            asm volatile("mrs %0, CurrentEL" : "=r"(value))
/// @brief EL value shift
#define CURRENT_EL_SHIFT                               2
/// @brief Extract EL value
#define CURRENT_EL_VALUE(value)                        ((value & BITS(CURRENT_EL_SHIFT, CURRENT_EL_SHIFT + 1)) >> CURRENT_EL_SHIFT)

/// @brief Get Saved Program Status Register (EL1)
#define GetSPSR_EL1(value)                             asm volatile("mrs %0, SPSR_EL1" : "=r"(value))

/// @brief Get Exception Syndrome Register (EL1)
#define GetESR_EL1(value)                              asm volatile("mrs %0, ESR_EL1" : "=r"(value))

/// @brief SPSR_EL M[3:0] field bit shift
#define SPSR_EL1_M30_SHIFT                             0
/// @brief SPSR_EL M[3:0] field bit mask
#define SPSR_EL1_M30_MASK                              BITS(0, 3)
/// @brief SPSR_EL M[3:0] field exctraction
#define SPSR_EL1_M30(value)                            ((value >> SPSR_EL1_M30_SHIFT) & SPSR_EL1_M30_MASK)
/// @brief SPSR_EL M[3:0] field value for EL0t mode
#define SPSR_EL_M30_EL0t                               0x0
/// @brief SPSR_EL M[3:0] field value for EL1t mode
#define SPSR_EL_M30_EL1t                               0x4
/// @brief SPSR_EL M[3:0] field value for EL1h mode
#define SPSR_EL_M30_EL1h                               0x5
/// @brief SPSR_EL M[3:0] field value for EL2t mode
#define SPSR_EL_M30_EL2t                               0x8
/// @brief SPSR_EL M[3:0] field value for EL2h mode
#define SPSR_EL_M30_EL2h                               0x9
/// @brief SPSR_EL M[3:0] field value for EL3t mode
#define SPSR_EL_M30_EL3t                               0xC
/// @brief SPSR_EL M[3:0] field value for EL3h mode
#define SPSR_EL_M30_EL3h                               0xD

/// @brief ESR_EL1 EC field bit shift
#define ESR_EL1_EC_SHIFT                               26
/// @brief ESR_EL1 EC field bit mask
#define ESR_EL1_EC_MASK                                BITS(0, 5)
/// @brief ESR_EL1 EC field extraction
#define ESR_EL1_EC(value)                              ((value >> ESR_EL1_EC_SHIFT) & ESR_EL1_EC_MASK)
/// @brief ESR_EL1 EC field value for unknown exception
#define ESR_EL1_EC_UNKNOWN                             0x00
/// @brief ESR_EL1 EC field value for trapped WF<x> instruction exception
#define ESR_EL1_EC_TRAPPED_WFx_INSTRUCTION             0x01
/// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0F
#define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0F    0x03
/// @brief ESR_EL1 EC field value for MCRR or MRRC instruction exception when coproc = 0x0F
#define ESR_EL1_EC_TRAPPED_MCRR_MRRC_ACCESS_CORPROC_0F 0x04
/// @brief ESR_EL1 EC field value for MCR or MRC instruction exception when coproc = 0x0E
#define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_COPROC_0E    0x05
/// @brief ESR_EL1 EC field value for trapped LDC or STC instruction exception
#define ESR_EL1_EC_TRAPPED_LDC_STC_ACCESS              0x06
/// @brief ESR_EL1 EC field value for unknown SME, SVE, SIMD or Floating pointer instruction exception
#define ESR_EL1_EC_TRAPPED_SME_SVE_SIMD_FP_ACCESS      0x07
/// @brief ESR_EL1 EC field value for trapped LD64<x> or ST64<x> instruction exception
#define ESR_EL1_EC_TRAPPED_LD64x_ST64x_ACCESS          0x0A
/// @brief ESR_EL1 EC field value for trapped MRRC instruction exception when coproc = 0x0C
#define ESR_EL1_EC_TRAPPED_MRRC_ACCESS_COPROC_0E       0x0C
/// @brief ESR_EL1 EC field value for branch target exception
#define ESR_EL1_EC_BRANCH_TARGET_EXCEPTION             0x0D
/// @brief ESR_EL1 EC field value for illegal executions state exception
#define ESR_EL1_EC_ILLEGAL_EXECUTION_STATE             0x0E
/// @brief ESR_EL1 EC field value for trapped SVC 32 bit instruction exception
#define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_32          0x11
/// @brief ESR_EL1 EC field value for trapped SVC 64 bit instruction exception
#define ESR_EL1_EC_TRAPPED_SVC_INSTRUCTION_64          0x15
/// @brief ESR_EL1 EC field value for MCR or MRC 64 bit instruction exception
#define ESR_EL1_EC_TRAPPED_MCR_MRC_ACCESS_64           0x18
/// @brief ESR_EL1 EC field value for trapped SVE access exception
#define ESR_EL1_EC_TRAPPED_SVE_ACCESS                  0x19
/// @brief ESR_EL1 EC field value for trapped TStart access exception
#define ESR_EL1_EC_TRAPPED_TSTART_ACCESS               0x1B
/// @brief ESR_EL1 EC field value for pointer authentication failure exception
#define ESR_EL1_EC_POINTER_AUTHENTICATION_FAILURE      0x1C
/// @brief ESR_EL1 EC field value for trapped SME access exception
#define ESR_EL1_EC_TRAPPED_SME_ACCESS                  0x1D
/// @brief ESR_EL1 EC field value for granule protection check exception
#define ESR_EL1_EC_GRANULE_PROTECTION_CHECK            0x1E
/// @brief ESR_EL1 EC field value for instruction abort from lower EL exception
#define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_LOWER_EL     0x20
/// @brief ESR_EL1 EC field value for instruction abort from same EL exception
#define ESR_EL1_EC_INSTRUCTION_ABORT_FROM_SAME_EL      0x21
/// @brief ESR_EL1 EC field value for PC alignment fault exception
#define ESR_EL1_EC_PC_ALIGNMENT_FAULT                  0x22
/// @brief ESR_EL1 EC field value for data abort from lower EL exception
#define ESR_EL1_EC_DATA_ABORT_FROM_LOWER_EL            0x24
/// @brief ESR_EL1 EC field value for data abort from same EL exception
#define ESR_EL1_EC_DATA_ABORT_FROM_SAME_EL             0x25
/// @brief ESR_EL1 EC field value for SP alignment fault exception
#define ESR_EL1_EC_SP_ALIGNMENT_FAULT                  0x27
/// @brief ESR_EL1 EC field value for trapped 32 bit FP instruction exception
#define ESR_EL1_EC_TRAPPED_FP_32                       0x28
/// @brief ESR_EL1 EC field value for trapped 64 bit FP instruction exception
#define ESR_EL1_EC_TRAPPED_FP_64                       0x2C
/// @brief ESR_EL1 EC field value for SError interrupt exception
#define ESR_EL1_EC_SERROR_INTERRUPT                    0x2F
/// @brief ESR_EL1 EC field value for Breakpoint from lower EL exception
#define ESR_EL1_EC_BREAKPOINT_FROM_LOWER_EL            0x30
/// @brief ESR_EL1 EC field value for Breakpoint from same EL exception
#define ESR_EL1_EC_BREAKPOINT_FROM_SAME_EL             0x31
/// @brief ESR_EL1 EC field value for SW step from lower EL exception
#define ESR_EL1_EC_SW_STEP_FROM_LOWER_EL               0x32
/// @brief ESR_EL1 EC field value for SW step from same EL exception
#define ESR_EL1_EC_SW_STEP_FROM_SAME_EL                0x33
/// @brief ESR_EL1 EC field value for Watchpoint from lower EL exception
#define ESR_EL1_EC_WATCHPOINT_FROM_LOWER_EL            0x34
/// @brief ESR_EL1 EC field value for Watchpoint from same EL exception
#define ESR_EL1_EC_WATCHPOINT_FROM_SAME_EL             0x35
/// @brief ESR_EL1 EC field value for 32 bit BKPT instruction exception
#define ESR_EL1_EC_BKPT_32                             0x38
/// @brief ESR_EL1 EC field value for 64 bit BRK instruction exception
#define ESR_EL1_EC_BRK_64                              0x3C

/// @brief ESR_EL1 ISS field bit shift
#define ESR_EL1_ISS_SHIFT                              0
/// @brief ESR_EL1 ISS field bit mask
#define ESR_EL1_ISS_MASK                               BITS(0, 24)
/// @brief ESR_EL1 ISS field extraction
#define ESR_EL1_ISS(value)                             ((value >> ESR_EL1_ISS_SHIFT) & ESR_EL1_ISS_MASK)
