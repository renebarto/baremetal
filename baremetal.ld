/*------------------------------------------------------------------------------
// Copyright   : Copyright(c) 2023 Rene Barto
//
// File        : MemoryAccess.cpp
//
// Namespace   : baremetal
//
// Class       : MemoryAccess
//
// Description : Memory read/write
//
//------------------------------------------------------------------------------
//
// Baremetal - A C++ bare metal environment for embedded 64 bit ARM devices
//
// Intended support is for 64 bit code only, running on Raspberry Pi (3 or 4) and Odroid
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
//------------------------------------------------------------------------------*/

ENTRY(_start)

PHDRS
{
    init PT_LOAD FILEHDR PHDRS FLAGS(RE);
    text PT_LOAD FILEHDR PHDRS FLAGS(RE);
    rodata PT_LOAD FLAGS(RE);
    data PT_LOAD FLAGS(RWE);
}

SECTIONS
{
    . = 0x80000;
    .text : {
        KEEP(*(.text.boot))
        *(.text* .text.* .gnu.linkonce.t*)

        _etext = .;
    } : text

    . = SIZEOF_HEADERS;
    .init : {
        *(.init)
    } : init

    .fini : {
        *(.fini)
    } : fini

    .rodata : {
        *(.rodata*)
    } : rodata

    .init_array : {
        __init_start = .;

        KEEP(*(.init_array*))

        __init_end = .;
    }

    .data : {
        *(.data*)
    } : data

    .bss : {
        __bss_start = .;

        *(.bss*)
        *(COMMON)

        __bss_end = .;
    } : data
}
/* bss size is actual size rounded down to blocks of 8 bytes */
__bss_size = (__bss_end - __bss_start) >> 3;
